#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <stdint.h>

#define CALIBRATION_SAMPLES 32u

typedef enum { CALIBRATING, RUNNING, SENSOR_FAULT } SensorStatus;
typedef enum { GROUND, CLIMB, HOLD, DESCEND } VerticalState;

typedef struct {
  SensorStatus status;
  VerticalState vertical_state;
  float pressure_pa;
  float temperature_c;
  float altitude_m;
  uint8_t calibration_count;
} FlightData;

void Dashboard_Update(const FlightData *data, uint32_t now_ms);

#endif
