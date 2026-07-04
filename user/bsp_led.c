#include "bsp_led.h"

void BSP_LED_Init(void) {
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
    BSP_LED_Off();
}

void BSP_LED_On(void) {
#if LED_ACTIVE_HIGH
    GPIO_SetBits(GPIOA, GPIO_Pin_3);
#else
    GPIO_ResetBits(GPIOA, GPIO_Pin_3);
#endif
}

void BSP_LED_Off(void) {
#if LED_ACTIVE_HIGH
    GPIO_ResetBits(GPIOA, GPIO_Pin_3);
#else
    GPIO_SetBits(GPIOA, GPIO_Pin_3);
#endif
}