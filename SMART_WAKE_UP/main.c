//#include "gpio.h"
#include "tim.h"
#include "usart.h"
//#include "urm37.h"
//#include "servo.h"
#include "st7920_fonts.h"
#include "st7920.h"
#include "ds3231.h"

#define ANGLEMAX 90
#define ANGLEMIN -90

static uint8_t Second = 0;
static uint8_t Minute = 0;
static uint8_t Hour = 0;
static uint8_t DayWeek = 1;
static uint8_t DayMonth = 1;
static uint8_t Month = 1;
static uint8_t Year = 0;

//static uint8_t second = 0;
/*float temperature = 0;*/

int main(void) 
{
	
	/*URM37_Init();
	SERVO_Init();*/
	USART_Serial_Begin(115200);
	
	/*GPIO_PinMode(GPIOB, 0, OUTPUT);
	GPIO_PinMode(GPIOB, 7, OUTPUT);
	GPIO_PinMode(GPIOB, 14, OUTPUT);*/
	DS3231_Init(); 
	ST7920_init();
	ST7920_Clear();
	ST7920_GraphicMode(1);
	ST7920_Clear_GLCD_Buffer();
	//ST7920_Send_GLCD_Buffer();
	
	//uint8_t dataS[7] = {DS3231_DEC_BCD(0), DS3231_DEC_BCD(55), DS3231_DEC_BCD(21), DS3231_DEC_BCD(6), DS3231_DEC_BCD(10), DS3231_DEC_BCD(3), DS3231_DEC_BCD(1)};
	//DS3231_WriteMemory(0x68, 0x00, dataS, 7);
	
	while (1) 
	{
		/* Turn on the LEDs (Set PB0, PB7, and PB14 to 1) */
		/*GPIO_DigitalWrite(GPIOB, 0, LOW);
		GPIO_DigitalWrite(GPIOB, 7, HIGH);
		GPIO_DigitalWrite(GPIOB, 14, LOW);
		temperature = URM37_GetTemperature();
		USART_Serial_Print("Temperature: %f\n\r", temperature);*/
		
		//SERVO_SetAngle(0, ANGLEMIN, ANGLEMAX);

		/* Delay for 1000 ms */
		//TIM_Wait(200);

		/* Turn off the LEDs (Reset PB0, PB7, and PB14 to 0) */
		/*GPIO_DigitalWrite(GPIOB, 0, HIGH);
		GPIO_DigitalWrite(GPIOB, 7, LOW);
		GPIO_DigitalWrite(GPIOB, 14, HIGH);*/

		/* Delay for 1000 ms */
		/*TIM_Wait(1000);
		
		GPIO_DigitalWrite(GPIOB, 0, LOW);
		GPIO_DigitalWrite(GPIOB, 7, HIGH);
		GPIO_DigitalWrite(GPIOB, 14, LOW);
		
		TIM_Wait(1000);*/
		//DS3231_Send(0x33);
		//USART_Serial_Print("secondes : %d \n\r", second);
		uint8_t data[13];
		DS3231_Read(0x68,0x0,data,7);
		Second = DS3231_BCD_DEC(data[0] & 0x7F);
		Minute = DS3231_BCD_DEC(data[1]);
		Hour = DS3231_BCD_DEC(data[2] & 0x3F);
		DayWeek = DS3231_BCD_DEC(data[3]);
		DayMonth = DS3231_BCD_DEC(data[4]);
		Month = DS3231_BCD_DEC(data[5]);
		Year = DS3231_BCD_DEC(data[6]);
		
		USART_Serial_Print("%02d : %02d : %02d ---- %02d/%02d/20%02d\r\n", Hour, Minute, Second, DayMonth, Month, Year);
		
		ST7920_Clear_GLCD_Buffer();
				
		//DrawFilledRectangle(1, 0, 128, 64);
		ST7920_Font_Print(1, 7, 20, Arial28x28, "%02d:%02d:%02d", Hour, Minute, Second);
		ST7920_Send_GLCD_Buffer();
		//ST7920_SendString(0, 0, "test");
		//TIM_Wait(1000);
		//ST7920_Clear_GLCD_Buffer();
		//ST7920_Font_Print(0, 20, Arial28x28, "2");
		//ST7920_Send_GLCD_Buffer();
		//ST7920_Clear();
		TIM_WaitMicros(1);
	}
}
