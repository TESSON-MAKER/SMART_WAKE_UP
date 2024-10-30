#ifndef URM37_H
#define URM37_H

#include <stdint.h>
#include <stm32f7xx.h>

extern volatile uint8_t URM37_Temperature[4];
extern volatile uint8_t URM37_Distance[4];

void URM37_Init(void);
float URM37_GetTemperature(void);
uint16_t URM37_GetDistance(void);
void USART2_IRQHandler(void);
void URM37_Measure(uint8_t *type);

#endif /* URM37_H */
