#ifndef __BSP_MOTOR_H
#define __BSP_MOTOR_H
#include "stm32f10x.h"

void BSP_Motor_Init(void);
void BSP_Motor_Left(int16_t pwm);    // -999 ~ +999
void BSP_Motor_Right(int16_t pwm);
#endif