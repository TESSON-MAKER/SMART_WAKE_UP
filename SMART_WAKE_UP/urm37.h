#ifndef URM37_H
#define URM37_H

#include <stdint.h>
#include "stm32f767xx.h"

void URM37_Init(void);
void URM37_Send_Command(uint8_t command1, uint8_t command2, uint8_t command3, uint8_t command4);
void URM37_Get_Response(uint8_t* data);
float URM37_GetTemperature(void);
uint16_t URM37_GetDistance(void);

#endif /* URM37_H */
