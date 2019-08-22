#define NEED_MASTERONLY_I2C		/* Check that the Sensor_i2c bus is implemented */ 
 
#include <defs.h> 
#include <cfgint.h> 
#include <debug.h> 
 
#ifdef CFG_SENSOR_TCN75A			/* Compile the source only if at least one SENSOR_TCN75A is implemented by the user */ 
 
#include <hal/i2c.h>				/* I2C functions */ 
#include <i2c_dev.h>				/* Master Only I2C functions */ 
#include <hal/system.h>				/* System functions */ 
	 
#include <app.h>					/* App functions */ 
#include <log.h>					/* Log functions */ 
#include <sensor.h>					/* Sensors functions */ 
#include <sensor_discrete.h>		/* Discrete sensor functions */ 
#include <sensor_threshold.h>		/* Threshold related functions */ 
 
#include <sensor_tcn75a.h>		/* Sensor related header file */ 

#include <user_tcn75a.h> /* base driver for temperature sensor */

#ifndef HAS_MASTERONLY_I2C 
    #error Enable master-only I2C support to use TCN75A sensors. 
#endif 
 
 
/* ------------------------------------------------------------------ */ 
/* Sensor's methods 			                              */ 
/* ------------------------------------------------------------------ */ 
 
/* Sensor specific methods */ 

/* Function called by the core to initialize the device */ 
static char sensor_tcn75a_init(sensor_t *sensor);
 
sensor_methods_t PROGMEM sensor_tcn75a_methods =
  { 
   /* Called when the core asks for event */ 
  fill_event:           &sensor_threshold_fill_event,
  /* Called when the core asks for sensor value */ 
  fill_reading:	&sensor_tcn75a_fill_rd,
  /* Called when the core asks for sensor arm */ 
  rearm:		&sensor_threshold_rearm,
  /* Called when a new threshold value is forced */ 
  set_thresholds:	&sensor_threshold_set_thresholds,
  /* Called when thresholds value are requested */ 
  get_thresholds:	&sensor_threshold_get_thresholds,
  /* Called to set a new threshold hysteresis value */ 
  set_hysteresis:	&sensor_threshold_set_hysteresis,
  /* Called to get the threshold hysteresis value */ 
  get_hysteresis:	&sensor_threshold_get_hysteresis,
  /* Called when the core asks for sensor init */
  init:			&sensor_tcn75a_init
}; 

/* ------------------------------------------------------------------ */ 
/* Memory allocation 						      */ 
/* ------------------------------------------------------------------ */ 
/* CFG_SENSOR_TCN75A defined in impc-config/config_sensors.h */ 
static const
sensor_tcn75a_ro_t PROGMEM sensor_tcn75a_ro[] = {CFG_SENSOR_TCN75A}; 




#define SENSOR_TCN75A_COUNT	sizeofarray(sensor_tcn75a_ro) 

/* Read-write info structures of TCN75A temperature sensors */ 
static struct sensor_tcn75a { 
    sensor_threshold_t	sensor; 
} sensor_tcn75a[SENSOR_TCN75A_COUNT] WARM_BSS; 

typedef struct sensor_tcn75a sensor_tcn75a_t; 


/* ------------------------------------------------------------------ */ 
/* Sensor declaration						      */ 
/* ------------------------------------------------------------------ */ 
static unsigned short sensor_tcn75a_first; 
DECL_SENSOR_GROUP(master, sensor_tcn75a_ro, sensor_tcn75a, &sensor_tcn75a_first); 
 
/* ------------------------------------------------------------------ */ 
/* Flag variable and state											  */ 
/* ------------------------------------------------------------------ */ 
static unsigned char sensor_tcn75a_global_flags; 
 
#define TCN75A_GLOBAL_INIT	(1 << 0)	/* initialize all sensors */ 
#define TCN75A_GLOBAL_UPDATE	(1 << 1)	/* update all sensors */ 

/* ------------------------------------------------------------------ */ 
/* This section contains functions specific to the device.			  */ 
/* ------------------------------------------------------------------ */ 
unsigned char
initialize_sensor_tcn75a(unsigned char id)
{ 
    return 0x00; 
} 
 
unsigned char
read_sensor_tcn75a(unsigned char id)
{
  unsigned char temp;
  char ret = user_tcn75a_read(34, &temp);

  // problem in the sensor reading... 
  if (ret != 0) {
    return 0xFF;     
  }

  return temp;
} 

/* ------------------------------------------------------------------ */ 
/* This section contains Template sensor methods. 					  */ 
/* ------------------------------------------------------------------ */ 
 
