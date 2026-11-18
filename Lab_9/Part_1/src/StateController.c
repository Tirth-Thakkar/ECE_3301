#include "../include/StateController.h"
#include "../../lib/include/Button.h"
#include "../../lib/include/Indicators.h"
#include "../../lib/include/LCD.h"
#include "../include/ADC.h"

#include <stdio.h>

#define SAMPLE_PERIOD_MS 100u

static StateController fsm_t;

static void process_button(uint32_t tick) {
    uint8_t event = Button_Update(tick);

    if (event & (BUTTON_SHORT_PRESS | BUTTON_LONG_PRESS)) {
        switch (fsm_t.current_state) {
        case SHOW_HIGH_BITS:
            fsm_t.current_state = SHOW_LOW_BITS;
            break;
        case SHOW_LOW_BITS:
            fsm_t.current_state = SHOW_HIGH_BITS;
            break;
        }
    }
}

static void sample_adc(uint32_t tick) {
    if ((uint32_t)(tick - fsm_t.last_sample_ms) < SAMPLE_PERIOD_MS) {
        return;
    }
    fsm_t.last_sample_ms = tick;
    fsm_t.adc_value = ADC_Read();
    Dashboard_Update(fsm_t.adc_value);
}

void fsm_init(void) {
    fsm_t.current_state = SHOW_HIGH_BITS;
    fsm_t.last_sample_ms = 0u;
    fsm_t.adc_value = 0u;

    Button_Init();
    Indicators_Init();
    LCD_Init();
    ADC_Init();
    Dashboard_Update(fsm_t.adc_value);
}

void fsm_run(uint32_t tick) {
    process_button(tick);
    sample_adc(tick);
    Indicators_ShowBits(fsm_t.adc_value, fsm_t.current_state == SHOW_LOW_BITS);
}

void Dashboard_Update(uint16_t value) {
    char top[LCD_COLS + 1u];
    char bits[11];

    Indicators_FormatBinary(value, bits);
    snprintf(top, sizeof top, "ADC:%04u", (unsigned int)value);
    LCD_PrintLine(0u, top);
    LCD_PrintLine(1u, bits);
}