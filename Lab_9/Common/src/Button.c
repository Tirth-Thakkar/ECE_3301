#include "../include/Button.h"
#include <xc.h>

#define DEBOUNCE_MS 20u
#define HOLD_MS 1000u

static uint8_t candidate;
static uint8_t stable;
static uint8_t hold_sent;
static uint32_t changed_ms;
static uint32_t pressed_ms;

void Button_Init(void) {
  ANSELBbits.ANSB4 = 0u;
  TRISBbits.TRISB4 = 1u;
  WPUBbits.WPUB4 = 1u;
  INTCON2bits.RBPU = 0u;
  candidate = 1u;
  stable = 1u; /* S1 is active-low. A held button is debounced at startup. */
  hold_sent = 0u;
  changed_ms = 0u;
  pressed_ms = 0u;
}

uint8_t Button_Update(uint32_t now_ms) {
  uint8_t raw = PORTBbits.RB4;
  uint8_t event = 0u;

  if (raw != candidate) {
    candidate = raw;
    changed_ms = now_ms;
  }
  if (candidate != stable && (uint32_t)(now_ms - changed_ms) >= DEBOUNCE_MS) {
    stable = candidate;
    if (stable == 0u) {
      pressed_ms = now_ms;
      hold_sent = 0u;
    } else if (!hold_sent) {
      event = BUTTON_SHORT_PRESS;
    }
  }
  if (stable == 0u && !hold_sent &&
      (uint32_t)(now_ms - pressed_ms) >= HOLD_MS) {
    hold_sent = 1u;
    event = BUTTON_LONG_PRESS;
  }
  return event;
}
