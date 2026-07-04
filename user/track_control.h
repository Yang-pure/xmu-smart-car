#ifndef __TRACK_CONTROL_H
#define __TRACK_CONTROL_H
#include "stm32f10x.h"

void  Track_Control_Init(void);
void  Track_Control_Reset(void);
float Track_Control_Compute(int8_t track_offset, float base_speed, float *out_speedL, float *out_speedR);
#endif
