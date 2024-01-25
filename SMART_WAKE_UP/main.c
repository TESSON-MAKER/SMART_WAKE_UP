#include "sh1106.h"
#include "tim.h"
#include "gpio.h"

#include "usart.h"
#include "urm37.h"
#include "servo.h"
#include "st7920_fonts.h"
//#include "st7920.h"
#include "ds3231.h"
#include "buttons.h"
#include <stdbool.h>

#define ANGLEMAX 90
#define ANGLEMIN -90

#define GPIO_PIN_11 (1 << 11)
#define GPIO_PIN_2 (1 << 2)

static uint8_t DS3231_Second = 0;
static uint8_t DS3231_Minute = 0;
static uint8_t DS3231_Hour = 0;
static uint8_t DS3231_DayWeek = 1;
static uint8_t DS3231_DayMonth = 1;
static uint8_t DS3231_Month = 1;
static uint8_t DS3231_Year = 0;
static uint8_t DS3231_Century = 0;

static bool state1 = 0;
static bool state2 = 0;
static bool state3 = 0;

//static uint8_t second = 0;
float temperature = 0;
uint16_t distance = 0;

const char *days[] = {"NA", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
const char *months[] = {"NA", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

int main(void) 
{
	SH1106_Init();
	//TIM_Wait(1700);
	URM37_Init();
	USART_Serial_Begin(115200);
	
	GPIO_PinMode(GPIOB, 7, OUTPUT);
	GPIO_PinMode(GPIOB, 14, OUTPUT);
	GPIO_PinMode(GPIOB, 0, OUTPUT);
	
	DS3231_Init(); 
	//ST7920_Init();
	//ST7920_Clear();
	//ST7920_GraphicMode(1);
	SH1106_ClearBuffer();
	SH1106_SendBuffer();
	BUTTONS_Init();
	
	/*uint8_t dataS[7] = {DS3231_DEC_BCD(0), DS3231_DEC_BCD(02), DS3231_DEC_BCD(17), DS3231_DEC_BCD(02), DS3231_DEC_BCD(23), DS3231_DEC_BCD(01), DS3231_DEC_BCD(24)};
	DS3231_WriteMemory(0x68, 0x00, dataS, 7);*/	
	
	while (1) 
	{
		
		GPIO_DigitalWrite(GPIOB, 7, state1);
		
		if(BUTTONS_Switch()) /*future partie setting*/
		{
			if(BUTTONS_Click(&buttonTop)) state2 ^= 1;	
			if(BUTTONS_Click(&buttonBottom)) state3 ^= 1;
			if(BUTTONS_Click(&buttonRight)) state2 ^= 1;	
			if(BUTTONS_Click(&buttonLeft)) state3 ^= 1;	
			if(BUTTONS_Click(&buttonReturn)) state3 ^= 1;	
			if(BUTTONS_Click(&buttonReset)) state2 ^= 1;	
		}
		
		GPIO_DigitalWrite(GPIOB, 14, state2);
		GPIO_DigitalWrite(GPIOB, 0, state3);
		
		URM37_Measure(URM37_Temperature);
		temperature = URM37_GetTemperature();

		uint8_t data[7] = {0};
		DS3231_Read(0x68,0x0,data,7);
		DS3231_Second = DS3231_BCD_DEC(data[0] & 0x7F);
		DS3231_Minute = DS3231_BCD_DEC(data[1]);
		DS3231_Hour = DS3231_BCD_DEC(data[2] & 0x3F);
		DS3231_DayWeek = DS3231_BCD_DEC(data[3]);
		DS3231_DayMonth = DS3231_BCD_DEC(data[4] );
		DS3231_Month = DS3231_BCD_DEC(data[5]);
		DS3231_Year = DS3231_BCD_DEC(data[6]);
		DS3231_Century = DS3231_BCD_DEC(data[5] & 0x80);
		
		USART_Serial_Print("%.1f --- %d \r\n", temperature, distance);
		
		/*ST7920_Clear_GLCD_Buffer();*/
		SH1106_ClearBuffer();
				
		/*---------------------Affichage de l'heure--------------------------------------*/
		if (DS3231_HS) SH1106_FontPrint(1, 0, 20, Arial28x28, "E:DS3231");
		else 
		{
			//SH1106_DrawFilledRectangle(!state, 0, 0, 127, 63);
			SH1106_FontPrint(1, 7, 13, Arial28x28, "%02d:%02d:%02d", DS3231_Hour, DS3231_Minute, DS3231_Second);
			SH1106_FontPrint(1, 0, 39, Arial12x12, "%s,", days[DS3231_DayWeek]);
			SH1106_FontPrint(1, 0, 52, Arial12x12, "%s %d, 2%d%02d", months[DS3231_Month], DS3231_DayMonth, DS3231_Century, DS3231_Year);
			SH1106_DrawLine(1, 0, 37, 128, 37);
			SH1106_DrawLine(1, 0, 12, 128, 12);
		}
		/*---------------------Affichage de la temperature et de la distance-------------*/
		SH1106_FontPrint(1, 0, 0, Arial12x12, "T : %.1f degrees", temperature);
		//SH1106_FontPrint(1, 0, 0, Arial12x12, "Distance : %dcm", distance);
		SH1106_SendBuffer();

		state1 ^= 1;
		
	}
}
