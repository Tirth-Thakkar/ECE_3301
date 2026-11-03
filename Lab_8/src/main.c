// ECE 3301 Lab 8 - PIC18F46K22, BMP280, and HD44780 LCD.
#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "lcd.h"

#include "Configuration.h"
#include "Lab8_Settings.h"

typedef enum { GROUND, CLIMB, HOLD, DESCEND } VerticalState;
typedef enum { SYS_CALIBRATING, SYS_RUNNING, SYS_SENSOR_FAULT } SystemState;

static SystemState systemState = SYS_CALIBRATING;
static VerticalState verticalState = GROUND;
static float pressure = 0.0f, temperature = 0.0f;
static float referencePressure = 0.0f;
static float rawAltitude = 0.0f, filteredAltitude = 0.0f, deltaAltitude = 0.0f;
static float altHistory[DH_WINDOW_STEPS];
static uint8_t histIndex = 0u, histFilled = 0u;
static uint8_t climbCount = 0u, descendCount = 0u;
static uint8_t discarded = 0u, calCount = 0u;
static float firstPressure = 0.0f, diffSum = 0.0f;
static float calMin = 110000.0f, calMax = 0.0f;
static uint8_t retryTicks = 0u, readyScreens = 0u, lcdUpdates = 0u;
static uint8_t lcdPage = 0u, lastID = 0xFFu;
static uint8_t idProblem = 1u;
static float debugDhMin = 0.0f, debugDhMax = 0.0f;
static uint8_t debugDhValid = 0u;

volatile uint8_t sensorTask = 0u;
volatile uint8_t stateTask = 0u;
volatile uint8_t lcdTask = 0u;

static uint16_t dig_T1, dig_P1;
static int16_t dig_T2, dig_T3, dig_P2, dig_P3, dig_P4;
static int16_t dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static float t_fine;

// Wait for the I2C bus, with a timeout to avoid freezing.
static uint8_t I2C_WaitIdle(void)
{
    uint16_t t = I2C_TIMEOUT_COUNT;
    while ((SSP2CON2 & 0x1Fu) || (SSP2STAT & 0x04u)) {
        if (--t == 0u) return 0u;
    }
    return 1u;
}

static uint8_t I2C_WaitIF(void)
{
    uint16_t t = I2C_TIMEOUT_COUNT;
    while (!PIR3bits.SSP2IF) {
        if (--t == 0u) return 0u;
    }
    PIR3bits.SSP2IF = 0u;
    return 1u;
}

// Configure RD0/RD1 for 100 kHz MSSP2 I2C.
static void I2C_Init(void)
{
    SSP2CON1bits.SSPEN = 0u;
    ANSELD &= (uint8_t)~0x03u;
    TRISD  |= 0x03u;
    SSP2STAT = 0x80u;
    SSP2ADD = (uint8_t)(_XTAL_FREQ / (4UL * 100000UL) - 1UL);
    SSP2CON2 = 0x00u;
    SSP2CON1 = 0x28u;
    PIR3bits.SSP2IF = 0u;
}

static uint8_t I2C_Start(void)
{
    if (!I2C_WaitIdle()) return 0u;
    PIR3bits.SSP2IF = 0u;
    SSP2CON2bits.SEN = 1u;
    return I2C_WaitIF();
}

static uint8_t I2C_RepeatedStart(void)
{
    if (!I2C_WaitIdle()) return 0u;
    PIR3bits.SSP2IF = 0u;
    SSP2CON2bits.RSEN = 1u;
    return I2C_WaitIF();
}

static uint8_t I2C_Stop(void)
{
    if (!I2C_WaitIdle()) return 0u;
    PIR3bits.SSP2IF = 0u;
    SSP2CON2bits.PEN = 1u;
    return I2C_WaitIF();
}

static uint8_t I2C_Write(uint8_t value)
{
    if (!I2C_WaitIdle()) return 0u;
    SSP2CON1bits.WCOL = 0u;
    PIR3bits.SSP2IF = 0u;
    SSP2BUF = value;
    if (SSP2CON1bits.WCOL) return 0u;
    if (!I2C_WaitIF()) return 0u;
    return (uint8_t)(SSP2CON2bits.ACKSTAT == 0u);
}

