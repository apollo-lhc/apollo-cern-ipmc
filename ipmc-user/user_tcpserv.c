/***********************************************************************
Nom ......... : user_tcpserv.c
Role ........ : TCP/IP server for L0MDT project
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.2 - 2019-07-31
***********************************************************************/

static const char debug = 0;

#include <app.h>
#include <cfgint.h>
#include <ipmc.h>
#include <log.h>
#include <debug.h>

#include <net/tcp.h>
#include <net/ip.h>

#include <user_tcpserv.h>

#include <user_zynq.h>
#include <user_pca9545.h>
#include <user_tcn75.h>
#include <user_gpio.h>
#include <user_i2c.h>
#include <user_version.h>
#include <user_uart_forward.h>

#include <user_helpers.h>

#define MAX_USER_TCPSERV_CLIENT 10
#define CMD_LINE_MAX_LEN 50
#define MAX_PARAM_LEN 20
#define MAX_I2C_LEN 6

// structure to hold status of a connection
typedef struct cmd_buf_n {
  unsigned char data[CMD_LINE_MAX_LEN];
  unsigned short len;
  unsigned char expert;
  unsigned char hex;
  unsigned char eol[3];
  unsigned char i2c_bus;
} cmd_buf_t;

cmd_buf_t cmd_buf[MAX_USER_TCPSERV_CLIENT];

static const unsigned char DEBUG = 0;

static unsigned char i2c_bin_data[MAX_I2C_LEN];
static unsigned char i2c_ascii_data[MAX_I2C_LEN][10];

/* ================================================================ */

void
lowercase(unsigned char s[]);

void
remove_extra_spaces(unsigned char s[]);

int
get_next_param(unsigned char * param, unsigned char * line);

int
get_cmd_index(const unsigned char * cmd);



int
append_to_cmd_buffer(int conn_idx,
                     const unsigned char * data,
                     unsigned short len);

int
chomp_cmd(unsigned char * cmd_line,
          int conn_idx);

/* ================================================================ */

// let's declare strings as global variables so they are not included
// in stack.

static const unsigned char connection_request_str[] =
  "<_> user_tcpserv: "
  "New connection request\n";

static const unsigned char conn_not_avail_str[] =
  "<W> user_tcpserv: "
  "No user_tcpserv connection slot available\n";

static const unsigned char disconn_req_str[] =
  "<_> user_tcpserv: "
  "Disconnect request received\n";

static const unsigned char expert_str[] =
  "Expert mode is enabled.\n";

static const unsigned char prompt_str[] = ":: ";

static const unsigned char help_str[] = "help";

static const unsigned char question_mark_str[] = "?";

static const unsigned char set_gpio_help_str[] =
  "Usage: set_gpio <signal name> <value>.\n"
  "Value should be 1 (one) to activate the signal.\n"
  "Value should be 0 (zero) to deactivate the signal.\n";

static const unsigned char ok_str[] = "OK\n";

static const unsigned char set_gpio_error_str[] =
  "Signal cannot be written.\n";

static const unsigned char signal_not_found_str[] =
  "Signal not found.\n";

static const unsigned char get_gpio_help_str[] =
  "Usage: get_gpio <signal name>.\n"
  "Returns 1 (one) if signal is activated.\n"
  "Returns 0 (zero) if signal is deactivated.\n";

static const unsigned char gpio_enabled_str[] = "1\n";
static const unsigned char gpio_disabled_str[] = "0\n";
static const unsigned char gpio_error_str[] =
  "Unexpected value read from signal.\n";

static const unsigned char expert_help_str[] =
  "Usage: expert_mode <value>.\n"
  "Value should be 'on' (no quotes) to allow "
  "overwrite of special signals.\n"
  "Any other value, including empty (no value), "
  "deactivates expert mode.\n";

static const unsigned char expert_off_str[] =
  "Expert mode deactivated.\n";

static const unsigned char cmds_header_str[] =
  "Available commands:\n";
static const unsigned char signals_header_str[] =
  "Available signals (marked if expert mode required):\n";
static const unsigned char help_footer_str[] =
  "Try also \"<command> help\".\n";

static const unsigned char expert_label_str[] = " (E)";

