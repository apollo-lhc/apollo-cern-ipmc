#ifndef GENERIC_I2C_H
#define GENERIC_I2C_H

/* 
 * For simple i2c devices that don't use the restart signal:
 * i2c_r i2c_addr
 * i2c_w i2c_addr data
 * For ones that require multiple bytes/restart for register access:
 * i2c_r i2c_addr reg_addr
 * i2c_w i2c_addr reg_addr data
 */

/* Write to I2C slave without registers */
char
i2c_read(const unsigned char i2c_addr,
         unsigned char * data,
         const char len);

/* Read from I2C slave without registers */
char
i2c_write(const unsigned char i2c_addr,
          const unsigned char data[],
          const unsigned char len);

/* Write to I2C slave targeting an initial register */
char
i2c_reg_read(const unsigned char i2c_addr,
             const unsigned char reg_addr,
             unsigned char * data,
             const char len);

/* Read from I2C slave targeting an initial register */
char
i2c_reg_write(const unsigned char i2c_addr,
              const unsigned char reg_addr,
              const unsigned char data[],
              const char len);

#endif /* GENERIC_I2C_H */
