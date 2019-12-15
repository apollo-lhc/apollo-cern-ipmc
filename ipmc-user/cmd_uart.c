#include <cmd_uart.h>

#include <user_uart.h>
#include <user_helpers.h>
#include <user_cmd.h>

#include <cmd_defs.h>

#define NAME(Variable) ((unsigned char *) #Variable)

int
uart_forward(unsigned char * params,
             unsigned char * reply,
             const int conn_idx)
{
  static unsigned char param[MAX_PARAM_LEN];
  static unsigned char z[] = "Zynq";
  static unsigned char none[] = "None";
  static unsigned char m1[] = "Mezzanine 1";
  static unsigned char m2[] = "Mezzanine 2";
  static unsigned char err_set[] = 
    "Error: unknown destination.";

  if (get_next_param(param, params, ' ')) {
    char f = user_uart_forward_get();
    if (0 == f) {
      return strcpyl(reply, z);
    }
    if (1 == f) {
      return strcpyl(reply, none);
    }
    if (2 == f) {
      return strcpyl(reply, m1);
    }
    if (3 == f) {
      return strcpyl(reply, m2);
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

  if (str_eq(param, (unsigned char *) NAME(z)) == 1) {
    user_uart_forward_set(UART_ZYNQ); 
  } else if (str_eq(param, NAME(m1)) == 1) {
    user_uart_forward_set(UART_MEZZ_1);
  } else if (str_eq(param, NAME(m2)) == 1) {
    user_uart_forward_set(UART_MEZZ_2);
  } else if (str_eq(param, none) == 1) {
    user_uart_forward_set(UART_NONE);
  } else {
    return strcpyl(reply, err_set);
  }
  
  return strcpyl(reply, ok_str);
}


