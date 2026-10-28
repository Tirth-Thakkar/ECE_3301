#include <xc.h>
#include "../include/SPI.h"
#include "../include/LIS3DH.h"

uint8_t LIS3DH_ReadRegister(uint8_t reg)
{
    uint8_t value;

    LATDbits.LATD0 = 0;
    SPI_Transfer(reg | 0x80);
    value = SPI_Transfer(0x00);
    LATDbits.LATD0 = 1;

    return value;
}

void LIS3DH_WriteRegister(uint8_t reg, uint8_t data)
{
    LATDbits.LATD0 = 0;
    SPI_Transfer(reg & 0x3F);
    SPI_Transfer(data);
    LATDbits.LATD0 = 1;
}

uint8_t LIS3DH_Init(void)
{
    if (LIS3DH_ReadRegister(LIS3DH_WHO_AM_I) != LIS3DH_ID)
    {
        return 0;
    }

    LIS3DH_WriteRegister(LIS3DH_CTRL_REG4, 0x88);
    LIS3DH_WriteRegister(LIS3DH_CTRL_REG1, 0x57);

    if (LIS3DH_ReadRegister(LIS3DH_CTRL_REG4) != 0x88)
    {
        return 0;
    }

    if (LIS3DH_ReadRegister(LIS3DH_CTRL_REG1) != 0x57)
    {
        return 0;
    }

    return 1;
}

static int16_t LIS3DH_Decode(uint8_t low, uint8_t high)
{
    uint16_t raw;
    int16_t value;

    raw = ((uint16_t)high << 8) | low;
    value = (int16_t)(raw >> 4);

    if (value & 0x0800)
    {
        value -= 4096;
    }

    return value;
}

void LIS3DH_ReadAccel(int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t data[6];
    uint8_t i;

    LATDbits.LATD0 = 0;
    SPI_Transfer(0xC0 | LIS3DH_OUT_X_L);

    for (i = 0; i < 6; i++)
    {
        data[i] = SPI_Transfer(0x00);
    }

    LATDbits.LATD0 = 1;

    *ax = LIS3DH_Decode(data[0], data[1]);
    *ay = LIS3DH_Decode(data[2], data[3]);
    *az = LIS3DH_Decode(data[4], data[5]);
}
