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
    SM_ARMING_COMPL, // Arming Seq Complete
} Signal;

void fsm_init(void);
void fsm_update(uint8_t tick);

#endif // STATECONTROLLER_H