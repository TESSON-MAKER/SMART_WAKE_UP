#ifndef ESP01_H
#define ESP01_H

#include <stm32f7xx.h>
#include <stdio.h>
#include <stdarg.h>

#define ESP01_BAUDRATE 115200
#define UART7_AF8 0x08

// UART Initialization
void ESP01_Usart_Init(void);
void ESP01_Send(const char *command);
int ESP01_Receive(char *buffer, int bufferSize, int timeoutMs);
int ESP01_SendAndCheck(const char *command, const char *expected, int timeoutMs);
int ESP01_InitWiFi(const char *ssid, const char *password);
int ESP01_GetTime(char *timeBuffer, int bufferSize, int timeoutMs);

#endif /* ESP01_H */
