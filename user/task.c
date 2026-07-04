#include "task.h"
#include "bsp_motor.h"
#include "bsp_encoder.h"
#include "bsp_track.h"
#include "pid_control.h"
#include "track_control.h"
#include "bsp_led.h"
#include "bsp_buzzer.h"
#include "bsp_oled.h"
#include "delay.h"

uint8_t task1_active = 0;
uint8_t task2_active = 0;
uint8_t task3_active = 0;
uint8_t task4_active = 0;

float target_speed_left  = 0.0f;
float target_speed_right = 0.0f;
float target_yaw_task1   = 0.0f;
float yaw_correction     = 0.0f;
float yaw_error_straight = 0.0f;

static uint8_t task2_state = 0;
static float task2_start_yaw = 0.0f;

static uint8_t route_state = 0;
static uint8_t route_lap = 0;
static uint8_t route_target_laps = 0;
static float route_start_yaw = 0.0f;
static uint16_t route_line_lost_ticks = 0;
static int8_t route_last_offset = 0;
static uint8_t route_has_line = 0;

static uint16_t signal_timer_ms = 0;
static uint32_t task_runtime_ms = 0;
static uint32_t oled_display_time_ms = 0;

#define NORMAL_SPEED          30.0f
#define TRACK_SPEED           12.0f
#define TRACK_SAFE_SPEED      10.0f
#define TRACK_SEARCH_SPEED    8.0f
#define TASK2_TRACK_GAIN      1.20f
#define TASK2_TRACK_BIAS      (-3.0f)

#define TASK3_STRAIGHT_SPEED  30.0f
#define TASK3_ARC_SPEED       13.0f
#define TASK4_STRAIGHT_SPEED  34.0f
#define TASK4_ARC_SPEED       15.0f

#define KP_YAW                0.25f
#define MAX_CORR              4.0f
#define STRAIGHT_START_BIAS   0.0f
#define STRAIGHT_START_DIST   25.0f
#define STRAIGHT_MIN_SPEED    16.0f
#define STRAIGHT_SLOW_DIST    25.0f
#define STRAIGHT_YAW_ABORT    8.0f
#define STRAIGHT_YAW_OFFSET   0.0f

#define ROUTE_AC_CM           100.0f
#define ROUTE_CB_END_CM       225.6f
#define ROUTE_BD_END_CM       325.6f
#define ROUTE_LAP_CM          451.2f
#define ROUTE_LINE_FIND_TICKS 40
#define ROUTE_LINE_LOST_GRACE 25
#define ROUTE_LINE_LOST_STOP  120
#define TASK1_FAILSAFE_MS     7000
#define TASK2_FAILSAFE_MS     25000
#define TASK3_FAILSAFE_MS     35000
#define TASK4_FAILSAFE_MS     120000

extern float Yaw;

static float Normalize_Yaw(float yaw) {
    while (yaw > 180.0f) {
        yaw -= 360.0f;
    }
    while (yaw < -180.0f) {
        yaw += 360.0f;
    }
    return yaw;
}

static void Signal_Start(uint16_t freq, uint16_t ms) {
    BSP_LED_On();
    BSP_Buzzer_On(freq);
    signal_timer_ms = ms;
}

static void BeepBlink(uint16_t f, uint16_t t) {
    BSP_LED_On();
    BSP_Buzzer_On(f);
    delay_ms(t);
    BSP_Buzzer_Off();
    BSP_LED_Off();
}

static float AvgYaw(void) {
    float sum = 0.0f;
    int i;

    for (i = 0; i < 20; i++) {
        sum += Yaw;
        delay_ms(5);
    }
    return sum / 20.0f;
}

static void Reset_Output(void) {
    target_speed_left = 0.0f;
    target_speed_right = 0.0f;
    yaw_correction = 0.0f;
    yaw_error_straight = 0.0f;
    Pid_Control_Stop();
    Track_Control_Reset();
}

static void Reset_Task_Runtime(void) {
    task_runtime_ms = 0;
}

static uint8_t Runtime_Expired(uint32_t limit_ms) {
    return (task_runtime_ms >= limit_ms);
}

static void Stop_All_Tasks(void) {
    task1_active = 0;
    task2_active = 0;
    task3_active = 0;
    task4_active = 0;
}

