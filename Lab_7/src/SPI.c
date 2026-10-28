#include "../include/SPI.h"
#include <xc.h>

void SPI_Init(void) {
  TRISCbits.TRISC3 = 0;
  TRISCbits.TRISC4 = 1;
  TRISCbits.TRISC5 = 0;

  LATDbits.LATD0 = 1;
  TRISDbits.TRISD0 = 0;

  SSP1CON1bits.SSPEN = 0;
  SSP1STATbits.SMP = 1;
  SSP1STATbits.CKE = 0;
  SSP1CON1bits.CKP = 1;
  SSP1CON1bits.SSPM = 0b0010;
  SSP1CON1bits.SSPEN = 1;
}

uint8_t SPI_Transfer(uint8_t data) {
  SSP1BUF = data;
  while (SSP1STATbits.BF == 0) {
  }
  return SSP1BUF;
}
