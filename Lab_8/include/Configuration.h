#ifndef ECE3301_LAB8_CONFIGURATION_H
#define ECE3301_LAB8_CONFIGURATION_H
/*
 * PIC18F46K22 device configuration for Curiosity HPC (DM164136).
 * Layout and config symbols follow the Lab_7/include/Configuration.h example.
 * IMPORTANT difference from Lab 7: this lab uses 16 MHz INTOSC, PLL OFF,
 * matching 1-ms Timer2 and the project-wide _XTAL_FREQ macro.
 * LVP must remain ON for the board's low-voltage programmer.
 * Include this header from exactly ONE C translation unit: main.c.
 */
#pragma config FOSC = INTIO67
#pragma config PLLCFG = OFF
#pragma config PRICLKEN = OFF
#pragma config FCMEN = OFF
#pragma config IESO = OFF
#pragma config PWRTEN = ON
#pragma config BOREN = SBORDIS
#pragma config BORV = 250
#pragma config WDTEN = OFF
#pragma config WDTPS = 32768
#pragma config CCP2MX = PORTC1
#pragma config PBADEN = OFF
#pragma config CCP3MX = PORTB5
#pragma config HFOFST = ON
#pragma config T3CMX = PORTC0
#pragma config P2BMX = PORTD2
#pragma config MCLRE = EXTMCLR
#pragma config STVREN = ON
#pragma config LVP = ON
#pragma config XINST = OFF
#pragma config CP0 = OFF
#pragma config CP1 = OFF
#pragma config CP2 = OFF
#pragma config CP3 = OFF
#pragma config CPB = OFF
#pragma config CPD = OFF
#pragma config WRT0 = OFF
#pragma config WRT1 = OFF
#pragma config WRT2 = OFF
#pragma config WRT3 = OFF
#pragma config WRTC = OFF
#pragma config WRTB = OFF
#pragma config WRTD = OFF
#pragma config EBTR0 = OFF
#pragma config EBTR1 = OFF
#pragma config EBTR2 = OFF
#pragma config EBTR3 = OFF
#pragma config EBTRB = OFF
#endif
