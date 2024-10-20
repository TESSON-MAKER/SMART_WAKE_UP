#ifndef USART_H
#define USART_H

#include <stm32f7xx.h>

void USART_Serial_Begin(uint32_t baud_rate);
void USART_Serial_Print(const char *format, ...);

#endif
