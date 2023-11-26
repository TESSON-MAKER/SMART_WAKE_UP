/*
 * ST7920.c
 *
 *  Created on: Nov 16, 2022
 *      Author: hussamaldean
 */


/*
 *
 * CS-> PA0
 * RST->PA1
 * */

#include <stdio.h>
#include "st7920.h"
#include "tim.h"

#define CS_LOW 		(GPIOA->BSRR=GPIO_BSRR_BR0)
#define CS_HIGH 	(GPIOA->BSRR=GPIO_BSRR_BS0)

#define RST_LOW 	(GPIOA->BSRR=GPIO_BSRR_BR1)
#define RST_HIGH 	(GPIOA->BSRR=GPIO_BSRR_BS1)

#define SCK_LOW 	(GPIOA->BSRR=GPIO_BSRR_BR5)
#define SCK_HIGH 	(GPIOA->BSRR=GPIO_BSRR_BS5)

#define SID_LOW 	(GPIOA->BSRR=GPIO_BSRR_BR7)
#define SID_HIGH 	(GPIOA->BSRR=GPIO_BSRR_BS7)


static uint8_t numRows = 64;
static uint8_t numCols = 128;
static uint8_t Graphic_Check = 0;
static uint8_t GLCD_Buffer[(128*64)/8];

#define LCD_CLS         0x01
#define LCD_HOME        0x02
#define LCD_ADDRINC     0x06
#define LCD_DISPLAYON   0x0C
#define LCD_DISPLAYOFF  0x08
#define LCD_CURSORON    0x0E
#define LCD_CURSORBLINK 0x0F
#define LCD_BASIC       0x30
#define LCD_EXTEND      0x34
#define LCD_GFXMODE     0x36
#define LCD_TXTMODE     0x34
#define LCD_STANDBY     0x01
#define LCD_SCROLL      0x03
#define LCD_SCROLLADDR  0x40
#define LCD_ADDR        0x80
#define LCD_LINE0       0x80
#define LCD_LINE1       0x90
#define LCD_LINE2       0x88
#define LCD_LINE3       0x98

static void ST7920_spi_init(void)
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; //enable clock for GPIOA
	
	//Initialisation de la pin PA1-CS
	GPIOA->MODER |= GPIO_MODER_MODER1_0;
	GPIOA->MODER &= ~GPIO_MODER_MODER1_1;

	//Initialisation de la pin PA0-RST
	GPIOA->MODER |= GPIO_MODER_MODER0_0;
	GPIOA->MODER &= ~GPIO_MODER_MODER0_1;

	//Initialisation de la pin PA5-SCK
	GPIOA->MODER |= GPIO_MODER_MODER5_0;
	GPIOA->MODER &= ~GPIO_MODER_MODER5_1;

	//Initialisation de la pin PA7-MOSI
	GPIOA->MODER |= GPIO_MODER_MODER7_0;
	GPIOA->MODER &= ~GPIO_MODER_MODER7_1;
}


static void SendByteSPI(uint8_t byte)
{
	for(int i=0;i<8;i++)
	{
		if((byte<<i)&0x80)
			{
				SID_HIGH;  // SID=1  OR MOSI
			}

		else {SID_LOW;}  // SID=0
		SCK_HIGH;  // SCLK =0  OR SCK
		TIM_WaitMicros(1);
		SCK_LOW;  // SCLK=1
		TIM_WaitMicros(1);
	}
}

static void ST7920_SendCmd (uint8_t cmd)
{

	CS_HIGH;  // PUll the CS high

	SendByteSPI(0xf8+(0<<1));  // send the SYNC + RS(0)
	SendByteSPI(cmd&0xf0);  // send the higher nibble first
	SendByteSPI((cmd<<4)&0xf0);  // send the lower nibble
	TIM_WaitMicros(20);

	CS_LOW;  // PUll the CS LOW

}

static void ST7920_SendData (uint8_t data)
{

	CS_HIGH;

	SendByteSPI(0xf8+(1<<1));  // send the SYNC + RS(1)
	SendByteSPI(data&0xf0);  // send the higher nibble first
	SendByteSPI((data<<4)&0xf0);  // send the lower nibble
	TIM_WaitMicros(20);
	
	CS_LOW;  // PUll the CS LOW
}

void ST7920_SendString(int row, int col, char* string)
{
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0x90;
            break;
        case 2:
            col |= 0x88;
            break;
        case 3:
            col |= 0x98;
            break;
        default:
            col |= 0x80;
            break;
    }

    ST7920_SendCmd(col);

    while (*string)
    	{
    		ST7920_SendData(*string++);
    	}
}

const uint8_t MIN_ASCII_VALUE = 31;
const uint8_t MAX_ASCII_VALUE = 127;
const uint8_t ASCII_OFFSET = 32;

