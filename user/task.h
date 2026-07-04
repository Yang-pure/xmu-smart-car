#ifndef __TASK_H
#define __TASK_H

#include "stm32f10x.h"

extern uint8_t task1_active;
extern uint8_t task2_active;
extern uint8_t task3_active;
extern uint8_t task4_active;
extern float target_speed_left;
extern float target_speed_right;
extern float yaw_error_straight;
extern float target_yaw_task1;   // 新增
extern float yaw_correction;     // 新增

void Task1_Init(void);
void Task2_Init(void);
void Task3_Init(void);
void Task4_Init(void);
void Task1_Control(void);
void Task2_Control(void);
void Task3_Control(void);
void Task4_Control(void);
void Task_UpdateTimers(void);
uint8_t IsTaskRunning(void);
uint32_t Get_Oled_Time(void);

#endif
