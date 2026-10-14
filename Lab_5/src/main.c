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
    // 16 MHz Internal Oscillator
    OSCCONbits.IRCF = 0b111; 
    OSCCONbits.SCS = 0b10;
    OSCTUNEbits.PLLEN = 0;

    // Post & Pre-Scaler Config
    // T2OUTPS<3:0> = 0000, 1:1 Postscaler
    T2OUTPS0 = 0b0;
    T2OUTPS1 = 0b0;
    T2OUTPS2 = 0b0;
    T2OUTPS3 = 0b0;
    
    // T2CKPS <1:0> = 1X 16:1 Prescaler 
    T2CKPS0 = 0b1;
    T2CKPS1 = 0b1;   

    // Enable Timer2
    T2CONbits.TMR2ON = 1;
    PR2 = 249;

    // Inturrupt Config
    

}