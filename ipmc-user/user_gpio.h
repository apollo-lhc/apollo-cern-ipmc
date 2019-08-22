#ifndef USER_GPIO_H
#define USER_GPIO_H

enum signals {
              ipmc_zynq_en,
              en_one_jtag_chain,
              uart_addr0,
              uart_addr1,
              zynq_boot_mode0,
              zynq_boot_mode1,
              sense_rst_n,
              mezz2_en,
              mezz1_en,
              m24512_we_n,
              eth_sw_pwr_good,
              eth_sw_reset_n,
              en_12v,
              fp_latch,
              blue_led,
              payload_reset_n,
              startup_flag
};


void
gpio_init(void);

int
get_n_pins(void);

// int
// get_signal_index(const char * sm_signal_name);

const char *
get_signal_sm_name(int idx);

const int
get_signal_expert_mode(int idx);

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

// pin_map_t
// get_gpio_signal(int idx);

#endif // USER_GPIO_H
