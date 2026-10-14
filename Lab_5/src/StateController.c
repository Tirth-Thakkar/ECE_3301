#include "../include/StateController.h"
#include <xc.h>

// Io Defines
#define ARM_REQUEST PORTBbits.RB0
#define FAULT PORTBbits.RB1
#define COMM_HEARTBEAT PORTBbits.RB2

#define ARMED_LED LATDbits.LATD0
#define FAULT_LED LATDbits.LATD1
#define COMM_HEARTBEAT_LED LATDbits.LATD2
#define SAFE_LED LATDbits.LATD3

// Initial State Declaration
void fsm_init(void);

// Utils
void gen_heartbeat(uint32_t*, uint32_t);

// State Transition Handlers
void handle_arm_request(void);
void handle_arming_seq(void);
void handle_armed(void);
void handle_fault(void);

void fsm_run(uint32_t tick) {
        comm_event = comm_req && !comm_previous;
        comm_previous = comm_req;
        
        fsm_t.sys_tick = tick;

        switch(fsm_t.current_state) {
            case STATE_SAFE:
                handle_arm_request();
                if (fsm_t.current_state == STATE_SAFE) {
                    gen_heartbeat(&fsm_t.rate_group.ms_500, 500u);
                }
                break;
            case STATE_ARMING:
                handle_arming_seq();
                if (fsm_t.current_state == STATE_ARMING) {
                    gen_heartbeat(&fsm_t.rate_group.ms_200, 200u);
                }
                break;
            case STATE_ARMED:
                handle_armed();
                if (fsm_t.current_state == STATE_ARMED) {
                    gen_heartbeat(&fsm_t.rate_group.ms_1000, 1000u);
                }
                break;
            case STATE_FAULT:
                handle_fault();
                break;
            default:
                break;
    }

    ARMED_LED = (fsm_t.current_state == STATE_ARMED);
    FAULT_LED = (fsm_t.current_state == STATE_FAULT);
    SAFE_LED = (fsm_t.current_state == STATE_SAFE);
    
    if (fsm_t.current_state == STATE_FAULT) {
        COMM_HEARTBEAT_LED = 0;
    }

    return;
}

// Set base params
void fsm_init(void) {
    fsm_t.current_state = STATE_SAFE; 
    fsm_t.rate_group = rate_group_t;
    fsm_t.sys_tick = 0b0; 
    comm_previous = 0b0; 
    comm_event = 0b0;
    LATD = 0x00;
}
 

// State Transition Handlers
void handle_arm_request(void) {
    if(arm_req) {
        fsm_t.current_state = STATE_ARMING;
        fsm_t.rate_group.ms_200 = fsm_t.sys_tick;
        fsm_t.rate_group.ms_2000 = fsm_t.sys_tick;
        COMM_HEARTBEAT_LED = 0;
    }
}

void handle_arming_seq(void) {
    if (!arm_req) {
        fsm_t.current_state = STATE_SAFE;
        fsm_t.rate_group.ms_500 = fsm_t.sys_tick;
        COMM_HEARTBEAT_LED = 0;
    } else if ((uint32_t)(fsm_t.sys_tick - fsm_t.rate_group.ms_2000) >= 2000u) {
        fsm_t.current_state = STATE_ARMED;
        fsm_t.rate_group.ms_1000 = fsm_t.sys_tick;
        fsm_t.rate_group.ms_3000 = fsm_t.sys_tick;
        COMM_HEARTBEAT_LED = 0;
    }
}

void handle_armed(void) {
    if (!arm_req) {
        fsm_t.current_state = STATE_SAFE;
        fsm_t.rate_group.ms_500 = fsm_t.sys_tick;
        COMM_HEARTBEAT_LED = 0;
        return;
    }

    if (comm_event) {
        // comm event on timeout is considred received.
        fsm_t.rate_group.ms_3000 = fsm_t.sys_tick;
    }
    if ((uint32_t)(fsm_t.sys_tick - fsm_t.rate_group.ms_3000) >= 3000u) {
        fsm_t.current_state = STATE_FAULT;
    }
}

void handle_fault(void) {
    if (!fault_status && !arm_req) {
        fsm_t.current_state = STATE_SAFE;
        fsm_t.rate_group.ms_500 = fsm_t.sys_tick;
    }
}

void monitor_safety(void) {
    if (fault_status) {
        fsm_t.current_state = STATE_FAULT;
        ARMED_LED = 0;
        FAULT_LED = 1;
        COMM_HEARTBEAT_LED = 0;
    }

    return;
}

void gen_heartbeat(uint32_t *rate, uint32_t interval) {
    if ((uint32_t)(fsm_t.sys_tick - *rate) >= interval) {
        COMM_HEARTBEAT_LED = !COMM_HEARTBEAT_LED;
        *rate = fsm_t.sys_tick;
    }
    
    return;
}
