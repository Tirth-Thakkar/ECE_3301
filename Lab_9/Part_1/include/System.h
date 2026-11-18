#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

void System_Init(void);
void System_StartTimer(void);
void System_TimerInterrupt(void);
uint32_t System_Millis(void);

#endif
