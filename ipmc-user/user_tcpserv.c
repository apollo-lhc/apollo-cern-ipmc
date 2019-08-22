/***********************************************************************
Nom ......... : user_tcpserv.c
Role ........ : TCP/IP server for L0MDT project
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.2 - 2019-07-31
***********************************************************************/

#include <app.h>
#include <cfgint.h>
#include <ipmc.h>
#include <log.h>
#include <debug.h>

#include <net/tcp.h>
#include <net/ip.h>

#include <user_tcpserv.h>

#include <app/signal.h>

#include <user_zynq.h>
#include <user_pca9545.h>
#include <user_tcn75a.h>
#include <user_gpio.h>
#include <user_i2c.h>
#include <user_version.h>

#include <user_helpers.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#define MAX_USER_TCPSERV_CLIENT 10
#define CMD_LINE_MAX_LEN 50
#define MAX_PARAM_LEN 20

// structure to hold status of a connection
typedef struct cmd_buf_n {
  char data[CMD_LINE_MAX_LEN];
  unsigned short len;
  char expert;
  char hex;
  char eol[3];
  char i2c_bus;
} cmd_buf_t;

cmd_buf_t cmd_buf[MAX_USER_TCPSERV_CLIENT] = {{.i2c_bus = 2}};

/* ================================================================ */

void
lowercase(char s[]);

void
remove_extra_spaces(char s[]);

int
get_next_param(char * param, char * line);

int
get_cmd_index(const char * cmd);



int
append_to_cmd_buffer(int conn_idx,
                     const unsigned char * data,
                     unsigned short len);

int
chomp_cmd(char * cmd_line,
          int conn_idx);

/* ================================================================ */

// let's declare strings as global variables so they are not included
// in stack.

static const char connection_request_str[] =
  "<_> user_tcpserv: "
  "New connection request\n";

static const char conn_not_avail_str[] =
  "<W> user_tcpserv: "
  "No user_tcpserv connection slot available\n";

static const char disconn_req_str[] =
  "<_> user_tcpserv: "
  "Disconnect request received\n";

static const char expert_str[] =
  "Expert mode is enabled.\n";

static const char prompt_str[] = ":: ";

static const char help_str[] = "help";

static const char question_mark_str[] = "?";

static const char set_gpio_help_str[] =
  "Usage: set_gpio <signal name> <value>.\n"
  "Value should be 1 (one) to activate the signal.\n"
  "Value should be 0 (zero) to deactivate the signal.\n";

static const char ok_str[] = "OK\n";

static const char set_gpio_error_str[] =
  "Signal cannot be written.\n";

static const char signal_not_found_str[] =
  "Signal not found.\n";

static const char get_gpio_help_str[] =
  "Usage: get_gpio <signal name>.\n"
  "Returns 1 (one) if signal is activated.\n"
  "Returns 0 (zero) if signal is deactivated.\n";

static const char gpio_enabled_str[] = "1\n";
static const char gpio_disabled_str[] = "0\n";
static const char gpio_error_str[] =
  "Unexpected value read from signal.\n";

static const char expert_help_str[] =
  "Usage: expert_mode <value>.\n"
  "Value should be 'on' (no quotes) to allow "
  "overwrite of special signals.\n"
  "Any other value, including empty (no value), "
  "deactivates expert mode.\n";

static const char expert_off_str[] =
  "Expert mode deactivated.\n";

static const char cmds_header_str[] =
  "Available commands:\n";
static const char signals_header_str[] =
  "Available signals (marked if expert mode required):\n";
static const char help_footer_str[] =
  "Try also \"<command> help\".\n";

static const char expert_label_str[] = " (E)";

static const char i2c_mux_write_help_str[] =
  "Usage: write_i2c_mux <mask>\n"
  "mask: 4-bit integer, one for each lane.\n"
  "Ex: \"write_i2c_mux 3\" enables two lanes.\n";

static const char i2c_mux_read_help_str[] =
  "Usage: read_i2c_mux\n";

static const char i2c_mux_error_str[] =
  "I2C mux operation error.\n";

static const char tcn75a_help_str[] =
  "Usage: read_tcn75a <sensor UID>\n"
  "  UID is U34, U35 or U36.\n";

static const char error_str[] =
  "Something has gone wrong.\n";

static const char version_help_str[] =
  "Usage: version\n";

static const char echo_str[] =
  "Command not recognized. Echoing:\n";

static const char help_i2c_write[] =
  "Low level I2C write operation with no target register.\n"
  "Usage: i2c_w <i2c_addr> <data>\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <data> multiple values to be written separated by spaces.\n";

static const char help_i2c_reg_write[] =
  "Low level I2C write operation with target register.\n"
  "Usage: i2c_reg_w <i2c_addr> <reg_addr> <data>\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <reg_addr> is the target register.\n"
  "  <data> multiple values to be written separated by spaces.\n";

static const char err_i2c_addr[] =
  "Invalid I2C address.\n";

static const char err_i2c_data[] =
  "Invalid data.\n";

static const char err_i2c_write_transaction[] =
  "I2C write transaction unsuccessful.\n";

