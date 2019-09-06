/* Sensor related header file */ 
#include <sensor_tcn75.h>

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
#include <user_tcn75.h> /* base driver for temperature sensor */

/* Compile the source only if at least one SENSOR_TCN75 is implemented by the user */
#ifdef CFG_SENSOR_TCN75

// sensors can disappear 
#define CFG_SENSOR_TCN75_DYNAMIC

// /* Check that the Sensor_i2c bus is implemented */ 
// #define HAS_MASTERONLY_I2C
// 
// #ifndef HAS_MASTERONLY_I2C 
// #error Enable master-only I2C support to use TCN75 sensors. 
// #endif 

/* Forward declarations */
static char
sensor_tcn75_set_thresholds(sensor_t *sensor
                            , unsigned char mask
                            , unsigned char *thresholds);

static char
sensor_tcn75_set_hysteresis(sensor_t *sensor
                            , unsigned char *hysteresis);

static char
sensor_tcn75_fill_reading(sensor_t *sensor
                          , unsigned char *msg);

static char
sensor_tcn75_init(sensor_t *sensor);

/* ------------------------------------------------------------------ */

/* TCN75 temperature sensor methods */
sensor_methods_t PROGMEM sensor_tcn75_methods = {
    .fill_event     = sensor_threshold_fill_event,
    .fill_reading   = sensor_tcn75_fill_reading,
    .rearm          = sensor_threshold_rearm,
    .set_thresholds = sensor_tcn75_set_thresholds,
    .get_thresholds = sensor_threshold_get_thresholds,
    .set_hysteresis = sensor_tcn75_set_hysteresis,
    .get_hysteresis = sensor_threshold_get_hysteresis,
    .init           = sensor_tcn75_init
};

/* CFG_SENSOR_TCN75 defined in impc-config/config_sensors.h */ 
static const
sensor_tcn75_ro_t PROGMEM sensor_tcn75_ro[] = { CFG_SENSOR_TCN75 }; 

#define TCN75_COUNT sizeofarray(sensor_tcn75_ro)

/* Read-write info structures of TCN75 temperature sensors */ 
static struct sensor_tcn75 { 
  sensor_threshold_t sensor;
  unsigned char flags;
} sensor_tcn75[TCN75_COUNT] WARM_BSS; 
typedef struct sensor_tcn75 sensor_tcn75_t; 

static unsigned short tcn75_first; 
DECL_SENSOR_GROUP(master, sensor_tcn75_ro, sensor_tcn75, &tcn75_first); 

/* TCN75 temperature sensor flags */
#define TCN75_UPDATE		(1 << 0)	/* need to re-read the sensor */
#define TCN75_INITIALIZED	(1 << 1)	/* the sensor is initialized */

/* Global flags for all TCN75 sensors */
static unsigned char sensor_tcn75_global_flags;
#define TCN75_GLOBAL_INIT	(1 << 0)	/* initialize all sensors */
#define TCN75_GLOBAL_UPDATE	(1 << 1)	/* update all sensors */

/* ------------------------------------------------------------------ */

/*
    The following function updates the TCN75
    hardware registers.
*/
static void
sensor_tcn75_update_limits(unsigned char num)
{
  unsigned char tos, thyst;
  sensor_tcn75_t *tcn75;
  
  tcn75 = &sensor_tcn75[num];
  
  /* calculate Tos and Thyst values */
  tos = tcn75->sensor.thresholds[4];
  thyst = tcn75->sensor.thresholds[4] - tcn75->sensor.hysteresis[0] - 1;
  
#ifdef DEBUG
  debug_printf(PSTR("tcn75 #%02x, thyst = %02x, tos = %02x \n", num, thyst, tos));
#endif
  
  /* set the lower temperature limit (DGH upper critical) */
  user_tcn75_write_reg(sensor_tcn75_ro[num].id
                       , TCN75_THYST_REG
                       , thyst);
    
    /* set the upper temperature limit (AGH upper critical) */
  user_tcn75_write_reg(sensor_tcn75_ro[num].id
                       , TCN75_TOS_REG
                       , tos);
}

