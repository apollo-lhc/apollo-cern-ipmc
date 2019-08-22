#ifndef USER_GPIO_H
#define USER_GPIO_H

#include <app/signal.h>

typedef struct pin_map_n {
  const char * sm_name;
  const int output;
  const int expert;
  const signal_t ipmc_name;
  const int initial;
} pin_map_t;


void
gpio_init(void);

int
get_n_pins(void);

int
get_signal_index(const char * sm_signal_name);

int
is_expert_constrained(int idx);

int
activate_gpio(int idx);

int
unprotected_activate_gpio(int idx);

int
deactivate_gpio(int idx);

int
unprotected_deactivate_gpio(int idx);

int
get_gpio_state(int idx);

int
enable_expert_mode(void);

int
disable_expert_mode(void);

int
is_expert_mode_on(void);

pin_map_t
get_gpio_signal(int idx);

#endif // USER_GPIO_H
