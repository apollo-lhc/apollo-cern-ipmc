#include <cmd_pim400kz.h>

#include <user_pim400kz.h>
#include <user_i2c_msgs.h>
#include <user_cmd.h>
#include <user_helpers.h>

#include <debug.h>


static const unsigned char pim400kz_err_i2c[] =
  "ERROR: I2C communication failed.";

int
pim400kz_get_status(unsigned char * params,
                    unsigned char * reply,
                    const int conn_idx)
{
  int ret;

  static const unsigned char help_msg[] =
    "Usage: pim400kz_get_status";

  static unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params, ' ');

  if(ret != 0) {
    uint8_t status;
    uint8_t ret = user_pim400kz_get_status(&status);
    if(ret){
      return strcpyl(reply, pim400kz_err_i2c);
    }

    unsigned char *r = reply;

    a_from_i(param, (int) status, 1);
    r += strcpyl(r, param);

    return r-reply;
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, help_msg);
  }

  return strcpyl(reply, err_param);
}

int
pim400kz_set_status(unsigned char * params,
                    unsigned char * reply,
                    const int conn_idx)
{
  int ret;

  static const unsigned char help_msg[] =
    "Usage: pim400kz_set_status <8-bit value>";

  static unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params, ' ');

  if(ret != 0) {
    return strcpyl(reply, err_param);
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, help_msg);
  }


  int aux;
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strcpyl(reply, err_ia);
  }
  uint8_t status = (uint8_t) aux;

  ret = user_pim400kz_set_status(&status);
  if(ret){
    return strcpyl(reply, pim400kz_err_i2c);
  }

  return strcpyl(reply, ok_str);
}

int
pim400kz_get_holdup_voltage(unsigned char * params,
                            unsigned char * reply,
                            const int conn_idx)
{
  int ret;

  static const unsigned char help_msg[] =
    "Usage: pim400kz_get_holdup_voltage";

  static unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params, ' ');

  if(ret != 0) {
    uint8_t v_holdup;
    uint8_t ret = user_pim400kz_get_holdup_voltage(&v_holdup);
    if(ret){
      return strcpyl(reply, pim400kz_err_i2c);
    }

    unsigned char *r = reply;

    float v = v_holdup * 0.398;
    int left = (int) v;
    int right = (int)((v - left)*100);

    a_from_i(param, left, 0);
    r += strcpyl(r, param);
    r += strcpyl(r, (unsigned char *) ".");
    a_from_i(param, right, 0);
    r += strcpyl(r, param);

    static const unsigned char suffix[] = " V";
    r += strcpyl(r, suffix);
    return r-reply;
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, help_msg);
  }

  return strcpyl(reply, err_param);
}

int
pim400kz_get_current(unsigned char * params,
                     unsigned char * reply,
                     const int conn_idx)
{
  int ret;

  static const unsigned char help_msg[] =
    "Usage: pim400kz_get_current";

  static unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params, ' ');

  if(ret != 0) {
    uint8_t current;
    uint8_t ret = user_pim400kz_get_current(&current);
    if(ret){
      return strcpyl(reply, pim400kz_err_i2c);
    }

    unsigned char *r = reply;

    a_from_i(param, (int) current * 94, 0);
    r += strcpyl(r, param);

    static const unsigned char suffix[] = " mA";
    r += strcpyl(r, suffix);
    return r-reply;
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, help_msg);
  }

  return strcpyl(reply, err_param);
}

int
pim400kz_get_af_voltage(unsigned char * params,
                        unsigned char * reply,
                        const int conn_idx)
{
  int ret;

  static const unsigned char help_msg[] =
    "Usage: pim400kz_get_af_voltage";

  static unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params, ' ');

  if(ret != 0) {
    uint8_t v_af;
    uint8_t ret = user_pim400kz_get_af_voltage(&v_af);
    if(ret){
      return strcpyl(reply, pim400kz_err_i2c);
    }

    unsigned char *r = reply;

    float v = v_af * 0.325;
    int left = (int) v;
    int right = (int)((v - left)*100);

    a_from_i(param, left, 0);
    r += strcpyl(r, param);
    r += strcpyl(r, (unsigned char *) ".");
    a_from_i(param, right, 0);
    r += strcpyl(r, param);

    static const unsigned char suffix[] = " V";
    r += strcpyl(r, suffix);
    
    return r-reply;
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, help_msg);
  }

  return strcpyl(reply, err_param);
}

int
pim400kz_get_bf_voltage(unsigned char * params,
                        unsigned char * reply,
                        const int conn_idx)
{
  int ret;

  static const unsigned char help_msg[] =
    "Usage: pim400kz_get_bf_voltage";

  static unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params, ' ');

  if(ret != 0) {
    uint8_t v_bf;
    uint8_t ret = user_pim400kz_get_bf_voltage(&v_bf);
    if(ret){
      return strcpyl(reply, pim400kz_err_i2c);
    }

    unsigned char *r = reply;

    float v = v_bf * 0.325;
    int left = (int) v;
    int right = (int)((v - left)*100);

    a_from_i(param, left, 0);
    r += strcpyl(r, param);
    r += strcpyl(r, (unsigned char *) ".");
    a_from_i(param, right, 0);
    r += strcpyl(r, param);

    static const unsigned char suffix[] = " V";
    r += strcpyl(r, suffix);
    
    return r-reply;
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, help_msg);
  }

  return strcpyl(reply, err_param);
}

int
pim400kz_get_temp(unsigned char * params,
                  unsigned char * reply,
                  const int conn_idx)
{
  int ret;

  static const unsigned char help_msg[] =
    "Usage: pim400kz_get_temp";

  static unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params, ' ');

  if(ret != 0) {
    uint8_t temp;
    uint8_t ret = user_pim400kz_get_temp(&temp);
    if(ret){
      return strcpyl(reply, pim400kz_err_i2c);
    }

    unsigned char *r = reply;

    float t = temp * 1.961 - 50;
    int left = (int) t;
    int right = (int)((t - left) * 100);

    a_from_i(param, left, 0);
    r += strcpyl(r, param);
    r += strcpyl(r, (unsigned char *) ".");
    a_from_i(param, right, 0);
    r += strcpyl(r, param);

    static const unsigned char suffix[] = " oC";
    r += strcpyl(r, suffix);

    return r-reply;
  }

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, help_msg);
  }

  return strcpyl(reply, err_param);
}
