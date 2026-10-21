#include <xc.h>
#include <string.h>
#include "../include/Utils.h"

static struct StateController fsm_t;

static RateGroup rate_group_t = {
    .ms_200  = {0u,  200u},
    .ms_500  = {0u,  500u},
    .ms_1000 = {0u, 1000u},
    .ms_2000 = {0u, 2000u},
    .ms_3000 = {0u, 3000u}
};

static Response response_t = {"$ACK, ", ""};
static Response state_response_t = {"$STATE, ", ""};
static Timestamp timestamp_t = {"$TIME, ", 0};

void fsm_run(uint32_t tick) {
    fsm_t.sys_tick = tick;
    char input_buffer[2] = {0};

    fsm_t.state_response.code = "$STATE, ";
    fsm_t.state_response.message = (fsm_t.current_state == STATE_SAFE) ? "SAFE" : "ARMED";
    fsm_t.timestamp.time = fsm_t.sys_tick / 1000u;

    if ((uint32_t)(fsm_t.sys_tick - fsm_t.rate_group.ms_1000.time) >= fsm_t.rate_group.ms_1000.threshold) {
        fsm_t.rate_group.ms_1000.time = fsm_t.sys_tick;
        transmit_response(&fsm_t.state_response);
        transmit_timestamp(&fsm_t.timestamp);
    }

    if (!response_callback(&fsm_t.ack_response, input_buffer)) {
        return;
    }

    if (strcmp(fsm_t.ack_response.code, "$ERR, ") == 0) {
        return;
    }

    switch(input_buffer[0]) {
        case 'S':
        case 's':
            fsm_t.state_response.code = "$STATUS, ";
            fsm_t.ack_response.message = "STATUS";
            transmit_response(&fsm_t.state_response);
            transmit_response(&fsm_t.ack_response);
            break;
        case 'A':
        case 'a':
            fsm_t.current_state = STATE_ARMED;
            ARMED_LED = 1;
            fsm_t.state_response.message = "ARMED";
            fsm_t.ack_response.message = "ARM";
            transmit_response(&fsm_t.ack_response);
            transmit_response(&fsm_t.state_response);
            break;
        case 'D':
        case 'd':
            fsm_t.current_state = STATE_SAFE;
            ARMED_LED = 0;
            fsm_t.state_response.message = "SAFE";
            fsm_t.ack_response.message = "DISARM";
            transmit_response(&fsm_t.ack_response);
            transmit_response(&fsm_t.state_response);
            break;
        }

}

void fsm_init(void) {
    fsm_t.current_state = STATE_SAFE; 
    fsm_t.rate_group = rate_group_t;
    fsm_t.ack_response = response_t;
    fsm_t.state_response = state_response_t;
    fsm_t.timestamp = timestamp_t;
    fsm_t.sys_tick = 0b0;
    ARMED_LED = 0;
    
    transmit_string("DRONE GROUND LINK ONLINE\r\n");
}


