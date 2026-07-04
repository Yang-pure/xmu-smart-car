#ifndef __BSP_TRACK_H
#define __BSP_TRACK_H
#include "stm32f10x.h"

void BSP_Track_Init(void);
int8_t BSP_Track_GetOffset(void);   // offset in 0.1cm, no line = 88
#endif
