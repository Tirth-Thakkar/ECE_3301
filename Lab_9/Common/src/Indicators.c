#include "../include/Indicators.h"
#include <xc.h>

void Indicators_Init(void) {
  /* D2..D5 use RA4..RA7. Preserve AN0, AN1, and all LCD/I2C pins. */
  ANSELA &= 0x0fu;
  LATA &= 0x0fu;
  TRISA &= 0x0fu;
}

void Indicators_ShowBits(uint16_t value, uint8_t lower_nibble) {
  uint8_t nibble;

  if (lower_nibble) {
    nibble = (uint8_t)(value & 0x0fu);
  } else {
    nibble = (uint8_t)((value >> 6) & 0x0fu);
  }
  LATA = (uint8_t)((LATA & 0x0fu) | (uint8_t)(nibble << 4));
}

void Indicators_FormatBinary(uint16_t value, char bits[11]) {
  uint8_t bit;

  for (bit = 0u; bit < 10u; ++bit) {
    bits[bit] = (value & ((uint16_t)1u << (9u - bit))) ? '1' : '0';
  }
  bits[10] = '\0';
}
