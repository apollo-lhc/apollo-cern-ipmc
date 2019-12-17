/* Sensor related header file */ 
#include <sensor_uc.h>

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
#include <user_uc.h> /* base driver for temperature sensor */

/* Compile the source only if at least one SENSOR_UC is implemented by the user */
#ifdef CFG_SENSOR_UC

// sensors can disappear 
#define CFG_SENSOR_UC_DYNAMIC

// /* Check that the Sensor_i2c bus is implemented */ 
// #define HAS_MASTERONLY_I2C
// 
// #ifndef HAS_MASTERONLY_I2C 
// #error Enable master-only I2C support to use UC sensors. 
// #endif 

/* Forward declarations */

static char
sensor_uc_fill_reading(sensor_t *sensor
                          , unsigned char *msg);

static char
sensor_uc_init(sensor_t *sensor);

/* ------------------------------------------------------------------ */

/* UC temperature sensor methods */
sensor_methods_t PROGMEM sensor_uc_methods = {
    .fill_event     = sensor_threshold_fill_event,
    .fill_reading   = sensor_uc_fill_reading,
    .rearm          = sensor_threshold_rearm,
    .set_thresholds = sensor_threshold_set_thresholds,
    .get_thresholds = sensor_threshold_get_thresholds,
    .set_hysteresis = sensor_threshold_set_hysteresis,
    .get_hysteresis = sensor_threshold_get_hysteresis,
    .init           = sensor_uc_init
};

/* CFG_SENSOR_UC defined in impc-config/config_sensors.h */ 
static const
sensor_uc_ro_t PROGMEM sensor_uc_ro[] = { CFG_SENSOR_UC }; 

#define UC_COUNT sizeofarray(sensor_uc_ro)

/* Read-write info structures of UC temperature sensors */ 
static struct sensor_uc { 
  sensor_threshold_t sensor;
  unsigned char flags;
} sensor_uc[UC_COUNT] WARM_BSS; 
typedef struct sensor_uc sensor_uc_t; 

static unsigned short uc_first; 
DECL_SENSOR_GROUP(master, sensor_uc_ro, sensor_uc, &uc_first); 

/* UC temperature sensor flags */
#define UC_UPDATE		(1 << 0)	/* need to re-read the sensor */
#define UC_INITIALIZED	(1 << 1)	/* the sensor is initialized */

/* Global flags for all UC sensors */
static unsigned char sensor_uc_global_flags;
#define UC_GLOBAL_INIT	(1 << 0)	/* initialize all sensors */
#define UC_GLOBAL_UPDATE	(1 << 1)	/* update all sensors */


/*
    The following function updates the reading
    of the given UC sensor.
*/
static void
sensor_uc_update_reading(unsigned char num
                            , unsigned char flags)
{
  unsigned char snum, reading;
  sensor_t *sensor;
  
  snum = uc_first + num;
  sensor = sensor_get_struct(&master_sensor_set, snum);
  if (sensor == NULL) {
    return;
  }
  
  // /* Do not update non present sensors */
  // if (sensor->status & STATUS_NOT_PRESENT) {
  //   return;
  // }
    
  /* read temperature */
  if (user_uc_get_temp(sensor_uc_ro[num].addr
                         , &reading)) {
    /* the sensor does not respond */
    if (!(sensor->status & STATUS_SENSOR_DISABLE)) {
      log_preamble();
      debug_printf("disabling UC sensor at 0x%02x \n"
                   , sensor_uc_ro[num].addr);
      sensor->status |= STATUS_SENSOR_DISABLE;
      sensor->status |= STATUS_SCAN_DISABLE;
    }
  } else {
    if (sensor->status & STATUS_SENSOR_DISABLE) {
      log_preamble();
      debug_printf("enabling UC sensor at 0x%02x \n"
                   ,sensor_uc_ro[num].addr);
      sensor->status &= ~STATUS_SENSOR_DISABLE;
      sensor->status &= ~STATUS_SCAN_DISABLE;
    }
    /* update sensor reading */
    sensor_threshold_update(&master_sensor_set, snum, reading, flags);
  }
}


