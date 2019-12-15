/* Sensor related header file */ 
#include <sensor_zynq.h>

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
#include <user_zynq.h> /* base driver for temperature sensor */

/* Compile the source only if at least one SENSOR_ZYNQ is implemented by the user */
#ifdef CFG_SENSOR_ZYNQ

// sensors can disappear 
#define CFG_SENSOR_ZYNQ_DYNAMIC

// /* Check that the Sensor_i2c bus is implemented */ 
// #define HAS_MASTERONLY_I2C
// 
// #ifndef HAS_MASTERONLY_I2C 
// #error Enable master-only I2C support to use ZYNQ sensors. 
// #endif 

/* Forward declarations */

static char
sensor_zynq_fill_reading(sensor_t *sensor
                          , unsigned char *msg);

static char
sensor_zynq_init(sensor_t *sensor);

/* ------------------------------------------------------------------ */

/* ZYNQ temperature sensor methods */
sensor_methods_t PROGMEM sensor_zynq_methods = {
    .fill_event     = sensor_threshold_fill_event,
    .fill_reading   = sensor_zynq_fill_reading,
    .rearm          = sensor_threshold_rearm,
    .set_thresholds = sensor_threshold_set_thresholds,
    .get_thresholds = sensor_threshold_get_thresholds,
    .set_hysteresis = sensor_threshold_set_hysteresis,
    .get_hysteresis = sensor_threshold_get_hysteresis,
    .init           = sensor_zynq_init
};

/* CFG_SENSOR_ZYNQ defined in impc-config/config_sensors.h */ 
static const
sensor_zynq_ro_t PROGMEM sensor_zynq_ro[] = { CFG_SENSOR_ZYNQ }; 

#define ZYNQ_COUNT sizeofarray(sensor_zynq_ro)

/* Read-write info structures of ZYNQ temperature sensors */ 
static struct sensor_zynq { 
  sensor_threshold_t sensor;
  unsigned char flags;
} sensor_zynq[ZYNQ_COUNT] WARM_BSS; 
typedef struct sensor_zynq sensor_zynq_t; 

static unsigned short zynq_first; 
DECL_SENSOR_GROUP(master, sensor_zynq_ro, sensor_zynq, &zynq_first); 

/* ZYNQ temperature sensor flags */
#define ZYNQ_UPDATE		(1 << 0)	/* need to re-read the sensor */
#define ZYNQ_INITIALIZED	(1 << 1)	/* the sensor is initialized */

/* Global flags for all ZYNQ sensors */
static unsigned char sensor_zynq_global_flags;
#define ZYNQ_GLOBAL_INIT	(1 << 0)	/* initialize all sensors */
#define ZYNQ_GLOBAL_UPDATE	(1 << 1)	/* update all sensors */


/*
    The following function updates the reading
    of the given ZYNQ sensor.
*/
static void
sensor_zynq_update_reading(unsigned char num
                            , unsigned char flags)
{
  unsigned char snum, reading;
  sensor_t *sensor;
  
  snum = zynq_first + num;
  sensor = sensor_get_struct(&master_sensor_set, snum);
  if (sensor == NULL) {
    return;
  }
  
  // /* Do not update non present sensors */
  // if (sensor->status & STATUS_NOT_PRESENT) {
  //   return;
  // }
    
  /* read temperature */
  if (user_zynq_get_temp(sensor_zynq_ro[num].addr
                         , &reading)) {
    /* the sensor does not respond */
    if (!(sensor->status & STATUS_SENSOR_DISABLE)) {
      log_preamble();
      debug_printf("disabling ZYNQ sensor at 0x%02x \n"
                   , sensor_zynq_ro[num].addr);
      sensor->status |= STATUS_SENSOR_DISABLE;
      sensor->status |= STATUS_SCAN_DISABLE;
    }
  } else {
    if (sensor->status & STATUS_SENSOR_DISABLE) {
      log_preamble();
      debug_printf("enabling ZYNQ sensor at 0x%02x \n"
                   ,sensor_zynq_ro[num].addr);
      sensor->status &= ~STATUS_SENSOR_DISABLE;
      sensor->status &= ~STATUS_SCAN_DISABLE;
    }
    /* update sensor reading */
    sensor_threshold_update(&master_sensor_set, snum, reading, flags);
  }
}


