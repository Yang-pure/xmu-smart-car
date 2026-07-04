#ifndef __CONTROL_H
#define __CONTROL_H
#include "sys.h"
#include "motor.h"

#define Middle_angle 0

#define PI 3.14159265							//PI‘≤÷‹¬ 
void Get_Angle(u8 way);

int Position_Control(int CurrentPos,int TargetPos);
int Velocity_Control(int measure,int target);
#endif
