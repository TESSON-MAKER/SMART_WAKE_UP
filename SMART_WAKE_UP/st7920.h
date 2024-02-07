#ifndef ST7920_H_
#define ST7920_H_

#include "stdint.h"
#include <stdarg.h>
#include "stm32f7xx.h"

void ST7920_Char(uint8_t x, uint8_t y, uint8_t *font_buffer, uint8_t c);
void ST7920_SendString(int row, int col, char* string);
void ST7920_Font_Print(uint8_t color, int16_t x, int16_t y, uint8_t *font_buffer, const char *format, ...);
void ST7920_GraphicMode (int enable) ;
void ST7920_DrawBitmap(const unsigned char* graphic);
void ST7920_Send_GLCD_Buffer(void);
void ST7920_Clear(void);
void ST7920_Clear_GLCD_Buffer(void);
void ST7920_Init(void);
void DrawLine(uint8_t color, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void DrawRectangle(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void DrawFilledRectangle(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void DrawCircle(uint8_t color, uint8_t x0, uint8_t y0, uint8_t radius);
void DrawFilledCircle(uint8_t color, int16_t x0, int16_t y0, int16_t r);
void DrawTriangle(uint8_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3);
void DrawFilledTriangle(uint8_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3);
void SetPixel(uint8_t pixel, int16_t x, int16_t y);

#endif /* ST7920_H_ */
