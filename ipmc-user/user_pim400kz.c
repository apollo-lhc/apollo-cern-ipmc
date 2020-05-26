#include <user_pim400kz.h>

#include <user_i2c.h>
#include <user_i2c_msgs.h>
#include <user_gpio.h>

#include <user_helpers.h>

#include <debug.h>

#define PIM400KZ_ADDR 0x2F // 7-bit addr

#define PIM400KZ_REGADDR_STATUS   0x1E
#define PIM400KZ_REGADDR_V_HLDP   0x1F
#define PIM400KZ_REGADDR_48V_IOUT 0x21
#define PIM400KZ_REGADDR_48V_AF   0x22
#define PIM400KZ_REGADDR_48V_BF   0x23
#define PIM400KZ_REGADDR_TEMP     0x28

char
user_pim400kz_init(void)
{
  return 0;
}


char
user_pim400kz_read(uint8_t regaddr, uint8_t * val)
{
  return user_i2c_reg_read(PIM400KZ_ADDR
                           , regaddr
                           , val
                           , 1
                           , MNGMNT_I2C_BUS);
}

char
user_pim400kz_write(uint8_t regaddr, uint8_t * val)
{
  uint8_t data[] = {regaddr, *val};

  return user_i2c_reg_write(PIM400KZ_ADDR
                                , regaddr
                                , data
                                , 2
                                , MNGMNT_I2C_BUS);
}

char
user_pim400kz_get_status(uint8_t * val)
{
  return user_pim400kz_read(PIM400KZ_REGADDR_STATUS, val);
}

char
user_pim400kz_set_status(uint8_t * val)
{
  return user_pim400kz_write(PIM400KZ_REGADDR_STATUS, val);
}

char
user_pim400kz_get_holdup_voltage(uint8_t * val)
{
  return user_pim400kz_read(PIM400KZ_REGADDR_V_HLDP, val);
}

char
user_pim400kz_get_current(uint8_t * val)
{
  return user_pim400kz_read(PIM400KZ_REGADDR_48V_IOUT, val);
}

char
user_pim400kz_get_af_voltage(uint8_t * val)
{
  return user_pim400kz_read(PIM400KZ_REGADDR_48V_AF, val);
}

char
user_pim400kz_get_bf_voltage(uint8_t * val)
{
  return user_pim400kz_read(PIM400KZ_REGADDR_48V_BF, val);
}

char
user_pim400kz_get_temp(uint8_t * val)
{
  // | 28h | TEMP | Module Temperature | (1.961 oC/bit)-50 oC |
  return user_pim400kz_read(PIM400KZ_REGADDR_TEMP, val);
}

