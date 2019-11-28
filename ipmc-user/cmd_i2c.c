#include <cmd_i2c.h>

#include <cmd_defs.h>

#include <user_cmd.h>
#include <user_i2c_msgs.h>
#include <user_helpers.h>

#include <debug.h>

static unsigned char i2c_bin_data[MAX_I2C_LEN];
static unsigned char i2c_ascii_data[MAX_I2C_LEN][10];

static const unsigned char help_i2c_write[] =
  "Low level I2C write operation with no target register.\n"
  "Usage: i2c_w <i2c_addr> <data>\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <data> multiple values to be written separated by spaces.";

static const unsigned char help_i2c_reg_write[] =
  "Low level I2C write operation with target register.\n"
  "Usage: i2c_reg_w <i2c_addr> <reg_addr> <data>\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <reg_addr> is the target register.\n"
  "  <data> multiple values to be written separated by spaces.";

static const unsigned char help_i2c_read[] =
  "Low level I2C read operation with no target register.\n"
  "Usage: i2c_r <i2c_addr> [nbytes]\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  [nbytes] number of bytes to be read, 1 if empty.";

static const unsigned char help_i2c_reg_read[] =
  "Low level I2C read operation with target register.\n"
  "Usage: i2c_reg_r <i2c_addr> <reg_addr> [nbytes]\n"
  "  <i2c_addr> is the target 7-bit I2C address.\n"
  "  <reg_addr> is the target register.\n"
  "  [nbytes] number of bytes to be read, 1 if empty.";

static const unsigned char help_set_i2c_bus[] =
  "Usage: set_i2c_bus <bus ID>\n"
  "  <bus ID>:\n"
  "    M for management bus \n"
  "    S for sensor bus";

static const unsigned char help_get_i2c_bus[] =
  "Usage: get_i2c_bus";

static const unsigned char err_i2c_bus[] =
  "Invalid I2C bus ID.";

static const unsigned char str_i2c_bus_management[] =
  "Management";

static const unsigned char str_i2c_bus_sensor[] =
  "Sensor";

static const char DEBUG = 0;

int
i2c_w(unsigned char * params,
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
    return strcpyl(reply, help_i2c_write);
  }


  // getting I2C address
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strcpyl(reply, err_i2c_addr);
  }

  char i2c_addr = (char) aux;

  // getting the data
  unsigned char i2c_data[MAX_I2C_LEN];
  int i2c_len = 0;
  while (get_next_param(param, params, ' ') == 0) {
    ret = i_from_a(&aux,
                   param,
                   &(cmd_buf[conn_idx].hex));
    if (ret != 0) {
      return strcpyl(reply, err_i2c_data);
    }

    i2c_data[i2c_len] = (unsigned char) aux;
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
  ret = user_i2c_write(i2c_addr,
                       i2c_data,
                       i2c_len,
                       cmd_buf[conn_idx].i2c_bus);
  if(ret != 0){
    return strcpyl(reply, err_i2c_write_transaction);
  }

  return strcpyl(reply, ok_str);
}

int
i2c_r(unsigned char * params,
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
    return strcpyl(reply, help_i2c_read);
  }

  if (DEBUG) {
    debug_printf("~~~~~~~~ %s\n", param);
  }
  
  // getting I2C address
  ret = i_from_a(&aux,
                 param,
                 &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strcpyl(reply, err_i2c_addr);
  }
  unsigned char i2c_addr = (unsigned char) aux;

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

  unsigned char i2c_raw_data[MAX_I2C_LEN];
  ret = user_i2c_read(i2c_addr,
                      i2c_raw_data,
                      i2c_len,
                      cmd_buf[conn_idx].i2c_bus);
  if(ret != 0){
    return strcpyl(reply, err_i2c_read_transaction);
  }


  int i;
  unsigned char i2c_data[MAX_I2C_LEN][6];
  for (i = 0; i < i2c_len; i++) {
    ret = a_from_i(&(*i2c_data[i]),
                   i2c_raw_data[i],
                   cmd_buf[conn_idx].hex);
    if(ret != 0){
      return strcpyl(reply, err_ia);
    }
  }

  unsigned char * p = reply;
  for (i = 0; i < i2c_len; i++) {
    p += strcpyl(p, i2c_data[i]);
    *p = ' ';
    p++;
  }
  *(p-1) = '\0';

  return strlenu(reply);
}


int
i2c_reg_w(unsigned char * params,
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
    return strcpyl(reply, help_i2c_reg_write);
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
  ret = user_i2c_reg_write(i2c_addr,
                           reg_addr,
                           i2c_bin_data,
                           i2c_len,
                           cmd_buf[conn_idx].i2c_bus);
  if(ret != 0){
    return strcpyl(reply, err_i2c_write_transaction);
  }

  return strcpyl(reply, ok_str);
}

int
i2c_reg_r(unsigned char * params,
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
    return strcpyl(reply, help_i2c_reg_read);
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
  unsigned char i2c_addr = (unsigned char) aux;

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

  ret = user_i2c_reg_read(i2c_addr,
                          reg_addr,
                          i2c_bin_data,
                          i2c_len,
                          cmd_buf[conn_idx].i2c_bus);
  if(ret != 0){
    return strcpyl(reply, err_i2c_read_transaction);
  }

  int i;
  for (i = 0; i < i2c_len; i++) {
    ret = a_from_i(&(*i2c_ascii_data[i]),
                   i2c_bin_data[i],
                   cmd_buf[conn_idx].hex);
    if(ret != 0){
      return strcpyl(reply, err_ia);
    }

    if (DEBUG) {
      debug_printf("## %s\n", i2c_ascii_data[i]);
    }
  }

  unsigned char * p = reply;
  for (i = 0; i < i2c_len; i++) {
    p += strcpyl(p, i2c_ascii_data[i]);
    *p = ' ';
    p++;
  }
  *(p-1) = '\0';

  if (DEBUG) {
    debug_printf("## %s\n", reply);
  }

  return strlenu(reply);
}

int
set_i2c_bus(unsigned char * params,
                unsigned char * reply,
                int conn_idx)
{
  char ret;
  
  unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params, ' ');
  if (ret != 0) {
    return strcpyl(reply, err_param);
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, help_set_i2c_bus);
  }

  if (DEBUG) {
    debug_printf("== %s\n", param);
  }

  if (param[0] == 'm') {
    cmd_buf[conn_idx].i2c_bus = MNGMNT_I2C_BUS;
    return strcpyl(reply, ok_str);
  }
  
  if (param[0] == 's') {
    cmd_buf[conn_idx].i2c_bus = SENSOR_I2C_BUS;
    return strcpyl(reply, ok_str);
  }
  
  return strcpyl(reply, err_i2c_bus);
}


int
get_i2c_bus(unsigned char * params,
            unsigned char * reply,
            int conn_idx)
{
  unsigned char param[MAX_PARAM_LEN];
  get_next_param(param, params, ' ');
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, help_get_i2c_bus);
  }

  if (DEBUG) {
    debug_printf("== %s\n", param);
  }

  if (cmd_buf[conn_idx].i2c_bus == 1) {
    return strcpyl(reply, str_i2c_bus_management);
  }

  if (cmd_buf[conn_idx].i2c_bus == 2) {
    return strcpyl(reply, str_i2c_bus_sensor);
  }

  return strcpyl(reply, err_i2c_bus);
}

