/***********************************************************************
Nom ......... : user_i2c.c
Role ........ : generic i2c driver
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 2019-07-31
***********************************************************************/

#include <user_i2c.h>

#include <hal/i2c.h>

#include <i2c_dev.h>

#include <debug.h>

/* sensor I2C bus number */
#define SENSOR_I2C_BUS     2

static const char i2c_debug_str[] =
  "+++++ I2C @ 0x%03hhx (@ 0x%03hhx), stats: %d.\n";

/* Write to I2C slave without registers */
char
i2c_read(unsigned char i2c_addr,
         unsigned char * data,
         const char len,
         char i2c_bus)
{
  i2c_addr <<= 1;
  i2c_addr |= 1;
  short int full_addr = MO_CHANNEL_ADDRESS(i2c_bus, i2c_addr);
  
  int ret = i2c_dev_read(full_addr, data, len);
  debug_printf(i2c_debug_str, full_addr, i2c_addr, ret);

  int i;
  for (i=0; i < len; i++) {
    debug_printf("+++++ data[%d]: %x\n", i, data[i]);
  }
  
  return (ret < I2C_OK) ? (-1) : 0;
}

/* Read from I2C slave without registers */
char
i2c_write(unsigned char i2c_addr,
          const unsigned char data[],
          const unsigned char len,
          char i2c_bus)
{
  i2c_addr <<= 1; 
  short int full_addr = MO_CHANNEL_ADDRESS(i2c_bus, i2c_addr);
  
  int ret = i2c_io(full_addr | I2C_START | I2C_STOP,
                   (unsigned char *) data,
                   len);
  debug_printf(i2c_debug_str, full_addr, i2c_addr, ret);
  
  int i;
  for (i=0; i < len; i++) {
    debug_printf("+++++ data[%d]: %x\n", i, data[i]);
  }

  return (ret < I2C_OK) ? (-1) : 0;
}

/* Write to I2C slave targeting an initial register */
char
i2c_reg_read(unsigned char i2c_addr,
             const unsigned char reg_addr,
             unsigned char * data,
             const char len,
             char i2c_bus)
{
  i2c_addr <<= 1;
  // i2c_addr |= 1;
  short int full_addr = MO_CHANNEL_ADDRESS(i2c_bus, i2c_addr);
  char ret = i2c_dev_read_reg(full_addr,
                              reg_addr,
                              data,
                              len);
  return (ret < I2C_OK) ? (-1) : 0;
}

/* Read from I2C slave targeting an initial register */
char
i2c_reg_write(unsigned char i2c_addr,
              const unsigned char reg_addr,
              const unsigned char data[],
              const char len,
              char i2c_bus)
{
  i2c_addr <<= 1;
  short int full_addr = MO_CHANNEL_ADDRESS(i2c_bus, i2c_addr);
  char ret = i2c_dev_write_reg(full_addr,
                               reg_addr,
                               (unsigned char *) data,
                               len);

  return (ret < I2C_OK) ? (-1) : 0;
}
