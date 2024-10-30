#ifndef DS3231_H_
#define DS3231_H_

#include <stm32f7xx.h>
#include <stdint.h>

#define AF4 0x04

void DS3231_Init(void);
int DS3231_BCD_DEC(unsigned char x);
int DS3231_DEC_BCD(unsigned char x);
void DS3231_Read(uint8_t slav_add, uint8_t memadd, uint8_t *data, uint8_t length );
void DS3231_WriteMemory(uint8_t slav_add, uint8_t memadd, uint8_t *data, uint8_t length);

#endif /* DS3231_H_ */
