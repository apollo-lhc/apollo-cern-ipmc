#ifndef CMD_TCN75_H
#define CMD_TCN75_H

int
read_tcn75a(unsigned char * params,
            unsigned char * reply,
            const int conn_idx);

#endif // CMD_TCN75_H
