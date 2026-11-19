#ifndef ICM20948_H
#define ICM20948_H

#include <stdint.h>

#define ICM20948_ADDRESS_DEFAULT 0x69u
#define ICM20948_ADDRESS_ALTERNATE 0x68u
#define ICM20948_REG_BANK_SEL 0x7fu
#define ICM20948_BANK_0 0x00u
#define ICM20948_BANK_2 0x20u

// User bank 0.
#define ICM20948_WHO_AM_I 0x00u
#define ICM20948_ID 0xeau
#define ICM20948_USER_CTRL 0x03u
#define ICM20948_PWR_MGMT_1 0x06u
#define ICM20948_PWR_MGMT_2 0x07u
#define ICM20948_ACCEL_XOUT_H 0x2du

// User bank 2.
#define ICM20948_ACCEL_SMPLRT_DIV_1 0x10u
#define ICM20948_ACCEL_SMPLRT_DIV_2 0x11u
#define ICM20948_ACCEL_CONFIG 0x14u

uint8_t ICM20948_I2C_Reset(void);
uint8_t ICM20948_I2C_Configure(void);
uint8_t ICM20948_I2C_ReadAccel(int16_t *ax, int16_t *ay, int16_t *az);

#endif
