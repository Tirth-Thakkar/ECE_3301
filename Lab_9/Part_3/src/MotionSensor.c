#include "../include/MotionSensor.h"
#include "../include/LIS3DH.h"

#define SENSOR_RETRY_MS 1000u

static MotionData motion;
static uint32_t retry_started_ms;

void MotionSensor_Init(uint32_t tick) {
  motion.ready = LIS3DH_I2C_Init();
  motion.ax = 0;
  motion.ay = 0;
  motion.az = 0;
  retry_started_ms = tick;
}

void MotionSensor_Sample(uint32_t tick) {
  if (!motion.ready) {
    if ((uint32_t)(tick - retry_started_ms) >= SENSOR_RETRY_MS) {
      motion.ready = LIS3DH_I2C_Init();
      retry_started_ms = tick;
    }
    return;
  }

  if (!LIS3DH_I2C_ReadAccel(&motion.ax, &motion.ay, &motion.az)) {
    motion.ready = 0u;
    retry_started_ms = tick;
  }
}

const MotionData *MotionSensor_Data(void) { return &motion; }
