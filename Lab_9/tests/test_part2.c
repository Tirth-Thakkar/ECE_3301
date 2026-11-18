#include "../Common/include/Button.h"
#include "../Common/include/Indicators.h"
#include "../Common/include/LCD.h"
#include "../Part_2/include/ADC.h"
#include "../Part_2/include/Dashboard.h"
#include "../Part_2/include/StateController.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t button_event;
static ADCChannel configured_channel;
static ADCClock configured_clock;
static ADCAcquisition configured_acquisition;
static unsigned raw_reads;
static unsigned bursts;
static uint8_t lower_nibble;
static uint16_t displayed;
static const char *label;

void ADC_Init(void) {}
void ADC_Configure(ADCChannel channel, ADCClock clock,
                   ADCAcquisition acquisition) {
  configured_channel = channel;
  configured_clock = clock;
  configured_acquisition = acquisition;
}
uint16_t ADC_Read(void) {
  ++raw_reads;
  return 350u;
}
uint16_t ADC_ReadBurst(uint16_t samples[ADC_BURST_SAMPLES]) {
  (void)samples;
  ++bursts;
  return 512u;
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
  (void)value;
  lower_nibble = lower;
}
void Dashboard_Update(uint16_t value, const char *name) {
  displayed = value;
  label = name;
}

int main(void) {
  fsm_init();
  assert(fsm_state() == EXPERIMENT_A_NORMAL && !strcmp(label, "A:NORM"));
  fsm_run(99u);
  assert(bursts == 0u && raw_reads == 0u);
  fsm_run(100u);
  assert(bursts == 1u && displayed == 512u);
  button_event = BUTTON_SHORT_PRESS;
  fsm_run(101u);
  assert(lower_nibble && fsm_state() == EXPERIMENT_A_NORMAL);
  button_event = BUTTON_LONG_PRESS;
  fsm_run(102u);
  assert(fsm_state() == EXPERIMENT_A_FAST && !strcmp(label, "A:FAST"));
  assert(configured_channel == ADC_AN0 && configured_clock == ADC_CLOCK_FAST);
  assert(displayed == 0u); /* Clear the old reading when its settings change. */

  fsm_select_experiment(EXPERIMENT_B_REFERENCE, 200u);
  fsm_run(299u);
  assert(raw_reads == 0u);
  fsm_run(300u);
  assert(raw_reads == 1u && bursts == 1u && displayed == 350u);

  const ADCAcquisition expected_acquisition[] = {
      ADC_ACQUIRE_4_TAD, ADC_ACQUIRE_4_TAD, ADC_ACQUIRE_4_TAD,
      ADC_ACQUIRE_0_TAD, ADC_ACQUIRE_2_TAD, ADC_ACQUIRE_8_TAD,
      ADC_ACQUIRE_20_TAD};
  for (unsigned state = 0u; state < EXPERIMENT_COUNT; ++state) {
    fsm_select_experiment((ExperimentState)state, 400u);
    assert(fsm_state() == (ExperimentState)state);
    assert(configured_channel == (state < 2u ? ADC_AN0 : ADC_AN1));
    assert(configured_clock ==
           (state == 1u ? ADC_CLOCK_FAST : ADC_CLOCK_NORMAL));
    assert(configured_acquisition == expected_acquisition[state]);
  }
  button_event = BUTTON_LONG_PRESS;
  fsm_run(401u);
  assert(fsm_state() ==
         EXPERIMENT_A_NORMAL); /* Last experiment wraps to first. */
  fsm_run(402u);
  assert(fsm_state() == EXPERIMENT_A_NORMAL);
  fsm_select_experiment(EXPERIMENT_COUNT, 403u);
  assert(fsm_state() == EXPERIMENT_A_NORMAL);

  uint32_t start = UINT32_MAX - 50u;
  fsm_select_experiment(EXPERIMENT_B_REFERENCE, start);
  fsm_run(start + 99u);
  assert(raw_reads == 1u);
  fsm_run(start + 100u);
  assert(raw_reads == 2u);
  puts("Part 2 experiment transitions, ADC settings, and reference timing "
       "passed.");
  return 0;
}
