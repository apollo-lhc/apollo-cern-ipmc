/* 
    Description:
    This header file defines the interfaces
    provided by the sensor_tcn75.c module.

    $Revision: 12269 $
*/

#ifndef SENSOR_TCN75_H
#define SENSOR_TCN75_H

#include <sensor.h>

/* Read-only info structure of an Template sensor */
typedef struct {
  sensor_ro_t s;
  unsigned char id;
} sensor_tcn75_ro_t;

/* Template sensor methods */
extern sensor_methods_t PROGMEM sensor_tcn75_methods;

/* Auxiliary macro for defining TCN sensor info */
#define SENSOR_TCN75(s, id_p, alert)      \
    {					         \
      SA(sensor_tcn75_methods, s, alert),       \
      .id= (id_p)                                \
    }
     
#endif /* SENSOR_TCN75_H */
