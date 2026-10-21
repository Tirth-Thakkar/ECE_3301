#include <xc.h>
#include <string.h>
#include "../include/Utils.h"

void init_sys(void) {
    // Disable Interrupts
    INTCONbits.GIE = 0;
    INTCONbits.PEIE = 0;

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
    
    // Set RA7 as output (1 = input, 0 = output)
    ARMED_LED = 0; 
    TRISAbits.TRISA7 = 0;

    // Baud Rate = FOSC / [16 × (SPBRG + 1)]
    // SPBRG = (16,000,000 / (16 × 9600)) - 1 = 103
    ANSELCbits.ANSC6 = 0; // Enable digital I/O on RC6
    ANSELCbits.ANSC7 = 0; // Enable digital I/O on RC7
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
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
}

void UART_SEND(char c) {
    // Check Transmit Buffer Status
    while (!PIR1bits.TX1IF); 
    TXREG1 = c; // Load the character into the transmit register
}

uint8_t UART_RECEIVE(char *c) {
    // Check Receive Buffer Status
    if (!PIR1bits.RC1IF) {
        return 0;
    }

    *c = RCREG1;
    return 1;
}

void transmit_string(char *str) {
    while (*str != '\0') {
        UART_SEND(*str++);
    }
}

void transmit_timestamp(const Timestamp *timestamp) {
    char int_buffer[11] = {0};
    uint32_t time = timestamp->time;
    uint8_t index = sizeof(int_buffer) - 1;

    do {
        int_buffer[--index] = (char)(time % 10u) + '0';
        time /= 10u;
    } while (time != 0);

    transmit_string(timestamp->code);
    transmit_string(&int_buffer[index]);
    
    UART_SEND('\r'); 
    UART_SEND('\n');
}

void transmit_response(const Response *response) {
    transmit_string(response->code);
    transmit_string(response->message);

    UART_SEND('\r'); 
    UART_SEND('\n');
}

uint8_t response_callback(Response *err_response, char *input_buffer) {
    char valid_input_u[] = "SAD";
    char valid_input_l[] = "sad";

    if (RCSTA1bits.OERR) {
        RCSTA1bits.CREN = 0; // Clear the overrun error
        RCSTA1bits.CREN = 1; // Re-enable continuous reception
        err_response->code = "$ERR, ";
        err_response->message = "Overrun Error";
        transmit_response(err_response);
        return 1;
    }

    if (!PIR1bits.RC1IF) {
        return 0;
    }

    // FERR belongs to the next unread character, so check it before RCREG1.
    uint8_t framing_error = RCSTA1bits.FERR;
    if (!UART_RECEIVE(input_buffer)) {
        return 0;
    }

    if (framing_error) {
        err_response->code = "$ERR, ";
        err_response->message = "Framing Error";
        transmit_response(err_response);
        return 1;
    }

    // Ignore line endings added by the serial terminal.
    if (input_buffer[0] == '\r' || input_buffer[0] == '\n') {
        return 0;
    }

    if (input_buffer[0] != '\0' && (strchr(valid_input_u, input_buffer[0]) || strchr(valid_input_l, input_buffer[0]))) {
        err_response->code = "$ACK, ";
        err_response->message = "";
    } else {
        err_response->code = "$ERR, ";
        err_response->message = "CMD";
        transmit_response(err_response);
    }

    return 1;
}
