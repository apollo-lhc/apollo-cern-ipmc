/***********************************************************************
Nom ......... : user_i2c.c
Role ........ : generic i2c driver
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 2019-07-31
***********************************************************************/
#define NEED_MASTERONLY_I2C


#include <user_i2c.h>

// their headers
#include <hal/i2c.h>
#include <i2c_dev.h>
#include <debug.h>

#include <user_gpio.h>

// static const char i2c_debug_reg_str[] =
//   "+++++ \nI2C @0x%03hhx, reg: %02hhx, ret: %hhd, len: %hhd.";
// 
// static const char i2c_debug_str[] =
//   "+++++ \nI2C @0x%03hhx, ret: %hhd, len: %hhd.";

// static const unsigned char DEBUG = 0;

static short int watchdog_counter = 0;


void
user_i2c_sensor_decrement_watchdog_counter (void);

void
user_i2c_sensor_reset_watchdog_counter (void);


/* Write to I2C slave without registers */
char
user_i2c_read(const unsigned char i2c_addr,
              unsigned char data[],
              const char len,
              const i2c_bus_t i2c_bus)
{
  short int full_addr = MO_CHANNEL_ADDRESS(i2c_bus, i2c_addr << 1);

  // int i;
  // for(i = 0; i < len; i++) {
  //   data[i]=21;
  // }
  
  char ret = 0;
  ret = i2c_dev_read(full_addr, data, len);

  // if (DEBUG) {
  //   debug_printf(i2c_debug_str, full_addr, ret, len);
  //   // int i;
  //   for (i=0; i < len; i++) {
  //     debug_printf("+++++ data[%d]: %x\n", i, data[i]);
  //   }
  // }
  
  if (i2c_bus == 2) {
    if (ret == 0 ) {
      user_i2c_sensor_reset_watchdog_counter();
    } else {
      user_i2c_sensor_decrement_watchdog_counter();
    }
  }

  return (ret < I2C_OK) ? (-1) : 0;
}

/* Read from I2C slave without registers */
char
user_i2c_write(const unsigned char i2c_addr,
               const unsigned char data[],
               const unsigned char len,
               const i2c_bus_t i2c_bus)
{
  short int full_addr = MO_CHANNEL_ADDRESS(i2c_bus, i2c_addr << 1);

  unsigned char ret = len;
  ret = monly_i2c_io(full_addr |I2C_WRITE|I2C_START|I2C_STOP,
                     (unsigned char *) data,
                     len);

  if (i2c_bus == 2) {
    if (ret == len ) {
      user_i2c_sensor_reset_watchdog_counter();
    } else {
      user_i2c_sensor_decrement_watchdog_counter();
    }
  }
  
  return (ret < I2C_OK) ? (-1) : 0;
}

/* Write to I2C slave targeting an initial register */
char
user_i2c_reg_read(const unsigned char i2c_addr,
                  const unsigned char reg_addr,
                  unsigned char data[],
                  const char len,
                  const i2c_bus_t i2c_bus)
{

  char ret = 0;

  short int full_addr = MO_CHANNEL_ADDRESS(i2c_bus, i2c_addr << 1);

  // if (DEBUG) {
  //   debug_printf(i2c_debug_reg_str, full_addr, reg_addr, ret, len);
  // }

  // int i;
  // for(i = 0; i < len; i++) {
  //   data[i] = (unsigned char) 21;
  // }
  
  ret = i2c_dev_read_reg(full_addr,
                         reg_addr,
                         data,
                         len);

  // if (DEBUG) {
  //   int i;
  //   for (i=0; i < len; i++) {
  //     int d = data[i];
  //     debug_printf("+++++ data[%d]: %x\n", i, d);
  //   }
  // }

  if (i2c_bus == 2) {
    if (ret == 0 ) {
      user_i2c_sensor_reset_watchdog_counter();
    } else {
      user_i2c_sensor_decrement_watchdog_counter();
    }
  }

  return (ret < I2C_OK) ? (-1) : 0;
}

/* Read from I2C slave targeting an initial register */
char
user_i2c_reg_write(const unsigned char i2c_addr,
                   const unsigned char reg_addr,
                   const unsigned char data[],
                   const char len,
                   const i2c_bus_t i2c_bus)
{
  short int full_addr = MO_CHANNEL_ADDRESS(i2c_bus, i2c_addr << 1);

  char ret = 0;
  ret = i2c_dev_write_reg(full_addr,
                          reg_addr,
                          (unsigned char *) data,
                          len);
  
  if (i2c_bus == 2) {
    if (ret == 0 ) {
      user_i2c_sensor_reset_watchdog_counter();
    } else {
      user_i2c_sensor_decrement_watchdog_counter();
    }
  }

  return (ret < I2C_OK) ? (-1) : 0;
}


void
user_i2c_sensor_decrement_watchdog_counter (void) {
  if (watchdog_counter > 0) {
    watchdog_counter--;
  }  
  return;
}

void
user_i2c_sensor_reset_watchdog_counter (void) {
  watchdog_counter = 256;
  return;
}

// When the number of tries allowed for I2C communication through
// SENSOR_BUS is reached, it is assumed that the channel is stuck and
// the I2C multiplexer need to be reset. Whatdog counter is
// automatically reset when the communication runs fine.
TIMER_CALLBACK(100ms, i2c_sensor_watchdog_timercback)
{
  if (user_get_gpio(sense_rst_n) == 0){
    user_set_gpio(sense_rst_n, 1);
    return;
  }
  
  if (watchdog_counter == 0) {
    user_set_gpio(sense_rst_n, 0);
    return;
  }
}