static const unsigned char i2c_mux_write_help_str[] =
  "Usage: write_i2c_mux <mask>\n"
  "mask: 4-bit integer, one for each lane.\n"
  "Ex: \"write_i2c_mux 3\" enables two lanes.\n";

static const unsigned char i2c_mux_read_help_str[] =
  "Usage: read_i2c_mux\n";

static const unsigned char i2c_mux_error_str[] =
  "I2C mux operation error.\n";

static const unsigned char tcn75a_help_str[] =
  "Usage: read_tcn75a <sensor UID>\n"
  "  UID is U34, U35 or U36.\n";

static const unsigned char error_str[] =
  "Something has gone wrong.\n";

static const unsigned char version_help_str[] =
  "Usage: version\n";

static const unsigned char echo_str[] =
  "Command not recognized. Echoing:\n";

static const unsigned char help_i2c_write[] =
  "Low level I2C write operation with no target register.\n"
  "Usage: i2c_w <i2c_addr> <data>\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <data> multiple values to be written separated by spaces.\n";

static const unsigned char help_i2c_reg_write[] =
  "Low level I2C write operation with target register.\n"
  "Usage: i2c_reg_w <i2c_addr> <reg_addr> <data>\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <reg_addr> is the target register.\n"
  "  <data> multiple values to be written separated by spaces.\n";

static const unsigned char err_i2c_addr[] =
  "Invalid I2C address.\n";

static const unsigned char err_i2c_data[] =
  "Invalid data.\n";

static const unsigned char err_i2c_write_transaction[] =
  "I2C write transaction unsuccessful.\n";

static const unsigned char help_i2c_read[] =
  "Low level I2C read operation with no target register.\n"
  "Usage: i2c_r <i2c_addr> [nbytes]\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  [nbytes] number of bytes to be read, 1 if empty.\n";

static const unsigned char help_i2c_reg_read[] =
  "Low level I2C read operation with target register.\n"
  "Usage: i2c_reg_r <i2c_addr> <reg_addr> [nbytes]\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <reg_addr> is the target register.\n"
  "  [nbytes] number of bytes to be read, 1 if empty.\n";

static const unsigned char err_i2c_read_transaction[] =
  "I2C read transaction unsuccessful.\n";

static const unsigned char err_i2c_itoa[] =
  "Conversion from integer to ASCII failed.\n";

static const unsigned char err_i2c_len[] =
  "Invalid length.\n";

static const unsigned char err_param[] =
  "Unexpected parameters format.\n";

static const unsigned char err_i2c_reg_addr[] =
  "Invalid register address.\n";

static const unsigned char help_set_i2c_bus[] =
  "Usage: set_i2c_bus <bus ID>\n"
  "  <bus ID>:\n"
  "    M for management bus \n"
  "    S for sensor bus\n";

static const unsigned char help_get_i2c_bus[] =
  "Usage: get_i2c_bus\n";

static const unsigned char err_i2c_bus[] =
  "Invalid I2C bus ID.\n";

static const unsigned char str_i2c_bus_management[] =
  "Management\n";

static const unsigned char str_i2c_bus_sensor[] =
  "Sensor\n";

static const unsigned char help_zynq_restart[] =
  "Restart only Zynq.\n"
  "Usage: zynq_restart [delay]\n"
  "  [delay]: integer, seconds, defaults to 2.\n";

static const unsigned char err_zynq_restart[] =
  "Zynq restart did not happen.\n";

static const unsigned char help_zynq_i2c_write[] =
  "Low level I2C write operation for Zynq registers.\n"
  "Usage: zynq_i2c_w <i2c_addr> <reg_addr> <data>\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <reg_addr> is the target register.\n"
  "  <data> multiple space separated integer values.\n";

static const unsigned char help_zynq_i2c_read[] =
  "Low level I2C read operation for Zynq registers.\n"
  "Usage: zynq_i2c_r <i2c_addr> <reg_addr> [nbytes]\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <reg_addr> is the target register.\n"
  "  [nbytes] number of bytes to be read, 1 if empty.\n";


/* ================================================================ */

int
set_gpio(unsigned char * params,
         unsigned char * reply,
         int conn_idx);

int
get_gpio(unsigned char * params,
         unsigned char * reply,
         int conn_idx);


