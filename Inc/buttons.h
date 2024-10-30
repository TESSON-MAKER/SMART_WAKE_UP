

#ifndef BUTTONS_H
#define BUTTONS_H

#include <stm32f7xx.h>

extern volatile uint8_t BUTTON_TopState; 
extern volatile uint8_t BUTTON_BottomState;
extern volatile uint8_t BUTTON_RightState;
extern volatile uint8_t BUTTON_LeftState;
extern volatile uint8_t BUTTON_Switch;

void BUTTONS_Init(void);
void EXTI15_10_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI0_IRQHandler(void);
void BUTTONS_KeyState(void);
void TIM2_IRQHandler(void);

#endif  // BUTTONS_H
