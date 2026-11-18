# Lab 9: Pre-lab and Parts 1–3

Reference: [Lab 9 handout](../Resources/Labs/Lab_9.pdf). Calculations below use a 16 MHz internal oscillator, VDD = 3.300 V, and 50 °C for acquisition timing. **Pending** means a physical measurement or demonstration is still needed; predicted values are not recorded as observations.

## Pre-lab

| # | Question | Answer |
|---|----------|--------|
| 1 | ADC step size, VREF / 1024 | 3.300 / 1024 = **0.00322265625 V = 3.22265625 mV/LSB**. |
| 2 | One count in simulated battery volts | 12.6 / 1023 = **0.0123167155 V/count**, about **12.317 mV/count**. |
| 3 | First count classified OK for V > 11.10 V | adc > 11.10 × 1023 / 12.6 = 901.2143, so **902**. |
| 4 | First count that is not CRITICAL for V ≥ 10.50 V | adc ≥ 10.50 × 1023 / 12.6 = 852.5, so **853**. |
| 5 | TAD at FOSC/16, /32, /64 | **1 µs, 2 µs, 4 µs**, respectively. |
| 6 | Chosen ADC clock and reason | **FOSC/32**, ADCS = `010`, TAD = **2 µs**. This exceeds the required 1 µs minimum and leaves ample time for 1 kHz sampling. |
| 7 | Minimum TACQ, 10 kΩ source at 50 °C | **7.454 µs ≈ 7.45 µs**, calculated below. |
| 8 | Chosen ACQT and time | **4 TAD = 8 µs**, ACQT = `010`; 8 µs exceeds 7.454 µs. |
| 9 | Acquisition + conversion | 4 TAD + 11 TAD = 15 × 2 µs = **30 µs**. The separate post-conversion recovery is 2 TAD = 4 µs; software adds overhead. |
| 10 | Largest sum of 16 samples and C type | 16 × 1023 = **16,368**, held by **`uint16_t`**. The rounded-average numerator is at most 16,376. |
| 11 | Number of full-scale samples a uint16_t sum can hold | floor(65,535 / 1023) = **64 samples**. 64 × 1023 = 65,472; 65 × 1023 = 66,495 would overflow. |
| 12 | Largest adc × 1260 and C type | 1023 × 1260 = **1,288,980**, requiring **`uint32_t`**. Cast before multiplication: `(uint32_t)adc * 1260UL`. |

Acquisition model:

```text
TACQ  = TAMP + TC + TCOFF
TC    = CHOLD × (RIC + RSS + RS) × ln(2047)
      = 13.5 pF × (1 kΩ + 700 Ω + 10 kΩ) × ln(2047)
      = 1.204231 µs
TCOFF = (50 − 25) × 0.05 µs = 1.25 µs
TACQ  = 5 + 1.204231 + 1.25 = 7.454231 µs
```

This agrees with the 7.45 µs worked example in the [provided PIC datasheet](../Resources/General/PIC_Datasheet.pdf), Equation 17-1, printed page 299. In this revision (DS40001412H), acquisition requirements are in **Section 17.4**; the handout refers to Section 17.3. The example's Note 3 specifies the 10 kΩ source limit used by the handout; the preceding paragraph in this revision inconsistently says 3 kΩ. The 100 kΩ and 1 MΩ tests deliberately exceed either limit.

Normal ADC setup: `ADCON0 = 0x01` (AN0, enabled), `ADCON1 = 0x00` (VDD/VSS references), `ADCON2 = 0x92` (right aligned, 4 TAD acquisition, FOSC/32). RA0 is an analog input. S1/RB4 and the LED outputs are explicitly digital. `FOSC = INTIO67`, the PLL is disabled at runtime, LVP is enabled, and `_XTAL_FREQ=16000000UL` is defined for every build target.

## Project structure and board setup

Each section is a separate project with the same `include/`, `src/`, CMake, and Make layout as the prior labs. Choose the folder to choose the section. Each project has a short `main.c` that initializes the system and repeatedly calls `fsm_run()`, following the earlier `StateController` approach.

