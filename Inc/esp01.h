#ifndef ESP01_H
#define ESP01_H

#include <stm32f7xx.h>
#include <stdio.h>
#include <stdarg.h>

#define ESP01_BAUDRATE 115200
#define UART7_AF8 0x08

#define MAX_RESPONSE_SIZE 128
#define DEFAULT_TIMEOUT_MS 5000
#define MAX_RETRIES 3
#define API_KEY "test"
#define OPENWEATHER_HOST "test"

void ESP01_Usart_Init(void);
void ESP01_Send(const char *command);
int ESP01_Receive(char *buffer, int bufferSize, int timeoutMs);
int ESP01_SendAndCheck(const char *command, const char *expected, int timeoutMs);
int ESP01_InitWiFi(const char *ssid, const char *password);
int ESP01_GetTime(char *timeBuffer, int bufferSize, int timeoutMs);
void ESP01_GetWeather(const char *city, char *responseBuffer, int bufferSize);
float ESP01_ParseTemperature(const char *response);
float ESP01_GetTemperature(const char *city);

#endif /* ESP01_H */