/*
    The following function updates the reading
    of the given TCN75 sensor.
*/
static void
sensor_tcn75_update_reading(unsigned char num
                            , unsigned char flags)
{
  unsigned char snum, reading;
  sensor_t *sensor;
  
  snum = tcn75_first + num;
  sensor = sensor_get_struct(&master_sensor_set, snum);
  if (sensor == NULL) {
    return;
  }
  
  /* Do not update non present sensors */
  if (sensor->status & STATUS_NOT_PRESENT) {
    return;
  }
    
  /* read temperature */
#ifndef CFG_SENSOR_TCN75_DYNAMIC
  if (!sensor_tcn75_read_temp(sensor_tcn75_ro[num].id
                              , TCN75_TEMP_REG
                              , &reading)) {
    /* update sensor reading */
    sensor_threshold_update(&master_sensor_set, snum, reading, flags);
  }
#else
  if (user_tcn75_read_reg(sensor_tcn75_ro[num].id
                             , TCN75_TEMP_REG
                             , &reading)) {
    /* the sensor does not respond */
    if (!(sensor->status & STATUS_SENSOR_DISABLE)) {
      log_preamble();
      debug_printf("disabling TCN75 sensor at u%d \n"
                   , sensor_tcn75_ro[num].id);
      sensor->status |= STATUS_SENSOR_DISABLE;
    }
  } else {
    if (sensor->status & STATUS_SENSOR_DISABLE) {
      log_preamble();
      debug_printf("enabling TCN75 sensor at %d \n"
                   ,sensor_tcn75_ro[num].id);
      sensor->status &= ~STATUS_SENSOR_DISABLE;
    }
    /* update sensor reading */
    sensor_threshold_update(&master_sensor_set, snum, reading, flags);
  }
#endif
}


/*
    The following function initializes the given
    TCN75 sensor.
*/
static void
sensor_tcn75_initialize(unsigned char num)
{
  sensor_tcn75_t *tcn75;

  tcn75 = &sensor_tcn75[num];
  
  /* skip initialization, if it is already initialized or not present */
  if ((tcn75->flags & TCN75_INITIALIZED) ||
      (tcn75->sensor.s.status & STATUS_NOT_PRESENT)) {
    return;
  }
  
#ifdef DEBUG
  debug_printf(PSTR("tcn75 #%02x, initialize\n", num));
#endif
  
  /* Configure TCN75:
   *  - interrupt mode
   *  - active low
   *  - one fault to generate event
   */
  user_tcn75_write_conf(sensor_tcn75_ro[num].id
                          , TCN75_CFG_TM | TCN75_CFG_FT_1);
  
  /* update TCN75 limits */
  sensor_tcn75_update_limits(num);
  
  /* read the current temperature */
  sensor_tcn75_update_reading(num, SENSOR_INITIAL_UPDATE);
}

/*
    Sensor initialization.
*/
static char
sensor_tcn75_init(sensor_t *sensor)
{
  unsigned char num = ((struct sensor_tcn75 *) sensor) - sensor_tcn75;
  sensor_tcn75_initialize(num);
  return 0;
}

/* ------------------------------------------------------------------ */

/*
    This section contains various callbacks
    registered by the TCN75 driver.
*/

/* Interrupt handler */
// static void
// sensor_tcn75_intr(unsigned char irq)
// {
//   unsigned char i;
//   
// #ifdef DEBUG
//   debug_printf("interrupt #%02x \n", irq);
// #endif
//   
//   /* temporary block irq */
//   irq_block(irq);
//   
//   for (i = 0; i < TCN75_COUNT; i++) {
//     if (PRG_RD(sensor_tcn75_ro[i].irq) == irq) {
//       sensor_tcn75[i].flags |= TCN75_UPDATE;
//     }
//   }
// }

