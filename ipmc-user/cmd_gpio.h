#ifndef CMD_GPIO_H
#define CMD_GPIO_H

#include <user_gpio.h>

int
set_gpio(unsigned char * params,
         unsigned char * reply,
         int conn_idx);

int
get_gpio(unsigned char * params,
         unsigned char * reply,
         int conn_idx);

#endif // CMD_GPIO_H
