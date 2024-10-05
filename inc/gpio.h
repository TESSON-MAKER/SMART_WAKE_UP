#ifndef GPIO_H
#define GPIO_H

#include <stm32f7xx.h>

void GPIO_PinMode(GPIO_TypeDef *GPIO, uint16_t PIN, uint8_t mode);
void GPIO_DigitalWrite(GPIO_TypeDef *PORT, uint16_t PIN, uint8_t state);
uint8_t GPIO_DigitalRead(GPIO_TypeDef *PORT, uint16_t PIN);
uint16_t GPIO_AnalogRead(GPIO_TypeDef *GPIO, uint16_t PIN);

#define INPUT  0
#define OUTPUT 1

#endif /* GPIO_H */
