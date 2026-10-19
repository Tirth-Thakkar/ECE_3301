#ifndef UTILS_H
#define UTILS_H

#define _XTAL_FREQ 64000000  
#define ARMED_LED LATAbits.LATA7
#define TIMER_INTR_EN  PIE1bits.TMR2IE
#define TIMER_INTR_FLAG PIR1bits.TMR2IF

#include "../include/StateController.h"

void init_sys(void);

void start_timer(void);

void UART_SEND(char c);

void UART_RECEIVE(char *c);

void transmit_string(char *str); 

void transmit_timestamp(const Timestamp *timestamp);

void transmit_response(const Response *response);

void response_callback(Response *response, char *input_buffer);

#endif // UTILS_H