#ifndef ADC_H
#define ADC_H

#include <stdint.h>

#define ADC_MAX_COUNT 1023u
#define ADC_BLOCK_SAMPLES 16u
#define ADC_CLOCK_NORMAL 0x02u
#define ADC_ACQT_4 0x10u

typedef struct {
  uint16_t value;
  uint16_t peak_to_peak;
} ADCBlock;

extern volatile uint32_t adc_completed_blocks;

void ADC_Init(void);
void ADC_Start(void);
void ADC_Interrupt(void);
uint8_t ADC_TakeBlock(ADCBlock *block);
void ADC_SetAveraging(uint8_t enabled);

#endif
