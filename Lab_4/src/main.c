#include <xc.h>
#include <stdint.h>
#include "../include/Configuration.h"
#include "../include/StateController.h"
#define XTAL_FREQ 64000000
#define TMR0_PRELOAD_H 0x3C
#define TMR0_PRELOAD_L 0xB0

volatile uint8_t sys_tick = 0;

void gen_sys_tick(void); 
void configure_sys(void);

void main(void) {
    configure_sys();
    fsm_init();
    while(1){
        gen_sys_tick();
        monitor_safety();
        fsm_run(sys_tick);
    }
}

void gen_sys_tick(void) {
    // Check Overflow
    if (INTCONbits.TMR0IF) {
        TMR0H = TMR0_PRELOAD_H; 
        TMR0L = TMR0_PRELOAD_L;
        // Clear the overflow flag
        INTCONbits.TMR0IF = 0; 
        sys_tick++; 
    }
}

void configure_sys(void) {
    // Input & Output Config
    TRISB = 0x01; // Configure B as input
    ANSELB = 0x00; // Disable Analog 
    TRISD = 0x00; // Configure D as output 

    // System Timer Config
    OSCCONbits.IRCF = 0b111; // 16 MHz Internal Oscillator
    OSCCONbits.SCS = 0b10; // System Clock Source

    T0CONbits.TMR0ON = 1; // Enable Timer 1
    T0CONbits.T08BIT = 0; // 16 bit mode
    T0CONbits.T0CS = 0; // Internal instruction cycle clock
    T0CONbits.PSA = 0; // Enable Prescaler
    T0CONbits.T0PS = 0b010; // 1:8 Prescaler
}