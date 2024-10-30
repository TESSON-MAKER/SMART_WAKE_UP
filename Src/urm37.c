#include "urm37.h"

#define USART2_AF 0x07
#define BAUD_RATE 9600

static volatile int indexT = 0;

volatile uint8_t URM37_Temperature[4] = {0x11, 0x00, 0x00, 0x11};
volatile uint8_t URM37_Distance[4] = {0x22, 0x00, 0x00, 0x22};

static uint8_t URM37_TempReceive[4] = {0};
static uint8_t URM37_DistReceive[4] = {0};

static  int indexR = 0;
static uint8_t dataR[4] = {0};

static uint8_t URM37_BUSY = 0;

/*******************************************************************
 * @name       :URM37_Init(void)
 * @date       :2024-01-19
 * @function   :USART Initialization
 * @parameters :None
 * @retvalue   :None
********************************************************************/ 
void URM37_Init(void)
{
	// Enable clock for GPIO port D
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

	// Initialization of pin PD5 (Tx)
	GPIOD->MODER |= GPIO_MODER_MODER5_1;
	GPIOD->MODER &= ~GPIO_MODER_MODER5_0;

	// Initialization of pin PD6 (Rx)
	GPIOD->MODER |= GPIO_MODER_MODER6_1; 
	GPIOD->MODER &= ~GPIO_MODER_MODER6_0;

	GPIOD->AFR[0] |= USART2_AF << GPIO_AFRL_AFRL5_Pos;
	GPIOD->AFR[0] |= USART2_AF << GPIO_AFRL_AFRL6_Pos;  

	// Enable clock for USART2
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	// Calculate USART2 BRR value for the desired BAUD_RATE
	USART2->BRR = SystemCoreClock / BAUD_RATE;  // Set the baud rate

	USART2->CR1 = USART_CR1_TE | USART_CR1_RE;  // Enable transmission and reception
	USART2->CR1 |= USART_CR1_UE;  // Enable USART

	NVIC_SetPriority(USART2_IRQn, 0);
	NVIC_EnableIRQ(USART2_IRQn);
	
	// Configurer les interruptions pour la r?eption et la transmission USART2
	USART2->CR1 |= USART_CR1_RXNEIE;
}

/*******************************************************************
 * @name       :URM37_Measure(void)
 * @date       :2024-01-19
 * @function   :Begin measure
 * @parameters :Temperature or distance
 * @retvalue   :None
********************************************************************/ 
void URM37_Measure(uint8_t *type)
{
	if (!URM37_BUSY) 
	{
		while (indexT < 4) 
		{
			while (!(USART2->ISR & USART_ISR_TXE));
			USART2->TDR = type[indexT];
			indexT++;
		}
		indexT = 0;
		URM37_BUSY = 1;
	}
}

/*******************************************************************
 * @name       :USART2_IRQHandler(void) 
 * @date       :2024-01-19
 * @function   :Get data about URM37 with IRQ
 * @parameters :None
 * @retvalue   :None
********************************************************************/ 
void USART2_IRQHandler(void) 
{
	if ((USART2->ISR & USART_ISR_RXNE) && (indexR < 4))
	{
		dataR[indexR] = (uint8_t)USART2->RDR; //Receive data
		indexR++;	
		
		if(indexR >= 4)
		{
			if (dataR[0] == 0x11) for(int i=0; i<4; ++i) URM37_TempReceive[i] = dataR[i];
			else if (dataR[0] == 0x22) for(int i=0; i<4; ++i) URM37_DistReceive[i] = dataR[i];
			
			URM37_BUSY = 0;
			indexR = 0;
		}
	}
}

/*******************************************************************
 * @name       :URM37_GetTemperature(void)
 * @date       :2024-01-23
 * @function   :Get temperature
 * @parameters :None
 * @retvalue   :None
********************************************************************/
float URM37_GetTemperature(void)
{
	if (URM37_TempReceive[0] != 0x11 || URM37_TempReceive[1] == 0xFF || URM37_TempReceive[2] == 0xFF)
		return 0; // Reading is not valid

	// Combine the high and low bytes to get the 12-bit temperature value
	int16_t temperature = (URM37_TempReceive[1] << 8) | URM37_TempReceive[2];

	// Convert the temperature to a float with 0.1 degC resolution
	float temperatureFloat = temperature * 0.1;

	// Check if the temperature is negative
	if (temperatureFloat > 204.7) // 204.7 is the maximum positive value in 12-bit two's complement
		temperatureFloat -= 409.6; // Convert the negative temperature to a signed value

	return temperatureFloat;
}

/*******************************************************************
 * @name       :URM37_GetDistance(void)
 * @date       :2024-01-23
 * @function   :Get temperature
 * @parameters :None
 * @retvalue   :None
********************************************************************/
uint16_t URM37_GetDistance(void)
{    
	if (URM37_TempReceive[0] != 0x22 || URM37_TempReceive[1] == 0xFF || URM37_TempReceive[2] == 0xFF)
		return 0; // Reading is not valid
	
	uint16_t distance = (uint16_t)((URM37_DistReceive[1] << 8) | URM37_DistReceive[2]);
	return distance;
}