int
expert_mode(unsigned char * params,
            unsigned char * reply,
            int conn_idx);

int
help(unsigned char * params,
         unsigned char * reply,
         int conn_idx);

int
write_i2c_mux(unsigned char * params,
              unsigned char * reply,
              int conn_idx);

int
read_i2c_mux(unsigned char * params,
             unsigned char * reply,
             int conn_idx);

int
read_tcn75a(unsigned char * params,
            unsigned char * reply,
            int conn_idx);

int
i2c_w(unsigned char * params,
      unsigned char * reply,
      int conn_idx);

int
i2c_r(unsigned char * params,
      unsigned char * reply,
      int conn_idx);

int
i2c_reg_w(unsigned char * params,
          unsigned char * reply,
          int conn_idx);

int
i2c_reg_r(unsigned char * params,
          unsigned char * reply,
          int conn_idx);

int
version(unsigned char * params,
        unsigned char * reply,
        int conn_idx);

int
set_i2c_bus(unsigned char * params,
            unsigned char * reply,
            int conn_idx);

int
get_i2c_bus(unsigned char * params,
            unsigned char * reply,
            int conn_idx);

int
zynq_restart(unsigned char * params,
           unsigned char * reply,
           int conn_idx);

int
zynq_i2c_w(unsigned char * params,
           unsigned char * reply,
           int conn_idx);

int
zynq_i2c_r(unsigned char * params,
           unsigned char * reply,
           int conn_idx);

int
uart_forward(unsigned char * params
             , unsigned char * reply
             , int conn_idx);

/* ================================================================ */

typedef struct cmd_map_n {
  const unsigned char * cmd;
  int (*fnc_ptr)(unsigned char *, unsigned char *, int);
} cmd_map_t;


#define NAME(Variable) (#Variable)
#define CMD_FUNC(n) {(unsigned char *) NAME(n), n}
#define ALIAS_FUNC(a,n) {a, n}

