#include "bsp_track.h"

void BSP_Track_Init(void) {
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

    gpio.GPIO_Pin   = GPIO_Pin_9;
    gpio.GPIO_Mode  = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpio);

    gpio.GPIO_Pin   = GPIO_Pin_12;
    GPIO_Init(GPIOB, &gpio);

    gpio.GPIO_Pin   = GPIO_Pin_11;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin   = GPIO_Pin_10;
    GPIO_Init(GPIOC, &gpio);

    gpio.GPIO_Pin   = GPIO_Pin_12;
    GPIO_Init(GPIOC, &gpio);
}

int8_t BSP_Track_GetOffset(void) {
    uint8_t s1 = (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9)  == 1);
    uint8_t s2 = (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 1);
    uint8_t s3 = (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11) == 1);
    uint8_t s4 = (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_10) == 1);
    uint8_t s5 = (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_12) == 1);
    int8_t count = (int8_t)(s1 + s2 + s3 + s4 + s5);
    int16_t weighted_x10;

    if(count == 0) {
        return 88;
    }

    weighted_x10 = (int16_t)((-42 * s1) + (-15 * s2) + (0 * s3) + (15 * s4) + (42 * s5));
    return (int8_t)(weighted_x10 / count);
}
