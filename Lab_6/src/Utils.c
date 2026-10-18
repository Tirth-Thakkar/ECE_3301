#include <xc.h>
#include "../include/Utils.h"

void init_sys(void) {
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
    
    // Timer 2 Interrupt Config
    IPR1bits.TMR2IP = 0;
    
    // Set RA7 as output (1 = input, 0 = output)
    TRISAbits.TRISA7 = 0;
    ARMED_LED = 0; 

    // Baud Rate = FOSC / [16 × (SPBRG + 1)]
    // SPBRG = (16,000,000 / (16 × 9600)) - 1 = 103
    TXSTA1bits.BRGH = 1;
    
    TRISCbits.TRISC6 = 1; // Set RC6 as input
    TRISCbits.TRISC7 = 1; // Set RC7 as input
    TXSTA1bits.SYNC = 0; // Set asynchronous mode
    TXSTA1bits.TX9 = 0; // Set 8-bit transmission
    TXSTA1bits.BRGH = 1; // Set high baud rate
    TXSTA1bits.TXEN = 1; // Enable transmission
    
    RCSTA1bits.SPEN = 1; // Enable serial port
    RCSTA1bits.CREN = 1; // Enable continuous reception
    RCSTA1bits.RX9 = 0; // Set 8-bit reception

    BAUDCON1bits.BRG16 = 0;
    SPBRG1 = 103;

    TIMER_INTR_FLAG = 0;
    TIMER_INTR_EN = 1;
    
    return; 
}

void start_timer(void) {
    // Start Timer
    T2CONbits.TMR2ON = 1;
    INTCONbits.GIEL = 1;
}

void UART_SEND(char c) {
   // Check TSR Reg Status 
    while (!PIR1bits.TX1IF); 
    TXREG1 = c; // Load the character into the transmit register
}

void transmit_string(char *str) {
    while (*str != '\0') {
        UART_SEND(*str++);
    }
}


void transmit_timestap(const Timestamp *timestamp) {
    char hundreds = (timestamp->time / 100u) + '0';
    char tens = ((timestamp->time / 10u) % 10u) + '0';
    char ones = (timestamp->time % 10u) + '0';
    
    char int_buffer[] = {hundreds, tens, ones, '\0'};

    transmit_string(timestamp->code);
    transmit_string(int_buffer);
}

void transmit_response(const Response *response) {
    transmit_string(response->code);
    transmit_string(response->message);
}