#include "Rs485_HL.h"

#define RS485_INT_CHANNEL        USART1_IRQn
const PORT_INF RS485_PORT_TX = {GPIOA, GPIO_Pin_9};
const PORT_INF RS485_PORT_RX = {GPIOA, GPIO_Pin_10};
const PORT_INF RS485_PORT_CTRL = {GPIOA, GPIO_Pin_11};
const PORT_INF RS485_PORT_PWR = {GPIOA, GPIO_Pin_12};
void Rs485_InitInterface(u32 baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
	
    USART_DeInit(RS485_PORT);

    GPIO_InitStructure.GPIO_Pin = RS485_PORT_TX.Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(RS485_PORT_TX.Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = RS485_PORT_RX.Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(RS485_PORT_RX.Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = RS485_PORT_CTRL.Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(RS485_PORT_CTRL.Port, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = RS485_PORT_PWR.Pin;
    GPIO_Init(RS485_PORT_PWR.Port, &GPIO_InitStructure);
    
    Rs485_EnableRx();

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    //Configure USART
    USART_Init(RS485_PORT, &USART_InitStructure);
    // Enable the USART
    USART_Cmd(RS485_PORT, ENABLE);
    //USART_IT_IDLE   USART_IT_RXNE
    USART_ITConfig(RS485_PORT, USART_IT_IDLE, ENABLE);
    USART_DMACmd(RS485_PORT, USART_DMAReq_Tx, ENABLE);
    USART_DMACmd(RS485_PORT, USART_DMAReq_Rx, ENABLE);
}

void Rs485_ConfigInt(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = RS485_INT_CHANNEL;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void Rs485_EnableInt(FunctionalState rxState, FunctionalState txState)
{
    USART_ClearITPendingBit(RS485_PORT, USART_IT_RXNE | USART_IT_TC);

    USART_ITConfig(RS485_PORT, USART_IT_RXNE, rxState);
    USART_ITConfig(RS485_PORT, USART_IT_TC, txState);
}
void Rs485_Delayms(u32 n)
{
    n *= 0x3800;
    n++;
    while(n--);
}
void Rs485_WriteByte(u8 ch)
{
	
    while(((RS485_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);
	(RS485_PORT)->DR = (ch & (u16)0x01FF);
}


void Rs485_InitTxDma(u8 *pTxBuffer, u32 len)
{
    NVIC_InitTypeDef  NVIC_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};

    //DMA1通道5配置
    DMA_DeInit(RS485_TXDMA_CH);
    //外设地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(RS485_PORT->DR));
    
    //内存地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)pTxBuffer;
    //dma传输方向单向
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    //设置DMA在传输时缓冲区的长度
    DMA_InitStructure.DMA_BufferSize = len;
    //设置DMA的外设递增模式，一个外设
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    //设置DMA的内存递增模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    //外设数据字长
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    //内存数据字长
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    //设置DMA的传输模式
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    //设置DMA的优先级别
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    //设置DMA的2个memory中的变量互相访问
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(RS485_TXDMA_CH, &DMA_InitStructure);

    DMA_ITConfig(RS485_TXDMA_CH, DMA_IT_TC, ENABLE);  //open DMA send inttrupt
    Rs485_DisableTxDma();// 关闭DMA
    
    //dma interrupt
    NVIC_InitStructure.NVIC_IRQChannel = RS485_TXDMA_INT;                                       //

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = INT_PRIORITY_UART_DMA;    //
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = INT_PRIORITY_GROUP;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void Rs485_InitRxDma(u8 *pRxBuffer, u32 len)
{
    NVIC_InitTypeDef  NVIC_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};

    DMA_DeInit(RS485_RXDMA_CH);
    //外设地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(RS485_PORT->DR));

    //内存地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)pRxBuffer;
    //dma传输方向单向
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    //设置DMA在传输时缓冲区的长度
    DMA_InitStructure.DMA_BufferSize = len;
    //设置DMA的外设递增模式，一个外设
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    //设置DMA的内存递增模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    //外设数据字长
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    //内存数据字长
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    //设置DMA的传输模式
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    //设置DMA的优先级别
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    //设置DMA的2个memory中的变量互相访问
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    
    DMA_Init(RS485_RXDMA_CH, &DMA_InitStructure);
    DMA_ITConfig(RS485_RXDMA_CH, DMA_IT_TC, ENABLE);  //open DMA send inttrupt
    Rs485_DisableRxDma();// 关闭DMA
    
    //dma interrupt
    NVIC_InitStructure.NVIC_IRQChannel = RS485_RXDMA_INT; 

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = INT_PRIORITY_UART_DMA;    //
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = INT_PRIORITY_GROUP;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void Rs485_WriteBuffer(u8 *pFrame, u16 len)
{
    u16 i = 0;
    Rs485_EnableTx();
    Rs485_Delayms(1);
    for(i = 0; i < len; i++)
    {
		Rs485_WriteByte(pFrame[i]);
    }
    Rs485_ChkTxOver();
    Rs485_Delayms(1);
    Rs485_EnableRx();
    Rs485_Delayms(1);
}

void Rs485_WriteWarStr(char *str)
{
    Rs485_WriteBuffer((u8 *)str, strlen(str));
    Rs485_WriteByte('\r');
    Rs485_WriteByte('\n');
}



