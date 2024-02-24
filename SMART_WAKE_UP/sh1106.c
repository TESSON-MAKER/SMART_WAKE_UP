#include "sh1106.h"
#include "tim.h"

#define SH1106_DC_LOW (GPIOA->BSRR=GPIO_BSRR_BR0)
#define SH1106_DC_HIGH (GPIOA->BSRR=GPIO_BSRR_BS0)

#define SH1106_CS_LOW (GPIOC->BSRR=GPIO_BSRR_BR0)
#define SH1106_CS_HIGH (GPIOC->BSRR=GPIO_BSRR_BS0)

#define SH1106_RST_LOW (GPIOC->BSRR=GPIO_BSRR_BR1)
#define SH1106_RST_HIGH (GPIOC->BSRR=GPIO_BSRR_BS1)

#define SPI1_AF 0x05

enum SH1106_COMMANDS
{
	XLevelL = 0x02,
	XLevelH = 0x10,
	YLevel = 0xB0, 

	DISPLAY_OFF = 0xAE,
	DISPLAY_ON = 0xAF,
	DISPLAY_CLK = 0xD5,
	MULTIX_RAT = 0xA8,
	DISPLAY_OFFSET = 0xD3,
	DISPLAY_STARTLINE = 0x40,
	SET_DCDC = 0xAD,
	SET_SEGMENT = 0xA1,
	SET_COM_OUT_SCAN_DIR = 0xC8,
	SET_COM_HARD = 0xDA,
	SET_CONTRAST = 0x81,
	SET_PRECHARGE = 0xD9,
	SET_VCOM = 0xDB,
	START_LINE = 0x40,
	NORMAL_DISPLAY = 0xA6,
	REVERSE_DISPLAY = 0xA7,
	ENTIRE_DISPLAY_OFF = 0xA4,
	ENTIRE_DISPLAY_ON = 0xA5
};

enum SH1106_DIMENSIONS
{
	WIDTH = 128, 
	HEIGHT = 64,
	SH1106_DATA_SIZE = 8,
};

static uint8_t SH1106_Buffer[(WIDTH*HEIGHT)/SH1106_DATA_SIZE];

/*******************************************************************
 * @name       :SH1106_SpiInit
 * @date       :2024-01-03
 * @function   :SPI Initialization
 * @parameters :None
 * @retvalue   :None
********************************************************************/ 
static void SH1106_SpiInit(void)
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

	GPIOA->AFR[0] |= SPI1_AF << GPIO_AFRL_AFRL5_Pos;
	GPIOA->AFR[0] |= SPI1_AF << GPIO_AFRL_AFRL7_Pos;

	//Enable clock access to SPI1 module
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	//Set MSB first
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST;

	//Set mode to MASTER
	SPI1->CR1 |= SPI_CR1_MSTR;

	//Select software slave management by
	//setting SSM=1 and SSI=1
	SPI1->CR1 |= SPI_CR1_SSM;
	SPI1->CR1 |= SPI_CR1_SSI;

	//Set SPI mode to be MODE1 (CPHA0 CPOL0)
	SPI1->CR1 &= ~SPI_CR1_CPHA;
	SPI1->CR1 &= ~SPI_CR1_CPOL;

	//Set the frequency of SPI to 500kHz
	SPI1->CR1 |= SPI_CR1_BR_1;

	//Enable SPI module
	SPI1->CR1 |= SPI_CR1_SPE;
}

/*******************************************************************
 * @name       :SH1106_Spi_Transmit
 * @date       :2024-01-03
 * @function   :Send with spi
 * @parameters :data
 * @retvalue   :None
********************************************************************/ 
static void SH1106_Spi_Transmit(uint8_t data)
{
	//Wait until TXE is set
	while(!(SPI1->SR & (SPI_SR_TXE)));

	//Write the data to the data register
	*(volatile uint8_t*) & SPI1->DR = data;
			
	//Wait until TXE is set
	while(!(SPI1->SR & (SPI_SR_TXE)));

	
	//Wait for BUSY flag to reset
	while((SPI1->SR & (SPI_SR_BSY)));

	//Clear OVR flag
	(void)SPI1->DR;
	(void)SPI1->SR;
}

