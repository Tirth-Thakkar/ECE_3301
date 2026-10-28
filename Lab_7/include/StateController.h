#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H

#include <stdint.h>

typedef enum
{
    SAFE,
    ARMING,
    ARMED,
    FAULT
} DroneState;

typedef enum
{
    STABLE,
    MOVING,
    TILTED,
    EXCESSIVE
} MotionState;

void fsm_init(uint8_t sensor_ok);
void fsm_run(uint32_t tick, uint8_t fault, uint8_t arm, uint8_t comm);
void fsm_motion(int16_t ax, int16_t ay, int16_t az, uint32_t tick);
DroneState fsm_state(void);
MotionState fsm_motion_state(void);
const char *fsm_state_name(void);
const char *fsm_motion_name(void);

#endif
