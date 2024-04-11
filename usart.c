#include "usart.h"

void USART_Serial_Begin(uint32_t baud_rate) 
{
    // Activation de l'horloge pour USART3
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    // Configuration des broches PD8 (TX) et PD9 (RX)
    GPIOD->MODER |= GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;
    GPIOD->AFR[1] |= (7 << 0) | (7 << 4);  // AF7 pour USART3

    // Configuration de l'UART
    USART3->BRR = SystemCoreClock / baud_rate;
    USART3->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void USART_Serial_Print(const char *format, ...) 
{
	char buffer[128];  // Vous pouvez ajuster la taille du tampon selon vos besoins
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	for (int i = 0; buffer[i] != '\0'; i++) 
	{
		while (!(USART3->ISR & USART_ISR_TXE));  // Attente de la disponibilite du registre de transmission
		USART3->TDR = buffer[i];  // Transmission du caractere
	}
}

