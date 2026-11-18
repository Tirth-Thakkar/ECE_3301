#include "../include/ADC.h"
#include <xc.h>

void ADC_Init(void) {
  PIE1bits.ADIE = 0u;
  ADCON0 = 0u;
  ANSELAbits.ANSA0 = 1u;
  TRISAbits.TRISA0 = 1u;
  ADCON1 = 0u;    /* VDD and VSS references. */
  ADCON2 = 0x92u; /* Right aligned, 4 TAD acquisition, FOSC/32. */
  PIR1bits.ADIF = 0u;
  ADCON0bits.ADON = 1u;
}

uint16_t ADC_Read(void) {
  uint16_t value;

  ADCON0bits.GO_nDONE = 1u;
  while (ADCON0bits.GO_nDONE) {
  }
  value = (uint16_t)(((uint16_t)ADRESH << 8) | ADRESL);
  __delay_us(4); /* Datasheet requires 2 TAD recovery between conversions. */
  PIR1bits.ADIF = 0u;

  /* Keep the full result so the ADFM=0 experiment exposes its error. */
  return value;
}
