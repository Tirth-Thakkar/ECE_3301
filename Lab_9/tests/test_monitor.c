#include "../../Lab_8/include/BMP280.h"
#include "../Part_3/include/ADC.h"
#include "../Part_3/include/Dashboard.h"
#include "../Part_3/include/LIS3DH.h"
#include "../Part_3/include/System.h"
#include "../lib/include/Button.h"
#include "../lib/include/Indicators.h"
#include "xc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

MockADC mock_adc;
MockAnalogA mock_ansela;
MockDirectionA mock_trisa;
uint8_t ADCON1, ADCON2, ADRESH, ADRESL, LATA, T2CON, TMR2, PR2;
struct MockPriority RCONbits;
struct MockClock OSCCONbits;
struct MockPLL OSCTUNEbits;
struct MockInterrupt INTCONbits;
struct MockADCEnable PIE1bits;
struct MockADCFlag PIR1bits;
struct MockAnalogB ANSELBbits;
struct MockDirectionB TRISBbits;
struct MockPullupB WPUBbits;
struct MockPullupEnable INTCON2bits;
struct MockPortB PORTBbits;

static char lcd[2][17];
static uint8_t sensor_address;
static uint8_t sensor_registers[256];
static uint8_t bus_failed;
static uint8_t bmp_available = 1u;
static float pressure_pa = 100000.0f;

void I2C_Init(void) {}
int BMP280_Init(void) { return bmp_available; }
int BMP280_ReadSample(BMP280Sample *sample) {
    if (!bmp_available) {
        return 0;
    }
    sample->pressure_pa = pressure_pa;
    sample->temperature_c = 25.0f;
    return 1;
}

void LCD_PrintLine(uint8_t row, const char *line) {
    assert(row < 2u);
    assert(strlen(line) <= 16u);
    snprintf(lcd[row], sizeof lcd[row], "%s", line);
}

int I2C_ReadRegisters(uint8_t address, uint8_t reg, uint8_t *data,
                      uint8_t count) {
    if (bus_failed || address != sensor_address) {
        return 0;
    }
    /* Multi-byte reads must explicitly select the device's auto-increment. */
    if (count > 1u) {
        assert(reg & 0x80u);
    }
    memcpy(data, &sensor_registers[reg & 0x7fu], count);
    return 1;
}

int I2C_WriteRegister(uint8_t address, uint8_t reg, uint8_t value) {
    if (bus_failed || address != sensor_address) {
        return 0;
    }
    sensor_registers[reg] = value;
    return 1;
}

static void sample(uint16_t value) {
    ADRESH = (uint8_t)(value >> 8);
    ADRESL = (uint8_t)value;
    ADCON0bits.GO_nDONE = 0u;
    PIR1bits.ADIF = 1u;
    ADC_Interrupt();
    assert(PIR1bits.ADIF == 0u);
}

static void constant_block(uint16_t value) {
    for (uint8_t i = 0u; i < 16u; ++i) {
        sample(value);
    }
}

static void test_adc(void) {
    ADCBlock block;
    ADC_Init();
    assert(ADCON0 == 0x01u && ADCON1 == 0u && ADCON2 == 0x92u);
    assert(ANSELAbits.ANSA0 && TRISAbits.TRISA0);
    INTCONbits.GIE = 1u;
    for (uint16_t i = 0u; i < 15u; ++i) {
        sample(i);
        assert(!ADC_TakeBlock(&block));
    }
    sample(15u);
    assert(ADC_TakeBlock(&block));
    assert(block.value == 8u && block.peak_to_peak == 15u);
    assert(INTCONbits.GIE == 1u && adc_completed_blocks == 1u);
    assert(!ADC_TakeBlock(&block));

    constant_block(1023u);
    assert(ADC_TakeBlock(&block));
    assert(block.value == 1023u && block.peak_to_peak == 0u);

    ADC_SetAveraging(0u);
    for (uint16_t i = 0u; i < 15u; ++i) {
        sample(i);
    }
    sample(999u);
    assert(ADC_TakeBlock(&block));
    assert(block.value == 999u && block.peak_to_peak == 999u);

    ADC_SetAveraging(1u);
    constant_block(100u);
    constant_block(200u);
    INTCONbits.GIE = 0u;
    assert(ADC_TakeBlock(&block));
    assert(block.value == 200u && block.peak_to_peak == 0u);
    assert(adc_completed_blocks == 5u && INTCONbits.GIE == 0u);

    ADCON0bits.GO_nDONE = 0u;
    PIR1bits.ADIF = 1u;
    ADC_Start();
    assert(!ADCON0bits.GO_nDONE); /* Never overwrite an unread result. */
    PIR1bits.ADIF = 0u;
    ADC_Start();
    assert(ADCON0bits.GO_nDONE);
}

