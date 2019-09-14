/***********************************************************************
Nom ......... : user_pca9545.c
Role ........ : TCN75A driver
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 30/05/2019
***********************************************************************/

// this source code header
#include <user_tcn75.h>

// their headers
#include <hal/irq.h>
#include <hal/i2c.h>
#include <i2c_dev.h>

// our headers
#include <user_pca9545.h>

// #ifndef HAS_MASTERONLY_I2C
// #error Enable master-only I2C support to use TCN75 sensors.
// #endif

/* sensor I2C bus number */
#define SENSOR_I2C_BUS     2

/* I2C address of the TCN75A I2C switch in the I2C sensor bus */
#define TCN75A_U34_I2C_ADDR MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x90)
#define TCN75A_U35_I2C_ADDR MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x92) 
#define TCN75A_U36_I2C_ADDR MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x94) 

/* ------------------------------------------------------------------ */

/* Write the TCN75 configuration register */
char inline
user_tcn75_write_conf(unsigned char id
                      , unsigned char conf)
{

  unsigned short addr;
  if (34 == id) {
    addr = TCN75A_U34_I2C_ADDR;
  } else if (35 == id) {
    addr = TCN75A_U35_I2C_ADDR;
  } else if (36 == id){
    addr = TCN75A_U36_I2C_ADDR;
  } else {
    return -1;
  }

  unsigned char prev;  
  if (user_pca9545_read(&prev) != 0) {
    return -2;
  }
  
  if (user_pca9545_write(0x01) != 0) {
    return -2;
  }

  char ret1 = i2c_dev_write_reg(addr, TCN75_CFG_REG, &conf, 1);
  char ret2 = user_pca9545_write(prev);
  
  return ret1 | ret2;
}

/* Write a TCN75 temperature register (temp, Tos, or Thyst) */
char inline
user_tcn75_write_reg(unsigned char id
                     , unsigned char reg
                     , char temp)
{
  unsigned short addr;
  if (34 == id) {
    addr = TCN75A_U34_I2C_ADDR;
  } else if (35 == id) {
    addr = TCN75A_U35_I2C_ADDR;
  } else if (36 == id){
    addr = TCN75A_U36_I2C_ADDR;
  } else {
    return -1;
  }


  unsigned char prev;
  if (user_pca9545_read(&prev) != 0) {
    return -2;
  }
  
  if (user_pca9545_write(0x01) != 0) {
    return -2;
  }

  unsigned char data[2] = { temp, 0 };
  char ret1 = i2c_dev_write_reg(addr, reg, data, 2);
  char ret2 = user_pca9545_write(prev);

  return ret1|ret2;
}

/* Read a TCN75 temperature register (temp, Tos, or Thyst) */
char inline
user_tcn75_read_reg(unsigned char id
                    , unsigned char reg
                    , unsigned char *temp)
{

  unsigned short addr;
  if (34 == id) {
    addr = TCN75A_U34_I2C_ADDR;
  } else if (35 == id) {
    addr = TCN75A_U35_I2C_ADDR;
  } else if (36 == id){
    addr = TCN75A_U36_I2C_ADDR;
  } else {
    return -1;
  }

  unsigned char prev;
  if (user_pca9545_read(&prev) != 0) {
    return -2;
  }
  
  if (user_pca9545_write(0x01) != 0) {
    return -2;
  }
  
  unsigned char aux[2];
  char ret1 = i2c_dev_read_reg(addr, reg, aux, 2);
  if (ret1) {
    /* the sensor does not respond: set unreal temperature */
    *temp = -99;
  }
  
  char ret2 = user_pca9545_write(prev);

  if (ret1|ret2) {
    return ret1|ret2;
  }

  *temp = aux[0];
  
#ifdef DEBUG
  debug_printf("tcn75 @%02x, temp = %02x \n ", addr >> 8, addr & 0xFF, data[0]);
#endif
  
  return 0;
}
