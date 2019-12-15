#include <cmd_gpio.h>
#include <cmd_defs.h>

#include <user_cmd.h>
#include <user_helpers.h>

static const unsigned char signal_not_found_str[] =
  "Signal not found.";

int
set_gpio(unsigned char * params,
         unsigned char * reply,
         const int conn_idx)
{
  // debug_printf("<_> ======= write_gpio_signal\n");

  static const unsigned char set_gpio_help_str[] =
    "Usage: set_gpio <signal name> <value>.\n"
    "Value should be 1 (one) to activate the signal.\n"
    "Value should be 0 (zero) to deactivate the signal.";

  static const unsigned char set_gpio_error_str[] =
    "Signal cannot be written.";

  static unsigned char param[MAX_PARAM_LEN];
  get_next_param(param, params, ' ');

  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, set_gpio_help_str);
  }
  
  int idx = user_get_signal_index(param);
  if (idx >= 0) {
    get_next_param(param, params, ' ');
    if (param[0] == '1' || param[0] == 'h'){
      if (user_set_gpio(idx, 1) == 0) {
        return strcpyl(reply, ok_str);
      }
    }
    else if (param[0] == '0' || param[0] == 'l') {
      if (user_set_gpio(idx, 0) == 0) {
        return strcpyl(reply, ok_str);
      }      
    }
    return strcpyl(reply, set_gpio_error_str);
  }
  return strcpyl(reply, signal_not_found_str);
}

// read pin and fill reply string with associated value. returns the
// size of the reply.
int
get_gpio(unsigned char * params,
         unsigned char * reply,
         const int conn_idx)
{

  static const unsigned char get_gpio_help_str[] =
    "Usage: get_gpio <signal name>.\n"
    "Returns 1 (one) if signal is activated.\n"
    "Returns 0 (zero) if signal is deactivated.";

  static const unsigned char gpio_enabled_str[] = "1";
  static const unsigned char gpio_disabled_str[] = "0";

  // debug_printf("<_> ======= read_gpio_signal\n");

  static unsigned char param[MAX_PARAM_LEN];
  get_next_param(param, params, ' ');

  // debug_printf("<_> ======= ");
  // debug_printf(sm_signal_name);
  // debug_printf("\n");

  // debug_printf("######## read_gpio_signal 1\n");


  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, get_gpio_help_str);
  }

  // debug_printf("######## read_gpio_signal 1.3\n");

  int idx = user_get_signal_index(param);
  
  if (idx >= 0) {

    // debug_printf("######## read_gpio_signal 2 (idx found)\n");

    int v = user_get_gpio(idx);
    
    if (v > 0) {
      return strcpyl(reply, gpio_enabled_str);
    } else {
      return strcpyl(reply, gpio_disabled_str);
    }
  }
  return strcpyl(reply, signal_not_found_str);
}

