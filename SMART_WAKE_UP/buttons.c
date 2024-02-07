#include "buttons.h"

extern volatile uint8_t BUTTON_TopState = 0;
extern volatile uint8_t BUTTON_BottomState = 0;
extern volatile uint8_t BUTTON_RightState = 0;
extern volatile uint8_t BUTTON_LeftState = 0;
extern volatile uint8_t BUTTON_Switch = 0;

static void BUTTONS_TIM2_Init(void);

void BUTTONS_Init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Enable clock for EXTI unit

	// Top Button
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER &= ~GPIO_MODER_MODER11;
	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI11_PD; // Configure button pin (PD11) for EXTI
	EXTI->RTSR |= EXTI_RTSR_TR11; // Configure interrupt on rising edge
	EXTI->IMR |= EXTI_IMR_MR11; // Enable EXTI11 interrupt

	// Bottom Button
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	GPIOE->MODER &= ~GPIO_MODER_MODER2;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PE; // Configure button pin (PE2) for EXTI
	EXTI->RTSR |= EXTI_RTSR_TR2; // Configure rising interrupt
	EXTI->IMR |= EXTI_IMR_MR2; // Enable EXTI2 interrupt

	// Right Button
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODER4;
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PA; // Configure button pin (PA4) for EXTI
	EXTI->RTSR |= EXTI_RTSR_TR4; // Configure rising interrupt
	EXTI->IMR |= EXTI_IMR_MR4; // Enable EXTI4 interrupt

	// Left Button
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	GPIOB->MODER &= ~GPIO_MODER_MODER3;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PB; // Configure button pin (PB3) for EXTI
	EXTI->RTSR |= EXTI_RTSR_TR3; // Configure rising interrupt
	EXTI->IMR |= EXTI_IMR_MR3; // Enable EXTI3 interrupt

	// Switch
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	GPIOE->MODER &= ~GPIO_MODER_MODER0;

	NVIC_SetPriority(EXTI15_10_IRQn, 0); // Set EXTI11 interrupt priority
	NVIC_EnableIRQ(EXTI15_10_IRQn); // Enable interrupt in NVIC

	NVIC_SetPriority(EXTI2_IRQn, 1);
	NVIC_EnableIRQ(EXTI2_IRQn); // Enable interrupt in NVIC

	NVIC_SetPriority(EXTI4_IRQn, 2); // Set EXTI4 interrupt priority
	NVIC_EnableIRQ(EXTI4_IRQn); // Enable interrupt in NVIC

	NVIC_SetPriority(EXTI3_IRQn, 3); // Set EXTI3 interrupt priority
	NVIC_EnableIRQ(EXTI3_IRQn); // Enable interrupt in NVIC


	BUTTONS_TIM2_Init();
}

static void BUTTONS_TIM2_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable TIM2

	TIM2->PSC = 16000 - 1;
	TIM2->ARR = 200 - 1;

	TIM2->DIER |= TIM_DIER_UIE;
	TIM2->CR1 |= TIM_CR1_CEN; // Activate the timer

	NVIC_SetPriority(TIM2_IRQn, 4);
	NVIC_EnableIRQ(TIM2_IRQn);
}

void EXTI15_10_IRQHandler(void)
{
	if (EXTI->PR & EXTI_PR_PR11) // Check if it is EXTI11 interrupt
	{
		TIM2->CNT = 0;
		EXTI->PR |= EXTI_PR_PR11; // Clear interrupt flag
		BUTTON_TopState = 1;
	}
}

void EXTI2_IRQHandler(void)
{
	if (EXTI->PR & EXTI_PR_PR2) // Check if it is EXTI2 interrupt
	{
		TIM2->CNT = 0;
		EXTI->PR |= EXTI_PR_PR2; // Clear interrupt flag
		BUTTON_BottomState = 1;
	}
}

void EXTI4_IRQHandler(void)
{
	if (EXTI->PR & EXTI_PR_PR4) // Check if it is EXTI4 interrupt
	{
		EXTI->PR |= EXTI_PR_PR4; // Clear interrupt flag
		BUTTON_RightState = 1;
	}
}

void EXTI3_IRQHandler(void)
{
	if (EXTI->PR & EXTI_PR_PR3) // Check if it is EXTI3 interrupt
	{
		EXTI->PR |= EXTI_PR_PR3; // Clear interrupt flag
		BUTTON_LeftState = 1;
	}
}

void BUTTONS_KeyState(void)
{
	BUTTON_Switch = (GPIOE->IDR & GPIO_IDR_ID0) ? 1 : 0;
}
	

void TIM2_IRQHandler(void)
{
	if (TIM2->SR & TIM_SR_UIF) // Check if counter overflow flag is set
	{
		TIM2->SR &= ~TIM_SR_UIF; // Clear counter overflow flag

		BUTTON_TopState = (GPIOD->IDR & GPIO_IDR_ID11) ? 1 : 0;
    BUTTON_BottomState = (GPIOE->IDR & GPIO_IDR_ID2) ? 1 : 0;
	}
}
