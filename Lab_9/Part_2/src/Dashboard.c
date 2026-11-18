#include "../include/Dashboard.h"
#include "../../lib/include/Indicators.h"
#include "../../lib/include/LCD.h"
#include <stdio.h>

void Dashboard_Update(uint16_t value, const char *experiment_name) {
    char top[2u * LCD_COLS + 1u];
    char bits[11];

    Indicators_FormatBinary(value, bits);
    snprintf(top, sizeof top, "ADC:%04u %s", (unsigned int)value,
             experiment_name);
    LCD_PrintLine(0u, top);
    LCD_PrintLine(1u, bits);
}
