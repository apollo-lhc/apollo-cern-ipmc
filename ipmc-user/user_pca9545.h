#ifndef USER_PCA9545_H
#define USER_PCA9545_H

/* Write the PCA9545A I2C switch slave select register */
char
user_pca9545_write(unsigned char mask);

/* Rread the PCA9545A I2C switch slave select register */
char
user_pca9545_read(unsigned char * mask);

#endif /* USER_PCA9545_H */
