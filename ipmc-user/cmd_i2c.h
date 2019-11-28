#ifndef CMD_I2C_H
#define CMD_I2C_H

#include <user_i2c.h>

int
i2c_w(unsigned char * params,
      unsigned char * reply,
      int conn_idx);

int
i2c_r(unsigned char * params,
      unsigned char * reply,
      int conn_idx);

int
i2c_reg_w(unsigned char * params,
          unsigned char * reply,
          int conn_idx);

int
i2c_reg_r(unsigned char * params,
          unsigned char * reply,
          int conn_idx);

int
set_i2c_bus(unsigned char * params,
            unsigned char * reply,
            int conn_idx);

int
get_i2c_bus(unsigned char * params,
            unsigned char * reply,
            int conn_idx);


#endif // CMD_I2C_H
