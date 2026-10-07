#include <xc.h>
#include <stdint.h>
#include "../include/Configuration.h"
#include "../include/StateController.h"
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
    sys_tick = 0;
    // Check Overflow
    if (INTCONbits.TMR0IF) {
        TMR0H = TMR0_PRELOAD_H; 
        TMR0L = TMR0_PRELOAD_L;
        // Clear the overflow flag
        INTCONbits.TMR0IF = 0; 
        sys_tick = 1;
    }
}

void configure_sys(void) {
    // Input & Output Config
    ANSELB = 0x00; // Disable Analog
    TRISB = 0x07;  // Configure B as input (RB0, RB1, RB2)
    WPUB = 0x07;   // Keep active-low button 
    INTCON2bits.RBPU = 0;
    
    ANSELD = 0x00; // Disable Analog
    LATD = 0x00;   // Zero outputs
    TRISD = 0x00; // Configure D as output

    // System Timer Config
    OSCCONbits.IRCF = 0b111; // 16 MHz Internal Oscillator
    OSCCONbits.SCS = 0b10; // System Clock Source
    OSCTUNEbits.PLLEN = 0; // Internal clock at 16 MHz

    T0CONbits.TMR0ON = 0; // Configure Timer0 
    T0CONbits.T08BIT = 0; // 16 bit mode
    T0CONbits.T0CS = 0; // Internal instruction cycle clock
    T0CONbits.PSA = 0; // Enable Prescaler
    T0CONbits.T0PS = 0b010; // 1:8 Prescaler
    TMR0H = TMR0_PRELOAD_H;
    TMR0L = TMR0_PRELOAD_L;
    INTCONbits.TMR0IF = 0;
    T0CONbits.TMR0ON = 1;
}