#include <xc.h>
#include "../include/Configuration.h"

void main(void)
{
    unsigned int count = 0;
    unsigned char state = 0;

    OSCCONbits.IRCF = 0b111;            // 16MHz
    OSCCONbits.SCS = 0b10;              // Internal Oscilltor

    ANSELB = 0x00;
    ANSELD = 0x00;

    TRISB = 0x00;
    TRISD = 0x00;

                                        // Start with East/West green
    LATD = 0x09;
    LATB = 0x24;

    T0CONbits.TMR0ON = 0;
    T0CONbits.T08BIT = 0;
    T0CONbits.T0CS = 0;
    T0CONbits.PSA = 0;
    T0CONbits.T0PS = 0b010;

    TMR0H = 0x3C;                       // 65536 - 15536 = 50000 counts
    TMR0L = 0xB0;                       // TMR = 60535

    
    INTCONbits.TMR0IF = 0;              // Clear overflow flag

    T0CONbits.TMR0ON = 1;               // Start Timer0

    while(1)
    {
                                        // Check if 50,000 Timer0 counts have passed
        if(INTCONbits.TMR0IF == 1)      // Turns to 1 once TMR0 overflows
        {
            INTCONbits.TMR0IF = 0;      // Resets to 0

            TMR0H = 0x3C;
            TMR0L = 0xB0;               // Set TMR0 back to 15536

            count++;

                                        // East/West green for 6 seconds
            if(state == 0 && count == 60)
            {
                LATD = 0x12;
                LATB = 0x12;

                count = 0;
                state = 1;
            }

            else if(state == 1 && count == 30)
            {
                LATD = 0x24;            // Yellow for 3 seconds
                LATB = 0x09;

                count = 0;
                state = 2;
            }

                                        
            else if(state == 2 && count == 60)
            {
                LATD = 0x12;            // North/South green for 6 seconds
                LATB = 0x12;

                count = 0;
                state = 3;
            }

            else if(state == 3 && count == 30)
            {
                LATD = 0x09;            // Yellow for 3 seconds
                LATB = 0x24;

                count = 0;
                state = 0;
            }
        }
    }
}