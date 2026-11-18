#ifndef MOTIONSENSOR_H
#define MOTIONSENSOR_H

#include <stdint.h>

typedef struct {
    uint8_t ready;
    int16_t ax;
    int16_t ay;
    int16_t az;
} MotionData;

void MotionSensor_Init(uint32_t tick);
void MotionSensor_Sample(uint32_t tick);
const MotionData *MotionSensor_Data(void);

#endif
