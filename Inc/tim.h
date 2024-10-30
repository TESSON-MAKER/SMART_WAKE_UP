#ifndef TIM_H
#define TIM_H

#include <stm32f7xx.h>

void TIM_WaitMicros(unsigned int us);
void TIM_Wait(unsigned int ms);

#endif
