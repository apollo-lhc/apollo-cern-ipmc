#include <user_cmd.h>

#include <net/ip.h>
#include <debug.h>

#include <user_helpers.h>

#include <cmd_eeprom.h>
#include <cmd_expert.h>
#include <cmd_gpio.h>
#include <cmd_i2c.h>
#include <cmd_pca9545.h>
#include <cmd_tcn75.h>
#include <cmd_uart.h>
#include <cmd_version.h>
#include <cmd_zynq.h>
#include <cmd_pim400kz.h>

#define NAME(Variable) (#Variable)
#define CMD_FUNC(n) {(unsigned char *) NAME(n), n}
#define ALIAS_FUNC(a,n) {a, n}

cmd_buf_t cmd_buf[MAX_USER_TCPSERV_CLIENT];

int
help(unsigned char * params,
     unsigned char * reply,
     const int conn_idx);

int
signals (unsigned char * params,
         unsigned char * reply,
         const int conn_idx);

const cmd_map_t cmd_map[] = {
    CMD_FUNC(expert_mode                ),
    CMD_FUNC(get_gpio                   ),
    CMD_FUNC(get_i2c_bus                ),
    CMD_FUNC(set_i2c_bus                ),
    CMD_FUNC(i2c_reg_r                  ),
    CMD_FUNC(i2c_reg_w                  ),
    CMD_FUNC(i2c_r                      ),
    CMD_FUNC(i2c_w                      ),
    CMD_FUNC(read_i2c_mux               ),
    CMD_FUNC(read_tcn75a                ),
    CMD_FUNC(set_gpio                   ),
    CMD_FUNC(eeprom_wren                ),
    CMD_FUNC(eeprom_read                ),
    CMD_FUNC(eeprom_save                ),
    CMD_FUNC(eeprom_get_version         ),
    CMD_FUNC(eeprom_set_version         ),
    CMD_FUNC(eeprom_get_sn              ),
    CMD_FUNC(eeprom_set_sn              ),
    CMD_FUNC(eeprom_get_rn              ),
    CMD_FUNC(eeprom_set_rn              ),
    CMD_FUNC(eeprom_get_mac_addr        ),
    CMD_FUNC(eeprom_set_mac_addr        ),
    CMD_FUNC(pim400kz_get_af_voltage    ),
    CMD_FUNC(pim400kz_get_bf_voltage    ),
    CMD_FUNC(pim400kz_get_current       ),
    CMD_FUNC(pim400kz_get_holdup_voltage),
    CMD_FUNC(pim400kz_get_status        ),
    CMD_FUNC(pim400kz_get_temp          ),
    CMD_FUNC(pim400kz_set_status        ), 
    CMD_FUNC(signals                    ),
    CMD_FUNC(uart_forward               ),
    CMD_FUNC(write_i2c_mux              ),
    CMD_FUNC(version                    ),
    CMD_FUNC(zynq_restart               ),
    CMD_FUNC(zynq_i2c_r                 ),
    CMD_FUNC(zynq_i2c_w                 ),
    CMD_FUNC(help                       ),
    ALIAS_FUNC(question_mark_str, help)
};

// bugged if list of commands is empty.
static const int N_COMMANDS = sizeof(cmd_map) / sizeof(cmd_map[0]);

/* ================================================================ */

// convert string to lowercase
void
lowercase(unsigned char s[])
{
  int c = 0;
  
  while (s[c] != '\0') {
    if (s[c] >= 'A' && s[c] <= 'Z') {
      s[c] = s[c] + 32;
    }
    c++;
  }
  return;
}


