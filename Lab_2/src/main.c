#include <xc.h>
#include "../include/Configuration.h"
#define _XTAL_FREQ 64000000  // Set the system clock frequency (32 MHz for DM164136 with PIC18F48Q10)


void main (void){

    // Configure Oscillator 64MHz
    OSCCON = 0b01110000;
    OSCTUNE= 0b01000000;

    while(1){
        __delay_ms(1000);
    }

}