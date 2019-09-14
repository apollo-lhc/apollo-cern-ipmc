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
#include <debug.h>

// our headers
#include <user_gpio.h>
#include <user_pca9545.h>
// #include <user_power_sequence.h>

/* sensor I2C bus number */
#define SENSOR_I2C_BUS         2

// /* I2C address of the ZYNQ I2C switch in the I2C sensor bus */
// #define ZYNQ_I2C_ADDR   MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x30 << 1) 

// temp address just for testing
#define TCN75A_U34_I2C_ADDR MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x90) 

static char zynq_restart_delay = 5;
static char zynq_restart = 0;

static unsigned char DEBUG = 0;

/* ------------------------------------------------------------------ */
// helper

static char inline
zynq_i2c_write_preserve_mux(short int addr
                            , unsigned char reg
                            , unsigned int * data
                            , char len) {
  unsigned char prev;
  unsigned char i;
  char ret1 = 1;
  char ret2 = 2;
  char ret3 = 4;
  char ret4 = 8;
  unsigned char * p = (unsigned char *) data;
  char l = len * 4;

  for (i = 0; (i < 5
               && (ret1 = user_pca9545_read(&prev))); i++) {
    udelay(20000);
  }
  for (i = 0; (i < 5
               && !ret1
               && (ret2 = user_pca9545_write(0x08))); i++) {
    udelay(20000);
  }
  if (DEBUG) {
    unsigned char aux;
    if (user_pca9545_read(&aux) == 0) {
      debug_printf("Current mux state: 0x%02x\n", aux);
    }
  }
  for (i = 0; (i < 5
               && !ret2
               && (ret3 = i2c_dev_write_reg(addr, reg, p, l))); i++) {
    udelay(20000);
  }
  for (i = 0; (i < 5
               && !ret1
               && (ret4 = user_pca9545_write(prev))); i++) {
    udelay(20000);
  }

  return ret1 | ret2 | ret3 | ret4;
}

static char inline
zynq_i2c_read_preserve_mux(short int addr
                           , unsigned char reg
                           , unsigned int * data
                           , char len) {
  unsigned char prev;
  unsigned char i;
  char ret1 = 1;
  char ret2 = 2;
  char ret3 = 4;
  char ret4 = 8;
  unsigned char * p = (unsigned char *) data;
  char l = len * 4;

  for (i = 0; (i < 5
               && (ret1 = user_pca9545_read(&prev))); i++) {
    udelay(20000);
  }
  for (i = 0; (i < 5
               && !ret1
               && (ret2 = user_pca9545_write(0x08))); i++) {
    udelay(20000);
  }
  if (DEBUG) {
    unsigned char aux;
    if (user_pca9545_read(&aux) == 0) {
      debug_printf("Current mux state: 0x%02x\n", aux);
    }
  }
  for (i = 0; (i < 5
               && !ret2
               && (ret3 = i2c_dev_read_reg(addr, reg, p, l))); i++) {

    udelay(20000);
  }
  for (i = 0; (i < 5
               && !ret1
               && (ret4 = user_pca9545_write(prev))); i++) {
    udelay(20000);
  }

  return ret1 | ret2 | ret3 | ret4;
}

/* ------------------------------------------------------------------ */

char
user_zynq_i2c_write(unsigned char addr
                    ,unsigned char reg
                    , unsigned int * data
                    , char len)
{
  short int faddr = MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, addr << 1); 
  return zynq_i2c_write_preserve_mux(faddr, reg, data, len);
}

char
user_zynq_i2c_read(unsigned char addr
                   ,unsigned char reg
                   , unsigned int * data
                   , char len)
{
  short int faddr = MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, addr << 1); 
  return zynq_i2c_read_preserve_mux(faddr, reg, data, len);
}



