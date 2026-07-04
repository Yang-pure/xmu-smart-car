#ifndef __TURN_CONTROL_H
#define __TURN_CONTROL_H
#include "stm32f10x.h"

void Turn_Control_Init(void);
void Turn_Control_Reset(void);
float Turn_Control_Compute(float target_yaw, float current_yaw, float *out_speedL, float *out_speedR);
#endif
