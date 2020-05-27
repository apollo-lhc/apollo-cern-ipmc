/* Sensor related header file */ 
#include <sensor_pim400kz.h>

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
#include <user_pim400kz.h> /* base driver for temperature sensor */

/* Compile the source only if at least one SENSOR_PIM400KZ is implemented by the user */
#ifdef CFG_SENSOR_PIM400KZ

// sensors can disappear 
#define CFG_SENSOR_PIM400KZ_DYNAMIC

// /* Check that the Sensor_i2c bus is implemented */ 
// #define HAS_MASTERONLY_I2C
// 
// #ifndef HAS_MASTERONLY_I2C 
// #error Enable master-only I2C support to use PIM400KZ sensors. 
// #endif 

/* Forward declarations */

static char
sensor_pim400kz_fill_reading(sensor_t *sensor
                             , unsigned char *msg);

static char
sensor_pim400kz_init(sensor_t *sensor);

/* ------------------------------------------------------------------ */

/* PIM400KZ temperature sensor methods */
sensor_methods_t PROGMEM sensor_pim400kz_methods = {
    .fill_event     = sensor_threshold_fill_event,
    .fill_reading   = sensor_pim400kz_fill_reading,
    .rearm          = sensor_threshold_rearm,
    .set_thresholds = sensor_threshold_set_thresholds,
    .get_thresholds = sensor_threshold_get_thresholds,
    .set_hysteresis = sensor_threshold_set_hysteresis,
    .get_hysteresis = sensor_threshold_get_hysteresis,
    .init           = sensor_pim400kz_init
};

/* CFG_SENSOR_PIM400KZ defined in impc-config/config_sensors.h */ 
static const
sensor_pim400kz_ro_t PROGMEM sensor_pim400kz_ro[] = { CFG_SENSOR_PIM400KZ }; 

#define PIM400KZ_COUNT sizeofarray(sensor_pim400kz_ro)

/* Read-write info structures of PIM400KZ temperature sensors */ 
static struct sensor_pim400kz { 
  sensor_threshold_t sensor;
  unsigned char flags;
} sensor_pim400kz[PIM400KZ_COUNT] WARM_BSS; 
typedef struct sensor_pim400kz sensor_pim400kz_t; 

static unsigned short pim400kz_first; 
DECL_SENSOR_GROUP(master, sensor_pim400kz_ro, sensor_pim400kz, &pim400kz_first); 

/* PIM400KZ temperature sensor flags */
#define PIM400KZ_UPDATE		(1 << 0)	/* need to re-read the sensor */
#define PIM400KZ_INITIALIZED	(1 << 1)	/* the sensor is initialized */

/* Global flags for all PIM400KZ sensors */
static unsigned char sensor_pim400kz_global_flags;
#define PIM400KZ_GLOBAL_INIT	(1 << 0)	/* initialize all sensors */
#define PIM400KZ_GLOBAL_UPDATE	(1 << 1)	/* update all sensors */


/*
    The following function updates the reading
    of the given PIM400KZ sensor.
*/
static void
sensor_pim400kz_update_reading(unsigned char num
                            , unsigned char flags)
{
  unsigned char snum, reading;
  sensor_t *sensor;
  int ret;

  static unsigned char temp_en_msg[]
    = "Enabling PIM400KZ temperature sensor.";
  static unsigned char temp_dis_msg[]
    = "Disabling PIM400KZ temperature sensor.";

  static unsigned char curr_en_msg[]
    = "Enabling PIM400KZ current sensor.";
  static unsigned char curr_dis_msg[]
    = "Disabling PIM400KZ current sensor.";

  snum = pim400kz_first + num;
  sensor = sensor_get_struct(&master_sensor_set, snum);
  if (sensor == NULL) {
    return;
  }
  
  /* read sensor */
  switch(sensor_pim400kz_ro[num].type){
  case 0:
    ret = user_pim400kz_get_temp(&reading);
    break;
  case 1:
    ret = user_pim400kz_get_current(&reading);
    break;
  default:
    return;
  }
    
  if (ret) {
    /* the sensor does not respond */
    if (!(sensor->status & STATUS_SENSOR_DISABLE)) {
      log_preamble();
      if (sensor_pim400kz_ro[num].type == 0) {
        debug_printf((char *) temp_dis_msg);
      } else {
        debug_printf((char *) curr_dis_msg);
      }
      sensor->status |= STATUS_SENSOR_DISABLE;
      sensor->status |= STATUS_SCAN_DISABLE;
    }
  } else {
    if (sensor->status & STATUS_SENSOR_DISABLE) {
      log_preamble();
      if (sensor_pim400kz_ro[num].type == 0) {
        debug_printf((char *) temp_en_msg);
      } else {
        debug_printf((char *) curr_en_msg);
      }
      sensor->status &= ~STATUS_SENSOR_DISABLE;
      sensor->status &= ~STATUS_SCAN_DISABLE;
    }
    /* update sensor reading */
    sensor_threshold_update(&master_sensor_set, snum, reading, flags);
  }
}


