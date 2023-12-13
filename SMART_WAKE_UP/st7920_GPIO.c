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
static uint8_t lcd_data[3];

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

static void st7920_spi_transmit(uint8_t *data,uint32_t size)
{
	uint32_t s=0;

	while(s<size)
	{
		for(int i=0;i<8;i++)
		{
			if((data[s]<<i)&0x80) SID_HIGH;  // SID=1  OR MOSI
			else SID_LOW;  // SID=0
			SCK_HIGH;  // SCLK =0  OR SCK
			TIM_WaitMicros4(1);
			SCK_LOW;  // SCLK=1
			TIM_WaitMicros4(1);
		}
		s++;
	}
}

static void ST7920_SendCmd(uint8_t cmd)
{
	CS_HIGH;  // PUll the CS high

	lcd_data[0]=0xF8;
	lcd_data[1]=(cmd&0xf0);
	lcd_data[2]=((cmd<<4)&0xf0);

	st7920_spi_transmit(lcd_data,3);

	CS_LOW;  // PUll the CS LOW

}

static void ST7920_SendData (uint8_t data)
{

	CS_HIGH;

	lcd_data[0]=0xFA;
	lcd_data[1]=(data&0xf0);
	lcd_data[2]=((data<<4)&0xf0);

	st7920_spi_transmit(lcd_data,3);

	CS_LOW;  // PUll the CS LOW
}