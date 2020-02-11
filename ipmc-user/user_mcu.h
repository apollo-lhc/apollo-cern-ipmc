#ifndef USER_MCU_H
#define USER_MCU_H

char
user_mcu_i2c_write(unsigned char addr
                   , unsigned char reg
                   , unsigned char * data
                   , char len);

char
user_mcu_i2c_read(unsigned char addr
                  , unsigned char reg
                  , unsigned char * data
                  , char len);

char
user_mcu_get_temp(unsigned char addr
                  , unsigned char reg
                  , unsigned char * v);

#endif /* USER_MCU_H */
