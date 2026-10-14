#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H
#include <stdint.h>

typedef enum {
    STATE_SAFE,
    STATE_ARMING,
    STATE_ARMED, 
    STATE_FAULT
} State;

typedef struct {
    uint32_t ms_200;
    uint32_t ms_500;
    uint32_t ms_1000;
    uint32_t ms_2000;
    uint32_t ms_3000;
} RateGroup;


void fsm_init(void);
void fsm_run(uint32_t tick);

// Handles Immediate Arm Disabling + Fault Triggering
void monitor_safety(void); 

#endif // STATECONTROLLER_H
