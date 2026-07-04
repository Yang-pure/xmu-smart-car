#include "bsp_buzzer.h"

#define TIM2_MOTOR_PERIOD   999

// 使用 PA2 (TIM2_CH3)
void BSP_Buzzer_Init(void) {
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // PA2 复用为 TIM2_CH3
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // 定时器基础配置：预分频 71 -> 计数时钟 1MHz (72MHz/72)
    TIM_TimeBaseStruct.TIM_Prescaler = 71;
    TIM_TimeBaseStruct.TIM_Period = 1000 - 1;   // 默认 1kHz
    TIM_TimeBaseStruct.TIM_ClockDivision = 0;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);

    // PWM 模式
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 500;           // 50% 占空比
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC3Init(TIM2, &TIM_OCInitStruct);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_Cmd(TIM2, ENABLE);
    BSP_Buzzer_Off();
}

void BSP_Buzzer_On(uint16_t freq_hz) {
    if (freq_hz == 0) {
        BSP_Buzzer_Off();
        return;
    }
    TIM_SetAutoreload(TIM2, TIM2_MOTOR_PERIOD);
    TIM_SetCompare3(TIM2, 0);
    TIM_PrescalerConfig(TIM2, 71, TIM_PSCReloadMode_Immediate);
    uint32_t arr = (1000000 / freq_hz) - 1;
    if (arr < 1) arr = 1;
    if (arr > 65535) arr = 65535;
    TIM_SetAutoreload(TIM2, arr);
    TIM_SetCompare3(TIM2, arr / 2);
    TIM_Cmd(TIM2, ENABLE);
}

void BSP_Buzzer_Off(void) {
    TIM_SetCompare3(TIM2, 0);
    TIM_SetAutoreload(TIM2, TIM2_MOTOR_PERIOD);
    TIM_PrescalerConfig(TIM2, 0, TIM_PSCReloadMode_Immediate);
}
