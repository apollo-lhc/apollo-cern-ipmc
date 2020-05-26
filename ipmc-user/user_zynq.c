/***********************************************************************
Nom ......... : user_zynq.c
Role ........ : ZYNQ driver
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 30/05/2019
***********************************************************************/

#include <user_zynq.h>

// their headers
// #include <hal/i2c.h>
// #include <i2c_dev.h>
#include <hal/time.h>
#include <debug.h>
#include <net/ip.h>
#include <net/eth.h>
#include <app/master/app.h>

// our headers
#include <user_i2c.h>
#include <user_gpio.h>
#include <user_pca9545.h>
#include <user_eeprom.h>
// #include <user_power_sequence.h>


// /* I2C address of the ZYNQ I2C switch in the I2C sensor bus */
// #define ZYNQ_I2C_ADDR   MO_CHANNEL_ADDRESS(SENSOR_I2C_BUS, 0x30 << 1) 


static uint8_t zynq_restart_delay = 5;
static uint8_t zynq_restart = 0;
static uint8_t eeprom2zynq = 0;
// static uint8_t DEBUG = 0;

/* ------------------------------------------------------------------ */
// helper


char
user_zynq_i2c_write(unsigned char addr
                    , unsigned char reg
                    , unsigned char * data
                    , char len)
{

  if (user_get_gpio(ipmc_zynq_en) == 0) {
    return -1;
  }

  if ( user_pca9545_write(0x08) == 0 ) {
    return user_i2c_reg_write(addr, reg, data, len,
                              SENSOR_I2C_BUS);
  }
  return -1;
}



char
user_zynq_i2c_read(unsigned char addr
                   , unsigned char reg
                   , unsigned char * data
                   , char len)
{
  if (user_get_gpio(ipmc_zynq_en) == 0) {
    return -1;
  }

  if ( user_pca9545_write(0x08) == 0 ) {
    return user_i2c_reg_read(addr, reg, data, len,
                             SENSOR_I2C_BUS);
  }
  return -1;
}

/* ------------------------------------------------------------------ */


