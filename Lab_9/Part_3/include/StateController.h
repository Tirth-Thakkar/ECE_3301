#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H

#include "ADC.h"

typedef enum { FLIGHT_VIEW, ENGINEER_VIEW } ViewState;

typedef struct {
  ViewState current_state;
  uint8_t averaging;
  ADCBlock adc;
} StateController;

void fsm_init(void);
void fsm_run(uint32_t tick, uint8_t tasks);

#endif
