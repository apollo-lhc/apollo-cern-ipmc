#ifndef __USER_TCPSERV_H__
#define __USER_TCPSERV_H__

#include <net/ip.h>
#include <net/tcp.h>

/* Structure used to save client information */
typedef struct{
	ip_addr_t to;
	unsigned short to_port;
	unsigned char opened;
}user_tcpserv_users_t;

/* Callback handlers */
char
user_tcpserv_data_handler(const ip_addr_t to,
                          unsigned short to_port,
                          const ip_addr_t from,
                          unsigned short from_port,
                          const unsigned char *data,
                          unsigned short len,
                          unsigned char *reply,
                          unsigned *replyLen);

char
user_tcpserv_connect_handler(const ip_addr_t from,
                             unsigned short from_port);

char
user_tcpserv_disconnect_handler(const ip_addr_t from,
                                unsigned short from_port);

#endif
