#ifndef __BSP_OLED_H
#define __BSP_OLED_H
#include "stm32f10x.h"

#define OLED_CMD  0
#define OLED_DATA 1

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
void OLED_ShowTime(uint8_t x, uint8_t y, uint32_t time_ms);
void OLED_Refresh(void);
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t);
void OLED_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot);

#endif