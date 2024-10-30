#include "gpio.h"

void GPIO_PinMode(GPIO_TypeDef *GPIO, uint16_t PIN, uint8_t mode) 
{
    // Check if the provided GPIO is valid and enable its clock
    if (GPIO == GPIOA) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if (GPIO == GPIOB) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if (GPIO == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    else if (GPIO == GPIOD) RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    else if (GPIO == GPIOE) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    else if (GPIO == GPIOF) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    else if (GPIO == GPIOG) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    else if (GPIO == GPIOH) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
    else if (GPIO == GPIOI) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
	else if (GPIO == GPIOK) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOKEN;
    else return; // Returns if the provided GPIO is not supported or invalid

    if (PIN > 15) return; // Returns if the provided pin number is invalid (range: 0-15)

    // Configure pin mode based on the provided mode parameter
    if (mode == INPUT) 
    {
        GPIO->MODER &= ~(3U << (2 * PIN));  // Clear mode bits for the specific pin to set it as an INPUT
    } 
    else if (mode == OUTPUT) 
    {
        GPIO->MODER &= ~(3U << (2 * PIN));  // Clear mode bits first
        GPIO->MODER |= (1U << (2 * PIN));  // Set mode bits to OUTPUT for the specific pin
    } 
    else return; // Returns if an unsupported mode is provided
}

void GPIO_DigitalWrite(GPIO_TypeDef *PORT, uint16_t PIN, uint8_t state) 
{
    if (state == 1) PORT->BSRR = 1U << PIN;
    else if (state == 0) PORT->BSRR = 1U << (PIN + 16);
}

uint8_t GPIO_DigitalRead(GPIO_TypeDef *PORT, uint16_t PIN) 
{
    if (PORT->IDR & (1U << PIN)) return 1;
    else return 0;
}

uint16_t GPIO_AnalogRead(GPIO_TypeDef *GPIO, uint16_t PIN) 
{
    // Configure the GPIO pin for analog input
    GPIO->MODER |= (3UL << (2 * PIN));  // Set mode bits to Analog

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
