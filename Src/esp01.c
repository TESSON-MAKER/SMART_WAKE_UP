#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "esp01.h"
#include "tim.h"

/*******************************************************************
 * @name       :ESP01_Usart_Init
 * @function   :Initialize UART7 for ESP01
 * @parameters :None
 * @retvalue   :None
********************************************************************/
void ESP01_Usart_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    RCC->APB1ENR |= RCC_APB1ENR_UART7EN;
    GPIOE->MODER = (GPIOE->MODER & ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER7)) | (GPIO_MODER_MODER8_1 | GPIO_MODER_MODER7_1);
    GPIOE->AFR[1] = (GPIOE->AFR[1] & ~GPIO_AFRH_AFRH0_Msk) | (UART7_AF8 << GPIO_AFRH_AFRH0_Pos);
    GPIOE->AFR[0] = (GPIOE->AFR[0] & ~GPIO_AFRL_AFRL7_Msk) | (UART7_AF8 << GPIO_AFRL_AFRL7_Pos);
    UART7->BRR = SystemCoreClock / ESP01_BAUDRATE;
    UART7->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

/*******************************************************************
 * @name       :ESP01_Send
 * @function   :Send an AT command to the ESP01 module
 * @parameters :const char *command - command to send
 * @retvalue   :None
********************************************************************/
void ESP01_Send(const char *cmd) {
    while (*cmd) {
        while (!(UART7->ISR & USART_ISR_TXE));
        UART7->TDR = *cmd++;
    }
    while (!(UART7->ISR & USART_ISR_TC));
}

/*******************************************************************
 * @name       :ESP01_SendFormatted
 * @function   :Format and send an AT command to the ESP01 module
 * @parameters :const char *format - format string for the command
 *              ... - arguments for the format string
 * @retvalue   :None
********************************************************************/
void ESP01_SendFormatted(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    ESP01_Send(buffer);
}

/*******************************************************************
 * @name       :ESP01_Receive
 * @function   :Receive a response with a timeout
 * @parameters :char *buffer - buffer to store the received data
 *              int bufferSize - size of the buffer
 *              int timeoutMs - timeout in milliseconds
 * @retvalue   :1 if successful, 0 if timeout occurred
********************************************************************/
int ESP01_Receive(char *buf, int size, int timeout) {
    int i = 0, elapsed = 0;
    while (i < size - 1) {
        while (!(UART7->ISR & USART_ISR_RXNE)) {
            if (elapsed++ >= timeout) return (buf[i] = '\0'), 0;
            TIM_Wait(1);
        }
        if ((buf[i++] = UART7->RDR) == '\n') break;
    }
    buf[i] = '\0';
    return 1;
}

/*******************************************************************
 * @name       :ESP01_SendAndCheck
 * @function   :Send a command and check if the expected response is received
 * @parameters :const char *command - command to send
 *              const char *expected - expected response
 *              int timeoutMs - timeout in milliseconds
 * @retvalue   :1 if the expected response is found, 0 otherwise
********************************************************************/
int ESP01_SendAndCheck(const char *cmd, const char *expected, int timeout) {
    for (int i = 0; i < MAX_RETRIES; i++) {
        ESP01_Send(cmd);
        char resp[MAX_RESPONSE_SIZE];
        if (ESP01_Receive(resp, sizeof(resp), timeout) && strstr(resp, expected)) return 1;
    }
    return 0;
}

/*******************************************************************
 * @name       :ESP01_InitWiFi
 * @function   :Initialize Wi-Fi connection with SSID and password
 * @parameters :const char *ssid - Wi-Fi SSID
 *              const char *password - Wi-Fi password
 * @retvalue   :1 if successful, 0 if failed
********************************************************************/
int ESP01_InitWiFi(const char *ssid, const char *pwd) {
    return ESP01_SendAndCheck("AT+CWMODE=1\r\n", "OK", 2000) &&
           (ESP01_SendFormatted("AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd), ESP01_SendAndCheck("", "OK", 5000));
}

/*******************************************************************
 * @name       :ESP01_GetTime
 * @function   :Retrieve current time via NTP
 * @parameters :char *timeBuffer - buffer to store the time
 *              int bufferSize - size of the buffer
 *              int timeoutMs - timeout in milliseconds
 * @retvalue   :1 if successful, 0 otherwise
********************************************************************/
int ESP01_GetTime(char *buf, int size, int timeout) {
    ESP01_Send("AT+CIPSNTPCFG=1,1,\"pool.ntp.org\"\r\n");
    return ESP01_SendAndCheck("AT+CIPSNTPTIME?\r\n", "+CIPSNTPTIME:", timeout) && ESP01_Receive(buf, size, timeout);
}

/*******************************************************************
 * @name       :ESP01_GetWeather
 * @function   :Send HTTP GET request to OpenWeatherMap for weather data
 * @parameters :city - Name of the city
 *              responseBuffer - Buffer to store HTTP response
 *              bufferSize - Size of the buffer
 * @retvalue   :None
********************************************************************/
void ESP01_GetWeather(const char *city, char *buf, int size) {
    ESP01_Send("AT+CIPSTART=\"TCP\",\"" OPENWEATHER_HOST "\",80\r\n");
    if (!ESP01_SendAndCheck("", "OK", 5000)) return;
    ESP01_SendFormatted("AT+CIPSEND=%d\r\n", snprintf(NULL, 0, "GET /data/2.5/weather?q=%s&appid=%s&units=metric HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", city, API_KEY, OPENWEATHER_HOST));
    ESP01_SendFormatted("GET /data/2.5/weather?q=%s&appid=%s&units=metric HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", city, API_KEY, OPENWEATHER_HOST);
    ESP01_Receive(buf, size, 5000);
    ESP01_Send("AT+CIPCLOSE\r\n");
}

/*******************************************************************
 * @name       :ESP01_ParseTemperature
 * @function   :Parse temperature from JSON response
 * @parameters :response - HTTP response containing JSON data
 * @retvalue   :Temperature as a float, or -999.0 on error
********************************************************************/
float ESP01_ParseTemperature(const char *resp) {
    const char *ptr = strstr(resp, "\"temp\":");
    return ptr ? atof(ptr + 7) : -999.0;
}

/*******************************************************************
 * @name       :ESP01_GetTemperature
 * @function   :Get temperature of a city using OpenWeatherMap API
 * @parameters :city - Name of the city
 * @retvalue   :Temperature as a float, or -999.0 on error
********************************************************************/
float ESP01_GetTemperature(const char *city) {
    char resp[512] = {0};
    ESP01_GetWeather(city, resp, sizeof(resp));
    return ESP01_ParseTemperature(resp);
}
