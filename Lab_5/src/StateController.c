#include "../include/StateController.h"
#include <xc.h>

// Io Defines
#define ARM_REQUEST PORTBbits.RB1
#define FAULT PORTBbits.RB0
#define COMM_HEARTBEAT PORTBbits.RB2

#define ARMED_LED LATDbits.LATD0
#define FAULT_LED LATDbits.LATD1
#define COMM_HEARTBEAT_LED LATDbits.LATD2
#define SAFE_LED LATDbits.LATD3
#define DEBOUNCE_MS 50u

static struct StateController fsm_t;
static RateGroup rate_group_t = {0, 0, 0, 0, 0};

volatile uint8_t comm_req = 0;
static uint8_t comm_event = 0;

volatile uint8_t arm_req = 0;
static uint8_t arm_event = 0;
static uint8_t arm_seen = 0;

volatile uint8_t fault_status = 0;
static uint8_t fault_event = 0;

static uint32_t last_arm_time = 0;
static uint32_t last_comm_time = 0;
static uint8_t comm_seen = 0;

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
    
    arm_req = comm_req = fault_status = 0;
    arm_event = comm_event = fault_event = 0;
    arm_seen = comm_seen = 0;
    last_arm_time = last_comm_time = 0;
   
    LATD = 0x00;
    SAFE_LED = 1;
}


void process_requests(uint32_t tick) {
    uint8_t high_enabled = INTCONbits.GIEH;
    uint8_t low_enabled = INTCONbits.GIEL;

    INTCONbits.GIEH = 0;
    INTCONbits.GIEL = 0;
    
    // Handle Events
    fault_event = fault_status;
    arm_event = arm_req;
    comm_event = comm_req;
    fault_status = 0;
    arm_req = 0;
    comm_req = 0;
   
    INTCONbits.GIEL = low_enabled;
    INTCONbits.GIEH = high_enabled;

    // Fault Not Debounced
    if (fault_event) {
        arm_event = 0;
        comm_event = 0;
        return;
    }

    if (arm_event) {
        if (arm_seen && (uint32_t)(tick - last_arm_time) < DEBOUNCE_MS) {
            arm_event = 0;
        } else {
            last_arm_time = tick;
            arm_seen = 1;
        }
    }

    if (comm_event) {
        if (comm_seen && (uint32_t)(tick - last_comm_time) < DEBOUNCE_MS) {
            comm_event = 0;
        } else {
            last_comm_time = tick;
            comm_seen = 1;
        }
    }
}

// State Transition Handlers
void handle_arm_request(void) {
    if (arm_event && !ARM_REQUEST && FAULT) {
        fsm_t.current_state = STATE_ARMING;
        fsm_t.rate_group.ms_200 = fsm_t.sys_tick;
        fsm_t.rate_group.ms_2000 = fsm_t.sys_tick;
        COMM_HEARTBEAT_LED = 0;
    }
}

void handle_arming_seq(void) {
    if (ARM_REQUEST) {
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
    if (ARM_REQUEST) {
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
    if (!fault_event && FAULT && ARM_REQUEST) {
        fsm_t.current_state = STATE_SAFE;
        fsm_t.rate_group.ms_500 = fsm_t.sys_tick;
    }
}

void monitor_safety(void) {
    if (fault_event) {
        fsm_t.current_state = STATE_FAULT;
        ARMED_LED = 0;
        SAFE_LED = 0;
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