```text
Lab_9/
  Prelab.md
  Common/                 Shared LCD, button, LED, and configuration files
  Part_1/                 Polled AN0 measurement and bit display
    CMakeLists.txt
    Makefile
    include/
    src/
  Part_2/                 Back-to-back ADC experiments
    CMakeLists.txt
    Makefile
    include/
    src/
  Part_3/                 Interrupt sampling, averaging, and I²C sensors
    CMakeLists.txt
    Makefile
    include/
    src/
  tests/                  Host tests for the controllers and components
```

| Component | Responsibility |
|-----------|----------------|
| `main.c` | Initialization, main loop, and ISR dispatch only. |
| `StateController.c` | Holds the application's named states, handles S1 events, and calls the appropriate tasks through `fsm_init()` and `fsm_run()`. |
| `System.c` | Configures the 16 MHz clock and Timer2; owns the tick and, in Part 3, the task flags and their protected snapshot. |
| `ADC.c` | Implements only the ADC behavior needed by that section: one polled reading, an experiment burst, or interrupt-driven blocks. |
| `Dashboard.c` | Formats that section's LCD views. Part 3 extends the Lab 8 dashboard with a regular three-page rotation and the Engineer view. |
| `Part_3/src/Altimeter.c` | Retains Lab 8 calibration, pressure filtering, and vertical-motion logic, with separate methods for acquisition, calibration, and altitude updates. |
| `Part_3/src/MotionSensor.c` | Samples the LIS3DH and retries failed initialization/read operations. |
| `Part_3/src/LIS3DH.c` | Adapts the Lab 7 register setup and sample decoding to the shared I²C bus. |
| `Common/` | Reuses the unchanged Lab 8 LCD driver and the shared button and indicator helpers. |

Part 3 directly links the existing Lab 8 BMP280 and I²C drivers. Its altimeter and dashboard methods repurpose the Lab 8 application code. Each project has its own explicit source list, so Parts 1 and 2 contain no sensor code.

| Signal | Connection / configuration |
|--------|----------------------------|
| Supply | J45 = 3V3; measure TVDD before calibration. Keep the 28-pin socket and both mikroBUS sockets empty. |
| POT1 | On-board wiper at RA0/AN0; do not attach another pot to RA0. |
| Part 2 pot | External 10 kΩ pot between 3V3 and GND; wiper → series resistor → RA1/AN1. |
| Bit LEDs | D5..D2 = RA7..RA4. High view maps bits 9..6 in that order; low view maps bits 3..0. |
| S1 | RB4, active-low, pull-up enabled, analog input disabled. |
| LCD | RS=RD2, E=RD3, D4..D7=RD4..RD7, R/W=GND; retain the Lab 8 supply wiring. |
| BMP280 | MSSP2: RD0=SCL, RD1=SDA, address `0x76`, as in Lab 8. |
| LIS3DH | Share RD0=SCL and RD1=SDA with BMP280; common GND and 3.3 V power/logic. CS must be tied to 3.3 V. Strap SA0 to GND (`0x18`) or 3.3 V (`0x19`); the driver tries both addresses. Ensure SDA/SCL have pull-ups to 3.3 V. |

