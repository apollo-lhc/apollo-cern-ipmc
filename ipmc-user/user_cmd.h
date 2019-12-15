#ifndef USER_CMD_H
#define USER_CMD_H

#include <cmd_defs.h>

/* ================================================================ */

void
lowercase(unsigned char s[]);

void
remove_extra_spaces(unsigned char s[]);

int
get_next_param(unsigned char * param
               , unsigned char * line
               , char delim);

int
get_cmd_index(const unsigned char * cmd);

int
append_to_cmd_buffer(const int conn_idx
                     , const unsigned char * data
                     , unsigned short len);
int
chomp_cmd(unsigned char * cmd_line
          , const int conn_idx);

unsigned char *
get_cmd_spare(const int conn_idx);

/* ================================================================ */

// structure to hold status of a connection
typedef struct cmd_buf_n {
  unsigned int len;
  unsigned int expert;
  unsigned int hex;
  // unsigned char eol[3];
  unsigned int i2c_bus;
  unsigned int i2c_mux_mask;
  unsigned char data[CMD_LINE_MAX_LEN];
  unsigned char spare[CMD_LINE_MAX_LEN];
} cmd_buf_t;

typedef struct cmd_map_n {
  const unsigned char * cmd;
  int (*fnc_ptr)(unsigned char * params
                 , unsigned char * reply
                 , const int conn_idx);
} cmd_map_t;

extern cmd_buf_t cmd_buf[MAX_USER_TCPSERV_CLIENT];
extern const cmd_map_t cmd_map[];

#endif // USER_CMD_H