#ifdef NEED_SLAVE_CALLBACKS
/* Slave up callback */
SLAVE_UP_CALLBACK(sensor_tcn75_slave_up)
{
  /* schedule global TCN75 init */
  sensor_tcn75_global_flags |= TCN75_GLOBAL_INIT;
}

/* Slave down callback */
SLAVE_DOWN_CALLBACK(sensor_tcn75_slave_down)
{
  unsigned char i;
  
  /* unschedule global TCN75 init */
  sensor_tcn75_global_flags &= ~TCN75_GLOBAL_INIT;
  
  for (i = 0; i < TCN75_COUNT; i++) {
    /* mark the sensor as not initialized */
    sensor_tcn75[i].flags &= ~TCN75_INITIALIZED;
    
    // /* disable interrupts from this sensor */
    // if (PRG_RD(sensor_tcn75_ro[i].irq) < IRQ_NUM) {
    //   irq_block(PRG_RD(sensor_tcn75_ro[i].irq));
    // }
  }
}
#endif

/* 1 second callback */
TIMER_CALLBACK(1s, sensor_tcn75_1s_callback)
{
    unsigned char flags;

    /* schedule global TCN75 update */
    save_flags_cli(flags);
    sensor_tcn75_global_flags |= TCN75_GLOBAL_UPDATE;
    restore_flags(flags);
}

/* Main loop callback */
MAIN_LOOP_CALLBACK(sensor_tcn75_poll)
{
  // unsigned char i, flags, gflags, irq;
  unsigned char i;
  unsigned char flags;
  unsigned char gflags;
  sensor_tcn75_t *tcn75;
  
  /* get/clear global TCN75 flags */
  save_flags_cli(flags);
  gflags = sensor_tcn75_global_flags;
  sensor_tcn75_global_flags = 0;
  restore_flags(flags);
  
  if (gflags & TCN75_GLOBAL_INIT) {
    /* make a delay to let the slave (if any) stabilize */
    udelay(20000);
    
    /* initialize all TCN75 */
    for (i = 0; i < TCN75_COUNT; i++) {
      sensor_tcn75_initialize(i);
    }
    
    // /* acknowledge all interrupts */
    // for (i = 0; i < TCN75_COUNT; i++) {
    //   irq = PRG_RD(sensor_tcn75_ro[i].irq);
    //   if (irq < IRQ_NUM) {
    //     irq_ack(irq);
    //   }
    // }
    
    /* mark all TCN75 as initialized and */
    /* schedule their updates */
    save_flags_cli(flags);
    for (i = 0; i < TCN75_COUNT; i++) {
      sensor_tcn75[i].flags = TCN75_INITIALIZED | TCN75_UPDATE;
    }
    restore_flags(flags);
  }
  
  for (i = 0; i < TCN75_COUNT; i++) {
    tcn75 = &sensor_tcn75[i];
    
    /* check if sensor update is needed */
    if ((gflags & TCN75_GLOBAL_UPDATE) || (tcn75->flags & TCN75_UPDATE)) {
      /* clear the update flag */
      save_flags_cli(flags);
      tcn75->flags &= ~TCN75_UPDATE;
      restore_flags(flags);
      
      /* update sensor reading */
      sensor_tcn75_update_reading(i, 0);
    }
  }
  
  // /* enable all interrupts */
  // for (i = 0; i < TCN75_COUNT; i++) {
  //   irq = PRG_RD(sensor_tcn75_ro[i].irq);
  //   if (irq < IRQ_NUM) {
  //     irq_unblock(irq);
  //   }
  // }
}

