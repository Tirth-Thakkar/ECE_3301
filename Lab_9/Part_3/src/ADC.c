#include "../include/ADC.h"
#include <xc.h>

volatile uint32_t adc_completed_blocks;

static volatile uint16_t block_sum;
static volatile uint16_t block_min;
static volatile uint16_t block_max;
static volatile uint8_t block_count;
static volatile uint8_t averaging_enabled;
static volatile uint16_t published_value;
static volatile uint16_t published_pp;
static volatile uint8_t data_ready;

void ADC_Init(void) {
  PIE1bits.ADIE = 0u;
  ADCON0 = 0u;
  ANSELAbits.ANSA0 = 1u;
  TRISAbits.TRISA0 = 1u;
  ADCON1 = 0u; /* PVCFG = 00 (VDD), NVCFG = 00 (VSS). */
  ADCON2 = (uint8_t)(0x80u | ADC_ACQT_4 | ADC_CLOCK_NORMAL);
  PIR1bits.ADIF = 0u;
  block_sum = 0u;
  block_min = ADC_MAX_COUNT;
  block_max = 0u;
  block_count = 0u;
  adc_completed_blocks = 0u;
  averaging_enabled = 1u;
  published_value = 0u;
  published_pp = 0u;
  data_ready = 0u;
  ADCON0bits.ADON = 1u; /* Channel defaults to AN0; start separately. */
}

void ADC_Start(void) {
  /* ADIF also guards a finished result that has not been collected yet. */
  if (!ADCON0bits.GO_nDONE && !PIR1bits.ADIF) {
    ADCON0bits.GO_nDONE = 1u;
  }
}

void ADC_Interrupt(void) {
  uint16_t value;

  PIR1bits.ADIF = 0u;
  value = (uint16_t)(((uint16_t)ADRESH << 8) | ADRESL);
  block_sum += value;
  if (value < block_min) {
    block_min = value;
  }
  if (value > block_max) {
    block_max = value;
  }
  if (++block_count == ADC_BLOCK_SAMPLES) {
    published_value = averaging_enabled
                          ? (uint16_t)((block_sum + 8u) / ADC_BLOCK_SAMPLES)
                          : value;
    published_pp = (uint16_t)(block_max - block_min);
    ++adc_completed_blocks;
    data_ready = 1u; /* Publish last, after both 16-bit values are ready. */
    block_sum = 0u;
    block_min = ADC_MAX_COUNT;
    block_max = 0u;
    block_count = 0u;
  }
}

uint8_t ADC_TakeBlock(ADCBlock *block) {
  uint8_t enabled = INTCONbits.GIE;
  uint8_t ready;

  INTCONbits.GIE = 0u;
  ready = data_ready;
  if (ready) {
    block->value = published_value;
    block->peak_to_peak = published_pp;
    data_ready = 0u;
  }
  INTCONbits.GIE = enabled;
  return ready;
}

void ADC_SetAveraging(uint8_t enabled) { averaging_enabled = (enabled != 0u); }
