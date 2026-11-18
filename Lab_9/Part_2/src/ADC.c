#include "../include/ADC.h"
#include <xc.h>

void ADC_Init(void) {
  PIE1bits.ADIE = 0u;
  ADCON0 = 0u;
  ADCON1 = 0u; /* VDD and VSS references. */
  ADCON2 = 0x92u;
  PIR1bits.ADIF = 0u;
  ADC_Configure(ADC_AN0, ADC_CLOCK_NORMAL, ADC_ACQUIRE_4_TAD);
  ADCON0bits.ADON = 1u;
}

void ADC_Configure(ADCChannel channel, ADCClock clock,
                   ADCAcquisition acquisition) {
  uint8_t acquisition_bits = (uint8_t)((uint8_t)acquisition << 3);

  if (channel == ADC_AN0) {
    ANSELAbits.ANSA0 = 1u;
    TRISAbits.TRISA0 = 1u;
    ADCON0bits.CHS = 0u;
  } else {
    ANSELAbits.ANSA1 = 1u;
    TRISAbits.TRISA1 = 1u;
    ADCON0bits.CHS = 1u;
  }
  ADCON2 = (uint8_t)(0x80u | acquisition_bits | (uint8_t)clock);
  PIR1bits.ADIF = 0u;
}

uint16_t ADC_Read(void) {
  uint16_t value;

  ADCON0bits.GO_nDONE = 1u;
  while (ADCON0bits.GO_nDONE) {
  }
  value = (uint16_t)(((uint16_t)ADRESH << 8) | ADRESL);
  __delay_us(4); /* 2 TAD recovery at the normal FOSC/32 clock. */
  PIR1bits.ADIF = 0u;
  return value;
}

uint16_t ADC_ReadBurst(uint16_t samples[ADC_BURST_SAMPLES]) {
  uint16_t sum = 0u;
  uint8_t sample;

  /* Keep the capture loop short. Interrupts remain off in this project. */
  for (sample = 0u; sample < ADC_BURST_SAMPLES; ++sample) {
    ADCON0bits.GO_nDONE = 1u;
    while (ADCON0bits.GO_nDONE) {
    }
    samples[sample] = (uint16_t)(((uint16_t)ADRESH << 8) | ADRESL);
    __delay_us(4);
  }
  PIR1bits.ADIF = 0u;

  /* The first result benefits from idle acquisition before the burst. */
  for (sample = 1u; sample < ADC_BURST_SAMPLES; ++sample) {
    sum += samples[sample];
  }
  return (uint16_t)((sum + 7u) / 15u);
}
