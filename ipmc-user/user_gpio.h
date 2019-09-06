#ifndef USER_GPIO_H
#define USER_GPIO_H

typedef
enum {
      ipmc_zynq_en       // 0
      , en_one_jtag_chain  // 1
      , uart_addr0         // 2
      , uart_addr1         // 3
      , zynq_boot_mode0    // 4
      , zynq_boot_mode1    // 5
      , sense_rst_n        // 6
      , mezz2_en           // 7
      , mezz1_en           // 8
      , m24512_we_n        // 9
      , eth_sw_pwr_good    // 10
      , eth_sw_reset_n     // 11
      , en_12v             // 12
      , fp_latch           // 13
      , blue_led           // 14
      , payload_reset_n    // 15
      , shelf_operation    // 16
      , zynq_i2c_on        // 17
      , shutdown_req       // 18
      // , red_led            // 19
      // , amber_led          // 20
      // , orange_led         // 21
      // , white_led          // 22
      // , green_led           // 23
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
user_get_gpio(sm_signal_t sm_signal);

int
user_enable_expert_mode(void);

int
user_disable_expert_mode(void);

int
user_is_expert_mode_on(void);

void
user_dump_gpios(void);

// pin_map_t
// get_gpio_signal(sm_signal_t sm_signal);

#endif // USER_GPIO_H
