#include <xc.h>
#include <stdint.h>
#include "../include/Configuration.h"
#include "../include/StateController.h"

volatile uint8_t sys_tick = 0;

void gen_sys_tick(uint8_t); 

void main(void) {
    fsm_init();
    while(1){
        gen_sys_tick(sys_tick);
        fsm_run(sys_tick);
    }
}

// TODO Complete w/ Hardware Timer
void gen_sys_tick(uint8_t tick) {
    tick++;
    sys_tick = tick;
}