static const cmd_map_t cmd_map[] = {
    CMD_FUNC(expert_mode      ),
    CMD_FUNC(get_gpio         ),
    CMD_FUNC(get_i2c_bus      ),
    CMD_FUNC(set_i2c_bus      ),
    CMD_FUNC(i2c_reg_r        ),
    CMD_FUNC(i2c_reg_w        ),
    CMD_FUNC(i2c_r            ),
    CMD_FUNC(i2c_w            ),
    CMD_FUNC(read_i2c_mux     ),
    CMD_FUNC(read_tcn75a      ),
    CMD_FUNC(set_gpio         ),
    CMD_FUNC(uart_forward     ),
    CMD_FUNC(write_i2c_mux    ),
    CMD_FUNC(version          ),
    CMD_FUNC(zynq_restart     ),
    CMD_FUNC(zynq_i2c_r       ),
    CMD_FUNC(zynq_i2c_w       ),
    CMD_FUNC(help             ),
    ALIAS_FUNC(question_mark_str, help)
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
      cmd_buf[i].expert = 0;
      cmd_buf[i].hex = 0;
      cmd_buf[i].eol[0] = '\n';
      cmd_buf[i].eol[1] = '\0';
      cmd_buf[i].i2c_bus = SENSOR_I2C_BUS;

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


  if (debug) {
    debug_printf("<_> ======= 1\n");
  }

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

  if (debug) {
    debug_printf("<_> ======= 2\n");
  }
  
  // append received data to associated buffer
  // check for overflow
  if (append_to_cmd_buffer(conn_idx, data, len) != 0){
    return 0;
  }

  unsigned char cmd_line[CMD_LINE_MAX_LEN];

  if (debug) {
    debug_printf("<_> ======= 3\n");
  }
  
  // look for termination
  // copy from buffer to cmd line;
  // terminates cmd line with '\0' for str_eq
  // shift remaining of the buffer to the start
  // returns the len of the command found, or -1 otherwise
  int cmd_len = chomp_cmd(cmd_line, conn_idx);
  
  if (debug) {
    debug_printf("<_> ======= 4\n");
  }

  if (cmd_len < 0) {
    *replyLen = 0;
    return 0;
  }
  
  
  if (debug) {
    debug_printf("<_> ======= cmd line: %s|\n", cmd_line);
  }
    

  remove_extra_spaces(cmd_line);
  if (debug) {
    debug_printf("<_> >>>>>> no extra spaces: %s|\n", cmd_line);
  }

  lowercase(cmd_line);
  if (debug) {
    debug_printf("<_> >>>>>> all lowercase: %s|\n", cmd_line);
  }

  unsigned char cmd[MAX_PARAM_LEN];
  get_next_param(cmd, cmd_line);
  if (debug) {
    debug_printf("<_> >>>>>> cmd: %s|\n", cmd);
  }

  int cmd_idx = get_cmd_index(cmd);
  if (debug) {
    debug_printf("<_> >>>>>> cmd_idx: %d\n", cmd_idx);
  }
  
  // if a command was found, execute it.
  if (cmd_idx >= 0) { 
    // execute command, get reply and associated length
    *replyLen = cmd_map[cmd_idx].fnc_ptr(cmd_line, reply, conn_idx);
    // debug_printf("<_> >>>>>> cmd reply len: %d\n", *replyLen);
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

  if (user_is_expert_mode_on() == 1) {
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
get_cmd_index(const unsigned char * cmd)
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
get_next_param(unsigned char * param,
               unsigned char * line)
{

  unsigned char * target = param;
  unsigned char * l = line;

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

  unsigned char tmp[50];
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
chomp_cmd(unsigned char * cmd_line,
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
      unsigned char * p = &(cmd_buf[conn_idx].data[i]);
      while (*p == '\r' || *p == '\n') {
        p++;
        cmd_buf[conn_idx].len--;
      }

      // shift data to beginning of buffer
      unsigned char * q = cmd_buf[conn_idx].data;
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
set_gpio(unsigned char * params,
         unsigned char * reply,
         int conn_idx)
{
  // debug_printf("<_> ======= write_gpio_signal\n");

  unsigned char param[MAX_PARAM_LEN];
  get_next_param(param, params);

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, set_gpio_help_str);
  }
  
  int idx = user_get_signal_index(param);
  if (idx >= 0) {
    get_next_param(param, params);
    if (param[0] == '1' || param[0] == 'h'){
      if (user_set_gpio(idx, 1) == 0) {
        return strlcpy(reply, ok_str);
      }
    }
    else if (param[0] == '0' || param[0] == 'l') {
      if (user_set_gpio(idx, 0) == 0) {
        return strlcpy(reply, ok_str);
      }      
    }
    return strlcpy(reply, set_gpio_error_str);
  }
  return strlcpy(reply, signal_not_found_str);
}

// read pin and fill reply string with associated value. returns the
// size of the reply.
int
get_gpio(unsigned char * params,
         unsigned char * reply,
         int conn_idx)
{

  // debug_printf("<_> ======= read_gpio_signal\n");

  unsigned char param[MAX_PARAM_LEN];
  get_next_param(param, params);

  // debug_printf("<_> ======= ");
  // debug_printf(sm_signal_name);
  // debug_printf("\n");

  // debug_printf("######## read_gpio_signal 1\n");


  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, get_gpio_help_str);
  }

  // debug_printf("######## read_gpio_signal 1.3\n");

  int idx = user_get_signal_index(param);
  
  if (idx >= 0) {

    // debug_printf("######## read_gpio_signal 2 (idx found)\n");

    int v = user_get_gpio(idx);
    
    if (v > 0) {
      return strlcpy(reply, gpio_enabled_str);
    } else {
      return strlcpy(reply, gpio_disabled_str);
    }
  }
  return strlcpy(reply, signal_not_found_str);
}

int
expert_mode(unsigned char * params,
            unsigned char * reply,
            int conn_idx)
{
  unsigned char param[MAX_PARAM_LEN];
  get_next_param(param, params);


  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, expert_help_str);
  }

  
  if (str_eq(param, (unsigned char *) "on") == 1){
    user_enable_expert_mode();
    return 0;
  }

  user_disable_expert_mode();
  return strlcpy(reply, expert_off_str);
}

