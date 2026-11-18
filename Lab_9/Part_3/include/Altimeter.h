#ifndef ALTIMETER_H
#define ALTIMETER_H

/* Retain Lab 8's sensor status, vertical states, and flight-data format. */
#include "../../../Lab_8/include/Dashboard.h"

void Altimeter_Init(uint32_t tick);
void Altimeter_Sample(uint32_t tick);
void Altimeter_UpdateMotion(void);
const FlightData *Altimeter_Data(void);

#endif
