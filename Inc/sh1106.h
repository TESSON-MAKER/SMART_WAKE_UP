#ifndef SH1106_H_
#define SH1106_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stm32f7xx.h>

#include "../Fonts/fonts.h"

//Pins activated/desactivated
#define SH1106_DC_LOW (GPIOA->BSRR=GPIO_BSRR_BR0)
#define SH1106_DC_HIGH (GPIOA->BSRR=GPIO_BSRR_BS0)

#define SH1106_CS_LOW (GPIOC->BSRR=GPIO_BSRR_BR1)
#define SH1106_CS_HIGH (GPIOC->BSRR=GPIO_BSRR_BS1)

#define SH1106_RST_LOW (GPIOC->BSRR=GPIO_BSRR_BR0)
#define SH1106_RST_HIGH (GPIOC->BSRR=GPIO_BSRR_BS0)

// SPI1_AF in alternate fonction
#define SH1106_SPI1_AF 0x05 

// Timeout
#define SH1106_TIMEOUT 1000

// Screen dimensions
#define SH1106_WIDTH     (uint16_t) 132
#define SH1106_HEIGHT    (uint8_t) 64
#define SH1106_DATA_SIZE (uint8_t) 8

// SH1106 command definitions 
#define SH1106_CMD_COL_LOW      (uint8_t) 0x00 // Set Lower Column Address
#define SH1106_CMD_COL_HIGH     (uint8_t) 0x10 // Set Higher Column Address
#define SH1106_CMD_STARTLINE    (uint8_t) 0x40 // Set Display Start Line
#define SH1106_CMD_CONTRAST     (uint8_t) 0x81 // Set Contrast Control Register (Double Bytes Command)
#define SH1106_CMD_SEG_NORM     (uint8_t) 0xA0 // Set Segment Re-map (X coordinate normal)
#define SH1106_CMD_SEG_INV      (uint8_t) 0xA1 // Set Segment Re-map (X coordinate inverted)
#define SH1106_CMD_EDON         (uint8_t) 0xA5 // Set Entire Display OFF/ON (Right rotates)
#define SH1106_CMD_EDOFF        (uint8_t) 0xA4 // Set Entire Display OFF/ON (Left rotates)
#define SH1106_CMD_INV_OFF      (uint8_t) 0xA6 // Set Normal/Reverse Display (pixels in normal display)
#define SH1106_CMD_INV_ON       (uint8_t) 0xA7 // Set Normal/Reverse Display (all pixels inverted)
#define SH1106_CMD_SETMUX       (uint8_t) 0xA8 // Set Multiplex Ration (Double Bytes Command)
#define SH1106_CMD_DCDC         (uint8_t) 0xAD // Set DC-DC OFF/ON (Double Bytes Command)
#define SH1106_CMD_DCDC_ON      (uint8_t) 0x8B // Set DC-DC ON
#define SH1106_CMD_DCDC_OFF     (uint8_t) 0x8A // Set DC-DC OFF
#define SH1106_CMD_DISP_ON      (uint8_t) 0xAF // Set Display ON
#define SH1106_CMD_DISP_OFF     (uint8_t) 0xAE // Set Display OFF (sleep mode)
#define SH1106_CMD_PAGE_ADDR    (uint8_t) 0xB0 // Set Page Address
#define SH1106_CMD_COM_NORM     (uint8_t) 0xC0 // Set Common Output Scan Direction (Y coordinate normal)
#define SH1106_CMD_COM_INV      (uint8_t) 0xC8 // Set Common Output Scan Direction (Y coordinate inverted)
#define SH1106_CMD_SETOFFS      (uint8_t) 0xD3 // Set display offset (Double Bytes Command)
#define SH1106_CMD_CLOCKDIV     (uint8_t) 0xD5 // Set display clock divide ratio/oscillator frequency (Double Bytes Command)
#define SH1106_CMD_DPCHARGE_PER (uint8_t) 0xD9 // Set Dis-charge/Pre-charge Period (Double Bytes Command)
#define SH1106_CMD_COM_HW       (uint8_t) 0xDA // Set COM pins hardware configuration (Double Bytes Command)
#define SH1106_CMD_VCOM_DLEVEL  (uint8_t) 0xDB // Set VCOM Deselect Level (Double Bytes Command)

#define SH1106_CMD_RMWRITE      (uint8_t) 0xE0 // Read-Modify-Write
#define SH1106_CMD_END          (uint8_t) 0xEE // End
#define SH1106_CMD_NOP          (uint8_t) 0xE3 // Nop

#define XLevelL                 (uint8_t) 0x02
#define XLevelH                 (uint8_t) 0x10
#define YLevel                  (uint8_t) 0xB0

void SH1106_Init(void);
void SH1106_SendCmd(uint8_t cmd);
void SH1106_SetPixel(uint8_t pixel, int16_t x, int16_t y);
void SH1106_DrawCharacter(uint8_t color, int16_t x, int16_t y, const Font *font, uint8_t letterNumber);
void SH1106_FontPrint(uint8_t color, int16_t x, int16_t y, const Font *font, const char *format, ...);
void SH1106_DrawLine(uint8_t color, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void SH1106_DrawRectangle(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void SH1106_DrawFilledRectangle(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void SH1106_DrawFilledCircle(uint8_t color, int16_t x0, int16_t y0, int16_t r);
void SH1106_DrawCircle(uint8_t color, uint8_t x0, uint8_t y0, uint8_t radius);
void SH1106_ClearBuffer(void);
void SH1106_SendBuffer(void);

#endif /* SH1106_H_ */
