#include "../include/BMP280.h"
#include "../include/I2C.h"
#include <xc.h>

#define _XTAL_FREQ 16000000UL

static struct {
  uint16_t t1, p1;
  int16_t t2, t3, p2, p3, p4, p5, p6, p7, p8, p9;
} calibration;
static int initialized;

static uint16_t read_u16(const uint8_t *bytes) {
  return (uint16_t)((uint16_t)bytes[0] |
                    ((uint16_t)bytes[1] << BMP280_BYTE_BITS));
}

static int32_t read_u20(const uint8_t *bytes) {
  uint32_t msb = (uint32_t)bytes[0] << BMP280_RAW_MSB_SHIFT;
  uint32_t lsb = (uint32_t)bytes[1] << BMP280_RAW_LSB_SHIFT;
  uint32_t xlsb = bytes[2] >> BMP280_RAW_XLSB_SHIFT;
  return (int32_t)(msb | lsb | xlsb);
}

int BMP280_Init(void) {
  uint8_t id, status, bytes[BMP280_CALIBRATION_BYTES];
  initialized = 0;
  if (!I2C_ReadRegisters(BMP280_ADDRESS, BMP280_ID_REG, &id, 1u) ||
      (id != BMP280_CHIP_ID && id != BME280_CHIP_ID) ||
      !I2C_WriteRegister(BMP280_ADDRESS, BMP280_RESET_REG,
                         BMP280_RESET_COMMAND)) {
    return 0;
  }
  __delay_ms(BMP280_RESET_DELAY_MS);

  for (uint8_t attempt = 0u;; ++attempt) {
    if (!I2C_ReadRegisters(BMP280_ADDRESS, BMP280_STATUS_REG, &status, 1u)) {
      return 0;
    }
    if (!(status & BMP280_NVM_BUSY_MASK)) {
      break;
    }
    if (attempt == BMP280_NVM_POLL_LIMIT - 1u) {
      return 0;
    }
    __delay_us(BMP280_NVM_POLL_US);
  }
  if (!I2C_ReadRegisters(BMP280_ADDRESS, BMP280_CALIBRATION_REG, bytes,
                         sizeof bytes)) {
    return 0;
  }

  calibration.t1 = read_u16(bytes + BMP280_CAL_T1_OFFSET);
  calibration.t2 = (int16_t)read_u16(bytes + BMP280_CAL_T2_OFFSET);
  calibration.t3 = (int16_t)read_u16(bytes + BMP280_CAL_T3_OFFSET);
  calibration.p1 = read_u16(bytes + BMP280_CAL_P1_OFFSET);
  calibration.p2 = (int16_t)read_u16(bytes + BMP280_CAL_P2_OFFSET);
  calibration.p3 = (int16_t)read_u16(bytes + BMP280_CAL_P3_OFFSET);
  calibration.p4 = (int16_t)read_u16(bytes + BMP280_CAL_P4_OFFSET);
  calibration.p5 = (int16_t)read_u16(bytes + BMP280_CAL_P5_OFFSET);
  calibration.p6 = (int16_t)read_u16(bytes + BMP280_CAL_P6_OFFSET);
  calibration.p7 = (int16_t)read_u16(bytes + BMP280_CAL_P7_OFFSET);
  calibration.p8 = (int16_t)read_u16(bytes + BMP280_CAL_P8_OFFSET);
  calibration.p9 = (int16_t)read_u16(bytes + BMP280_CAL_P9_OFFSET);

  if (!calibration.t1 || calibration.t1 == BMP280_INVALID_COEFFICIENT ||
      !calibration.p1 || calibration.p1 == BMP280_INVALID_COEFFICIENT) {
    return 0;
  }

  /* Set config while asleep, then enable T x2 / P x16 in normal mode. */
  initialized = I2C_WriteRegister(BMP280_ADDRESS, BMP280_CONFIG_REG,
                                  BMP280_CONFIG_VALUE) &&
                I2C_WriteRegister(BMP280_ADDRESS, BMP280_CTRL_MEAS_REG,
                                  BMP280_CTRL_MEAS_VALUE);
  return initialized;
}

