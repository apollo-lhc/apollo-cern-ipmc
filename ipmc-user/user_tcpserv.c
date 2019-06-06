/***********************************************************************

Nom ......... : user_tcpserv.c
Role ........ : TCP/IP server for L0MDT project
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.1 - 18/05/2019

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

#include <user_pca9545.h>
#include <user_tcn75a.h>

#include <user_helpers.h>

// it seems stdio cannot be used... 
// #include <stdio.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

/* ================================================================ */

// char *
// strcpy(char *str_dest, const char *str_src);

void
lowercase(char s[]);

void
remove_extra_spaces(char s[]);

// char *
// get_param(const char * s, const int pos);

void
get_next_param(char * param, char * line);

int
get_signal_index(const char * sm_signal_name);

int
get_cmd_index(const char * cmd);

/* ================================================================ */

typedef struct pin_map_n {
  const char * sm_name;
  const int output;
  const int expert;
  const signal_t ipmc_name;
  const int initial;
} pin_map_t;

static pin_map_t pin_map[] = {
  {"ipmc_zynq_en"     , 1, 1, USER_IO_3                 , 0}, 
  {"en_one_jtag_chain", 1, 0, USER_IO_4                 , 0}, 
  {"uart_addr0"       , 1, 0, USER_IO_5                 , 0}, 
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

// let's declare strings as global variables so they are not included
// in stack.

static const char connection_request_str[] =
  "<_> user_tcpserv: " "New connection request\n";

static const char conn_not_avail_str[] =
  "<W> user_tcpserv: "
  "No user_tcpserv connection slot available\n";

static const char disconn_req_str[] =
  "<_> user_tcpserv: " "Disconnect request received\n";

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

static const char version_str[] = "0.0.0-10\n";
static const char version_help_str[] =
  "Usage: version\n";

static const char echo_str[] = "echo: ";

  

/* ================================================================ */

int
write_gpio_signal(char * params, unsigned char * reply);

int
read_gpio_signal(char * params, unsigned char * reply);

int
set_expert_mode(char * params, unsigned char * reply);

int
help(char * params, unsigned char * reply);

int
write_i2c_mux(char * params, unsigned char * reply);

int
read_i2c_mux(char * params, unsigned char * reply);

int
read_tcn75a(char * params, unsigned char * reply);

int
version(char * params, unsigned char * reply);

/* ================================================================ */

typedef struct cmd_map_n {
  const char * cmd;
  int (*fnc_ptr)(char *, unsigned char *);
} cmd_map_t;

static cmd_map_t cmd_map[] = {
  {"expert_mode"    , & set_expert_mode  },
  {"get_gpio"       , & read_gpio_signal },
  {"read_i2c_mux"   , & read_i2c_mux     },
  {"read_tcn75a"    , & read_tcn75a      },
  {"set_gpio"       , & write_gpio_signal},
  {"write_i2c_mux"  , & write_i2c_mux    },
  {"version"        , & version          },
  {help_str         , & help             },
  {question_mark_str, & help             },
};

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
  debug_printf(connection_request_str);
  
  /* Save client information in a list */

  /* Scan the array to find an empty slot */
  for(i=0; i < MAX_USER_TCPSERV_CLIENT; i++){

    /* Check if current position is already used */
    if(user_tcpserv_clients[i].opened == 0){
      
      user_tcpserv_clients[i].to_port = from_port;

      /* If not, save the client info */
      memcpy(user_tcpserv_clients[i].to, from, sizeof(from));
      
      user_tcpserv_clients[i].opened = 1;


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
  debug_printf(conn_not_avail_str);
	
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
  debug_printf(disconn_req_str);
  
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

  char cmd_line[CMD_LINE_MAX_LEN];
  int cmd_len;

  if (data[len-2] == '\r') {
    cmd_len = len-2;
  }
  else if (data[len-1] == '\n') {
    cmd_len = len-1;
  }
  else {
    cmd_len = len;
  }
  
  memcpy(cmd_line, data, cmd_len);

  cmd_line[cmd_len] = '\0';

  // debug_printf("<_> user_tcpserv command line: ");
  // debug_printf(cmd_line);
  // debug_printf("\n");

  remove_extra_spaces(cmd_line);
  // debug_printf("<_> >>>>>> no extra spaces: ");
  // debug_printf(cmd_line);
  // debug_printf("\n");

  lowercase(cmd_line);
  // debug_printf("<_> >>>>>> all lowercase: ");
  // debug_printf(cmd_line);
  // debug_printf("\n");


  char cmd[30];
  get_next_param(cmd, cmd_line);
  // debug_printf("<_> >>>>>> cmd: ");
  // debug_printf(cmd);
  // debug_printf("\n");

  int cmd_idx = get_cmd_index(cmd);
  // debug_printf("<_> >>>>>> cmd_idx: ");
  // debug_printf("%d", cmd_idx);
  // debug_printf("\n");
 
  if (cmd_idx >= 0) { 
    // execute command, get reply and associated length
    *replyLen = cmd_map[cmd_idx].fnc_ptr(cmd_line, reply);
    // debug_printf("----- cmd reply len: %d\n", *replyLen);
  }

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

  if (expert_mode == 1) {

    unsigned char * r = &reply[*replyLen];
    int l = strlen(expert_str);
    memcpy(r, expert_str, l);
    r += l;
    *r = '\0';
    *replyLen += l;

    // debug_printf("----- expert reply len: %d\n", *replyLen);
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
void lowercase(char s[])
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


void remove_extra_spaces(char s[])
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

// look for signal information in the pin map table and return its
// position. -1 is returned in case no signal is found.
int
get_signal_index(const char * sm_signal_name)
{
  int i = 0;
  int map_len = sizeof(pin_map) / sizeof(pin_map[0]);

  for (i = 0; i < map_len; i++) {

    // debug_printf("<_> ++++++++ signal_ids: ");
    // debug_printf(pin_map[i].sm_name);
    // debug_printf(" ");
    // debug_printf(sm_signal_name);
    // debug_printf("\n");
    
    if (str_eq(pin_map[i].sm_name, sm_signal_name) == 1) {
      return i;
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
  int map_len = sizeof(cmd_map) / sizeof(cmd_map[0]);
  
  for (i = 0; i < map_len; i++) {

    // debug_printf("<_> ++++++++ cmd_ids: ");
    // debug_printf(cmd_map[i].cmd);
    // debug_printf(" ");
    // debug_printf(cmd);
    // debug_printf("\n");

    if (str_eq(cmd_map[i].cmd, cmd) == 1) {
      return i;
    }
  }
  return -1;
}

// copy the first word from line (until delimiter) to param, removing
// it from the content of the line.
void
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

  return;
}


/* ================================================================ */

int
write_gpio_signal(char * params,
                  unsigned char * reply)
{
  // debug_printf("<_> ======= write_gpio_signal\n");

  char param[30];
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
  
    signal_t userio_sig = pin_map[idx].ipmc_name;

    if ( (pin_map[idx].output == 1 && pin_map[idx].expert == 0)
         || (pin_map[idx].output == 1
             && pin_map[idx].expert == 1 && expert_mode == 1) ) {

      get_next_param(param, params);

      // debug_printf("<_> ======= value: ");
      // debug_printf(value);
      // debug_printf("\n");
      
      if (param[0] == '1' || param[0] == 'h'){
        signal_activate(&userio_sig);
        // debug_printf("<_> ======= signal deactivated\n");
        msg_len = strlen(ok_str);
        memcpy(reply, ok_str, msg_len);
      }
      else if (param[0] == '0' || param[0] == 'l') {
        signal_deactivate(&userio_sig);
        // debug_printf("<_> ======= signal activated\n");
        msg_len = strlen(ok_str);
        memcpy(reply, ok_str, msg_len);
      }
      else {
        msg_len = strlen(set_gpio_error_str);
        memcpy(reply, set_gpio_error_str, msg_len);
      }
    }
    else {
      msg_len = strlen(set_gpio_error_str);
      memcpy(reply, set_gpio_error_str, msg_len);
    }
  }
  else {
    msg_len = strlen(signal_not_found_str);
    memcpy(reply, signal_not_found_str, msg_len);
  }

  return msg_len;
}

// read pin and fill reply string with associated value. returns the
// size of the reply.
int
read_gpio_signal(char * params,
                 unsigned char * reply)
{

  // debug_printf("<_> ======= read_gpio_signal\n");

  char param[30];
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

  
    signal_t userio_sig = pin_map[idx].ipmc_name;
  
    int v = signal_read(&userio_sig);
  
    if (v == 1) {
      reply_len = strlen(gpio_enabled_str);
      memcpy(reply, gpio_enabled_str, reply_len);
    }
    else if (v == 0) {
      reply_len = strlen(gpio_disabled_str);
      memcpy(reply, gpio_disabled_str, reply_len);
    }
    else{
      reply_len = strlen(gpio_error_str);
      memcpy(reply, gpio_error_str, reply_len);
    }
  }
  else {
    // debug_printf("######## read_gpio_signal (idx not found) 3\n");
    reply_len = strlen(signal_not_found_str);
    memcpy(reply, signal_not_found_str, reply_len);
  }
  // debug_printf("######## read_gpio_signal  (final) 4\n");

  return reply_len;
}

int
set_expert_mode(char * params,
                unsigned char * reply)
{

  int reply_len = 0;
  
  char param[30];
  get_next_param(param, params);


  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(expert_help_str);
    memcpy(reply, expert_help_str, reply_len);
    return reply_len;
  }

  
  if (str_eq(param, "on") == 1){
    expert_mode = 1;
    reply_len = 0;
  }

  else {
    expert_mode = 0;
    reply_len = strlen(expert_off_str);
    memcpy(reply, expert_off_str, reply_len);
  }    
  
  return reply_len;
}

int
help (char * params,
      unsigned char * reply)
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

  map_len = sizeof(pin_map) / sizeof(pin_map[0]);
  for (i = 0; i < map_len; i++) {
    memcpy(r, "  ", 2);
    r += 2;
    len = strlen(pin_map[i].sm_name);
    memcpy(r, pin_map[i].sm_name, len);
    r += len;
    if (pin_map[i].expert == 1) {
      len = strlen(expert_label_str);
      memcpy(r, expert_label_str, len);
      r += len;
    }
    *r = '\n';
    r++;
  }

  // debug_printf("..... help 5\n");

  len = strlen(help_footer_str);
  memcpy(r, help_footer_str, len);
  r += len;
  *r = '\0';

  // debug_printf("..... help 6\n");

  return strlen((char *) reply);
  
}


int
write_i2c_mux(char * params, unsigned char * reply)
{

  int reply_len = 0;
  
  char param[30];
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
read_i2c_mux(char * params, unsigned char * reply)
{

  int reply_len = 0;
  
  char param[30];
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
read_tcn75a(char * params, unsigned char * reply)
{
  int reply_len = 0;
  
  char param[30];
  get_next_param(param, params);


  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    reply_len = strlen(tcn75a_help_str);
    memcpy(reply, tcn75a_help_str, reply_len);
    return reply_len;
  }

  unsigned char temp;
  char ret = tcn75a_read((unsigned char *) param, &temp);
  
  if(ret == 0){
    itoa(temp, (char *) reply);
    reply_len = strlen((char *) reply);
    reply[reply_len]='\n';
    reply_len++;
    reply[reply_len]='\0';    
    return reply_len;
  }
  
  reply_len = strlen(error_str);
  memcpy(reply, error_str, reply_len);
  return reply_len;

}

int
version(char * params, unsigned char * reply)
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

  reply_len = strlen(version_str);
  memcpy(reply, version_str, reply_len);
  return reply_len;

}
