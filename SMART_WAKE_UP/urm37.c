#include "urm37.h"

#define USART2_AF 0x07
#define BAUD_RATE 9600

// Function prototypes
static void URM37_Send_Command(uint8_t command1, uint8_t command2, uint8_t command3, uint8_t command4);
static void URM37_Get_Response(uint8_t* data);



// Initialize URM37
void URM37_Init(void)
{
	// Enable clock for GPIO port D
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

	// Configure PD5 (TX) and PD6 (RX) for USART communication
	GPIOD->MODER &= ~(GPIO_MODER_MODER5 | GPIO_MODER_MODER6);  // Reset the mode
	GPIOD->MODER |= GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1;  // Alternate mode
	GPIOD->AFR[0] |= (USART2_AF << GPIO_AFRL_AFRL5_Pos) | (USART2_AF << GPIO_AFRL_AFRL6_Pos);  // Alternate function 7 for PD5 and PD6

	// Enable clock for USART2
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	// Configure USART2 for communication
	USART2->BRR = SystemCoreClock / BAUD_RATE;
	USART2->CR1 = USART_CR1_TE | USART_CR1_RE;  // Enable transmission and reception
	USART2->CR1 |= USART_CR1_UE;  // Enable USART
}

// Send a command to URM37
static void URM37_Send_Command(uint8_t command1, uint8_t command2, uint8_t command3, uint8_t command4) 
{
	uint8_t commands[] = {command1, command2, command3, command4};

	for (int i = 0; i < 4; ++i) 
	{
		// Wait for the USART control register to be ready to send data
		while (!(USART2->ISR & USART_ISR_TXE));
		// Send the corresponding command
		USART2->TDR = (uint8_t)commands[i];
	}
}

// Receive response from URM37
static void URM37_Get_Response(uint8_t* data)
{
	for (int i = 0; i < 4; ++i) 
	{
		// Wait for data to be available in the receive buffer
		while (!(USART2->ISR & USART_ISR_RXNE));
		// Read the response byte 
		data[i] = (uint8_t)USART2->RDR;
	}
}

// Get temperature from URM37
float URM37_GetTemperature(void)
{
	uint8_t response[4];
	URM37_Send_Command(0x11, 0x00, 0x00, 0x11);
	URM37_Get_Response(response);

	// Check if the reading is valid
	if (response[0] != 0x11 || response[1] == 0xFF || response[2] == 0xFF)
		return 0; // Reading is not valid

	// Combine the high and low bytes to get the 12-bit temperature value
	int16_t temperature = (response[1] << 8) | response[2];

	// Convert the temperature to a float with 0.1 degC resolution
	float temperatureFloat = temperature * 0.1;

	// Check if the temperature is negative
	if (temperatureFloat > 204.7) // 204.7 is the maximum positive value in 12-bit two's complement
		temperatureFloat -= 409.6; // Convert the negative temperature to a signed value

	return temperatureFloat;
}

// Get distance from URM37
uint16_t URM37_GetDistance(void)
{
	uint8_t response[3];
	URM37_Send_Command(0x22, 0x00, 0x00, 0x22);
	URM37_Get_Response(response);

	// Check if the reading is valid and return distance if valid, otherwise return 0
	return (response[0] == 0x22 && response[1] != 0xFF && response[2] != 0xFF) ?
		((uint16_t)((response[1] << 8) | response[2])) : 0;
}