#include "../include/System.h"
#include <xc.h>

static volatile uint32_t ticks;

void System_Init(void) {
    INTCONbits.GIE = 0u;
    INTCONbits.PEIE = 0u;
    RCONbits.IPEN = 0u;
    OSCCONbits.IRCF = 7u;
    OSCCONbits.SCS = 2u;
    OSCTUNEbits.PLLEN = 0u;

    T2CON = 0u;
    TMR2 = 0u;
    PR2 = 249u; // 16 MHz / 4 / 16 / 250 = 1 kHz, as in Lab 8.
    PIR1bits.TMR2IF = 0u;
    PIE1bits.TMR2IE = 1u;
    ticks = 0u;
}

void System_StartTimer(void) {
    T2CON = 0x06u;
    INTCONbits.PEIE = 1u;
    INTCONbits.GIE = 1u;
}

void System_TimerInterrupt(void) {
    if (PIE1bits.TMR2IE && PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0u;
        ++ticks;
    }
}

uint32_t System_Millis(void) {
    uint8_t enabled = INTCONbits.GIE;
    uint32_t now;

    INTCONbits.GIE = 0u;
    now = ticks;
    INTCONbits.GIE = enabled;
    return now;
}
