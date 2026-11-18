#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

#define TASK_SENSOR 0x01u
#define TASK_MOTION 0x02u
#define TASK_DISPLAY 0x04u
#define TASK_IMU 0x08u

void System_Init(void);
void System_StartTimer(void);
void System_TimerInterrupt(void);
uint32_t System_ReadEvents(uint8_t *tasks);

#endif