void
remove_extra_spaces(unsigned char s[])
{
  unsigned char *p, *q;


  // p initially should point to the beginning of the string
  p = s;

  // let's eliminate space chars in the beginning of the command.
  // finding the first valid char (non space). It stops if end of
  // string is reached.
  while (*p == ' ' && *p != '\0') {
    p++;
  }

  // if string starts with spaces, remove them by mean of shifting the
  // whole vector.
  if (p != s) {
    q = s;
    while (*p != '\0') {
      *q = *p;
      p++;
      q++;
    }
    *q = '\0';
  }

  // now that we ensured that we have valid chars in the beginning of
  // the string, it is time to remove all the duplicated spaces

  // p will remember the last valid char, starting from beginning of
  // the string
  p = s;

  // q will point to the next char, which should be assessed
  q = s+1;

  // while the end of the string is not seen, let's sweep the target
  // string eliminating duplicate spaces
  while (*p != '\0' && *q != '\0') {

    // if last valid char is a space and current char is also a space,
    // then ignore it
    if (*p == ' ' && *q == ' ') {
      q++;
    }

    // otherwise, if it is not a sequence of spaces, it is a valid
    // character; which means that we need to copy the current char
    // and move target to next position. NB: overwriting a char with
    // its own content in the case that p and q are adjacent to each
    // other is faster than checking if value should be copied.
    else {
      p++;
      *p = *q;
      q++;
    }
    
  }

  // finish string
  *(p+1) = '\0';
  
  return;
}


// char *
// strcpy(char *str_dest,
//        const char *str_src)
// {
//     assert(str_dest != NULL && str_src != NULL);
//     char *temp = str_dest;
//     while((*str_dest++ = *str_src++) != '\0');
//     return temp;
// }

/* ================================================================ */

// look for command information in the command map table and return
// its position. -1 is returned in case no command is found.
int
get_cmd_index(const unsigned char * cmd)
{
  int i = 0;
  for (i = 0; i < N_COMMANDS; i++) {
    // debug_printf("\n<><><><> sizeof(cmd_map[%d].cmd): %d %d"
    //              , i, sizeof(cmd_map[i].cmd)
    //              , strlenu(cmd_map[i].cmd));
    if (str_eq(cmd_map[i].cmd, cmd) == 1) {
      return i;
    }
  }
  return -1;
}

// copy the first word from line (until delimiter) to param, removing
// it from the content of the line.
int
get_next_param(unsigned char * param,
               unsigned char * line,
               char delim)
{

  unsigned char * target = param;
  unsigned char * l = line;

  while (*l != delim && *l != '\0') {
    *target = *l;
    target++;
    l++;
  }
  *target = '\0';

  // stop if no param was available
  if (target == param) {
    return 1;
  }

  // shift remaining characters to the beginning of the command line
  target = line;
  if (*l != '\0') {  
    l++;
    while (*l != '\0') {
      *target = *l;
      target++;
      l++;
    }
  }
  *target = '\0';

  return 0;
}

// append received data to associated buffer
// check for overflow
int
append_to_cmd_buffer(const int conn_idx,
                     const unsigned char * data,
                     unsigned short len)
{

  // debug_printf("\nxxxxxxxxxxx");

  static unsigned char tmp[CMD_LINE_MAX_LEN];
  memcpy(tmp, data, len);
  tmp[len] = '\0';
  // debug_printf("\nxxxxxxxxxxx data: %s [%d]", tmp, len);
    
  int l = cmd_buf[conn_idx].len;
  if (l + len >= CMD_LINE_MAX_LEN) {
    cmd_buf[conn_idx].len = 0; 
    cmd_buf[conn_idx].data[0] = '\0'; 
    return -1;
  }

  cmd_buf[conn_idx].len += strcpyl(&(cmd_buf[conn_idx].data[l]), tmp);  
  
  // debug_printf("\nuuuuuuuuuuuuu cmd_buf[%d].data: %s [%d]"
  //              , conn_idx
  //              , cmd_buf[conn_idx].data
  //              , cmd_buf[conn_idx].len);
  
  return 0;
}


