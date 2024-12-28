#include <string.h>
#include <stdio.h>
#include "esp01.h"
#include "tim.h"

#define MAX_RESPONSE_SIZE 128
#define DEFAULT_TIMEOUT_MS 5000
#define MAX_RETRIES 3

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
    GPIOE->MODER &= ~GPIO_MODER_MODER8;     // Clear the configuration bits
    GPIOE->MODER |= GPIO_MODER_MODER8_1;    // Set PE8 to alternate function mode

    // Configure PE7 in alternate function mode (UART7 RX)
    GPIOE->MODER &= ~GPIO_MODER_MODER7;     // Clear the configuration bits
    GPIOE->MODER |= GPIO_MODER_MODER7_1;    // Set PE7 to alternate function mode

    // Assign alternate function AF7 to PE8 (UART7 TX)
    GPIOE->AFR[1] &= ~(UART7_AF8 << GPIO_AFRH_AFRH0_Pos); // Clear existing bits
    GPIOE->AFR[1] |= (UART7_AF8 << GPIO_AFRH_AFRH0_Pos);

    // Assign alternate function AF7 to PE7 (UART7 RX)
    GPIOE->AFR[0] &= ~(UART7_AF8 << GPIO_AFRL_AFRL7_Pos); // Clear existing bits
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
    const int tickInterval = 1; // 1ms per loop iteration

    while (i < bufferSize - 1)
    {
        // Wait for a character to be received or timeout
        while (!(UART7->ISR & USART_ISR_RXNE))
        {
            if (elapsedTime >= timeoutMs)
            {
                buffer[i] = '\0'; // Null-terminate in case of timeout
                return 0;         // Timeout occurred
            }
            elapsedTime += tickInterval;
            TIM_Wait(tickInterval); // Wait for 1ms
        }

        // Read the received character
        char c = UART7->RDR;
        buffer[i++] = c;

        // Stop at the end of the line
        if (c == '\n') break;
    }
    buffer[i] = '\0'; // Null-terminate the string
    return 1;         // Successfully received data
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
    int retries = 0;
    while (retries < MAX_RETRIES)
    {
        ESP01_Send(command); // Send the command
        char response[MAX_RESPONSE_SIZE];

        if (ESP01_Receive(response, sizeof(response), timeoutMs))
        {
            if (strstr(response, expected) != NULL)
            {
                return 1; // Expected response received
            }
        }
        retries++;
    }
    return 0; // Max retries reached or response not received
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
    // Set ESP01 to station mode and connect to Wi-Fi
    if (!ESP01_SendAndCheck("AT+CWMODE=1\r\n", "OK", 2000))
        return 0;

    // Create Wi-Fi connect command
    char command[MAX_RESPONSE_SIZE];
    snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);

    // Send the command and check the response
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
    // Configure the NTP server
    ESP01_Send("AT+CIPSNTPCFG=1,0,\"pool.ntp.org\"\r\n");

    // Request the NTP time
    if (!ESP01_SendAndCheck("AT+CIPSNTPTIME?\r\n", "+CIPSNTPTIME:", timeoutMs))
        return 0;

    // Receive the time
    return ESP01_Receive(timeBuffer, bufferSize, timeoutMs);
}

