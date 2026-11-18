/*
 * lcd.c - HD44780 16x2 character LCD driver (4-bit, write-only)
 *
 * ECE 3301 - Microcontrollers | Lab 8
 * Target: PIC18F46K22, MPLAB XC8
 *
 * See lcd.h for wiring and usage rules.
 *
 * Timing notes (HD44780U datasheet, VCC = 5 V):
 *   - Instruction execution: 37 us and 1.52 ms (clear/home) at 270 kHz.
 *     The LCD oscillator may be as slow as 190 kHz, which stretches these
 *     to about 53 us and 2.2 ms. R/W is grounded, so the busy flag cannot
 *     be read; this driver waits 60 us and 3 ms instead.
 *   - E high pulse >= 230 ns, data setup >= 80 ns before E falls.
 *     The LCD latches on the FALLING edge of E.
 *   - Power-on: wait > 15 ms after VCC reaches 4.5 V. This driver waits 50 ms.
 *   - The driver always initializes "by instruction" (0x3, 0x3, 0x3, 0x2),
 *     because the LCD's automatic power-on reset only works for supply
 *     rise times of 0.1-10 ms, which USB power does not guarantee.
 */

#include "../include/LCD.h"
#include <stdint.h>
#include <xc.h>

#define _XTAL_FREQ 16000000UL

/* ---------------------------------------------------------------------
 * Pin definitions (must match lcd.h wiring)
 * ------------------------------------------------------------------- */
#define LCD_RS LATDbits.LATD2
#define LCD_E LATDbits.LATD3

#define LCD_KEEP_MASK 0x03u /* RD0..RD1 belong to I2C - never touch */

/* ---------------------------------------------------------------------
 * Timing (see header comment for the reasoning)
 * ------------------------------------------------------------------- */
#define LCD_POWERUP_MS 50
#define LCD_SHORT_US 60       /* normal instructions and characters */
#define LCD_LONG_MS 3         /* clear display / return home        */
#define LCD_INIT_WAIT1_MS 5   /* after first 0x3 nibble  (> 4.1 ms)  */
#define LCD_INIT_WAIT2_US 150 /* after later init nibbles (> 100 us) */

/* ---------------------------------------------------------------------
 * Low level
 * ------------------------------------------------------------------- */

/* Put one nibble on D4..D7 and pulse E. RS must already be set.
 * Only RD4..RD7 change; RS (RD2), E (RD3), RD0 and RD1 are preserved. */
static void LCD_WriteNibble(uint8_t nibble) {
  LATD = (uint8_t)((LATD & 0x0Fu) | (uint8_t)((nibble & 0x0Fu) << 4));

  LCD_E = 1;     /* data has been stable for > 80 ns already */
  __delay_us(1); /* E high >= 230 ns                          */
  LCD_E = 0;     /* LCD latches the nibble on this edge       */
  __delay_us(1); /* E cycle >= 500 ns                         */
}

/* Send a full byte as two nibbles, high nibble first. */
static void LCD_WriteByte(uint8_t value, uint8_t rs) {
  LCD_RS = (rs != 0u) ? 1 : 0; /* RS set before E rises (>= 40 ns) */
  LCD_WriteNibble((uint8_t)(value >> 4));
  LCD_WriteNibble((uint8_t)(value & 0x0Fu));
}

/* ---------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------- */

void LCD_Command(uint8_t cmd) {
  LCD_WriteByte(cmd, 0u);

  /* 0x01 (clear) and 0x02/0x03 (home) are the slow instructions */
  if (cmd <= 0x03u) {
    __delay_ms(LCD_LONG_MS);
  } else {
    __delay_us(LCD_SHORT_US);
  }
}

void LCD_Char(char c) {
  LCD_WriteByte((uint8_t)c, 1u);
  __delay_us(LCD_SHORT_US);
}

void LCD_Init(void) {
  /* RD2..RD7: digital outputs, driven low. RD0/RD1 left alone. */
  ANSELD &= LCD_KEEP_MASK;
  LATD &= LCD_KEEP_MASK;
  TRISD &= LCD_KEEP_MASK;

  /* Wait for the LCD supply to settle */
  __delay_ms(LCD_POWERUP_MS);

  /* Initialization by instruction (datasheet Figure 24, 4-bit) */
  LCD_RS = 0;
  LCD_WriteNibble(0x03u);
  __delay_ms(LCD_INIT_WAIT1_MS);
  LCD_WriteNibble(0x03u);
  __delay_us(LCD_INIT_WAIT2_US);
  LCD_WriteNibble(0x03u);
  __delay_us(LCD_INIT_WAIT2_US);
  LCD_WriteNibble(0x02u); /* switch to 4-bit interface */
  __delay_us(LCD_INIT_WAIT2_US);

  /* From here on, every byte is sent as two nibbles */
  LCD_Command(LCD_CMD_FUNC_4BIT_2L); /* 4-bit, 2 lines, 5x8 dots    */
  LCD_Command(LCD_CMD_DISPLAY_OFF);
  LCD_Command(LCD_CMD_CLEAR);
  LCD_Command(LCD_CMD_ENTRY_INC);
  LCD_Command(LCD_CMD_DISPLAY_ON);
}

void LCD_Clear(void) { LCD_Command(LCD_CMD_CLEAR); }

void LCD_SetCursor(uint8_t row, uint8_t col) {
  uint8_t address;

  if (col >= LCD_COLS) {
    col = (uint8_t)(LCD_COLS - 1u);
  }

  /* 2-line display: line 1 starts at DDRAM 0x00, line 2 at 0x40 */
  address = (row == 0u) ? 0x00u : 0x40u;

  LCD_Command((uint8_t)(LCD_CMD_SET_DDRAM | (uint8_t)(address + col)));
}

void LCD_Print(const char *s) {
  while (*s != '\0') {
    LCD_Char(*s);
    s++;
  }
}

void LCD_PrintLine(uint8_t row, const char *s) {
  uint8_t i;

  LCD_SetCursor(row, 0u);

  for (i = 0u; i < LCD_COLS; i++) {
    if (*s != '\0') {
      LCD_Char(*s);
      s++;
    } else {
      LCD_Char(' '); /* pad so old characters are overwritten */
    }
  }
}