static void Reset_Route_Line(void) {
    route_line_lost_ticks = 0;
    route_last_offset = 0;
    route_has_line = 0;
}

static void Reset_Route_Task(void) {
    route_state = 0;
    route_lap = 0;
    route_target_laps = 0;
    route_start_yaw = 0.0f;
    Reset_Route_Line();
}

static void Straight_With_Yaw(float base_speed) {
    float error;
    float corr;
    float start_bias;
    float dist;

    error = Yaw - target_yaw_task1;
    while (error > 180.0f) {
        error -= 360.0f;
    }
    while (error < -180.0f) {
        error += 360.0f;
    }

    corr = -KP_YAW * error;
    if (corr > MAX_CORR) {
        corr = MAX_CORR;
    }
    if (corr < -MAX_CORR) {
        corr = -MAX_CORR;
    }

    yaw_correction = corr;
    yaw_error_straight = error;

    dist = BSP_Encoder_GetDistance();
    if (dist < 0.0f) {
        dist = 0.0f;
    }
    if (dist < STRAIGHT_START_DIST) {
        start_bias = STRAIGHT_START_BIAS * (1.0f - dist / STRAIGHT_START_DIST);
    } else {
        start_bias = 0.0f;
    }

    target_speed_left = base_speed - corr + start_bias;
    target_speed_right = base_speed + corr - start_bias;
    Pid_Control_SetSpeed(target_speed_left, target_speed_right);
}

static float Straight_End_Speed(float base_speed, float dist, float stop_dist) {
    float remain = stop_dist - dist;
    float speed;

    if (remain <= 0.0f) {
        return 0.0f;
    }
    if (remain >= STRAIGHT_SLOW_DIST) {
        return base_speed;
    }

    speed = STRAIGHT_MIN_SPEED + (base_speed - STRAIGHT_MIN_SPEED) * remain / STRAIGHT_SLOW_DIST;
    if (speed < STRAIGHT_MIN_SPEED) {
        speed = STRAIGHT_MIN_SPEED;
    }
    return speed;
}

static uint8_t Straight_Yaw_Abnormal(void) {
    return (yaw_error_straight > STRAIGHT_YAW_ABORT || yaw_error_straight < -STRAIGHT_YAW_ABORT);
}

static void Track_With_Speed(int8_t offset, float base_speed) {
    float spL = base_speed;
    float spR = base_speed;

    if (offset != 88) {
        Track_Control_Compute(offset, base_speed, &spL, &spR);
    } else {
        Track_Control_Reset();
    }

    target_speed_left = spL;
    target_speed_right = spR;
    yaw_correction = 0.0f;
    yaw_error_straight = 0.0f;
    Pid_Control_SetSpeed(spL, spR);
}

static int8_t Limit_Track_Offset(float offset) {
    if (offset > 80.0f) {
        return 80;
    }
    if (offset < -80.0f) {
        return -80;
    }
    return (int8_t)offset;
}

static void Track_Task2(int8_t offset) {
    int8_t adjusted_offset = offset;

    if (offset != 88) {
        adjusted_offset = Limit_Track_Offset((float)offset * TASK2_TRACK_GAIN + TASK2_TRACK_BIAS);
    }

    Track_With_Speed(adjusted_offset, TRACK_SPEED);
}

static float Route_Straight_Speed(void) {
    if (task4_active) {
        return TASK4_STRAIGHT_SPEED;
    }
    return TASK3_STRAIGHT_SPEED;
}

static float Route_Arc_Speed(void) {
    if (task4_active) {
        return TASK4_ARC_SPEED;
    }
    return TASK3_ARC_SPEED;
}

static float Route_Lap_Start_Dist(void) {
    return ROUTE_LAP_CM * (float)route_lap;
}

void Task1_Init(void) {
    Stop_All_Tasks();
    task2_state = 0;
    Reset_Route_Task();
    Reset_Output();
    Reset_Task_Runtime();

    delay_ms(800);
    target_yaw_task1 = Normalize_Yaw(AvgYaw() + STRAIGHT_YAW_OFFSET);
    BSP_Encoder_ResetDistance();
    Pid_Control_ResetRamp();
    BeepBlink(2000, 50);
    BSP_Encoder_ResetDistance();
    Pid_Control_ResetRamp();

    task1_active = 1;
}