/* Initialization callback */
INIT_CALLBACK(sensor_tcn75_init_callback)
{
  // unsigned char i;
  unsigned char flags;
  // unsigned char i, irq, flags;
  // BITMAP_INIT(irq_mask, IRQ_NUM);
  
  // for (i = 0; i < TCN75_COUNT; i++) {
  //   irq  = PRG_RD(sensor_tcn75_ro[i].irq);
  //   
  //   /* Configure irq:
  //    *  - block irq
  //    *  - low level for interrupt
  //    *  - register interrupt handler
  //    */
  //   if (irq < IRQ_NUM && !BITMAP_ISSET(irq_mask, irq)) {
  //     irq_block(irq);
  //     irq_conf(irq, IRQ_LOW_LEVEL);
  //     irq_register(irq, sensor_tcn75_intr);
  //     BITMAP_SET(irq_mask, irq);
  //   }
  // }
  
  /* schedule global initialization */
  save_flags_cli(flags);
  sensor_tcn75_global_flags |= TCN75_GLOBAL_INIT;
  restore_flags(flags);
}

/* ------------------------------------------------------------------ */

/*
  This section contains TCN75 sensor methods.
*/

/* Set sensor thresholds */
static char
sensor_tcn75_set_thresholds(sensor_t *sensor
                            , unsigned char mask
                            , unsigned char *thresholds)
{
  /* set thesholds */
  sensor_threshold_set_thresholds(sensor, mask, thresholds);
  
  /* update limits */
  sensor_tcn75_update_limits(((sensor_tcn75_t*)sensor) - sensor_tcn75);
  
  return 0;
}

/* Set sensor hysteresis */
static char
sensor_tcn75_set_hysteresis(sensor_t *sensor
                            , unsigned char *hysteresis)
{
  /* set hysteresis */
  sensor_threshold_set_hysteresis(sensor, hysteresis);
  
  /* update limits */
  sensor_tcn75_update_limits(((sensor_tcn75_t*)sensor) - sensor_tcn75);
  
  return 0;
}

/* Fill the Get Sensor Reading reply */
static char
sensor_tcn75_fill_reading(sensor_t *sensor
                          , unsigned char *msg)
{
  /* update current reading */
  sensor_tcn75_update_reading(((sensor_tcn75_t *)sensor) - sensor_tcn75, 0);
  
  /* fill the reply */
  return sensor_threshold_fill_reading(sensor, msg);
}


