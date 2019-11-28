#include <cmd_pca9545.h>

#include <user_pca9545.h>

#include <user_helpers.h>
#include <user_i2c_msgs.h>
#include <user_cmd.h>

#include <cmd_defs.h>

static const unsigned char i2c_mux_write_help_str[] =
  "Usage: write_i2c_mux <mask>\n"
  "mask: 4-bit integer, one for each lane.\n"
  "Ex: \"write_i2c_mux 3\" enables two lanes.";

static const unsigned char i2c_mux_read_help_str[] =
  "Usage: read_i2c_mux";

static const unsigned char i2c_mux_error_str[] =
  "I2C mux operation error.";


int
write_i2c_mux(unsigned char * params,
              unsigned char * reply,
              int conn_idx)
{
  char ret;
  int aux;
  unsigned char param[MAX_PARAM_LEN];

  if (get_next_param(param, params, ' ')) {
    return strcpyl(reply, err_param);
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, i2c_mux_write_help_str);
  }

  // getting I2C address
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strcpyl(reply, err_i2c_addr);
  }

  unsigned char mask = (unsigned char) aux;

  if(user_pca9545_write(mask)) {
    return strcpyl(reply, i2c_mux_error_str);
  }
  
  return strcpyl(reply, ok_str);
}


int
read_i2c_mux(unsigned char * params,
             unsigned char * reply,
             int conn_idx)
{
  unsigned char param[MAX_PARAM_LEN];

  get_next_param(param, params, ' ');
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, i2c_mux_read_help_str);
  }

  unsigned char value;
  if (user_pca9545_read(&value)) {
    return strcpyl(reply, i2c_mux_error_str);
  }
  
  if (a_from_i(reply, value, cmd_buf[conn_idx].hex)) {
    return strcpyl(reply, err_ia);
  }

  int len = strlenu(reply);
  reply[len] = '\0';
  return len;
}