int
help (unsigned char * params,
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

  for (i = 0; i < user_get_n_pins(); i++) {
    r += strlcpy(r, (unsigned char *) "  ");
    r += strlcpy(r, user_get_signal_sm_name(i));
    if (user_get_signal_expert_mode(i) == 1) {
      r += strlcpy(r, expert_label_str);
    }
    *r = '\n';
    r++;
  }

  // debug_printf("..... help 5\n");

  r += strlcpy(r, help_footer_str);
  *r = '\0';

  // debug_printf("..... help 6\n");

  return strlen(reply);
  
}


int
write_i2c_mux(unsigned char * params,
              unsigned char * reply,
              int conn_idx)
{
  char ret;
  int aux;
  unsigned char param[MAX_PARAM_LEN];

  if (get_next_param(param, params)) {
    return strlcpy(reply, err_param);
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, i2c_mux_write_help_str);
  }

  // getting I2C address
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strlcpy(reply, err_i2c_addr);
  }

  unsigned char mask = (unsigned char) aux;

  if(user_pca9545_write(mask)) {
    return strlcpy(reply, i2c_mux_error_str);
  }
  
  return strlcpy(reply, ok_str);
}


int
read_i2c_mux(unsigned char * params,
             unsigned char * reply,
             int conn_idx)
{
  unsigned char param[MAX_PARAM_LEN];

  get_next_param(param, params);
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, i2c_mux_read_help_str);
  }

  unsigned char value;
  if (user_pca9545_read(&value)) {
    return strlcpy(reply, i2c_mux_error_str);
  }
  
  if (a_from_i(reply, value, cmd_buf[conn_idx].hex)) {
    return strlcpy(reply, err_i2c_itoa);
  }

  int len = strlen(reply);
  reply[len++] = '\n';
  reply[len] = '\0';
  return len;
}

int
read_tcn75a(unsigned char * params,
            unsigned char * reply,
            int conn_idx)
{
  int reply_len = 0;
  char ret;
  unsigned char param[MAX_PARAM_LEN];

  get_next_param(param, params);

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, tcn75a_help_str);
  }

  unsigned char temp;
  int id;

  i_from_a(&id,
           param,
           &(cmd_buf[conn_idx].hex));

  ret = user_tcn75_read_reg((unsigned char) id, 0x00, &temp);
  if (ret != 0) {
    return strlcpy(reply, error_str);
  }

  
  ret = a_from_i(reply, temp, 0);
  if(ret != 0){
    return strlcpy(reply, err_i2c_itoa);
  }

  reply_len = strlen(reply);
  reply[reply_len++]='\n';
  reply[reply_len]='\0';    
  return reply_len;
}

int
version(unsigned char * params,
        unsigned char * reply,
        int conn_idx)
{
  unsigned char param[20];
  get_next_param(param, params);

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, version_help_str);
  }

  return user_get_version(reply);
}


int
i2c_w(unsigned char * params,
          unsigned char * reply,
          int conn_idx)
{
  char ret;
  int aux;
  
  unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params);
  if (ret != 0) {
    return strlcpy(reply, err_param);
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, help_i2c_write);
  }


  // getting I2C address
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strlcpy(reply, err_i2c_addr);
  }

  char i2c_addr = (char) aux;

  // getting the data
  unsigned char i2c_data[MAX_I2C_LEN];
  int i2c_len = 0;
  while (get_next_param(param, params) == 0) {
    ret = i_from_a(&aux,
                   param,
                   &(cmd_buf[conn_idx].hex));
    if (ret != 0) {
      return strlcpy(reply, err_i2c_data);
    }

    i2c_data[i2c_len] = (char) aux;
    i2c_len++;
  }

  // making sure we have something to send to the target...
  if (i2c_len == 0) {
    return strlcpy(reply, err_i2c_data);
  }

  // but it can not be too much...
  if (i2c_len > MAX_I2C_LEN) {
    return strlcpy(reply, err_i2c_len);
  }

  // writing to the I2C address
  ret = user_i2c_write(i2c_addr,
                       i2c_data,
                       i2c_len,
                       cmd_buf[conn_idx].i2c_bus);
  if(ret != 0){
    return strlcpy(reply, err_i2c_write_transaction);
  }

  return strlcpy(reply, ok_str);
}

