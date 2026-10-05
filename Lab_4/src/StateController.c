#include "../include/StateController.h"

struct StateController {
    State current_state;
    Signal current_signal;
    RateGroup rate_group;
    uint8_t sys_tick;
};

static struct StateController fsm_t;
static RateGroup rate_group_t = {0, 0, 0, 0, 0}; 

// Set base params
void fsm_init(void) {
    fsm_t.current_state = STATE_SAFE; 
    fsm_t.current_signal = SM_RESET; 
    fsm_t.rate_group = rate_group_t;
    fsm_t.sys_tick = 0b0; 
}
 
// Update based on the system tick
void fsm_sys_tick_increment(uint8_t tick) {
    fsm_t.sys_tick += tick;
    // Divide the 10ms sys_tick to operate rate groups
}
