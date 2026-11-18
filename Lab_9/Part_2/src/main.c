#include "../../Common/include/Configuration.h"
#include "../include/StateController.h"
#include "../include/System.h"
#include <xc.h>

void main(void) {
  System_Init();
  fsm_init();
  System_StartTimer();

  while (1) {
    fsm_run(System_Millis());
  }
}
