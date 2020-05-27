#ifndef CMD_PIM400KZ_H
#define CMD_PIM400KZ_H

int
pim400kz_get_status(unsigned char * params,
                    unsigned char * reply,
                    const int conn_idx);

int
pim400kz_set_status(unsigned char * params,
                    unsigned char * reply,
                    const int conn_idx);

int
pim400kz_get_holdup_voltage(unsigned char * params,
                            unsigned char * reply,
                            const int conn_idx);

int
pim400kz_get_current(unsigned char * params,
                     unsigned char * reply,
                     const int conn_idx);

int
pim400kz_get_af_voltage(unsigned char * params,
                        unsigned char * reply,
                        const int conn_idx);

int
pim400kz_get_bf_voltage(unsigned char * params,
                        unsigned char * reply,
                        const int conn_idx);

int
pim400kz_get_temp(unsigned char * params,
                  unsigned char * reply,
                  const int conn_idx);

#endif // CMD_PIM400KZ_H
