#include <cmd_pca9545.h>

#include <user_pca9545.h>

#include <user_helpers.h>
#include <user_i2c_msgs.h>
#include <user_cmd.h>

#include <cmd_defs.h>

int
write_i2c_mux(unsigned char * params,
              unsigned char * reply,
              const int conn_idx)
{

  static const unsigned char i2c_mux_write_help_str[] =
    "Usage: write_i2c_mux <mask>\n"
    "mask: 4-bit integer, one for each lane.\n"
    "Ex: \"write_i2c_mux 3\" enables two lanes.";

  int ret;
  int aux;
  static unsigned char param[MAX_PARAM_LEN];

  if (get_next_param(param, params, ' ')) {
    return strcpyl(reply, err_param);
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, i2c_mux_write_help_str);
  }

  // getting mask
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strcpyl(reply, err_ia);
  }

  cmd_buf[conn_idx].i2c_mux_mask = (unsigned char) aux;  
  return strcpyl(reply, ok_str);
}


int
read_i2c_mux(unsigned char * params,
             unsigned char * reply,
             const int conn_idx)
{

  static const unsigned char i2c_mux_read_help_str[] =
    "Usage: read_i2c_mux";

  int ret;
  static unsigned char param[MAX_PARAM_LEN];

  get_next_param(param, params, ' ');
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, i2c_mux_read_help_str);
  }

  ret = a_from_i(reply, cmd_buf[conn_idx].i2c_mux_mask, cmd_buf[conn_idx].hex);
  if (ret) {
    return strcpyl(reply, err_ia);
  }
  
  return strlenu(reply);
}
