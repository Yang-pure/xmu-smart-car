#include "bsp_oled.h"

#define OLED_SCL_PIN    GPIO_Pin_10
#define OLED_SDA_PIN    GPIO_Pin_11
#define OLED_PORT       GPIOB

#define OLED_SCL_H()    GPIO_SetBits(OLED_PORT, OLED_SCL_PIN)
#define OLED_SCL_L()    GPIO_ResetBits(OLED_PORT, OLED_SCL_PIN)
#define OLED_SDA_H()    GPIO_SetBits(OLED_PORT, OLED_SDA_PIN)
#define OLED_SDA_L()    GPIO_ResetBits(OLED_PORT, OLED_SDA_PIN)
#define OLED_SDA_IN()   GPIO_ReadInputDataBit(OLED_PORT, OLED_SDA_PIN)

#define OLED_SDA_OUT()  {OLED_PORT->CRH &= 0xFFFF0FFF; OLED_PORT->CRH |= 0x00003000;}
#define OLED_SDA_INM()  {OLED_PORT->CRH &= 0xFFFF0FFF; OLED_PORT->CRH |= 0x00008000;}

static void OLED_Delay(void) {
    volatile uint32_t i;
    for (i = 0; i < 200; i++);
}

static int OLED_IIC_Start(void) {
    OLED_SDA_OUT();
    OLED_SDA_H();
    OLED_SCL_H();
    OLED_Delay();
    OLED_SDA_L();
    OLED_Delay();
    OLED_SCL_L();
    return 1;
}

static void OLED_IIC_Stop(void) {
    OLED_SDA_OUT();
    OLED_SCL_L();
    OLED_SDA_L();
    OLED_Delay();
    OLED_SCL_H();
    OLED_SDA_H();
    OLED_Delay();
}

static int OLED_IIC_Wait_Ack(void) {
    uint8_t t = 0;
    OLED_SDA_INM();
    OLED_SDA_H();
    OLED_Delay();
    OLED_SCL_H();
    OLED_Delay();
    while (OLED_SDA_IN()) {
        t++;
        if (t > 200) {
            OLED_IIC_Stop();
            return 0;
        }
    }
    OLED_SCL_L();
    return 1;
}

static void OLED_IIC_SendByte(uint8_t data) {
    uint8_t i;
    OLED_SDA_OUT();
    OLED_SCL_L();
    for (i = 0; i < 8; i++) {
        if (data & 0x80)
            OLED_SDA_H();
        else
            OLED_SDA_L();
        data <<= 1;
        OLED_Delay();
        OLED_SCL_H();
        OLED_Delay();
        OLED_SCL_L();
        OLED_Delay();
    }
}

static const uint8_t OLED_INIT_CMD[] = {
    0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
    0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
    0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF
};

static uint8_t OLED_GRAM[128][8];

static void OLED_WriteByte(uint8_t data, uint8_t mode) {
    OLED_IIC_Start();
    OLED_IIC_SendByte(0x78);
    OLED_IIC_Wait_Ack();
    if (mode == OLED_CMD)
        OLED_IIC_SendByte(0x00);
    else
        OLED_IIC_SendByte(0x40);
    OLED_IIC_Wait_Ack();
    OLED_IIC_SendByte(data);
    OLED_IIC_Wait_Ack();
    OLED_IIC_Stop();
}

void OLED_Refresh(void) {
    for (uint8_t page = 0; page < 8; page++) {
        OLED_WriteByte(0xB0 + page, OLED_CMD);
        OLED_WriteByte(0x00, OLED_CMD);
        OLED_WriteByte(0x10, OLED_CMD);
        OLED_IIC_Start();
        OLED_IIC_SendByte(0x78);
        OLED_IIC_Wait_Ack();
        OLED_IIC_SendByte(0x40);
        OLED_IIC_Wait_Ack();
        for (uint8_t col = 0; col < 128; col++) {
            OLED_IIC_SendByte(OLED_GRAM[col][page]);
            OLED_IIC_Wait_Ack();
        }
        OLED_IIC_Stop();
    }
}

