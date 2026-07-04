#include "turn_control.h"

static float Kp_turn, Kd_turn;
static float prev_error;

void Turn_Control_Init(void) {
    Kp_turn = 6.0f;
    Kd_turn = 2.0f;
    Turn_Control_Reset();
}

void Turn_Control_Reset(void) {
    prev_error = 0.0f;
}

float Turn_Control_Compute(float target_yaw, float current_yaw, float *out_speedL, float *out_speedR) {
    float error;
    float diff;

    error = target_yaw - current_yaw;
    while(error > 180.0f) {
        error -= 360.0f;
    }
    while(error < -180.0f) {
        error += 360.0f;
    }

    diff = Kp_turn * error + Kd_turn * (error - prev_error);
    prev_error = error;

    if(diff > 30.0f) {
        diff = 30.0f;
    }
    if(diff < -30.0f) {
        diff = -30.0f;
    }

    *out_speedL = diff;
    *out_speedR = -diff;

    return error;
}
