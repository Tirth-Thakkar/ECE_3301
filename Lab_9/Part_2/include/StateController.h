#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H

#include <stdint.h>

typedef enum {
    EXPERIMENT_A_NORMAL,
    EXPERIMENT_A_FAST,
    EXPERIMENT_B_REFERENCE,
    EXPERIMENT_B_0_TAD,
    EXPERIMENT_B_2_TAD,
    EXPERIMENT_B_8_TAD,
    EXPERIMENT_B_20_TAD,
    EXPERIMENT_COUNT
} ExperimentState;

typedef struct {
    ExperimentState current_state;
    uint8_t lower_nibble;
    uint16_t adc_value;
    uint32_t last_sample_ms;
} StateController;

extern uint16_t experiment_samples[16];

void fsm_init(void);
void fsm_run(uint32_t tick);
void fsm_select_experiment(ExperimentState experiment, uint32_t tick);
ExperimentState fsm_state(void);

void Dashboard_Update(uint16_t value, const char *experiment_name);
#endif
