#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H
#include <stdint.h>

typedef enum {
    STATE_SAFE,
    STATE_ARMING,
    STATE_ARMED, 
    STATE_FAULT
} State;

typedef enum {
    SM_RESET, // Trigger Safe
    SM_ARMING, // Arming Sequence Started
    SM_ARMED, // Arming Seq Complete
    SM_FAULT, // Fault Triggered
} Signal;

typedef struct {
    uint8_t ms_200; 
    uint8_t ms_500; 
    uint8_t ms_1000; 
    uint8_t ms_2000; 
    uint8_t ms_3000; 
} RateGroup;


void fsm_init(void);
void fsm_run(uint8_t tick);
void fsm_sys_tick_increment(uint8_t tick);

#endif // STATECONTROLLER_H