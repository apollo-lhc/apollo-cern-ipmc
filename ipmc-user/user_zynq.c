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

// Zynq was not killed... yet.
static char resurrect_zynq = 0;

char
user_zynq_read_version(unsigned char * version)
{

 // temporary for testing
 if (user_pca9545_write(0x01) != 0) return -2;
 char ret = i2c_dev_read_reg(TCN75A_U34_I2C_ADDR, 0x00, version, 1);

 // if (pca9545_write(0x08) != 0) {
 //   // not possible to set I2C mux
 //   return -2;
 // }
 // char ret = i2c_dev_read_reg(ZYNQ_I2C_ADDR, 0x00, version, 1);
 return (ret < I2C_OK) ? (-1) : 0;
}

char
user_zynq_reset(char delay)
{
  // Make sure UART ADDR [1:0] is "01"
  user_unprotected_set_gpio(uart_addr1, 0);
  user_unprotected_set_gpio(uart_addr0, 1);

  // kill Zynq
  user_unprotected_set_gpio(ipmc_zynq_en, 0);

  // resurrect Zynq needed
  resurrect_zynq = delay;

  return 0;
}

// people like to kill the poor Zynq sometimes
// let's resurrect it!
TIMER_CALLBACK(1s, user_zynq_timercback)
{

  // Does Zynq need to revive?
  if (resurrect_zynq == 0) {
    return;
  }

  // counting down for Zynq to awake!
  if (--resurrect_zynq) {
    return;
  }

  // Start Zynq power up sequence
  user_unprotected_set_gpio(ipmc_zynq_en, 1);

  // Make sure UART ADDR [1:0] is "00"
  user_unprotected_set_gpio(uart_addr1, 0);
  user_unprotected_set_gpio(uart_addr0, 0);

  return;
}
