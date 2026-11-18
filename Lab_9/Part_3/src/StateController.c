#include "../include/StateController.h"
#include "../../lib/include/Button.h"
#include "../../lib/include/Indicators.h"
#include "../../lib/include/LCD.h"
#include "../include/Altimeter.h"
#include "../include/Dashboard.h"
#include "../include/MotionSensor.h"
#include "../include/System.h"

static StateController fsm_t;

static void process_button(uint32_t tick) {
    uint8_t event = Button_Update(tick);

    if (event & BUTTON_SHORT_PRESS) {
        switch (fsm_t.current_state) {
        case FLIGHT_VIEW:
            fsm_t.current_state = ENGINEER_VIEW;
            break;
        case ENGINEER_VIEW:
            fsm_t.current_state = FLIGHT_VIEW;
            break;
        }
    }
    if (event & BUTTON_LONG_PRESS) {
        fsm_t.averaging = !fsm_t.averaging;
        ADC_SetAveraging(fsm_t.averaging);
    }
}

static void update_display(uint32_t tick) {
    switch (fsm_t.current_state) {
    case FLIGHT_VIEW:
        Dashboard_Flight(Altimeter_Data(), MotionSensor_Data(), tick);
        break;
    case ENGINEER_VIEW:
        Dashboard_Engineer(&fsm_t.adc, fsm_t.averaging);
        break;
    }
}

static void run_sensor_tasks(uint32_t tick, uint8_t tasks) {
    if (tasks & TASK_SENSOR) {
        Altimeter_Sample(tick);
    }
    if (tasks & TASK_MOTION) {
        Altimeter_UpdateMotion();
    }
    if (tasks & TASK_IMU) {
        MotionSensor_Sample(tick);
    }
}

void fsm_init(void) {
    fsm_t.current_state = FLIGHT_VIEW;
    fsm_t.averaging = 1u;
    fsm_t.adc.value = 0u;
    fsm_t.adc.peak_to_peak = 0u;

    Button_Init();
    Indicators_Init();
    LCD_Init();
    ADC_Init();
    Altimeter_Init(0u);
    MotionSensor_Init(0u);
    update_display(0u);
}

void fsm_run(uint32_t tick, uint8_t tasks) {
    process_button(tick);
    ADC_TakeBlock(&fsm_t.adc);
    Indicators_ShowBits(fsm_t.adc.value, fsm_t.current_state == ENGINEER_VIEW);
    run_sensor_tasks(tick, tasks);

    if (tasks & TASK_DISPLAY) {
        update_display(tick);
    }
}