/*
    The following function initializes the given
    UC sensor.
*/
static void
sensor_uc_initialize(unsigned char num)
{
  return;
}

/*
    Sensor initialization.
*/
static char
sensor_uc_init(sensor_t *sensor)
{
  unsigned char num = ((struct sensor_uc *) sensor) - sensor_uc;
  sensor_uc_initialize(num);
  return 0;
}


#ifdef NEED_SLAVE_CALLBACKS
/* Slave up callback */
SLAVE_UP_CALLBACK(sensor_uc_slave_up)
{
  /* schedule global UC init */
  sensor_uc_global_flags |= UC_GLOBAL_INIT;
}

/* Slave down callback */
SLAVE_DOWN_CALLBACK(sensor_uc_slave_down)
{
  unsigned char i;
  
  /* unschedule global UC init */
  sensor_uc_global_flags &= ~UC_GLOBAL_INIT;
  
  for (i = 0; i < UC_COUNT; i++) {
    /* mark the sensor as not initialized */
    sensor_uc[i].flags &= ~UC_INITIALIZED;
  }
}
#endif

/* 1 second callback */
TIMER_CALLBACK(1s, sensor_uc_1s_callback)
{
    unsigned char flags;

    /* schedule global UC update */
    save_flags_cli(flags);
    sensor_uc_global_flags |= UC_GLOBAL_UPDATE;
    restore_flags(flags);
}

/* Main loop callback */
MAIN_LOOP_CALLBACK(sensor_uc_poll)
{
  // unsigned char i, flags, gflags, irq;
  unsigned char i;
  unsigned char flags;
  unsigned char gflags;
  sensor_uc_t *uc;
  
  /* get/clear global UC flags */
  save_flags_cli(flags);
  gflags = sensor_uc_global_flags;
  sensor_uc_global_flags = 0;
  restore_flags(flags);
  
  if (gflags & UC_GLOBAL_INIT) {
    /* make a delay to let the slave (if any) stabilize */
    udelay(20000);
    
    /* initialize all UC */
    for (i = 0; i < UC_COUNT; i++) {
      sensor_uc_initialize(i);
    }
    
    /* mark all UC as initialized and */
    /* schedule their updates */
    save_flags_cli(flags);
    for (i = 0; i < UC_COUNT; i++) {
      sensor_uc[i].flags = UC_INITIALIZED | UC_UPDATE;
    }
    restore_flags(flags);
  }
  
  for (i = 0; i < UC_COUNT; i++) {
    uc = &sensor_uc[i];
    
    /* check if sensor update is needed */
    if ((gflags & UC_GLOBAL_UPDATE) || (uc->flags & UC_UPDATE)) {
      /* clear the update flag */
      save_flags_cli(flags);
      uc->flags &= ~UC_UPDATE;
      restore_flags(flags);
      
      /* update sensor reading */
      sensor_uc_update_reading(i, 0);
    }
  }
}

/* Initialization callback */
INIT_CALLBACK(sensor_uc_init_callback)
{
  unsigned char flags;
  
  /* schedule global initialization */
  save_flags_cli(flags);
  sensor_uc_global_flags |= UC_GLOBAL_INIT;
  restore_flags(flags);
}

/* ------------------------------------------------------------------ */

/*
  This section contains UC sensor methods.
*/

/* Fill the Get Sensor Reading reply */
static char
sensor_uc_fill_reading(sensor_t *sensor
                          , unsigned char *msg)
{
  /* update current reading */
  sensor_uc_update_reading(((sensor_uc_t *)sensor) - sensor_uc, 0);
  
  /* fill the reply */
  return sensor_threshold_fill_reading(sensor, msg);
}
 
#endif /* CFG_SENSOR_TCN75 */ 
