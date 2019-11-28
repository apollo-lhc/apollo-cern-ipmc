#include <cmd_zynq.h>

#include <user_zynq.h>
#include <user_helpers.h>
#include <user_cmd.h>
#include <user_i2c_msgs.h>

#include <cmd_defs.h>

#include <debug.h>

static unsigned char i2c_bin_data[MAX_I2C_LEN];
static unsigned char i2c_ascii_data[MAX_I2C_LEN][10];

static unsigned char DEBUG = 0;

static const unsigned char help_zynq_restart[] =
  "Restart only Zynq.\n"
  "Usage: zynq_restart [delay]\n"
  "  [delay]: integer, seconds, defaults to 2.";

static const unsigned char err_zynq_restart[] =
  "Zynq restart did not happen.";

static const unsigned char help_zynq_i2c_write[] =
  "Low level I2C write operation for Zynq registers.\n"
  "Usage: zynq_i2c_w <i2c_addr> <reg_addr> <data>\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <reg_addr> is the target register.\n"
  "  <data> multiple space separated integer values.";

static const unsigned char help_zynq_i2c_read[] =
  "Low level I2C read operation for Zynq registers.\n"
  "Usage: zynq_i2c_r <i2c_addr> <reg_addr> [nbytes]\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <reg_addr> is the target register.\n"
  "  [nbytes] number of bytes to be read, 1 if empty.";

int
zynq_restart(unsigned char * params,
           unsigned char * reply,
           int conn_idx)
{
  unsigned char param[MAX_PARAM_LEN];
  int delay = 2;

  char ret = get_next_param(param, params, ' ');

  if (ret == 0) {
    if (str_eq(param, help_str) == 1
        || str_eq(param, question_mark_str) == 1) {
      return strcpyl(reply, help_zynq_restart);
    }

    unsigned char tmp; 
    ret = i_from_a (&delay,
                    param,
                    &tmp);
    if (ret != 0) {
      return strcpyl(reply, err_param);
    }
  }

  
  if (user_zynq_request_restart((char) delay) == 0) { 
    return strcpyl(reply, ok_str);
  }
  
  return strcpyl(reply, err_zynq_restart);
}


int
zynq_i2c_w(unsigned char * params,
           unsigned char * reply,
           int conn_idx)
{
  char ret;
  int aux;
  
  unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params, ' ');
  if (ret != 0) {
    return strcpyl(reply, err_param);
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, help_zynq_i2c_write);
  }

  // getting I2C address
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strcpyl(reply, err_i2c_addr);
  }
  char i2c_addr = (char) aux;

  // getting register address
  ret = get_next_param(param, params, ' ');
  if (ret != 0) {
    return strcpyl(reply, err_i2c_reg_addr);
  }
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strcpyl(reply, err_i2c_reg_addr);
  }
  unsigned char reg_addr = (char) aux;

  // getting the data
  int i2c_len = 0;
  while (get_next_param(param, params, ' ') == 0) {
    ret = i_from_a(&aux,
                   param,
                   &(cmd_buf[conn_idx].hex));
    if (ret != 0) {
      return strcpyl(reply, err_i2c_data);
    }

    i2c_bin_data[i2c_len] = (char) aux;
    i2c_len++;
  }

  // making sure we have something to send to the target...
  if (i2c_len == 0) {
    return strcpyl(reply, err_i2c_data);
  }

  // but it can not be too much...
  if (i2c_len > MAX_I2C_LEN) {
    return strcpyl(reply, err_i2c_len);
  }

  // writing to the I2C address
  ret = user_zynq_i2c_write(i2c_addr,
                            reg_addr,
                            i2c_bin_data,
                            i2c_len);
  if(ret != 0){
    return strcpyl(reply, err_i2c_write_transaction);
  }

  return strcpyl(reply, ok_str);
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
  ret = get_next_param(param, params, ' ');
  if (ret != 0) {
    return strcpyl(reply, err_param);
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, help_zynq_i2c_read);
  }

  if (DEBUG) {
    debug_printf("== %s\n", param);
  }
  
  // getting I2C address
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strcpyl(reply, err_i2c_addr);
  }
  unsigned char i2c_addr = (char) aux;

  // getting register address
  ret = get_next_param(param, params, ' ');
  if (ret != 0) {
    return strcpyl(reply, err_i2c_reg_addr);
  }
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strcpyl(reply, err_i2c_reg_addr);
  }
  unsigned char reg_addr = (char) aux;

  // getting the length of the reading; default to 1.
  int i2c_len = 1;
  if (get_next_param(param, params, ' ') == 0) {
    // lets fake the hex parsing
    unsigned char tmp; 
    ret = i_from_a (&i2c_len,
                    param,
                    &tmp);
    if (ret != 0 || i2c_len > MAX_I2C_LEN) {
      return strcpyl(reply, err_i2c_len);
    }
  }

  ret = user_zynq_i2c_read(i2c_addr,
                           reg_addr,
                           i2c_bin_data,
                           i2c_len);
  if(ret != 0){
    reply_len = strlenu(err_i2c_read_transaction);
    memcpy(reply, err_i2c_read_transaction, reply_len);
    return reply_len;
  }

  int i;
  for (i = 0; i < i2c_len; i++) {
    ret = a_from_i(&(*i2c_ascii_data[i]),
                   i2c_bin_data[i],
                   cmd_buf[conn_idx].hex);
    if(ret != 0){
      return strcpyl(reply, err_ia);
    }

    debug_printf("## %s\n", i2c_ascii_data[i]);
  }

  unsigned char * p = reply;
  for (i = 0; i < i2c_len; i++) {
    debug_printf("zynq i2c data vector [%d]: %s\n", i, i2c_ascii_data[i]);
    p += strcpyl(p, i2c_ascii_data[i]);
    *p = ' ';
    p++;
  }
  *(p-1) = '\0';

  debug_printf("## %s\n", reply);

  return strlenu(reply);
}
