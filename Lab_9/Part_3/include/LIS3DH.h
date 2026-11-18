#ifndef LAB9_LIS3DH_H
#define LAB9_LIS3DH_H

#include <stdint.h>

/* Reuse Lab 7's register definitions with the MSSP2 I2C transport. */
#define LIS3DH_WHO_AM_I 0x0fu
#define LIS3DH_ID 0x33u
#define LIS3DH_CTRL_REG1 0x20u
#define LIS3DH_CTRL_REG4 0x23u
#define LIS3DH_OUT_X_L 0x28u

uint8_t LIS3DH_I2C_Init(void);
uint8_t LIS3DH_I2C_ReadAccel(int16_t *ax, int16_t *ay, int16_t *az);

#endif
