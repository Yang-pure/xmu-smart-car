#ifndef __BSP_BUZZER_H
#define __BSP_BUZZER_H
#include "stm32f10x.h"

void BSP_Buzzer_Init(void);
void BSP_Buzzer_On(uint16_t freq_hz);
void BSP_Buzzer_Off(void);

#endif