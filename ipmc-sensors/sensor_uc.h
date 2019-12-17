/* 
    Description:
    This header file defines the interfaces
    provided by the sensor_uc.c module.

    $Revision: 12269 $
*/

#ifndef SENSOR_UC_H
#define SENSOR_UC_H

#include <sensor.h>

/* Read-only info structure of an Template sensor */
typedef struct {
  sensor_ro_t s;
  unsigned char addr;
} sensor_uc_ro_t;

/* Template sensor methods */
extern sensor_methods_t PROGMEM sensor_uc_methods;

/* Auxiliary macro for defining TCN sensor info */
#define SENSOR_UC(s, addr_p, alert)      \
    {					         \
      SA(sensor_uc_methods, s, alert),       \
      .addr= (addr_p)                                \
    }
     
#endif /* SENSOR_UC_H */
