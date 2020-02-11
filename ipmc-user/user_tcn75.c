/***********************************************************************
Nom ......... : user_pca9545.c
Role ........ : TCN75A driver
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 30/05/2019
***********************************************************************/

// this source code header
#include <user_tcn75.h>

// their headers
// #include <hal/irq.h>
// #include <hal/i2c.h>
// #include <i2c_dev.h>
#include <hal/time.h>
#include <debug.h>

// our headers
#include <user_i2c.h>
#include <user_pca9545.h>

// #ifndef HAS_MASTERONLY_I2C
// #error Enable master-only I2C support to use TCN75 sensors.
// #endif

/* I2C address of the TCN75A I2C switch in the I2C sensor bus */
// #define TCN75A_U34_I2C_ADDR MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x90)
// #define TCN75A_U35_I2C_ADDR MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x92) 
// #define TCN75A_U36_I2C_ADDR MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x94) 

#define TCN75A_U34_I2C_ADDR (0x90 >> 1)
#define TCN75A_U35_I2C_ADDR (0x92 >> 1) 
#define TCN75A_U36_I2C_ADDR (0x94 >> 1) 

#define I2C_TRIAL_DELAY 20000

// static const unsigned char DEBUG = 0;

/* ------------------------------------------------------------------ */
// helper

static char inline
get_tcn_addr(char id
             , short int * addr) {
  if (id == 34) {
    *addr = TCN75A_U34_I2C_ADDR;
    return 0;
  }
  if (id == 35) {
    *addr = TCN75A_U35_I2C_ADDR;
    return 0;
  }
  if (id == 36) {
    *addr = TCN75A_U36_I2C_ADDR;
    return 0;
  }
  return -1;
}


static char inline
tcn75_i2c_write(unsigned char addr
                , unsigned char reg
                , unsigned char * data
                , char len) {
  if ( user_pca9545_write(0x01) ) {
    return user_i2c_reg_write(addr, reg, data, len,
                              SENSOR_I2C_BUS);
  }
  return -1;
}

static char inline
tcn75_i2c_read(unsigned char addr
               , unsigned char reg
               , unsigned char * data
               , char len) {
  if ( user_pca9545_write(0x01) == 0 ) {
    return user_i2c_reg_read(addr, reg, data, len,
                             SENSOR_I2C_BUS);
  }
  return -1;
}

/* ------------------------------------------------------------------ */

/* Write the TCN75 configuration register */
char inline
user_tcn75_write_conf(unsigned char id
                      , unsigned char conf)
{
  short int addr;
  if (get_tcn_addr(id, &addr)) {
    return -1;
  }
  return tcn75_i2c_write(addr, TCN75_CFG_REG, &conf, 1);
}

/* Write a TCN75 temperature register (temp, Tos, or Thyst) */
char inline
user_tcn75_write_reg(unsigned char id
                     , unsigned char reg
                     , char temp)
{
  short int addr;
  if (get_tcn_addr(id, &addr)) {
    return -1;
  }

  unsigned char data[2] = { temp, 0 };
  return tcn75_i2c_write(addr, reg, data, 2);
}

/* Read a TCN75 temperature register (temp, Tos, or Thyst) */
char inline
user_tcn75_read_reg(unsigned char id
                    , unsigned char reg
                    , unsigned char *temp)
{

  static unsigned char previous_temp;

  short int addr;
  if (get_tcn_addr(id, &addr)) {
    return -1;
  }
  
  unsigned char aux[2];
  if (tcn75_i2c_read(addr, reg, aux, 2)) {
    /* the sensor does not respond: set unreal temperature */
    *temp = previous_temp;
    return -1;
  }
  
  previous_temp = *temp = aux[0];
  // if (DEBUG) {
  //   debug_printf("tcn75 @%03x, temp = %02x\n", addr, aux[0]);
  // }
  return 0;
}


