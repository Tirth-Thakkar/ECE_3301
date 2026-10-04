#include "../include/StateController.h"

struct StateController {
    State current_state;
    Signal current_signal;
    uint8_t tick_count;
};

static struct StateController fsm_t;


