#ifndef LIS3DH_H
#define LIS3DH_H

#include <stdint.h>

#define LIS3DH_WHO_AM_I 0x0F
#define LIS3DH_ID 0x33
#define LIS3DH_CTRL_REG1 0x20
#define LIS3DH_CTRL_REG4 0x23
#define LIS3DH_OUT_X_L 0x28

uint8_t LIS3DH_ReadRegister(uint8_t reg);
void LIS3DH_WriteRegister(uint8_t reg, uint8_t data);
uint8_t LIS3DH_Init(void);
void LIS3DH_ReadAccel(int16_t *ax, int16_t *ay, int16_t *az);

#endif
