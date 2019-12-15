#ifndef CMD_H
#define CMD_H

#define MAX_PARAM_LEN (20)
#define MAX_USER_TCPSERV_CLIENT (10)
#define CMD_LINE_MAX_LEN (4*16)
#define MAX_I2C_LEN (16)


extern const unsigned char error_str[];

extern const unsigned char help_str[];

extern const unsigned char question_mark_str[];

extern const unsigned char err_param[];

extern const unsigned char err_ia[];

extern const unsigned char ok_str[];

extern const unsigned char cmds_header_str[];
extern const unsigned char signals_header_str[];
extern const unsigned char help_footer_str[];

#endif // CMD_H

