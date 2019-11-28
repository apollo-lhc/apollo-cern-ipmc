#include <cmd_uart.h>

#include <user_uart.h>
#include <user_helpers.h>
#include <user_cmd.h>

#include <cmd_defs.h>


int
uart_forward(unsigned char * params,
             unsigned char * reply,
             int conn_idx)
{
  unsigned char param[MAX_PARAM_LEN];
  if (get_next_param(param, params, ' ')) {
    char f = user_uart_forward_get();
    if (0 == f) {
      static unsigned char msg[] = "Zynq";
      return strcpyl(reply, msg);
    }
    if (1 == f) {
      static unsigned char msg[] = "None";
      return strcpyl(reply, msg);
    }
    if (2 == f) {
      static unsigned char msg[] = "Mezzanine 1";
      return strcpyl(reply, msg);
    }
    if (3 == f) {
      static unsigned char msg[] = "Mezzanine 2";
      return strcpyl(reply, msg);
    }
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    static unsigned char msg[] = 
      "Usage: uart_forward [dest].\n"
      "  dest: empty - returns current config;\n"
      "  dest: z - set forward to Zynq;\n"
      "  dest: m1 - set forward to Mezzanine 1;\n"
      "  dest: m2 - set forward to Mezzanine 2;\n"
      "  dest: none - disable forward.";
    return strcpyl(reply, msg);
  }

  if (str_eq(param, (unsigned char *) "z") == 1) {
    user_uart_forward_set(UART_ZYNQ); 
  } else if (str_eq(param, (unsigned char *) "m1") == 1) {
    user_uart_forward_set(UART_MEZZ_1);
  } else if (str_eq(param, (unsigned char *) "m2") == 1) {
    user_uart_forward_set(UART_MEZZ_2);
  } else if (str_eq(param, (unsigned char *) "none") == 1) {
    user_uart_forward_set(UART_NONE);
  } else {
    static unsigned char msg[] = 
      "Error: unknown destination.";
    return strcpyl(reply, msg);
  }
  
  return strcpyl(reply, ok_str);
}