/*******************************************************************
 * @name       :SH1106_SendCmd
 * @date       :2024-01-03
 * @function   :Send command
 * @parameters :cmd
 * @retvalue   :None
********************************************************************/ 
static void SH1106_SendCmd(uint8_t cmd)
{
	SH1106_DC_LOW; //Command mode
	SH1106_CS_LOW;
	SH1106_Spi_Transmit(cmd);
	SH1106_CS_HIGH;
}

/*******************************************************************
 * @name       :SH1106_SendDoubleCmd
 * @date       :2024-01-03
 * @function   :Send double command
 * @parameters :cmd1, cmd2
 * @retvalue   :None
********************************************************************/ 
static void SH1106_SendDoubleCmd(uint8_t cmd1, uint8_t cmd2)
{
	SH1106_SendCmd(cmd1);
	SH1106_SendCmd(cmd2);
}

/*******************************************************************
 * @name       :SH1106_SendData
 * @date       :2024-01-03
 * @function   :Send data
 * @parameters :dat
 * @retvalue   :None
********************************************************************/ 
static void SH1106_SendData(uint8_t dat)
{
	SH1106_DC_HIGH; //Data mode
	SH1106_CS_LOW;
	SH1106_Spi_Transmit(dat);
	SH1106_CS_HIGH;
}

/*******************************************************************
 * @name       :SH1106_SendBuffer
 * @date       :2024-01-03
 * @function   :Send buffer
 * @parameters :None
 * @retvalue   :None
********************************************************************/ 
void SH1106_SendBuffer(void)
{
	for(int i=0; i<SH1106_DATA_SIZE; i++)  
	{  
		SH1106_SendCmd(YLevel+i);
		SH1106_SendCmd(XLevelL);
		SH1106_SendCmd(XLevelH);
		for(int n=0; n<WIDTH; n++)
		{
			SH1106_SendData(SH1106_Buffer[i*WIDTH+n]); 
		}
	}
}

/*******************************************************************
 * @name       :SH1106_Reset
 * @date       :2024-01-03
 * @function   :Reset OLED screen
 * @parameters :None
 * @retvalue   :None
********************************************************************/ 
static void SH1106_Reset(void)
{
	SH1106_RST_HIGH;
	TIM_Wait(100);
	SH1106_RST_LOW;
	TIM_Wait(100);
	SH1106_RST_HIGH;
}

/*******************************************************************
 * @name       :SH1106_SetPixel
 * @date       :2024-01-03
 * @function   :Set pixel in buffer
 * @parameters :color, x, y
 * @retvalue   :None
********************************************************************/ 
void SH1106_SetPixel(uint8_t color, int16_t x, int16_t y) 
{
	if (x >= WIDTH || y >= HEIGHT || x < 0 || y < 0) return;

	uint16_t index = (y / 8) * 128 + x;
	uint8_t bitOffset = y % SH1106_DATA_SIZE;

	if (color) SH1106_Buffer[index] |= (1 << bitOffset);
	else SH1106_Buffer[index] &= ~(1 << bitOffset);
}

/*******************************************************************
 * @name       : SH1106_DrawCharacter
 * @date       : 2024-01-03
 * @function   : Draw a character at specified position
 * @parameters : color, x, y, font, letterNumberAscii
 * @retvalue   : None
********************************************************************/
#define SH1106_MIN_ASCII_VALUE 31
#define SH1106_MAX_ASCII_VALUE 127
#define SH1106_ASCII_OFFSET 32

void SH1106_DrawCharacter(uint8_t color, int16_t x, int16_t y, const Font *font, uint8_t letterNumberAscii) 
{
	if (letterNumberAscii < SH1106_MIN_ASCII_VALUE || letterNumberAscii > SH1106_MAX_ASCII_VALUE) return;
	
	uint8_t letterNumber = letterNumberAscii - SH1106_ASCII_OFFSET;
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
				if (pixel) SH1106_SetPixel(color, a, b);
			}
		}
	}
}

