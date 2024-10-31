#include "usart.h"

/*******************************************************************
 * @name       :USART_Serial_Begin
 * @date       :2024-10-31
 * @function   :Initializes USART3 with the specified baud rate for serial communication.
 * @parameters :baud_rate - The desired communication speed in bits per second.
 * @retvalue   :None
********************************************************************/
void USART_Serial_Begin(uint32_t baud_rate) 
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; // Enable the clock for GPIOD
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN; // Enable the clock for USART3

    GPIOD->MODER |= GPIO_MODER_MODER8_1; // Configure PD8 as alternate function (TX)
    GPIOD->MODER |= GPIO_MODER_MODER9_1; // Configure PD9 as alternate function (RX)

    GPIOD->AFR[1] |= USART3_AF7 << GPIO_AFRH_AFRH0_Pos; // Set PD8 to AF7 (USART3 TX)
    GPIOD->AFR[1] |= USART3_AF7 << GPIO_AFRH_AFRH1_Pos; // Set PD9 to AF7 (USART3 RX)

    USART3->BRR = SystemCoreClock / baud_rate; // Set baud rate
    USART3->CR1 = USART_CR1_TE; // Enable transmitter
    USART3->CR1 |= USART_CR1_RE; // Enable receiver
    USART3->CR1 |= USART_CR1_UE; // Enable USART3
}

/*******************************************************************
 * @name       :USART_Serial_Print
 * @date       :2024-01-03
 * @function   :Sends formatted text via USART3 for serial communication.
 * @parameters :format - Format string as in printf, followed by variables to format.
 * @retvalue   :None
********************************************************************/
void USART_Serial_Print(const char *format, ...) 
{
    char buffer[128]; // Buffer for storing the formatted string
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args); // Format the input string
    va_end(args);

    for (int i = 0; buffer[i] != '\0'; i++)
    {
        while (!(USART3->ISR & USART_ISR_TXE)); // Wait until the TX register is empty
        USART3->TDR = buffer[i]; // Transmit character
    }
}
