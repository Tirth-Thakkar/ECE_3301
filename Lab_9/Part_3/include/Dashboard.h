#ifndef LAB9_DASHBOARD_H
#define LAB9_DASHBOARD_H

#include "ADC.h"
#include "Altimeter.h"
#include "MotionSensor.h"

void Dashboard_Flight(const FlightData *flight, const MotionData *motion,
                      uint32_t tick);
void Dashboard_Engineer(const ADCBlock *block, uint8_t averaging);
void Dashboard_Motion(const MotionData *motion);

#endif
