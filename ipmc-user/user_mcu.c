/***********************************************************************
Nom ......... : user_mcu.c
Role ........ : MCU driver
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 30/05/2019
***********************************************************************/

#include <user_mcu.h>

// their headers
// #include <hal/i2c.h>
// #include <i2c_dev.h>
#include <hal/time.h>
#include <debug.h>

// our headers
#include <user_i2c.h>
#include <user_gpio.h>
#include <user_pca9545.h>
#include <user_eeprom.h>
// #include <user_power_sequence.h>


// /* I2C address of the MCU I2C switch in the I2C sensor bus */
// #define MCU_I2C_ADDR   MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x30 << 1) 

/* ------------------------------------------------------------------ */
// helper


char
user_mcu_i2c_write(unsigned char addr
                    , unsigned char reg
                    , unsigned char * data
                    , char len)
{
  if (user_pca9545_write(0x02) == 0) {
    return user_i2c_reg_write(addr, reg, data, len, SENSOR_I2C_BUS);
  }
  return -1;
}




char
user_mcu_i2c_read(unsigned char addr
                   , unsigned char reg
                   , unsigned char * data
                   , char len)
{
  if (user_pca9545_write(0x02) == 0) {
    char ret = user_i2c_reg_read(addr, reg, data, len, SENSOR_I2C_BUS);
    int sign = (int) *data;
    if ( sign < 0 ) {
      *data = 0;
    }
    return ret;
  }
  return -1;
}


char
user_mcu_get_temp(unsigned char addr
                 , unsigned char reg
                 , unsigned char * v)
{
  if (user_mcu_i2c_read(addr, reg, v, 1) == 0
      && *v != 0xFF
      && *v != 0xFE) {
    return 0;
  } else {
    return -1;
  }
}



