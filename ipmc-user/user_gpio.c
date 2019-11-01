/***********************************************************************
Nom ......... : user_gpio.c
Role ........ : GPIO resources for the L0MDT project
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 18/05/2019
***********************************************************************/

#include <user_gpio.h>

// their headers
#include <app.h>
#include <cfgint.h>
#include <ipmc.h>
#include <log.h>
#include <debug.h>
#include <app/signal.h>
#include <app/master/fru_led.h>

// our headers
#include <user_helpers.h>

#define NAME(Variable) (#Variable)
#define PIN(n,o,e,s,d,i) [n]={(unsigned char *) NAME(n), o, e, s, d, i}

/* ================================================================ */


typedef struct pin_map_n {
  const unsigned char * sm_name;
  const char output;
  const char expert;
  const signal_t ipmc_name;
  const char initial;
  const char inverted;
} pin_map_t;

static pin_map_t
pin_map[] = {
    PIN(ipmc_zynq_en      , 1, 1, USER_IO_3                 , 0, 1) // 0
    , PIN(en_one_jtag_chain , 1, 0, USER_IO_4                 , 0, 1) // 1
    , PIN(uart_addr0        , 1, 0, USER_IO_5                 , 1, 1) // 2
    , PIN(uart_addr1        , 1, 0, USER_IO_6                 , 1, 1) // 3
    , PIN(zynq_boot_mode0   , 1, 1, USER_IO_7                 , 1, 1) // 4
    , PIN(zynq_boot_mode1   , 1, 1, USER_IO_8                 , 1, 1) // 5
    , PIN(sense_rst_n       , 1, 0, USER_IO_9                 , 1, 1) // 6
    , PIN(mezz2_en          , 1, 0, USER_IO_10                , 0, 1) // 7
    , PIN(mezz1_en          , 1, 0, USER_IO_11                , 0, 1) // 8
    , PIN(m24512_we_n       , 1, 0, USER_IO_12                , 1, 1) // 9
    , PIN(eth_sw_pwr_good   , 0, 0, USER_IO_13                , 0, 1) // 10
    , PIN(eth_sw_reset_n    , 1, 1, USER_IO_16                , 1, 1) // 11
    , PIN(en_12v            , 1, 1, CFG_PAYLOAD_DCDC_EN_SIGNAL, 0, 0) // 12
    , PIN(fp_latch          , 0, 0, CFG_HANDLE_SWITCH_SIGNAL  , 0, 1) // 13
    , PIN(blue_led          , 1, 1, CFG_BLUE_LED_SIGNAL       , 1, 0) // 14
    , PIN(payload_reset_n   , 0, 1, CFG_PAYLOAD_RESET_SIGNAL  , 1, 1) // 15
    , PIN(shelf_operation   , 1, 1, USER_IO_17                , 1, 1) // 16
    , PIN(zynq_i2c_on       , 1, 1, USER_IO_18                , 0, 1) // 17
    , PIN(debug_0           , 1, 1, USER_IO_19                , 0, 1) // 18
    , PIN(debug_1           , 1, 1, USER_IO_20                , 0, 1) // 19
    , PIN(debug_2           , 1, 1, USER_IO_21                , 0, 1) // 20
    , PIN(ps_delay_0        , 1, 1, IPM_IO_0                  , 0, 1) // 21
    , PIN(ps_delay_1        , 1, 1, IPM_IO_1                  , 0, 1) // 22
    , PIN(ps_delay_2        , 1, 1, IPM_IO_2                  , 0, 1) // 23
    , PIN(ps_delay_3        , 1, 1, IPM_IO_3                  , 0, 1) // 24
    , PIN(ps_delay_4        , 1, 1, IPM_IO_4                  , 0, 1) // 25
    , PIN(ps_delay_5        , 1, 1, IPM_IO_5                  , 0, 1) // 26
    , PIN(ps_delay_6        , 1, 1, IPM_IO_6                  , 0, 1) // 27
    , PIN(cm_off_req        , 1, 1, IPM_IO_10                 , 0, 1) // 28
    , PIN(cm_off_res        , 1, 1, IPM_IO_11                 , 0, 1) // 29
};

/* ================================================================ */

static int expert_mode = 0;

static const int N_PINS = sizeof(pin_map) / sizeof(pin_map[0]);

/* ================================================================ */

int
user_get_n_pins(void)
{
  return N_PINS;
}

const unsigned char *
user_get_signal_sm_name(sm_signal_t sm_signal)
{
  return pin_map[sm_signal].sm_name;
}

const int
user_get_signal_expert_mode(sm_signal_t sm_signal)
{
  return pin_map[sm_signal].expert;
}

// look for signal information in the pin map table and return its
// position. -1 is returned in case no signal is found.
int
user_get_signal_index(const unsigned char * sm_signal_name)
{
  int i = 0;
  for (i = 0; i < N_PINS; i++) {
    if (str_eq(pin_map[i].sm_name, sm_signal_name) == 1) {
      return i;
    }
  }
  return -1;
}

int
user_unprotected_set_gpio(sm_signal_t sm_signal, int level)
{
  // Some signals use inverted logic for some reason...
  if (1 == pin_map[sm_signal].inverted){
    level = !level;
  }
  
  if (1 == level) {
    signal_activate(&pin_map[sm_signal].ipmc_name);
    return 0;
  }
  
  if (0 == level) {
    signal_deactivate(&pin_map[sm_signal].ipmc_name);
    return 0;
  }
  
  return -3;
}

int
user_set_gpio(sm_signal_t sm_signal, int level)
{
  if (pin_map[sm_signal].expert == 1 && expert_mode == 0) {
    return -1;
  }

  if (pin_map[sm_signal].output == 0 && expert_mode == 0) {
    // pin is input
    return -2;
  }

  return user_unprotected_set_gpio(sm_signal, level);
}

// read pin and fill reply string with associated value. returns the
// size of the reply.
int
user_get_gpio(sm_signal_t sm_signal)
{
  int value = signal_read(&pin_map[sm_signal].ipmc_name);

  // Some signals use inverted logic for some reason...
  if (1 == pin_map[sm_signal].inverted){
    return !value;
  }

  return value;
}


int
user_is_expert_constrained(sm_signal_t sm_signal)
{
  return pin_map[sm_signal].expert;
}

int
user_enable_expert_mode(void){
  expert_mode = 1;
  return 0;
}

int
user_disable_expert_mode(void)
{
  expert_mode = 0;
  return 0;
}


int
user_is_expert_mode_on(void)
{
  return expert_mode;
}

pin_map_t
user_get_gpio_signal(sm_signal_t sm_signal)
{
  return pin_map[sm_signal];
}

void
user_gpio_init(void)
{
  int i;
  for (i = 0; i < N_PINS; i++) {
    if(pin_map[i].output) {
      user_unprotected_set_gpio(i, pin_map[i].initial);
    }
  }
  return;
}

void
user_dump_gpios(void)
{
  int i;
  for (i = 0; i < N_PINS; i++) {
    debug_printf("%s = %d\n",
                 pin_map[i].sm_name,
                 user_get_gpio(i));
  }
  debug_printf("---------\n");
}


