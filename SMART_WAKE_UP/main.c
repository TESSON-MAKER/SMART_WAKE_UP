#include "gpio.h"
#include "tim.h"
#include "usart.h"
#include "urm37.h"
#include "servo.h"
#include "st7920_fonts.h"
#include "st7920.h"

#define ANGLEMAX 90
#define ANGLEMIN -90

/*float temperature = 0;*/

int main(void) 
{
	
	
	/*URM37_Init();
	SERVO_Init();
	USART_Serial_Begin(115200);*/
	
	/*GPIO_PinMode(GPIOB, 0, OUTPUT);
	GPIO_PinMode(GPIOB, 7, OUTPUT);
	GPIO_PinMode(GPIOB, 14, OUTPUT);*/
	
	ST7920_init();
	ST7920_Clear();
	ST7920_GraphicMode(1);
	
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
    for (int i =0; i<50; i++)
		{
			ST7920_Clear_GLCD_Buffer();
			ST7920_Font_Print(i, 20, Arial28x28, "Paul !");
			ST7920_Send_GLCD_Buffer();
			TIM_Wait(10);
		}
	}
}
