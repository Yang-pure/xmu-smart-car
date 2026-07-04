#ifndef __USART1_H
#define __USART1_H

#include "stm32f10x.h"
#include <stdio.h>

void usart1_init(uint32_t bound);
void vofa_send_vel(float v1, float v2);
void vofa_send_vel3(float v1, float v2, float v3);
void vofa_send_multi(float *data, uint8_t num);
void vofa_send_debug(float speedL, float speedR, float targetL, float targetR, float yaw);

#endif
