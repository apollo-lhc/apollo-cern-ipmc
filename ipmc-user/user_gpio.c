/***********************************************************************
Nom ......... : user_gpio.c
Role ........ : GPIO resources for the L0MDT project
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 18/05/2019
***********************************************************************/

#include <app.h>
#include <cfgint.h>
#include <ipmc.h>
#include <log.h>
#include <debug.h>
#include <app/signal.h>

#include <user_helpers.h>

#include <user_gpio.h>

#define NAME(Variable) (#Variable)
#define PIN(n,o,e,s,d) [n]={NAME(n), o, e, s, d}

/* ================================================================ */


typedef struct pin_map_n {
  const char * sm_name;
  const int output;
  const int expert;
  const signal_t ipmc_name;
  const int initial;
} pin_map_t;

static pin_map_t
pin_map[] = {
             PIN(ipmc_zynq_en     , 1, 1, USER_IO_3                 , 0),
             PIN(en_one_jtag_chain, 1, 0, USER_IO_4                 , 0), 
             PIN(uart_addr0       , 1, 0, USER_IO_5                 , 1), 
             PIN(uart_addr1       , 1, 0, USER_IO_6                 , 0), 
             PIN(zynq_boot_mode0  , 1, 1, USER_IO_7                 , 1), 
             PIN(zynq_boot_mode1  , 1, 1, USER_IO_8                 , 1), 
             PIN(sense_rst_n      , 1, 0, USER_IO_9                 , 1), 
             PIN(mezz2_en         , 1, 0, USER_IO_10                , 0), 
             PIN(mezz1_en         , 1, 0, USER_IO_11                , 0), 
             PIN(m24512_we_n      , 1, 0, USER_IO_12                , 1), 
             PIN(eth_sw_pwr_good  , 0, 0, USER_IO_13                , 0), 
             PIN(eth_sw_reset_n   , 1, 1, USER_IO_16                , 1),
             PIN(en_12v           , 1, 1, CFG_PAYLOAD_DCDC_EN_SIGNAL, 0),
             PIN(fp_latch         , 0, 0, CFG_HANDLE_SWITCH_SIGNAL  , 0),
             PIN(blue_led         , 1, 1, CFG_BLUE_LED_SIGNAL       , 1),
             PIN(payload_reset_n  , 0, 1, CFG_PAYLOAD_RESET_SIGNAL  , 1),
             PIN(startup_flag     , 1, 1, USER_IO_14                , 0)
};

/* ================================================================ */

static int expert_mode = 0;

static const int N_PINS = sizeof(pin_map) / sizeof(pin_map[0]);

/* ================================================================ */

int
get_n_pins(void)
{
  return N_PINS;
}

const char *
get_signal_sm_name(int idx)
{
  return pin_map[idx].sm_name;
}

const int
get_signal_expert_mode(int idx)
{
  return pin_map[idx].expert;
}

// look for signal information in the pin map table and return its
// position. -1 is returned in case no signal is found.
int
get_signal_index(const char * sm_signal_name)
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
unprotected_activate_gpio(int idx)
{
  if (pin_map[idx].output == 0) {
    // pin is input
    return -2;
  }
    
  signal_t userio_sig = pin_map[idx].ipmc_name;
  if (1 == str_eq(pin_map[idx].sm_name, "en_12v")) {
    signal_activate(&userio_sig);
  } else {
    // reversed for some reason
    signal_deactivate(&userio_sig);
  }
  
  return 0;
}

int
activate_gpio(int idx)
{
  if (pin_map[idx].expert == 1 && expert_mode == 0) {
    return -1;
  }
  return unprotected_activate_gpio(idx);
}

int
unprotected_deactivate_gpio(int idx)
{
  if (pin_map[idx].output == 0) {
    // pin is input
    return -2;
  }
    
  signal_t userio_sig = pin_map[idx].ipmc_name;
  if (1 == str_eq(pin_map[idx].sm_name, "en_12v")) {
    signal_deactivate(&userio_sig);
  } else {
    // reversed for some reason
    signal_activate(&userio_sig);
  }
  
  return 0;
}

int
deactivate_gpio(int idx)
{
  if (pin_map[idx].expert == 1 && expert_mode == 0) {
    return -1;
  }
  return unprotected_deactivate_gpio(idx);
}

// read pin and fill reply string with associated value. returns the
// size of the reply.
int
get_gpio_state(int idx)
{
  signal_t userio_sig = pin_map[idx].ipmc_name;

  int value = signal_read(&userio_sig);

  if(1 == str_eq(pin_map[idx].sm_name, "en_12v")){
    return value;
  }
  else {
    return !value;
  }
}


int
is_expert_constrained(int idx)
{
  return pin_map[idx].expert;
}

int
enable_expert_mode(void){
  expert_mode = 1;
  return 0;
}

int
disable_expert_mode(void)
{
  expert_mode = 0;
  return 0;
}


int
is_expert_mode_on(void)
{
  return expert_mode;
}

pin_map_t
get_gpio_signal(int idx)
{
  return pin_map[idx];
}

void
gpio_init(void)
{
  int i;
  for (i = 0; i < N_PINS; i++) {
    if (pin_map[i].initial == 1) {
      unprotected_activate_gpio(i);
    } else if (pin_map[i].initial == 0) {
       unprotected_deactivate_gpio(i);
    }
 }
  return;
}
