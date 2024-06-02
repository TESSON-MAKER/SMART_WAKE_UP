#include "st7920.h"
#include "tim.h"

/*******************************************************************
 * @name       :ST7920_SpiInit
 * @date       :2024-01-03
 * @function   :SPI Initialization
 * @parameters :None
 * @retvalue   :None
********************************************************************/ 
static void ST7920_SpiInit(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // enable clock for GPIOA
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // enable clock for GPIOC

	// Initialization of pin PA0-DC
	GPIOA->MODER |= GPIO_MODER_MODER0_0;
	GPIOA->MODER &= ~GPIO_MODER_MODER0_1;

	// Initialization of pin PC0-CS
	GPIOC->MODER |= GPIO_MODER_MODER0_0;
	GPIOC->MODER &= ~GPIO_MODER_MODER0_1;

	// Initialization of pin PC1-RST
	GPIOC->MODER |= GPIO_MODER_MODER1_0;
	GPIOC->MODER &= ~GPIO_MODER_MODER1_1;

	// Initialization of pin PA5-SCK
	GPIOA->MODER |= GPIO_MODER_MODER5_1;
	GPIOA->MODER &= ~GPIO_MODER_MODER5_0;

	// Initialization of pin PA7-MOSI
	GPIOA->MODER |= GPIO_MODER_MODER7_1;
	GPIOA->MODER &= ~GPIO_MODER_MODER7_0;

	GPIOA->AFR[0] |= ST7920_SPI1_AF << GPIO_AFRL_AFRL5_Pos;
	GPIOA->AFR[0] |= ST7920_SPI1_AF << GPIO_AFRL_AFRL7_Pos;

	//Enable clock access to SPI1 module
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	//Set MSB first
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST;

	//Set mode to MASTER
	SPI1->CR1 |= SPI_CR1_MSTR;

	//Select software slave management by setting SSM=1 and SSI=1
	SPI1->CR1 |= SPI_CR1_SSM;
	SPI1->CR1 |= SPI_CR1_SSI;

	//Set SPI mode to be MODE1 (CPHA0 CPOL0)
	SPI1->CR1 |= SPI_CR1_CPHA;
	SPI1->CR1 &= ~SPI_CR1_CPOL;

	//Set the frequency of SPI to 500kHz
	SPI1->CR1 |= SPI_CR1_BR_2;

	//Enable SPI module
	SPI1->CR1 |= SPI_CR1_SPE;
}                                                  

/*******************************************************************
 * @name       :ST7920_SpiTransmit
 * @date       :2024-05-26
 * @function   :Send with spi
 * @parameters :data
 * @retvalue   :None
********************************************************************/
static void ST7920_SpiTransmit(uint8_t msg)
{
	//Wait until TXE is set
	while(!(SPI1->SR & (SPI_SR_TXE)));

	//Write the data to the data register
	*(volatile uint8_t*) & SPI1->DR = msg;
			
	//Wait until TXE is set
	while(!(SPI1->SR & (SPI_SR_TXE)));

	//Wait for BUSY flag to reset
	while((SPI1->SR & (SPI_SR_BSY)));

	//Clear OVR flag
	(void)SPI1->DR;
	(void)SPI1->SR;
}

/*******************************************************************
 * @name       :ST7920_SendCmd
 * @date       :2024-05-26
 * @function   :Send command
 * @parameters :cmd
 * @retvalue   :None
********************************************************************/
static void ST7920_SendCmd(uint8_t cmd)
{
	ST7920_CS_HIGH;  
	ST7920_SpiTransmit(ST7920_CMD);
	ST7920_SpiTransmit(cmd & ST7920_FOUR_STRONG_BITS);
	ST7920_SpiTransmit((cmd<<4) & ST7920_FOUR_STRONG_BITS);
	ST7920_CS_LOW;  
}

/*******************************************************************
 * @name       :ST7920_SendData
 * @date       :2024-05-26
 * @function   :Send data
 * @parameters :data
 * @retvalue   :None
********************************************************************/
static void ST7920_SendData (uint8_t data)
{
	ST7920_CS_HIGH;  
	ST7920_SpiTransmit(ST7920_DATA);
	ST7920_SpiTransmit(data & ST7920_FOUR_STRONG_BITS);
	ST7920_SpiTransmit((data<<4) & ST7920_FOUR_STRONG_BITS);
	ST7920_CS_LOW;  
}