// look for termination
// copy from buffer to cmd line;
// terminates cmd line with '\0' for str_eq
// shift remaining of the buffer to the start
// returns the len of the command found, or -1 otherwise
int
chomp_cmd(unsigned char * cmd_line,
          const int conn_idx)
{
  int i, l;

  // debug_printf("<_> ~~~~ chomp starting\n");
  // debug_printf("<_> ~~~~ cmd_idx: %d\n", conn_idx);
  // debug_printf("<_> ~~~~ cmd_len: %d\n", cmd_buf[conn_idx].len);

  for (i = 0; i < cmd_buf[conn_idx].len; i++) {
    
    if (cmd_buf[conn_idx].data[i] == '\n'
        || cmd_buf[conn_idx].data[i] == '\r'){
      break;
    }
  }

  // no terminator?
  if (i == cmd_buf[conn_idx].len) {
    return -1;
  }

  // debug_printf("<_> ~~~~ cmd_buff[i]: %c\n", cmd_buf[conn_idx].data[i]);
  
  
  // copy command to be processed
  memcpy(cmd_line, cmd_buf[conn_idx].data, i);
  cmd_line[i] = '\0';
  memcpy(cmd_buf[conn_idx].spare, cmd_buf[conn_idx].data, i);
  cmd_buf[conn_idx].spare[i] = '\0';
  
  // debug_printf("<_> ~~~~ terminated: %s\n", cmd_line);
      

  //remove terminator(s)
  l = i; // save length for later
  while (cmd_buf[conn_idx].data[i] == '\r'
         || cmd_buf[conn_idx].data[i] == '\n') {
    i++;
  }
  cmd_buf[conn_idx].len -= i;

  memcpy(cmd_buf[conn_idx].data
         , &(cmd_buf[conn_idx].data[i])
         , cmd_buf[conn_idx].len);
  
  return l;
  
  // unsigned char * p = &(cmd_buf[conn_idx].data[i]);
  // while (*p == '\r' || *p == '\n') {
  //   p++;
  //   cmd_buf[conn_idx].len--;
  // }
  // 
  // cmd_buf[conn_idx].len = m
  // 
  //   // shift data to beginning of buffer
  //   unsigned char * q = cmd_buf[conn_idx].data;
  // cmd_buf[conn_idx].len -= i;
  // int k;
  // for (k = 0; k < cmd_buf[conn_idx].len; k++){
  //   *q = *p;
  //   p++;
  //   q++;
  // };
  // 
  // // return len of the command
  // return i;
  //   }
  // }

  // // no terminator found -> no command
  // return -1;
}

int
help (unsigned char * params
      , unsigned char * reply
      , const int conn_idx)
{
  // debug_printf("..... help 0\n");

  static const unsigned char cmds_header_str[] =
    "Available commands:";
  static const unsigned char help_footer_str[] =
    "\nTry also \"<command> help\".";
  
  unsigned char * r = reply;
  // int len;

  int i;

  // debug_printf("..... help 1\n");

  r += strcpyl(r, cmds_header_str);

  // debug_printf("..... help 2\n");

  for (i = 0; i < N_COMMANDS; i++) {
    r += strcpyl(r, (unsigned char *) "\n  ");
    r += strcpyl(r, cmd_map[i].cmd);
  }

  // debug_printf("..... help 5\n");

  strcpyl(r, help_footer_str);

  // debug_printf("..... help 6\n");

  return strlenu(reply);  
}

int
signals (unsigned char * params
         , unsigned char * reply
         , const int conn_idx)
{
  // debug_printf("..... help 0\n");

  static const unsigned char signals_header_str[] =
    "Available signals (marked if expert mode required):";

  unsigned char * r = reply;
  // int len;

  int i;

  r += strcpyl(r, signals_header_str);

  for (i = 0; i < user_get_n_pins(); i++) {
    r += strcpyl(r, (unsigned char *) "\n  ");
    r += strcpyl(r, user_get_signal_sm_name(i));
    if (user_get_signal_expert_mode(i) == 1) {
      r += strcpyl(r, expert_label_str);
    }
  }

  // debug_printf("..... help 6\n");

  return strlenu(reply);
  
}


unsigned char *
get_cmd_spare(const int conn_idx)
{
  return cmd_buf[conn_idx].spare;
}
