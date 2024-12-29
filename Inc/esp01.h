#ifndef ESP01_H
#define ESP01_H

#include <stm32f7xx.h> // For STM32 hardware definitions

/* UART and ESP01 Configuration */
#define ESP01_BAUDRATE 115200
#define UART7_AF8 0x08

/* Constants for HTTP and API requests */
#define MAX_RESPONSE_SIZE 512
#define DEFAULT_TIMEOUT_MS 5000
#define MAX_RETRIES 3
#define API_KEY "test"                // Replace "test" with your actual OpenWeather API key
#define OPENWEATHER_HOST "test"       // Replace "test" with the actual OpenWeather host

/* Function Declarations */
void ESP01_Usart_Init(void);                                       // Initialize UART for ESP01
void ESP01_Send(const char *command);                         // Send a command to ESP01
int ESP01_Receive(char *buffer, int bufferSize, int timeoutMs);    // Receive data from ESP01
int ESP01_SendAndCheck(const char *command, const char *expected, int timeoutMs); // Send command and check response
int ESP01_InitWiFi(const char *ssid, const char *password);        // Connect to Wi-Fi network
int ESP01_GetTime(char *timeBuffer, int bufferSize, int timeoutMs); // Get time via NTP
void ESP01_GetWeather(const char *city, char *responseBuffer, int bufferSize); // Fetch weather data
float ESP01_ParseTemperature(const char *response);               // Parse temperature from response
float ESP01_GetTemperature(const char *city);                     // Get temperature for a city

#endif /* ESP01_H */