/*******************************************************************
 * @name       :ST7920_SendString
 * @date       :2024-05-26
 * @function   :Send string
 * @parameters :row, col, string
 * @retvalue   :None
********************************************************************/ 
void ST7920_SendString(int row, int col, char* string)
{
	switch (row)
	{
		case 0:
			col |= ST7920_CMD_LINE0;
			break;
		case 1:
			col |= ST7920_CMD_LINE1;
			break;
		case 2:
			col |= ST7920_CMD_LINE2;
			break;
		case 3:
			col |= ST7920_CMD_LINE3;
			break;
		default:
			col |= ST7920_CMD_LINE0;
			break;
	}

	ST7920_SendCmd(col);
	while (*string) ST7920_SendData(*string++);
}

/*******************************************************************
 * @name       :ST7920_GraphicMode
 * @date       :2024-05-26
 * @function   :Select graphic mode
 * @parameters :enable (1 or 0)
 * @retvalue   :None
********************************************************************/ 
void ST7920_GraphicMode(int enable)
{
	if (enable)
	{
		ST7920_SendCmd(ST7920_CMD_BASIC);
		TIM_Wait(1);
		ST7920_SendCmd(ST7920_CMD_EXTEND);
		TIM_Wait(1);
		ST7920_SendCmd(ST7920_CMD_GFXMODE);
		TIM_Wait(1);
		Graphic_Check = 1;
	}
	else 
	{
		ST7920_SendCmd(ST7920_CMD_BASIC);
		TIM_Wait(1);
		Graphic_Check = 0;
	}
}

/*******************************************************************
 * @name       :ST7920_SendBuffer
 * @date       :2024-06-01
 * @function   :Select graphic mode
 * @parameters :enable (1 or 0)
 * @retvalue   :None
********************************************************************/ 
void ST7920_SendBuffer(void)
{
	for (uint8_t y = 0; y < 64; y++)
	{
		uint8_t verticalCoord = (y < 32) ? y : y - 32;
		uint8_t horizontalCmd = (y < 32) ? ST7920_CMD_LINE0 : ST7920_CMD_LINE2;

		for (uint8_t x = 0; x < 8; x++)
		{
			ST7920_SendCmd(ST7920_CMD_LINE0 | verticalCoord);
			ST7920_SendCmd(horizontalCmd | x);
			ST7920_SendData(ST7920_Buffer[2 * x + 16 * y]);
			ST7920_SendData(ST7920_Buffer[2 * x + 1 + 16 * y]);
		}
	}
}

/*******************************************************************
 * @name       : ST7920_SetPixel
 * @date       : 2024-06-01
 * @function   : Draw a character at specified position
 * @parameters : color, x, y, font, letterNumberAscii
 * @retvalue   : None
********************************************************************/
void ST7920_SetPixel(uint8_t color, int16_t x, int16_t y) 
{
	if (x >= 0 && x < ST7920_WIDTH && y >= 0 && y < ST7920_HEIGHT) 
	{
		uint16_t index = y * (ST7920_WIDTH / ST7920_DATA_SIZE) + (x / ST7920_DATA_SIZE);
		uint8_t bitOffset = 0x80u >> (x % ST7920_DATA_SIZE);

		if (color) ST7920_Buffer[index] |= bitOffset;
		else ST7920_Buffer[index] &= ~bitOffset;
	}
}

/*******************************************************************
 * @name       : ST7920_DrawCharacter
 * @date       : 2024-01-03
 * @function   : Draw a character at specified position
 * @parameters : color, x, y, font, letterNumberAscii
 * @retvalue   : None
********************************************************************/
void ST7920_DrawCharacter(uint8_t color, int16_t x, int16_t y, const Font *font, uint8_t letterNumberAscii) 
{
	if (letterNumberAscii < font->asciiBegin || letterNumberAscii > font->asciiEnd) return;
	
	uint8_t letterNumber = letterNumberAscii - font->asciiOffset;
	uint16_t index_letterSize = letterNumber * font->datasize;
	uint8_t letterSize = font->data[index_letterSize];

	for (int column = 0; column < letterSize; column++) 
	{
		for (int byteColumn = 0; byteColumn < font->bytesPerColums; byteColumn++) 
		{
			uint16_t index_Buffer = 1 + letterNumber * font->datasize + byteColumn + font->bytesPerColums * column;
			uint8_t data = font->data[index_Buffer];
			for (int bit = 0; bit < 8; bit++) 
			{
				uint8_t pixel = (data >> bit) & 1;
				int16_t a = x + column;
				int16_t b = y + (bit + 8 * byteColumn);
				if (pixel) ST7920_SetPixel(color, a, b);
			}
		}
	}
}

