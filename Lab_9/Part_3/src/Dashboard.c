#include "../include/Dashboard.h"
#include "../../lib/include/Indicators.h"
#include "../../lib/include/LCD.h"
#include <stdio.h>

#define READY_DURATION_MS 1000u
#define PAGE_DURATION_MS 2000u

typedef enum { ALTITUDE_PAGE, ENVIRONMENT_PAGE, MOTION_PAGE } FlightPage;

static SensorStatus previous_status = CALIBRATING;
static FlightPage page;
static uint32_t page_started_ms;
static uint8_t ready_message;

static void show_altitude(const FlightData *flight) {
    static const char *const names[] = {"GROUND", "CLIMB", "HOLD", "DESCEND"};
    char top[2u * LCD_COLS + 1u];
    char bottom[2u * LCD_COLS + 1u];

    snprintf(top, sizeof top, "ALT:%7.2f m", (double)flight->altitude_m);
    snprintf(bottom, sizeof bottom, "STATE: %s", names[flight->vertical_state]);
    LCD_PrintLine(0u, top);
    LCD_PrintLine(1u, bottom);
}

static void show_environment(const FlightData *flight) {
    char top[2u * LCD_COLS + 1u];
    char bottom[2u * LCD_COLS + 1u];

    snprintf(top, sizeof top, "P:%.1f Pa", (double)flight->pressure_pa);
    snprintf(bottom, sizeof bottom, "T:%.1f C", (double)flight->temperature_c);
    LCD_PrintLine(0u, top);
    LCD_PrintLine(1u, bottom);
}

static uint8_t show_sensor_status(const FlightData *flight, uint32_t tick) {
    char bottom[LCD_COLS + 1u];

    if (flight->status != previous_status) {
        previous_status = flight->status;
        page = ALTITUDE_PAGE;
        page_started_ms = tick;
        ready_message = flight->status == RUNNING;
    }
    switch (flight->status) {
    case SENSOR_FAULT:
        LCD_PrintLine(0u, "BMP280 FAULT");
        LCD_PrintLine(1u, "CHECK SDA/SCL");
        return 1u;
    case CALIBRATING:
        LCD_PrintLine(0u, "CALIBRATING...");
        snprintf(bottom, sizeof bottom, "STILL %02u/%u",
                 (unsigned int)flight->calibration_count,
                 (unsigned int)CALIBRATION_SAMPLES);
        LCD_PrintLine(1u, bottom);
        return 1u;
    case RUNNING:
        return 0u;
    }
    return 0u;
}

static uint8_t show_ready_message(uint32_t tick) {
    if (!ready_message) {
        return 0u;
    }
    if ((uint32_t)(tick - page_started_ms) < READY_DURATION_MS) {
        LCD_PrintLine(0u, "READY");
        LCD_PrintLine(1u, "ALT:     0.00 m");
        return 1u;
    }
    ready_message = 0u;
    page_started_ms = tick;
    return 0u;
}

void Dashboard_Flight(const FlightData *flight, const MotionData *motion,
                      uint32_t tick) {
    if (show_sensor_status(flight, tick) || show_ready_message(tick)) {
        return;
    }
    if ((uint32_t)(tick - page_started_ms) >= PAGE_DURATION_MS) {
        page_started_ms = tick;
        switch (page) {
        case ALTITUDE_PAGE:
            page = ENVIRONMENT_PAGE;
            break;
        case ENVIRONMENT_PAGE:
            page = MOTION_PAGE;
            break;
        case MOTION_PAGE:
            page = ALTITUDE_PAGE;
            break;
        }
    }

    switch (page) {
    case ALTITUDE_PAGE:
        show_altitude(flight);
        break;
    case ENVIRONMENT_PAGE:
        show_environment(flight);
        break;
    case MOTION_PAGE:
        Dashboard_Motion(motion);
        break;
    }
}

void Dashboard_Engineer(const ADCBlock *block, uint8_t averaging) {
    char top[2u * LCD_COLS + 1u];
    char bottom[2u * LCD_COLS + 1u];
    char bits[11];

    Indicators_FormatBinary(block->value, bits);
    if (block->peak_to_peak <= 99u) {
        snprintf(top, sizeof top, "ADC:%04u AVG:%s", (unsigned int)block->value,
                 averaging ? "ON" : "OFF");
        snprintf(bottom, sizeof bottom, "%s PP:%02u", bits,
                 (unsigned int)block->peak_to_peak);
    } else {
        snprintf(top, sizeof top, "ADC:%04u PP:%u", (unsigned int)block->value,
                 (unsigned int)block->peak_to_peak);
        snprintf(bottom, sizeof bottom, "%s A:%s", bits,
                 averaging ? "ON" : "OFF");
    }
    LCD_PrintLine(0u, top);
    LCD_PrintLine(1u, bottom);
}

void Dashboard_Motion(const MotionData *motion) {
    char top[2u * LCD_COLS + 1u];
    char bottom[2u * LCD_COLS + 1u];

    if (!motion->ready) {
        LCD_PrintLine(0u, "ICM20948 FAULT");
        LCD_PrintLine(1u, "CHECK I2C / CS");
        return;
    }
    snprintf(top, sizeof top, "X:%+5d Y:%+5d", (int)motion->ax,
             (int)motion->ay);
    snprintf(bottom, sizeof bottom, "Z:%+5d mg", (int)motion->az);
    LCD_PrintLine(0u, top);
    LCD_PrintLine(1u, bottom);
}
