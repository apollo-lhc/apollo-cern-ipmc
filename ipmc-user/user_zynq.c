/***********************************************************************
Nom ......... : user_zynq.c
Role ........ : ZYNQ driver
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 30/05/2019
***********************************************************************/

#include <user_zynq.h>

// their headers
#include <hal/i2c.h>
#include <i2c_dev.h>

// our headers
#include <user_gpio.h>
#include <user_pca9545.h>

/* sensor I2C bus number */
#define SENSOR_I2C_BUS     2

// /* I2C address of the ZYNQ I2C switch in the I2C sensor bus */
// #define ZYNQ_I2C_ADDR   MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x30 << 1) 

// temp address just for testing
#define TCN75A_U34_I2C_ADDR MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x90) 

char
zynq_read_version(unsigned char * version)
{

 // temporary for testing
 if (pca9545_write(0x01) != 0) return -2;
 char ret = i2c_dev_read_reg(TCN75A_U34_I2C_ADDR, 0x00, version, 1);

 // if (pca9545_write(0x08) != 0) {
 //   // not possible to set I2C mux
 //   return -2;
 // }
 // char ret = i2c_dev_read_reg(ZYNQ_I2C_ADDR, 0x00, version, 1);
 return (ret < I2C_OK) ? (-1) : 0;
}

char
zynq_reset(void)
{
  // Make sure UART ADDR [1:0] is "01"
  unprotected_deactivate_gpio(uart_addr1);
  unprotected_activate_gpio(uart_addr0);

  // Shutdow Zynq supplies
  unprotected_deactivate_gpio(ipmc_zynq_en);

  // Zynq is tired, let it rest
  udelay(1000 * 1000);
  
  // Start Zynq power up sequence
  unprotected_deactivate_gpio(ipmc_zynq_en);

  // Make sure UART ADDR [1:0] is "00"
  unprotected_deactivate_gpio(uart_addr1);
  unprotected_deactivate_gpio(uart_addr0);

  return 0;
}
