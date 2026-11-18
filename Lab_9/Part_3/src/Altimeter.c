#include "../include/Altimeter.h"
#include "../../../Lab_8/include/BMP280.h"
#include "../../../Lab_8/include/I2C.h"
#include <math.h>

#define SENSOR_RETRY_MS 1000u
#define DISCARD_SAMPLES 5u
#define MOTION_WINDOW 5u
#define MOTION_CONFIRMATIONS 2

/* Lab 8's calibration, filtering, and motion thresholds. */
#define CALIBRATION_SPREAD_PA 5.0f
#define FILTER_ALPHA 0.8f
#define ALTITUDE_SCALE_M 44330.0f
#define ALTITUDE_PRESSURE_EXPONENT 0.1903f
#define MOTION_ENTER_M 0.12f
#define MOTION_EXIT_M 0.06f
#define GROUND_BAND_M 0.25f

static FlightData flight;
static float reference_pa;
static float first_pa;
static float pressure_sum;
static float minimum_pa;
static float maximum_pa;
static float altitude_history[MOTION_WINDOW];
static uint32_t retry_started_ms;
static uint8_t discarded;
static uint8_t history_index;
static uint8_t history_count;
static int8_t confirmation;

static void reset_reference(void) {
  discarded = 0u;
  flight.calibration_count = 0u;
  reference_pa = 0.0f;
  first_pa = 0.0f;
  pressure_sum = 0.0f;
  minimum_pa = 0.0f;
  maximum_pa = 0.0f;
  flight.altitude_m = 0.0f;
  flight.vertical_state = GROUND;
  history_index = 0u;
  history_count = 0u;
  confirmation = 0;
}

static void start_sensor(uint32_t tick) {
  I2C_Init();
  if (BMP280_Init()) {
    reset_reference();
    flight.status = CALIBRATING;
  } else {
    flight.status = SENSOR_FAULT;
    retry_started_ms = tick;
  }
}

static uint8_t read_sample(uint32_t tick) {
  BMP280Sample sample;

  if (!BMP280_ReadSample(&sample)) {
    flight.status = SENSOR_FAULT;
    retry_started_ms = tick;
    return 0u;
  }
  flight.pressure_pa = sample.pressure_pa;
  flight.temperature_c = sample.temperature_c;
  return 1u;
}

static void calibrate_reference(void) {
  if (discarded < DISCARD_SAMPLES) {
    ++discarded;
    return;
  }
  if (flight.calibration_count == 0u) {
    first_pa = flight.pressure_pa;
    minimum_pa = flight.pressure_pa;
    maximum_pa = flight.pressure_pa;
  }

  /* Sum offsets to preserve float precision near 100000 Pa. */
  pressure_sum += flight.pressure_pa - first_pa;
  if (flight.pressure_pa < minimum_pa) {
    minimum_pa = flight.pressure_pa;
  }
  if (flight.pressure_pa > maximum_pa) {
    maximum_pa = flight.pressure_pa;
  }

  if (++flight.calibration_count == CALIBRATION_SAMPLES) {
    if (maximum_pa - minimum_pa > CALIBRATION_SPREAD_PA) {
      reset_reference();
    } else {
      reference_pa = first_pa + pressure_sum / CALIBRATION_SAMPLES;
      flight.status = RUNNING;
    }
  }
}

static void update_altitude(void) {
  float pressure_ratio = flight.pressure_pa / reference_pa;
  float raw_altitude_m =
      ALTITUDE_SCALE_M *
      (1.0f - powf(pressure_ratio, ALTITUDE_PRESSURE_EXPONENT));

  flight.altitude_m =
      FILTER_ALPHA * flight.altitude_m + (1.0f - FILTER_ALPHA) * raw_altitude_m;
}

void Altimeter_Init(uint32_t tick) { start_sensor(tick); }

void Altimeter_Sample(uint32_t tick) {
  /* Keep the Lab 8 sensor states explicit, as in the earlier controllers. */
  switch (flight.status) {
  case SENSOR_FAULT:
    if ((uint32_t)(tick - retry_started_ms) >= SENSOR_RETRY_MS) {
      start_sensor(tick);
    }
    break;
  case CALIBRATING:
    if (read_sample(tick)) {
      calibrate_reference();
    }
    break;
  case RUNNING:
    if (read_sample(tick)) {
      update_altitude();
    }
    break;
  }
}

void Altimeter_UpdateMotion(void) {
  float delta_m;

  if (flight.status != RUNNING) {
    return;
  }
  delta_m = flight.altitude_m - altitude_history[history_index];
  altitude_history[history_index] = flight.altitude_m;
  history_index = (uint8_t)((history_index + 1u) % MOTION_WINDOW);
  if (history_count < MOTION_WINDOW) {
    ++history_count;
    return;
  }

  if ((flight.vertical_state == CLIMB && delta_m >= MOTION_EXIT_M) ||
      (flight.vertical_state == DESCEND && delta_m <= -MOTION_EXIT_M)) {
    confirmation = 0;
    return;
  }

  flight.vertical_state =
      fabsf(flight.altitude_m) < GROUND_BAND_M ? GROUND : HOLD;
  if (delta_m > MOTION_ENTER_M) {
    confirmation = confirmation > 0 ? confirmation + 1 : 1;
  } else if (delta_m < -MOTION_ENTER_M) {
    confirmation = confirmation < 0 ? confirmation - 1 : -1;
  } else {
    confirmation = 0;
  }

  if (confirmation == MOTION_CONFIRMATIONS ||
      confirmation == -MOTION_CONFIRMATIONS) {
    flight.vertical_state = confirmation > 0 ? CLIMB : DESCEND;
    confirmation = 0;
  }
}

const FlightData *Altimeter_Data(void) { return &flight; }
