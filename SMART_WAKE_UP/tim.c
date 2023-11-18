#include "tim.h"

void TIM_WaitMicros(unsigned int us)
{
	SysTick->LOAD=16-1;
	SysTick->VAL=0;
	SysTick->CTRL=0x5;
	for (unsigned i=0; i<us; i++) while(!(SysTick->CTRL &0x10000));
	SysTick->CTRL=0;	
}

void TIM_Wait(unsigned int ms)
{
	SysTick->LOAD=16000-1;
	SysTick->VAL=0;
	SysTick->CTRL=0x5;
	for (unsigned int i=0; i<ms; i++) while(!(SysTick->CTRL &0x10000));
	SysTick->CTRL=0;	
}
