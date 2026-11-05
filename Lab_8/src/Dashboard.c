#include "../include/Dashboard.h"
#include "../include/LCD.h"
#include <stdio.h>

#define READY_DURATION_MS 1000u
#define PAGE_DURATION_MS 2000u

void Dashboard_Update(const FlightData *data, uint32_t now_ms) {
  static SensorStatus previous_status = CALIBRATING;
  static uint32_t page_started_ms;
  static uint8_t page;
  static int ready;
  static const char *const state_names[] = {"GROUND", "CLIMB", "HOLD",
                                            "DESCEND"};
  char top[LCD_COLS + 1u], bottom[LCD_COLS + 1u];

  if (data->status != previous_status) {
    previous_status = data->status;
    page = 0u;
    page_started_ms = now_ms;
    ready = data->status == RUNNING;
  }

  if (data->status == SENSOR_FAULT) {
    LCD_PrintLine(0, "BMP280 FAULT");
    LCD_PrintLine(1, "CHECK SDA/SCL");
    return;
  }
  if (data->status == CALIBRATING) {
    LCD_PrintLine(0, "CALIBRATING...");
    snprintf(bottom, sizeof bottom, "STILL %02u/%u",
             (unsigned int)data->calibration_count,
             (unsigned int)CALIBRATION_SAMPLES);
    LCD_PrintLine(1, bottom);
    return;
  }
  if (ready) {
    if ((uint32_t)(now_ms - page_started_ms) < READY_DURATION_MS) {
      LCD_PrintLine(0, "READY");
      LCD_PrintLine(1, "ALT:     0.00 m");
      return;
    }
    ready = 0;
    page_started_ms = now_ms;
  }
  if ((uint32_t)(now_ms - page_started_ms) >= PAGE_DURATION_MS) {
    page ^= 1u;
    page_started_ms = now_ms;
  }

  if (page == 0u) {
    snprintf(top, sizeof top, "ALT:%7.2f m", (double)data->altitude_m);
    snprintf(bottom, sizeof bottom, "STATE: %s",
             state_names[data->vertical_state]);
  } else {
    snprintf(top, sizeof top, "P:%.1f Pa", (double)data->pressure_pa);
    snprintf(bottom, sizeof bottom, "T:%.1f C", (double)data->temperature_c);
  }
  LCD_PrintLine(0, top);
  LCD_PrintLine(1, bottom);
}