int
i2c_r(unsigned char * params,
         unsigned char * reply,
         int conn_idx)
{
  char ret;
  int aux;  
  
  unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params);
  if (ret != 0) {
    return strlcpy(reply, err_param);
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, help_i2c_read);
  }

  if (DEBUG) {
    debug_printf("~~~~~~~~ %s\n", param);
  }
  
  // getting I2C address
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strlcpy(reply, err_i2c_addr);
  }
  unsigned char i2c_addr = (char) aux;

  // getting the length of the reading; default to 1.
  int i2c_len = 1;
  if (get_next_param(param, params) == 0) {
    // lets fake the hex parsing
    unsigned char tmp; 
    ret = i_from_a (&i2c_len,
                    param,
                    &tmp);
    if (ret != 0 || i2c_len > MAX_I2C_LEN) {
      return strlcpy(reply, err_i2c_len);
    }
  }

  unsigned char i2c_raw_data[MAX_I2C_LEN];
  ret = user_i2c_read(i2c_addr,
                      i2c_raw_data,
                      i2c_len,
                      cmd_buf[conn_idx].i2c_bus);
  if(ret != 0){
    return strlcpy(reply, err_i2c_read_transaction);
  }


  int i;
  unsigned char i2c_data[MAX_I2C_LEN][6];
  for (i = 0; i < i2c_len; i++) {
    ret = a_from_i(&(*i2c_data[i]),
                   i2c_raw_data[i],
                   cmd_buf[conn_idx].hex);
    if(ret != 0){
      return strlcpy(reply, err_i2c_itoa);
    }
  }

  unsigned char * p = reply;
  for (i = 0; i < i2c_len; i++) {
    p += strlcpy(p, i2c_data[i]);
    *p = ' ';
    p++;
  }
  *(p-1) = '\0';

  return strlen(reply);
}


int
i2c_reg_w(unsigned char * params,
                  unsigned char * reply,
                  int conn_idx)
{
  char ret;
  int aux;
  
  unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params);
  if (ret != 0) {
    return strlcpy(reply, err_param);
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, help_i2c_reg_write);
  }

  // getting I2C address
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strlcpy(reply, err_i2c_addr);
  }
  char i2c_addr = (char) aux;

  // getting register address
  ret = get_next_param(param, params);
  if (ret != 0) {
    return strlcpy(reply, err_i2c_reg_addr);
  }
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strlcpy(reply, err_i2c_reg_addr);
  }
  unsigned char reg_addr = (char) aux;

  // getting the data
  int i2c_len = 0;
  while (get_next_param(param, params) == 0) {
    ret = i_from_a(&aux,
                   param,
                   &(cmd_buf[conn_idx].hex));
    if (ret != 0) {
      return strlcpy(reply, err_i2c_data);
    }

    i2c_bin_data[i2c_len] = (char) aux;
    i2c_len++;
  }

  // making sure we have something to send to the target...
  if (i2c_len == 0) {
    return strlcpy(reply, err_i2c_data);
  }

  // but it can not be too much...
  if (i2c_len > MAX_I2C_LEN) {
    return strlcpy(reply, err_i2c_len);
  }

  // writing to the I2C address
  ret = user_i2c_reg_write(i2c_addr,
                           reg_addr,
                           i2c_bin_data,
                           i2c_len,
                           cmd_buf[conn_idx].i2c_bus);
  if(ret != 0){
    return strlcpy(reply, err_i2c_write_transaction);
  }

  return strlcpy(reply, ok_str);
}

