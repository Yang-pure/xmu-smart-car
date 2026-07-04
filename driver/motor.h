
#ifndef __MOTOR_H
#define __MOTOR_H
#include <sys.h>	 
#include "stm32f10x.h"
#define BIN2   PBout(3)
#define BIN1   PBout(4)

#define AIN2   PAout(5)
#define AIN1   PAout(4)

#define STBY   PAout(12)

#define PWMA   TIM2->CCR1  //PA0
#define PWMB   TIM2->CCR2  //PA1
void PWM_Init(void);
void Motor_Init(void);
void Set_Pwm(int motor_left,int motor_right);
int PWM_Limit(int IN,int max,int min);
int myabs(int a);
#endif