static uint8_t button(uint8_t level, uint32_t now) {
    PORTBbits.RB4 = level;
    return Button_Update(now);
}

static void test_button(void) {
    Button_Init();
    assert(!ANSELBbits.ANSB4 && TRISBbits.TRISB4 && WPUBbits.WPUB4);
    assert(button(0u, 0u) == 0u);
    assert(button(1u, 5u) == 0u);
    assert(button(0u, 10u) == 0u);
    assert(button(0u, 29u) == 0u);
    assert(button(0u, 30u) == 0u);
    assert(button(1u, 100u) == 0u);
    assert(button(1u, 120u) == BUTTON_SHORT_PRESS);
    assert(button(1u, 121u) == 0u);

    assert(button(0u, 200u) == 0u);
    assert(button(0u, 220u) == 0u);
    assert(button(0u, 1219u) == 0u);
    assert(button(0u, 1220u) == BUTTON_LONG_PRESS);
    assert(button(0u, 1250u) == 0u);
    assert(button(1u, 1300u) == 0u);
    assert(button(1u, 1320u) == 0u); /* Hold release must not switch view. */

    Button_Init();
    uint32_t start = UINT32_MAX - 500u;
    assert(button(0u, start) == 0u);
    assert(button(0u, start + 20u) == 0u);
    assert(button(0u, start + 1020u) == BUTTON_LONG_PRESS);
}

static void test_display(void) {
    ADCBlock block = {902u, 3u};
    char bits[11];
    ANSELA = 0xffu;
    TRISA = 0xffu;
    LATA = 0x07u;
    Indicators_Init();
    assert(ANSELA == 0x0fu && TRISA == 0x0fu && LATA == 0x07u);
    Indicators_ShowBits(902u, 0u);
    assert(LATA == 0xe7u);
    Indicators_ShowBits(902u, 1u);
    assert(LATA == 0x67u);
    Indicators_FormatBinary(902u, bits);
    assert(!strcmp(bits, "1110000110"));
    Dashboard_Engineer(&block, 1u);
    assert(!strcmp(lcd[0], "ADC:0902 AVG:ON"));
    assert(!strcmp(lcd[1], "1110000110 PP:03"));
    block.peak_to_peak = 1023u;
    Dashboard_Engineer(&block, 0u);
    assert(strstr(lcd[0], "PP:1023"));
    assert(strstr(lcd[1], "A:OFF"));

    MotionData motion = {1u, -2048, 2047, -1};
    Dashboard_Motion(&motion);
    assert(strstr(lcd[0], "-2048") && strstr(lcd[0], "+2047"));
    motion.ready = 0u;
    Dashboard_Motion(&motion);
    assert(!strcmp(lcd[0], "LIS3DH FAULT"));

    FlightData flight = {CALIBRATING, GROUND, 100000.0f, 25.0f, 0.0f, 4u};
    Dashboard_Flight(&flight, &motion, 0u);
    assert(!strcmp(lcd[0], "CALIBRATING..."));
    flight.status = RUNNING;
    Dashboard_Flight(&flight, &motion, 250u);
    assert(!strcmp(lcd[0], "READY"));
    Dashboard_Flight(&flight, &motion, 1250u);
    assert(strstr(lcd[0], "ALT:"));
    Dashboard_Flight(&flight, &motion, 3250u);
    assert(strstr(lcd[0], "P:"));
    Dashboard_Flight(&flight, &motion, 5250u);
    assert(!strcmp(lcd[0], "LIS3DH FAULT"));
    Dashboard_Flight(&flight, &motion, 7250u);
    assert(strstr(lcd[0], "ALT:"));
    flight.status = SENSOR_FAULT;
    Dashboard_Flight(&flight, &motion, 7500u);
    assert(!strcmp(lcd[0], "BMP280 FAULT"));
}

