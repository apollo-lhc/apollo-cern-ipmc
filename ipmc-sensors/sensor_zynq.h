/* 
    Description:
    This header file defines the interfaces
    provided by the sensor_zynq.c module.

    $Revision: 12269 $
*/

#ifndef SENSOR_ZYNQ_H
#define SENSOR_ZYNQ_H

#include <sensor.h>

/* Read-only info structure of an Template sensor */
typedef struct {
  sensor_ro_t s;
  unsigned char addr;
} sensor_zynq_ro_t;

/* Template sensor methods */
extern sensor_methods_t PROGMEM sensor_zynq_methods;

/* Auxiliary macro for defining TCN sensor info */
#define SENSOR_ZYNQ(s, addr_p, alert)      \
    {					         \
      SA(sensor_zynq_methods, s, alert),       \
      .addr= (addr_p)                                \
    }
     
#endif /* SENSOR_ZYNQ_H */
