/***********************************************************************
Nom ......... : user_pca9545.c
Role ........ : PCA9545 driver
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 30/05/2019
***********************************************************************/

#include <user_pca9545.h>

#include <hal/i2c.h>
// #include <app/serial.h>

#include <i2c_dev.h>

/* sensor I2C bus number */
#define SENSOR_I2C_BUS     2

/* I2C address of the PCA9545A I2C switch in the I2C sensor bus */
#define PCA9545_I2C_ADDR   MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x70 << 1) 

static const char i2c_debug_str[] =
  "+++++ PCA9545 @ 0x%03x, data: 0x%02x, stats: %d.\n";


char
pca9545_write(unsigned char mask)
{

  // char i2c_dev_write_reg(unsigned short addr,
  //                          unsigned char reg,
  //                          unsigned char *pdata,
  //                          unsigned char size);

  // char ret = i2c_dev_write_reg(PCA9545_I2C_ADDR,
  //                              mask,
  //                              NULL,
  //                              0);

  // char ret = i2c_dev_write(PCA9545_I2C_ADDR,
  //                          &mask,
  //                          1);

  
     
  int ret = i2c_io(PCA9545_I2C_ADDR | I2C_START | I2C_STOP, &mask, 1);
  

  // debug_printf(i2c_debug_str, PCA9545_I2C_ADDR, mask, ret);
  return (ret < I2C_OK) ? (-1) : 0;
}

char
pca9545_read(unsigned char * mask)
{

  // char i2c_dev_read(unsigned short addr,
  //                   unsigned char *pdata,
  //                   unsigned char size)

  

  char ret = i2c_dev_read(PCA9545_I2C_ADDR,
                          mask,
                          1);

  *mask = (unsigned char) (*mask + 48); 

  // 
  //    
  // int ret = i2c_io(PCA9545_I2C_ADDR | I2C_START | I2C_STOP, &mask, 1);
  // 

  // debug_printf(i2c_debug_str, PCA9545_I2C_ADDR, mask, ret);
  return (ret < I2C_OK) ? (-1) : 0;
}
