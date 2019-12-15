#include <user_eeprom.h>

#include <user_i2c.h>
#include <user_i2c_msgs.h>
#include <user_gpio.h>

#include <user_helpers.h>

#include <debug.h>

#define MEM_ADDR 0x50
#define ID_PAGE 0x58

static struct __attribute__((packed)) {
  uint8_t version;
  union {
    struct __attribute__((packed)) {
      uint8_t serial_number;
      uint8_t revision_number;
      uint8_t mac_eth0[6];
      uint8_t mac_eth1[6];
    } v0;
  };
} eeprom =
  {
   .version=0xFE
  };

char
user_eeprom_init(void)
{
  return user_eeprom_read();
}

char
user_eeprom_set_version(uint8_t v)
{
  eeprom.version = v;
  // debug_printf("\n:::::::::: %d - %d", eeprom.version, v);
  return 0;
}

char
user_eeprom_get_version(uint8_t * v)
{
  *v = eeprom.version;
  // debug_printf("\n:::::::::: %d - %d", eeprom.version, *v);
  return 0;
}

char
user_eeprom_read(void)
{
  uint8_t addr[] = {0x00, 0x00};
  char ret;

  ret = user_i2c_write(MEM_ADDR, addr, 2, MNGMNT_I2C_BUS);
  if (ret != 0) {
    return -1;
  }

  ret = user_i2c_read(MEM_ADDR
                      , (uint8_t *) &eeprom
                      , sizeof(eeprom)
                      , MNGMNT_I2C_BUS);
  if ( ret != 0) {
    return -2;
  }

  return 0;
}

char
user_eeprom_write(void)
{
  if (user_get_gpio(m24512_we_n) == 1) {
    return -1;
  }

  if (sizeof(eeprom) > 128) {
    return -2;
  }
  
  static uint8_t buffer[2+sizeof(eeprom)];

  buffer[0] = 0x00;
  buffer[1] = 0x00;
  memcpy(&buffer[2], (uint8_t *) &eeprom, sizeof(eeprom));

  user_i2c_write(MEM_ADDR, buffer, sizeof(buffer), MNGMNT_I2C_BUS);

  return 0;
}

char
user_eeprom_get_serial_number(uint8_t * sn)
{
  switch(eeprom.version){
  case 0:
    *(sn) = eeprom.v0.serial_number;
    break;
  default:
    return -1;
  }
  return 0;
}

char
user_eeprom_set_serial_number(uint8_t sn)
{
  switch(eeprom.version){
  case 0:
    eeprom.v0.serial_number = sn;
    break;
  default:
    return -1;
  }
  return 0;
}

char
user_eeprom_get_revision_number(uint8_t * rn)
{
  switch(eeprom.version){
  case 0:
    *(rn) = eeprom.v0.revision_number;
    break;
  default:
    return -1;
  }
  return 0;
}

char
user_eeprom_set_revision_number(uint8_t rn)
{
  switch(eeprom.version){
  case 0:
    eeprom.v0.revision_number = rn;
    break;
  default:
    return -1;
  }
  return 0;
}

char
user_eeprom_get_mac_addr(uint8_t eth
                         , uint8_t * mac)
{
  int i;

  switch(eeprom.version){
  case 0:
    if (eth == 0) {
      for (i = 0; i < 6; i++) {
        mac[i] = eeprom.v0.mac_eth0[i];
      }
    } else if (eth == 1) {
      for (i = 0; i < 6; i++) {
        mac[i] = eeprom.v0.mac_eth1[i];
      }
    } else {
      return -2;
    }
    break;
  default:
    return -1;
  }
  return 0;
}

char
user_eeprom_set_mac_addr(uint8_t eth
                         , uint8_t * mac)
{
  int i;

  switch(eeprom.version){
  case 0:
    if (eth == 0) {
      for (i = 0; i < 6; i++) {
        eeprom.v0.mac_eth0[i] = mac[i];
      }
    } else if (eth == 1) {
      for (i = 0; i < 6; i++) {
        eeprom.v0.mac_eth1[i] = mac[i];
      }
    } else {
      return -2;
    }
    break;
  default:
    return -1;
  }
  return 0;
}

char
user_eeprom_write_enable(void)
{
  user_set_gpio(m24512_we_n, 0);
  return 0;
}

char
user_eeprom_write_disable(void)
{
  user_set_gpio(m24512_we_n, 1);
  return 0;
}

char
user_eeprom_get_wren(uint8_t *lvl)
{
  *lvl = !user_get_gpio(m24512_we_n);
  return 0;
}
