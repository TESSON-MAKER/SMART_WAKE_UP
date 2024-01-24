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

void BUTTONS_Init(void);
uint8_t BUTTON_Click(Button *button);

#endif  // BUTTONS_H