void Task1_Control(void) {
    float dist;

    if (!task1_active) {
        return;
    }

    dist = BSP_Encoder_GetDistance();
    if (dist >= 100.0f || Runtime_Expired(TASK1_FAILSAFE_MS)) {
        Reset_Output();
        task1_active = 0;
        Signal_Start(2000, 120);
        return;
    }

    Straight_With_Yaw(Straight_End_Speed(NORMAL_SPEED, dist, 100.0f));
    if (Straight_Yaw_Abnormal()) {
        Reset_Output();
        task1_active = 0;
        Signal_Start(800, 300);
    }
}

void Task2_Init(void) {
    Stop_All_Tasks();
    task2_state = 0;
    Reset_Route_Task();
    Reset_Output();
    Reset_Task_Runtime();

    delay_ms(800);
    task2_start_yaw = Normalize_Yaw(AvgYaw() + STRAIGHT_YAW_OFFSET);
    target_yaw_task1 = task2_start_yaw;
    BSP_Encoder_ResetDistance();
    Pid_Control_ResetRamp();
    BeepBlink(2000, 50);
    BSP_Encoder_ResetDistance();
    Pid_Control_ResetRamp();

    task2_active = 1;
}

void Task2_Control(void) {
    float dist;
    int8_t offset;

    if (!task2_active) {
        return;
    }

    dist = BSP_Encoder_GetDistance();
    offset = BSP_Track_GetOffset();

    if (Runtime_Expired(TASK2_FAILSAFE_MS)) {
        Reset_Output();
        task2_active = 0;
        Signal_Start(800, 300);
        return;
    }

    switch (task2_state) {
        case 0:
            if (dist >= ROUTE_AC_CM) {
                Signal_Start(2000, 60);
                Track_Control_Reset();
                task2_state = 1;
            } else {
                Straight_With_Yaw(Straight_End_Speed(NORMAL_SPEED, dist, ROUTE_AC_CM));
                if (Straight_Yaw_Abnormal()) {
                    Reset_Output();
                    task2_active = 0;
                    Signal_Start(800, 300);
                }
            }
            break;

        case 1:
            if (dist >= ROUTE_CB_END_CM) {
                Signal_Start(2000, 60);
                target_yaw_task1 = Normalize_Yaw(task2_start_yaw + 180.0f);
                task2_state = 2;
            } else {
                Track_Task2(offset);
            }
            break;

        case 2:
            if (dist >= ROUTE_BD_END_CM) {
                Signal_Start(2000, 60);
                Track_Control_Reset();
                task2_state = 3;
            } else {
                Straight_With_Yaw(Straight_End_Speed(NORMAL_SPEED, dist, ROUTE_BD_END_CM));
                if (Straight_Yaw_Abnormal()) {
                    Reset_Output();
                    task2_active = 0;
                    Signal_Start(800, 300);
                }
            }
            break;

        case 3:
            if (dist >= ROUTE_LAP_CM) {
                Reset_Output();
                task2_active = 0;
                Signal_Start(2000, 80);
            } else {
                Track_Task2(offset);
            }
            break;

        default:
            Reset_Output();
            task2_active = 0;
            break;
    }
}

static void Route_Init(uint8_t laps) {
    Stop_All_Tasks();
    task2_state = 0;
    Reset_Route_Task();
    Reset_Output();
    Reset_Task_Runtime();

    delay_ms(800);
    route_start_yaw = Normalize_Yaw(AvgYaw() + STRAIGHT_YAW_OFFSET);
    target_yaw_task1 = route_start_yaw;
    route_target_laps = laps;
    route_state = 0;
    route_lap = 0;
    Reset_Route_Line();
    BSP_Encoder_ResetDistance();
    Pid_Control_ResetRamp();
    BeepBlink(2000, 50);
    BSP_Encoder_ResetDistance();
    Pid_Control_ResetRamp();
}

void Task3_Init(void) {
    Route_Init(1);
    task3_active = 1;
}

void Task4_Init(void) {
    Route_Init(4);
    task4_active = 1;
}

static void Route_Stop(uint8_t finished_ok) {
    Reset_Output();
    Stop_All_Tasks();
    Reset_Route_Task();

    if (finished_ok) {
        Signal_Start(2000, 120);
    } else {
        Signal_Start(800, 300);
    }
}

