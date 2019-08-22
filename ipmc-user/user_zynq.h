#ifndef USER_ZYNQ_H
#define USER_ZYNQ_H

/* check if zynq is active */
char
user_zynq_read_version(unsigned char * version);

char
user_zynq_reset(void);

#endif /* USER_ZYNQ_H */
