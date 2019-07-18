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


#ifndef NULL
#define NULL ((void *)0)
#endif

/* ================================================================ */

static pin_map_t pin_map[] = {
  {"ipmc_zynq_en"     , 1, 1, USER_IO_3                 , 0}, 
  {"en_one_jtag_chain", 1, 0, USER_IO_4                 , 0}, 
  {"uart_addr0"       , 1, 0, USER_IO_5                 , 1}, 
  {"uart_addr1"       , 1, 0, USER_IO_6                 , 0}, 
  {"zynq_boot_mode0"  , 1, 1, USER_IO_7                 , 1}, 
  {"zynq_boot_mode1"  , 1, 1, USER_IO_8                 , 1}, 
  {"sense_rst"        , 1, 0, USER_IO_9                 , 0}, 
  {"mezz2_en"         , 1, 0, USER_IO_10                , 0}, 
  {"mezz1_en"         , 1, 0, USER_IO_11                , 0}, 
  {"m24512_we_n"      , 1, 0, USER_IO_12                , 1}, 
  {"eth_sw_pwr_good"  , 0, 0, USER_IO_13                , 0}, 
  {"eth_sw_reset_n"   , 1, 1, USER_IO_16                , 1},
  {"en_12v"           , 1, 1, CFG_PAYLOAD_DCDC_EN_SIGNAL, 0},
  {"fp_latch"         , 0, 0, CFG_HANDLE_SWITCH_SIGNAL  , 0},
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
activate_gpio(int idx)
{
  if ( (pin_map[idx].output == 1 && pin_map[idx].expert == 0)
       || (pin_map[idx].output == 1
           && pin_map[idx].expert == 1 && expert_mode == 1) ) {
    
    signal_t userio_sig = pin_map[idx].ipmc_name;
    
    if(1 == str_eq(pin_map[idx].sm_name, "en_12v")){
      signal_activate(&userio_sig);
    }
    else {
      signal_deactivate(&userio_sig);
    }
    return 0;
  }
  return -1;
}

int
deactivate_gpio(int idx)
{
  if ( (pin_map[idx].output == 1 && pin_map[idx].expert == 0)
       || (pin_map[idx].output == 1
           && pin_map[idx].expert == 1 && expert_mode == 1) ) {
    
    signal_t userio_sig = pin_map[idx].ipmc_name;
    
    if(1 == str_eq(pin_map[idx].sm_name, "en_12v")){
      signal_deactivate(&userio_sig);
    }
    else {
      signal_activate(&userio_sig);
    }
    return 0;
  }
  return -1;
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
    if (pin_map[i].output == 1) {
      if (pin_map[i].initial > 0) {
        activate_gpio(i);
      }
      else {
        deactivate_gpio(i);
      }
    }
  }
  return;
}