char
user_zynq_read_version(unsigned char * version)
{
  // temporary code for testing down to here
  // -----------------------------------------

  // if (pca9545_write(0x08) != 0) {
  //   // not possible to set I2C mux
  //   return -2;
  // }
  // char ret = i2c_dev_read_reg(ZYNQ_I2C_ADDR, 0x00, version, 1);
  
  // return (ret < I2C_OK) ? (-1) : 0;
  return -1;
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
    // debug_printf("ZYNQ_POWER_OFF");

    addr0 = user_get_gpio(uart_addr0);
    addr1 = user_get_gpio(uart_addr1);

    user_unprotected_set_gpio(uart_addr0, 1);
    user_unprotected_set_gpio(uart_addr1, 0);

    user_unprotected_set_gpio(ipmc_zynq_en, 0);

    restart_delay = zynq_restart_delay;
    state = DELAY;
    break;
  case DELAY:
    // debug_printf("zynq reset DELAY");
    if ( restart_delay > 0) {
      restart_delay--;
    } else {
      user_unprotected_set_gpio(ipmc_zynq_en, 1);
      state = ZYNQ_POWER_ON_ACK;
    }
    break;
  case ZYNQ_POWER_ON_ACK:
    // debug_printf("zynq reset POWERON_ACK");
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

char
user_zynq_get_temp(unsigned char addr
                   , unsigned char * v)
{
  if (user_zynq_i2c_read(addr, 0, v, 1) == 0
      && *v != 0) {
    return 0;
  } else {
    return -1;
  }
}

// char
// user_zynq_get_temp(unsigned char

char
user_zynq_i2c_write_on_boot(void)
{
  static uint8_t mac_addr[6];
  char ret;

  // data from eeprom
  ret = user_eeprom_get_mac_addr(0, mac_addr);
  if (ret != 0) {
    return -1;
  }
  user_zynq_i2c_write(0x60, 16, mac_addr, 6);

  ret = user_eeprom_get_mac_addr(1, mac_addr);
  if (ret != 0) {
    return -2;
  }
  user_zynq_i2c_write(0x60, 24, mac_addr, 6);

  uint8_t sn;
  ret = user_eeprom_get_serial_number(&sn);
  if (ret != 0) {
    return -3;
  }
  user_zynq_i2c_write(0x60, 2, &sn, 1);

  uint8_t rn;
  ret = user_eeprom_get_revision_number(&rn);
  if (ret != 0) {
    return -4;
  }
  user_zynq_i2c_write(0x60, 3, &rn, 1);

  // local info
  user_zynq_i2c_write(0x60, 4, &app_hardware_address, 1);
  
  eeprom2zynq = 1;
  return 0;
}

TIMER_CALLBACK(1s, zynq_i2c_update_ipmc_info_timercback_1s)
{
  static ip_addr_t ip_addr;
  static mac_addr_t mac_addr;

  eth_get_mac(0, mac_addr);
  user_zynq_i2c_write(0x67, 0, mac_addr, sizeof(mac_addr_t));

  ip_get_ip(0, ip_addr);
  user_zynq_i2c_write(0x67, 8, ip_addr, sizeof(ip_addr_t));

  ip_get_ip(0, ip_addr);
  user_zynq_i2c_write(0x67, 8, ip_addr, sizeof(ip_addr_t));
  
  return;
}


// This is a function to coordenate the initialization of the Zynq and
// the power negotiation with the shelf manager.
TIMER_CALLBACK(100ms, user_zynq_i2c_on_timercback_100ms)
{
  
  // runs each 500ms only
  static int cnt = 0;
  if (++cnt % 5){
    return;
  }
  
  if (user_get_gpio(ipmc_zynq_en) == 0) {
    // zynq is off, no way zynq i2c is functional...
    user_unprotected_set_gpio(zynq_i2c_on, 0);
    eeprom2zynq = 0;
    return;
  }
  
  unsigned char v = 0;
  if (user_zynq_i2c_read(0x60, 0, &v, 1) == 0
      && (v & 0x1) == 0x1 ) {
    // reading success and zynq boot is over
    user_unprotected_set_gpio(zynq_i2c_on, 1);
    if (eeprom2zynq == 0) {
      user_zynq_i2c_write_on_boot();
    }
  } else {
    // reading error or zynq boot is not over
    user_unprotected_set_gpio(zynq_i2c_on, 0);
  }

  return;
}


// This is a function to coordenate the CM power off
// during power blade power-off sequence.
TIMER_CALLBACK(1s, user_zynq_cm_off_timercback_100ms)
{
  
  if (user_get_gpio(ipmc_zynq_en) == 0) {
    // zynq disabled, request can not be answered
    // just a guard
    user_unprotected_set_gpio(cm_off_res, 0);
    return;
  }

  unsigned char v = 0x1 << 4;
  if (user_get_gpio(cm_off_req) == 1) {
    // there is a shutdown request, tells zynq!
    if (user_zynq_i2c_read(0x60, 0, &v, 1) == 1) {
      return;
    }
    v |= 0x1 << 4;
    user_zynq_i2c_write(0x60, 0, &v, 1);
  } else {
    // ipmc_zynq_en == 1 and cm_off_req == 0
    // normal operation
    // no request, no response needed
    user_unprotected_set_gpio(cm_off_res, 0);
    return;
  }
  
  // status of the cm_off_res should change only when reading works
  if (user_zynq_i2c_read(0x60, 0, &v, 1) == 1) {
    return;
  }
    
  if ((v & (0x1<<5)) != 0 ) {
    // reading success and zynq boot is over
    user_unprotected_set_gpio(cm_off_res, 1);
  } else {
    // reading error or cm is not off yet
    user_unprotected_set_gpio(cm_off_res, 0);
  }

  return;
}


// This is a function to coordenate the initialization of the Zynq and
// the power negotiation with the shelf manager.
TIMER_CALLBACK(1s, user_zynq_restart_timercback_1s)
{
  if (zynq_restart == 1) {
    user_zynq_restart();
  }

  // if (DEBUG) {
  // 
  //   static const unsigned char addr = 0x61;
  //   static unsigned char reg = 0;
  //   static unsigned char reg_prev;
  //   static unsigned char data = 0;
  //   unsigned char data_prev;
  // 
  //   debug_printf("zynq_i2c_on: %d\n", user_get_gpio(zynq_i2c_on));
  // 
  //   if (user_zynq_i2c_write(addr, reg, &data, 1)) {
  //     debug_printf("zynq_i2c_write(0x%02x)(%d)(%d) failed.\n", addr, reg, data);
  //   }
  // 
  //   data++;
  //   reg_prev = reg;
  //   reg = (reg < 9) ? reg + 1 : 0;
  //   
  //   if (user_zynq_i2c_read(addr, reg_prev, &data_prev, 1)){
  //     debug_printf("zynq_i2c_read(0x%02x)(%d) failed.\n", addr, reg_prev);
  //   } else {
  //     debug_printf("zynq_i2c_read(0x%02x)(%d): %d\n", addr, reg_prev, data_prev);
  //   }
  // }
  
  return;
}


