#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

#define BUTTON_SHORT_PRESS 0x01u
#define BUTTON_LONG_PRESS 0x02u

void Button_Init(void);
uint8_t Button_Update(uint32_t now_ms);

#endif
