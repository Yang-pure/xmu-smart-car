#ifndef __PID_CONTROL_H
#define __PID_CONTROL_H
#include "stm32f10x.h"

void Pid_Control_Init(void);
void Pid_Control_ResetRamp(void);
void Pid_Control_SetSpeed(float target_left, float target_right);
void Pid_Control_Stop(void);
#endif