static const char help_i2c_read[] =
  "Low level I2C read operation with no target register.\n"
  "Usage: i2c_r <i2c_addr> [nbytes]\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  [nbytes] number of bytes to be read, 1 if empty.\n";

static const char help_i2c_reg_read[] =
  "Low level I2C read operation with target register.\n"
  "Usage: i2c_reg_r <i2c_addr> <reg_addr> [nbytes]\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <reg_addr> is the target register.\n"
  "  [nbytes] number of bytes to be read, 1 if empty.\n";

static const char err_i2c_read_transaction[] =
  "I2C read transaction unsuccessful.\n";

static const char err_i2c_itoa[] =
  "Conversion from integer to ASCII failed.\n";

static const char err_i2c_len[] =
  "Invalid length.\n";

static const char err_param[] =
  "Unexpected parameters format.\n";

static const char err_i2c_reg_addr[] =
  "Invalid register address.\n";

static const char help_set_i2c_bus[] =
  "Usage: set_i2c_bus <bus ID>\n"
  "  <bus ID>:\n"
  "    M for management bus \n"
  "    S for sensor bus\n";

static const char help_get_i2c_bus[] =
  "Usage: get_i2c_bus\n";

static const char err_i2c_bus[] =
  "Invalid I2C bus ID.\n";

static const char str_i2c_bus_management[] =
  "Management\n";

static const char str_i2c_bus_sensor[] =
  "Sensor\n";

static const char help_zynq_reset[] =
  "Reset only Zynq.\n"
  "Usage: zynq_reset\n";

static const char err_zynq_reset[] =
  "Zynq reset did not happen.\n";


/* ================================================================ */

int
cmd_write_gpio_signal(char * params,
                      unsigned char * reply,
                      int conn_idx);

int
cmd_read_gpio_signal(char * params,
                     unsigned char * reply,
                     int conn_idx);
                     

int
cmd_set_expert_mode(char * params,
                    unsigned char * reply,
                    int conn_idx);

int
cmd_help(char * params,
         unsigned char * reply,
         int conn_idx);

int
cmd_write_i2c_mux(char * params,
                  unsigned char * reply,
                  int conn_idx);

int
cmd_read_i2c_mux(char * params,
                 unsigned char * reply,
                 int conn_idx);

int
cmd_read_tcn75a(char * params,
                unsigned char * reply,
                int conn_idx);

int
cmd_i2c_write(char * params,
              unsigned char * reply,
              int conn_idx);

int
cmd_i2c_read(char * params,
             unsigned char * reply,
             int conn_idx);

int
cmd_i2c_reg_write(char * params,
                  unsigned char * reply,
                  int conn_idx);

int
cmd_i2c_reg_read(char * params,
                 unsigned char * reply,
                 int conn_idx);

int
cmd_version(char * params,
            unsigned char * reply,
            int conn_idx);

int
cmd_set_i2c_bus(char * params,
                unsigned char * reply,
                int conn_idx);

int
cmd_get_i2c_bus(char * params,
                unsigned char * reply,
                int conn_idx);

int
cmd_zynq_reset(char * params,
               unsigned char * reply,
               int conn_idx);

/* ================================================================ */

typedef struct cmd_map_n {
  const char * cmd;
  int (*fnc_ptr)(char *, unsigned char *, int);
} cmd_map_t;

static cmd_map_t cmd_map[] = {
  {"expert_mode"    , cmd_set_expert_mode  },
  {"get_gpio"       , cmd_read_gpio_signal },
  {"get_i2c_bus"    , cmd_get_i2c_bus      },
  {"set_i2c_bus"    , cmd_set_i2c_bus      },
  {"i2c_reg_r"      , cmd_i2c_reg_read     },
  {"i2c_reg_w"      , cmd_i2c_reg_write    },
  {"i2c_r"          , cmd_i2c_read         },
  {"i2c_w"          , cmd_i2c_write        },
  {"read_i2c_mux"   , cmd_read_i2c_mux     },
  {"read_tcn75a"    , cmd_read_tcn75a      },
  {"set_gpio"       , cmd_write_gpio_signal},
  {"write_i2c_mux"  , cmd_write_i2c_mux    },
  {"version"        , cmd_version          },
  {"zynq_reset"     , cmd_zynq_reset       },
  {help_str         , cmd_help             },
  {question_mark_str, cmd_help             }
};

static const int N_COMMANDS = sizeof(cmd_map) / sizeof(cmd_map[0]);

/* ================================================================ */

// // buffer to hold expert mode for each connection
// char buff_expert[MAX_USER_TCPSERV_CLIENT];
// 
// // buffer to hold output mode; 0: integer; 1: hexadecimal
// char buff_hex[MAX_USER_TCPSERV_CLIENT];

/* ================================================================ */

/* Connection slot array used to manage the client */
user_tcpserv_users_t user_tcpserv_clients[MAX_USER_TCPSERV_CLIENT];

/* INIT_CALLBACK: called at initialisation of the IPMC */
INIT_CALLBACK(user_tcpserv_init)
{
  /* Connect TCP server using the following parameters:
     
     - user_tcpserv_connect_handler: callback function called when a
       client sends a connection request
     
     - user_tcpserv_disconnect_handler: callback function called when
       a client is disconnected
     
     - user_tcpserv_data_handler: callback called when data are
       received
     
     - 2004: port number used
  */
  tcp_connect_server(user_tcpserv_connect_handler,
                     user_tcpserv_disconnect_handler,
                     user_tcpserv_data_handler,
                     2345);
}

