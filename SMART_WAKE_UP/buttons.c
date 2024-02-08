#include "buttons.h"

// Global variables to store button states
volatile uint8_t BUTTON_TopState = 0;
volatile uint8_t BUTTON_BottomState = 0;
volatile uint8_t BUTTON_RightState = 0;
volatile uint8_t BUTTON_LeftState = 0;
volatile uint8_t BUTTON_Switch = 0;

// Delay values for button press detection
static uint16_t PushDelay = 1000;
static uint16_t IncrementDelay = 100;

// Function prototypes
static void BUTTONS_InitGPIO(void);
static void BUTTONS_InitInterrupts(void);
static void BUTTONS_TIM2_Init(void);

// Initialize GPIO for buttons
static void BUTTONS_InitGPIO(void)
{
    // Top Button (PD11)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    GPIOD->MODER &= ~GPIO_MODER_MODER11;
    SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI11_PD;
    EXTI->RTSR |= EXTI_RTSR_TR11;
    EXTI->IMR |= EXTI_IMR_MR11;

    // Bottom Button (PE2)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    GPIOE->MODER &= ~GPIO_MODER_MODER2;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PE;
    EXTI->RTSR |= EXTI_RTSR_TR2;
    EXTI->IMR |= EXTI_IMR_MR2;

    // Right Button (PA4)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~GPIO_MODER_MODER4;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PA;
    EXTI->RTSR |= EXTI_RTSR_TR4;
    EXTI->IMR |= EXTI_IMR_MR4;

    // Left Button (PB3)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    GPIOB->MODER &= ~GPIO_MODER_MODER3;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PB;
    EXTI->RTSR |= EXTI_RTSR_TR3;
    EXTI->IMR |= EXTI_IMR_MR3;

    // Switch (PE0)
    GPIOE->MODER &= ~GPIO_MODER_MODER0;
}

// Initialize interrupts for buttons
static void BUTTONS_InitInterrupts(void)
{
    // Set interrupt priorities and enable them in NVIC
    NVIC_SetPriority(EXTI15_10_IRQn, 0);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI2_IRQn, 1);
    NVIC_EnableIRQ(EXTI2_IRQn);
    NVIC_SetPriority(EXTI4_IRQn, 2);
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_SetPriority(EXTI3_IRQn, 3);
    NVIC_EnableIRQ(EXTI3_IRQn);
}

// Initialize TIM2 for button repetition
static void BUTTONS_TIM2_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable TIM2

    TIM2->PSC = 16000 - 1; // Set prescaler
    TIM2->ARR = PushDelay - 1; // Set auto-reload value

    TIM2->DIER |= TIM_DIER_UIE; // Enable update interrupt
    TIM2->CR1 |= TIM_CR1_CEN; // Activate the timer

    NVIC_SetPriority(TIM2_IRQn, 4); // Set TIM2 interrupt priority
    NVIC_EnableIRQ(TIM2_IRQn); // Enable TIM2 interrupt in NVIC
}

// Initialize buttons and related peripherals
void BUTTONS_Init(void)
{
    // Enable clock for EXTI unit
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    BUTTONS_InitGPIO();
    BUTTONS_InitInterrupts();
    BUTTONS_TIM2_Init();
}

// EXTI interrupt handler for Top Button
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR11)
    {
        TIM2->CNT = 0; // Reset TIM2 counter
        EXTI->PR |= EXTI_PR_PR11; // Clear interrupt flag
        BUTTON_TopState = 1; // Set Top Button state
    }
}

// EXTI interrupt handler for Right Button
void EXTI2_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR2)
    {
        TIM2->CNT = 0; // Reset TIM2 counter
        EXTI->PR |= EXTI_PR_PR2; // Clear interrupt flag
        BUTTON_BottomState = 1; // Set Right Button state
    }
}

// EXTI interrupt handler for Bottom Button
void EXTI4_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR4)
    {
        EXTI->PR |= EXTI_PR_PR4; // Clear interrupt flag
        BUTTON_RightState = 1; // Set Bottom Button state
    }
}

// EXTI interrupt handler for Left Button
void EXTI3_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR3)
    {
        EXTI->PR |= EXTI_PR_PR3; // Clear interrupt flag
        BUTTON_LeftState = 1; // Set Left Button state
    }
}

// Read state of the Switch
void BUTTONS_KeyState(void)
{
    BUTTON_Switch = (GPIOE->IDR & GPIO_IDR_ID0) ? 1 : 0;
}

// TIM2 interrupt handler for button repetition and hold detection
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF) // Check if update interrupt flag is set
    {
        TIM2->SR &= ~TIM_SR_UIF; // Clear update interrupt flag

        if ((GPIOD->IDR & GPIO_IDR_ID11) && !(GPIOE->IDR & GPIO_IDR_ID2))
        {
            // Top button pressed
            BUTTON_TopState = 1;
            TIM2->ARR = IncrementDelay - 1; // Set auto-reload value for repetition
        }
        else if ((GPIOE->IDR & GPIO_IDR_ID2) && !(GPIOD->IDR & GPIO_IDR_ID11))
        {
            // Bottom button pressed
            BUTTON_BottomState = 1;
            TIM2->ARR = IncrementDelay - 1; // Set auto-reload value for repetition
        }
        else
        {
            // None or all buttons pressed
            TIM2->ARR = PushDelay - 1; // Set auto-reload value for repetition
            BUTTON_TopState = 0;
            BUTTON_BottomState = 0;
        }
        TIM2->CNT = 0; // Reset TIM2 counter for repetition
    }
}
