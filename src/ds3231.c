#include "../inc/ds3231.h"

/*******************************************************************
 * @name       :DS3231_Init
 * @date       :2024-01-03
 * @function   :Initializes the DS3231 by configuring I2C and GPIO
 * @parameters :None
 * @retvalue   :None
********************************************************************/
void DS3231_Init(void) 
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	GPIOB->MODER |= GPIO_MODER_MODER8_1;
	GPIOB->MODER &= ~GPIO_MODER_MODER8_0;

	GPIOB->MODER |= GPIO_MODER_MODER9_1;
	GPIOB->MODER &= ~GPIO_MODER_MODER9_0;

	GPIOB->OTYPER |= GPIO_OTYPER_OT8;
	GPIOB->OTYPER |= GPIO_OTYPER_OT9;

	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8;
	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR9;

	GPIOB->AFR[1] |= (AF4<<0)|(AF4<<4);

	I2C1->CR1 &= ~I2C_CR1_PE;
	I2C1->TIMINGR = 0x0000C1C1;
}

/*******************************************************************
 * @name       :DS3231_BCD_DEC
 * @date       :2024-01-03
 * @function   :Converts BCD (Binary Coded Decimal) to Decimal
 * @parameters :x (unsigned char) - BCD value to be converted
 * @retvalue   :Converted decimal value
********************************************************************/
int DS3231_BCD_DEC(unsigned char x) 
{
	return x - 6 * (x >> 4); // Conversion of BCD to decimal
}

/*******************************************************************
 * @name       :DS3231_DEC_BCD
 * @date       :2024-01-03
 * @function   :Converts Decimal to BCD (Binary Coded Decimal)
 * @parameters :x (unsigned char) - Decimal value to be converted
 * @retvalue   :Converted BCD value
********************************************************************/
int DS3231_DEC_BCD(unsigned char x) 
{
	return x + 6 * (x / 10); // Conversion of decimal to BCD
}

/*******************************************************************
 * @name       :DS3231_Read
 * @date       :2024-01-03
 * @function   :Reads data from the DS3231 device over I2C
 * @parameters :slav_add, memadd, data, length
 * @retvalue   :None
********************************************************************/
void DS3231_Read(uint8_t slav_add, uint8_t memadd, uint8_t *data, uint8_t length) 
{
	I2C1->CR1 |= I2C_CR1_PE; // Enable I2C
	I2C1->CR2 = (slav_add<<1); // Set slave address
	I2C1->CR2 &= ~I2C_CR2_ADD10; // 7-bit addressing
	I2C1->CR2 |= (1<<I2C_CR2_NBYTES_Pos); // Set number to transfer to 1 for write operation
	I2C1->CR2 &= ~I2C_CR2_RD_WRN; // Set the mode to write mode
	I2C1->CR2 &= ~I2C_CR2_AUTOEND; // Software end
	I2C1->CR2 |= I2C_CR2_START; // Generate start

	while(!(I2C1->ISR & I2C_ISR_TC)) // Wait until transfer is completed
	{
		if(I2C1->ISR & I2C_ISR_TXE) // Check if TX buffer is empty
		{
			I2C1->TXDR = (memadd); // Send memory address
		}
	}

	I2C1->CR1 &= ~I2C_CR1_PE; // Reset I2C
	I2C1->CR1 |= I2C_CR1_PE; // Re-enable I2C
	I2C1->CR2 = (slav_add<<1); // Set slave address
	I2C1->CR2 |= I2C_CR2_RD_WRN; // Set mode to read operation
	I2C1->CR2 |= ((length) << I2C_CR2_NBYTES_Pos); // Set length to the required length
	I2C1->CR2 |= I2C_CR2_AUTOEND; // Auto generate stop after transfer completed
	I2C1->CR2 |= I2C_CR2_START; // Generate start

	while(!(I2C1->ISR & I2C_ISR_STOPF)) // Wait until stop is generated
	{
		if(I2C1->ISR & I2C_ISR_RXNE) // If RX buffer is not empty
		{
			*data++ = I2C1->RXDR; // Read the data and increment the pointer
		}
	}

	I2C1->CR1 &= ~I2C_CR1_PE; // Disable the peripheral
}

/*******************************************************************
 * @name       :DS3231_WriteMemory
 * @date       :2024-01-03
 * @function   :Writes data to the DS3231 device over I2C
 * @parameters :slav_add, memadd, data, length
 * @retvalue   :None
********************************************************************/ 
void DS3231_WriteMemory(uint8_t slav_add, uint8_t memadd, uint8_t *data, uint8_t length) 
{
	I2C1->CR1 |= I2C_CR1_PE; // Enable I2C
	I2C1->CR2 = (slav_add << 1); // Set slave address
	I2C1->CR2 &= ~I2C_CR2_ADD10; // 7-bit addressing
	I2C1->CR2 |= (length << I2C_CR2_NBYTES_Pos); // Set number to transfer to length for write operation
	I2C1->CR2 &= ~I2C_CR2_RD_WRN; // Set the mode to write mode
	I2C1->CR2 |= I2C_CR2_AUTOEND; // Hardware end
	I2C1->CR2 |= I2C_CR2_START; // Generate start

	while (!(I2C1->ISR & I2C_ISR_STOPF)) // Wait until stop is generated
	{
		if (I2C1->ISR & I2C_ISR_TXE) // Check if TX buffer is empty
		{
			I2C1->TXDR = *data++; // Send memory address and data
		}
	}

	I2C1->CR1 &= ~I2C_CR1_PE; // Disable I2C
}
