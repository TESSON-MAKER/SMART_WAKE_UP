#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp01.h"
#include "tim.h"

/*******************************************************************
 * @name       :ESP01_Usart_Init
 * @function   :Initialize UART7 for ESP01
 * @parameters :None
 * @retvalue   :None
********************************************************************/
void ESP01_Usart_Init(void)
{
    // Enable clock for GPIOE (port used by UART7)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    // Enable clock for UART7
    RCC->APB1ENR |= RCC_APB1ENR_UART7EN;

    // Configure PE8 in alternate function mode (UART7 TX)
    GPIOE->MODER &= ~GPIO_MODER_MODER8;
    GPIOE->MODER |= GPIO_MODER_MODER8_1;

    // Configure PE7 in alternate function mode (UART7 RX)
    GPIOE->MODER &= ~GPIO_MODER_MODER7;
    GPIOE->MODER |= GPIO_MODER_MODER7_1;

    // Assign alternate function AF7 to PE8 (UART7 TX)
    GPIOE->AFR[1] &= ~(UART7_AF8 << GPIO_AFRH_AFRH0_Pos);
    GPIOE->AFR[1] |= (UART7_AF8 << GPIO_AFRH_AFRH0_Pos);

    // Assign alternate function AF7 to PE7 (UART7 RX)
    GPIOE->AFR[0] &= ~(UART7_AF8 << GPIO_AFRL_AFRL7_Pos);
    GPIOE->AFR[0] |= (UART7_AF8 << GPIO_AFRL_AFRL7_Pos);

    // Set the baud rate
    UART7->BRR = SystemCoreClock / ESP01_BAUDRATE;

    // Enable transmitter (TE) and receiver (RE)
    UART7->CR1 |= USART_CR1_TE;
    UART7->CR1 |= USART_CR1_RE;

    // Enable UART7 peripheral
    UART7->CR1 |= USART_CR1_UE;
}

/*******************************************************************
 * @name       :ESP01_Send
 * @function   :Send an AT command to the ESP01 module
 * @parameters :const char *command - command to send
 * @retvalue   :None
********************************************************************/
void ESP01_Send(const char *command)
{
    while (*command)
    {
        while (!(UART7->ISR & USART_ISR_TXE)); // Wait until the buffer is ready
        UART7->TDR = *command++;              // Send the character
    }
    while (!(UART7->ISR & USART_ISR_TC));     // Wait for the transmission to complete
}

/*******************************************************************
 * @name       :ESP01_Receive
 * @function   :Receive a response with a timeout
 * @parameters :char *buffer - buffer to store the received data
 *              int bufferSize - size of the buffer
 *              int timeoutMs - timeout in milliseconds
 * @retvalue   :1 if successful, 0 if timeout occurred
********************************************************************/
int ESP01_Receive(char *buffer, int bufferSize, int timeoutMs)
{
    int i = 0;
    int elapsedTime = 0;

    while (i < bufferSize - 1)
    {
        // Wait for a character or timeout
        while (!(UART7->ISR & USART_ISR_RXNE))
        {
            if (elapsedTime >= timeoutMs)
            {
                buffer[i] = '\0'; // Null-terminate the string
                return 0;         // Timeout occurred
            }
            elapsedTime++;
            TIM_Wait(1); // Wait for 1 ms
        }

        // Read and append the received character
        char c = UART7->RDR;
        buffer[i++] = c;

        // Stop if end of line is reached
        if (c == '\n') break;
    }

    buffer[i] = '\0'; // Null-terminate the string
    return 1;         // Data successfully received
}

