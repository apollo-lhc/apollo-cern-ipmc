#ifndef SENSOR_PCA9545_H
#define SENSOR_PCA9545_H

/* Write the PCA9545A I2C switch slave select register */
char
pca9545_write(unsigned char mask);

/* Rread the PCA9545A I2C switch slave select register */
char
pca9545_read(unsigned char * mask);

#endif /* SENSOR_PCA9545_H */
