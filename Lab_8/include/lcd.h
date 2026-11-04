/*
 * lcd.h - HD44780 16x2 character LCD driver (4-bit, write-only)
 *
 * ECE 3301 - Microcontrollers | Lab 8
 * Target: PIC18F46K22, MPLAB XC8
 *
 * Wiring (fixed by this driver):
 *     LCD RS     -> RD2
 *     LCD E      -> RD3
 *     LCD D4..D7 -> RD4..RD7
 *     LCD R/W    -> GND   (write-only: the busy flag is never read)
 *     LCD D0..D3 -> not connected
 *
 *     RD0 and RD1 are never touched (reserved for MSSP2 I2C).
 *
 * Requirements:
 *     - _XTAL_FREQ must be defined for the whole project and must equal
 *       the real CPU clock in Hz (see lcd.c).
 *     - Call these functions from main-loop code only, never from an ISR.
 *     - Every function blocks briefly (microseconds to a few ms).
 *       LCD_Init() blocks about 60 ms and is meant for startup only.
 */

#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define LCD_COLS 16u
#define LCD_ROWS 2u

/* HD44780 instruction codes used by this driver */
#define LCD_CMD_CLEAR 0x01u     /* clear display, cursor home (slow) */
#define LCD_CMD_HOME 0x02u      /* cursor home (slow)                */
#define LCD_CMD_ENTRY_INC 0x06u /* increment address, no shift      */
#define LCD_CMD_DISPLAY_OFF 0x08u
#define LCD_CMD_DISPLAY_ON 0x0Cu   /* display on, cursor off, no blink  */
#define LCD_CMD_FUNC_4BIT_2L 0x28u /* 4-bit bus, 2 lines, 5x8 font      */
#define LCD_CMD_SET_DDRAM 0x80u    /* OR with a DDRAM address           */

/* Initialize PORTD pins RD2..RD7 and the LCD. Call once at startup. */
void LCD_Init(void);

/* Send one instruction byte (RS = 0) and wait its execution time. */
void LCD_Command(uint8_t cmd);

/* Write one character at the current cursor position (RS = 1). */
void LCD_Char(char c);

/* Clear the whole display (takes 3 ms - do not use in periodic updates). */
void LCD_Clear(void);

/* Move the cursor. row: 0 = top, 1 = bottom. col: 0..15. */
void LCD_SetCursor(uint8_t row, uint8_t col);

/* Write a string starting at the current cursor position (no padding). */
void LCD_Print(const char *s);

/* Write a full line: exactly 16 characters, padded with spaces or
 * truncated. Use this for periodic dashboard updates - no flicker,
 * no leftover characters. */
void LCD_PrintLine(uint8_t row, const char *s);

#endif /* LCD_H */
