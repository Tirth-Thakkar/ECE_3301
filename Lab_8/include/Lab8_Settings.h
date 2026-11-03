#ifndef ECE3301_LAB8_SETTINGS_H
#define ECE3301_LAB8_SETTINGS_H
/*
 * Compile-time settings for ECE 3301 Lab 8.
 * _XTAL_FREQ must be set by the compiler for ALL source files (including lcd.c).
 * The standalone Timer2 design requires 16 MHz, with PLL disabled.
 */
#ifndef _XTAL_FREQ
#error "Define _XTAL_FREQ=16000000UL for the entire XC8 project"
#endif
#if _XTAL_FREQ != 16000000UL
#error "Timer2 timing requires a 16 MHz system clock"
#endif

#define BMP280_ADDR7             0x76u   /* BMP280 SDO tied to GND */
#define BMP280_WRITE             ((BMP280_ADDR7 << 1) | 0u)
#define BMP280_READ              ((BMP280_ADDR7 << 1) | 1u)
#define BMP280_CHIP_ID           0x58u
#define BMP280_CONFIG_VALUE      0x00u   /* 0.5ms standby, sensor IIR off */
#define BMP280_CTRL_MEAS_VALUE   0x57u   /* T x2, P x16, normal mode */
#define I2C_TIMEOUT_COUNT        10000u
#define N_DISCARD                5u
#define N_CAL                    32u
#define CAL_MAX_SPREAD_PA        5.0f
#define FILTER_ALPHA             0.80f
#define DH_WINDOW_STEPS          5u
#define CONFIRM_STEPS            2u

/* PLACEHOLDERS: replace these with measured thresholds (lab Section 28).
 * Record the 30-s stationary min/max dH and the moving peaks before submitting.
 * Stationary dH min/max: TODO (measured on actual hardware)
 * Moving dH climb/descend peaks: TODO (measured on actual hardware)
 */
#define CLIMB_ENTER_M            0.12f
#define CLIMB_EXIT_M             0.06f
#define DESCEND_ENTER_M          0.12f
#define DESCEND_EXIT_M           0.06f
#define GROUND_BAND_M            0.25f
#define ENABLE_DH_DEBUG_PAGE     0u     /* set 1 during threshold measurements */

#endif
