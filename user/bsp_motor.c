#include "bsp_motor.h"

#define LEFT_IN1_PORT   GPIOA
#define LEFT_IN1_PIN    GPIO_Pin_5
#define LEFT_IN2_PORT   GPIOA
#define LEFT_IN2_PIN    GPIO_Pin_4
#define RIGHT_IN1_PORT  GPIOB
#define RIGHT_IN1_PIN   GPIO_Pin_4
#define RIGHT_IN2_PORT  GPIOB
#define RIGHT_IN2_PIN   GPIO_Pin_3

void BSP_Motor_Init(void) {
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // ????
    gpio.GPIO_Pin   = LEFT_IN1_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LEFT_IN1_PORT, &gpio);
    gpio.GPIO_Pin   = LEFT_IN2_PIN;
    GPIO_Init(LEFT_IN2_PORT, &gpio);
    gpio.GPIO_Pin   = RIGHT_IN1_PIN;
    GPIO_Init(RIGHT_IN1_PORT, &gpio);
    gpio.GPIO_Pin   = RIGHT_IN2_PIN;
    GPIO_Init(RIGHT_IN2_PORT, &gpio);

    // ??????
    GPIO_SetBits(LEFT_IN1_PORT, LEFT_IN1_PIN);
    GPIO_SetBits(LEFT_IN2_PORT, LEFT_IN2_PIN);
    GPIO_SetBits(RIGHT_IN1_PORT, RIGHT_IN1_PIN);
    GPIO_SetBits(RIGHT_IN2_PORT, RIGHT_IN2_PIN);

    // PWM ?? PA0, PA1
    gpio.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // TIM2 ??:PWM ?? 10kHz (ARR=999)
    TIM_TimeBaseStructure.TIM_Period = 999;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // PWM ?? 1
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_Cmd(TIM2, ENABLE);
}

void BSP_Motor_Left(int16_t pwm) {
    if(pwm > 999)  pwm = 999;
    if(pwm < -999) pwm = -999;
    if(pwm > 0) {
        GPIO_SetBits(LEFT_IN1_PORT, LEFT_IN1_PIN);
        GPIO_ResetBits(LEFT_IN2_PORT, LEFT_IN2_PIN);
        TIM_SetCompare1(TIM2, pwm);
    } else if(pwm < 0) {
        GPIO_ResetBits(LEFT_IN1_PORT, LEFT_IN1_PIN);
        GPIO_SetBits(LEFT_IN2_PORT, LEFT_IN2_PIN);
        TIM_SetCompare1(TIM2, -pwm);
    } else {
        GPIO_SetBits(LEFT_IN1_PORT, LEFT_IN1_PIN);
        GPIO_SetBits(LEFT_IN2_PORT, LEFT_IN2_PIN);
        TIM_SetCompare1(TIM2, 0);
    }
}

void BSP_Motor_Right(int16_t pwm) {
    if(pwm > 999)  pwm = 999;
    if(pwm < -999) pwm = -999;

    if(pwm > 0) {
        // ??? IN1=1, IN2=0 ? ???? IN1=0, IN2=1(??????)
        GPIO_ResetBits(RIGHT_IN1_PORT, RIGHT_IN1_PIN);   // IN1 = 0
        GPIO_SetBits(RIGHT_IN2_PORT, RIGHT_IN2_PIN);     // IN2 = 1
        TIM_SetCompare2(TIM2, pwm);
    } else if(pwm < 0) {
        GPIO_SetBits(RIGHT_IN1_PORT, RIGHT_IN1_PIN);     // IN1 = 1
        GPIO_ResetBits(RIGHT_IN2_PORT, RIGHT_IN2_PIN);   // IN2 = 0
        TIM_SetCompare2(TIM2, -pwm);
    } else {
        GPIO_SetBits(RIGHT_IN1_PORT, RIGHT_IN1_PIN);
        GPIO_SetBits(RIGHT_IN2_PORT, RIGHT_IN2_PIN);
        TIM_SetCompare2(TIM2, 0);
    }
}