// /* TIMER_CALLBACK: called every 1s */
// TIMER_CALLBACK(1s, user_tcpserv_timercback)
// {
//   unsigned i;
//   
//   unsigned char *data = (unsigned char *)"USER_TCPSERV_TIMERCBACK \n\r";
//   unsigned len = 26;
// 	
//   /* Scan the connection slot to send data to all of the clients */
//   for(i=0; i < MAX_USER_TCPSERV_CLIENT; i++){
//     
//     /* Check if the slot is in use */
//     if(user_tcpserv_clients[i].opened){
//       
//       /* Send "len" bytes of "data" */
//       tcp_send_packet(user_tcpserv_clients[i].to,
//                       user_tcpserv_clients[i].to_port,
//                       data,
//                       len);
//       
//     }
//     
//   }
// }

/* 
 * Name: user_tcpserv_connect_handler
 *
 * Parameters:
 *		- from: client IP address
 * 
 *              - from_port: client tcp port
 *
 * Description: Called when a client requests a connection. The
 *              prototype of this function is defined by the TCP/IP
 *              library and cannot be changed.
 */
char
user_tcpserv_connect_handler(const ip_addr_t from,
                             unsigned short from_port)
{
  unsigned i;
  
  /* Display information in debug console */
  // debug_printf(connection_request_str);
  
  /* Save client information in a list */

  /* Scan the array to find an empty slot */
  for(i=0; i < MAX_USER_TCPSERV_CLIENT; i++){

    /* Check if current position is already used */
    if(user_tcpserv_clients[i].opened == 0){
      
      user_tcpserv_clients[i].to_port = from_port;

      /* If not, save the client info */
      memcpy(user_tcpserv_clients[i].to, from, sizeof(from));
      
      user_tcpserv_clients[i].opened = 1;
      cmd_buf[i].data[0] = '\0';
      cmd_buf[i].len = 0;


      // didn't work, leaving it out for now.
      //
      // // attempt to send prompt in the beginning of the
      // // communication
      // // tcp_send_packet(user_tcpserv_clients[i].to,
      // //                 user_tcpserv_clients[i].to_port,
      // //                 (unsigned char *) ":: ",
      // //                 3);

      /* Return successfully */
      return 0; 
    }
  }

  /* Print a warning message when no slot is available */
  // debug_printf(conn_not_avail_str);
	
  /* And quit de function with error */
  return 1;
}

/* 
 * Name: user_tcpserv_disconnect_handler
 *
 * Parameters:
 *		- from: client IP address
 *		- from_port: client tcp port
 *
 * Description: Called when a client sends a disconnect request. The
 *              prototype of this function is defined by the TCP/IP
 *              library and cannot be changed.
 */
char
user_tcpserv_disconnect_handler(const ip_addr_t from,
                                unsigned short from_port)
{
  unsigned i;
  
  /* Display information in debug console */
  // debug_printf(disconn_req_str);
  
  /* Search for the client slot in the array */
  for(i=0; i < MAX_USER_TCPSERV_CLIENT; i++){
		
    /* Check the information */
    if(user_tcpserv_clients[i].opened == 1
       && !memcmp(from,
                  user_tcpserv_clients[i].to,
                  sizeof(from))
       && from_port == user_tcpserv_clients[i].to_port){
			
      /* Remove the information and set the opened variable to 0 (slot not used) */
      user_tcpserv_clients[i].to_port = 0;
      memset(user_tcpserv_clients[i].to, 0, 4);
      user_tcpserv_clients[i].opened = 0;
			
    }
  }

  /* Return successfully */
  return 0;
}

/* 
 * Name: user_tcpserv_data_handler
 *
 * Parameters:
 *		- to: server IP address (IPMC)
 *		- to_port: server tcp port (IPMC)
 *		- from: client IP address
 * 		- from_port: client tcp port
 *		- data: data received(Array)
 *		- len: Length in byte of the data array
 * 		- reply: array to push a reply
 *		- replyLen: Length of the reply in byte
 *
 * Description: Called when a client sends data. The prototype of this
 *              function is defined by the TCP/IP library and cannot
 *              be changed.
 */
