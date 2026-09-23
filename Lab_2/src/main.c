#include <xc.h>
#include <stdlib.h>
#include <stdint.h>
#include "../include/Configuration.h"

#define _XTAL_FREQ 64000000  // Set the system clock frequency (32 MHz for DM164136 with PIC18F48Q10)


void init(void) {
    // Configure Oscillator 64MHz
    OSCCON = 0b01110000;
    OSCTUNE= 0b01000000;
    
    // Configure Inputs  
    TRISBbits.TRISB4 = 0x01; // HPC Button
    
    // Configure Ports C and D as outputs
    TRISC = 0x00;
    TRISD = 0x00;

    // Init Vals
    PORTC = 0x00;
    PORTD = 0x00;

    // Seed Random
    srand(TMR0);
};


int button_engaged(void) {
    // Debounce the button
    static uint8_t prev_state = 0;
    uint8_t curr_state = !PORTBbits.RB4;

    if (curr_state != prev_state) {
        __delay_ms(20);
        curr_state = !PORTBbits.RB4;

        if (curr_state != prev_state) {
            prev_state = curr_state;
            return curr_state;
        }
    }

    return 0;
};

void set_led_arr(void) {
    // Hex Values for D6
    uint8_t val_map[] = {0x08, 0x22, 0x22, 0x2A, 0x55, 0x5D, 0x77};

    uint8_t val_c = (uint8_t) (rand() % 6 + 1);
    uint8_t val_d = (uint8_t) (rand() % 6 + 1);
    
    // Set val
    PORTC = val_map[val_c-1];
    PORTD = val_map[val_d-1];

    __delay_ms(3000);

    // Clear DSP
    PORTC = 0x00;
    PORTD = 0x00;

};

void main (void){    
    
    init();
    
    while(1){
        if (button_engaged()){ 
            set_led_arr();
        }
    }
}
