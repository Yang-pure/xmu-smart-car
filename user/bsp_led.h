#ifndef __BSP_LED_H
#define __BSP_LED_H
#include "stm32f10x.h"

// 根据实际硬件定义：1 表示高电平点亮，0 表示低电平点亮
#define LED_ACTIVE_HIGH  1

void BSP_LED_Init(void);
void BSP_LED_On(void);
void BSP_LED_Off(void);

#endif