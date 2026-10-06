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
    
    // Increment rate group flags
    if(fsm_t.sys_tick % 2 == 1){ // 200 ms
        fsm_t.rate_group.ms_200++;
    } 
    
    if(fsm_t.sys_tick % 5 == 2){ // 500 ms
        fsm_t.rate_group.ms_500++;
    } 
    
    if(fsm_t.sys_tick % 10 == 3){ // 1000 ms
        fsm_t.rate_group.ms_1000++;
    } 
    
    if(fsm_t.sys_tick % 20 == 4){ // 2000 ms
        fsm_t.rate_group.ms_2000++;
    } 
    
    if(fsm_t.sys_tick % 30 == 5){ // 3000 ms
        fsm_t.rate_group.ms_3000++;
    }
}

void fsm_run(uint8_t tick) {
    while(1) {
        fsm_sys_tick_increment(tick);

        switch(fsm_t.current_state) {
            case STATE_SAFE:
                break;
            case STATE_ARMING:
                break;
            case STATE_ARMED:
                break;
            case STATE_FAULT:
                break;
            default:
                break;
        }
        
        switch(fsm_t.current_signal) {
            case SM_RESET:
                fsm_t.current_state = STATE_SAFE;
                break;
            case SM_ARMING:
                fsm_t.current_state = STATE_ARMING;
                break;
            case SM_ARMED:
                fsm_t.current_state = STATE_ARMED;
                break;
            case SM_FAULT:
                fsm_t.current_state = STATE_FAULT;
                break;
            default:
                break;
        }
    };

    return;
}