int
i2c_reg_r(unsigned char * params,
             unsigned char * reply,
             int conn_idx)
{
  int reply_len = 0;
  char ret;
  int aux;  
  
  unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params);
  if (ret != 0) {
    return strlcpy(reply, err_param);
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, help_i2c_reg_read);
  }

  if (DEBUG) {
    debug_printf("== %s\n", param);
  }
  
  // getting I2C address
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strlcpy(reply, err_i2c_addr);
  }
  unsigned char i2c_addr = (char) aux;

  // getting register address
  ret = get_next_param(param, params);
  if (ret != 0) {
    return strlcpy(reply, err_i2c_reg_addr);
  }
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strlcpy(reply, err_i2c_reg_addr);
  }
  unsigned char reg_addr = (char) aux;

  // getting the length of the reading; default to 1.
  int i2c_len = 1;
  if (get_next_param(param, params) == 0) {
    // lets fake the hex parsing
    unsigned char tmp; 
    ret = i_from_a (&i2c_len,
                    param,
                    &tmp);
    if (ret != 0 || i2c_len > MAX_I2C_LEN) {
      return strlcpy(reply, err_i2c_len);
    }
  }

  ret = user_i2c_reg_read(i2c_addr,
                          reg_addr,
                          i2c_bin_data,
                          i2c_len,
                          cmd_buf[conn_idx].i2c_bus);
  if(ret != 0){
    reply_len = strlen(err_i2c_read_transaction);
    memcpy(reply, err_i2c_read_transaction, reply_len);
    return reply_len;
  }

  int i;
  for (i = 0; i < i2c_len; i++) {
    ret = a_from_i(&(*i2c_ascii_data[i]),
                   i2c_bin_data[i],
                   cmd_buf[conn_idx].hex);
    if(ret != 0){
      return strlcpy(reply, err_i2c_itoa);
    }

    if (DEBUG) {
      debug_printf("## %s\n", i2c_ascii_data[i]);
    }
  }

  unsigned char * p = reply;
  for (i = 0; i < i2c_len; i++) {
    p += strlcpy(p, i2c_ascii_data[i]);
    *p = ' ';
    p++;
  }
  *(p-1) = '\0';

  if (DEBUG) {
    debug_printf("## %s\n", reply);
  }

  return strlen(reply);
}

int
set_i2c_bus(unsigned char * params,
                unsigned char * reply,
                int conn_idx)
{
  char ret;
  
  unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params);
  if (ret != 0) {
    return strlcpy(reply, err_param);
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, help_set_i2c_bus);
  }

  if (DEBUG) {
    debug_printf("== %s\n", param);
  }

  if (param[0] == 'm') {
    cmd_buf[conn_idx].i2c_bus = MNGMNT_I2C_BUS;
    return strlcpy(reply, ok_str);
  }
  
  if (param[0] == 's') {
    cmd_buf[conn_idx].i2c_bus = SENSOR_I2C_BUS;
    return strlcpy(reply, ok_str);
  }
  
  return strlcpy(reply, err_i2c_bus);
}


int
get_i2c_bus(unsigned char * params,
            unsigned char * reply,
            int conn_idx)
{
  unsigned char param[MAX_PARAM_LEN];
  get_next_param(param, params);
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, help_get_i2c_bus);
  }

  if (DEBUG) {
    debug_printf("== %s\n", param);
  }

  if (cmd_buf[conn_idx].i2c_bus == 1) {
    return strlcpy(reply, str_i2c_bus_management);
  }

  if (cmd_buf[conn_idx].i2c_bus == 2) {
    return strlcpy(reply, str_i2c_bus_sensor);
  }

  return strlcpy(reply, err_i2c_bus);
}


int
zynq_restart(unsigned char * params,
           unsigned char * reply,
           int conn_idx)
{
  unsigned char param[MAX_PARAM_LEN];
  int delay = 2;

  char ret = get_next_param(param, params);

  if (ret == 0) {
    if (str_eq(param, help_str) == 1
        || str_eq(param, question_mark_str) == 1) {
      return strlcpy(reply, help_zynq_restart);
    }

    unsigned char tmp; 
    ret = i_from_a (&delay,
                    param,
                    &tmp);
    if (ret != 0) {
      return strlcpy(reply, err_param);
    }
  }

  
  if (user_zynq_request_restart((char) delay) == 0) { 
    return strlcpy(reply, ok_str);
  }
  
  return strlcpy(reply, err_zynq_restart);
}


