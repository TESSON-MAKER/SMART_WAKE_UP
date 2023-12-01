#include "gpio.h"

void enableGPIOClock(GPIO_TypeDef *GPIO) 
{
	if (GPIO == GPIOA) 
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	} 
	else if (GPIO == GPIOB) 
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	} 
	else if (GPIO == GPIOC) 
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	} 
	else if (GPIO == GPIOD) 
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	} 
	else if (GPIO == GPIOE) 
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	} 
	else if (GPIO == GPIOF) 
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
	} 
	else if (GPIO == GPIOG) 
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
	} 
	else if (GPIO == GPIOH) 
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
	} 
	else if (GPIO == GPIOI) 
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
	} 
	else 
	{
	/* Error */
	}
}

void GPIO_PinMode(GPIO_TypeDef *GPIO, uint32_t PIN, uint8_t mode) 
{
	enableGPIOClock(GPIO);
	
	if (mode == INPUT) 
	{
		GPIO->MODER &= ~(3UL << (2 * PIN));  /* Clear the mode bits */
	} 
	else if (mode == OUTPUT) 
	{
		GPIO->MODER |= (1UL << (2 * PIN));  /* Set the mode bits to OUTPUT */
	}
}

void GPIO_DigitalWrite(GPIO_TypeDef *PORT, uint32_t PIN, uint8_t state) 
{
	if (state == 1) PORT->BSRR = 1U << PIN;
	else if (state == 0) PORT->BSRR = 1U << (PIN + 16);
}

uint8_t GPIO_DigitalRead(GPIO_TypeDef *PORT, uint32_t PIN) 
{
	if (PORT->IDR & (1U << PIN)) return 1;
	else return 0;
}

uint16_t GPIO_AnalogRead(GPIO_TypeDef *GPIO, uint32_t PIN) 
{
	// Configure the GPIO pin for analog input
	GPIO->MODER |= (3UL << (2 * PIN));  // Set the mode bits to Analog

	// Start the analog-to-digital conversion
	ADC1->SQR3 = PIN;  // Set the channel to read
	ADC1->CR2 |= ADC_CR2_ADON;  // Turn on the ADC

	// Wait for the conversion to complete
	while (!(ADC1->SR & ADC_SR_EOC));

	// Read the analog value
	uint16_t analogValue = ADC1->DR;

	// Reset the pin mode to INPUT (digital mode)
	GPIO->MODER &= ~(3UL << (2 * PIN));

	return analogValue;
}
