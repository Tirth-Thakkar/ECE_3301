#ifndef ADC_H
#define ADC_H

#include <stdint.h>

#define ADC_BURST_SAMPLES 16u

typedef enum { ADC_AN0 = 0, ADC_AN1 = 1 } ADCChannel;
typedef enum { ADC_CLOCK_FAST = 0, ADC_CLOCK_NORMAL = 2 } ADCClock;
typedef enum {
  ADC_ACQUIRE_0_TAD = 0,
  ADC_ACQUIRE_2_TAD = 1,
  ADC_ACQUIRE_4_TAD = 2,
  ADC_ACQUIRE_8_TAD = 4,
  ADC_ACQUIRE_20_TAD = 7
} ADCAcquisition;

void ADC_Init(void);
void ADC_Configure(ADCChannel channel, ADCClock clock,
                   ADCAcquisition acquisition);
uint16_t ADC_Read(void);
uint16_t ADC_ReadBurst(uint16_t samples[ADC_BURST_SAMPLES]);

#endif
