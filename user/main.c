#include "stm32f10x.h"
#include "bsp_motor.h"
#include "bsp_encoder.h"
#include "bsp_track.h"
#include "pid_control.h"
#include "track_control.h"
#include "task.h"
#include "bsp_led.h"
#include "bsp_buzzer.h"
#include "usart1.h"
#include "delay.h"
#include "IOI2C.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "bsp_oled.h"
#include <math.h>

/* 任务所需全局变量 */
uint8_t Way_Angle = 1;
int Temperature;
float Angle_Balance, Gyro_Balance, Gyro_Turn;
float Acceleration_Z;

/* 按键 */
#define KEY1_PIN    GPIO_Pin_1
#define KEY2_PIN    GPIO_Pin_9
#define KEY_PORT    GPIOB
#define KEY_LONG_PRESS_MS 800

/* 声明外部变量，便于在中断中发送到VOFA */
extern float target_yaw_task1;
extern float yaw_correction;
extern float Yaw;

void SWJ_Config(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}

void delay_us(uint32_t nus) {
    uint32_t i;
    for (i = 0; i < nus * 8; i++);
}
void delay_ms(uint32_t nms) {
    uint32_t i, j;
    for (i = 0; i < nms; i++)
        for (j = 0; j < 7200; j++);
}

static uint16_t Wait_Key_Release(uint16_t key_pin) {
    uint16_t press_ms = 0;

    while (GPIO_ReadInputDataBit(KEY_PORT, key_pin) == 0) {
        delay_ms(10);
        if (press_ms < 5000) {
            press_ms += 10;
        }
    }

    return press_ms;
}

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SWJ_Config();
    usart1_init(115200);
    printf("=== Car PD test ===\r\n");

    BSP_LED_Init();
    BSP_Buzzer_Init();
    BSP_Motor_Init();
    BSP_Encoder_Init();
    BSP_Track_Init();
    Pid_Control_Init();
    Track_Control_Init();

    /* MPU6050 初始化（DMP 模式） */
    IIC_Init();
    MPU6050_initialize();
    DMP_Init();

    OLED_Init();
    OLED_Clear();
    OLED_ShowString(60, 24, "1", 16);
    OLED_Refresh();

    /* 按键 GPIO */
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin = KEY1_PIN | KEY2_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT, &gpio);

    /* SysTick 1ms 中断 */
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);

    Pid_Control_Stop();
    printf("PB1 short -> Task1 (100cm straight)\r\n");
    printf("PB9 short -> Task2 (mixed one lap)\r\n");
    printf("PB1 long  -> Task3 (A-C-B-D-A one lap)\r\n");
    printf("PB9 long  -> Task4 (A-C-B-D-A four laps)\r\n");

    while (1) {
        static uint8_t oled_tick = 0;
        static uint8_t was_running = 0;
        if (IsTaskRunning()) {
            was_running = 1;
            oled_tick++;
            if (oled_tick >= 20) {
                oled_tick = 0;
                OLED_Clear();
                OLED_ShowTime(32, 24, Get_Oled_Time());
                OLED_Refresh();
            }
        } else {
            if (was_running) {
                was_running = 0;
                OLED_Clear();
                OLED_ShowTime(32, 24, Get_Oled_Time());
                OLED_Refresh();
            }
            if (GPIO_ReadInputDataBit(KEY_PORT, KEY1_PIN) == 0) {
                delay_ms(20);
                if (GPIO_ReadInputDataBit(KEY_PORT, KEY1_PIN) == 0) {
                    if (Wait_Key_Release(KEY1_PIN) >= KEY_LONG_PRESS_MS) {
                        Task3_Init();
                    } else {
                        Task1_Init();
                    }
                }
            } else if (GPIO_ReadInputDataBit(KEY_PORT, KEY2_PIN) == 0) {
                delay_ms(20);
                if (GPIO_ReadInputDataBit(KEY_PORT, KEY2_PIN) == 0) {
                    if (Wait_Key_Release(KEY2_PIN) >= KEY_LONG_PRESS_MS) {
                        Task4_Init();
                    } else {
                        Task2_Init();
                    }
                }
            }
        }
        delay_ms(10);
    }
}

void SysTick_Handler(void) {
    static uint8_t cnt = 0, vofa_cnt = 0;
    cnt++;
    if (cnt >= 10) {
        cnt = 0;

        BSP_Encoder_UpdateSpeed();
        Read_DMP();   // 更新 Yaw

        if (task1_active) Task1_Control();
        else if (task2_active) Task2_Control();
        else if (task3_active) Task3_Control();
        else if (task4_active) Task4_Control();

        Task_UpdateTimers();

        vofa_cnt++;
        if (vofa_cnt >= 10) {     // 每 100ms 发送一次
            vofa_cnt = 0;
            if (IsTaskRunning()) {
                float data[9];
                // 将角误差归一化到 [-180,180]
                data[0] = Yaw;
                data[1] = target_yaw_task1;
                data[2] = yaw_error_straight;
                data[3] = yaw_correction;
                data[4] = target_speed_left;
                data[5] = target_speed_right;
                data[6] = BSP_Encoder_GetSpeed_Left();
                data[7] = BSP_Encoder_GetSpeed_Right();
                data[8] = BSP_Encoder_GetDistance();
                vofa_send_multi(data, 9);
            } else {
                // 非任务一时可发送速度等信息，但保持接口一致
                float data[9] = {0};
                vofa_send_multi(data, 9);
            }
        }
    }
}
