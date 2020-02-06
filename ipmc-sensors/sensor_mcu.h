/* 
    Description:
    This header file defines the interfaces
    provided by the sensor_mcu.c module.

    $Revision: 12269 $
*/

#ifndef SENSOR_MCU_H
#define SENSOR_MCU_H

#include <sensor.h>

/* Read-only info structure of an Template sensor */
typedef struct {
  sensor_ro_t s;
  unsigned char i2c_addr;
  unsigned char reg_addr;
} sensor_mcu_ro_t;

/* Template sensor methods */
extern sensor_methods_t PROGMEM sensor_mcu_methods;

/* Auxiliary macro for defining TCN sensor info */
#define SENSOR_MCU(s, i2c_addr_p, reg_addr_p, alert)             \
{                                                               \
  SA(sensor_mcu_methods, s, alert)                               \
  , .i2c_addr = (i2c_addr_p)                                    \
  , .reg_addr = (reg_addr_p)                                    \
}

#endif /* SENSOR_MCU_H */
