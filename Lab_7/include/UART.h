#ifndef UART_H
#define UART_H

#include <stdint.h>

void UART_Init(void);
void UART_Send(char c);
void UART_String(const char *text);
void UART_Number(uint16_t value);
void UART_Hex8(uint8_t value);
void UART_G(int16_t millig);

#endif
