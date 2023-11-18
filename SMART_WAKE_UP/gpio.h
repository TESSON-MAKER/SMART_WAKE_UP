#ifndef GPIO_H
#define GPIO_H

#include "stm32f7xx.h"

void enableGPIOClock(GPIO_TypeDef *GPIO);
void GPIO_PinMode(GPIO_TypeDef *GPIO, uint32_t PIN, uint8_t mode);
void GPIO_DigitalWrite(GPIO_TypeDef *PORT, uint32_t PIN, uint8_t state);
uint8_t GPIO_DigitalRead(GPIO_TypeDef *PORT, uint32_t PIN);
uint16_t GPIO_AnalogRead(GPIO_TypeDef *GPIO, uint32_t PIN);

#define INPUT  0
#define OUTPUT 1

#define HIGH 1
#define LOW 0

#endif /* GPIO_H */
