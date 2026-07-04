#include "bsp_encoder.h"

#define ENC_RESOLUTION      13
#define ENC_MULT            4
#define GEAR_RATIO          48
#define PULSE_PER_WHEEL     (ENC_RESOLUTION * ENC_MULT * GEAR_RATIO)   // 2496
#define WHEEL_DIAMETER      6.5f
#define WHEEL_CIRCUM        (3.14159f * WHEEL_DIAMETER)               // 20.42 cm
#define PULSE_PER_CM        ((float)PULSE_PER_WHEEL / WHEEL_CIRCUM)   // 122.2

static float speed_left  = 0.0f;
static float speed_right = 0.0f;
static float dist_left   = 0.0f;
static float dist_right  = 0.0f;

void BSP_Encoder_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);

    // PA6, PA7 (TIM3) ????
    gpio.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    gpio.GPIO_Mode  = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // PB6, PB7 (TIM4) ????
    gpio.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    gpio.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &gpio);

    // TIM3 ?????
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 6;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    TIM_Cmd(TIM3, ENABLE);

    // TIM4 ?????
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 6;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);
    TIM_Cmd(TIM4, ENABLE);
}

void BSP_Encoder_UpdateSpeed(void) {
    // TIM3 ? ??(????????????)
    // TIM4 ? ??(????????????)
    int16_t pulseL_raw = (int16_t)TIM_GetCounter(TIM3);
    int16_t pulseR_raw = (int16_t)TIM_GetCounter(TIM4);
    TIM_SetCounter(TIM3, 0);
    TIM_SetCounter(TIM4, 0);

    // ????????,????,????
    pulseL_raw = -pulseL_raw;
    pulseR_raw = -pulseR_raw;

    speed_left  = (float)pulseL_raw;
    speed_right = (float)pulseR_raw;

    dist_left  += (float)pulseL_raw / PULSE_PER_CM;
    dist_right += (float)pulseR_raw / PULSE_PER_CM;
}

float BSP_Encoder_GetSpeed_Left(void)  { return speed_left; }
float BSP_Encoder_GetSpeed_Right(void) { return speed_right; }
float BSP_Encoder_GetDistance(void)    { return (dist_left - dist_right) / 2.0f; }
void BSP_Encoder_ResetDistance(void) {
    speed_left = 0.0f;
    speed_right = 0.0f;
    dist_left = 0.0f;
    dist_right = 0.0f;
    TIM_SetCounter(TIM3, 0);
    TIM_SetCounter(TIM4, 0);
}
