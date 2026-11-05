#include "../include/BMP280.h"
#include "../include/Configuration.h"
#include "../include/Dashboard.h"
#include "../include/I2C.h"
#include "../include/LCD.h"
#include <math.h>
#include <xc.h>

#define SENSOR_PERIOD_MS 50u
#define MOTION_PERIOD_MS 100u
#define DISPLAY_PERIOD_MS 250u
#define SENSOR_RETRY_MS 1000u

#define OSCILLATOR_IRCF_16MHZ 7u
#define CLOCK_SOURCE_INTERNAL 2u
#define TIMER2_1MS_PR2 249u
#define TIMER2_ON_PRESCALE_16 0x06u

#define DISCARD_SAMPLES 5u
#define MOTION_WINDOW 5u /* 500 ms at 10 Hz. */
#define MOTION_CONFIRMATIONS 2
#define ALTITUDE_SCALE_M 44330.0f
#define ALTITUDE_PRESSURE_EXPONENT 0.1903f

/* Starting values; tune from stationary noise and movement measurements. */
#define CALIBRATION_SPREAD_PA 5.0f
#define FILTER_ALPHA 0.8f
#define MOTION_ENTER_M 0.12f
#define MOTION_EXIT_M 0.06f
#define GROUND_BAND_M 0.25f

static volatile uint32_t ticks;

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

static void configure_clock(void);
static void start_timer(void);
static uint32_t millis(void);

static void reset_reference(void);

static void start_sensor(uint32_t now_ms);
static void sample_sensor(uint32_t now_ms);
static void update_motion(void);

void __interrupt() timer_isr(void);

void main(void) {
  uint32_t last_sensor_ms = 0u;
  uint32_t last_motion_ms = 0u;
  uint32_t last_display_ms = 0u;

  configure_clock();
  LCD_Init();
  start_sensor(0u);
  Dashboard_Update(&flight, 0u);
  start_timer();

  while (1) {
    uint32_t now_ms = millis();

    if ((uint32_t)(now_ms - last_sensor_ms) >= SENSOR_PERIOD_MS) {
      last_sensor_ms = now_ms;
      sample_sensor(now_ms);
    }

    if ((uint32_t)(now_ms - last_motion_ms) >= MOTION_PERIOD_MS) {
      last_motion_ms = now_ms;
      if (flight.status == RUNNING) {
        update_motion();
      }
    }

    if ((uint32_t)(now_ms - last_display_ms) >= DISPLAY_PERIOD_MS) {
      last_display_ms = now_ms;
      Dashboard_Update(&flight, now_ms);
    }
  }
}

static void configure_clock(void) {
  INTCONbits.GIE = 0u;
  RCONbits.IPEN = 0u;
  OSCCONbits.IRCF = OSCILLATOR_IRCF_16MHZ;
  OSCCONbits.SCS = CLOCK_SOURCE_INTERNAL;
  OSCTUNEbits.PLLEN = 0u;
}

static void start_timer(void) {
  T2CON = 0u;
  TMR2 = 0u;
  PR2 = TIMER2_1MS_PR2;
  PIR1bits.TMR2IF = 0u;
  PIE1bits.TMR2IE = 1u;
  T2CON = TIMER2_ON_PRESCALE_16;
  INTCONbits.PEIE = 1u;
  INTCONbits.GIE = 1u;
}

void __interrupt() timer_isr(void) {
  if (PIE1bits.TMR2IE && PIR1bits.TMR2IF) {
    PIR1bits.TMR2IF = 0u;
    ++ticks;
  }
}

static uint32_t millis(void) {
  uint8_t enabled = INTCONbits.GIE;
  INTCONbits.GIE = 0u;
  uint32_t now = ticks;
  INTCONbits.GIE = enabled;
  return now;
}

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

static void start_sensor(uint32_t now_ms) {
  I2C_Init();
  if (BMP280_Init()) {
    reset_reference();
    flight.status = CALIBRATING;
  } else {
    flight.status = SENSOR_FAULT;
    retry_started_ms = now_ms;
  }
}

static void sample_sensor(uint32_t now_ms) {
  BMP280Sample sample;

  if (flight.status == SENSOR_FAULT) {
    if ((uint32_t)(now_ms - retry_started_ms) >= SENSOR_RETRY_MS) {
      start_sensor(now_ms);
    }
    return;
  }
  if (!BMP280_ReadSample(&sample)) {
    flight.status = SENSOR_FAULT;
    retry_started_ms = now_ms;
    return;
  }
  flight.pressure_pa = sample.pressure_pa;
  flight.temperature_c = sample.temperature_c;

  if (flight.status == CALIBRATING) {
    if (discarded < DISCARD_SAMPLES) {
      ++discarded;
      return;
    }
    if (flight.calibration_count == 0u) {
      first_pa = sample.pressure_pa;
      minimum_pa = sample.pressure_pa;
      maximum_pa = sample.pressure_pa;
    }
    /* Summing offsets preserves float precision near 100000 Pa. */
    float pressure_offset_pa = sample.pressure_pa - first_pa;
    pressure_sum += pressure_offset_pa;
    if (sample.pressure_pa < minimum_pa) {
      minimum_pa = sample.pressure_pa;
    }
    if (sample.pressure_pa > maximum_pa) {
      maximum_pa = sample.pressure_pa;
    }

    if (++flight.calibration_count == CALIBRATION_SAMPLES) {
      float pressure_spread_pa = maximum_pa - minimum_pa;

      if (pressure_spread_pa > CALIBRATION_SPREAD_PA) {
        reset_reference();
      } else {
        float mean_offset_pa = pressure_sum / CALIBRATION_SAMPLES;
        reference_pa = first_pa + mean_offset_pa;
        flight.status = RUNNING;
      }
    }
    return;
  }

  float pressure_ratio = sample.pressure_pa / reference_pa;
  float altitude_fraction =
      1.0f - powf(pressure_ratio, ALTITUDE_PRESSURE_EXPONENT);
  float raw_altitude_m = ALTITUDE_SCALE_M * altitude_fraction;

  float retained_altitude_m = FILTER_ALPHA * flight.altitude_m;
  float measured_altitude_m = (1.0f - FILTER_ALPHA) * raw_altitude_m;
  flight.altitude_m = retained_altitude_m + measured_altitude_m;
}

static void update_motion(void) {
  float delta_m = flight.altitude_m - altitude_history[history_index];
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
