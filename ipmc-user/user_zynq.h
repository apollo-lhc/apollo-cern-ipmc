#ifndef USER_ZYNQ_H
#define USER_ZYNQ_H

/* check if zynq is active */
char
zynq_read_version(unsigned char * version);

char
zynq_reset(void);

#endif /* USER_ZYNQ_H */