char
user_zynq_read_version(unsigned char * version)
{
  
  // -----------------------------------------
  // temporary for testing from this point on 

  unsigned char prev;  
  if (user_pca9545_read(&prev) != 0) {
    return -2;
  }
  
  if (user_pca9545_write(0x01) != 0) {
    return -2;
  }

  char ret = i2c_dev_read_reg(TCN75A_U34_I2C_ADDR, 0x00, version, 1);

  if (user_pca9545_write(prev) != 0) {
    return -4;
  }

  // temporary code for testing down to here
  // -----------------------------------------

  // if (pca9545_write(0x08) != 0) {
  //   // not possible to set I2C mux
  //   return -2;
  // }
  // char ret = i2c_dev_read_reg(ZYNQ_I2C_ADDR, 0x00, version, 1);
  
  return (ret < I2C_OK) ? (-1) : 0;
}

char
user_zynq_request_restart(char delay)
{
  zynq_restart_delay = delay;
  zynq_restart = 1;
  return 0;
}

void
user_zynq_restart(void)
{
  static enum {
               ZYNQ_POWER_OFF,
               DELAY,
               ZYNQ_POWER_ON_ACK
  } state = ZYNQ_POWER_OFF;

  static char addr0;
  static char addr1;

  static char restart_delay;
  
  switch (state) {
  case ZYNQ_POWER_OFF:
    debug_printf("ZYNQ_POWER_OFF");

    addr0 = user_get_gpio(uart_addr0);
    addr1 = user_get_gpio(uart_addr1);

    user_unprotected_set_gpio(uart_addr0, 1);
    user_unprotected_set_gpio(uart_addr1, 0);

    user_unprotected_set_gpio(ipmc_zynq_en, 0);

    restart_delay = zynq_restart_delay;
    state = DELAY;
    break;
  case DELAY:
    debug_printf("zynq reset DELAY");
    if ( restart_delay > 0) {
      restart_delay--;
    } else {
      user_unprotected_set_gpio(ipmc_zynq_en, 1);
      state = ZYNQ_POWER_ON_ACK;
    }
    break;
  case ZYNQ_POWER_ON_ACK:
    debug_printf("zynq reset POWERON_ACK");
    if (user_get_gpio(zynq_i2c_on) == 1) {
      user_unprotected_set_gpio(uart_addr0, addr0);
      user_unprotected_set_gpio(uart_addr1, addr1);

      zynq_restart = 0;
      state = ZYNQ_POWER_OFF;
    }
    break;
  }
  return;
}


// This is a function to coordenate the initialization of the Zynq and
// the power negotiation with the shelf manager.
TIMER_CALLBACK(100ms, user_zynq_timercback_100ms)
{
  // runs each 500ms only
  static int cnt = 0;
  if (++cnt % 5){
    return;
  }

  unsigned char version;
  if (user_zynq_read_version(& version)) {
    // reading error
    user_unprotected_set_gpio(zynq_i2c_on, 0);
  } else {
    // reading success
    user_unprotected_set_gpio(zynq_i2c_on, 1);
  }

  return;
}

// This is a function to coordenate the initialization of the Zynq and
// the power negotiation with the shelf manager.
TIMER_CALLBACK(1s, user_zynq_restart_timercback_1s)
{
  static const unsigned char addr = 0x61;
  static unsigned char reg = 0;
  static unsigned char reg_prev;
  static unsigned int data = 0;
  unsigned int data_prev;

  if (DEBUG) {
    debug_printf("zynq_i2c_on: %d\n", user_get_gpio(zynq_i2c_on));

    if (user_zynq_i2c_write(addr, reg, &data, 1)) {
      debug_printf("zynq_i2c_write(0x%02x)(%d)(%d) failed.\n", addr, reg, data);
    }

    data++;
    reg_prev = reg;
    reg = (reg < 9) ? reg + 1 : 0;
    
    if (user_zynq_i2c_read(addr, reg_prev, &data_prev, 1)){
      debug_printf("zynq_i2c_read(0x%02x)(%d) failed.\n", addr, reg_prev);
    } else {
      debug_printf("zynq_i2c_read(0x%02x)(%d): %d\n", addr, reg_prev, data_prev);
    }
  }
  
  if (zynq_restart == 1) {
    user_zynq_restart();
  }
  return;
}


