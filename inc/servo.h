#ifndef SERVO_H
#define SERVO_H

#include <stm32f7xx.h>

void SERVO_Init(void);
void SERVO_SetAngle(int32_t angle, int32_t angleMin, int32_t angleMax);

#endif
