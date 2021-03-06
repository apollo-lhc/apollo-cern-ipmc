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
      , debug_0            // 18
      , debug_1            // 19
      , debug_2            // 20
      , ps_delay_0         // 21
      , ps_delay_1         // 22
      , ps_delay_2         // 23
      , ps_delay_3         // 24
      , ps_delay_4         // 25
      , ps_delay_5         // 26
      , ps_delay_6         // 27
      , cm_off_req         // 28
      , cm_off_res         // 29
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

void
user_dump_gpios(void);

// pin_map_t
// get_gpio_signal(sm_signal_t sm_signal);

#endif // USER_GPIO_H
