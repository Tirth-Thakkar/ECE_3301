#include "../include/StateController.h"
#include "../../Common/include/Button.h"
#include "../../Common/include/Indicators.h"
#include "../../Common/include/LCD.h"
#include "../include/ADC.h"
#include "../include/Dashboard.h"

#define SAMPLE_PERIOD_MS 100u

static StateController fsm_t;
uint16_t experiment_samples[ADC_BURST_SAMPLES];

static const char *experiment_name(void) {
  switch (fsm_t.current_state) {
  case EXPERIMENT_A_NORMAL:
    return "A:NORM";
  case EXPERIMENT_A_FAST:
    return "A:FAST";
  case EXPERIMENT_B_REFERENCE:
    return "B:REF";
  case EXPERIMENT_B_0_TAD:
    return "B:0";
  case EXPERIMENT_B_2_TAD:
    return "B:2";
  case EXPERIMENT_B_8_TAD:
    return "B:8";
  case EXPERIMENT_B_20_TAD:
    return "B:20";
  default:
    return "";
  }
}

void fsm_select_experiment(ExperimentState experiment, uint32_t tick) {
  /* Each case states the channel, clock, and acquisition time explicitly. */
  switch (experiment) {
  case EXPERIMENT_A_NORMAL:
    ADC_Configure(ADC_AN0, ADC_CLOCK_NORMAL, ADC_ACQUIRE_4_TAD);
    break;
  case EXPERIMENT_A_FAST:
    ADC_Configure(ADC_AN0, ADC_CLOCK_FAST, ADC_ACQUIRE_4_TAD);
    break;
  case EXPERIMENT_B_REFERENCE:
    ADC_Configure(ADC_AN1, ADC_CLOCK_NORMAL, ADC_ACQUIRE_4_TAD);
    break;
  case EXPERIMENT_B_0_TAD:
    ADC_Configure(ADC_AN1, ADC_CLOCK_NORMAL, ADC_ACQUIRE_0_TAD);
    break;
  case EXPERIMENT_B_2_TAD:
    ADC_Configure(ADC_AN1, ADC_CLOCK_NORMAL, ADC_ACQUIRE_2_TAD);
    break;
  case EXPERIMENT_B_8_TAD:
    ADC_Configure(ADC_AN1, ADC_CLOCK_NORMAL, ADC_ACQUIRE_8_TAD);
    break;
  case EXPERIMENT_B_20_TAD:
    ADC_Configure(ADC_AN1, ADC_CLOCK_NORMAL, ADC_ACQUIRE_20_TAD);
    break;
  default:
    return;
  }

  fsm_t.current_state = experiment;
  fsm_t.adc_value = 0u;
  /* Always allow a full reference interval after changing the channel. */
  fsm_t.last_sample_ms = tick;
  Dashboard_Update(fsm_t.adc_value, experiment_name());
}

static void process_button(uint32_t tick) {
  uint8_t event = Button_Update(tick);

  if (event & BUTTON_SHORT_PRESS) {
    fsm_t.lower_nibble = !fsm_t.lower_nibble;
  }
  if (event & BUTTON_LONG_PRESS) {
    ExperimentState next = (ExperimentState)(fsm_t.current_state + 1u);
    if (next == EXPERIMENT_COUNT) {
      next = EXPERIMENT_A_NORMAL;
    }
    fsm_select_experiment(next, tick);
  }
}

static void sample_adc(uint32_t tick) {
  if ((uint32_t)(tick - fsm_t.last_sample_ms) < SAMPLE_PERIOD_MS) {
    return;
  }
  fsm_t.last_sample_ms = tick;

  if (fsm_t.current_state == EXPERIMENT_B_REFERENCE) {
    fsm_t.adc_value = ADC_Read();
  } else {
    fsm_t.adc_value = ADC_ReadBurst(experiment_samples);
  }
  Dashboard_Update(fsm_t.adc_value, experiment_name());
}

void fsm_init(void) {
  fsm_t.lower_nibble = 0u;
  Button_Init();
  Indicators_Init();
  LCD_Init();
  ADC_Init();
  fsm_select_experiment(EXPERIMENT_A_NORMAL, 0u);
}

void fsm_run(uint32_t tick) {
  process_button(tick);
  sample_adc(tick);
  Indicators_ShowBits(fsm_t.adc_value, fsm_t.lower_nibble);
}

ExperimentState fsm_state(void) { return fsm_t.current_state; }
