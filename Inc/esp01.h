#ifndef ESP01_H
#define ESP01_H

#include <stm32f7xx.h> // For STM32 hardware definitions

/* UART and ESP01 Configuration */
#define ESP01_BAUDRATE 115200
#define UART7_AF8 0x08

void ESP01_UsartInit(void);
void ESP01_SendString(const char *str);
void ESP01_ReceiveString(char *buffer, uint16_t maxLength);

#endif /* ESP01_H */
