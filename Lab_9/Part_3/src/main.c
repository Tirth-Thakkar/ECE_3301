#include "../../Common/include/Configuration.h"
#include "../include/ADC.h"
#include "../include/StateController.h"
#include "../include/System.h"
#include <xc.h>

void main(void) {
  System_Init();
  fsm_init();
  System_StartTimer();

  while (1) {
    uint8_t tasks;
    uint32_t tick = System_ReadEvents(&tasks);
    fsm_run(tick, tasks);
  }
}

void __interrupt() isr(void) {
  /* Collect an old result before the timer can start another conversion. */
  if (PIE1bits.ADIE && PIR1bits.ADIF) {
    ADC_Interrupt();
  }
  System_TimerInterrupt();
}