/*
    The following function initializes the given
    ZYNQ sensor.
*/
static void
sensor_zynq_initialize(unsigned char num)
{
  return;
}

/*
    Sensor initialization.
*/
static char
sensor_zynq_init(sensor_t *sensor)
{
  unsigned char num = ((struct sensor_zynq *) sensor) - sensor_zynq;
  sensor_zynq_initialize(num);
  return 0;
}


#ifdef NEED_SLAVE_CALLBACKS
/* Slave up callback */
SLAVE_UP_CALLBACK(sensor_zynq_slave_up)
{
  /* schedule global ZYNQ init */
  sensor_zynq_global_flags |= ZYNQ_GLOBAL_INIT;
}

/* Slave down callback */
SLAVE_DOWN_CALLBACK(sensor_zynq_slave_down)
{
  unsigned char i;
  
  /* unschedule global ZYNQ init */
  sensor_zynq_global_flags &= ~ZYNQ_GLOBAL_INIT;
  
  for (i = 0; i < ZYNQ_COUNT; i++) {
    /* mark the sensor as not initialized */
    sensor_zynq[i].flags &= ~ZYNQ_INITIALIZED;
  }
}
#endif

/* 1 second callback */
TIMER_CALLBACK(1s, sensor_zynq_1s_callback)
{
    unsigned char flags;

    /* schedule global ZYNQ update */
    save_flags_cli(flags);
    sensor_zynq_global_flags |= ZYNQ_GLOBAL_UPDATE;
    restore_flags(flags);
}

/* Main loop callback */
MAIN_LOOP_CALLBACK(sensor_zynq_poll)
{
  // unsigned char i, flags, gflags, irq;
  unsigned char i;
  unsigned char flags;
  unsigned char gflags;
  sensor_zynq_t *zynq;
  
  /* get/clear global ZYNQ flags */
  save_flags_cli(flags);
  gflags = sensor_zynq_global_flags;
  sensor_zynq_global_flags = 0;
  restore_flags(flags);
  
  if (gflags & ZYNQ_GLOBAL_INIT) {
    /* make a delay to let the slave (if any) stabilize */
    udelay(20000);
    
    /* initialize all ZYNQ */
    for (i = 0; i < ZYNQ_COUNT; i++) {
      sensor_zynq_initialize(i);
    }
    
    /* mark all ZYNQ as initialized and */
    /* schedule their updates */
    save_flags_cli(flags);
    for (i = 0; i < ZYNQ_COUNT; i++) {
      sensor_zynq[i].flags = ZYNQ_INITIALIZED | ZYNQ_UPDATE;
    }
    restore_flags(flags);
  }
  
  for (i = 0; i < ZYNQ_COUNT; i++) {
    zynq = &sensor_zynq[i];
    
    /* check if sensor update is needed */
    if ((gflags & ZYNQ_GLOBAL_UPDATE) || (zynq->flags & ZYNQ_UPDATE)) {
      /* clear the update flag */
      save_flags_cli(flags);
      zynq->flags &= ~ZYNQ_UPDATE;
      restore_flags(flags);
      
      /* update sensor reading */
      sensor_zynq_update_reading(i, 0);
    }
  }
}

/* Initialization callback */
INIT_CALLBACK(sensor_zynq_init_callback)
{
  unsigned char flags;
  
  /* schedule global initialization */
  save_flags_cli(flags);
  sensor_zynq_global_flags |= ZYNQ_GLOBAL_INIT;
  restore_flags(flags);
}

/* ------------------------------------------------------------------ */

/*
  This section contains ZYNQ sensor methods.
*/

/* Fill the Get Sensor Reading reply */
static char
sensor_zynq_fill_reading(sensor_t *sensor
                          , unsigned char *msg)
{
  /* update current reading */
  sensor_zynq_update_reading(((sensor_zynq_t *)sensor) - sensor_zynq, 0);
  
  /* fill the reply */
  return sensor_threshold_fill_reading(sensor, msg);
}
 
#endif /* CFG_SENSOR_TCN75 */ 
