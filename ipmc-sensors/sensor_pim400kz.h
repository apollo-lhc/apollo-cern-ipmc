/* 
    Description:
    This header file defines the interfaces
    provided by the sensor_pim400kz.c module.

    $Revision: 12269 $
*/

#ifndef SENSOR_PIM400KZ_H
#define SENSOR_PIM400KZ_H

#include <sensor.h>

/* Read-only info structure of an Template sensor */
typedef struct {
  sensor_ro_t s;
  unsigned char type;
} sensor_pim400kz_ro_t;

/* Template sensor methods */
extern sensor_methods_t PROGMEM sensor_pim400kz_methods;

/* Auxiliary macro for defining PIM400KZ sensor info */
#define SENSOR_PIM400KZ(s, type_p, alert)       \
  {SA(sensor_pim400kz_methods, s, alert), .type = (type_p)}

#endif /* SENSOR_PIM400KZ_H */
