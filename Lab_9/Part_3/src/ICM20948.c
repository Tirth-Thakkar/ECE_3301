#include "../include/ICM20948.h"
#include "../include/I2C.h"

#define DEVICE_RESET 0x80u
#define CLOCK_AUTO 0x01u
#define ACCEL_ONLY 0x07u
#define ACCEL_2G_50HZ_DLPF 0x19u
#define ACCEL_RATE_DIVISOR 10u // 1125 / (1 + 10) = about 102 Hz.
#define ACCEL_COUNTS_PER_G 16384L

static uint8_t address;
static uint8_t detected;
static uint8_t configured;

static uint8_t select_bank(uint8_t bank) {
    return I2C_WriteRegister(address, ICM20948_REG_BANK_SEL, bank) != 0;
}

static uint8_t identify(uint8_t candidate) {
    uint8_t id;

    address = candidate;
    return select_bank(ICM20948_BANK_0) &&
           I2C_ReadRegisters(address, ICM20948_WHO_AM_I, &id, 1u) &&
           id == ICM20948_ID;
}

uint8_t ICM20948_I2C_Reset(void) {
    detected = 0u;
    configured = 0u;
    if (!identify(ICM20948_ADDRESS_DEFAULT) &&
        !identify(ICM20948_ADDRESS_ALTERNATE)) {
        return 0u;
    }
    if (!I2C_WriteRegister(address, ICM20948_PWR_MGMT_1, DEVICE_RESET)) {
        return 0u;
    }
    detected = 1u;
    return 1u;
}

static uint8_t write_checked(uint8_t reg, uint8_t value) {
    uint8_t readback;

    return I2C_WriteRegister(address, reg, value) &&
           I2C_ReadRegisters(address, reg, &readback, 1u) && readback == value;
}

uint8_t ICM20948_I2C_Configure(void) {
    uint8_t power;

    configured = 0u;
    if (!detected || !select_bank(ICM20948_BANK_0) ||
        !I2C_ReadRegisters(address, ICM20948_PWR_MGMT_1, &power, 1u) ||
        (power & DEVICE_RESET)) {
        return 0u;
    }

    // Wake into continuous sampling, keep I2C interface enabled
    if (!write_checked(ICM20948_PWR_MGMT_1, CLOCK_AUTO) ||
        !write_checked(ICM20948_PWR_MGMT_2, ACCEL_ONLY) ||
        !write_checked(ICM20948_USER_CTRL, 0u)) {
        return 0u;
    }

    if (!select_bank(ICM20948_BANK_2) ||
        !write_checked(ICM20948_ACCEL_SMPLRT_DIV_1, 0u) ||
        !write_checked(ICM20948_ACCEL_SMPLRT_DIV_2, ACCEL_RATE_DIVISOR) ||
        !write_checked(ICM20948_ACCEL_CONFIG, ACCEL_2G_50HZ_DLPF) ||
        !select_bank(ICM20948_BANK_0)) {
        return 0u;
    }
    configured = 1u;
    return 1u;
}

static int16_t decode_milligravity(uint8_t high, uint8_t low) {
    uint16_t raw = (uint16_t)(((uint16_t)high << 8) | low);
    int32_t counts = raw;
    int32_t milligravity;

    if (raw & 0x8000u) {
        counts -= 65536L;
    }
    milligravity = counts * 1000L;
    if (milligravity >= 0L) {
        milligravity += ACCEL_COUNTS_PER_G / 2L;
    } else {
        milligravity -= ACCEL_COUNTS_PER_G / 2L;
    }
    return (int16_t)(milligravity / ACCEL_COUNTS_PER_G);
}

uint8_t ICM20948_I2C_ReadAccel(int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t bytes[6];

    // ICM bursts auto-increment without LIS3DH's sub-address flag
    if (!configured || !ax || !ay || !az || !select_bank(ICM20948_BANK_0) ||
        !I2C_ReadRegisters(address, ICM20948_ACCEL_XOUT_H, bytes,
                           sizeof bytes)) {
        return 0u;
    }
    *ax = decode_milligravity(bytes[0], bytes[1]);
    *ay = decode_milligravity(bytes[2], bytes[3]);
    *az = decode_milligravity(bytes[4], bytes[5]);
    return 1u;
}
