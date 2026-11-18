#include "../include/System.h"
#include <xc.h>

static uint32_t ticks;

void System_Init(void) {
    INTCONbits.GIE = 0u;
    INTCONbits.PEIE = 0u;
    PIE1bits.ADIE = 0u;
    PIE1bits.TMR2IE = 0u;
    RCONbits.IPEN = 0u;
    OSCCONbits.IRCF = 7u;
    OSCCONbits.SCS = 2u;
    OSCTUNEbits.PLLEN = 0u;

    T2CON = 0u;
    TMR2 = 0u;
    PR2 = 249u;
    PIR1bits.TMR2IF = 0u;
    ticks = 0u;
}

void System_StartTimer(void) { T2CON = 0x06u; }

uint32_t System_Millis(void) {
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0u;
        ++ticks;
    }
    return ticks;
}
