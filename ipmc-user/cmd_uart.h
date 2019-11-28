#ifndef CMD_UART_H
#define CMD_UART_H

int
uart_forward(unsigned char * params
             , unsigned char * reply
             , int conn_idx);

#endif // CMD_UART_H
