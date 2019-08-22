#ifndef USER_GPIO_H
#define USER_GPIO_H

typedef
enum {
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
} sm_signal_t;


void
user_gpio_init(void);

int
user_get_n_pins(void);

int
user_get_signal_index(const unsigned char * sm_signal_name);

const unsigned char *
user_get_signal_sm_name(sm_signal_t sm_signal);

const int
user_get_signal_expert_mode(sm_signal_t sm_signal);

int
user_is_expert_constrained(sm_signal_t sm_signal);

int
user_set_gpio(sm_signal_t sm_signal, int level);

int
user_unprotected_set_gpio(sm_signal_t sm_signal, int level);

int
user_get_gpio_state(sm_signal_t sm_signal);

int
user_enable_expert_mode(void);

int
user_disable_expert_mode(void);

int
user_is_expert_mode_on(void);

// pin_map_t
// get_gpio_signal(sm_signal_t sm_signal);

#endif // USER_GPIO_H
