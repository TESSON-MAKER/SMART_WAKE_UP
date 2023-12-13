#ifndef URM37_H
#define URM37_H

#include <stdint.h>
#include "stm32f767xx.h"

void URM37_Init(void);
float URM37_GetTemperature(void);
uint16_t URM37_GetDistance(void);

#endif /* URM37_H */
