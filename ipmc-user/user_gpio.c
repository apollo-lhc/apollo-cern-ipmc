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
             PIN(ipmc_zynq_en     , 1, 1, USER_IO_3                 , 0, 1),
             PIN(en_one_jtag_chain, 1, 0, USER_IO_4                 , 0, 1), 
             PIN(uart_addr0       , 1, 0, USER_IO_5                 , 1, 1), 
             PIN(uart_addr1       , 1, 0, USER_IO_6                 , 0, 1), 
             PIN(zynq_boot_mode0  , 1, 1, USER_IO_7                 , 1, 1), 
             PIN(zynq_boot_mode1  , 1, 1, USER_IO_8                 , 1, 1), 
             PIN(sense_rst_n      , 1, 0, USER_IO_9                 , 1, 1), 
             PIN(mezz2_en         , 1, 0, USER_IO_10                , 0, 1), 
             PIN(mezz1_en         , 1, 0, USER_IO_11                , 0, 1), 
             PIN(m24512_we_n      , 1, 0, USER_IO_12                , 1, 1), 
             PIN(eth_sw_pwr_good  , 0, 0, USER_IO_13                , 0, 1), 
             PIN(eth_sw_reset_n   , 1, 1, USER_IO_16                , 1, 1),
             PIN(en_12v           , 1, 1, CFG_PAYLOAD_DCDC_EN_SIGNAL, 0, 0),
             PIN(fp_latch         , 0, 0, CFG_HANDLE_SWITCH_SIGNAL  , 0, 1),
             PIN(blue_led         , 1, 1, CFG_BLUE_LED_SIGNAL       , 1, 0),
             PIN(payload_reset_n  , 0, 1, CFG_PAYLOAD_RESET_SIGNAL  , 1, 1),
             PIN(startup_flag     , 1, 1, USER_IO_14                , 0, 1)
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
  if (pin_map[sm_signal].output == 0) {
    // pin is input
    return -2;
  }


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
  return user_unprotected_set_gpio(sm_signal, level);
}

// read pin and fill reply string with associated value. returns the
// size of the reply.
int
user_get_gpio_state(sm_signal_t sm_signal)
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
    user_set_gpio(i, pin_map[i].initial);
  }
  return;
}