void ST7920_Font_Print(int16_t x, int16_t y, uint8_t *font_buffer, const char *str) 
{
	// Retrieving font data from font_buffer
	uint8_t dataSize = font_buffer[0];
	uint8_t length = font_buffer[1];
	uint8_t height = font_buffer[2];
	uint8_t bytesPerColumns = font_buffer[3];
		
	while (*str && x < numCols && y < numRows) 
	{
		uint8_t currentChar = *str;
		if (currentChar < MIN_ASCII_VALUE || currentChar > MAX_ASCII_VALUE) return; // Check if ASCII value is valid

		uint8_t letterNumber = currentChar - ASCII_OFFSET; // Character number in font_buffer
		uint8_t letterSize = font_buffer[4 + letterNumber * dataSize]; // Character size
				
		for (int column = 0; column < letterSize; column++) 
		{
			for (int byteColumn = 0; byteColumn < bytesPerColumns; byteColumn++) 
			{
				uint8_t data = font_buffer[5 + letterNumber * dataSize + byteColumn + bytesPerColumns * column];
				for (int bit = 0; bit < 8; bit++) 
				{
					uint8_t pixel = (data >> bit) & 1;
					int16_t a = x + column;
					int16_t b = y + (bit + 8 * byteColumn);
					SetPixel(pixel, a, b);
				}
			}
		}
		x += letterSize + (length / 10); // Increment x by character width plus space
		str++; // Move to the next character in the string
	}
}

// switch to graphic mode or normal mode::: enable = 1 -> graphic mode enable = 0 -> normal mode

void ST7920_GraphicMode (int enable)   // 1-enable, 0-disable
{
	if (enable == 1)
	{
		ST7920_SendCmd(0x30);  // 8 bit mode
		TIM_Wait(1);
		ST7920_SendCmd(0x34);  // switch to Extended instructions
		TIM_Wait(1);
		ST7920_SendCmd(0x36);  // enable graphics
		TIM_Wait(1);
		Graphic_Check = 1;  // update the variable
	}

	else if (enable == 0)
	{
		ST7920_SendCmd(0x30);  // 8 bit mode
		TIM_Wait(1);
		Graphic_Check = 0;  // update the variable
	}
}

void ST7920_DrawBitmap(const unsigned char* graphic)
{
	uint8_t x, y;
	for(y = 0; y < 64; y++)
	{
		if(y < 32)
		{
			for(x = 0; x < 8; x++)							// Draws top half of the screen.
			{												// In extended instruction mode, vertical and horizontal coordinates must be specified before sending data in.
				ST7920_SendCmd(0x80 | y);				// Vertical coordinate of the screen is specified first. (0-31)
				ST7920_SendCmd(0x80 | x);				// Then horizontal coordinate of the screen is specified. (0-8)
				ST7920_SendData(graphic[2*x + 16*y]);		// Data to the upper byte is sent to the coordinate.
				ST7920_SendData(graphic[2*x+1 + 16*y]);	// Data to the lower byte is sent to the coordinate.
			}
		}
		else
		{
			for(x = 0; x < 8; x++)							// Draws bottom half of the screen.
			{												// Actions performed as same as the upper half screen.
				ST7920_SendCmd(0x80 | (y-32));			// Vertical coordinate must be scaled back to 0-31 as it is dealing with another half of the screen.
				ST7920_SendCmd(0x88 | x);
				ST7920_SendData(graphic[2*x + 16*y]);
				ST7920_SendData(graphic[2*x+1 + 16*y]);
			}
		}
	}
}


// Update the display with the selected graphics
void ST7920_Send_GLCD_Buffer(void)
{
	ST7920_DrawBitmap(GLCD_Buffer);
}

void ST7920_Clear_GLCD_Buffer(void)
{
	for (int i = 0; i<1023; i++) GLCD_Buffer[i] = 0;
}

void ST7920_Clear(void)
{
	if (Graphic_Check == 1)  // if the graphic mode is set
	{
		uint8_t x, y;
		for(y = 0; y < 64; y++)
		{
			if(y < 32)
			{
				ST7920_SendCmd(0x80 | y);
				ST7920_SendCmd(0x80);
			}
			else
			{
				ST7920_SendCmd(0x80 | (y-32));
				ST7920_SendCmd(0x88);
			}
			for(x = 0; x < 8; x++)
			{
				ST7920_SendData(0);
				ST7920_SendData(0);
			}
		}
	}

	else
	{
		ST7920_SendCmd(0x01);   // clear the display using command
		TIM_Wait(20); // TIM_Wait>1.6 ms
	}
}

void SetPixel(uint8_t pixel, int16_t x, int16_t y) 
{
    if (x < numCols && y < numRows && x >= 0 && y >= 0) 
		{
        if (pixel == 1) GLCD_Buffer[y * (numCols/8) + (x/8)] |= 0x80u >> (x%8);
        else GLCD_Buffer[y * (numCols/8) + (x/8)] &= ~(0x80u >> (x%8));
    }
}


/* draw a line
 * start point (X0, Y0)
 * end point (X1, Y1)
 */
void DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	int dx = (x1 >= x0) ? x1 - x0 : x0 - x1;
	int dy = (y1 >= y0) ? y1 - y0 : y0 - y1;
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

  for (;;)
  {
    SetPixel(1, x0, y0);
    if (x0 == x1 && y0 == y1) break;
    int e2 = err + err;
    if (e2 > -dy)
    {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx)
    {
      err += dx;
      y0 += sy;
    }
  }
}