static void test_lis3dh(void) {
    int16_t ax = 123, ay = 456, az = 789;
    for (sensor_address = 0x18u; sensor_address <= 0x19u; ++sensor_address) {
        memset(sensor_registers, 0, sizeof sensor_registers);
        sensor_registers[LIS3DH_WHO_AM_I] = LIS3DH_ID;
        assert(LIS3DH_I2C_Init());
        assert(sensor_registers[LIS3DH_CTRL_REG1] == 0x57u);
        assert(sensor_registers[LIS3DH_CTRL_REG4] == 0x88u);
        const uint8_t bytes[] = {0x00, 0x80, 0xf0, 0xff, 0xf0, 0x7f};
        memcpy(&sensor_registers[LIS3DH_OUT_X_L], bytes, sizeof bytes);
        assert(LIS3DH_I2C_ReadAccel(&ax, &ay, &az));
        assert(ax == -2048 && ay == -1 && az == 2047);
    }
    bus_failed = 1u;
    assert(!LIS3DH_I2C_ReadAccel(&ax, &ay, &az));
    assert(ax == -2048 && ay == -1 && az == 2047);
    assert(!LIS3DH_I2C_Init());
    bus_failed = 0u;
    sensor_address = 0x18u;
    sensor_registers[LIS3DH_WHO_AM_I] = 0u;
    assert(!LIS3DH_I2C_Init());
}

static void test_scheduler(void) {
    System_Init();
    ADC_Init();
    System_StartTimer();
    assert(PR2 == 249u && T2CON == 0x06u && PIE1bits.ADIE);
    assert(INTCONbits.GIE && INTCONbits.PEIE);

    for (uint32_t tick = 1u; tick <= 10000u; ++tick) {
        uint8_t tasks;
        uint8_t expected = 0u;
        PIR1bits.TMR2IF = 1u;
        System_TimerInterrupt();
        assert(ADCON0bits.GO_nDONE);
        sample((uint16_t)(tick & 1023u));
        assert(System_ReadEvents(&tasks) == tick);
        if (tick % 10u == 0u)
            expected |= TASK_IMU;
        if (tick % 50u == 0u)
            expected |= TASK_SENSOR;
        if (tick % 100u == 0u)
            expected |= TASK_MOTION;
        if (tick % 250u == 0u)
            expected |= TASK_DISPLAY;
        assert(tasks == expected);
        assert(INTCONbits.GIE);
        System_ReadEvents(&tasks);
        assert(tasks == 0u);
    }
    assert(adc_completed_blocks == 625u);
}

static void test_altimeter(void) {
    bmp_available = 1u;
    pressure_pa = 100000.0f;
    Altimeter_Init(0u);
    assert(Altimeter_Data()->status == CALIBRATING);
    for (uint32_t i = 1u; i <= 37u; ++i) {
        Altimeter_Sample(i * 50u);
    }
    assert(Altimeter_Data()->status == RUNNING);
    pressure_pa = 90000.0f;
    Altimeter_Sample(1900u);
    assert(Altimeter_Data()->altitude_m > 0.0f);

    bmp_available = 0u;
    Altimeter_Sample(2000u);
    assert(Altimeter_Data()->status == SENSOR_FAULT);
    bmp_available = 1u;
    Altimeter_Sample(2999u);
    assert(Altimeter_Data()->status == SENSOR_FAULT);
    Altimeter_Sample(3000u);
    assert(Altimeter_Data()->status == CALIBRATING);

    /* Unstable calibration must restart without publishing a reference. */
    for (uint32_t i = 1u; i <= 37u; ++i) {
        pressure_pa = i % 2u ? 100000.0f : 100010.0f;
        Altimeter_Sample(3000u + i * 50u);
    }
    assert(Altimeter_Data()->status == CALIBRATING);
    assert(Altimeter_Data()->calibration_count == 0u);
}

static void test_motion_sensor(void) {
    sensor_address = 0x18u;
    sensor_registers[LIS3DH_WHO_AM_I] = LIS3DH_ID;
    const uint8_t bytes[] = {0x00, 0x80, 0xf0, 0xff, 0xf0, 0x7f};
    memcpy(&sensor_registers[LIS3DH_OUT_X_L], bytes, sizeof bytes);
    MotionSensor_Init(0u);
    assert(MotionSensor_Data()->ready);
    MotionSensor_Sample(10u);
    assert(MotionSensor_Data()->ax == -2048);
    assert(MotionSensor_Data()->ay == -1 && MotionSensor_Data()->az == 2047);
    bus_failed = 1u;
    MotionSensor_Sample(20u);
    assert(!MotionSensor_Data()->ready);
    bus_failed = 0u;
    MotionSensor_Sample(1019u);
    assert(!MotionSensor_Data()->ready);
    MotionSensor_Sample(1020u);
    assert(MotionSensor_Data()->ready);
}

int main(void) {
    test_adc();
    test_button();
    test_display();
    test_lis3dh();
    test_scheduler();
    test_altimeter();
    test_motion_sensor();
    puts("Part 3 ADC, scheduler, dashboard, button, and sensor checks passed.");
    return 0;
}
