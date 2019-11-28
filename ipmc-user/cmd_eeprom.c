#include <cmd_eeprom.h>

#include <user_eeprom.h>
#include <user_i2c_msgs.h>
#include <user_cmd.h>
#include <user_helpers.h>

int
eeprom_wren(unsigned char * params,
              unsigned char * reply,
              int conn_idx)
{
  char ret;

  static const unsigned char eeprom_wren_help[] =
    "Usage: eeprom_wren [level]\n"
    "[level]:\n"
    "  <empty>: retrieves status of the EEPROM write enable\n"
    "  0 or off: disable EEPROM writing capabilities\n"
    "  1 or on: enable EEPROM writing capabilities";

  static const unsigned char wr_enabled[] = "1";
  static const unsigned char wr_disabled[] = "0";

  unsigned char param[MAX_PARAM_LEN];
  ret = get_next_param(param, params, ' ');

  if (ret != 0) {
    uint8_t lvl;
    user_eeprom_get_wren(&lvl);
    if (lvl == 0) {
      return strcpyl(reply, wr_disabled);
    } else if (lvl == 1) {
      return strcpyl(reply, wr_enabled);
    }
  }
    
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, eeprom_wren_help);
  }
  
  if (str_eq(param, (unsigned char *) "1") == 1
      || str_eq(param, (unsigned char *) "on") == 1) {
    user_eeprom_write_enable();
    return strcpyl(reply, ok_str);
  }

  if (str_eq(param, (unsigned char *) "0") == 1
      || str_eq(param, (unsigned char *)"off") == 1) {
    user_eeprom_write_disable();
    return strcpyl(reply, ok_str);
  }

  return strcpyl(reply, err_param);
}

int
eeprom_save(unsigned char * params,
            unsigned char * reply,
            int conn_idx)
{
  unsigned char param[MAX_PARAM_LEN];
  char ret;
  
  static const unsigned char help[] =
    "Usage: eeprom_save";

  static const unsigned char err_wren[] =
    "Write mode disabled";

  static const unsigned char err_size[] =
    "EEPROM format is too big!";

  ret = get_next_param(param, params, ' ');
  if (ret == 0) {
    if (str_eq(param, help_str) == 1
        || str_eq(param, question_mark_str) == 1) {
      return strcpyl(reply, help);
    }
  }

  ret = user_eeprom_write();
  if (ret == -1) {
    return strcpyl(reply, err_wren);
  } else if (ret == -2){
    return strcpyl(reply, err_size);
  } else {
    return strcpyl(reply, ok_str);
  }
}

int
eeprom_read(unsigned char * params,
            unsigned char * reply,
            int conn_idx)
{
  unsigned char param[MAX_PARAM_LEN];

  static const unsigned char help[] =
    "Usage: eeprom_read";

  char ret = get_next_param(param, params, ' ');
  if (ret == 0) {
    if (str_eq(param, help_str) == 1
        || str_eq(param, question_mark_str) == 1) {
      return strcpyl(reply, help);
    }
  }

  ret = user_eeprom_read();

  if (ret == -1) {
    return strcpyl(reply, err_i2c_write_transaction);
  }
  
  if (ret == -2) {
    return strcpyl(reply, err_i2c_read_transaction);
  }
  
  return strcpyl(reply, ok_str);
}


int
eeprom_get_version(unsigned char * params,
                   unsigned char * reply,
                   int conn_idx)
{
  char ret;

  static const unsigned char get_version_help[] =
    "Usage: eeprom_get_version";

  unsigned char param[MAX_PARAM_LEN];
  
  ret = get_next_param(param, params, ' ');
  if (ret == 0) {
    if (str_eq(param, help_str) == 1
        || str_eq(param, question_mark_str) == 1) {
      return strcpyl(reply, get_version_help);
    }
  }

  uint8_t v;
  user_eeprom_get_version(&v);
  
  ret = a_from_i (param
                  , (int) v
                  , cmd_buf[conn_idx].hex);
  if (ret != 0) {
    return strcpyl(reply, err_ia);
  }
  
  return strcpyl(reply, param);
}

