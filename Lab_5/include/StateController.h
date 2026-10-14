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


struct StateController {
    State current_state;
    RateGroup rate_group;
    uint32_t sys_tick;
};

extern volatile uint8_t comm_req;
extern volatile uint8_t arm_req;
extern volatile uint8_t fault_status;

void fsm_init(void);
void fsm_run(uint32_t tick);
void process_requests(uint32_t tick);

// Handles Immediate Arm Disabling + Fault Triggering
void monitor_safety(void); 

#endif // STATECONTROLLER_H
