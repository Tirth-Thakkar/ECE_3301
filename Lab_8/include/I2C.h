#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void I2C_Init(void);
int I2C_ReadRegisters(uint8_t address, uint8_t reg, uint8_t *data,
                      uint8_t count);
int I2C_WriteRegister(uint8_t address, uint8_t reg, uint8_t value);

#endif
