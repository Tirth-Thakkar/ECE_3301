#include "../include/Utils.h"

static struct StateController fsm_t;
static RateGroup rate_group_t = {0, 0, 0, 0, 0};
static Response ack_response_t = {"$ACK, ", ""};
static Response state_response_t = {"$STATE, ", ""};
static Timestamp timestamp_t = {"$TIME, ", 0};

void fsm_run(uint32_t tick) {
    fsm_t.sys_tick = tick;
    // handle_rx_fault();
    // rx --> rx_handle --> transition     
}

void fsm_init(void) {
    fsm_t.current_state = STATE_SAFE; 
    fsm_t.rate_group = rate_group_t;
    fsm_t.ack_response = ack_response_t;
    fsm_t.state_response = state_response_t;
    fsm_t.timestamp = timestamp_t;
    fsm_t.sys_tick = 0b0; 
}


