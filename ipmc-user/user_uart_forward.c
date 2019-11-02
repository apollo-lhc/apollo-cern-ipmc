#include <user_uart_forward.h>

#include <user_gpio.h>

char
user_uart_forward_get(void)
{
  char re = 0;
  re |= (user_get_gpio(uart_addr0) << 0);
  re |= (user_get_gpio(uart_addr1) << 1);
  return re;
}

void
user_uart_forward_set(char dest)
{
  user_set_gpio(uart_addr0, dest & 0x01);
  user_set_gpio(uart_addr1, dest & 0x02);
  return;
}