char
user_tcpserv_data_handler(const ip_addr_t to,
                          unsigned short to_port,
                          const ip_addr_t from,
                          unsigned short from_port,
                          const unsigned char *data,
                          unsigned short len,
                          unsigned char *reply,
                          unsigned *replyLen)
{

  int i;
  int conn_idx = -1;


  // debug_printf("<_> ======= 1\n");

  for(i = 0; i < MAX_USER_TCPSERV_CLIENT; i++){
		
    /* Check the information */
    if(user_tcpserv_clients[i].opened == 1
       && !memcmp(from,
                  user_tcpserv_clients[i].to,
                  sizeof(from))
       && from_port == user_tcpserv_clients[i].to_port){
      conn_idx = i;
    }
  }

  if (conn_idx == -1){
    return -1;
  }

  // debug_printf("<_> ======= 2\n");

  // append received data to associated buffer
  // check for overflow
  if (append_to_cmd_buffer(conn_idx, data, len) != 0){
    return 0;
  }

  char cmd_line[CMD_LINE_MAX_LEN];

  // debug_printf("<_> ======= 3\n");

  // look for termination
  // copy from buffer to cmd line;
  // terminates cmd line with '\0' for str_eq
  // shift remaining of the buffer to the start
  // returns the len of the command found, or -1 otherwise
  int cmd_len = chomp_cmd(cmd_line, conn_idx);
  
  // debug_printf("<_> ======= 4\n");
  
  if (cmd_len < 0) {
    *replyLen = 0;
    return 0;
  }
  
  
  // debug_printf("<_> user_tcpserv command line: %s\n", cmd_line);

  debug_printf("<_> ======= cmd line: %s\n", cmd_line);

  remove_extra_spaces(cmd_line);
  // debug_printf("<_> >>>>>> no extra spaces: %s\n", cmd_line);

  lowercase(cmd_line);
  // debug_printf("<_> >>>>>> all lowercase: %s\n", cmd_line);

  char cmd[MAX_PARAM_LEN];
  get_next_param(cmd, cmd_line);
  debug_printf("<_> >>>>>> cmd: %s\n", cmd);

  int cmd_idx = get_cmd_index(cmd);
  debug_printf("<_> >>>>>> cmd_idx: %d\n", cmd_idx);

  // if a command was found, execute it.
  if (cmd_idx >= 0) { 
    // execute command, get reply and associated length
    *replyLen = cmd_map[cmd_idx].fnc_ptr(cmd_line, reply, conn_idx);
    debug_printf("<_> >>>>>> cmd reply len: %d\n", *replyLen);
  }

  // if a command was not found, echo it.
  else {

    int l = strlen(echo_str);
    memcpy(reply, echo_str, l);

    unsigned char * r = &reply[l];

    /* Clone the request in the reply buffer */
    memcpy(r, data, len);

    /* Set the reply length */
    *replyLen = l+len;

    // finish string
    reply[*replyLen] = '\0';

    // debug_printf("----- echo reply len: %d\n", *replyLen);
  }

  if (is_expert_mode_on() == 1) {
    unsigned char * r = &reply[*replyLen];
    int l = strlen(expert_str);
    memcpy(r, expert_str, l);
    r += l;
    *r = '\0';
    *replyLen += l;
  }

  unsigned char * p = &reply[*replyLen];
  memcpy(p, prompt_str, 3);
  *replyLen += 3;
  reply[*replyLen] = '\0';

  // debug_printf("----- final reply len: %d\n", *replyLen);

  // debug_printf("<_> user_tcpserv response: ");
  // debug_printf((char *) reply);

  /* Return the function successfully */
  return 0;
}

/* ================================================================ */

