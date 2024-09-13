#include "LTE_HL.h"

#define LTE_INT_CHANNEL        USART3_IRQn
const PORT_INF LTE_PORT_TX = {GPIOB, GPIO_Pin_10};
const PORT_INF LTE_PORT_RX = {GPIOB, GPIO_Pin_11};

const PORT_INF LTE_PORT_KEY = {GPIOB, GPIO_Pin_3};
const PORT_INF LTE_PORT_BOOT_CTR = {GPIOB, GPIO_Pin_4};
const PORT_INF LTE_PORT_RST = {GPIOB, GPIO_Pin_5};

void LTE_InitInterface(u32 baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = LTE_PORT_KEY.Pin;
    GPIO_Init(LTE_PORT_KEY.Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LTE_PORT_BOOT_CTR.Pin;
    GPIO_Init(LTE_PORT_BOOT_CTR.Port, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LTE_PORT_RST.Pin;
    GPIO_Init(LTE_PORT_RST.Port, &GPIO_InitStructure);

    LTE_BootLow();
    LTE_KeyLow();
    LTE_CtrlLow();
    USART_DeInit(LTE_PORT);

    GPIO_InitStructure.GPIO_Pin = LTE_PORT_TX.Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(LTE_PORT_TX.Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LTE_PORT_RX.Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(LTE_PORT_RX.Port, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    //Configure USART
    USART_Init(LTE_PORT, &USART_InitStructure);

    USART_Cmd(LTE_PORT, ENABLE);
    //USART_DMACmd(LTE_PORT, USART_DMAReq_Tx, ENABLE);
}

void LTE_ConfigInt(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = LTE_INT_CHANNEL;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = INT_PRIORITY_GROUP >> 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = INT_PRIORITY_GROUP & 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void LTE_EnableInt(FunctionalState rxState, FunctionalState txState)
{
    USART_ClearITPendingBit(LTE_PORT, USART_IT_RXNE | USART_IT_TC);

    USART_ITConfig(LTE_PORT, USART_IT_RXNE, rxState);
    USART_ITConfig(LTE_PORT, USART_IT_TC, txState);
}

void LTE_WriteByte(u8 ch)
{
    while(((LTE_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);
	(LTE_PORT)->DR = (ch & (u16)0x01FF);
}

void LTE_Delayms(u32 n)
{                  //3800-72M 6000-120M
    n *= 0x6000;
    n++;
    while(n--);
}

void LTE_WriteBuffer(u8 *pFrame, u16 len)
{
    u16 i = 0;
    for(i = 0; i < len; i++)
    {
        while(((LTE_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);
        (LTE_PORT)->DR = (pFrame[i] & (u16)0x01FF);
    }
}

void LTE_WriteCmd(char *str)
{
    LTE_WriteBuffer((u8 *)str, strlen(str));
    LTE_WriteByte('\r');
    LTE_WriteByte('\n');
}

