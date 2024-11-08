#ifndef DS3231_H_
#define DS3231_H_

#include <stdint.h>
#include <stm32f7xx.h>

#define DS3231_I2C1_AF 0x04
#define DS3231_I2C_ADRESS 0x68

void DS3231_Init(void);
int DS3231_BCD_DEC(unsigned char x);
int DS3231_DEC_BCD(unsigned char x);
void DS3231_Read(uint8_t memadd, uint8_t *data, uint8_t length, uint16_t timeout);
void DS3231_Write(uint8_t memadd, uint8_t *data, uint8_t length, uint16_t timeout);

#endif /* DS3231_H_ */
