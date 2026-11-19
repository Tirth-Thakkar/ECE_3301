#include "../include/MotionSensor.h"
#include "../include/ICM20948.h"

#define SENSOR_RETRY_MS 1000u
#define POWERUP_WAIT_MS 100u
#define RESET_WAIT_MS 100u
#define ACCEL_STARTUP_MS 50u

typedef enum {
    IMU_POWERUP,
    IMU_RESETTING,
    IMU_STARTING,
    IMU_RUNNING,
    IMU_FAULT
} MotionState;

static MotionData motion;
static MotionState state;
static uint32_t state_started_ms;

static void enter_state(MotionState next, uint32_t tick) {
    state = next;
    state_started_ms = tick;
    motion.ready = 0u;
}

static void reset_sensor(uint32_t tick) {
    if (ICM20948_I2C_Reset()) {
        enter_state(IMU_RESETTING, tick);
    } else {
        enter_state(IMU_FAULT, tick);
    }
}

static void read_acceleration(uint32_t tick) {
    if (ICM20948_I2C_ReadAccel(&motion.ax, &motion.ay, &motion.az)) {
        motion.ready = 1u;
        state = IMU_RUNNING;
    } else {
        enter_state(IMU_FAULT, tick);
    }
}

void MotionSensor_Init(uint32_t tick) {
    motion.ax = 0;
    motion.ay = 0;
    motion.az = 0;
    enter_state(IMU_POWERUP, tick);
}

void MotionSensor_Sample(uint32_t tick) {
    uint32_t elapsed = (uint32_t)(tick - state_started_ms);

    /* Use the existing tick for every hardware wait, including retries. */
    switch (state) {
    case IMU_POWERUP:
        if (elapsed >= POWERUP_WAIT_MS) {
            reset_sensor(tick);
        }
        break;
    case IMU_RESETTING:
        if (elapsed >= RESET_WAIT_MS) {
            if (ICM20948_I2C_Configure()) {
                enter_state(IMU_STARTING, tick);
            } else {
                enter_state(IMU_FAULT, tick);
            }
        }
        break;
    case IMU_STARTING:
        if (elapsed >= ACCEL_STARTUP_MS) {
            read_acceleration(tick);
        }
        break;
    case IMU_RUNNING:
        read_acceleration(tick);
        break;
    case IMU_FAULT:
        if (elapsed >= SENSOR_RETRY_MS) {
            reset_sensor(tick);
        }
        break;
    }
}

const MotionData *MotionSensor_Data(void) { return &motion; }