/*******************************************************************
 * @name       : SH1106_DrawStr
 * @date       : 2024-01-03
 * @function   : Set pixel in buffer
 * @parameters : color, x, y, font, content
 * @retvalue   : None
********************************************************************/ 
void SH1106_DrawStr(uint8_t color, int16_t x, int16_t y, const Font *font, const char *format)
{
	while (*format && x < WIDTH && y < HEIGHT) 
	{
		uint8_t currentChar = *format;

		SH1106_DrawCharacter(color, x, y, font, currentChar);

		// Create a space between the letters
		uint8_t letterNumber = currentChar - SH1106_ASCII_OFFSET;
		uint16_t index_letterSize = letterNumber * font->datasize;
		uint8_t letterSize = font->data[index_letterSize];
		x += letterSize + (font->length / 10);

		// Go to next letter
		format++;
	}
}

/*******************************************************************
 * @name       : SH1106_FontPrint
 * @date       : 2024-01-03
 * @function   : Set pixel in buffer
 * @parameters : color, x, y, font, content
 * @retvalue   : None
********************************************************************/ 
void SH1106_FontPrint(uint8_t color, int16_t x, int16_t y, const Font *font, const char *format, ...) 
{
	va_list args;
	va_start(args, format);
	char formatted_string[50]; 
	vsprintf(formatted_string, format, args);
	va_end(args);

	SH1106_DrawStr(color, x, y, font, formatted_string); 
}

