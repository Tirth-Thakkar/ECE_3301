#include "../include/I2C.h"
#include <xc.h>

#define I2C_TIMEOUT 10000u
#define I2C_PIN_MASK 0x03u
#define I2C_BUSY_MASK 0x1fu
#define I2C_READ_WRITE_MASK 0x04u
#define I2C_STANDARD_SPEED 0x80u
#define I2C_MASTER_MODE 0x28u
#define I2C_100KHZ_BAUD 39u
#define I2C_ADDRESS_MAX 0x7fu
#define I2C_READ_BIT 0x01
#define I2C_ADDRESS_SHIFT 1
#define I2C_START 0
#define I2C_REPEATED_START 1

static int wait_idle(void) {
  uint16_t remaining = I2C_TIMEOUT;
  while ((SSP2CON2 & I2C_BUSY_MASK) || (SSP2STAT & I2C_READ_WRITE_MASK)) {
    if (--remaining == 0u) {
      return 0;
    }
  }
  return 1;
}

static int wait_done(void) {
  uint16_t remaining = I2C_TIMEOUT;
  while (!PIR3bits.SSP2IF) {
    if (PIR3bits.BCL2IF || --remaining == 0u) {
      return 0;
    }
  }
  PIR3bits.SSP2IF = 0u;
  return !PIR3bits.BCL2IF;
}

static int start(int repeated) {
  if (!wait_idle()) {
    return 0;
  }
  PIR3bits.SSP2IF = 0u;
  if (repeated) {
    SSP2CON2bits.RSEN = 1u;
  } else {
    SSP2CON2bits.SEN = 1u;
  }
  return wait_done();
}

static int stop(void) {
  if (!wait_idle()) {
    return 0;
  }
  PIR3bits.SSP2IF = 0u;
  SSP2CON2bits.PEN = 1u;
  return wait_done();
}

static int write_byte(uint8_t value) {
  if (!wait_idle()) {
    return 0;
  }
  SSP2CON1bits.WCOL = 0u;
  PIR3bits.SSP2IF = 0u;
  SSP2BUF = value;
  return !SSP2CON1bits.WCOL && wait_done() && !SSP2CON2bits.ACKSTAT;
}

static int read_byte(uint8_t *value, int last) {
  if (!wait_idle()) {
    return 0;
  }
  PIR3bits.SSP2IF = 0u;
  SSP2CON2bits.RCEN = 1u;
  if (!wait_done()) {
    return 0;
  }
  *value = SSP2BUF;
  if (!wait_idle()) {
    return 0;
  }
  PIR3bits.SSP2IF = 0u;
  SSP2CON2bits.ACKDT = (last != 0);
  SSP2CON2bits.ACKEN = 1u;
  return wait_done();
}

void I2C_Init(void) {
  SSP2CON1bits.SSPEN = 0u;
  ANSELD &= (uint8_t)~I2C_PIN_MASK;
  TRISD |= I2C_PIN_MASK;
  SSP2STAT = I2C_STANDARD_SPEED;
  SSP2ADD = I2C_100KHZ_BAUD;
  SSP2CON2 = 0u;
  SSP2CON1 = I2C_MASTER_MODE;
  PIR3bits.SSP2IF = 0u;
  PIR3bits.BCL2IF = 0u;
}

int I2C_ReadRegisters(uint8_t address, uint8_t reg, uint8_t *data,
                      uint8_t count) {
  if (address > I2C_ADDRESS_MAX || !data || !count) {
    return 0;
  }

  int ok = start(I2C_START) &&
           write_byte((uint8_t)(address << I2C_ADDRESS_SHIFT)) &&
           write_byte(reg) && start(I2C_REPEATED_START) &&
           write_byte((uint8_t)((address << I2C_ADDRESS_SHIFT) | I2C_READ_BIT));
  for (uint8_t i = 0u; ok && i < count; ++i) {
    ok = read_byte(&data[i], i == count - 1u);
  }
  int stopped = stop();
  return ok && stopped;
}

int I2C_WriteRegister(uint8_t address, uint8_t reg, uint8_t value) {
  if (address > I2C_ADDRESS_MAX) {
    return 0;
  }
  int ok = start(I2C_START) &&
           write_byte((uint8_t)(address << I2C_ADDRESS_SHIFT)) &&
           write_byte(reg) && write_byte(value);
  int stopped = stop();
  return ok && stopped;
}
