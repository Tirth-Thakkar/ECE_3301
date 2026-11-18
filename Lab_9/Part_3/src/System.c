#include "../include/System.h"
#include "../include/ADC.h"
#include <xc.h>

#define SENSOR_PERIOD_MS 50u
#define MOTION_PERIOD_MS 100u
#define DISPLAY_PERIOD_MS 250u
#define IMU_PERIOD_MS 10u

static volatile uint32_t ticks;
static volatile uint8_t task_flags;
static volatile uint8_t sensor_tick;
static volatile uint8_t motion_tick;
static volatile uint8_t display_tick;
static volatile uint8_t imu_tick;

void System_Init(void) {
    INTCONbits.GIE = 0u;
    INTCONbits.PEIE = 0u;
    RCONbits.IPEN = 0u;
    OSCCONbits.IRCF = 7u;
    OSCCONbits.SCS = 2u;
    OSCTUNEbits.PLLEN = 0u;

    T2CON = 0u;
    TMR2 = 0u;
    PR2 = 249u; // Timer2: 16 MHz / 4 / 16 / 250 = 1 kHz.
    PIR1bits.TMR2IF = 0u;
    PIE1bits.TMR2IE = 1u;
    ticks = 0u;
    task_flags = 0u;
    sensor_tick = 0u;
    motion_tick = 0u;
    display_tick = 0u;
    imu_tick = 0u;
}

void System_StartTimer(void) {
    // Required ADC interrupt enable order: ADIF, ADIE, PEIE, then GIE.
    PIR1bits.ADIF = 0u;
    PIE1bits.ADIE = 1u;
    T2CON = 0x06u;
    INTCONbits.PEIE = 1u;
    INTCONbits.GIE = 1u;
}

void System_TimerInterrupt(void) {
    if (!PIE1bits.TMR2IE || !PIR1bits.TMR2IF) {
        return;
    }

    PIR1bits.TMR2IF = 0u;
    ++ticks;
    ADC_Start();

    if (++sensor_tick >= SENSOR_PERIOD_MS) {
        sensor_tick = 0u;
        task_flags |= TASK_SENSOR;
    }
    if (++motion_tick >= MOTION_PERIOD_MS) {
        motion_tick = 0u;
        task_flags |= TASK_MOTION;
    }
    if (++display_tick >= DISPLAY_PERIOD_MS) {
        display_tick = 0u;
        task_flags |= TASK_DISPLAY;
    }
    if (++imu_tick >= IMU_PERIOD_MS) {
        imu_tick = 0u;
        task_flags |= TASK_IMU;
    }
}

uint32_t System_ReadEvents(uint8_t *tasks) {
    uint8_t enabled = INTCONbits.GIE;
    uint32_t now;

    INTCONbits.GIE = 0u;
    now = ticks;
    *tasks = task_flags;
    task_flags = 0u;
    INTCONbits.GIE = enabled;
    return now;
}