static float compensate_temperature(int32_t raw_temperature, float *t_fine) {
  float linear_delta = (float)raw_temperature / TEMP_LINEAR_RAW_SCALE -
                       (float)calibration.t1 / TEMP_LINEAR_CAL_SCALE;
  float quadratic_delta = (float)raw_temperature / TEMP_QUADRATIC_RAW_SCALE -
                          (float)calibration.t1 / TEMP_QUADRATIC_CAL_SCALE;

  float linear_correction = linear_delta * (float)calibration.t2;
  float quadratic_correction =
      quadratic_delta * quadratic_delta * (float)calibration.t3;

  *t_fine = linear_correction + quadratic_correction;
  return *t_fine / TEMP_FINE_SCALE;
}

static float compensate_pressure(int32_t raw_pressure, float t_fine) {
  float temperature_delta = t_fine / PRESS_TEMP_DIVISOR - PRESS_TEMP_OFFSET;

  float quadratic_offset = temperature_delta * temperature_delta *
                           (float)calibration.p6 / PRESS_QUADRATIC_OFFSET_SCALE;
  float linear_offset =
      temperature_delta * (float)calibration.p5 * PRESS_LINEAR_OFFSET_GAIN;
  float pressure_offset = quadratic_offset + linear_offset;
  pressure_offset = pressure_offset / PRESS_OFFSET_DIVISOR +
                    (float)calibration.p4 * PRESS_OFFSET_CAL_GAIN;

  float quadratic_scale = (float)calibration.p3 * temperature_delta *
                          temperature_delta / PRESS_QUADRATIC_SCALE_DIVISOR;
  float linear_scale = (float)calibration.p2 * temperature_delta;
  float pressure_scale = (quadratic_scale + linear_scale) / PRESS_SCALE_DIVISOR;
  pressure_scale =
      (1.0f + pressure_scale / PRESS_CAL_SCALE_DIVISOR) * (float)calibration.p1;

  if (pressure_scale == 0.0f) {
    return 0.0f;
  }

  float pressure = PRESS_RAW_RANGE - (float)raw_pressure;
  pressure -= pressure_offset / PRESS_RAW_OFFSET_DIVISOR;
  pressure = pressure * PRESS_RAW_GAIN / pressure_scale;

  float quadratic_correction = (float)calibration.p9 * pressure * pressure /
                               PRESS_QUADRATIC_CORRECTION_SCALE;
  float linear_correction =
      pressure * (float)calibration.p8 / PRESS_LINEAR_CORRECTION_SCALE;
  float pressure_correction =
      (quadratic_correction + linear_correction + (float)calibration.p7) /
      PRESS_CORRECTION_SCALE;

  return pressure + pressure_correction;
}

int BMP280_ReadSample(BMP280Sample *sample) {
  uint8_t bytes[BMP280_SAMPLE_BYTES];
  if (!initialized || !sample ||
      !I2C_ReadRegisters(BMP280_ADDRESS, BMP280_DATA_REG, bytes,
                         sizeof bytes)) {
    return 0;
  }
  int32_t raw_pressure = read_u20(bytes);
  int32_t raw_temperature = read_u20(bytes + BMP280_TEMPERATURE_OFFSET);
  float t_fine;
  BMP280Sample result;
  result.temperature_c = compensate_temperature(raw_temperature, &t_fine);
  result.pressure_pa = compensate_pressure(raw_pressure, t_fine);
  if (!(result.pressure_pa >= BMP280_PRESSURE_MIN_PA &&
        result.pressure_pa <= BMP280_PRESSURE_MAX_PA &&
        result.temperature_c >= BMP280_TEMPERATURE_MIN_C &&
        result.temperature_c <= BMP280_TEMPERATURE_MAX_C)) {
    return 0;
  }
  *sample = result;
  return 1;
}
