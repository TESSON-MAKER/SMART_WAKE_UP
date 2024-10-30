#ifndef ST7920_H_
#define ST7920_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stm32f7xx.h>

#include "../Fonts/fonts.h"

//Pins activated/desactivated
#define ST7920_CS_LOW (GPIOC->BSRR=GPIO_BSRR_BR1)
#define ST7920_CS_HIGH (GPIOC->BSRR=GPIO_BSRR_BS1)

#define ST7920_RST_LOW (GPIOC->BSRR=GPIO_BSRR_BR0)
#define ST7920_RST_HIGH (GPIOC->BSRR=GPIO_BSRR_BS0)

// SPI1_AF in alternate fonction
#define ST7920_SPI1_AF 0x05 

// Screen dimensions
#define ST7920_WIDTH     (uint8_t) 128
#define ST7920_HEIGHT    (uint8_t) 64
#define ST7920_DATA_SIZE (uint8_t) 8

static uint8_t Graphic_Check = 0;

// ST7920 command definitions 
#define ST7920_CMD              (uint8_t) 0xF8 // Command mode
#define ST7920_DATA             (uint8_t) 0xFA // Data mode
#define ST7920_FOUR_STRONG_BITS (uint8_t) 0xF0 // Four strong bits

// Basic commands
#define ST7920_CMD_LCD_CLS      (uint8_t) 0x01 // Clear LCD screen
#define ST7920_CMD_HOME         (uint8_t) 0x02 // Return cursor to home position (0,0)
#define ST7920_CMD_ADDRINC      (uint8_t) 0x06 // Increment cursor address after writing each character
#define ST7920_CMD_DISPLAYON    (uint8_t) 0x0C // Turn on display without cursor
#define ST7920_CMD_DISPLAYOFF   (uint8_t) 0x08 // Turn off display
#define ST7920_CMD_CURSORON     (uint8_t) 0x0E // Turn on cursor without display
#define ST7920_CMD_CURSORBLINK  (uint8_t) 0x0F // Make cursor blink
#define ST7920_CMD_BASIC        (uint8_t) 0x30 // Basic display mode
#define ST7920_CMD_EXTEND       (uint8_t) 0x34 // Extended display mode
#define ST7920_CMD_GFXMODE      (uint8_t) 0x36 // Graphic mode
#define ST7920_CMD_TXTMODE      (uint8_t) 0x34 // Text mode
#define ST7920_CMD_STANDBY      (uint8_t) 0x01 // Put display in standby mode
#define ST7920_CMD_SCROLL       (uint8_t) 0x03 // Enable scrolling
#define ST7920_CMD_SCROLLADDR   (uint8_t) 0x40 // Start address of scrolling area

// Cursor position commands
#define ST7920_CMD_ADDR         (uint8_t) 0x80 // Address of cursor position
#define ST7920_CMD_LINE0        (uint8_t) 0x80 // Set cursor to first line
#define ST7920_CMD_LINE1        (uint8_t) 0x90 // Set cursor to second line
#define ST7920_CMD_LINE2        (uint8_t) 0x88 // Set cursor to third line
#define ST7920_CMD_LINE3        (uint8_t) 0x98 // Set cursor to fourth line

// Extended commands
#define ST7920_EXT_BASIC        (uint8_t) 0x30 // Set to basic instruction set
#define ST7920_EXT_EXTEND       (uint8_t) 0x34 // Set to extended instruction set
#define ST7920_EXT_GFXMODE      (uint8_t) 0x36 // Set to graphic display mode
#define ST7920_EXT_TXTMODE      (uint8_t) 0x34 // Set to text display mode
#define ST7920_EXT_SCROLLADDR   (uint8_t) 0x40 // Set scroll address
#define ST7920_EXT_ADDR         (uint8_t) 0x80 // Set DDRAM address
#define ST7920_EXT_GFX_ADDR     (uint8_t) 0x80 // Set GDRAM address

// ST7920 reverse display command definitions
#define ST7920_CMD_REVERSE_LINE0 (uint8_t) 0x24 // Reverse display of the first line
#define ST7920_CMD_REVERSE_LINE1 (uint8_t) 0x25 // Reverse display of the second line
#define ST7920_CMD_REVERSE_LINE2 (uint8_t) 0x26 // Reverse display of the third line
#define ST7920_CMD_REVERSE_LINE3 (uint8_t) 0x27 // Reverse display of the fourth line

// Buffer for display data
static uint8_t ST7920_Buffer[(ST7920_WIDTH * ST7920_HEIGHT) / ST7920_DATA_SIZE];

void ST7920_Init(void);
void ST7920_GraphicMode(int enable);
void ST7920_SetPixel(uint8_t pixel, int16_t x, int16_t y);
void ST7920_DrawCharacter(uint8_t color, int16_t x, int16_t y, const Font *font, uint8_t letterNumber);
void ST7920_FontPrint(uint8_t color, int16_t x, int16_t y, const Font *font, const char *format, ...);
void ST7920_DrawLine(uint8_t color, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void ST7920_DrawRectangle(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void ST7920_DrawFilledRectangle(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void ST7920_DrawFilledCircle(uint8_t color, int16_t x0, int16_t y0, int16_t r);
void ST7920_DrawCircle(uint8_t color, uint8_t x0, uint8_t y0, uint8_t radius);
void ST7920_ClearBuffer(void);
void ST7920_SendBuffer(void);

#endif /* ST7920_H_ */