/* Fill the Get Sensor Reading reply */ 
static char
sensor_tcn75a_fill_rd(sensor_t *sensor, unsigned char *msg)
{ 
  /* Get instance index using the pointer address */ 
  unsigned char i, sval; 
  unsigned short snum; 
  
  i = ((sensor_tcn75a_t *) sensor) - sensor_tcn75a; 
  sval = read_sensor_tcn75a(sensor_tcn75a_ro[i].id); 
  snum = i + sensor_tcn75a_first; 
  
  /* Update sensor value */ 
  sensor_threshold_update(&master_sensor_set, snum, sval, 0); 
  
  return sensor_threshold_fill_reading(sensor, msg); 
} 

/* Sensor initialization. */ 
static char
sensor_tcn75a_init(sensor_t *sensor)
{ 
  /* Get instance index using the pointer address */ 
  unsigned char i = ((sensor_tcn75a_t *) sensor) - sensor_tcn75a; 
  
  /* Execute init function */ 
  initialize_sensor_tcn75a(sensor_tcn75a_ro[i].id); 
 
  return 0;    
} 
 
/* ------------------------------------------------------------------ */ 
/* This section contains callbacks used to manage the sensor. 		  */ 
/* ------------------------------------------------------------------ */ 
 
/* 1 second callback */ 
TIMER_CALLBACK(1s, sensor_tcn75a_1s_callback)
{ 
    unsigned char flags; 
 
    /* 
     * -> Save interrupt state and disable interrupts 
     *   Note: that ensure flags variable is not written by 
     *         two processes at the same time. 
     */ 
    save_flags_cli(flags); 
 
    /* Change flag to schedule and update */ 
    sensor_tcn75a_global_flags |= TCN75A_GLOBAL_UPDATE; 
 
    /* 
     * -> Restore interrupt state and enable interrupts 
     *   Note: restore the system 
     */ 
    restore_flags(flags); 
} 
 
/* Initialization callback */ 
INIT_CALLBACK(sensor_tcn75a_init_all)
{ 
    unsigned char flags; 
 
    /* 
     * -> Save interrupt state and disable interrupts 
     *   Note: that ensure flags variable is not written by 
     *         two processes at the same time. 
     */ 
    save_flags_cli(flags); 
 
    /* Change flag to schedule and update */ 
    sensor_tcn75a_global_flags |= TCN75A_GLOBAL_INIT; 
 
    /* 
     * -> Restore interrupt state and enable interrupts 
     *   Note: restore the system 
     */ 
    restore_flags(flags); 
} 
 
/* Main loop callback */ 
MAIN_LOOP_CALLBACK(sensor_tcn75a_poll)
{ 
 
    unsigned char i, flags, gflags, pcheck, sval; 
    unsigned short snum; 
 
    /* Disable interrupts */ 
    save_flags_cli(flags); 
 
    /* Saved flag state into a local variable */ 
    gflags = sensor_tcn75a_global_flags; 
 
    /* Clear flags */ 
    sensor_tcn75a_global_flags = 0; 
 
    /* Enable interrupts */ 
    restore_flags(flags); 
 
    if (gflags & TCN75A_GLOBAL_INIT) { 
 
        /* initialize all Template sensors */ 
        for (i = 0; i < SENSOR_TCN75A_COUNT; i++) { 
 
        	/* Check if the sensor is present         */ 
        	/*    e.g.: can be absent in case of RTM  */ 
            pcheck = sensor_tcn75a[i].sensor.s.status; 
 
            if (!(pcheck & STATUS_NOT_PRESENT)) { 
                initialize_sensor_tcn75a(sensor_tcn75a_ro[i].id); 
            } 
		} 
    } 
 
    if (gflags & TCN75A_GLOBAL_UPDATE) { 
 
    	/* update all sensor readings */ 
        for (i = 0; i < SENSOR_TCN75A_COUNT; i++) { 
 
        	/* Check if the sensor is present         */ 
        	/*    e.g.: can be absent in case of RTM  */ 
        	pcheck = sensor_tcn75a[i].sensor.s.status; 
        	snum = sensor_tcn75a_first + i; 
            if (!(pcheck & STATUS_NOT_PRESENT)) { 
                WDT_RESET; 
                sval = read_sensor_tcn75a(sensor_tcn75a_ro[i].id); 
                sensor_threshold_update(&master_sensor_set, snum, sval, flags); 
            } 
        } 
    } 
 
} 
 
#endif /* CFG_SENSOR_TCN75A */ 