// convert string to lowercase
void
lowercase(char s[])
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
remove_extra_spaces(char s[])
{
  char *p, *q;


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

// look for connection information in the command buffer and returns
// its position. -1 is returned in case nothing is found.
int
get_cmd_buffer_index(const ip_addr_t from_addr,
                     const unsigned short from_port)
{
  int i = 0;

  for (i = 0; i < MAX_USER_TCPSERV_CLIENT; i++) {
    if (0 == memcmp(&(user_tcpserv_clients[i].to),
                    &from_addr, sizeof(from_addr))){
      if (user_tcpserv_clients[i].to_port == from_port){
        // same ip address and same port considered unique connection.
        return i;
      }
    }
  }
  return -1;
}

// look for command information in the command map table and return
// its position. -1 is returned in case no command is found.
int
get_cmd_index(const char * cmd)
{
  int i = 0;
  for (i = 0; i < N_COMMANDS; i++) {
    if (str_eq(cmd_map[i].cmd, cmd) == 1) {
      return i;
    }
  }
  return -1;
}

// copy the first word from line (until delimiter) to param, removing
// it from the content of the line.
int
get_next_param(char * param,
               char * line)
{

  char * target = param;
  char * l = line;

  while (*l != ' ' && *l != '\0') {
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
append_to_cmd_buffer(int conn_idx,
                     const unsigned char * data,
                     unsigned short len)
{

  // debug_printf("<_> ++++ append starting\n");

  char tmp[50];
  memcpy(tmp, data, len);
  tmp[len] = '\0';
  
  // debug_printf("<_> ++++ len: %d\n", len);
  // debug_printf("<_> ++++ data: %s\n", tmp);

  int l = cmd_buf[conn_idx].len; 
  if (l + len >= CMD_LINE_MAX_LEN) {
    return -1;
  }

  memcpy(&(cmd_buf[conn_idx].data[l]), data, len);
  cmd_buf[conn_idx].len += len;
  cmd_buf[conn_idx].data[cmd_buf[conn_idx].len] = '\0';

  // debug_printf("<_> ++++ buf: %s\n", cmd_buf[conn_idx].data);
  
  return 0;
}


// look for termination
// copy from buffer to cmd line;
// terminates cmd line with '\0' for str_eq
// shift remaining of the buffer to the start
// returns the len of the command found, or -1 otherwise
int
chomp_cmd(char * cmd_line,
              int conn_idx)
{
  int i;

  // debug_printf("<_> ~~~~ chomp starting\n");
  // debug_printf("<_> ~~~~ cmd_idx: %d\n", conn_idx);
  // debug_printf("<_> ~~~~ cmd_len: %d\n", cmd_buf[conn_idx].len);

  for (i = 0; i < cmd_buf[conn_idx].len; i++) {

    // debug_printf("<_> ~~~~ cmd_buff[i]: %c\n", cmd_buf[conn_idx].data[i]);

    if (cmd_buf[conn_idx].data[i] == '\n'
        || cmd_buf[conn_idx].data[i] == '\r'){

      // copy command to be processed
      memcpy(cmd_line, cmd_buf[conn_idx].data, i);
      cmd_line[i] = '\0';

      // debug_printf("<_> ~~~~ terminated: %s\n", cmd_line);
      

      //remove terminator(s)
      char * p = &(cmd_buf[conn_idx].data[i]);
      while (*p == '\r' || *p == '\n') {
        p++;
        cmd_buf[conn_idx].len--;
      }

      // shift data to beginning of buffer
      char * q = cmd_buf[conn_idx].data;
      cmd_buf[conn_idx].len -= i;
      int k;
      for (k = 0; k < cmd_buf[conn_idx].len; k++){
        *q = *p;
        p++;
        q++;
      };

      // return len of the command
      return i;
    }
  }

  // no terminator found -> no command
  return -1;
}



/* ================================================================ */

int
cmd_write_gpio_signal(char * params,
                      unsigned char * reply,
                      int conn_idx)
{
  // debug_printf("<_> ======= write_gpio_signal\n");

  char param[MAX_PARAM_LEN];
  get_next_param(param, params);

  int msg_len;
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    msg_len = strlen(set_gpio_help_str);
    memcpy(reply, set_gpio_help_str, msg_len);
    return msg_len;
  }
  
  int idx = get_signal_index(param);
  if (idx >= 0) {
    get_next_param(param, params);
    if (param[0] == '1' || param[0] == 'h'){
      if (activate_gpio(idx) == 0) {
        msg_len = strlen(ok_str);
        memcpy(reply, ok_str, msg_len);
        return msg_len;
      }
    }
    else if (param[0] == '0' || param[0] == 'l') {
      if (deactivate_gpio(idx) == 0) {
        msg_len = strlen(ok_str);
        memcpy(reply, ok_str, msg_len);
        return msg_len;
      }      
    }
    msg_len = strlen(set_gpio_error_str);
    memcpy(reply, set_gpio_error_str, msg_len);
    return msg_len;
  }
  msg_len = strlen(signal_not_found_str);
  memcpy(reply, signal_not_found_str, msg_len);
  return msg_len;
}

// read pin and fill reply string with associated value. returns the
// size of the reply.
int
cmd_read_gpio_signal(char * params,
                     unsigned char * reply,
                     int conn_idx)
{

  // debug_printf("<_> ======= read_gpio_signal\n");

  char param[MAX_PARAM_LEN];
  get_next_param(param, params);

  // debug_printf("<_> ======= ");
  // debug_printf(sm_signal_name);
  // debug_printf("\n");

  int reply_len = 0;

  // debug_printf("######## read_gpio_signal 1\n");


  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(get_gpio_help_str);
    memcpy(reply, get_gpio_help_str, reply_len);
    return reply_len;
  }

  // debug_printf("######## read_gpio_signal 1.3\n");

  int idx = get_signal_index(param);
  
  if (idx >= 0) {

    // debug_printf("######## read_gpio_signal 2 (idx found)\n");

    int v = get_gpio_state(idx);
    
    if (v > 0) {
      reply_len = strlen(gpio_enabled_str);
      memcpy(reply, gpio_enabled_str, reply_len);
      return reply_len;
    }
    else {
      reply_len = strlen(gpio_disabled_str);
      memcpy(reply, gpio_disabled_str, reply_len);
      return reply_len;
    }
  }
  reply_len = strlen(signal_not_found_str);
  memcpy(reply, signal_not_found_str, reply_len);
  return reply_len;
}

int
cmd_set_expert_mode(char * params,
                    unsigned char * reply,
                    int conn_idx)
{

  int reply_len = 0;
  
  char param[MAX_PARAM_LEN];
  get_next_param(param, params);


  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(expert_help_str);
    memcpy(reply, expert_help_str, reply_len);
    return reply_len;
  }

  
  if (str_eq(param, "on") == 1){
    enable_expert_mode();
    reply_len = 0;
    return reply_len;
  }

  disable_expert_mode();
  reply_len = strlen(expert_off_str);
  memcpy(reply, expert_off_str, reply_len);
  return reply_len;
}

