/* Sensor related header file */ 
#include <sensor_mcu.h>

// their headers
#include <defs.h> 
#include <cfgint.h> 
#include <debug.h> 
#include <log.h>              /* Log functions */ 

// #include <sensor.h>           /* Sensors functions */ 
#include <sensor_threshold.h> /* Threshold related functions */ 
// #include <sensor_discrete.h>  /* Discrete sensor functions */ 

//#include <hal/i2c.h>    /* I2C functions */ 
//#include <i2c_dev.h>     /* Master Only I2C functions */ 
//#include <hal/system.h>	 /* System functions */ 
//
//#include <app.h>              /* App functions */ 

// our headers
#include <user_mcu.h> /* base driver for temperature sensor */

/* Compile the source only if at least one SENSOR_MCU is implemented by the user */
#ifdef CFG_SENSOR_MCU

// sensors can disappear 
#define CFG_SENSOR_MCU_DYNAMIC

// /* Check that the Sensor_i2c bus is implemented */ 
// #define HAS_MASTERONLY_I2C
// 
// #ifndef HAS_MASTERONLY_I2C 
// #error Enable master-only I2C support to use MCU sensors. 
// #endif 

/* Forward declarations */

static char
sensor_mcu_fill_reading(sensor_t *sensor
                          , unsigned char *msg);

static char
sensor_mcu_init(sensor_t *sensor);

/* ------------------------------------------------------------------ */

/* MCU temperature sensor methods */
sensor_methods_t PROGMEM sensor_mcu_methods = {
    .fill_event     = sensor_threshold_fill_event,
    .fill_reading   = sensor_mcu_fill_reading,
    .rearm          = sensor_threshold_rearm,
    .set_thresholds = sensor_threshold_set_thresholds,
    .get_thresholds = sensor_threshold_get_thresholds,
    .set_hysteresis = sensor_threshold_set_hysteresis,
    .get_hysteresis = sensor_threshold_get_hysteresis,
    .init           = sensor_mcu_init
};

/* CFG_SENSOR_MCU defined in impc-config/config_sensors.h */ 
static const
sensor_mcu_ro_t PROGMEM sensor_mcu_ro[] = { CFG_SENSOR_MCU }; 

#define MCU_COUNT sizeofarray(sensor_mcu_ro)

/* Read-write info structures of MCU temperature sensors */ 
static struct sensor_mcu { 
  sensor_threshold_t sensor;
  unsigned char flags;
} sensor_mcu[MCU_COUNT] WARM_BSS; 
typedef struct sensor_mcu sensor_mcu_t; 

static unsigned short mcu_first; 
DECL_SENSOR_GROUP(master, sensor_mcu_ro, sensor_mcu, &mcu_first); 

/* MCU temperature sensor flags */
#define MCU_UPDATE		(1 << 0)	/* need to re-read the sensor */
#define MCU_INITIALIZED	(1 << 1)	/* the sensor is initialized */

/* Global flags for all MCU sensors */
static unsigned char sensor_mcu_global_flags;
#define MCU_GLOBAL_INIT	(1 << 0)	/* initialize all sensors */
#define MCU_GLOBAL_UPDATE	(1 << 1)	/* update all sensors */


/*
    The following function updates the reading
    of the given MCU sensor.
*/
static void
sensor_mcu_update_reading(unsigned char num
                            , unsigned char flags)
{
  unsigned char snum, reading;
  sensor_t *sensor;
  
  snum = mcu_first + num;
  sensor = sensor_get_struct(&master_sensor_set, snum);
  if (sensor == NULL) {
    return;
  }
  
  // /* Do not update non present sensors */
  // if (sensor->status & STATUS_NOT_PRESENT) {
  //   return;
  // }
    
  /* read temperature */
  if (user_mcu_get_temp(sensor_mcu_ro[num].i2c_addr,
                       sensor_mcu_ro[num].reg_addr
                       , &reading)) {
    /* the sensor does not respond */
    if (!(sensor->status & STATUS_SENSOR_DISABLE)) {
      log_preamble();
      debug_printf("disabling MCU sensor at (0x%02x, 0x%02x) \n"
                   , sensor_mcu_ro[num].i2c_addr
                   , sensor_mcu_ro[num].reg_addr);
      sensor->status |= STATUS_SENSOR_DISABLE;
      sensor->status |= STATUS_SCAN_DISABLE;
    }
  } else {
    if (sensor->status & STATUS_SENSOR_DISABLE) {
      log_preamble();
      debug_printf("enabling MCU sensor at (0x%02x, 0x%02x) \n"
                   ,sensor_mcu_ro[num].i2c_addr
                   ,sensor_mcu_ro[num].reg_addr);
      sensor->status &= ~STATUS_SENSOR_DISABLE;
      sensor->status &= ~STATUS_SCAN_DISABLE;
    }
    /* update sensor reading */
    sensor_threshold_update(&master_sensor_set, snum, reading, flags);
  }
}


