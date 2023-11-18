#include "urm37.h"

void URM37_Init(void)
{
    // Enable the clock for port D (may vary depending on the used pin)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    
    // Configure PD5 (TX)
    GPIOD->MODER &= ~(GPIO_MODER_MODER5);  // Reset the mode
    GPIOD->MODER |= GPIO_MODER_MODER5_1;  // Alternate mode
    GPIOD->AFR[0] |= (7U << (5 * 4));  // Alternate function 7 for PD5

    // Configure PD6 (RX)
    GPIOD->MODER &= ~(GPIO_MODER_MODER6);  // Reset the mode
    GPIOD->MODER |= GPIO_MODER_MODER6_1;  // Alternate mode
    GPIOD->AFR[0] |= (7U << (6 * 4));  // Alternate function 7 for PD6
    
    // Enable the clock for USART2
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // Configure the USART2 registers (9600 baud, 8 data bits, no parity)
    USART2->BRR = SystemCoreClock/9600;  // 9600 baud from a 16 MHz clock
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE;  // Enable transmission and reception
    USART2->CR1 |= USART_CR1_UE;  // Enable USART
}

void URM37_Send_Command(uint8_t command1, uint8_t command2, uint8_t command3, uint8_t command4) 
{
    // Wait for the USART control register to be ready to send data
    while (!(USART2->ISR & USART_ISR_TXE));

    // Send the commands
    USART2->TDR = (uint8_t)command1;
    while (!(USART2->ISR & USART_ISR_TXE)); 
    USART2->TDR = (uint8_t)command2;
    while (!(USART2->ISR & USART_ISR_TXE)); 
    USART2->TDR = (uint8_t)command3;
    while (!(USART2->ISR & USART_ISR_TXE)); 
    USART2->TDR = (uint8_t)command4;
    while (!(USART2->ISR & USART_ISR_TXE)); 
}

void URM37_Get_Response(uint8_t* data)
{
    // Wait for data to be available in the receive buffer
    while (!(USART2->ISR & USART_ISR_RXNE));
    // Read the first response byte 
    data[0] = (uint8_t)USART2->RDR;

    // Wait for the second response byte to be available (Low)
    while (!(USART2->ISR & USART_ISR_RXNE));
    // Read the second response byte
    data[1] = (uint8_t)USART2->RDR;

    // Wait for the third response byte to be available
    while (!(USART2->ISR & USART_ISR_RXNE));
    // Read the third response byte
    data[2] = (uint8_t)USART2->RDR;

    // Wait for the fourth response byte to be available
    while (!(USART2->ISR & USART_ISR_RXNE));
    // Read the fourth response byte (SUM)
    data[3] = (uint8_t)USART2->RDR;
}

float URM37_GetTemperature(void)
{
    uint8_t response[4];
    URM37_Send_Command(0x11, 0x00, 0x00, 0x11);
    URM37_Get_Response(response);

    // Check if the reading is valid
    if (response[0] == 0x11 && response[1] != 0xFF && response[2] != 0xFF) 
    {
        // Combine the high and low bytes to get the 12-bit temperature value
        int16_t temperature = ((response[1] << 8) | response[2]);

        // Check if the temperature is negative
        if ((response[1] & 0xF0) == 0xF0) 
        {
            // Convert the negative temperature to a signed value
            temperature = -((~temperature + 1) & 0xFFF); // Two's complement
        }

        // Convert the temperature to a float with 0.1 degC resolution
        float temperatureFloat = temperature * 0.1;

        return temperatureFloat;
    } 
    else 
    {
        return 0; // Reading is not valid
    }
}

uint16_t URM37_GetDistance(void)
{
    uint8_t response[3];
    URM37_Send_Command(0x22, 0x00, 0x00, 0x22);
    URM37_Get_Response(response);

    // Check if the reading is valid
    if (response[0] == 0x22 && response[1] != 0xFF && response[2] != 0xFF) 
    {
        uint16_t distance = (uint16_t)((response[1] << 8) | response[2]);
        return distance;
    } 
    else 
    {
        return 0;
    }
}
