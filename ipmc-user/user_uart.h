#ifndef USER_UART_FORWARD_H
#define USER_UART_FORWARD_H

enum UART_FORWARD
  {
   UART_ZYNQ
   , UART_NONE
   , UART_MEZZ_1
   , UART_MEZZ_2
  };

char
user_uart_forward_get(void);

void
user_uart_forward_set(char dest);

#endif /* USER_UART_FORWARD_H */
