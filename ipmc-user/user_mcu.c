/***********************************************************************
Nom ......... : user_uc.c
Role ........ : UC driver
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 30/05/2019
***********************************************************************/

#include <user_uc.h>

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


// /* I2C address of the UC I2C switch in the I2C sensor bus */
// #define UC_I2C_ADDR   MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x30 << 1) 

/* ------------------------------------------------------------------ */
// helper


char
user_uc_i2c_write(unsigned char addr
                    , unsigned char reg
                    , unsigned char * data
                    , char len)
{

  unsigned char i;
  char ret2 = 2;
  char ret3 = 4;

  // configure mux
  for (i = 0;
       (i < 5 && (ret2 = user_pca9545_write(0x02)));
       i++) {
    udelay(20000);
  }

  // write data to uC
  for (i = 0;
       (i < 5
        && !ret2
        && (ret3 = user_i2c_reg_write(addr, reg, data, len, SENSOR_I2C_BUS)));
       i++) {
    udelay(20000);
  }

  return ret2 | ret3;
}



char
user_uc_i2c_read(unsigned char addr
                   , unsigned char reg
                   , unsigned char * data
                   , char len)
{
  unsigned char i;
  char ret2 = 2;
  char ret3 = 4;

  // configure mux
  for (i = 0;
       (i < 5
        && (ret2 = user_pca9545_write(0x02)));
       i++) {
    udelay(20000);
  }

  // read data from uC
  for (i = 0;
       (i < 5
        && !ret2
        && (ret3 = user_i2c_reg_read(addr, reg, data, len, SENSOR_I2C_BUS)));
       i++) {
    udelay(20000);
  }

  return ret2 | ret3;
}


char
user_uc_get_temp(unsigned char addr
                 , unsigned char reg
                 , unsigned char * v)
{
  if (user_uc_i2c_read(addr, reg, v, 1) == 0
      && *v != 0xFF
      && *v != 0xFE) {
    return 0;
  } else {
    return -1;
  }
}



