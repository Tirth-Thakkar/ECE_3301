#include "../include/LIS3DH.h"
#include "../include/I2C.h"

#define LIS3DH_ADDRESS_LOW 0x18u
#define LIS3DH_ADDRESS_HIGH 0x19u
#define LIS3DH_AUTO_INCREMENT 0x80u
#define LIS3DH_HIGH_RES_2G_BDU 0x88u
#define LIS3DH_100HZ_XYZ 0x57u

static uint8_t address;

uint8_t LIS3DH_I2C_Init(void) {
    uint8_t id;
    uint8_t control;

    for (address = LIS3DH_ADDRESS_LOW; address <= LIS3DH_ADDRESS_HIGH;
         ++address) {
        if (I2C_ReadRegisters(address, LIS3DH_WHO_AM_I, &id, 1u) &&
            id == LIS3DH_ID) {
            break;
        }
    }
    if (address > LIS3DH_ADDRESS_HIGH ||
        !I2C_WriteRegister(address, LIS3DH_CTRL_REG4, LIS3DH_HIGH_RES_2G_BDU) ||
        !I2C_WriteRegister(address, LIS3DH_CTRL_REG1, LIS3DH_100HZ_XYZ) ||
        !I2C_ReadRegisters(address, LIS3DH_CTRL_REG4, &control, 1u) ||
        control != LIS3DH_HIGH_RES_2G_BDU ||
        !I2C_ReadRegisters(address, LIS3DH_CTRL_REG1, &control, 1u) ||
        control != LIS3DH_100HZ_XYZ) {
        return 0u;
    }
    return 1u;
}

static int16_t LIS3DH_Decode(uint8_t low, uint8_t high) {
    uint16_t raw = ((uint16_t)high << 8) | low;
    int16_t value = (int16_t)(raw >> 4);
    if (value & 0x0800) {
        value -= 4096;
    }
    return value;
}

uint8_t LIS3DH_I2C_ReadAccel(int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t data[6];
    if (!I2C_ReadRegisters(address, LIS3DH_OUT_X_L | LIS3DH_AUTO_INCREMENT,
                           data, sizeof data)) {
        return 0u;
    }
    *ax = LIS3DH_Decode(data[0], data[1]);
    *ay = LIS3DH_Decode(data[2], data[3]);
    *az = LIS3DH_Decode(data[4], data[5]);
    return 1u;
}