/*
    The following function initializes the given
    MCU sensor.
*/
static void
sensor_mcu_initialize(unsigned char num)
{
  return;
}

/*
    Sensor initialization.
*/
static char
sensor_mcu_init(sensor_t *sensor)
{
  unsigned char num = ((struct sensor_mcu *) sensor) - sensor_mcu;
  sensor_mcu_initialize(num);
  return 0;
}


#ifdef NEED_SLAVE_CALLBACKS
/* Slave up callback */
SLAVE_UP_CALLBACK(sensor_mcu_slave_up)
{
  /* schedule global MCU init */
  sensor_mcu_global_flags |= MCU_GLOBAL_INIT;
}

/* Slave down callback */
SLAVE_DOWN_CALLBACK(sensor_mcu_slave_down)
{
  unsigned char i;
  
  /* unschedule global MCU init */
  sensor_mcu_global_flags &= ~MCU_GLOBAL_INIT;
  
  for (i = 0; i < MCU_COUNT; i++) {
    /* mark the sensor as not initialized */
    sensor_mcu[i].flags &= ~MCU_INITIALIZED;
  }
}
#endif

/* 1 second callback */
TIMER_CALLBACK(1s, sensor_mcu_1s_callback)
{
    unsigned char flags;

    /* schedule global MCU update */
    save_flags_cli(flags);
    sensor_mcu_global_flags |= MCU_GLOBAL_UPDATE;
    restore_flags(flags);
}

/* Main loop callback */
MAIN_LOOP_CALLBACK(sensor_mcu_poll)
{
  // unsigned char i, flags, gflags, irq;
  unsigned char i;
  unsigned char flags;
  unsigned char gflags;
  sensor_mcu_t *mcu;
  
  /* get/clear global MCU flags */
  save_flags_cli(flags);
  gflags = sensor_mcu_global_flags;
  sensor_mcu_global_flags = 0;
  restore_flags(flags);
  
  if (gflags & MCU_GLOBAL_INIT) {
    /* make a delay to let the slave (if any) stabilize */
    udelay(20000);
    
    /* initialize all MCU */
    for (i = 0; i < MCU_COUNT; i++) {
      sensor_mcu_initialize(i);
    }
    
    /* mark all MCU as initialized and */
    /* schedule their updates */
    save_flags_cli(flags);
    for (i = 0; i < MCU_COUNT; i++) {
      sensor_mcu[i].flags = MCU_INITIALIZED | MCU_UPDATE;
    }
    restore_flags(flags);
  }
  
  for (i = 0; i < MCU_COUNT; i++) {
    mcu = &sensor_mcu[i];
    
    /* check if sensor update is needed */
    if ((gflags & MCU_GLOBAL_UPDATE) || (mcu->flags & MCU_UPDATE)) {
      /* clear the update flag */
      save_flags_cli(flags);
      mcu->flags &= ~MCU_UPDATE;
      restore_flags(flags);
      
      /* update sensor reading */
      sensor_mcu_update_reading(i, 0);
    }
  }
}

/* Initialization callback */
INIT_CALLBACK(sensor_mcu_init_callback)
{
  unsigned char flags;
  
  /* schedule global initialization */
  save_flags_cli(flags);
  sensor_mcu_global_flags |= MCU_GLOBAL_INIT;
  restore_flags(flags);
}

/* ------------------------------------------------------------------ */

/*
  This section contains MCU sensor methods.
*/

/* Fill the Get Sensor Reading reply */
static char
sensor_mcu_fill_reading(sensor_t *sensor
                          , unsigned char *msg)
{
  /* update current reading */
  sensor_mcu_update_reading(((sensor_mcu_t *)sensor) - sensor_mcu, 0);
  
  /* fill the reply */
  return sensor_threshold_fill_reading(sensor, msg);
}
 
#endif /* CFG_SENSOR_TCN75 */ 
