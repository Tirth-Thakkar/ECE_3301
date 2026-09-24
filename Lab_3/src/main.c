#include <xc.h>
#include <stdlib.h>
#include <stdint.h>
#include "../include/Configuration.h"

#define _XTAL_FREQ 64000000  // Set the system clock frequency (32 MHz for DM164136 with PIC18F48Q10)

uint8_t intr_cnt = 0x00;

void init(void){
    // Configure Oscillator 64MHz
    OSCCON = 0b01110000;
    OSCTUNE= 0b01000000;
    
    // Configure Outputs
    TRISD = 0x00;
    TRISB = 0x00;

    LATD = 0x00;
    LATB = 0x00;
};

void set_light(void){
    uint8_t val_D = {0x09, 0x12, 0x24, 0x12};
    uint8_t val_B = {0x24, 0x12, 0x09, 0x12};
}

void main(void){
    init();
}