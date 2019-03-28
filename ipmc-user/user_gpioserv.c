
#include <app.h>
#include <cfgint.h>
#include <ipmc.h>
#include <log.h>
#include <debug.h>

#include <net/tcp.h>
#include <net/ip.h>

#include <app/signal.h>

#define MAX_USER_TCPSERV_CLIENT 10

/* Structure used to save client information */
typedef struct{
	ip_addr_t to;
	unsigned short to_port;
	unsigned char opened;
}user_tcpserv_users_t;

/* Callback handlers */
static char data_handler(const ip_addr_t to, unsigned short to_port,const ip_addr_t from, unsigned short from_port, const unsigned char *data, unsigned short len, unsigned char *reply, unsigned *replyLen);
static char connect_handler(const ip_addr_t from, unsigned short from_port);
static char disconnect_handler(const ip_addr_t from, unsigned short from_port);


/* ================================================================ */

/* Connection slot array used to manage the client */
static user_tcpserv_users_t clients[MAX_USER_TCPSERV_CLIENT];

/* INIT_CALLBACK: called at initialisation of the IPMC */
INIT_CALLBACK(user_gpioserv_init)
{
    tcp_connect_server(connect_handler, disconnect_handler, data_handler, 5000);
}

/* TIMER_CALLBACK: called every 1s */
TIMER_CALLBACK(1s, user_gpioserv_timercback)
{
	unsigned i;

	unsigned char *data = (unsigned char *)"alive\n";
	unsigned len = 6;
	
	/* Scan the connection slot to send data to all of the clients */
    for(i=0; i < MAX_USER_TCPSERV_CLIENT; i++){
		user_tcpserv_users_t *client = &clients[i];
		
		/* Check if the slot is in use */
		if(client->opened){
			
			/* Send "len" bytes of "data" */
			tcp_send_packet(client->to, client->to_port, data, len);
			
		}
		
	}
}

/* 
 * Name: user_tcpserv_connect_handler
 *
 * Parameters:
 *		- from: client IP address
 *		- from_port: client tcp port
 *
 * Description: Called when a client requests a connection. The prototype of this function is defined by the TCP/IP library
 *              and cannot be changed.
 */
char connect_handler(const ip_addr_t from, unsigned short from_port){
	unsigned i;

	/* Display information in debug console */
	debug_printf("<_> user_tcpserv: " "New connection request\n");

	/* Save client information in a list */
	for(i=0; i < MAX_USER_TCPSERV_CLIENT; i++){							/* Scan the array to find an empty slot */
		user_tcpserv_users_t *client = &clients[i];
		if(client->opened == 0){							/* Check if current position is already used */
			client->to_port = from_port;					/* */
			memcpy(client->to, from, sizeof(from));			/* If not, save the client info */
			client->opened = 1;								/* */

			return 0;														/* Return successfully */
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
 * Description: Called when a client sends a disconnect request. The prototype of this function is defined by the TCP/IP library
 *              and cannot be changed.
 */
char disconnect_handler(const ip_addr_t from, unsigned short from_port){
	unsigned i;

	/* Display information in debug console */
	debug_printf("<_> user_tcpserv: " "Disconnect request received\n");

	/* Search for the client slot in the array */
	for(i=0; i < MAX_USER_TCPSERV_CLIENT; i++){
		user_tcpserv_users_t *client = &clients[i];
		
		/* Check the information */
		if(client->opened == 1 && !memcmp(from, client->to, sizeof(from)) && from_port == client->to_port){
			
			/* Remove the information and set the opened variable to 0 (slot not used) */
			client->to_port = 0;
			memset(client->to, 0, 4);
			client->opened = 0;
			
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
 * Description: Called when a client sends data. The prototype of this function is defined by the TCP/IP library
 *              and cannot be changed.
 */
char data_handler(const ip_addr_t to, unsigned short to_port,const ip_addr_t from, unsigned short from_port, const unsigned char *data, unsigned short len, unsigned char *reply, unsigned *replyLen){

	if (len>=1) {
		signal_t userio_sig = USER_IO_20;
		if (data[0]=='1') signal_activate(&userio_sig);
		if (data[0]=='0') signal_deactivate(&userio_sig);
	}
	
	/* Set the reply length */
	*replyLen = 0;

	/* Return the function successfully */
	return 0;
}