static uint8_t Route_Arc_Control(float base_speed) {
    int8_t offset = BSP_Track_GetOffset();

    if (offset != 88) {
        route_has_line = 1;
        route_line_lost_ticks = 0;
        route_last_offset = offset;
        Track_With_Speed(offset, base_speed);
        return 1;
    }

    if (route_line_lost_ticks < 0xFFFF) {
        route_line_lost_ticks++;
    }

    if (!route_has_line && route_line_lost_ticks <= ROUTE_LINE_FIND_TICKS) {
        Straight_With_Yaw(TRACK_SEARCH_SPEED);
        return 1;
    }

    if (route_has_line && route_line_lost_ticks <= ROUTE_LINE_LOST_GRACE) {
        Track_With_Speed(route_last_offset, TRACK_SEARCH_SPEED);
        return 1;
    }

    if (route_line_lost_ticks <= ROUTE_LINE_LOST_STOP) {
        Track_With_Speed(route_last_offset, TRACK_SEARCH_SPEED);
        return 1;
    }

    Route_Stop(0);
    return 0;
}

static void Route_Control(void) {
    float dist;
    float lap_start;

    if (!task3_active && !task4_active) {
        return;
    }

    dist = BSP_Encoder_GetDistance();
    lap_start = Route_Lap_Start_Dist();

    if ((task3_active && Runtime_Expired(TASK3_FAILSAFE_MS)) ||
        (task4_active && Runtime_Expired(TASK4_FAILSAFE_MS))) {
        Route_Stop(0);
        return;
    }

    switch (route_state) {
        case 0:
            target_yaw_task1 = route_start_yaw;
            if (dist >= lap_start + ROUTE_AC_CM) {
                Signal_Start(2000, 60);
                Reset_Route_Line();
                Track_Control_Reset();
                route_state = 1;
            } else {
                Straight_With_Yaw(Route_Straight_Speed());
            }
            break;

        case 1:
            if (dist >= lap_start + ROUTE_CB_END_CM) {
                Signal_Start(2000, 60);
                target_yaw_task1 = Normalize_Yaw(route_start_yaw + 180.0f);
                Reset_Route_Line();
                Track_Control_Reset();
                route_state = 2;
            } else {
                Route_Arc_Control(Route_Arc_Speed());
            }
            break;

        case 2:
            target_yaw_task1 = Normalize_Yaw(route_start_yaw + 180.0f);
            if (dist >= lap_start + ROUTE_BD_END_CM) {
                Signal_Start(2000, 60);
                Reset_Route_Line();
                Track_Control_Reset();
                route_state = 3;
            } else {
                Straight_With_Yaw(Route_Straight_Speed());
            }
            break;

        case 3:
            if (dist >= lap_start + ROUTE_LAP_CM) {
                Signal_Start(2000, 60);
                route_lap++;
                if (route_lap >= route_target_laps) {
                    Route_Stop(1);
                    return;
                }
                target_yaw_task1 = route_start_yaw;
                Reset_Route_Line();
                Track_Control_Reset();
                route_state = 0;
            } else {
                Route_Arc_Control(Route_Arc_Speed());
            }
            break;

        default:
            Route_Stop(0);
            break;
    }
}

void Task3_Control(void) {
    Route_Control();
}

void Task4_Control(void) {
    Route_Control();
}

void Task_UpdateTimers(void) {
    if (IsTaskRunning()) {
        if (task_runtime_ms <= 0xFFFFFFF0) {
            task_runtime_ms += 10;
        }
        oled_display_time_ms = task_runtime_ms;
    } else {
        if (task_runtime_ms > 0) {
            oled_display_time_ms = task_runtime_ms;
        }
        task_runtime_ms = 0;
    }

    if (signal_timer_ms > 0) {
        if (signal_timer_ms <= 10) {
            signal_timer_ms = 0;
            BSP_Buzzer_Off();
            BSP_LED_Off();
        } else {
            signal_timer_ms -= 10;
        }
    }
}

uint32_t Get_Oled_Time(void) {
    return oled_display_time_ms;
}

uint8_t IsTaskRunning(void) {
    return (task1_active || task2_active || task3_active || task4_active);
}
