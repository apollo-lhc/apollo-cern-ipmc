/***********************************************************************
Nom ......... : user_pca9545.c
Role ........ : PCA9545 driver
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 30/05/2019
***********************************************************************/

#include <user_pca9545.h>

// #include <hal/i2c.h>
// #include <app/serial.h>

// #include <i2c_dev.h>

#include <debug.h>

#include <user_i2c.h>
#include <user_gpio.h>

/* I2C address of the PCA9545A I2C switch in the I2C sensor bus */
// #define PCA9545_I2C_ADDR   MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x70 << 1) 
#define PCA9545_I2C_ADDR   0x70 

// static const char i2c_debug_write_str[] =
//   "+++++ write PCA9545 0x%03x, data: 0x%02x, stats: %d.\n";
// 
// static const char i2c_debug_read_str[] =
//   "+++++ read PCA9545 0x%03x, data: 0x%02x, stats: %d.\n";

// static const char DEBUG = 0;

char
user_pca9545_write(unsigned char mask)
{
  char ret = user_i2c_write(PCA9545_I2C_ADDR, &mask, 1, SENSOR_I2C_BUS);
  
  // if (DEBUG) {
  //   debug_printf(i2c_debug_write_str, PCA9545_I2C_ADDR, mask, ret);
  // }
  
  return ret;
}

char
user_pca9545_read(unsigned char * mask)
{

  char ret = user_i2c_read(PCA9545_I2C_ADDR, mask, 1, SENSOR_I2C_BUS);
  
  // if (DEBUG) {
  //   debug_printf(i2c_debug_read_str,
  //                PCA9545_I2C_ADDR,
  //                *mask,
  //                ret);
  // }
  return ret;
}


