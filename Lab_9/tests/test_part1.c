#include "../Part_1/include/ADC.h"
#include "../Part_1/include/Dashboard.h"
#include "../Part_1/include/StateController.h"
#include "../lib/include/Button.h"
#include "../lib/include/Indicators.h"
#include "../lib/include/LCD.h"
#include <assert.h>
#include <stdio.h>

static uint8_t button_event;
static uint8_t lower_nibble;
static uint16_t displayed;
static unsigned reads;

void ADC_Init(void) {}
uint16_t ADC_Read(void) {
    ++reads;
    return 902u;
}
void Button_Init(void) {}
uint8_t Button_Update(uint32_t tick) {
    uint8_t event = button_event;
    (void)tick;
    button_event = 0u;
    return event;
}
void LCD_Init(void) {}
void Indicators_Init(void) {}
void Indicators_ShowBits(uint16_t value, uint8_t lower) {
    assert(value == 0u || value == 902u);
    lower_nibble = lower;
}
void Dashboard_Update(uint16_t value) { displayed = value; }

int main(void) {
    fsm_init();
    fsm_run(99u);
    assert(reads == 0u && !lower_nibble && displayed == 0u);
    fsm_run(100u);
    assert(reads == 1u && displayed == 902u);

    button_event = BUTTON_SHORT_PRESS;
    fsm_run(101u);
    assert(lower_nibble && reads == 1u);
    fsm_run(199u);
    assert(lower_nibble && reads == 1u);
    button_event = BUTTON_LONG_PRESS;
    fsm_run(200u);
    assert(!lower_nibble && reads == 2u);

    /* Keep the 100 ms sampling interval correct across timestamp wraparound. */
    fsm_run(UINT32_MAX - 50u);
    unsigned before = reads;
    fsm_run(48u);
    assert(reads == before);
    fsm_run(49u);
    assert(reads == before + 1u);
    puts("Part 1 sampling and nibble-state checks passed.");
    return 0;
}
