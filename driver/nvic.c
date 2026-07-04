/**********************************************************************
版权所有：源动力科技
淘    宝：https://1024tech.taobao.com/
文 件 名：nvic.c
版    本：V21.01.30
摘    要：
***********************************************************************/
#include "nvic.h"

void NVIC_config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;    

		//配置UART1中断     
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;               //通道设置为串口1中断    
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;       //中断占先等级    
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;              //中断响应优先级    
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 //打开中断    
    NVIC_Init(&NVIC_InitStructure);       
    
    
}







