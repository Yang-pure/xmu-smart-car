#include "usart1.h"

uint8_t data_to_send[64];

#pragma import(__use_no_semihosting)
struct __FILE
{
    int a;
};

FILE __stdout;

void _sys_exit(int x)
{
}

void _ttywrch(int ch)
{
    // 空实现，避免半主机链接错误
}

int fputc(int ch, FILE *f)
{
    USART1->SR;
    USART_SendData(USART1, (unsigned char)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
    return ch;
}

void usart1_init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
}

void USARTx_DMA_TX_Config(DMA_Channel_TypeDef* DMA_CHx, u32 peripheral_addr, u32 memory_addr, u16 data_length)
{
    DMA_InitTypeDef DMA_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_DeInit(DMA_CHx);

    DMA_InitStructure.DMA_PeripheralBaseAddr = peripheral_addr;
    DMA_InitStructure.DMA_MemoryBaseAddr = memory_addr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = data_length;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA_CHx, &DMA_InitStructure);
}

void USARTx_DMA_SEND_DATA(u32 SendBuff, u16 len)
{
    USARTx_DMA_TX_Config(DMA1_Channel4, (u32)&USART1->DR, (u32)SendBuff, len);
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    DMA_Cmd(DMA1_Channel4, ENABLE);
}

#define BYTE0(dwTemp)       (*( (char *)(&dwTemp)    ))
#define BYTE1(dwTemp)       (*( (char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*( (char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*( (char *)(&dwTemp) + 3))

void vofa_send_vel(float v1, float v2)
{
    unsigned char _cnt = 0;
    data_to_send[_cnt++] = BYTE0(v1);
    data_to_send[_cnt++] = BYTE1(v1);
    data_to_send[_cnt++] = BYTE2(v1);
    data_to_send[_cnt++] = BYTE3(v1);
    data_to_send[_cnt++] = BYTE0(v2);
    data_to_send[_cnt++] = BYTE1(v2);
    data_to_send[_cnt++] = BYTE2(v2);
    data_to_send[_cnt++] = BYTE3(v2);
    data_to_send[_cnt++] = 0x00;
    data_to_send[_cnt++] = 0x00;
    data_to_send[_cnt++] = 0x80;
    data_to_send[_cnt++] = 0x7F;
    USARTx_DMA_SEND_DATA((u32)data_to_send, _cnt);
}

void vofa_send_vel3(float v1, float v2, float v3)
{
    unsigned char _cnt = 0;
    data_to_send[_cnt++] = BYTE0(v1);
    data_to_send[_cnt++] = BYTE1(v1);
    data_to_send[_cnt++] = BYTE2(v1);
    data_to_send[_cnt++] = BYTE3(v1);
    data_to_send[_cnt++] = BYTE0(v2);
    data_to_send[_cnt++] = BYTE1(v2);
    data_to_send[_cnt++] = BYTE2(v2);
    data_to_send[_cnt++] = BYTE3(v2);
    data_to_send[_cnt++] = BYTE0(v3);
    data_to_send[_cnt++] = BYTE1(v3);
    data_to_send[_cnt++] = BYTE2(v3);
    data_to_send[_cnt++] = BYTE3(v3);
    data_to_send[_cnt++] = 0x00;
    data_to_send[_cnt++] = 0x00;
    data_to_send[_cnt++] = 0x80;
    data_to_send[_cnt++] = 0x7F;
    USARTx_DMA_SEND_DATA((u32)data_to_send, _cnt);
}

// 发送多个浮点数
void vofa_send_multi(float *data, uint8_t num)
{
    uint8_t idx = 0;
    uint8_t i;

    if (num > 10) num = 10;
    for (i = 0; i < num; i++) {
        data_to_send[idx++] = BYTE0(data[i]);
        data_to_send[idx++] = BYTE1(data[i]);
        data_to_send[idx++] = BYTE2(data[i]);
        data_to_send[idx++] = BYTE3(data[i]);
    }
    data_to_send[idx++] = 0x00;
    data_to_send[idx++] = 0x00;
    data_to_send[idx++] = 0x80;
    data_to_send[idx++] = 0x7F;
    USARTx_DMA_SEND_DATA((u32)data_to_send, idx);
}

void vofa_send_debug(float speedL, float speedR, float targetL, float targetR, float yaw)
{
    float data[5] = {speedL, speedR, targetL, targetR, yaw};
    vofa_send_multi(data, 5);
}

uint8_t ch;
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        ch = USART_ReceiveData(USART1);
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
