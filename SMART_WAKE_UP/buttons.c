#include "buttons.h"

extern Button buttonTop = {GPIOD, GPIO_IDR_ID11, 0};
extern Button buttonBottom = {GPIOE, GPIO_IDR_ID2, 0};


void BUTTONS_Init(void)
{
    // Configuration des ports GPIO pour les boutons 1 et 2
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    GPIOD->MODER &= ~GPIO_MODER_MODER11;  
    GPIOD->PUPDR &= ~GPIO_PUPDR_PUPDR11_1;  
    GPIOD->PUPDR |= GPIO_PUPDR_PUPDR11_0;  

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    GPIOE->MODER &= ~GPIO_MODER_MODER2;   
    GPIOE->PUPDR &= ~GPIO_PUPDR_PUPDR2_1;   
    GPIOE->PUPDR |= GPIO_PUPDR_PUPDR2_0;  
}

uint8_t BUTTON_Click(Button *button)
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
