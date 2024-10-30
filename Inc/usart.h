#ifndef USART_H
#define USART_H

#include <stdio.h>
#include <stdarg.h>
#include <stm32f767xx.h>

void USART_Serial_Begin(uint32_t baud_rate);
void USART_Serial_Print(const char *format, ...);

#endif