int
cmd_help (char * params,
          unsigned char * reply,
          int conn_idx)
{
  // debug_printf("..... help 0\n");

  unsigned char * r = reply;
  int len;

  int i;
  int map_len;

  // debug_printf("..... help 1\n");

  len = strlen(cmds_header_str);
  memcpy(r, cmds_header_str, len);
  r += len;

  // debug_printf("..... help 2\n");

  map_len = sizeof(cmd_map) / sizeof(cmd_map[0]);
  for (i = 0; i < map_len; i++) {
    memcpy(r, "  ", 2);
    r += 2;
    len = strlen(cmd_map[i].cmd);
    memcpy(r, cmd_map[i].cmd, len);
    r += len;
    *r = '\n';
    r++;
  }

  // debug_printf("..... help 3\n");

  len = strlen(signals_header_str);
  memcpy(r, signals_header_str, len);
  r += len;

  // debug_printf("..... help 4\n");

  for (i = 0; i < get_n_pins(); i++) {
    r += strlcpy(r, "  ");
    r += strlcpy(r, get_signal_sm_name(i));
    if (get_signal_expert_mode(i) == 1) {
      r += strlcpy(r, expert_label_str);
    }
    *r = '\n';
    r++;
  }

  // debug_printf("..... help 5\n");

  r += strlcpy(r, help_footer_str);
  *r = '\0';

  // debug_printf("..... help 6\n");

  return strlen((char *) reply);
  
}


int
cmd_write_i2c_mux(char * params,
                  unsigned char * reply,
                  int conn_idx)
{

  int reply_len = 0;
  
  char param[MAX_PARAM_LEN];
  get_next_param(param, params);


  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(i2c_mux_write_help_str);
    memcpy(reply, i2c_mux_write_help_str, reply_len);
    return reply_len;
  }

  char ret = pca9545_write((unsigned char) *param);

  if(ret == 0){
    reply_len = strlen(ok_str);
    memcpy(reply, ok_str, reply_len);
    return reply_len;
  }
  
  reply_len = strlen(i2c_mux_error_str);
  memcpy(reply, i2c_mux_error_str, reply_len);
  return reply_len;
}


int
cmd_read_i2c_mux(char * params,
                 unsigned char * reply,
                 int conn_idx)
{

  int reply_len = 0;
  
  char param[MAX_PARAM_LEN];
  get_next_param(param, params);


  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(i2c_mux_read_help_str);
    memcpy(reply, i2c_mux_read_help_str, reply_len);
    return reply_len;
  }

  char ret = pca9545_read(reply);

  if(ret == 0){
    reply[1] = '\n';
    return 2;
  }
  
  reply_len = strlen(i2c_mux_error_str);
  memcpy(reply, i2c_mux_error_str, reply_len);
  return reply_len;
}

int
cmd_read_tcn75a(char * params,
                unsigned char * reply,
                int conn_idx)
{
  int reply_len = 0;
  char ret;
  char param[MAX_PARAM_LEN];

  get_next_param(param, params);

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(tcn75a_help_str);
    memcpy(reply, tcn75a_help_str, reply_len);
    return reply_len;
  }

  unsigned char temp;
  ret = tcn75a_read((unsigned char *) param, &temp);
  if (ret != 0) {
    reply_len = strlen(error_str);
    memcpy(reply, error_str, reply_len);
    return reply_len;
  }

  
  ret = a_from_i((char *) reply,
                 temp,
                 0);
  if(ret != 0){
    reply_len = strlen(err_i2c_itoa);
    memcpy(reply, err_i2c_itoa, reply_len);
    return reply_len;
  }

  reply_len = strlen((char *) reply);
  reply[reply_len++]='\n';
  reply[reply_len]='\0';    
  return reply_len;
}

int
cmd_version(char * params,
            unsigned char * reply,
            int conn_idx)
{
  int reply_len = 0;
  
  char param[20];
  get_next_param(param, params);

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(version_help_str);
    memcpy(reply, version_help_str, reply_len);
    return reply_len;
  }

  reply_len = get_version(reply);
  return reply_len;
}


int
cmd_i2c_write(char * params,
              unsigned char * reply,
              int conn_idx)
{
  int reply_len = 0;
  char ret;
  int aux;
  
  char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params);
  if (ret != 0) {
    reply_len = strlen(err_param);
    memcpy(reply, err_param, reply_len);
    return reply_len;  
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(help_i2c_write);
    memcpy(reply, help_i2c_write, reply_len);
    return reply_len;
  }


  // getting I2C address
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    reply_len = strlen(err_i2c_addr);
    memcpy(reply, err_i2c_addr, reply_len);
    return reply_len;  
  }

  char i2c_addr = (char) aux;

  // getting the data
  unsigned char i2c_data[10];
  int i2c_len = 0;
  while (get_next_param(param, params) == 0) {
    ret = i_from_a(&aux,
                   param,
                   &(cmd_buf[conn_idx].hex));
    if (ret != 0) {
      reply_len = strlen(err_i2c_data);
      memcpy(reply, err_i2c_data, reply_len);
      return reply_len;  
    }

    i2c_data[i2c_len] = (char) aux;
    i2c_len++;
  }

  // making sure we have something to send to the target...
  if (i2c_len == 0) {
    reply_len = strlen(err_i2c_data);
    memcpy(reply, err_i2c_data, reply_len);
    return reply_len;  
  }

  // but it can not be too much...
  if (i2c_len > 10) {
    reply_len = strlen(err_i2c_len);
    memcpy(reply, err_i2c_len, reply_len);
    return reply_len;  
  }

  // writing to the I2C address
  ret = i2c_write(i2c_addr,
                  i2c_data,
                  i2c_len,
                  cmd_buf[conn_idx].i2c_bus);
  if(ret != 0){
    reply_len = strlen(err_i2c_write_transaction);
    memcpy(reply, err_i2c_write_transaction, reply_len);
    return reply_len;
  }

  reply_len = strlen(ok_str);
  memcpy(reply, ok_str, reply_len);
  return reply_len;  
}

