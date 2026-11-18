#include "../include/Dashboard.h"
#include "../../Common/include/Indicators.h"
#include "../../Common/include/LCD.h"
#include <stdio.h>

void Dashboard_Update(uint16_t value) {
  char top[LCD_COLS + 1u];
  char bits[11];

  Indicators_FormatBinary(value, bits);
  snprintf(top, sizeof top, "ADC:%04u", (unsigned int)value);
  LCD_PrintLine(0u, top);
  LCD_PrintLine(1u, bits);
}