void OLED_Clear(void) {
    for (uint8_t i = 0; i < 128; i++)
        for (uint8_t j = 0; j < 8; j++)
            OLED_GRAM[i][j] = 0x00;
}

void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t) {
    if (x > 127 || y > 63) return;
    if (t)
        OLED_GRAM[x][y/8] |=  (1 << (y%8));
    else
        OLED_GRAM[x][y/8] &= ~(1 << (y%8));
}

void OLED_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot) {
    for (uint8_t x = x1; x <= x2; x++)
        for (uint8_t y = y1; y <= y2; y++)
            OLED_DrawPoint(x, y, dot);
}

static const uint8_t asc2_0608[95][6] = {
    {0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x5F,0x00,0x00,0x00},
    {0x00,0x07,0x00,0x07,0x00,0x00},
    {0x14,0x7F,0x14,0x7F,0x14,0x00},
    {0x24,0x2A,0x7F,0x2A,0x12,0x00},
    {0x23,0x13,0x08,0x64,0x62,0x00},
    {0x36,0x49,0x55,0x22,0x50,0x00},
    {0x00,0x05,0x03,0x00,0x00,0x00},
    {0x00,0x1C,0x22,0x41,0x00,0x00},
    {0x00,0x41,0x22,0x1C,0x00,0x00},
    {0x14,0x08,0x3E,0x08,0x14,0x00},
    {0x08,0x08,0x3E,0x08,0x08,0x00},
    {0x00,0x50,0x30,0x00,0x00,0x00},
    {0x08,0x08,0x08,0x08,0x08,0x00},
    {0x00,0x60,0x60,0x00,0x00,0x00},
    {0x20,0x10,0x08,0x04,0x02,0x00},
    {0x3E,0x51,0x49,0x45,0x3E,0x00},
    {0x00,0x42,0x7F,0x40,0x00,0x00},
    {0x42,0x61,0x51,0x49,0x46,0x00},
    {0x21,0x41,0x45,0x4B,0x31,0x00},
    {0x18,0x14,0x12,0x7F,0x10,0x00},
    {0x27,0x45,0x45,0x45,0x39,0x00},
    {0x3C,0x4A,0x49,0x49,0x30,0x00},
    {0x01,0x71,0x09,0x05,0x03,0x00},
    {0x36,0x49,0x49,0x49,0x36,0x00},
    {0x06,0x49,0x49,0x29,0x1E,0x00},
    {0x00,0x36,0x36,0x00,0x00,0x00},
};

static const uint8_t asc2_0816[10][16] = {
    {0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x0F,0x10,0x20,0x20,0x10,0x0F,0x00},
    {0x00,0x10,0x10,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00},
    {0x00,0x70,0x08,0x08,0x08,0x88,0x70,0x00,0x00,0x30,0x28,0x24,0x22,0x21,0x30,0x00},
    {0x00,0x30,0x08,0x88,0x88,0x48,0x30,0x00,0x00,0x18,0x20,0x20,0x20,0x11,0x0E,0x00},
    {0x00,0x00,0xC0,0x20,0x10,0xF8,0x00,0x00,0x00,0x07,0x04,0x24,0x24,0x3F,0x24,0x00},
    {0x00,0xF8,0x08,0x88,0x88,0x08,0x08,0x00,0x00,0x19,0x21,0x20,0x20,0x11,0x0E,0x00},
    {0x00,0xE0,0x10,0x88,0x88,0x18,0x00,0x00,0x00,0x0F,0x11,0x20,0x20,0x11,0x0E,0x00},
    {0x00,0x38,0x08,0x08,0xC8,0x38,0x08,0x00,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00},
    {0x00,0x70,0x88,0x08,0x08,0x88,0x70,0x00,0x00,0x1C,0x22,0x21,0x21,0x22,0x1C,0x00},
    {0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x00,0x31,0x22,0x22,0x11,0x0F,0x00},
};