int
cmd_i2c_read(char * params,
             unsigned char * reply,
             int conn_idx)
{
  int reply_len = 0;
  char ret;
  int aux;  
  
  char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params);
  if (ret != 0) {
    reply_len = strlen(err_param);
    memcpy(reply, err_param, reply_len);
    return reply_len;  
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(help_i2c_read);
    memcpy(reply, help_i2c_read, reply_len);
    return reply_len;
  }

  debug_printf("~~~~~~~~ %s\n", param);
  
  // getting I2C address
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    reply_len = strlen(err_i2c_addr);
    memcpy(reply, err_i2c_addr, reply_len);
    return reply_len;  
  }
  unsigned char i2c_addr = (char) aux;

  // getting the length of the reading; default to 1.
  int i2c_len = 1;
  if (get_next_param(param, params) == 0) {
    // lets fake the hex parsing
    char tmp; 
    ret = i_from_a (&i2c_len,
                    param,
                    &tmp);
    if (ret != 0 || i2c_len > 10) {
      reply_len = strlen(err_i2c_len);
      memcpy(reply, err_i2c_len, reply_len);
      return reply_len;  
    }
  }

  unsigned char i2c_raw_data[10];
  ret = i2c_read(i2c_addr,
                 i2c_raw_data,
                 i2c_len,
                 cmd_buf[conn_idx].i2c_bus);
  if(ret != 0){
    reply_len = strlen(err_i2c_read_transaction);
    memcpy(reply, err_i2c_read_transaction, reply_len);
    return reply_len;
  }


  int i;
  unsigned char i2c_data[10][6];
  for (i = 0; i < i2c_len; i++) {
    ret = a_from_i((char *) &i2c_data[i],
                   i2c_raw_data[i],
                   cmd_buf[conn_idx].hex);
    if(ret != 0){
      reply_len = strlen(err_i2c_itoa);
      memcpy(reply, err_i2c_itoa, reply_len);
      return reply_len;
    }
  }

  unsigned char * p = reply;
  for (i = 0; i < i2c_len; i++) {
    p += strlcpy((char *) p, (const char *) i2c_data[i]);
    *p = ' ';
    p++;
  }
  *(p-1) = '\0';

  return strlen((const char *) reply);
}


int
cmd_i2c_reg_write(char * params,
                  unsigned char * reply,
                  int conn_idx)
{
  int reply_len = 0;
  char ret;
  int aux;
  
  char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params);
  if (ret != 0) {
    reply_len = strlen(err_param);
    memcpy(reply, err_param, reply_len);
    return reply_len;  
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(help_i2c_reg_write);
    memcpy(reply, help_i2c_reg_write, reply_len);
    return reply_len;
  }

  // getting I2C address
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    reply_len = strlen(err_i2c_addr);
    memcpy(reply, err_i2c_addr, reply_len);
    return reply_len;  
  }
  char i2c_addr = (char) aux;

  // getting register address
  ret = get_next_param(param, params);
  if (ret != 0) {
    reply_len = strlen(err_i2c_reg_addr);
    memcpy(reply, err_i2c_reg_addr, reply_len);
    return reply_len;  
  }
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    reply_len = strlen(err_i2c_reg_addr);
    memcpy(reply, err_i2c_reg_addr, reply_len);
    return reply_len;  
  }
  unsigned char reg_addr = (char) aux;

  // getting the data
  unsigned char i2c_data[10];
  int i2c_len = 0;
  while (get_next_param(param, params) == 0) {
    ret = i_from_a(&aux,
                   param,
                   &(cmd_buf[conn_idx].hex));
    if (ret != 0) {
      reply_len = strlen(err_i2c_data);
      memcpy(reply, err_i2c_data, reply_len);
      return reply_len;  
    }

    i2c_data[i2c_len] = (char) aux;
    i2c_len++;
  }

  // making sure we have something to send to the target...
  if (i2c_len == 0) {
    reply_len = strlen(err_i2c_data);
    memcpy(reply, err_i2c_data, reply_len);
    return reply_len;  
  }

  // but it can not be too much...
  if (i2c_len > 10) {
    reply_len = strlen(err_i2c_len);
    memcpy(reply, err_i2c_len, reply_len);
    return reply_len;  
  }

  // writing to the I2C address
  ret = i2c_reg_write(i2c_addr,
                      reg_addr,
                      i2c_data,
                      i2c_len,
                      cmd_buf[conn_idx].i2c_bus);
  if(ret != 0){
    reply_len = strlen(err_i2c_write_transaction);
    memcpy(reply, err_i2c_write_transaction, reply_len);
    return reply_len;
  }

  reply_len = strlen(ok_str);
  memcpy(reply, ok_str, reply_len);
  return reply_len;  
}

