#ifndef UTILS_H
#define UTILS_H

#include <xc.h>
#include "../include/Configuration.h"  
#include <stdint.h>
#include <stdlib.h>
#define _XTAL_FREQ 64000000  
#define ARMED_LED LATAbits.LATA7

void init_sys(void) {
    // Configure Oscillator 64MHz
    OSCCON = 0b01110000;
    OSCTUNE= 0b01000000;
    
    // Set RA7 as output (1 = input, 0 = output)
    TRISAbits.TRISA7 = 0;
    ARMED_LED = 0; 

    // Baud Rate = FOSC / [16 × (SPBRG + 1)]
    // SPBRG = (16,000,000 / (16 × 9600)) - 1 = 103
    TXSTA1bits.BRGH = 1;
    
    TRISCbits.TRISC6 = 1; // Set RC6 as input
    TRISCbits.TRISC7 = 1; // Set RC7 as input
    TXSTA1bits.SYNC = 0; // Set asynchronous mode
    TXSTA1bits.BRGH = 1; // Set high baud rate
    TXSTA1bits.TXEN = 1; // Enable transmission
    RCSTA1bits.SPEN = 1; // Enable serial port
    RCSTA1bits.CREN = 1; // Enable continuous reception

    BAUDCON1bits.BRG16 = 0;
    SPBRG1 = 103;
    
    return; 
}

void transmit_char(char c) {
   // Check TSR Reg Status 
    while (!PIR1bits.TX1IF); 
    TXREG1 = c; // Load the character into the transmit register
}

void trasnmit_string(char *str) {
    while (*str != '\0') {
        transmit_char(*str++);
    }
}

void transmit_int(uint8_t num) {
    return;
}

#endif // UTILS_H