static const uint8_t font16_dot[16] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x30,0x30,0x00,0x00,0x00,0x00,0x00,0x00,
};

static const uint8_t font16_S[16] = {
    0x00,0x70,0x88,0x08,0x08,0x88,0x70,0x00,
    0x00,0x0E,0x11,0x20,0x20,0x11,0x0E,0x00,
};

static const uint8_t font16_space[16] = {0};

static const uint8_t* OLED_GetFont16(char ch) {
    if (ch >= '0' && ch <= '9') return asc2_0816[ch - '0'];
    if (ch == '.') return font16_dot;
    if (ch == 'S') return font16_S;
    return font16_space;
}

static void OLED_DrawFont16(uint8_t x, uint8_t y, const uint8_t* font) {
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t line = font[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (line & (1 << j))
                OLED_DrawPoint(x + i, y + j, 1);
            else
                OLED_DrawPoint(x + i, y + j, 0);
        }
        line = font[i + 8];
        for (uint8_t j = 0; j < 8; j++) {
            if (line & (1 << j))
                OLED_DrawPoint(x + i, y + 8 + j, 1);
            else
                OLED_DrawPoint(x + i, y + 8 + j, 0);
        }
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size) {
    if (size == 12) {
        uint8_t c = ch - ' ';
        if (c > 94) return;
        for (uint8_t i = 0; i < 6; i++) {
            uint8_t line = asc2_0608[c][i];
            for (uint8_t j = 0; j < 8; j++) {
                if (line & (1 << j))
                    OLED_DrawPoint(x + i, y + j, 1);
                else
                    OLED_DrawPoint(x + i, y + j, 0);
            }
        }
    } else if (size == 16) {
        OLED_DrawFont16(x, y, OLED_GetFont16(ch));
    }
}

void OLED_ShowTime(uint8_t x, uint8_t y, uint32_t time_ms) {
    char buf[9];
    uint16_t sec = time_ms / 1000;
    uint8_t dec = (time_ms % 1000) / 10;
    uint8_t pos = 0;
    if (sec >= 100) {
        buf[pos++] = '0' + (sec / 100) % 10;
        buf[pos++] = '0' + (sec / 10) % 10;
    } else if (sec >= 10) {
        buf[pos++] = ' ';
        buf[pos++] = '0' + (sec / 10) % 10;
    } else {
        buf[pos++] = ' ';
        buf[pos++] = ' ';
    }
    buf[pos++] = '0' + sec % 10;
    buf[pos++] = '.';
    buf[pos++] = '0' + dec / 10;
    buf[pos++] = '0' + dec % 10;
    buf[pos++] = ' ';
    buf[pos++] = 'S';
    buf[pos] = '\0';
    OLED_ShowString(x, y, buf, 16);
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size) {
    while (*str) {
        OLED_ShowChar(x, y, *str, size);
        x += (size == 16 ? 8 : 6);
        if (x > 120) { x = 0; y += (size == 16 ? 2 : (size == 12 ? 2 : 1)); }
        str++;
    }
}

void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size) {
    char buf[12];
    for (uint8_t i = 0; i < len; i++) {
        buf[len - 1 - i] = (num % 10) + '0';
        num /= 10;
    }
    buf[len] = '\0';
    OLED_ShowString(x, y, buf, size);
}

void OLED_Init(void) {
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin = OLED_SCL_PIN | OLED_SDA_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(OLED_PORT, &gpio);
    OLED_SCL_H();
    OLED_SDA_H();

    for (uint8_t i = 0; i < sizeof(OLED_INIT_CMD); i++) {
        OLED_WriteByte(OLED_INIT_CMD[i], OLED_CMD);
    }
    OLED_Clear();
    OLED_Refresh();
}