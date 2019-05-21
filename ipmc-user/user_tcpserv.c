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

// it seems stdio cannot be used... 
// #include <stdio.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

/* ================================================================ */

// char *
// strcpy(char *str_dest, const char *str_src);

size_t
strlen(const char * str);

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

int
write_gpio_signal(char * params, unsigned char * reply);

int
read_gpio_signal(char * params, unsigned char * reply);

/* ================================================================ */

typedef struct pin_map_n {
  const char *sm_name;
  const int writable;
  const signal_t ipmc_name;
  const int initial;
} pin_map_t;

pin_map_t pin_map[] = {
  {"ipmc_zynq_en"     , 1, USER_IO_3 , 0}, 
  {"en_one_jtag_chain", 1, USER_IO_4 , 0}, 
  {"uart_addr0"       , 1, USER_IO_5 , 0}, 
  {"uart_addr1"       , 1, USER_IO_6 , 0}, 
  {"zynq_boot_mode0"  , 1, USER_IO_7 , 1}, 
  {"zynq_boot_mode1"  , 1, USER_IO_8 , 1}, 
  {"sense_rst"        , 1, USER_IO_9 , 0}, 
  {"mezz2_en"         , 1, USER_IO_10, 0}, 
  {"mezz1_en"         , 1, USER_IO_11, 0}, 
  {"m24512_we_n"      , 1, USER_IO_12, 1}, 
  {"eth_sw_pwr_good"  , 0, USER_IO_13, 0}, 
  {"eth_sw_reset_n"   , 1, USER_IO_16, 1},
  {NULL, 0, USER_IO_16, 0}
};

typedef struct cmd_map_n {
  const char * cmd;
  const int (*fnc_ptr)(const char *, unsigned char *);
} cmd_map_t;

