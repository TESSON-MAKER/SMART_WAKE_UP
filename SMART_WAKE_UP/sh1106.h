#ifndef SH1106_H_
#define SH1106_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#include "stm32f7xx.h"
#include "fonts.h"

void SH1106_Init(void);
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