int
zynq_i2c_w(unsigned char * params,
           unsigned char * reply,
           int conn_idx)
{
  char ret;
  int aux;
  
  unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params);
  if (ret != 0) {
    return strlcpy(reply, err_param);
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, help_zynq_i2c_write);
  }

  // getting I2C address
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strlcpy(reply, err_i2c_addr);
  }
  char i2c_addr = (char) aux;

  // getting register address
  ret = get_next_param(param, params);
  if (ret != 0) {
    return strlcpy(reply, err_i2c_reg_addr);
  }
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strlcpy(reply, err_i2c_reg_addr);
  }
  unsigned char reg_addr = (char) aux;

  // getting the data
  int i2c_len = 0;
  while (get_next_param(param, params) == 0) {
    ret = i_from_a(&aux,
                   param,
                   &(cmd_buf[conn_idx].hex));
    if (ret != 0) {
      return strlcpy(reply, err_i2c_data);
    }

    i2c_bin_data[i2c_len] = (char) aux;
    i2c_len++;
  }

  // making sure we have something to send to the target...
  if (i2c_len == 0) {
    return strlcpy(reply, err_i2c_data);
  }

  // but it can not be too much...
  if (i2c_len > MAX_I2C_LEN) {
    return strlcpy(reply, err_i2c_len);
  }

  // writing to the I2C address
  ret = user_zynq_i2c_write(i2c_addr,
                            reg_addr,
                            i2c_bin_data,
                            i2c_len);
  if(ret != 0){
    return strlcpy(reply, err_i2c_write_transaction);
  }

  return strlcpy(reply, ok_str);
}

int
zynq_i2c_r(unsigned char * params,
           unsigned char * reply,
           int conn_idx)
{
  int reply_len = 0;
  char ret;
  int aux;  
  
  unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params);
  if (ret != 0) {
    return strlcpy(reply, err_param);
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strlcpy(reply, help_zynq_i2c_read);
  }

  if (DEBUG) {
    debug_printf("== %s\n", param);
  }
  
  // getting I2C address
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strlcpy(reply, err_i2c_addr);
  }
  unsigned char i2c_addr = (char) aux;

  // getting register address
  ret = get_next_param(param, params);
  if (ret != 0) {
    return strlcpy(reply, err_i2c_reg_addr);
  }
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strlcpy(reply, err_i2c_reg_addr);
  }
  unsigned char reg_addr = (char) aux;

  // getting the length of the reading; default to 1.
  int i2c_len = 1;
  if (get_next_param(param, params) == 0) {
    // lets fake the hex parsing
    unsigned char tmp; 
    ret = i_from_a (&i2c_len,
                    param,
                    &tmp);
    if (ret != 0 || i2c_len > MAX_I2C_LEN) {
      return strlcpy(reply, err_i2c_len);
    }
  }

  ret = user_zynq_i2c_read(i2c_addr,
                           reg_addr,
                           i2c_bin_data,
                           i2c_len);
  if(ret != 0){
    reply_len = strlen(err_i2c_read_transaction);
    memcpy(reply, err_i2c_read_transaction, reply_len);
    return reply_len;
  }

  int i;
  for (i = 0; i < i2c_len; i++) {
    ret = a_from_i(&(*i2c_ascii_data[i]),
                   i2c_bin_data[i],
                   cmd_buf[conn_idx].hex);
    if(ret != 0){
      return strlcpy(reply, err_i2c_itoa);
    }

    debug_printf("## %s\n", i2c_ascii_data[i]);
  }

  unsigned char * p = reply;
  for (i = 0; i < i2c_len; i++) {
    debug_printf("zynq i2c data vector [%d]: %s\n", i, i2c_ascii_data[i]);
    p += strlcpy(p, i2c_ascii_data[i]);
    *p = ' ';
    p++;
  }
  *(p-1) = '\0';

  debug_printf("## %s\n", reply);

  return strlen(reply);
}


int
uart_forward(unsigned char * params,
             unsigned char * reply,
             int conn_idx)
{
  unsigned char param[MAX_PARAM_LEN];
  if (get_next_param(param, params)) {
    char f = user_uart_forward_get();
    if (0 == f) {
      static unsigned char msg[] = "Zynq\n";
      return strlcpy(reply, msg);
    }
    if (1 == f) {
      static unsigned char msg[] = "None\n";
      return strlcpy(reply, msg);
    }
    if (2 == f) {
      static unsigned char msg[] = "Mezzanine 1\n";
      return strlcpy(reply, msg);
    }
    if (3 == f) {
      static unsigned char msg[] = "Mezzanine 2\n";
      return strlcpy(reply, msg);
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
      "  dest: none - disable forward.\n";
    return strlcpy(reply, msg);
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
      "Error: unknown destination.\n";
    return strlcpy(reply, msg);
  }
  
  return strlcpy(reply, ok_str);
}
