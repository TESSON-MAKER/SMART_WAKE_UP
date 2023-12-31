#include "gpio.h"
#include "tim.h"
#include "usart.h"
#include "urm37.h"
#include "servo.h"
#include "st7920_fonts.h"
#include "st7920.h"
#include "ds3231.h"
#include <stdbool.h>

#define ANGLEMAX 90
#define ANGLEMIN -90

static uint8_t DS3231_Second = 0;
static uint8_t DS3231_Minute = 0;
static uint8_t DS3231_Hour = 0;
static uint8_t DS3231_DayWeek = 1;
static uint8_t DS3231_DayMonth = 1;
static uint8_t DS3231_Month = 1;
static uint8_t DS3231_Year = 0;

static bool state = 0;

//static uint8_t second = 0;
float temperature = 0;
uint16_t distance = 0;

int main(void) 
{
	
	URM37_Init();
	USART_Serial_Begin(115200);
	
	GPIO_PinMode(GPIOB, 7, OUTPUT);
	
	DS3231_Init(); 
	ST7920_Init();
	ST7920_Clear();
	ST7920_GraphicMode(1);
	ST7920_Clear_GLCD_Buffer();
	ST7920_Send_GLCD_Buffer();
	
	//uint8_t dataS[7] = {DS3231_DEC_BCD(0), DS3231_DEC_BCD(03), DS3231_DEC_BCD(17), DS3231_DEC_BCD(5), DS3231_DEC_BCD(31), DS3231_DEC_BCD(12), DS3231_DEC_BCD(23)};
	//DS3231_WriteMemory(0x68, 0x00, dataS, 7);
	int i = 0;
	
	while (1) 
	{
		GPIO_DigitalWrite(GPIOB, 7, state);
		temperature = URM37_GetTemperature();
		distance = URM37_GetDistance();

		uint8_t data[7] = {0};
		DS3231_Read(0x68,0x0,data,7);
		DS3231_Second = DS3231_BCD_DEC(data[0] & 0x7F);
		DS3231_Minute = DS3231_BCD_DEC(data[1]);
		DS3231_Hour = DS3231_BCD_DEC(data[2] & 0x3F);
		DS3231_DayWeek = DS3231_BCD_DEC(data[3]);
		DS3231_DayMonth = DS3231_BCD_DEC(data[4]);
		DS3231_Month = DS3231_BCD_DEC(data[5]);
		DS3231_Year = DS3231_BCD_DEC(data[6]);
		
		USART_Serial_Print("%02d : %02d : %02d ---- %02d/%02d/20%02d\r\n", DS3231_Hour, DS3231_Minute, DS3231_Second, DS3231_DayMonth, DS3231_Month, DS3231_Year);
		
		ST7920_Clear_GLCD_Buffer();
				
		/*---------------------Affichage de l'heure--------------------------------------*/
		if (DS3231_HS) ST7920_Font_Print(1, 0, 20, Arial28x28, "E:DS3231");
		else ST7920_Font_Print(1, 7+i, 20, Arial28x28, "%02d:%02d:%02d", DS3231_Hour, DS3231_Minute, DS3231_Second);
		
		/*---------------------Affichage de la temperature et de la distance-------------*/
		ST7920_Font_Print(1, 0, 50, Arial12x12, "T : %.1f degrees", temperature);
		ST7920_Font_Print(1, 0, 2, Arial12x12, "Dist : %d", distance);
		ST7920_Send_GLCD_Buffer();
		state ^= 1;		
	}
}
