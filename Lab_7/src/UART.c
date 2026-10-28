#include <xc.h>
#include "../include/UART.h"

void UART_Init(void)
{
    TRISCbits.TRISC6 = 1;
    TRISCbits.TRISC7 = 1;

    TXSTA1bits.SYNC = 0;
    TXSTA1bits.BRGH = 1;
    TXSTA1bits.TX9 = 0;
    BAUDCON1bits.BRG16 = 1;
    SPBRGH1 = 0;
    SPBRG1 = 34;
    RCSTA1bits.SPEN = 1;
    RCSTA1bits.CREN = 0;
    TXSTA1bits.TXEN = 1;
}

void UART_Send(char c)
{
    while (PIR1bits.TX1IF == 0)
    {
    }
    TXREG1 = c;
}

void UART_String(const char *text)
{
    while (*text != '\0')
    {
        UART_Send(*text++);
    }
}

void UART_Number(uint16_t value)
{
    char digits[5];
    uint8_t count = 0;

    do
    {
        digits[count++] = (char)('0' + (value % 10));
        value /= 10;
    } while (value != 0);

    while (count != 0)
    {
        UART_Send(digits[--count]);
    }
}

void UART_Hex8(uint8_t value)
{
    uint8_t digit;
    uint8_t i;

    for (i = 0; i < 2; i++)
    {
        digit = (value >> 4) & 0x0F;
        UART_Send((char)(digit < 10 ? '0' + digit : 'A' + digit - 10));
        value <<= 4;
    }
}

void UART_G(int16_t millig)
{
    uint16_t value;

    if (millig < 0)
    {
        UART_Send('-');
        value = (uint16_t)(-(int32_t)millig);
    }
    else
    {
        value = (uint16_t)millig;
    }

    UART_Number(value / 1000);
    UART_Send('.');
    UART_Send((char)('0' + (value % 1000) / 100));
    UART_Send((char)('0' + (value % 100) / 10));
    UART_Send((char)('0' + value % 10));
}
