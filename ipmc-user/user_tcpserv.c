/***********************************************************************
Nom ......... : user_tcpserv.c
Role ........ : TCP/IP server for L0MDT project
Auteur ...... : Thiago Costa de Paiva <tcpaiva@cern.ch>
Version ..... : V0.2 - 2019-07-31
***********************************************************************/

#include <user_tcpserv.h>

#include <app.h>
#include <cfgint.h>
#include <ipmc.h>
#include <log.h>
#include <debug.h>

#include <net/tcp.h>
#include <net/ip.h>

#include <user_i2c.h>
#include <user_expert.h>
// #include <user_zynq.h>
// #include <user_pca9545.h>
// #include <user_tcn75.h>
// #include <user_gpio.h>
// #include <user_version.h>
// #include <user_uart.h>


#include <user_helpers.h>

#include <user_cmd.h>

static const unsigned char DEBUG = 0;

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

static const unsigned char prompt_str[] = "\n:: ";

static const unsigned char echo_str[] =
  "Command not recognized. Echoing:\n";


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
      memcpy_(user_tcpserv_clients[i].to, from, sizeof(from));
      
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


  if (DEBUG) {
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

  if (DEBUG) {
    debug_printf("<_> ======= 2\n");
  }
  
  // append received data to associated buffer
  // check for overflow
  if (append_to_cmd_buffer(conn_idx, data, len) != 0){
    return 0;
  }

  unsigned char cmd_line[CMD_LINE_MAX_LEN];

  if (DEBUG) {
    debug_printf("<_> ======= 3\n");
  }
  
  // look for termination
  // copy from buffer to cmd line;
  // terminates cmd line with '\0' for str_eq
  // shift remaining of the buffer to the start
  // returns the len of the command found, or -1 otherwise
  int cmd_len = chomp_cmd(cmd_line, conn_idx);
  
  if (DEBUG) {
    debug_printf("<_> ======= 4\n");
  }

  if (cmd_len < 0) {
    *replyLen = 0;
    return 0;
  }
  
  
  if (DEBUG) {
    debug_printf("<_> ======= cmd line: %s|\n", cmd_line);
  }
    

  remove_extra_spaces(cmd_line);
  if (DEBUG) {
    debug_printf("<_> >>>>>> no extra spaces: %s|\n", cmd_line);
  }

  lowercase(cmd_line);
  if (DEBUG) {
    debug_printf("<_> >>>>>> all lowercase: %s|\n", cmd_line);
  }

  unsigned char cmd[MAX_PARAM_LEN];
  get_next_param(cmd, cmd_line, ' ');
  if (DEBUG) {
    debug_printf("<_> >>>>>> cmd: %s|\n", cmd);
  }

  int cmd_idx = get_cmd_index(cmd);
  if (DEBUG) {
    debug_printf("<_> >>>>>> cmd_idx: %d\n", cmd_idx);
  }
  
  // if a command was found, execute it.
  if (cmd_idx >= 0) { 
    // execute command, get reply and associated length
    *replyLen = cmd_map[cmd_idx].fnc_ptr(cmd_line, reply, conn_idx);
    // debug_printf("<_> >>>>>> cmd reply len: %d\n", *replyLen);
    
  } else {   // if a command was not found, echo it.

    unsigned char * r = reply;
    r += strcpyl(r, echo_str);
    r += strcpyl(r, get_cmd_spare(conn_idx));
    *replyLen = r - reply;
    // debug_printf("_+_+_+_ *replyLen: %d; len(reply): %d\n", *replyLen, strlenu(reply));
  }

  replyLen += strcpyl(&reply[*replyLen]
                      , user_expert_mode_get_state_msg());
  reply[*replyLen] = '\0';
  
  *replyLen += strcpyl(&reply[*replyLen], prompt_str);
  reply[*replyLen] = '\0';
  
  // debug_printf("----- final reply len: %d\n", *replyLen);

  // debug_printf("<_> user_tcpserv response: ");
  // debug_printf((char *) reply);

  /* Return the function successfully */
  return 0;
}

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

