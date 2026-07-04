#ifndef __BSP_ENCODER_H
#define __BSP_ENCODER_H
#include "stm32f10x.h"

void BSP_Encoder_Init(void);
void BSP_Encoder_UpdateSpeed(void);       // ? 10ms ????
float BSP_Encoder_GetSpeed_Left(void);    // cm/s
float BSP_Encoder_GetSpeed_Right(void);
float BSP_Encoder_GetDistance(void);      // cm
void BSP_Encoder_ResetDistance(void);
#endif