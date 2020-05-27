#ifndef USER_PIM400KZ_H
#define USER_PIM400KZ_H

// PIM400KZ
//
// | Addr | Parameter   | Description                         | Factor               |
// |------+-------------+-------------------------------------+----------------------|
// | 1Eh  | STATUS_BITS | Digital signals                     | N/A                  |
// | 1Fh  | V_HLDP      | Holdup voltage wrt -48V_OUT         | 0.398 V/bit          |
// | 21h  | -48V_IOUT   | -48V_OUT current                    | 0.094 A/bit          |
// | 22h  | -48V_AF     | Voltage between -48V_AF and VRTN_AF | 0.325 V/bit          |
// | 23h  | -48V_BF     | Voltage between -48V_BF and VRTN_BF | 0.325 V/bit          |
// | 28h  | TEMP        | Module Temperature                  | (1.961 oC/bit)-50 oC |
// |------+-------------+-------------------------------------+----------------------|
//
// | bit | name             | desc           | value | translation          |
// |-----+------------------+----------------+-------+----------------------|
// |   0 | ENABLE_AF_STATUS | Feed channel A |     0 | AF disabled          |
// |     |                  |                |     1 | AF enabled           |
// |   1 | ENABLE_BF_STATUS | Feed channel B |     0 | BF disabled          |
// |     |                  |                |     1 | BF enabled           |
// |   2 | ALARM_STATUS     |                |     0 | alarm not set        |
// |     |                  |                |     1 | alarm set            |
// |   3 | N/A              | Reserved       |       |                      |
// |   4 | HLDP_STATUS      |                |     0 | C_HLDP not connected |
// |     |                  |                |     1 | C_HLDP connected     |
// |   5 | HOTSWAP_STATUS   |                |     0 | switch off           |
// |     |                  |                |     1 | switch on            |
// |   6 | -48VOUT_STATUS   | Undervoltage   |     0 | below threshold      |
// |     |                  |                |     1 | above threshold      |
// |   7 | N/A              | Reserved       |       |                      |
// |-----+------------------+----------------+-------+----------------------|
//
// 7-bit addr + r/w bit: 0101 & xyz & r/w
// R addr : open (schematics) -> XYZ = 111 -> 5Eh write, 5Fh read -> 2Fh (7-bit addr)
//
// | S | addr | r/w | a | data | a/a_ | sr | addr | r/w | a | data | a/a_ | p |
// data: n bytes


typedef unsigned char uint8_t;

char
user_pim400kz_init(void);

char
user_pim400kz_get_status(uint8_t * val);

char
user_pim400kz_set_status(uint8_t * val);

char
user_pim400kz_get_holdup_voltage(uint8_t * val);

char
user_pim400kz_get_current(uint8_t * val);

char
user_pim400kz_get_af_voltage(uint8_t * val);

char
user_pim400kz_get_bf_voltage(uint8_t * val);

char
user_pim400kz_get_temp(uint8_t * val);

#endif // USER_PIM400KZ_H