/* Draw rectangle
 * start point (x,y)
 * w -> width
 * h -> height
 */
void DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	/* Check input parameters */
	if (
		x >= numCols ||
		y >= numRows
	) {
		/* Return error */
		return;
	}

	/* Check width and height */
	if ((x + w) >= numCols) {
		w = numCols - x;
	}
	if ((y + h) >= numRows) {
		h = numRows - y;
	}

	/* Draw 4 lines */
	DrawLine(x, y, x + w, y);         /* Top line */
	DrawLine(x, y + h, x + w, y + h); /* Bottom line */
	DrawLine(x, y, x, y + h);         /* Left line */
	DrawLine(x + w, y, x + w, y + h); /* Right line */
}




/* Draw filled rectangle
 * Start point (x,y)
 * w -> width
 * h -> height
 */
void DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint8_t i;

	/* Check input parameters */
	if (
		x >= numCols ||
		y >= numRows
	) {
		/* Return error */
		return;
	}

	/* Check width and height */
	if ((x + w) >= numCols) {
		w = numCols - x;
	}
	if ((y + h) >= numRows) {
		h = numRows - y;
	}

	/* Draw lines */
	for (i = 0; i <= h; i++) {
		/* Draw lines */
		DrawLine(x, y + i, x + w, y + i);
	}
}




/* draw circle
 * centre (x0,y0)
 * radius = radius
 */
void DrawCircle(uint8_t x0, uint8_t y0, uint8_t radius)
{
  int f = 1 - (int)radius;
  int ddF_x = 1;

  int ddF_y = -2 * (int)radius;
  int x = 0;

  SetPixel(1, x0, y0 + radius);
  SetPixel(1, x0, y0 - radius);
  SetPixel(1, x0 + radius, y0);
  SetPixel(1, x0 - radius, y0);

  int y = radius;
  while(x < y)
  {
    if(f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    SetPixel(1, x0 + x, y0 + y);
    SetPixel(1, x0 - x, y0 + y);
    SetPixel(1, x0 + x, y0 - y);
    SetPixel(1, x0 - x, y0 - y);
    SetPixel(1, x0 + y, y0 + x);
    SetPixel(1, x0 - y, y0 + x);
    SetPixel(1, x0 + y, y0 - x);
    SetPixel(1, x0 - y, y0 - x);
  }
}


// Draw Filled Circle

void DrawFilledCircle(int16_t x0, int16_t y0, int16_t r)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    SetPixel(1, x0, y0 + r);
    SetPixel(1, x0, y0 - r);
    SetPixel(1, x0 + r, y0);
    SetPixel(1, x0 - r, y0);
    DrawLine(x0 - r, y0, x0 + r, y0);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        DrawLine(x0 - x, y0 + y, x0 + x, y0 + y);
        DrawLine(x0 + x, y0 - y, x0 - x, y0 - y);

        DrawLine(x0 + y, y0 + x, x0 - y, y0 + x);
        DrawLine(x0 + y, y0 - x, x0 - y, y0 - x);
    }
}



// Draw Traingle with coordimates (x1, y1), (x2, y2), (x3, y3)
void DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3)
{
	/* Draw lines */
	DrawLine(x1, y1, x2, y2);
	DrawLine(x2, y2, x3, y3);
	DrawLine(x3, y3, x1, y1);
}



// Draw Filled Traingle with coordimates (x1, y1), (x2, y2), (x3, y3)
void DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3)
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
	yinc1 = 0, yinc2 = 0, den = 0, Column = 0, numadd = 0, numpixels = 0,
	curpixel = 0;

#define ABS(x)   ((x) > 0 ? (x) : -(x))

	deltax = ABS(x2 - x1);
	deltay = ABS(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	} else {
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	} else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay){
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		Column = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		Column = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
		DrawLine(x, y, x3, y3);

		Column += numadd;
		if (Column >= den) {
			Column -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}

void ST7920_init(void)
{
	TIM_Wait(100);
	ST7920_spi_init();

	RST_LOW;
	TIM_Wait(50);
	RST_HIGH;
	TIM_Wait(100);
	ST7920_SendCmd(0x30);  		// 8bit mode
	TIM_WaitMicros(110);  				//  >100us TIM_Wait

	ST7920_SendCmd(0x30);  		// 8bit mode
	TIM_WaitMicros(40);  				// >37us TIM_Wait

	ST7920_SendCmd(0x08);  // D=0, C=0, B=0
	TIM_WaitMicros(110);  // >100us TIM_Wait

	ST7920_SendCmd(0x01);  // clear screen
	TIM_Wait(12);  // >10 ms TIM_Wait

	ST7920_SendCmd(0x06);  // cursor increment right no shift
	TIM_Wait(1);  // 1ms TIM_Wait

	ST7920_SendCmd(0x0C);  // D=1, C=0, B=0
	TIM_Wait(1);  // 1ms TIM_Wait

	ST7920_SendCmd(0x02);  // return to home
	TIM_Wait(1);  // 1ms TIM_Wait
}
