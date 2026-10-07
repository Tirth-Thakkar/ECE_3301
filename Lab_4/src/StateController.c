#include "../include/StateController.h"
#include <xc.h>
#define _XTAL_FREQ 64000000

// Io Defines
#define ARM_REQUEST PORTBbits.RB0
#define FAULT PORTBbits.RB1
#define COMM_HEARTBEAT PORTBbits.RB2

#define ARMED_LED PORTDbits.RD0
#define FAULT_LED PORTDbits.RD1
#define COMM_HEARTBEAT_LED PORTDbits.RD2

struct StateController {
    State current_state;
    RateGroup rate_group;
    uint8_t sys_tick;
};

static struct StateController fsm_t;
static RateGroup rate_group_t = {0, 0, 0, 0, 0}; 

void fsm_init(void);
void fsm_sys_tick_increment(uint8_t);

// Utils
int button_engaged(int);
void gen_heartbeat(uint8_t*);

// State Transition Handlers
void handle_arm_request(void);
void handle_arming_seq(void);
void handle_armed(void);
void handle_fault(void);

void fsm_run(uint8_t tick) {
        fsm_sys_tick_increment(tick);

        switch(fsm_t.current_state) {
            case STATE_SAFE:
                fsm_t.rate_group.ms_500 = 0;
                // Set Outputs
                ARMED_LED = 0;
                FAULT_LED = 0;
                handle_arm_request();
                break;
            case STATE_ARMING:
                // Clear Rate Group Flag
                fsm_t.rate_group.ms_2000 = 0;
                handle_arming_seq();
                break;
            case STATE_ARMED:
                fsm_t.rate_group.ms_1000 = 0;
                ARMED_LED = 1;
                FAULT_LED = 0;
                handle_armed();
                break;
            case STATE_FAULT:
                ARMED_LED = 0;
                FAULT_LED = 1;
                COMM_HEARTBEAT_LED = 0;
                handle_fault();
                break;
            default:
                break;
    };

    return;
}

// Set base params
void fsm_init(void) {
    fsm_t.current_state = STATE_SAFE; 
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

// State Transition Handlers
void handle_arm_request(void) {
    if(button_engaged(ARM_REQUEST)) {
        fsm_t.current_state = STATE_ARMING;
    }
}

void handle_arming_seq(void) {
    int arm_request_status;
    
    while(arm_request_status || !fsm_t.rate_group.ms_2000) {
        arm_request_status = button_engaged(ARM_REQUEST);
        
        // Create 500ms Heartbeat During Arming State
        gen_heartbeat(&fsm_t.rate_group.ms_500);
    }

    if (arm_request_status && fsm_t.rate_group.ms_2000) {
        fsm_t.current_state = STATE_ARMED;
    } else {
        fsm_t.current_state = STATE_SAFE;
    }

    fsm_t.rate_group.ms_2000 = 0;
}

void handle_armed(void) {
    int armed_status = button_engaged(ARM_REQUEST);
    
    while(armed_status) {
        armed_status = button_engaged(ARM_REQUEST);

        // Create 1s Heartbeat During Armed State
        gen_heartbeat(&fsm_t.rate_group.ms_1000);
    }

    if (!armed_status) {
        fsm_t.current_state = STATE_SAFE;
    }
}

void handle_fault(void) {
    int fault_status = button_engaged(FAULT);
    int arm_status = button_engaged(ARM_REQUEST);

    if (!fault_status && !arm_status) {
        fsm_t.current_state = STATE_SAFE;
    }
}

void monitor_safety(void) {
    int fault_status = button_engaged(FAULT);
    
    if (fault_status) {
        fsm_t.current_state = STATE_FAULT;
    }

    return;
}

void gen_heartbeat(uint8_t *rate) {
    if(*rate) {
        COMM_HEARTBEAT_LED = !COMM_HEARTBEAT_LED;
        *rate = 0;
    }
    
    return;
}

// Debounce Util
int button_engaged(int button_io_reg) {
    // Debounce the button
    static uint8_t prev_state = 0;
    uint8_t curr_state = !button_io_reg;

    if (curr_state != prev_state) {
        __delay_ms(20);
        curr_state = !button_io_reg;

        if (curr_state != prev_state) {
            prev_state = curr_state;
            return curr_state;
        }
    }

    return 0;
};