/* 
    Description:
    This header file defines the interfaces
    provided by the sensor_tcn75a.c module.

    $Revision: 12269 $
*/

#ifndef __MASTER_SENSOR_TCN75A_H__
#define __MASTER_SENSOR_TCN75A_H__

#include <sensor.h>

/* Read-only info structure of an Template sensor */
typedef struct {
    sensor_ro_t s;
    unsigned short id;
} sensor_tcn75a_ro_t;

/* Template sensor methods */
extern sensor_methods_t PROGMEM sensor_tcn75a_methods;

static char sensor_tcn75a_fill_rd(sensor_t *sensor, unsigned char *msg);

/* Auxiliary macro for defining Template sensor info */
#define SENSOR_TCN75A(s, id_p, alert)		\
    {						\
        SA(sensor_tcn75a_methods, s, alert), \
        id: (id_p) \
    }
     
#endif /* __MASTER_SENSOR_TCN75A_H__ */
