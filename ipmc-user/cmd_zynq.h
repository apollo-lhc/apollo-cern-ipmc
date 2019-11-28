#ifndef CMD_ZYNQ_H
#define CMD_ZYNQ_H

int
zynq_restart(unsigned char * params,
           unsigned char * reply,
           int conn_idx);

int
zynq_i2c_w(unsigned char * params,
           unsigned char * reply,
           int conn_idx);

int
zynq_i2c_r(unsigned char * params,
           unsigned char * reply,
           int conn_idx);

#endif // CMD_ZYNQ_H