/*******************************************************************
 * @name       : ST7920_DrawStr
 * @date       : 2024-01-03
 * @function   : Set pixel in buffer
 * @parameters : color, x, y, font, content
 * @retvalue   : None
********************************************************************/ 
void ST7920_DrawStr(uint8_t color, int16_t x, int16_t y, const Font *font, const char *format)
{
	while (*format && x < ST7920_WIDTH && y < ST7920_HEIGHT) 
	{
		uint8_t currentChar = *format;

		ST7920_DrawCharacter(color, x, y, font, currentChar);

		// Create a space between the letters
		uint8_t letterNumber = currentChar - font->asciiOffset;
		uint16_t index_letterSize = letterNumber * font->datasize;
		uint8_t letterSize = font->data[index_letterSize];
		x += letterSize + (font->length / 10);

		// Go to next letter
		format++;
	}
}

/*******************************************************************
 * @name       : ST7920_FontPrint
 * @date       : 2024-01-03
 * @function   : Set pixel in buffer
 * @parameters : color, x, y, font, content
 * @retvalue   : None
********************************************************************/ 
void ST7920_FontPrint(uint8_t color, int16_t x, int16_t y, const Font *font, const char *format, ...) 
{
	va_list args;
	va_start(args, format);
	char formatted_string[50]; 
	vsprintf(formatted_string, format, args);
	va_end(args);

	ST7920_DrawStr(color, x, y, font, formatted_string); 
}