int
eeprom_set_version(unsigned char * params,
                   unsigned char * reply,
                   int conn_idx)
{
  static const unsigned char set_version_help[] =
    "Usage: eeprom_set_version <version>\n"
    " --> Saving to EEPROM is required.";

  unsigned char param[MAX_PARAM_LEN];
  char ret;
  int aux;

  ret = get_next_param(param, params, ' ');
  if (ret != 0) {
    return strcpyl(reply, err_param);
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, set_version_help);
  }

  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strcpyl(reply, err_ia);
  }
  char version = (char) aux;
  user_eeprom_set_serial_number(version);
  return strcpyl(reply, ok_str);
}

int
eeprom_set_sn(unsigned char * params,
              unsigned char * reply,
              int conn_idx)
{
  static const unsigned char set_sn_help[] =
    "Usage: eeprom_set_sn <serial_number>\n"
    " --> Saving to EEPROM is required.";

  static const unsigned char set_sn_err[] =
    "Serial number could not be parsed.";

  unsigned char param[MAX_PARAM_LEN];
  char ret;
  int aux;
  
  
  ret = get_next_param(param, params, ' ');
  if (ret != 0) {
    return strcpyl(reply, err_param);
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, set_sn_help);
  }
  
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strcpyl(reply, err_ia);
  }
  
  char sn = (char) aux;
  ret = user_eeprom_set_serial_number(sn);

  if (ret == -1) {
    return strcpyl(reply, set_sn_err);
  }
  
  return strcpyl(reply, ok_str);
}

int
eeprom_get_sn(unsigned char * params,
              unsigned char * reply,
              int conn_idx)
{
  char ret;

  static const unsigned char get_sn_help[] =
    "Provides serial number (in memory).\n"
    "Usage: eeprom_get_sn";
  
  static const unsigned char get_sn_err[] =
    "Serial number could not be parsed.";

  unsigned char param[MAX_PARAM_LEN];
  
  ret = get_next_param(param, params, ' ');
  if (ret == 0) {
    if (str_eq(param, help_str) == 1
        || str_eq(param, question_mark_str) == 1) {
      return strcpyl(reply, get_sn_help);
    }
  }

  uint8_t sn;
  ret = user_eeprom_get_serial_number(&sn);
  if (ret != 0) {
    return strcpyl(reply, get_sn_err);
  }
  
  ret = a_from_i (param
                  , (int) sn
                  , cmd_buf[conn_idx].hex);
  if (ret != 0) {
    return strcpyl(reply, err_ia);
  }
  
  return strcpyl(reply, param);
}


int
eeprom_set_rn(unsigned char * params,
              unsigned char * reply,
              int conn_idx)
{
  static const unsigned char set_rn_help[] =
    "Configure revision number.\n"
    "Usage: eeprom_set_rn <revision_number>\n"
    " --> Saving to EEPROM is required.";

  static const unsigned char set_rn_err[] =
    "Revision number could not be parsed.";

  unsigned char param[MAX_PARAM_LEN];
  char ret;
  int aux;
  
  
  ret = get_next_param(param, params, ' ');
  if (ret != 0) {
    return strcpyl(reply, err_param);
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, set_rn_help);
  }
  
  ret = i_from_a (&aux,
                  param,
                  &(cmd_buf[conn_idx].hex));
  if (ret != 0) {
    return strcpyl(reply, err_ia);
  }

  char sn = (char) aux;
  ret = user_eeprom_set_revision_number(sn);

  if (ret == -1) {
    return strcpyl(reply, set_rn_err);
  }
  
  return strcpyl(reply, ok_str);
}

int
eeprom_get_rn(unsigned char * params,
              unsigned char * reply,
              int conn_idx)
{
  char ret;

  static const unsigned char get_rn_help[] =
    "Provides revision number (in memory)."
    "\nUsage: eeprom_get_rn";
  
  static const unsigned char get_rn_err[] =
    "Revision number could not be parsed.";

  unsigned char param[MAX_PARAM_LEN];
  
  ret = get_next_param(param, params, ' ');
  if (ret == 0) {
    if (str_eq(param, help_str) == 1
        || str_eq(param, question_mark_str) == 1) {
      return strcpyl(reply, get_rn_help);
    }
  }

  uint8_t rn;
  ret = user_eeprom_get_revision_number(&rn);
  if (ret != 0) {
    return strcpyl(reply, get_rn_err);
  }
  
  ret = a_from_i (param
                  , (int) rn
                  , cmd_buf[conn_idx].hex);
  if (ret != 0) {
    return strcpyl(reply, err_ia);
  }
  
  return strcpyl(reply, param);
}