The requested IMU is the **Lab 7 LIS3DH, adapted to I²C**. Its Lab 7 register constants, 100 Hz / ±2 g high-resolution setup, and signed 12-bit decoding are reused. The old SPI wiring must be removed: RD0 was the Lab 7 chip select and now carries SCL. RC3–RC5 are available, and the SPI driver is not needed for this arrangement. The I²C mode, addresses, and auto-increment bit follow the [ST LIS3DH datasheet, Sections 6.1 and 8](https://www.st.com/resource/en/datasheet/lis3dh.pdf). The LIS3DH is an accelerometer; its displayed X/Y/Z readings use mg, as in Lab 7.

Build from the repository root:

```sh
make -C Lab_9/Part_1 build
make -C Lab_9/Part_2 build
make -C Lab_9/Part_3 build
```

The outputs are `Part_1/build/Lab9_Part1_app.hex`, `Part_2/build/Lab9_Part2_app.hex`, and `Part_3/build/Lab9_Part3_app.hex`, relative to `Lab_9/`. Program a project with its own `prog` target, for example `make -C Lab_9/Part_2 prog`. Running `make` inside a part builds and programs that part, matching the previous labs. The root convenience command `make -C Lab_9 build` builds all three projects.

For MPLAB X, create one PIC18F46K22 project for each part using its CMake source list and define `_XTAL_FREQ=16000000UL`. Include that part's headers and the referenced `Common` files. Part 3 additionally needs the Lab 8 BMP280/I²C sources and headers. Experiment selection in Part 2 is handled at runtime by its state controller.

## Part 1: See the bits

`ADC_Init()` selects AN0 and the pre-lab settings. `ADC_Read()` starts GO/DONE, polls until hardware clears it, and combines ADRESH:ADRESL. The polled ADC runs once every 100 ms, scheduled by the reused Timer2 1 ms tick; only the timer uses an interrupt in Part 1. S1 uses a 20 ms debounce with no delay. A short press switches the LEDs between bits 9:6 and 3:0; holding also switches the nibble in this test application. The LCD always shows the decimal count and all 10 binary bits with `LCD_PrintLine()`.

Example:

```text
ADC:0902
1110000110
```

### Calibration

Measure TVDD first: **VDD = Pending V**. At each position, use that measured VDD in both calculated columns. Determine the better match by comparing each calculation with the measured RA0 voltage; it cannot be selected from nominal pot positions alone.

| Position | Measured RA0 (V) | ADC count | Count × measured VDD / 1024 (V) | Count × measured VDD / 1023 (V) | Better match |
|----------|------------------|-----------|--------------------------------|--------------------------------|--------------|
| Near minimum | Pending | Pending | Pending | Pending | Pending |
| 25% | Pending | Pending | Pending | Pending | Pending |
| 50% | Pending | Pending | Pending | Pending | Pending |
| 75% | Pending | Pending | Pending | Pending | Pending |
| Near maximum | Pending | Pending | Pending | Pending | Pending |

Nominal predictions at exactly 0, 0.825, 1.650, 2.475, and 3.300 V are approximately 0, 256, 512, 768, and 1023 counts. ADC quantization uses VREF/1024; dividing by 1023 instead forces the maximum reported code to the reference endpoint. Real DMM readings, quantization, and ADC errors determine which reconstruction looks closer at a particular position.

Predicted bit behavior: the upper nibble is usually stable with POT1 stationary; low bits can flicker because of analog noise and quantization. A binary carry boundary can change upper bits too. Observed endpoint counts: **Pending / Pending**. Observed stable/flickering bits: **Pending**.

### Change one bit: ADFM

At exactly mid-scale, the correct right-aligned count is 512 (`0x0200`). With ADFM=0, the same conversion is left aligned: `512 << 6 = 32768` (`0x8000`). The unchanged combination of ADRESH and ADRESL therefore reports **32768**, or **64 times the true count**. `ADC_Read()` deliberately does not mask the result, so this error remains visible. At a different mid-scale code, predict `actual_count × 64`. The 10-character binary view shows only the low 10 bits of this invalid combined value.

In a debug build, stop after initialization, set `ADCON2bits.ADFM=0`, resume, and record the decimal result: **Pending**. Restore `ADFM=1` before checkoff or subsequent experiments.

Checkoff 1: endpoints, decimal/binary display, S1 nibble selection, calibration measurements, and alignment experiment: **Pending hardware demonstration**.

## Part 2: Break the ADC on purpose

All interrupts remain disabled throughout Part 2. Each burst stores 16 consecutive results in `experiment_samples[16]`, discards index 0, and displays the rounded mean `(sum_of_indices_1_to_15 + 7) / 15`. Summing, button handling, and LCD writes happen after the burst. A 4 µs recovery delay meets the normal clock's separate 2 TAD requirement; the short register/array loop adds some unavoidable acquisition time. There is a pause between displayed bursts, but no 100 ms pause between conversions **within** a burst. Discarding the first sample removes the one helped by the preceding idle interval.

Build and program the Part 2 project once:

```sh
make -C Lab_9/Part_2 build
make -C Lab_9/Part_2 prog
```

The controller starts in `EXPERIMENT_A_NORMAL`. **Hold S1 for 1 s to advance to the next experiment** in the table; after `B:20`, it returns to `A:NORM`. Release S1 between holds. A short press switches the LED nibble. The LCD identifies the active experiment, for example `ADC:0512 B:8`, with all 10 binary bits on the second line. The hardware wiring changes in Experiments B/C are still performed manually.

`fsm_select_experiment()` contains an explicit switch listing the channel, ADC clock, and acquisition setting for each state. Selecting a state clears the previous displayed result and allows at least 100 ms before the first new measurement. The debugger can also call this method or inspect `fsm_t.current_state` to identify the active experiment.

| LCD label / controller state | Input | ADCS / TAD | ACQT | Reading |
|------------|-------|------------|------|---------|
| `A:NORM` / `EXPERIMENT_A_NORMAL` | AN0/POT1 | FOSC/32 / 2 µs | 4 TAD = 8 µs | 16-sample burst, discard first |
| `A:FAST` / `EXPERIMENT_A_FAST` | AN0/POT1 | FOSC/2 / 125 ns | 4 TAD = 0.5 µs | Same burst; intentionally invalid ADC clock |
| `B:REF` / `EXPERIMENT_B_REFERENCE` | AN1 | FOSC/32 / 2 µs | 4 TAD = 8 µs | One reading with at least 100 ms between conversions |
| `B:0` / `EXPERIMENT_B_0_TAD` | AN1 | FOSC/32 / 2 µs | 0 | Burst |
| `B:2` / `EXPERIMENT_B_2_TAD` | AN1 | FOSC/32 / 2 µs | 2 TAD = 4 µs | Burst |
| `B:8` / `EXPERIMENT_B_8_TAD` | AN1 | FOSC/32 / 2 µs | 8 TAD = 16 µs | Burst |
| `B:20` / `EXPERIMENT_B_20_TAD` | AN1 | FOSC/32 / 2 µs | 20 TAD = 40 µs | Burst |

The polled Timer2 flag supplies the Part 2 UI clock. LCD writes can stretch that clock slightly; the reference interval remains at least 100 ms, and no interrupt can disturb a burst. `A:FAST` changes only ADCS, as required; this also shortens automatic acquisition because ACQT is expressed in TAD.

### Experiment A: Clock too fast

At each fixed position record `A:NORM`, then `A:FAST`, without moving POT1. Difference = fast count − normal count. Hold S1 through the remaining experiment states until `A:NORM` returns to restore the clock afterward.

| POT1 position | Count at chosen TAD = 2 µs | Count at 125 ns | Difference |
|---------------|----------------------------|-----------------|------------|
| Low | Pending | Pending | Pending |
| Middle | Pending | Pending | Pending |
| High | Pending | Pending | Pending |

Prediction: 125 ns is eight times faster than the permitted 1 µs minimum, so accuracy is unspecified. The error's size and sign must be measured; an apparently plausible count does not validate this clock.

### Experiment B: Source too weak

Wire the external 10 kΩ pot between **3V3 and GND**, with its wiper feeding RA1 through R. Set the settled RA1 voltage close to 1.65 V. For each R, record `B:REF` first, then `B:0`, `B:2`, `B:8`, and `B:20`. The ideal reference count at exactly VDD/2 is approximately 512.

At the pot midpoint its Thevenin resistance is `5 kΩ || 5 kΩ = 2.5 kΩ`. Predictions use the same 50 °C model as the pre-lab with `RS = R + 2.5 kΩ`:

```text
TACQ(µs) = 6.25 + 13.5e-6 × (1000 + 700 + R + 2500) × ln(2047)
```

| Series R | Predicted TACQ (µs) | Reference count | ACQT 0 | ACQT 2 TAD (4 µs) | ACQT 8 TAD (16 µs) | ACQT 20 TAD (40 µs) |
|----------|----------------------|-----------------|--------|--------------------|---------------------|----------------------|
| 1 kΩ | **6.79** | Pending | Pending | Pending | Pending | Pending |
| 100 kΩ | **16.97** | Pending | Pending | Pending | Pending | Pending |
| 1 MΩ | **109.61** | Pending | Pending | Pending | Pending | Pending |

Prediction: 16 and 40 µs exceed the 1 kΩ row's calculated acquisition requirement; only 40 µs exceeds the 100 kΩ row; none reaches the 1 MΩ row's requirement. The extra idle time in the software loop may improve shorter-ACQT measurements. High source resistance also produces leakage error that a longer acquisition cannot remove. The 100 ms reference largely removes settling error, but it can still contain leakage error.

### Experiment C: Add the capacitor

Keep R=1 MΩ and add **100 nF from RA1 to GND**. Let it settle and repeat `B:0` and `B:2` without changing the pot position, then turn the pot quickly.

| Condition, R = 1 MΩ | Reference count | ACQT 0 | ACQT 2 TAD | Response to a quick pot change |
|----------------------|-----------------|--------|------------|--------------------------------|
| Without capacitor (copy Experiment B) | Pending | Pending | Pending | Pending |
| With 100 nF | Pending | Pending | Pending | Pending |

1. **Why did a 100 ms gap hide the error?** While idle, the ADC input can recharge the hold capacitor through the source resistance. The long gap provides far more settling time than any acquisition interval in the table. The datasheet notes that the hold capacitor is discharged after each conversion, so subsequent conversions in a tight burst reveal weak-source acquisition problems.
2. **Which case had the largest error, and did the prediction explain it?** Prediction: the 1 MΩ source with the shortest acquisition and no capacitor should be the worst source-impedance case. The fast-clock experiment is outside specifications and might be worse overall. **Measured largest error and comparison with prediction: Pending.**
3. **What did the capacitor fix?** It supplies a local charge reservoir for the tiny hold capacitor and filters noise, reducing the repeated sampling voltage disturbance. It does not eliminate DC input leakage error.
4. **What did it cost?** Response time. Near the midpoint, τ ≈ (1 MΩ + 2.5 kΩ) × 100 nF = **100.25 ms**. About 5τ = **0.50 s** gives 99.3% settling; roughly 7.62τ = **0.76 s** is needed for a full-scale step to settle to about half an ADC count.
5. **What does this imply for a high-value battery divider?** Use its Thevenin resistance, not just the resistor sum, to calculate acquisition. High values reduce battery current but increase acquisition and leakage error. A local capacitor helps sampling and noise at the cost of response time; a lower impedance or suitable buffer is another option.

One observed result that differed from prediction, with explanation: **Pending**. Return to AN0 and the normal FOSC/32, 4 TAD settings by building/programming the `Part_3` project. Checkoff 2 tables and explanation: **Pending hardware demonstration**.

## Part 3: Interrupt-driven sampling and averaging

Part 3 extends the Lab 8 main loop and uses **only its existing Timer2**. `System.c` owns the tick and task flags. The 1 ms interrupt increments `ticks`, starts AN0 when GO/DONE and ADIF are clear, and sets task flags. The ADC interrupt clears ADIF, collects the result, and updates the running sum/min/max. Every 16 samples it publishes `(sum + 8) / 16`, `max − min`, and a data-ready flag. With averaging off, it publishes the newest raw sample while retaining the same min/max block statistics. `adc_completed_blocks` increments in both modes.

ADC interrupt enable order is **clear ADIF → set ADIE → set PEIE → set GIE**. If both ADC and timer flags are pending, the ISR collects the old ADC result before starting another conversion. All sensor reads, display formatting, LCD writes, and application state changes stay in the main loop.

| Task | Period / behavior |
|------|-------------------|
| AN0 acquisition/conversion start | 1 ms, only if the ADC is idle and no unread result is pending |
| ADC block publication | Every 16 completed conversions, nominally 16 ms / 62.5 blocks per second |
| LIS3DH over shared I²C | 10 ms (100 Hz), configuration/readback from Lab 7 |
| BMP280 sampling | 50 ms, retaining Lab 8 calibration/filtering |
| Vertical-motion update | 100 ms, retaining Lab 8 logic |
| LCD task | 250 ms, using full padded lines |
| Failed sensor retry | 1000 ms per sensor; sensor failure does not stop ADC interrupts |

Short S1 presses select Flight/Engineer view; holding for 1 s toggles averaging once, and releasing a held button does not also switch views. Flight view shows the existing altitude/environment pages and an added LIS3DH X/Y/Z page. D5..D2 show bits 9:6 in Flight view and bits 3:0 in Engineer view. Calibration and BMP280 faults retain their Lab 8 screens in Flight view; Engineer view remains available.

```text
ADC:0902 AVG:ON
1110000110 PP:03
```

For PP greater than 99, the display moves the complete PP value to the top line and the averaging indicator to the bottom, preserving all 10 binary bits within the 16-column limit.

### Shared-data protection

Every interrupt-shared object is `volatile`. `ADC_TakeBlock()` saves GIE, disables interrupts briefly, copies both 16-bit published values, clears the data-ready flag, and restores the saved GIE state. `System_ReadEvents()` copies the timestamp and task flags with the same pattern. No LCD or bus transaction runs in either critical section. A one-byte averaging control is shared with the ISR. The publication slot retains the most recent block if the main loop is late; the completed-block counter still counts every block. Inspect the 32-bit counter only while the debugger has halted the PIC, or copy it under the same protection if adding a runtime reader.

### Noise measurements

Hold POT1 still near mid-scale and keep its position fixed for both modes. Enter Engineer view, record 10 displayed values and their PP readings with averaging off, then hold S1 to turn averaging on and repeat. PP is calculated from the same 16 raw samples in either mode; averaging should mainly reduce the displayed count spread, not PP itself.

| Reading | ADC, averaging off | PP, averaging off | ADC, averaging on | PP, averaging on |
|---------|--------------------|-------------------|-------------------|------------------|
| 1 | Pending | Pending | Pending | Pending |
| 2 | Pending | Pending | Pending | Pending |
| 3 | Pending | Pending | Pending | Pending |
| 4 | Pending | Pending | Pending | Pending |
| 5 | Pending | Pending | Pending | Pending |
| 6 | Pending | Pending | Pending | Pending |
| 7 | Pending | Pending | Pending | Pending |
| 8 | Pending | Pending | Pending | Pending |
| 9 | Pending | Pending | Pending | Pending |
| 10 | Pending | Pending | Pending | Pending |

| Mode | Lowest displayed ADC | Highest displayed ADC | Spread (highest − lowest), counts | Typical PP (median of 10) |
|------|----------------------|-----------------------|------------------------------------|---------------------------|
| Averaging off | Pending | Pending | Pending | Pending |
| Averaging on | Pending | Pending | Pending | Pending |

### Sampling-rate check

Reset the PIC, allow **10,000 timer ticks** after the scheduler starts, then halt and inspect `ticks` and `adc_completed_blocks`. Samples occur at ticks 1, 2, …, so the last conversion must have finished before comparing the exact count. The expected rate is 1000 samples/s ÷ 16 = **62.5 blocks/s**, or **625 blocks in 10 s**. Stopping at exactly tick 10,000 before its ADC conversion finishes can show 624 blocks; wait at least the 30 µs conversion time or account for that pending result. If halting later, compare with `floor(completed_samples / 16)` for the elapsed ticks rather than assuming exactly 10 s.

| Elapsed ticks | Expected completed blocks | Observed blocks | Result |
|---------------|---------------------------|-----------------|--------|
| 10,000, after final conversion | **625** | Pending | Pending |

Repeat while the LCD is updating and both I²C sensors are active. LCD writes block only the main loop, so they should not change the completed-block rate. Checkoff 3 averaging demonstration, counter measurement, and shared-data explanation: **Pending hardware demonstration**.

## Software verification

| Check | Result |
|-------|--------|
| XC8 v4.00 build, Part 1 | Passed; HEX generated |
| XC8 v4.00 build, independent Part 2 project | Passed; one HEX includes all seven runtime experiment states |
| XC8 v4.00 build, Part 3 with BMP280 and I²C LIS3DH | Passed; HEX generated |
| Host tests: rounded/full-scale ADC blocks, PP with averaging off, latest-block handoff, interrupt-enable restoration, unread-result protection | Passed |
| Host tests: button bounce, short press, one hold event, hold release, timestamp wraparound | Passed |
| Host tests: binary/nibble output, full PP display, three-page rotation, LIS3DH signed decoding, both I²C addresses, bus failure | Passed |
| Host tests: Part 1 controller sampling cadence and nibble transitions; Part 2 experiment settings, state cycling, reference timing, and timestamp wraparound | Passed |
| Host simulation: 10,000 scheduler ticks, task periods, and 625 completed ADC blocks | Passed; physical timing remains Pending |
| Host tests: BMP280 calibration/retry states and LIS3DH acquisition/retry | Passed |
| LCD source/header comparison against Lab 8 | Identical |

Run the host tests with `make -C Lab_9 test`. The component tests compile the actual Part 3 ADC, scheduler, dashboard, sensor, button, and indicator code against mocked registers and bus/LCD endpoints. Separate controller tests exercise Parts 1 and 2 with mocked hardware methods. All host tests use warnings as errors. XC8 emits unused-function warnings for the Part 2 debugger state getter and the provided LCD helpers. These software checks establish compilation and logic behavior; the calibration, analog-error measurements, sample-rate demonstration, and physical circuit checkoffs above still require hardware.
