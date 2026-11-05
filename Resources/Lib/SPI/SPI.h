#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void SPI_Init(void);
uint8_t SPI_Transfer(uint8_t data);

#endif
