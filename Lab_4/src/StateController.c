#include "../include/StateController.h"
#include <xc.h>

// Io Defines
#define ARM_REQUEST PORTBbits.RB0
#define FAULT PORTBbits.RB1
#define COMM_HEARTBEAT PORTBbits.RB2

#define ARMED_LED LATDbits.LATD0
#define FAULT_LED LATDbits.LATD1
#define COMM_HEARTBEAT_LED LATDbits.LATD2

struct StateController {
    State current_state;
    RateGroup rate_group;
    uint8_t sys_tick;
};

static struct StateController fsm_t;
static RateGroup rate_group_t = {0, 0, 0, 0, 0}; 
static uint8_t comm_previous = 0;
static uint8_t comm_event = 0;

void fsm_init(void);
void fsm_sys_tick_increment(uint8_t);

// Utils
uint8_t button_engaged(int);
void gen_heartbeat(uint8_t*, uint8_t);

// State Transition Handlers
void handle_arm_request(void);
void handle_arming_seq(void);
void handle_armed(void);
void handle_fault(void);

void fsm_run(uint8_t tick) {
        // Sample COMM in every state; No Delay in Arming
        uint8_t comm_now = button_engaged(COMM_HEARTBEAT);
        comm_event = comm_now && !comm_previous;
        comm_previous = comm_now;
        fsm_sys_tick_increment(tick);

        switch(fsm_t.current_state) {
            case STATE_SAFE:
                handle_arm_request();
                if (fsm_t.current_state == STATE_SAFE) {
                    gen_heartbeat(&fsm_t.rate_group.ms_500, 5);
                }
                break;
            case STATE_ARMING:
                handle_arming_seq();
                if (fsm_t.current_state == STATE_ARMING) {
                    gen_heartbeat(&fsm_t.rate_group.ms_200, 2);
                }
                break;
            case STATE_ARMED:
                handle_armed();
                if (fsm_t.current_state == STATE_ARMED) {
                    gen_heartbeat(&fsm_t.rate_group.ms_1000, 10);
                }
                break;
            case STATE_FAULT:
                handle_fault();
                break;
            default:
                break;
    }

    // Reset rate-group counters when inactive.
    if (fsm_t.current_state != STATE_ARMING) {
        fsm_t.rate_group.ms_2000 = 0;
    }
    if (fsm_t.current_state != STATE_ARMED) {
        fsm_t.rate_group.ms_3000 = 0;
    }

    ARMED_LED = (fsm_t.current_state == STATE_ARMED);
    FAULT_LED = (fsm_t.current_state == STATE_FAULT);
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
    comm_previous = button_engaged(COMM_HEARTBEAT);
    comm_event = 0;
    LATD = 0x00;
}
 
// Advance only active groups on system tick
void fsm_sys_tick_increment(uint8_t tick) {
    if (!tick) {
        return;
    }

    fsm_t.sys_tick++;
    switch (fsm_t.current_state) {
        case STATE_SAFE:
            fsm_t.rate_group.ms_500++;
            break;
        case STATE_ARMING:
            fsm_t.rate_group.ms_200++;
            fsm_t.rate_group.ms_2000++;
            break;
        case STATE_ARMED:
            fsm_t.rate_group.ms_1000++;
            fsm_t.rate_group.ms_3000++;
            break;
        default:
            break;
    }
}

// State Transition Handlers
void handle_arm_request(void) {
    if(button_engaged(ARM_REQUEST)) {
        fsm_t.current_state = STATE_ARMING;
        fsm_t.rate_group.ms_200 = 0;
        COMM_HEARTBEAT_LED = 0;
    }
}

void handle_arming_seq(void) {
    if (!button_engaged(ARM_REQUEST)) {
        fsm_t.current_state = STATE_SAFE;
        fsm_t.rate_group.ms_500 = 0;
        COMM_HEARTBEAT_LED = 0;
    } else if (fsm_t.rate_group.ms_2000 >= 20) {
        fsm_t.current_state = STATE_ARMED;
        fsm_t.rate_group.ms_1000 = 0;
        COMM_HEARTBEAT_LED = 0;
    }
}

void handle_armed(void) {
    if (!button_engaged(ARM_REQUEST)) {
        fsm_t.current_state = STATE_SAFE;
        fsm_t.rate_group.ms_500 = 0;
        COMM_HEARTBEAT_LED = 0;
        return;
    }

    if (comm_event) {
        // comm event on timeout is considred received.
        fsm_t.rate_group.ms_3000 = 0;
    }
    if (fsm_t.rate_group.ms_3000 >= 30) {
        fsm_t.current_state = STATE_FAULT;
    }
}

void handle_fault(void) {
    int fault_status = button_engaged(FAULT);
    int arm_status = button_engaged(ARM_REQUEST);

    if (!fault_status && !arm_status) {
        fsm_t.current_state = STATE_SAFE;
        fsm_t.rate_group.ms_500 = 0;
    }
}

void monitor_safety(void) {
    int fault_status = button_engaged(FAULT);
    
    if (fault_status) {
        fsm_t.current_state = STATE_FAULT;
        ARMED_LED = 0;
        FAULT_LED = 1;
        COMM_HEARTBEAT_LED = 0;
    }

    return;
}

void gen_heartbeat(uint8_t *rate, uint8_t interval) {
    if(*rate >= interval) {
        COMM_HEARTBEAT_LED = !COMM_HEARTBEAT_LED;
        *rate = 0;
    }
    
    return;
}

uint8_t button_engaged(int button_io_reg) {
    return (uint8_t)(!button_io_reg);
};
