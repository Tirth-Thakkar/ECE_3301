#include <xc.h>
#include <stdint.h>
#include "../include/Configuration.h"
#include "../include/StateController.h"
volatile uint32_t system_ms = 0;
uint32_t sys_tick = 0;

void gen_sys_tick(void); 
void configure_sys(void);
void start_timer(void);

void main(void) {
    configure_sys();
    fsm_init();
    start_timer();
    while(1){
        gen_sys_tick();
        monitor_safety();
        fsm_run(sys_tick);
    }
}

void __interrupt(low_priority) low_isr(void) {
    if (PIE1bits.TMR2IE && PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;
        system_ms++;
    }
}

void gen_sys_tick(void) {
    uint8_t low_interrupts_enabled = INTCONbits.GIEL;
    INTCONbits.GIEL = 0;
    sys_tick = system_ms;
    INTCONbits.GIEL = low_interrupts_enabled;
}

void configure_sys(void) {
    // Disable Interrupts
    INTCONbits.GIEH = 0;
    INTCONbits.GIEL = 0;

    // Input & Output Config
    ANSELB = 0x00; // Disable Analog
    TRISB = 0x07;  // Configure B as input (RB0, RB1, RB2)
    WPUB = 0x07;   // Keep active-low button 
    INTCON2bits.RBPU = 0;
    
    ANSELD = 0x00; // Disable Analog
    LATD = 0x00;   // Zero outputs
    TRISD = 0x00; // Configure D as output

    // System Timer Config
    // 16 MHz Internal Oscillator
    OSCCONbits.IRCF = 0b111; 
    OSCCONbits.SCS = 0b10;
    OSCTUNEbits.PLLEN = 0;

    // Post & Pre-Scaler Config
    T2CONbits.TMR2ON = 0;
    T2CONbits.T2OUTPS = 0b0000; // 1:1 postscaler
    T2CONbits.T2CKPS = 0b10;    // 1:16 prescaler
    TMR2 = 0;
    PR2 = 249;

    // Falling Edge Button Config (Active Low)
    INTCON2bits.INTEDG0 = 0;
    INTCON2bits.INTEDG1 = 0;
    INTCON2bits.INTEDG2 = 0;

    // Interrupt Priority Config
    RCONbits.IPEN = 1;
    INTCON3bits.INT1IP = 0;
    INTCON3bits.INT2IP = 0;
    IPR1bits.TMR2IP = 0;

    // Clear Pending Flags & Enable Interrupts
    INTCONbits.INT0IF = 0;
    INTCON3bits.INT1IF = 0;
    INTCON3bits.INT2IF = 0;
    PIR1bits.TMR2IF = 0;

    INTCONbits.INT0IE = 1;
    INTCON3bits.INT1IE = 1;
    INTCON3bits.INT2IE = 1;
    PIE1bits.TMR2IE = 1;

}

void start_timer(void) {
    // Start Timer
    T2CONbits.TMR2ON = 1;
    INTCONbits.GIEL = 1;
    INTCONbits.GIEH = 1;
}
