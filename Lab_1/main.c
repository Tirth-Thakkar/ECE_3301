#include <xc.h>
#include "Configuration.h"
#define _XTAL_FREQ 64000000  // Set the system clock frequency (32 MHz for DM164136 with PIC18F48Q10)


void main(void) {
    // Configure Oscillator 64MHz
    OSCCON = 0b01110000;
    OSCTUNE= 0b01000000;
    // Set RA4 as output (1 = input, 0 = output)
    TRISAbits.TRISA4 = 0;  
    // Configure the microcontroller for LED control
    while (1) {
        LATAbits.LATA4 = 1;  // Turn on LED connected to RB0
        __delay_ms(500);       // Wait for 500 ms
        
        LATAbits.LATA4 = 0;  // Turn off LED
        __delay_ms(500);       // Wait for 500 ms
    }
}