/*******************************************************************
 * @name       :ESP01_SendAndCheck
 * @function   :Send a command and check if the expected response is received
 * @parameters :const char *command - command to send
 *              const char *expected - expected response
 *              int timeoutMs - timeout in milliseconds
 * @retvalue   :1 if the expected response is found, 0 otherwise
********************************************************************/
int ESP01_SendAndCheck(const char *command, const char *expected, int timeoutMs)
{
    for (int retries = 0; retries < MAX_RETRIES; retries++)
    {
        ESP01_Send(command); // Send the command
        char response[MAX_RESPONSE_SIZE];

        if (ESP01_Receive(response, sizeof(response), timeoutMs) && strstr(response, expected))
        {
            return 1; // Expected response received
        }
    }
    return 0; // Max retries reached or no valid response
}

/*******************************************************************
 * @name       :ESP01_InitWiFi
 * @function   :Initialize Wi-Fi connection with SSID and password
 * @parameters :const char *ssid - Wi-Fi SSID
 *              const char *password - Wi-Fi password
 * @retvalue   :1 if successful, 0 if failed
********************************************************************/
int ESP01_InitWiFi(const char *ssid, const char *password)
{
    if (!ESP01_SendAndCheck("AT+CWMODE=1\r\n", "OK", 2000))
        return 0;

    char command[MAX_RESPONSE_SIZE];
    snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);

    return ESP01_SendAndCheck(command, "OK", 5000);
}

/*******************************************************************
 * @name       :ESP01_GetTime
 * @function   :Retrieve current time via NTP
 * @parameters :char *timeBuffer - buffer to store the time
 *              int bufferSize - size of the buffer
 *              int timeoutMs - timeout in milliseconds
 * @retvalue   :1 if successful, 0 otherwise
********************************************************************/
int ESP01_GetTime(char *timeBuffer, int bufferSize, int timeoutMs)
{
    ESP01_Send("AT+CIPSNTPCFG=1,1,\"pool.ntp.org\"\r\n");

    return ESP01_SendAndCheck("AT+CIPSNTPTIME?\r\n", "+CIPSNTPTIME:", timeoutMs) &&
           ESP01_Receive(timeBuffer, bufferSize, timeoutMs);
}

/*******************************************************************
 * @name       :ESP01_GetWeather
 * @function   :Send HTTP GET request to OpenWeatherMap for weather data
 * @parameters :city - Name of the city
 *              responseBuffer - Buffer to store HTTP response
 *              bufferSize - Size of the buffer
 * @retvalue   :None
********************************************************************/
void ESP01_GetWeather(const char *city, char *responseBuffer, int bufferSize)
{
    ESP01_Send("AT+CIPSTART=\"TCP\",\"" OPENWEATHER_HOST "\",80\r\n");
    if (!ESP01_SendAndCheck("", "OK", 5000))
    {
        return;
    }

    char request[256];
    snprintf(request, sizeof(request),
             "GET /data/2.5/weather?q=%s&appid=%s&units=metric HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n",
             city, API_KEY, OPENWEATHER_HOST);

    char command[512];
    snprintf(command, sizeof(command), "AT+CIPSEND=%d\r\n", strlen(request));
    ESP01_Send(command);
    ESP01_Receive(responseBuffer, bufferSize, 5000);
    ESP01_Send("AT+CIPCLOSE\r\n");
}

/*******************************************************************
 * @name       :ESP01_ParseTemperature
 * @function   :Parse temperature from JSON response
 * @parameters :response - HTTP response containing JSON data
 * @retvalue   :Temperature as a float, or -999.0 on error
********************************************************************/
float ESP01_ParseTemperature(const char *response)
{
    const char *tempPtr = strstr(response, "\"temp\":");
    if (tempPtr != NULL)
    {
        return atof(tempPtr + 7);
    }
    return -999.0;
}

/*******************************************************************
 * @name       :ESP01_GetTemperature
 * @function   :Get temperature of a city using OpenWeatherMap API
 * @parameters :city - Name of the city
 * @retvalue   :Temperature as a float, or -999.0 on error
********************************************************************/
float ESP01_GetTemperature(const char *city)
{
    char response[512];
    memset(response, 0, sizeof(response));
    ESP01_GetWeather(city, response, sizeof(response));
    return ESP01_ParseTemperature(response);
}




