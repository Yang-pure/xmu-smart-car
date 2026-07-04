#include "track_control.h"

static float Kp_track, Kd_track;
static int8_t prev_offset;
static uint8_t has_prev_offset;

void Track_Control_Init(void) {
    Kp_track = 0.30f;
    Kd_track = 0.12f;
    Track_Control_Reset();
}

void Track_Control_Reset(void) {
    prev_offset = 0;
    has_prev_offset = 0;
}

float Track_Control_Compute(int8_t track_offset, float base_speed, float *out_speedL, float *out_speedR) {
    float d_offset = 0.0f;
    float delta;
    float max_delta;

    if(track_offset == 88) {
        Track_Control_Reset();
        *out_speedL = base_speed;
        *out_speedR = base_speed;
        return 0.0f;
    }

    if(has_prev_offset) {
        d_offset = (float)(track_offset - prev_offset);
    }
    prev_offset = track_offset;
    has_prev_offset = 1;

    delta = Kp_track * (float)track_offset + Kd_track * d_offset;
    max_delta = base_speed * 0.60f;

    if(max_delta < 4.0f) {
        max_delta = 4.0f;
    }
    if(max_delta > 20.0f) {
        max_delta = 20.0f;
    }

    if(delta > max_delta) {
        delta = max_delta;
    }
    if(delta < -max_delta) {
        delta = -max_delta;
    }

    *out_speedL = base_speed + delta;
    *out_speedR = base_speed - delta;

    return delta;
}
