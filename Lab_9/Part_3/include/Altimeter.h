#ifndef ALTIMETER_H
#define ALTIMETER_H

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

void Altimeter_Init(uint32_t tick);
void Altimeter_Sample(uint32_t tick);
void Altimeter_UpdateMotion(void);
const FlightData *Altimeter_Data(void);

#endif
