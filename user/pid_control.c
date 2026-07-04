#include "pid_control.h"
#include "bsp_motor.h"
#include "bsp_encoder.h"

typedef struct {
    float Kp, Ki, Kd;
    float integral, integral_max;
    float prev_error;
    int out_max;
} PID_t;

#define TARGET_RAMP_SCALE_STEP   0.0065f
#define TARGET_SPEED_LIMIT       45.0f

static PID_t pid_L, pid_R;
static float target_ramp_scale = 0.0f;

static float PID_Compute(PID_t *pid, float error) {
    float P;
    float I;
    float D;
    float out;

    P = pid->Kp * error;
    pid->integral += error;
    if(pid->integral > pid->integral_max) {
        pid->integral = pid->integral_max;
    }
    if(pid->integral < -pid->integral_max) {
        pid->integral = -pid->integral_max;
    }

    I = pid->Ki * pid->integral;
    D = pid->Kd * (error - pid->prev_error);
    pid->prev_error = error;

    out = P + I + D;
    if(out > pid->out_max) {
        out = pid->out_max;
    }
    if(out < -pid->out_max) {
        out = -pid->out_max;
    }
    return out;
}

static void PID_ResetState(PID_t *pid) {
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}

void Pid_Control_Init(void) {
    pid_L.Kp = 30.0f;
    pid_L.Ki = 1.3f;
    pid_L.Kd = 0.0f;
    pid_L.integral_max = 200.0f;
    pid_L.out_max = 999;
    PID_ResetState(&pid_L);

    pid_R.Kp = 30.0f;
    pid_R.Ki = 1.2f;
    pid_R.Kd = 0.0f;
    pid_R.integral_max = 200.0f;
    pid_R.out_max = 999;
    PID_ResetState(&pid_R);

    Pid_Control_ResetRamp();
}

void Pid_Control_ResetRamp(void) {
    target_ramp_scale = 0.0f;
}

void Pid_Control_SetSpeed(float targetL, float targetR) {
    float curL;
    float curR;
    float ramped_target_L;
    float ramped_target_R;
    int pwm_L;
    int pwm_R;

    if(targetL > TARGET_SPEED_LIMIT) {
        targetL = TARGET_SPEED_LIMIT;
    }
    if(targetL < -TARGET_SPEED_LIMIT) {
        targetL = -TARGET_SPEED_LIMIT;
    }
    if(targetR > TARGET_SPEED_LIMIT) {
        targetR = TARGET_SPEED_LIMIT;
    }
    if(targetR < -TARGET_SPEED_LIMIT) {
        targetR = -TARGET_SPEED_LIMIT;
    }

    if(target_ramp_scale < 1.0f) {
        target_ramp_scale += TARGET_RAMP_SCALE_STEP;
        if(target_ramp_scale > 1.0f) {
            target_ramp_scale = 1.0f;
        }
    }

    ramped_target_L = targetL * target_ramp_scale;
    ramped_target_R = targetR * target_ramp_scale;

    curL = BSP_Encoder_GetSpeed_Left();
    curR = BSP_Encoder_GetSpeed_Right();

    pwm_L = (int)PID_Compute(&pid_L, ramped_target_L - curL);
    pwm_R = (int)PID_Compute(&pid_R, -ramped_target_R - curR);

    BSP_Motor_Left(pwm_L);
    BSP_Motor_Right(pwm_R);
}

void Pid_Control_Stop(void) {
    BSP_Motor_Left(0);
    BSP_Motor_Right(0);
    PID_ResetState(&pid_L);
    PID_ResetState(&pid_R);
    Pid_Control_ResetRamp();
}
