#ifndef USER_ZYNQ_H
#define USER_ZYNQ_H

/* check if zynq is active */
char
user_zynq_read_version(unsigned char * version);

char
user_zynq_request_restart(char delay);

char
user_zynq_i2c_write(unsigned char addr
                    ,unsigned char reg
                    , int * data
                    , char len);

char
user_zynq_i2c_read(unsigned char addr
                   ,unsigned char reg
                   , int * data
                   , char len);

#endif /* USER_ZYNQ_H */