int
cmd_i2c_reg_read(char * params,
                 unsigned char * reply,
                 int conn_idx)
{
  int reply_len = 0;
  char ret;
  int aux;  
  
  char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params);
  if (ret != 0) {
    reply_len = strlen(err_param);
    memcpy(reply, err_param, reply_len);
    return reply_len;  
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(help_i2c_reg_read);
    memcpy(reply, help_i2c_reg_read, reply_len);
    return reply_len;
  }

  debug_printf("== %s\n", param);
  
  // getting I2C address
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    reply_len = strlen(err_i2c_addr);
    memcpy(reply, err_i2c_addr, reply_len);
    return reply_len;  
  }
  unsigned char i2c_addr = (char) aux;

  // getting register address
  ret = get_next_param(param, params);
  if (ret != 0) {
    reply_len = strlen(err_i2c_reg_addr);
    memcpy(reply, err_i2c_reg_addr, reply_len);
    return reply_len;  
  }
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    reply_len = strlen(err_i2c_reg_addr);
    memcpy(reply, err_i2c_reg_addr, reply_len);
    return reply_len;  
  }
  unsigned char reg_addr = (char) aux;

  // getting the length of the reading; default to 1.
  int i2c_len = 1;
  if (get_next_param(param, params) == 0) {
    // lets fake the hex parsing
    char tmp; 
    ret = i_from_a (&i2c_len,
                    param,
                    &tmp);
    if (ret != 0 || i2c_len > 10) {
      reply_len = strlen(err_i2c_len);
      memcpy(reply, err_i2c_len, reply_len);
      return reply_len;  
    }
  }

  unsigned char i2c_raw_data[10];
  ret = i2c_reg_read(i2c_addr,
                     reg_addr,
                     i2c_raw_data,
                     i2c_len,
                     cmd_buf[conn_idx].i2c_bus);
  if(ret != 0){
    reply_len = strlen(err_i2c_read_transaction);
    memcpy(reply, err_i2c_read_transaction, reply_len);
    return reply_len;
  }

  int i;
  unsigned char i2c_data[10][6];
  for (i = 0; i < i2c_len; i++) {
    ret = a_from_i((char *) &i2c_data[i],
                   i2c_raw_data[i],
                   cmd_buf[conn_idx].hex);
    if(ret != 0){
      reply_len = strlen(err_i2c_itoa);
      memcpy(reply, err_i2c_itoa, reply_len);
      return reply_len;
    }

    debug_printf("## %s\n", i2c_data[i]);
  }

  unsigned char * p = reply;
  for (i = 0; i < i2c_len; i++) {
    p += strlcpy((char *) p, (const char *) i2c_data[i]);
    *p = ' ';
    p++;
  }
  *(p-1) = '\0';

  debug_printf("## %s\n", reply);

  return strlen((const char *) reply);
}

int
cmd_set_i2c_bus(char * params,
                unsigned char * reply,
                int conn_idx)
{
  int reply_len = 0;
  char ret;
  
  char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params);
  if (ret != 0) {
    reply_len = strlen(err_param);
    memcpy(reply, err_param, reply_len);
    return reply_len;  
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(help_set_i2c_bus);
    memcpy(reply, help_set_i2c_bus, reply_len);
    return reply_len;
  }

  // debug_printf("== %s\n", param);

  if (str_eq(param, "m") == 1) {
    cmd_buf[conn_idx].i2c_bus = 1;
    reply_len = strlen(ok_str);
    memcpy(reply, ok_str, reply_len);
    return reply_len;
  }
  
  if (str_eq(param, "s") == 1) {
    cmd_buf[conn_idx].i2c_bus = 2;
    reply_len = strlen(ok_str);
    memcpy(reply, ok_str, reply_len);
    return reply_len;
  }
  
  reply_len = strlen(err_i2c_bus);
  memcpy(reply, err_i2c_bus, reply_len);
  return reply_len;
}


int
cmd_get_i2c_bus(char * params,
                unsigned char * reply,
                int conn_idx)
{
  int reply_len = 0;
  
  char param[MAX_PARAM_LEN];
  get_next_param(param, params);
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(help_get_i2c_bus);
    memcpy(reply, help_get_i2c_bus, reply_len);
    return reply_len;
  }

  // debug_printf("== %s\n", param);

  if (cmd_buf[conn_idx].i2c_bus == 1) {
    reply_len = strlen(str_i2c_bus_management);
    memcpy(reply, str_i2c_bus_management, reply_len);
    return reply_len;
  }

  if (cmd_buf[conn_idx].i2c_bus == 2) {
    reply_len = strlen(str_i2c_bus_sensor);
    memcpy(reply, str_i2c_bus_sensor, reply_len);
    return reply_len;
  }

  return 0;
}


int
cmd_zynq_reset(char * params,
               unsigned char * reply,
               int conn_idx)
{
  char param[MAX_PARAM_LEN];
  if (get_next_param(param, params) == 0) {
    if (str_eq(param, help_str) == 1
        || str_eq(param, question_mark_str) == 1) {
      return strlcpy((char *) reply, (char *) help_zynq_reset);
    }
  }

  if (zynq_reset() == 0) { 
    return strlcpy((char *) reply, (char *) ok_str);
  }
  
  return strlcpy((char *) reply, (char *) err_zynq_reset);
}

  

