#include <xc.h>
#include "../include/Utils.h"
#include "../include/Configuration.h"

volatile uint32_t system_ms = 0b0;
uint32_t sys_tick = 0;

void gen_sys_tick(void);

void main(void) {
    init_sys(); 
    fsm_init();
    start_timer();
    while (1) {
        gen_sys_tick(); 
        fsm_run(sys_tick);
    }
}

void __interrupt() isr(void) {
    if (TIMER_INTR_EN && TIMER_INTR_FLAG) {
        TIMER_INTR_FLAG = 0;
        system_ms++;
    }
}

void gen_sys_tick(void) {
    uint8_t interrupts_enabled = INTCONbits.GIE;
    INTCONbits.GIE = 0;
    sys_tick = system_ms;
    INTCONbits.GIE = interrupts_enabled;
}