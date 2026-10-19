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
    char* input_buffer;

    response_callback(&fsm_t.ack_response, input_buffer);

    if (strcmp(fsm_t.ack_response.code, "$ERR, ") == 1){
        return;
    }
    
    fsm_t.state_response.message = (fsm_t.current_state == STATE_SAFE) ? "SAFE\r\n" : "ARMED\r\n";

    switch(input_buffer[0]) {
        case 'S':
        case 's':
            transmit_response(&fsm_t.state_response);
            break;
        case 'A':
        case 'a':
            fsm_t.current_state = STATE_ARMED;
            fsm_t.state_response.message = "ARMED\r\n";
            transmit_response(&fsm_t.state_response);
            break;
        case 'D':
        case 'd':
            fsm_t.current_state = STATE_SAFE;
            fsm_t.state_response.message = "SAFE\r\n";
            transmit_response(&fsm_t.state_response);
            break;
        }

    transmit_response(&fsm_t.ack_response);

}

void fsm_init(void) {
    fsm_t.current_state = STATE_SAFE; 
    fsm_t.rate_group = rate_group_t;
    fsm_t.ack_response = response_t;
    fsm_t.state_response = state_response_t;
    fsm_t.timestamp = timestamp_t;
    fsm_t.sys_tick = 0b0;
    
    transmit_string("DRONE GROUND LINK ONLINE\r\n");
}


