#include <stm32f7xx.h>
#include "servo.h"

static const int period = 19999;

int SERVO_Map(int x, int in_min, int in_max, int out_min, int out_max);

void SERVO_Init(void) 
{
    // Enable the clock for port A
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Enable the clock for timer TIM2
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Configure pin PA0 (TIM2_CH1) in alternate function mode AF1 (TIM2)
    GPIOA->MODER &= ~GPIO_MODER_MODER0;
    GPIOA->MODER |= GPIO_MODER_MODER0_1;  // Alternate function
    GPIOA->AFR[0] |= (1 << (0 * 4));  // Alternate 1 for PA0

    // Reset the TIM2 registers
    TIM2->CR1 = 0;
    TIM2->CR2 = 0;
    TIM2->CCMR1 = 0x60;  // Configure channel 1 in PWM mode 1
    TIM2->CCER = TIM_CCER_CC1E;  // Enable channel 1
    TIM2->PSC = 15;  // Clock prescaler (may need adjustment to achieve the desired frequency)
    TIM2->ARR = period;  // Period (20 ms for a 50 Hz signal)
    TIM2->CCR1 = 1500;  // Initial comparison value (may need adjustment)

    TIM2->CR1 |= TIM_CR1_CEN;  // Enable timer TIM2
}

void SERVO_SetAngle(int32_t angle, int32_t angleMIN, int32_t angleMAX) 
{
    // Calculate pulse width limits
    const int middleMS = (int)(period * (1.5 / 20));
    const int maxMS = (int)(middleMS + period * (1.1 / 20));
    const int minMS = (int)(middleMS - period * (1.1 / 20));

    // Ensure the angle is within the specified range
    if (angle > angleMAX) angle = angleMAX;
    if (angle < angleMIN) angle = angleMIN;

    // Calculate the pulse width based on the angle
    const uint32_t pulse_width = (uint32_t)SERVO_Map(angle, -90, 90, minMS, maxMS);

    // Update the comparison value for channel 1
    TIM2->CCR1 = pulse_width;
}

int SERVO_Map(int x, int in_min, int in_max, int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
