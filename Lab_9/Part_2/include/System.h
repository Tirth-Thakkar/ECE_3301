#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

void System_Init(void);
void System_StartTimer(void);
uint32_t System_Millis(void);

#endif
