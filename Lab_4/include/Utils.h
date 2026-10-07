#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <xc.h>
#define _XTAL_FREQ 64000000

int button_engaged(int button_io_reg) {
    // Debounce the button
    static uint8_t prev_state = 0;
    uint8_t curr_state = !button_io_reg;

    if (curr_state != prev_state) {
        __delay_ms(20);
        curr_state = !button_io_reg;

        if (curr_state != prev_state) {
            prev_state = curr_state;
            return curr_state;
        }
    }

    return 0;
};

#endif // UTILS_H