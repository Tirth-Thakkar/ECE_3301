#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H
#include <stdint.h>

typedef enum State{
    STATE_SAFE,
    STATE_ARMED, 
} State;

typedef struct Interval {
    uint32_t time;
    uint32_t threshold;
} Interval;

typedef struct RateGroup {
    Interval ms_200;
    Interval ms_500;
    Interval ms_1000;
    Interval ms_2000;
    Interval ms_3000;
} RateGroup;

typedef struct Response {
    char* code;
    char* message;
} Response;

typedef struct Timestamp {
    char* code;
    uint8_t time;
} Timestamp;

typedef struct StateController{
    State current_state;
    RateGroup rate_group;
    Response ack_response;
    Response state_response;
    Timestamp timestamp;
    uint32_t sys_tick;
} StateController;

void fsm_init(void);
void fsm_run(uint32_t tick);

#endif // STATECONTROLLER_H