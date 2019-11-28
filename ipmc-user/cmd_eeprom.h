#ifndef CMD_EEPROM_H
#define CMD_EEPROM_H

int
eeprom_wren(unsigned char * params,
            unsigned char * reply,
            int conn_idx);

int
eeprom_save(unsigned char * params,
            unsigned char * reply,
            int conn_idx);

int
eeprom_read(unsigned char * params,
            unsigned char * reply,
            int conn_idx);

int
eeprom_set_version(unsigned char * params,
                   unsigned char * reply,
                   int conn_idx);

int
eeprom_get_version(unsigned char * params,
                   unsigned char * reply,
                   int conn_idx);

int
eeprom_set_sn(unsigned char * params,
              unsigned char * reply,
              int conn_idx);

int
eeprom_get_sn(unsigned char * params,
              unsigned char * reply,
              int conn_idx);

int
eeprom_set_rn(unsigned char * params,
              unsigned char * reply,
              int conn_idx);

int
eeprom_get_rn(unsigned char * params,
              unsigned char * reply,
              int conn_idx);

int
eeprom_set_mac_addr(unsigned char * params,
                    unsigned char * reply,
                    int conn_idx);

int
eeprom_get_mac_addr(unsigned char * params,
                    unsigned char * reply,
                    int conn_idx);

#endif // CMD_EEPROM_H