/////* ------------------------------------------------------------------ */ 
/////* This section contains functions specific to the device.			  */ 
/////* ------------------------------------------------------------------ */ 
////unsigned char
////initialize_sensor_tcn75(unsigned char id)
////{ 
////    return 0x00; 
////} 
//// 
////char
////read_sensor_tcn75(unsigned char id, unsigned char * temp)
////{
////  char ret = user_tcn75_read(id, temp);
////  return ret;
////} 
////
/////* ------------------------------------------------------------------ */ 
/////* This section contains Template sensor methods. 					  */ 
/////* ------------------------------------------------------------------ */ 
//// 
/////* Fill the Get Sensor Reading reply */ 
////static char
////sensor_tcn75_fill_rd(sensor_t *sensor, unsigned char *msg)
////{ 
////  /* Get instance index using the pointer address */ 
////  unsigned char i, sval; 
////  unsigned short snum;
////  char ret;
////
////  i = ((sensor_tcn75_t *) sensor) - sensor_tcn75;  
////  ret = read_sensor_tcn75(sensor_tcn75_ro[i].id, &sval); 
////  snum = i + sensor_tcn75_first; 
////  
////  /* Update sensor value */
////  if (ret == 0 ){
////    sensor_threshold_update(&master_sensor_set, snum, sval, 0);
////  }
////  
////  return sensor_threshold_fill_reading(sensor, msg); 
////} 
////
/////* Sensor initialization. */ 
////static char
////sensor_tcn75_init(sensor_t *sensor)
////{ 
////  /* Get instance index using the pointer address */ 
////  unsigned char i = ((sensor_tcn75_t *) sensor) - sensor_tcn75; 
////  
////  /* Execute init function */ 
////  initialize_sensor_tcn75(sensor_tcn75_ro[i].id); 
//// 
////  return 0;    
////} 
//// 
/////* ------------------------------------------------------------------ */ 
/////* This section contains callbacks used to manage the sensor. 		  */ 
/////* ------------------------------------------------------------------ */ 
//// 
/////* 1 second callback */ 
////TIMER_CALLBACK(1s, sensor_tcn75_1s_callback)
////{ 
////  unsigned char flags; 
////  
////  /*
////   * -> Save interrupt state and disable interrupts 
////   *    Note: that ensure flags variable is not written by 
////   *          two processes at the same time. 
////   */ 
////  save_flags_cli(flags); 
////  
////  /* Change flag to schedule and update */ 
////  sensor_tcn75_global_flags |= TCN75_GLOBAL_UPDATE; 
////  
////  /* 
////   * -> Restore interrupt state and enable interrupts 
////   *   Note: restore the system 
////   */ 
////  restore_flags(flags);
////
////  return;
////} 
//// 
/////* Initialization callback */ 
////INIT_CALLBACK(sensor_tcn75_init_all)
////{ 
////    unsigned char flags; 
//// 
////    /* 
////     * -> Save interrupt state and disable interrupts 
////     *   Note: that ensure flags variable is not written by 
////     *         two processes at the same time. 
////     */ 
////    save_flags_cli(flags); 
//// 
////    /* Change flag to schedule and update */ 
////    sensor_tcn75_global_flags |= TCN75_GLOBAL_INIT; 
//// 
////    /* 
////     * -> Restore interrupt state and enable interrupts 
////     *   Note: restore the system 
////     */ 
////    restore_flags(flags); 
////} 
//// 
/////* Main loop callback */ 
////MAIN_LOOP_CALLBACK(sensor_tcn75_poll)
////{ 
////  
////  unsigned char i, flags, gflags, pcheck, sval; 
////  unsigned short snum; 
//// 
////  /* Disable interrupts */ 
////  save_flags_cli(flags); 
//// 
////  /* Saved flag state into a local variable */ 
////  gflags = sensor_tcn75_global_flags; 
//// 
////  /* Clear flags */ 
////  sensor_tcn75_global_flags = 0; 
//// 
////  /* Enable interrupts */ 
////  restore_flags(flags); 
////
////  // // disable sensors if mux is disabled.
////  // if (user_get_gpio(sense_rst_n) == 0) {
////  //   for (i = 0; i < SENSOR_TCN75_COUNT; i++) {
////  //     sensor_tcn75[i].sensor.s.status |= STATUS_NOT_PRESENT;
////  //   }
////  // } else {
////  //   for (i = 0; i < SENSOR_TCN75_COUNT; i++) {
////  //     sensor_tcn75[i].sensor.s.status &= ~STATUS_NOT_PRESENT;
////  //   }
////  // }
////  
////  if (gflags & TCN75_GLOBAL_INIT) { 
////
////    /* initialize all Template sensors */ 
////    for (i = 0; i < SENSOR_TCN75_COUNT; i++) { 
////      
////      /* Check if the sensor is present         */ 
////      /*    e.g.: can be absent in case of RTM  */ 
////      pcheck = sensor_tcn75[i].sensor.s.status; 
////      
////      if (!(pcheck & STATUS_NOT_PRESENT)) { 
////        initialize_sensor_tcn75(sensor_tcn75_ro[i].id); 
////      } 
////    } 
////  } 
//// 
////  if (gflags & TCN75_GLOBAL_UPDATE) { 
////    
////    /* update all sensor readings */ 
////    for (i = 0; i < SENSOR_TCN75_COUNT; i++) { 
////      
////      /* Check if the sensor is present         */ 
////      /*    e.g.: can be absent in case of RTM  */ 
////      pcheck = sensor_tcn75[i].sensor.s.status; 
////      snum = sensor_tcn75_first + i; 
////      if (!(pcheck & STATUS_NOT_PRESENT)) { 
////        WDT_RESET; 
////        sval = read_sensor_tcn75(sensor_tcn75_ro[i].id); 
////        sensor_threshold_update(&master_sensor_set, snum, sval, flags); 
////      } 
////    } 
////  } 
////} 
 
#endif /* CFG_SENSOR_TCN75 */ 
