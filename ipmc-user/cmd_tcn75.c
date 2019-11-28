#include <cmd_tcn75.h>

#include <user_tcn75.h>
#include <user_i2c_msgs.h>
#include <user_helpers.h>
#include <user_cmd.h>

#include <cmd_defs.h>

static const unsigned char tcn75a_help_str[] =
  "Usage: read_tcn75a <sensor UID>\n"
  "  UID is U34, U35 or U36.";


int
read_tcn75a(unsigned char * params,
            unsigned char * reply,
            int conn_idx)
{
  int reply_len = 0;
  char ret;
  unsigned char param[MAX_PARAM_LEN];

  get_next_param(param, params, ' ');

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, tcn75a_help_str);
  }

  unsigned char temp;
  int id;

  i_from_a(&id,
           param,
           &(cmd_buf[conn_idx].hex));

  ret = user_tcn75_read_reg((unsigned char) id, 0x00, &temp);
  if (ret != 0) {
    return strcpyl(reply, error_str);
  }

  
  ret = a_from_i(reply, temp, 0);
  if(ret != 0){
    return strcpyl(reply, err_ia);
  }

  reply_len = strlenu(reply);
  reply[reply_len++]='\n';
  reply[reply_len]='\0';    
  return reply_len;
}

