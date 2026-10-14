#include <xc.h>
#include <stdint.h>
#include "../include/Configuration.h"
#include "../include/StateController.h"

#define _XTAL_FREQ 16000000

#define FAULT_INTR_EN  INTCONbits.INT0IE 
#define FAULT_INTR_FLAG INTCONbits.INT0IF

#define ARM_INTR_EN    INTCON3bits.INT1IE
#define ARM_INTR_FLAG  INTCON3bits.INT1IF

#define COMM_INTR_EN   INTCON3bits.INT2IE
#define COMM_INTR_FLAG INTCON3bits.INT2IF 

#define TIMER_INTR_EN  PIE1bits.TMR2IE
#define TIMER_INTR_FLAG PIR1bits.TMR2IF

volatile uint32_t system_ms = 0b0;
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
    if (TIMER_INTR_EN && TIMER_INTR_FLAG) {
        TIMER_INTR_FLAG = 0;
        system_ms++;
    }
    
    if (ARM_INTR_EN && ARM_INTR_FLAG) {
        ARM_INTR_FLAG = 0;
        arm_req = 0b1;
    }
    
    if (COMM_INTR_EN && COMM_INTR_FLAG) {
        COMM_INTR_FLAG = 0;
        comm_req = 0b1;
    }
}

void __interrupt(high_priority) high_isr(void) {
    if (FAULT_INTR_EN && FAULT_INTR_FLAG) {
        FAULT_INTR_FLAG = 0;
        fault_status = 0b1;
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

    // Note: Intr 0: Fault, Intr 1: Arm Req, Intr 2: Heartbeat 
    
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
    FAULT_INTR_FLAG = 0;
    ARM_INTR_FLAG = 0;
    COMM_INTR_FLAG = 0;
    TIMER_INTR_FLAG = 0;

    FAULT_INTR_EN = 1;
    ARM_INTR_EN = 1;
    COMM_INTR_EN = 1;
    TIMER_INTR_EN = 1;

}

void start_timer(void) {
    // Start Timer
    T2CONbits.TMR2ON = 1;
    INTCONbits.GIEL = 1;
    INTCONbits.GIEH = 1;
}
