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

static struct StateController fsm_t;
static RateGroup rate_group_t = {0, 0, 0, 0, 0}; 
static uint8_t comm_previous = 0b0;
static uint8_t comm_event = 0b0;

volatile uint32_t comm_req = 0b0; 
volatile uint32_t arm_req = 0b0;
volatile uint32_t fault_status = 0b0;

void fsm_init(void);
void fsm_run(uint32_t tick);

// Handles Immediate Arm Disabling + Fault Triggering
void monitor_safety(void); 

#endif // STATECONTROLLER_H
