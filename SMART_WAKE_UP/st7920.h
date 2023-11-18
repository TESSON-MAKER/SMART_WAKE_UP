/*
 * ST7920.h
 *
 *  Created on: Nov 16, 2022
 *      Author: hussamaldean
 */

#ifndef ST7920_H_
#define ST7920_H_

#include "stdint.h"
#include "stm32f7xx.h"

void ST7920_Char(uint8_t x, uint8_t y, uint8_t *font_buffer, uint8_t c);
void ST7920_SendString(int row, int col, char* string);
void ST7920_Font_Print(int16_t x, int16_t y, uint8_t *font_buffer, const char *str);
void ST7920_GraphicMode (int enable) ;
void ST7920_DrawBitmap(const unsigned char* graphic);
void ST7920_Send_GLCD_Buffer(void);
void ST7920_Clear(void);
void ST7920_Clear_GLCD_Buffer(void);
void ST7920_init(void);
void DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void DrawCircle(uint8_t x0, uint8_t y0, uint8_t radius);
void DrawFilledCircle(int16_t x0, int16_t y0, int16_t r);
void DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3);
void DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3);
void SetPixel(int16_t x, int16_t y);
void ResetPixel(int16_t x, int16_t y);

#endif /* ST7920_H_ */