/*******************************************************************
 * @name       :SH1106_DrawLine
 * @date       :2024-01-03
 * @function   :Draw a line
 * @parameters :color, x0, y0, x1, y1
 * @retvalue   :None
********************************************************************/ 
void SH1106_DrawLine(uint8_t color, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) 
{
	int dx = (x1 >= x0) ? x1 - x0 : x0 - x1;
	int dy = (y1 >= y0) ? y1 - y0 : y0 - y1;
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	while(1)
	{
		SH1106_SetPixel(color, x0, y0);
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
 * @name       :SH1106_DrawRectangle
 * @date       :2024-01-03
 * @function   :Draw rectangle
 * @parameters :color, x, y, w, h
 * @retvalue   :None
********************************************************************/ 
void SH1106_DrawRectangle(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	//Check input parameters
	if (x >= WIDTH || y >= HEIGHT) return;

	//Check width and height
	if ((x + w) >= WIDTH) w = WIDTH - x;
	if ((y + h) >= HEIGHT) h = HEIGHT - y;

	//Draw 4 lines
	SH1106_DrawLine(color, x, y, x + w, y);         //Top line
	SH1106_DrawLine(color, x, y + h, x + w, y + h); //Bottom line
	SH1106_DrawLine(color, x, y, x, y + h);         //Left line
	SH1106_DrawLine(color, x + w, y, x + w, y + h); //Right line
}

/*******************************************************************
 * @name       :SH1106_DrawFilledRectangle
 * @date       :2024-01-03
 * @function   :Draw rectangle
 * @parameters :color, x, y, w, h
 * @retvalue   :None
********************************************************************/ 
void SH1106_DrawFilledRectangle(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	//Check input parameters
	if (x >= WIDTH || y >= HEIGHT) return;

	//Check width and height
	if ((x + w) >= WIDTH) w = WIDTH - x;
	if ((y + h) >= HEIGHT) h = HEIGHT - y;

	//Draw lines
	for (int i = 0; i <= h; i++)
		SH1106_DrawLine(color, x, y + i, x + w, y + i);
}

/*******************************************************************
 * @name       :SH1106_DrawCircle
 * @date       :2024-01-03
 * @function   :Draw circle
 * @parameters :color, x0, y0, radius
 * @retvalue   :None
********************************************************************/ 
void SH1106_DrawCircle(uint8_t color, uint8_t x0, uint8_t y0, uint8_t radius)
{
	int x = radius;
	int y = 0;
	int err = 0;

	SH1106_SetPixel(color, x0, y0 + radius);
	SH1106_SetPixel(color, x0, y0 - radius);
	SH1106_SetPixel(color, x0 + radius, y0);
	SH1106_SetPixel(color, x0 - radius, y0);

	while (x >= y)
	{
		SH1106_SetPixel(color, x0 + x, y0 + y);
		SH1106_SetPixel(color, x0 - x, y0 + y);
		SH1106_SetPixel(color, x0 + x, y0 - y);
		SH1106_SetPixel(color, x0 - x, y0 - y);
		SH1106_SetPixel(color, x0 + y, y0 + x);
		SH1106_SetPixel(color, x0 - y, y0 + x);
		SH1106_SetPixel(color, x0 + y, y0 - x);
		SH1106_SetPixel(color, x0 - y, y0 - x);

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
 * @name       :SH1106_DrawFilledCircle
 * @date       :2024-01-03
 * @function   :Draw filled circle
 * @parameters :color, x0, y0, radius
 * @retvalue   :None
********************************************************************/ 
void SH1106_DrawFilledCircle(uint8_t color, int16_t x0, int16_t y0, int16_t r)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	SH1106_SetPixel(color, x0, y0 + r);
	SH1106_SetPixel(color, x0, y0 - r);
	SH1106_SetPixel(color, x0 + r, y0);
	SH1106_SetPixel(color, x0 - r, y0);
	SH1106_DrawLine(color, x0 - r, y0, x0 + r, y0);

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
		
		SH1106_DrawLine(color, x0 - x, y0 + y, x0 + x, y0 + y);
		SH1106_DrawLine(color, + x, y0 - y, x0 - x, y0 - y);
		
		SH1106_DrawLine(color, + y, y0 + x, x0 - y, y0 + x);
		SH1106_DrawLine(color, + y, y0 - x, x0 - y, y0 - x);
	}
}

/*******************************************************************
 * @name       :SH1106_ClearBuffer
 * @date       :2024-01-03
 * @function   :Clear buffer
 * @parameters :None
 * @retvalue   :None
********************************************************************/ 
void SH1106_ClearBuffer(void)
{
	uint16_t bufferSize = (WIDTH*HEIGHT)/SH1106_DATA_SIZE;
	for (int i=0; i<bufferSize; i++)
		SH1106_Buffer[i] = 0;
}

/*******************************************************************
 * @name       :SH1106_Init
 * @date       :2024-01-03
 * @function   :Init the screen
 * @parameters :None
 * @retvalue   :None
********************************************************************/ 
void SH1106_Init(void)
{
	SH1106_SpiInit();
	TIM_Wait(200);
	SH1106_Reset();
	SH1106_SendCmd(DISPLAY_OFF);
	SH1106_SendDoubleCmd(DISPLAY_CLK, 0x80);
	SH1106_SendDoubleCmd(MULTIX_RAT, 0x3F);
	SH1106_SendDoubleCmd(DISPLAY_OFFSET, 0x00);
	SH1106_SendCmd(DISPLAY_STARTLINE);
	SH1106_SendDoubleCmd(SET_DCDC, 0x8B);
	SH1106_SendCmd(SET_SEGMENT);
	SH1106_SendCmd(SET_COM_OUT_SCAN_DIR);
	SH1106_SendDoubleCmd(SET_COM_HARD, 0x12);
	SH1106_SendDoubleCmd(SET_CONTRAST, 0xFF);
	SH1106_SendDoubleCmd(SET_PRECHARGE, 0x1F); 
	SH1106_SendDoubleCmd(START_LINE, 0x33);	
	SH1106_SendCmd(NORMAL_DISPLAY);
	SH1106_SendCmd(ENTIRE_DISPLAY_OFF);
	TIM_Wait(10);
	SH1106_SendCmd(DISPLAY_ON);
}