/*******************************************************************
 * @name       :ST7920_DrawLine
 * @date       :2024-01-03
 * @function   :Draw a line
 * @parameters :color, x0, y0, x1, y1
 * @retvalue   :None
********************************************************************/ 
void ST7920_DrawLine(uint8_t color, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) 
{
	int dx = (x1 >= x0) ? x1 - x0 : x0 - x1;
	int dy = (y1 >= y0) ? y1 - y0 : y0 - y1;
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	while(1)
	{
		ST7920_SetPixel(color, x0, y0);
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

/*******************************************************************
 * @name       :ST7920_DrawRectangle
 * @date       :2024-01-03
 * @function   :Draw rectangle
 * @parameters :color, x, y, w, h
 * @retvalue   :None
********************************************************************/ 
void ST7920_DrawRectangle(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	//Check input parameters
	if (x >= ST7920_WIDTH || y >= ST7920_HEIGHT) return;

	//Check width and height
	if ((x + w) >= ST7920_WIDTH) w = ST7920_WIDTH - x;
	if ((y + h) >= ST7920_HEIGHT) h = ST7920_HEIGHT - y;

	//Draw 4 lines
	ST7920_DrawLine(color, x, y, x + w, y);         //Top line
	ST7920_DrawLine(color, x, y + h, x + w, y + h); //Bottom line
	ST7920_DrawLine(color, x, y, x, y + h);         //Left line
	ST7920_DrawLine(color, x + w, y, x + w, y + h); //Right line
}

/*******************************************************************
 * @name       :ST7920_DrawFilledRectangle
 * @date       :2024-01-03
 * @function   :Draw rectangle
 * @parameters :color, x, y, w, h
 * @retvalue   :None
********************************************************************/
void ST7920_DrawFilledRectangle(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	//Check input parameters
	if (x >= ST7920_WIDTH || y >= ST7920_HEIGHT) return;

	//Check width and height
	if ((x + w) >= ST7920_WIDTH) w = ST7920_WIDTH - x;
	if ((y + h) >= ST7920_HEIGHT) h = ST7920_HEIGHT - y;

	//Draw lines
	for (int i = 0; i <= h; i++)
		ST7920_DrawLine(color, x, y + i, x + w, y + i);
}

/*******************************************************************
 * @name       :ST7920_DrawCircle
 * @date       :2024-01-03
 * @function   :Draw circle
 * @parameters :color, x0, y0, radius
 * @retvalue   :None
********************************************************************/ 
void ST7920_DrawCircle(uint8_t color, uint8_t x0, uint8_t y0, uint8_t radius)
{
	int x = radius;
	int y = 0;
	int err = 0;

	ST7920_SetPixel(color, x0, y0 + radius);
	ST7920_SetPixel(color, x0, y0 - radius);
	ST7920_SetPixel(color, x0 + radius, y0);
	ST7920_SetPixel(color, x0 - radius, y0);

	while (x >= y)
	{
		ST7920_SetPixel(color, x0 + x, y0 + y);
		ST7920_SetPixel(color, x0 - x, y0 + y);
		ST7920_SetPixel(color, x0 + x, y0 - y);
		ST7920_SetPixel(color, x0 - x, y0 - y);
		ST7920_SetPixel(color, x0 + y, y0 + x);
		ST7920_SetPixel(color, x0 - y, y0 + x);
		ST7920_SetPixel(color, x0 + y, y0 - x);
		ST7920_SetPixel(color, x0 - y, y0 - x);

		y++;
		err += 1 + 2*y;

		if (2*(err - x) + 1 > 0)
		{
			x--;
			err += 1 - 2*x;
		}
	}
}

/*******************************************************************
 * @name       :ST7920_DrawFilledCircle
 * @date       :2024-01-03
 * @function   :Draw filled circle
 * @parameters :color, x0, y0, radius
 * @retvalue   :None
********************************************************************/ 
void ST7920_DrawFilledCircle(uint8_t color, int16_t x0, int16_t y0, int16_t r)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	ST7920_SetPixel(color, x0, y0 + r);
	ST7920_SetPixel(color, x0, y0 - r);
	ST7920_SetPixel(color, x0 + r, y0);
	ST7920_SetPixel(color, x0 - r, y0);
	ST7920_DrawLine(color, x0 - r, y0, x0 + r, y0);

	while (x < y) 
	{
		if (f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		
		ST7920_DrawLine(color, x0 - x, y0 + y, x0 + x, y0 + y);
		ST7920_DrawLine(color, + x, y0 - y, x0 - x, y0 - y);
		
		ST7920_DrawLine(color, + y, y0 + x, x0 - y, y0 + x);
		ST7920_DrawLine(color, + y, y0 - x, x0 - y, y0 - x);
	}
}

/*******************************************************************
 * @name       :ST7920_ClearBuffer
 * @date       :2024-01-03
 * @function   :Clear buffer
 * @parameters :None
 * @retvalue   :None
********************************************************************/ 
void ST7920_ClearBuffer(void)
{
	uint16_t bufferSize = (ST7920_WIDTH*ST7920_HEIGHT)/ST7920_DATA_SIZE;
	for (int i=0; i<bufferSize; i++)
		ST7920_Buffer[i] = 0;
}

/*******************************************************************
 * @name       :ST7920_Init
 * @date       :2024-01-03
 * @function   :Initialization of the component
 * @parameters :None
 * @retvalue   :None
********************************************************************/ 
void ST7920_Init(void)
{
	// Wait 100ms
	TIM_Wait(100);
	// Initialize SPI link
	ST7920_SpiInit();
	// Reset LOW
	ST7920_RST_LOW;
	// Wait 50ms
	TIM_Wait(50);
	// Reset HIGH
	ST7920_RST_HIGH;
	// Wait 100ms
	TIM_Wait(100);
	// 8bit mode
	ST7920_SendCmd(ST7920_CMD_BASIC);
	// Wait >100us
	TIM_WaitMicros(110);
	// 8bit mode
	ST7920_SendCmd(ST7920_CMD_BASIC);
	// Wait >37us
	TIM_WaitMicros(40);
	// D=0, C=0, B=0 (Display OFF)
	ST7920_SendCmd(ST7920_CMD_DISPLAYOFF);
	// Wait >100us
	TIM_WaitMicros(110);
	// Clear screen
	ST7920_SendCmd(ST7920_CMD_LCD_CLS);
	// Wait >10ms
	TIM_Wait(12);
	// Cursor increment right, no shift
	ST7920_SendCmd(ST7920_CMD_ADDRINC);
	// Wait 1ms
	TIM_Wait(1);
	// D=1, C=0, B=0 (Display ON)
	ST7920_SendCmd(ST7920_CMD_DISPLAYON);
	// Wait 1ms
	TIM_Wait(1);
	// Return to home
	ST7920_SendCmd(ST7920_CMD_HOME);
	// Wait 1ms
	TIM_Wait(1);
}
