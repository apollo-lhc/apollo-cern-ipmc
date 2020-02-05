#ifndef USER_UC_H
#define USER_UC_H

char
user_uc_i2c_write(unsigned char addr
                  , unsigned char reg
                  , unsigned char * data
                  , char len);

char
user_uc_i2c_read(unsigned char addr
                 , unsigned char reg
                 , unsigned char * data
                 , char len);

char
user_uc_get_temp(unsigned char addr
                 , unsigned char reg
                 , unsigned char * v);

#endif /* USER_UC_H */