static uint8_t I2C_Read(uint8_t *value, uint8_t nack)
{
    if (!I2C_WaitIdle()) return 0u;
    PIR3bits.SSP2IF = 0u;
    SSP2CON2bits.RCEN = 1u;
    if (!I2C_WaitIF()) return 0u;
    *value = SSP2BUF;
    if (!I2C_WaitIdle()) return 0u;
    SSP2CON2bits.ACKDT = (nack != 0u) ? 1u : 0u;
    PIR3bits.SSP2IF = 0u;
    SSP2CON2bits.ACKEN = 1u;
    return I2C_WaitIF();
}

static uint8_t BMP280_WriteRegister(uint8_t reg, uint8_t val)
{
    uint8_t ok = 0u;
    if (!I2C_Start()) goto done;
    if (!I2C_Write(BMP280_WRITE)) goto done;
    if (!I2C_Write(reg)) goto done;
    if (!I2C_Write(val)) goto done;
    ok = 1u;
done:
    if (!I2C_Stop()) ok = 0u;
    return ok;
}

// Read consecutive BMP280 registers in one I2C transaction.
static uint8_t BMP280_ReadBytes(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i, ok = 0u;
    if (len == 0u) return 0u;
    if (!I2C_Start()) goto done;
    if (!I2C_Write(BMP280_WRITE)) goto done;
    if (!I2C_Write(reg)) goto done;
    if (!I2C_RepeatedStart()) goto done;
    if (!I2C_Write(BMP280_READ)) goto done;
    for (i = 0u; i < len; ++i) {
        if (!I2C_Read(&buf[i], (uint8_t)(i == (uint8_t)(len - 1u)))) goto done;
    }
    ok = 1u;
done:
    if (!I2C_Stop()) ok = 0u;
    return ok;
}

static uint8_t BMP280_ReadRegister(uint8_t reg, uint8_t *value)
{
    return BMP280_ReadBytes(reg, value, 1u);
}