cmd_map_t cmd_map[] = {
  {"set_gpio", & write_gpio_signal},
  {"get_gpio", & read_gpio_signal},
  {NULL, NULL}
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
  debug_printf("<_> user_tcpserv: " "New connection request\n");
  
  /* Save client information in a list */

  /* Scan the array to find an empty slot */
  for(i=0; i < MAX_USER_TCPSERV_CLIENT; i++){

    /* Check if current position is already used */
    if(user_tcpserv_clients[i].opened == 0){
      
      user_tcpserv_clients[i].to_port = from_port;

      /* If not, save the client info */
      memcpy(user_tcpserv_clients[i].to, from, sizeof(from));
      
      user_tcpserv_clients[i].opened = 1;

      /* Return successfully */
      return 0; 
    }
  }

  /* Print a warning message when no slot is available */
  debug_printf("<W> user_tcpserv: " "No user_tcpserv connection slot available\n");
	
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
user_tcpserv_disconnect_handler(const ip_addr_t from, unsigned short from_port)
{
  unsigned i;
  
  /* Display information in debug console */
  debug_printf("<_> user_tcpserv: " "Disconnect request received\n");
  
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

  debug_printf("<_> user_tcpserv command line: ");
  debug_printf(cmd_line);
  debug_printf("\n");

  remove_extra_spaces(cmd_line);
  debug_printf("<_> >>>>>> extra spaces: ");
  debug_printf(cmd_line);
  debug_printf("\n");

  lowercase(cmd_line);
  debug_printf("<_> >>>>>> lowercase: ");
  debug_printf(cmd_line);
  debug_printf("\n");


  char cmd[30];
  get_next_param(cmd, cmd_line);
  debug_printf("<_> >>>>>> cmd: ");
  debug_printf(cmd);
  debug_printf("\n");

  int cmd_idx = get_cmd_index(cmd);
  debug_printf("<_> >>>>>> cmd_idx: ");
  debug_printf("%d", cmd_idx);
  debug_printf("\n");
 
  if (cmd_idx >= 0) { 
    // execute command, get reply and associated length
    *replyLen = cmd_map[cmd_idx].fnc_ptr(cmd_line, reply);
  }

  else {
    /* Clone the request in the reply buffer */
    memcpy(reply, data, len);
    reply[len]= '\0';
  
    /* Set the reply length */
    *replyLen = len;
  }

  debug_printf("<_> user_tcpserv response: ");
  debug_printf((const char *) reply);

  /* Return the function successfully */
  return 0;
}


/* ================================================================ */

int
write_gpio_signal(char * params,
                  unsigned char * reply)
{
  debug_printf("<_> ======= write_gpio_signal\n");

  char sm_signal_name[30];
  get_next_param(sm_signal_name, params);

  int msg_len;
  
  int idx = get_signal_index(sm_signal_name);
  
  if (idx >= 0) {
  
    signal_t userio_sig = pin_map[idx].ipmc_name;

    if (pin_map[idx].writable == 1) {

      char value[2];
      get_next_param(value, params);

      debug_printf("<_> ======= value: ");
      debug_printf(value);
      debug_printf("\n");
      
      if (value[0] == '1' || value[0] == 'h'){
        signal_deactivate(&userio_sig);
        debug_printf("<_> ======= signal deactivated\n");
      }
      else if (value[0] == '0' || value[0] == 'h') {
        signal_activate(&userio_sig);
        debug_printf("<_> ======= signal activated\n");
      }

      char msg[] = "OK\n";
      msg_len = strlen(msg);
      memcpy(reply, msg, msg_len);
    }

    else {
      char msg[] = "Cannot write to input signal.\n";
      msg_len = strlen(msg);
      memcpy(reply, msg, msg_len);
    }

  }

  else {
    char msg[] = "Signal not found.\n";
    msg_len = strlen(msg);
    memcpy(reply, msg, msg_len);
  }

  return msg_len;
}

// read pin and fill reply string with associated value. returns the
// size of the reply.
int
read_gpio_signal(char * params,
                 unsigned char * reply)
{

  debug_printf("<_> ======= read_gpio_signal\n");

  char sm_signal_name[30];
  get_next_param(sm_signal_name, params);

  debug_printf("<_> ======= ");
  debug_printf(sm_signal_name);
  debug_printf("\n");

  int idx = get_signal_index(sm_signal_name);
  
  int v;
  int reply_len = 0;
  
  if (idx >= 0) {
  
    signal_t userio_sig = pin_map[idx].ipmc_name;
  
    v = signal_read(&userio_sig);
  
    if (v == 0) {
      char msg[] = "1\n";
      reply_len = strlen(msg);
      memcpy(reply, msg, reply_len);
    }
    else if (v == 1) {
      char msg[] = "0\n";
      reply_len = strlen(msg);
      memcpy(reply, msg, reply_len);
    }
    else{
      char msg[] = "Unexpected value read from signal.\n";
      reply_len = strlen(msg);
      memcpy(reply, msg, reply_len);
    }
  }

  else {
    char msg[] = "Signal not found.\n";
    reply_len = strlen(msg);
    memcpy(reply, msg, reply_len);
  }

  return reply_len;
}

/* ================================================================ */

int
str_eq (const char *s1,
        const char *s2)
{
  const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;

  while (*p1 != '\0') {
    if (*p1 != * p2){
      return 0;
    }
    p1++;
    p2++;
  }

  if (*p2 != '\0') {
    return 0;
  }

  return 1;
}

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


// freebsd implementation of strlen
size_t
strlen(const char * str)
{
    const char *s;
    for (s = str; *s; ++s) {}
    return(s - str);
}

/* ================================================================ */

// look for signal information in the pin map table and return its
// position. -1 is returned in case no signal is found.
int
get_signal_index(const char * sm_signal_name)
{
  int i = 0;
  for (i = 0; pin_map[i].sm_name != NULL; i++) {

    debug_printf("<_> ++++++++ signal_ids: ");
    debug_printf(pin_map[i].sm_name);
    debug_printf(" ");
    debug_printf(sm_signal_name);
    debug_printf("\n");
    
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
  for (i = 0; cmd_map[i].cmd != NULL; i++) {

    debug_printf("<_> ++++++++ cmd_ids: ");
    debug_printf(cmd_map[i].cmd);
    debug_printf(" ");
    debug_printf(cmd);
    debug_printf("\n");

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


