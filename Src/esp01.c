#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "esp01.h"

/*******************************************************************
 * @name       :ESP01_Usart_Init
 * @function   :Initialize UART7 for ESP01
 * @parameters :None
 * @retvalue   :None
********************************************************************/
void ESP01_UsartInit(void)
{
    // Enable clock for GPIOE (port used by UART7)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    // Enable clock for UART7
    RCC->APB1ENR |= RCC_APB1ENR_UART7EN;

    // Configure PE8 in alternate function mode (UART7 TX)
    GPIOE->MODER &= ~GPIO_MODER_MODER8;     // Clear the configuration bits
    GPIOE->MODER |= GPIO_MODER_MODER8_1;    // Set PE8 to alternate function mode

    // Configure PE7 in alternate function mode (UART7 RX)
    GPIOE->MODER &= ~GPIO_MODER_MODER7;     // Clear the configuration bits
    GPIOE->MODER |= GPIO_MODER_MODER7_1;    // Set PE7 to alternate function mode

    // Assign alternate function AF7 to PE8 (UART7 TX)
    GPIOE->AFR[1] |= (UART7_AF8 << GPIO_AFRH_AFRH0_Pos);

    // Assign alternate function AF7 to PE7 (UART7 RX)
    GPIOE->AFR[0] |= (UART7_AF8 << GPIO_AFRL_AFRL7_Pos);

    // Set the baud rate
    UART7->BRR = SystemCoreClock / ESP01_BAUDRATE;

    // Enable transmitter (TE) and receiver (RE)
    UART7->CR1 = USART_CR1_TE | USART_CR1_RE;

    // Enable UART7 peripheral
    UART7->CR1 |= USART_CR1_UE;
}

/*******************************************************************
 * @name       : ESP01_SendByte
 * @function   : Sends a single byte of data via UART7.
 * @parameters : uint8_t data - The byte of data to be sent.
 * @retvalue   : None
 *******************************************************************/
static void ESP01_SendByte(uint8_t data)
{
    while (!(UART7->ISR & USART_ISR_TXE)); // Wait until the transmit data register is empty
    UART7->TDR = data; // Send the byte
}

/*******************************************************************
 * @name       : ESP01_SendString
 * @function   : Sends a null-terminated string via UART7.
 * @parameters : const char *str - Pointer to the null-terminated string to be sent.
 * @retvalue   : None
 *******************************************************************/
void ESP01_SendString(const char *str)
{
    while (*str) ESP01_SendByte(*str++); // Send each character
}

/*******************************************************************
 * @name       : ESP01_ReceiveByte
 * @function   : Receives a single byte of data via UART7.
 * @parameters : None
 * @retvalue   : uint8_t - The received byte of data from UART7.
 *******************************************************************/
static uint8_t ESP01_ReceiveByte(void)
{
    while (!(UART7->ISR & USART_ISR_RXNE));  // Wait until the receive data register is not empty
    return (uint8_t)(UART7->RDR & 0xFF);    // Read and return the received byte
}

/*******************************************************************
 * @name       : ESP01_ReceiveString
 * @function   : Receives a null-terminated string via UART7.
 * @parameters : char *buffer, uint16_t maxLength
 * @retvalue   : None
 *******************************************************************/
void ESP01_ReceiveString(char *buffer, uint16_t maxLength)
{
    uint16_t i = 0;
    char receivedChar;

    while (i < maxLength - 1) // Leave space for the null terminator '\0'
    {
        receivedChar = ESP01_ReceiveByte(); // Receive a character
        if (receivedChar == '\0') break; // If the null terminator is received
        buffer[i++] = receivedChar; // Store the character in the buffer
    }
    buffer[i] = '\0'; // Add null terminator to the end of the string
}
