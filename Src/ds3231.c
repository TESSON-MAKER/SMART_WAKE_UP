#include "ds3231.h"

/*******************************************************************
 * @name       :DS3231_Init
 * @date       :2024-01-03
 * @function   :Initialize the DS3231 RTC module
 * @parameters :None
 * @retvalue   :None
********************************************************************/
void DS3231_Init(void)
{
    // Enable clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // Enable GPIOB clock
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;  // Enable I2C1 clock
    
    // Set GPIOB pin 8 to AF mode
    GPIOB->MODER |= GPIO_MODER_MODER8_1;
    GPIOB->MODER &= ~GPIO_MODER_MODER8_0;

    // Set GPIOB pin 9 to AF mode
    GPIOB->MODER |= GPIO_MODER_MODER9_1;
    GPIOB->MODER &= ~GPIO_MODER_MODER9_0;
    
    // Configure pins as open-drain outputs
    GPIOB->OTYPER |= GPIO_OTYPER_OT8;
    GPIOB->OTYPER |= GPIO_OTYPER_OT9;

    // Configure the speed of the pins
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8;
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR9;
    
    GPIOB->AFR[1] |= DS3231_I2C_AF4 << GPIO_AFRH_AFRH0_Pos;
    GPIOB->AFR[1] |= DS3231_I2C_AF4 << GPIO_AFRH_AFRH1_Pos;

    // Disable I2C before configuration
    I2C1->CR1 &= ~I2C_CR1_PE;

    // Set timing for 100kHz I2C communication
    I2C1->TIMINGR = 0x0000C1C1;
}

/*******************************************************************
 * @name       :DS3231_BCD_DEC
 * @date       :2024-01-03
 * @function   :Convert BCD to decimal
 * @parameters :x - BCD value to convert
 * @retvalue   :Converted decimal value
********************************************************************/
int DS3231_BCD_DEC(unsigned char x)
{
    return x - 6 * (x >> 4); // Conversion from BCD to decimal
}

/*******************************************************************
 * @name       :DS3231_DEC_BCD
 * @date       :2024-01-03
 * @function   :Convert decimal to BCD
 * @parameters :x - Decimal value to convert
 * @retvalue   :Converted BCD value
********************************************************************/
int DS3231_DEC_BCD(unsigned char x)
{
    return x + 6 * (x / 10); // Conversion from decimal to BCD
}

/*******************************************************************
 * @name       :DS3231_Read
 * @date       :2024-01-03
 * @function   :Read data from DS3231
 * @parameters :memadd - Memory address in DS3231,
 *              data - Pointer to store read data,
 *              length - Number of bytes to read
 * @retvalue   :None
********************************************************************/
void DS3231_Read(uint8_t memadd, uint8_t *data, uint8_t length)
{
    I2C1->CR1 |= I2C_CR1_PE; // Initialize I2C for writing
    I2C1->CR2 = (DS3231_ADRESS << 1); // Set slave address
    I2C1->CR2 &= ~I2C_CR2_ADD10; // 7-bit addressing mode
    I2C1->CR2 |= (1 << I2C_CR2_NBYTES_Pos); // Set to 1 byte for address write
    I2C1->CR2 &= ~I2C_CR2_RD_WRN; // Write mode
    I2C1->CR2 &= ~I2C_CR2_AUTOEND; // Disable auto-end
    I2C1->CR2 |= I2C_CR2_START; // Generate start condition

    while (!(I2C1->ISR & I2C_ISR_TC)) // Wait for completion
    {
        if (I2C1->ISR & I2C_ISR_TXE) // Check TX buffer
        {
            I2C1->TXDR = memadd; // Send memory address
        }
    }

    I2C1->CR1 &= ~I2C_CR1_PE; // Restart I2C for reading
    I2C1->CR1 |= I2C_CR1_PE; // Enable I2C
    I2C1->CR2 = (DS3231_ADRESS << 1); // Set slave address
    I2C1->CR2 |= (length << I2C_CR2_NBYTES_Pos); // Set length
    I2C1->CR2 |= I2C_CR2_RD_WRN; // Set read mode
    I2C1->CR2 |= I2C_CR2_AUTOEND; // Enable auto-stop
    I2C1->CR2 |= I2C_CR2_START; // Generate start condition

    while (!(I2C1->ISR & I2C_ISR_STOPF)) // Wait for stop condition
    {
        if (I2C1->ISR & I2C_ISR_RXNE) // If RX buffer has data
        {
            *data++ = I2C1->RXDR; // Read data and increment pointer
        }
    }

    I2C1->CR1 &= ~I2C_CR1_PE; // Disable I2C
}

/*******************************************************************
 * @name       :DS3231_Write
 * @date       :2024-01-03
 * @function   :Write data to DS3231
 * @parameters :memadd - Memory address in DS3231,
 *              data - Pointer to data to write,
 *              length - Number of bytes to write
 * @retvalue   :None
********************************************************************/
void DS3231_Write(uint8_t memadd, uint8_t *data, uint8_t length)
{
    I2C1->CR1 |= I2C_CR1_PE; // Enable I2C
    I2C1->CR2 = (DS3231_ADRESS << 1); // Set slave address
    I2C1->CR2 &= ~I2C_CR2_ADD10; // 7-bit addressing mode
    I2C1->CR2 |= ((length + 1) << I2C_CR2_NBYTES_Pos); // Set number of bytes to transfer
    I2C1->CR2 &= ~I2C_CR2_RD_WRN; // Set write mode
    I2C1->CR2 |= I2C_CR2_AUTOEND; // Enable auto-stop
    I2C1->CR2 |= I2C_CR2_START; // Generate start condition

    int i = 0; // Initialize index
    while (!(I2C1->ISR & I2C_ISR_STOPF)) // Wait for stop condition
    {
        if (I2C1->ISR & I2C_ISR_TXE) // Check if the TX buffer is empty
        {
            I2C1->TXDR = (i == 0) ? memadd : data[i - 1]; // Send memory address or data sequentially
            i++; // Increment index
        }
    }

    I2C1->CR1 &= ~I2C_CR1_PE; // Disable I2C
}
