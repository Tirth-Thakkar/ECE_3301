#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H

#include <stdint.h>

typedef enum { SHOW_HIGH_BITS, SHOW_LOW_BITS } ViewState;

typedef struct {
  ViewState current_state;
  uint32_t last_sample_ms;
  uint16_t adc_value;
} StateController;

void fsm_init(void);
void fsm_run(uint32_t tick);
void Dashboard_Update(uint16_t value);

#endif
