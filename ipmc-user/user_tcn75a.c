/***********************************************************************
Nom ......... : user_pca9545.c
Role ........ : TCN75A driver
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 30/05/2019
***********************************************************************/

#include <user_tcn75a.h>

#include <user_helpers.h>


#include <hal/i2c.h>
// #include <app/serial.h>

#include <i2c_dev.h>

#include <user_pca9545.h>


/* sensor I2C bus number */
#define SENSOR_I2C_BUS     2

/* I2C address of the TCN75A I2C switch in the I2C sensor bus */
#define TCN75A_U34_I2C_ADDR MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x90) 
#define TCN75A_U35_I2C_ADDR MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x92) 
#define TCN75A_U36_I2C_ADDR MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x94) 

static const char i2c_debug_str[] =
  "+++++ PCA9545 @ 0x%03x, data: 0x%02x, stats: %d.\n";

char
tcn75a_read(char id, unsigned char * temp)
{

  char ret;

  if(34 == id){
    if (pca9545_write(0x01) != 0) return -3;
    ret = i2c_dev_read_reg(TCN75A_U34_I2C_ADDR, 0x00, temp, 1);
  }
  else if(35 == id){
    if (pca9545_write(0x01) != 0) return -3;
    ret = i2c_dev_read_reg(TCN75A_U35_I2C_ADDR, 0x00, temp, 1);
  }
  else if(36 == id){
    if (pca9545_write(0x01) != 0) return -3;
    ret = i2c_dev_read_reg(TCN75A_U36_I2C_ADDR, 0x00, temp, 1);
  }
  else {
    ret = -2;
  }

  // debug_printf(i2c_debug_str, PCA9545_I2C_ADDR, mask, ret);
  return (ret < I2C_OK) ? (-1) : 0;
}


// ============================================================