/*
    The following function initializes the given
    PIM400KZ sensor.
*/
static void
sensor_pim400kz_initialize(unsigned char num)
{
  return;
}

/*
    Sensor initialization.
*/
static char
sensor_pim400kz_init(sensor_t *sensor)
{
  unsigned char num = ((struct sensor_pim400kz *) sensor) - sensor_pim400kz;
  sensor_pim400kz_initialize(num);
  return 0;
}


#ifdef NEED_SLAVE_CALLBACKS
/* Slave up callback */
SLAVE_UP_CALLBACK(sensor_pim400kz_slave_up)
{
  /* schedule global PIM400KZ init */
  sensor_pim400kz_global_flags |= PIM400KZ_GLOBAL_INIT;
}

/* Slave down callback */
SLAVE_DOWN_CALLBACK(sensor_pim400kz_slave_down)
{
  unsigned char i;
  
  /* unschedule global PIM400KZ init */
  sensor_pim400kz_global_flags &= ~PIM400KZ_GLOBAL_INIT;
  
  for (i = 0; i < PIM400KZ_COUNT; i++) {
    /* mark the sensor as not initialized */
    sensor_pim400kz[i].flags &= ~PIM400KZ_INITIALIZED;
  }
}
#endif

/* 1 second callback */
TIMER_CALLBACK(1s, sensor_pim400kz_1s_callback)
{
    unsigned char flags;

    /* schedule global PIM400KZ update */
    save_flags_cli(flags);
    sensor_pim400kz_global_flags |= PIM400KZ_GLOBAL_UPDATE;
    restore_flags(flags);
}

/* Main loop callback */
MAIN_LOOP_CALLBACK(sensor_pim400kz_poll)
{
  // unsigned char i, flags, gflags, irq;
  unsigned char i;
  unsigned char flags;
  unsigned char gflags;
  sensor_pim400kz_t *pim400kz;
  
  /* get/clear global PIM400KZ flags */
  save_flags_cli(flags);
  gflags = sensor_pim400kz_global_flags;
  sensor_pim400kz_global_flags = 0;
  restore_flags(flags);
  
  if (gflags & PIM400KZ_GLOBAL_INIT) {
    /* make a delay to let the slave (if any) stabilize */
    udelay(20000);
    
    /* initialize all PIM400KZ */
    for (i = 0; i < PIM400KZ_COUNT; i++) {
      sensor_pim400kz_initialize(i);
    }
    
    /* mark all PIM400KZ as initialized and */
    /* schedule their updates */
    save_flags_cli(flags);
    for (i = 0; i < PIM400KZ_COUNT; i++) {
      sensor_pim400kz[i].flags = PIM400KZ_INITIALIZED | PIM400KZ_UPDATE;
    }
    restore_flags(flags);
  }
  
  for (i = 0; i < PIM400KZ_COUNT; i++) {
    pim400kz = &sensor_pim400kz[i];
    
    /* check if sensor update is needed */
    if ((gflags & PIM400KZ_GLOBAL_UPDATE) || (pim400kz->flags & PIM400KZ_UPDATE)) {
      /* clear the update flag */
      save_flags_cli(flags);
      pim400kz->flags &= ~PIM400KZ_UPDATE;
      restore_flags(flags);
      
      /* update sensor reading */
      sensor_pim400kz_update_reading(i, 0);
    }
  }
}

/* Initialization callback */
INIT_CALLBACK(sensor_pim400kz_init_callback)
{
  unsigned char flags;
  
  /* schedule global initialization */
  save_flags_cli(flags);
  sensor_pim400kz_global_flags |= PIM400KZ_GLOBAL_INIT;
  restore_flags(flags);
}

/* ------------------------------------------------------------------ */

/*
  This section contains PIM400KZ sensor methods.
*/

/* Fill the Get Sensor Reading reply */
static char
sensor_pim400kz_fill_reading(sensor_t *sensor
                          , unsigned char *msg)
{
  /* update current reading */
  sensor_pim400kz_update_reading(((sensor_pim400kz_t *)sensor) - sensor_pim400kz, 0);
  
  /* fill the reply */
  return sensor_threshold_fill_reading(sensor, msg);
}
 
#endif /* CFG_SENSOR_TCN75 */ 
