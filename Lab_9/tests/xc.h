#ifndef TEST_XC_H
#define TEST_XC_H

/* Register model for host tests only. XC8 builds use the real device header. */
#include <stdint.h>

typedef union {
  uint8_t value;
  struct {
    unsigned char ADON : 1;
    unsigned char GO_nDONE : 1;
    unsigned char CHS : 5;
  } bits;
} MockADC;

typedef union {
  uint8_t value;
  struct {
    unsigned char ANSA0 : 1;
    unsigned char ANSA1 : 1;
  } bits;
} MockAnalogA;

typedef union {
  uint8_t value;
  struct {
    unsigned char TRISA0 : 1;
    unsigned char TRISA1 : 1;
  } bits;
} MockDirectionA;

extern MockADC mock_adc;
extern MockAnalogA mock_ansela;
extern MockDirectionA mock_trisa;
#define ADCON0 mock_adc.value
#define ADCON0bits mock_adc.bits
#define ANSELA mock_ansela.value
#define ANSELAbits mock_ansela.bits
#define TRISA mock_trisa.value
#define TRISAbits mock_trisa.bits

extern uint8_t ADCON1, ADCON2, ADRESH, ADRESL, LATA, T2CON, TMR2, PR2;
extern struct MockInterrupt {
  uint8_t GIE;
  uint8_t PEIE;
} INTCONbits;
extern struct MockADCEnable {
  uint8_t ADIE;
  uint8_t TMR2IE;
} PIE1bits;
extern struct MockADCFlag {
  uint8_t ADIF;
  uint8_t TMR2IF;
} PIR1bits;
extern struct MockAnalogB {
  uint8_t ANSB4;
} ANSELBbits;
extern struct MockDirectionB {
  uint8_t TRISB4;
} TRISBbits;
extern struct MockPullupB {
  uint8_t WPUB4;
} WPUBbits;
extern struct MockPullupEnable {
  uint8_t RBPU;
} INTCON2bits;
extern struct MockPortB {
  uint8_t RB4;
} PORTBbits;

extern struct MockPriority {
  uint8_t IPEN;
} RCONbits;
extern struct MockClock {
  uint8_t IRCF;
  uint8_t SCS;
} OSCCONbits;
extern struct MockPLL {
  uint8_t PLLEN;
} OSCTUNEbits;

#define __delay_us(us) ((void)(us))

#endif
