#ifndef INDICATORS_H
#define INDICATORS_H

#include <stdint.h>

void Indicators_Init(void);
void Indicators_ShowBits(uint16_t value, uint8_t lower_nibble);
void Indicators_FormatBinary(uint16_t value, char bits[11]);

#endif
