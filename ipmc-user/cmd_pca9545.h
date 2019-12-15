#ifndef CMD_PCA9545_H
#define CMD_PCA9545_H

int
write_i2c_mux(unsigned char * params,
              unsigned char * reply,
              const int conn_idx);

int
read_i2c_mux(unsigned char * params,
             unsigned char * reply,
             const int conn_idx);

#endif // CMD_PCA9545_H