static uint16_t ReadU16LE(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

// Load the sensor's factory calibration values.
static uint8_t BMP280_ReadCalibration(void)
{
    uint8_t b[24];
    if (!BMP280_ReadBytes(0x88u, b, sizeof b)) return 0u;
    dig_T1 = ReadU16LE(&b[0]);
    dig_T2 = (int16_t)ReadU16LE(&b[2]);
    dig_T3 = (int16_t)ReadU16LE(&b[4]);
    dig_P1 = ReadU16LE(&b[6]);
    dig_P2 = (int16_t)ReadU16LE(&b[8]);
    dig_P3 = (int16_t)ReadU16LE(&b[10]);
    dig_P4 = (int16_t)ReadU16LE(&b[12]);
    dig_P5 = (int16_t)ReadU16LE(&b[14]);
    dig_P6 = (int16_t)ReadU16LE(&b[16]);
    dig_P7 = (int16_t)ReadU16LE(&b[18]);
    dig_P8 = (int16_t)ReadU16LE(&b[20]);
    dig_P9 = (int16_t)ReadU16LE(&b[22]);
    return (uint8_t)(dig_T1 != 0u && dig_T1 != 0xFFFFu &&
                     dig_P1 != 0u && dig_P1 != 0xFFFFu);
}

// Check the sensor ID, reset it, and enable measurements.
static uint8_t BMP280_Init(void)
{
    uint8_t status, tries;
    I2C_Init();
    lastID = 0xFFu;
    idProblem = 1u;
    if (!BMP280_ReadRegister(0xD0u, &lastID)) return 0u;
    if (lastID != BMP280_CHIP_ID && lastID != 0x60u) return 0u;
    idProblem = 0u;
    if (!BMP280_WriteRegister(0xE0u, 0xB6u)) return 0u;
    __delay_ms(2);
    for (tries = 0u; tries < 100u; ++tries) {
        if (!BMP280_ReadRegister(0xF3u, &status)) return 0u;
        if ((status & 0x01u) == 0u) break;
        __delay_us(100);
    }
    if (tries == 100u) return 0u;
    if (!BMP280_ReadCalibration()) return 0u;
    if (!BMP280_WriteRegister(0xF5u, BMP280_CONFIG_VALUE)) return 0u;
    if (!BMP280_WriteRegister(0xF4u, BMP280_CTRL_MEAS_VALUE)) return 0u;
    return 1u;
}

static float BMP280_CompensateTemperature(int32_t adc_T)
{
    float var1, var2;
    var1 = ((float)adc_T / 16384.0f - (float)dig_T1 / 1024.0f) * (float)dig_T2;
    var2 = ((float)adc_T / 131072.0f - (float)dig_T1 / 8192.0f);
    var2 = var2 * var2 * (float)dig_T3;
    t_fine = var1 + var2;
    return t_fine / 5120.0f;
}

static float BMP280_CompensatePressure(int32_t adc_P)
{
    float var1, var2, p;
    var1 = t_fine / 2.0f - 64000.0f;
    var2 = var1 * var1 * (float)dig_P6 / 32768.0f;
    var2 = var2 + var1 * (float)dig_P5 * 2.0f;
    var2 = var2 / 4.0f + (float)dig_P4 * 65536.0f;
    var1 = ((float)dig_P3 * var1 * var1 / 524288.0f + (float)dig_P2 * var1) / 524288.0f;
    var1 = (1.0f + var1 / 32768.0f) * (float)dig_P1;
    if (var1 == 0.0f) return 0.0f;
    p = 1048576.0f - (float)adc_P;
    p = (p - var2 / 4096.0f) * 6250.0f / var1;
    var1 = (float)dig_P9 * p * p / 2147483648.0f;
    var2 = p * (float)dig_P8 / 32768.0f;
    return p + (var1 + var2 + (float)dig_P7) / 16.0f;
}

// Read both measurements; compensate temperature before pressure.
static uint8_t BMP280_ReadSample(float *p, float *t)
{
    uint8_t b[6];
    int32_t adc_P, adc_T;
    if (!BMP280_ReadBytes(0xF7u, b, sizeof b)) return 0u;
    adc_P = (int32_t)(((uint32_t)b[0] << 12) | ((uint32_t)b[1] << 4) | (b[2] >> 4));
    adc_T = (int32_t)(((uint32_t)b[3] << 12) | ((uint32_t)b[4] << 4) | (b[5] >> 4));
    *t = BMP280_CompensateTemperature(adc_T);
    *p = BMP280_CompensatePressure(adc_P);
    return (uint8_t)((*p >= 30000.0f && *p <= 110000.0f) &&
                     (*t >= -40.0f && *t <= 85.0f));
}

// Reset calibration progress and previous altitude readings.
static void ResetCalibration(void)
{
    uint8_t i;
    discarded = 0u;
    calCount = 0u;
    firstPressure = 0.0f;
    diffSum = 0.0f;
    calMin = 110000.0f;
    calMax = 0.0f;
    for (i = 0u; i < DH_WINDOW_STEPS; ++i) altHistory[i] = 0.0f;
    histIndex = 0u;
    histFilled = 0u;
    climbCount = 0u;
    descendCount = 0u;
    deltaAltitude = 0.0f;
    rawAltitude = 0.0f;
    filteredAltitude = 0.0f;
    verticalState = GROUND;
    debugDhValid = 0u;
    readyScreens = 0u;
    lcdPage = 0u;
    lcdUpdates = 0u;
}

// Calculate altitude relative to the startup pressure.
static float CalculateAltitude(float p, float pRef)
{
    return 44330.0f * (1.0f - powf(p / pRef, 0.1903f));
}

// Read the sensor, calibrate at startup, or retry after a fault.
static void SensorTask(void)
{
    float p, t;
    if (systemState == SYS_SENSOR_FAULT) {
        if (++retryTicks >= 20u) {
            retryTicks = 0u;
            if (BMP280_Init()) {
                ResetCalibration();
                systemState = SYS_CALIBRATING;
            }
        }
        return;
    }
    if (!BMP280_ReadSample(&p, &t)) {
        retryTicks = 0u;
        idProblem = 0u;
        systemState = SYS_SENSOR_FAULT;
        return;
    }
    pressure = p;
    temperature = t;
    if (systemState == SYS_CALIBRATING) {
        if (discarded < N_DISCARD) { ++discarded; return; }
        if (calCount == 0u) firstPressure = pressure;
        diffSum += pressure - firstPressure;
        if (pressure < calMin) calMin = pressure;
        if (pressure > calMax) calMax = pressure;
        if (++calCount >= N_CAL) {
            if (calMax - calMin > CAL_MAX_SPREAD_PA) {
                ResetCalibration();
            } else {
                referencePressure = firstPressure + diffSum / (float)N_CAL;
                ResetCalibration();
                readyScreens = 4u;
                systemState = SYS_RUNNING;
            }
        }
        return;
    }
    rawAltitude = CalculateAltitude(pressure, referencePressure);
    filteredAltitude = FILTER_ALPHA * filteredAltitude +
                       (1.0f - FILTER_ALPHA) * rawAltitude;
}

static VerticalState StationaryState(void)
{
    return (fabsf(filteredAltitude) < GROUND_BAND_M) ? GROUND : HOLD;
}

// Measure altitude change over a 500 ms window.
static void UpdateDeltaAltitude(void)
{
    if (histFilled < DH_WINDOW_STEPS) {
        altHistory[histIndex] = filteredAltitude;
        histIndex = (uint8_t)((histIndex + 1u) % DH_WINDOW_STEPS);
        ++histFilled;
        deltaAltitude = 0.0f;
        return;
    }
    deltaAltitude = filteredAltitude - altHistory[histIndex];
    altHistory[histIndex] = filteredAltitude;
    histIndex = (uint8_t)((histIndex + 1u) % DH_WINDOW_STEPS);
    if (!debugDhValid) {
        debugDhMin = debugDhMax = deltaAltitude;
        debugDhValid = 1u;
    } else {
        if (deltaAltitude < debugDhMin) debugDhMin = deltaAltitude;
        if (deltaAltitude > debugDhMax) debugDhMax = deltaAltitude;
    }
}

// Use thresholds and hysteresis to decide the flight state.
static void UpdateVerticalState(void)
{
    UpdateDeltaAltitude();
    switch (verticalState) {
    case CLIMB:
        if (deltaAltitude < CLIMB_EXIT_M) verticalState = StationaryState();
        break;
    case DESCEND:
        if (deltaAltitude > -DESCEND_EXIT_M) verticalState = StationaryState();
        break;
    default:
        if (deltaAltitude > CLIMB_ENTER_M) {
            descendCount = 0u;
            if (++climbCount >= CONFIRM_STEPS) {
                verticalState = CLIMB;
                climbCount = 0u;
            }
        } else if (deltaAltitude < -DESCEND_ENTER_M) {
            climbCount = 0u;
            if (++descendCount >= CONFIRM_STEPS) {
                verticalState = DESCEND;
                descendCount = 0u;
            }
        } else {
            climbCount = descendCount = 0u;
            verticalState = StationaryState();
        }
        break;
    }
}

static const char *StateName(VerticalState s)
{
    switch (s) {
    case GROUND:  return "GROUND";
    case CLIMB:   return "CLIMB";
    case HOLD:    return "HOLD";
    case DESCEND: return "DESCEND";
    default:      return "?";
    }
}

// Format decimal values without floating-point printf.
static void FormatSignedHundredths(char *dest, size_t size, float val)
{
    uint32_t centi;
    char sign = ' ';
    if (val < 0.0f) { sign = '-'; val = -val; }
    if (val > 999.99f) val = 999.99f;
    centi = (uint32_t)(val * 100.0f + 0.5f);
    snprintf(dest, size, "%c%lu.%02u", sign,
             (unsigned long)(centi / 100u), (unsigned int)(centi % 100u));
}

// Show altitude/state or pressure/temperature on the LCD.
static void UpdateFlightDashboard(void)
{
    char line[17], value[14];
    if (systemState == SYS_SENSOR_FAULT) {
        LCD_PrintLine(0, "BMP280 FAULT");
        if (idProblem) {
            snprintf(line, sizeof line, "ID=0x%02X EXP 0x58", (unsigned int)lastID);
            LCD_PrintLine(1, line);
        } else {
            LCD_PrintLine(1, "CHECK SDA/SCL");
        }
        return;
    }
    if (systemState == SYS_CALIBRATING) {
        LCD_PrintLine(0, "CALIBRATING...");
        snprintf(line, sizeof line, "KEEP STILL %c%c/32",
                 (char)('0' + calCount / 10u), (char)('0' + calCount % 10u));
        LCD_PrintLine(1, line);
        return;
    }
    if (readyScreens != 0u) {
        LCD_PrintLine(0, "READY");
        LCD_PrintLine(1, "ALT:     0.00 m");
        --readyScreens;
        return;
    }
    if (lcdPage == 0u) {
        FormatSignedHundredths(value, sizeof value, filteredAltitude);
        snprintf(line, sizeof line, "ALT:%.8s m", value);
        LCD_PrintLine(0, line);
        snprintf(line, sizeof line, "STATE: %s", StateName(verticalState));
        LCD_PrintLine(1, line);
    } else if (lcdPage == 1u) {
        uint32_t p10 = (uint32_t)(pressure * 10.0f + 0.5f);
        int16_t t10 = (int16_t)(temperature * 10.0f +
                             ((temperature >= 0.0f) ? 0.5f : -0.5f));
        uint16_t tAbs = (uint16_t)((t10 < 0) ? -t10 : t10);
        snprintf(line, sizeof line, "P:%lu.%01u Pa",
                 (unsigned long)(p10 / 10u), (unsigned int)(p10 % 10u));
        LCD_PrintLine(0, line);
        snprintf(line, sizeof line, "T:%s%u.%01u C", (t10 < 0) ? "-" : "",
                 (unsigned int)(tAbs / 10u), (unsigned int)(tAbs % 10u));
        LCD_PrintLine(1, line);
    }
#if ENABLE_DH_DEBUG_PAGE
    else {
        FormatSignedHundredths(value, sizeof value, debugDhValid ? debugDhMin : 0.0f);
        snprintf(line, sizeof line, "dH min:%s", value);
        LCD_PrintLine(0, line);
        FormatSignedHundredths(value, sizeof value, debugDhValid ? debugDhMax : 0.0f);
        snprintf(line, sizeof line, "dH max:%s", value);
        LCD_PrintLine(1, line);
    }
#endif
    if (++lcdUpdates >= 8u) {
        lcdUpdates = 0u;
#if ENABLE_DH_DEBUG_PAGE
        lcdPage = (uint8_t)((lcdPage + 1u) % 3u);
#else
        lcdPage ^= 1u;
#endif
    }
}

// Configure Timer2 for a 1 ms interrupt.
static void Timer_Init(void)
{
    T2CON = 0x00u;
    TMR2 = 0x00u;
    PR2 = 249u;
    PIR1bits.TMR2IF = 0u;
    PIE1bits.TMR2IE = 1u;
    T2CON = 0x06u;
}

// Schedule the sensor, state, and LCD tasks with flags.
void __interrupt() Lab8_ISR(void)
{
    static uint16_t sensorCounter = 0u, stateCounter = 0u, lcdCounter = 0u;
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0u;
        if (++sensorCounter >= 50u)   { sensorCounter = 0u; sensorTask = 1u; }
        if (++stateCounter >= 100u)   { stateCounter = 0u; stateTask = 1u; }
        if (++lcdCounter >= 250u)     { lcdCounter = 0u; lcdTask = 1u; }
    }
}

static void System_Init(void)
{
    OSCCON = 0x70u;
    OSCTUNEbits.PLLEN = 0u;
    RCONbits.IPEN = 0u;
    INTCONbits.GIE = 0u;
    INTCONbits.PEIE = 0u;
}

void main(void)
{
    System_Init();
    LCD_Init();
    LCD_PrintLine(0, "ECE3301 LAB 8");
    LCD_PrintLine(1, "STARTING...");
    I2C_Init();
    Timer_Init();
    ResetCalibration();
    if (BMP280_Init()) systemState = SYS_CALIBRATING;
    else systemState = SYS_SENSOR_FAULT;
    INTCONbits.PEIE = 1u;
    INTCONbits.GIE = 1u;
    while (1) {
        // ProcessSafetyEvents();  // Add your existing safety handler here.
        if (sensorTask) {
            sensorTask = 0u;
            SensorTask();
        }
        if (stateTask) {
            stateTask = 0u;
            if (systemState == SYS_RUNNING && readyScreens == 0u)
                UpdateVerticalState();
        }
        if (lcdTask) {
            lcdTask = 0u;
            UpdateFlightDashboard();
        }
    }
}
