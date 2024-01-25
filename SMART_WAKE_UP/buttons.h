// buttons.h

#ifndef BUTTONS_H
#define BUTTONS_H

#include "stm32f7xx.h" 

typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
	int lastButtonState;
} Button;

extern Button buttonTop;
extern Button buttonBottom;
extern Button buttonRight;
extern Button buttonLeft;
extern Button buttonReturn;
extern Button buttonReset;

void BUTTONS_Init(void);
uint8_t BUTTONS_Click(Button *button);
uint8_t BUTTONS_Switch(void);

#endif  // BUTTONS_H
