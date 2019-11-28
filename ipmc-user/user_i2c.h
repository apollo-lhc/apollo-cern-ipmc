#ifndef GENERIC_I2C_H
#define GENERIC_I2C_H


/* sensor I2C bus number */
typedef enum
  {
   INTERNAL_I2C_BUS
   , MNGMNT_I2C_BUS
   , SENSOR_I2C_BUS
  } i2c_bus_t;

/* Write to I2C slave without registers */
char
user_i2c_read(unsigned char i2c_addr,
         unsigned char * data,
         const char len,
         i2c_bus_t i2c_bus);

/* Read from I2C slave without registers */
char
user_i2c_write(unsigned char i2c_addr,
          const unsigned char data[],
          const unsigned char len,
          i2c_bus_t i2c_bus);

/* Write to I2C slave targeting an initial register */
char
user_i2c_reg_read(unsigned char i2c_addr,
             const unsigned char reg_addr,
             unsigned char * data,
             const char len,
             i2c_bus_t i2c_bus);

/* Read from I2C slave targeting an initial register */
char
user_i2c_reg_write(unsigned char i2c_addr,
              const unsigned char reg_addr,
              const unsigned char data[],
              const char len,
              i2c_bus_t i2c_bus);

#endif /* GENERIC_I2C_H */