// mac addresses --------------------------

int
eeprom_set_mac_addr(unsigned char * params,
                    unsigned char * reply,
                    int conn_idx)
{
  static const unsigned char set_mac_addr_help[] =
    "Configure MAC addresses."
    "\nUsage: eeprom_set_mac_addr <eth> <mac addr>"
    "\n  <eth>: 0 or 1"
    "\n  <mac addr>: 6 bytes (hex or int)"
    "\n--> Saving to EEPROM is required.";

  static const unsigned char invalid_eth[] =
    "Invalid ETH number";

  static const unsigned char set_mac_addr_err[] =
    "MAC address could not be parsed.";
  
  unsigned char param[MAX_PARAM_LEN];
  char ret;
  int aux;
  
  
  ret = get_next_param(param, params, ' ');
  if (ret != 0) {
    return strcpyl(reply, err_param);
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, set_mac_addr_help);
  }

  uint8_t fake;
  ret = i_from_a (&aux, param, &fake);
  if (ret != 0) {
    return strcpyl(reply, err_ia);
  }

  if ( !(aux == 0 || aux == 1) ) {
    return strcpyl(reply, invalid_eth);    
  }
  uint8_t eth = (uint8_t) aux;
  
  uint8_t mac_addr[6];
  uint8_t i;
  for (i = 0; i < 6; i++) {
    
    ret = get_next_param(param, params, ' ');
    if (ret != 0) {
      return strcpyl(reply, err_param);
    }

    ret = i_from_a (&aux,
                    param,
                    &(cmd_buf[conn_idx].hex));
    if (ret != 0) {
      return strcpyl(reply, err_ia);
    }

    mac_addr[i] = (uint8_t) aux;
  }

  ret = user_eeprom_set_mac_addr(eth, mac_addr);
  if (ret == -1) {
      return strcpyl(reply, set_mac_addr_err);
  }
  
  return strcpyl(reply, ok_str);
}


int
eeprom_get_mac_addr(unsigned char * params,
                    unsigned char * reply,
                    int conn_idx)
{
  static const unsigned char get_mac_addr_help[] =
    "Retrieves a MAC address."
    "\nUsage: eeprom_get_mac_addr <eth>"
    "\n  <eth>: 0 or 1";

  static const unsigned char invalid_eth[] =
    "Invalid ETH number";

  static const unsigned char get_mac_addr_err[] =
    "MAC address could not be parsed.";
  
  unsigned char param[MAX_PARAM_LEN];
  char ret;
  int aux;
  
  ret = get_next_param(param, params, ' ');
  if (ret != 0) {
    return strcpyl(reply, err_param);
  }
  
  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, get_mac_addr_help);
  }

  uint8_t fake;
  ret = i_from_a (&aux, param, &fake);
  if (ret != 0) {
    return strcpyl(reply, err_ia);
  }

  if ( !(aux == 0 || aux == 1) ) {
    return strcpyl(reply, invalid_eth);    
  }
  uint8_t eth = (uint8_t) aux;
  
  uint8_t mac_addr[6];
  ret = user_eeprom_get_mac_addr(eth, mac_addr);
  if (ret == -1) {
      return strcpyl(reply, get_mac_addr_err);
  }

  uint8_t i;
  uint8_t * r = reply;
  for (i = 0; i < 6; i++) {

    ret = a_from_i (param, (int) mac_addr[i], 1);
    if (ret != 0) {
      return strcpyl(reply, err_ia);
    }
    
    r += strcpyl(r, param);
    r += strcpyl(r, (uint8_t *) " ");
  }
  *(r-1) = '\0';

  return r - reply;
}
