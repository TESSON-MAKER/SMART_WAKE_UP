#include "buttons.h"

extern Button buttonTop = {GPIOD, GPIO_IDR_ID11, 0};
extern Button buttonBottom = {GPIOE, GPIO_IDR_ID2, 0};
extern Button buttonRight = {GPIOA, GPIO_IDR_ID4, 0};
extern Button buttonLeft = {GPIOB, GPIO_IDR_ID2, 0};
extern Button buttonReturn = {GPIOB, GPIO_IDR_ID3, 0};
extern Button buttonReset = {GPIOF, GPIO_IDR_ID12, 0};

/*******************************************************************
 * @name       :BUTTONS_Init(void)
 * @date       :2024-01-25
 * @function   :Init the buttons
 * @parameters :None
 * @retvalue   :None
********************************************************************/
void BUTTONS_Init(void)
{
	//Button Top
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER &= ~GPIO_MODER_MODER11;  
	GPIOD->PUPDR &= ~GPIO_PUPDR_PUPDR11_1;  
	GPIOD->PUPDR |= GPIO_PUPDR_PUPDR11_0;  

	//Button Bottom
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	GPIOE->MODER &= ~GPIO_MODER_MODER2;   
	GPIOE->PUPDR &= ~GPIO_PUPDR_PUPDR2_1;   
	GPIOE->PUPDR |= GPIO_PUPDR_PUPDR2_0;  

	//Button Right
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODER4;   
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR4_1;   
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR4_0;  

	//Button Left
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	GPIOB->MODER &= ~GPIO_MODER_MODER2;   
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR2_1;   
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR2_0;  

	//Button Return
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	GPIOB->MODER &= ~GPIO_MODER_MODER3;   
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR3_1;   
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR3_0;  

	//Button Reset
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
	GPIOF->MODER &= ~GPIO_MODER_MODER12;   
	GPIOF->PUPDR &= ~GPIO_PUPDR_PUPDR12_1;   
	GPIOF->PUPDR |= GPIO_PUPDR_PUPDR12_0;  

	//Switch
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	GPIOE->MODER &= ~GPIO_MODER_MODER0;   
	GPIOE->PUPDR &= ~GPIO_PUPDR_PUPDR0_1;   
	GPIOE->PUPDR |= GPIO_PUPDR_PUPDR0_0;  
}

/*******************************************************************
 * @name       :BUTTONS_Click(Button *button)
 * @date       :2024-01-25
 * @function   :Click
 * @parameters :Type of button
 * @retvalue   :0 or 1
********************************************************************/
uint8_t BUTTONS_Click(Button *button)
{
	int ButtonState = button->port->IDR & button->pin;

	if (ButtonState && !button->lastButtonState)
	{
		button->lastButtonState = ButtonState;
		return 1; 
	}
	else
	{
		button->lastButtonState = ButtonState;
		return 0;
	}
}

/*******************************************************************
 * @name       :BUTTONS_Switch(Button *button)
 * @date       :2024-01-25
 * @function   :Switch
 * @parameters :None
 * @retvalue   :0 or 1
********************************************************************/
uint8_t BUTTONS_Switch(void) 
{
    return (GPIOE->IDR & GPIO_IDR_ID0) ? 1 : 0;
}
