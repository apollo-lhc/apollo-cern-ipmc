#ifndef USER_TCN75_H
#define USER_TCN75_H

/* TCN75 register definitions */
#define TCN75_TEMP_REG		0
#define TCN75_CFG_REG		1
#define TCN75_THYST_REG		2
#define TCN75_TOS_REG		3

#define TCN75_CFG_SD		(1 << 0)	/* Shutdown bit. */
#define TCN75_CFG_TM		(1 << 1)	/* Thermostat mode. */
#define TCN75_CFG_POL		(1 << 2)	/* O.S. Polarity Bit. */

/* O.S. Fault Tolerance bits. */
#define TCN75_CFG_FT_1		(0 << 3)	/* 1 fault */
#define TCN75_CFG_FT_2		(1 << 3)	/* 2 faults */
#define TCN75_CFG_FT_4		(2 << 3)	/* 4 faults */
#define TCN75_CFG_FT_6		(3 << 3)	/* 6 faults */

char inline
user_tcn75_write_conf(unsigned char id
                      , unsigned char conf);

char inline
user_tcn75_write_reg(unsigned char id
                     , unsigned char reg
                     , char temp);

char inline
user_tcn75_read_reg(unsigned char id
                    , unsigned char reg
                    , unsigned char *temp);

#endif /* USER_TCN75_H */
