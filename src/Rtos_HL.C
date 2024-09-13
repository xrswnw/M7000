#include "Rtos_HL.h"

// Debug log Out

#define UART_INT_CHANNEL        USART2_IRQn
const PORT_INF UART_PORT_TX = {GPIOA, GPIO_Pin_2};
const PORT_INF UART_PORT_RX = {GPIOA, GPIO_Pin_3};

void Uart_InitInterface()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
	NVIC_InitTypeDef NVIC_InitStructure = {0};
    
    USART_DeInit(UART_PORT);

    GPIO_InitStructure.GPIO_Pin = UART_PORT_TX.Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(UART_PORT_TX.Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = UART_PORT_RX.Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART_PORT_RX.Port, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = UART_BAUDRARE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(UART_PORT, &USART_InitStructure);
    USART_Cmd(UART_PORT, ENABLE);
    
    USART_ITConfig(UART_PORT, USART_IT_IDLE, ENABLE);
    //USART_DMACmd(UART_PORT, USART_DMAReq_Tx, ENABLE);
    USART_DMACmd(UART_PORT, USART_DMAReq_Rx, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = UART_INT_CHANNEL;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    
    NVIC_Init(&NVIC_InitStructure);
}

void Uart_EnableInt(FunctionalState rxState, FunctionalState txState)
{
    USART_ClearITPendingBit(UART_PORT, USART_IT_RXNE | USART_IT_TC);
    USART_ITConfig(UART_PORT, USART_IT_RXNE, ENABLE);
    USART_ITConfig(UART_PORT, USART_IT_TC, ENABLE);
}

void Uart_Delayms(u32 n)
{
    n *= 0x3800;
    n++;
    while(n--);
}
void Uart_WriteByte(u8 ch)
{
	
    while(((UART_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);
	(UART_PORT)->DR = (ch & (u16)0x01FF);
}

void Uart_WriteBuffer(u8 *pFrame, u16 len)
{
    u16 i = 0;

    for(i = 0; i < len; i++)
    {
		Uart_WriteByte(pFrame[i]);
    }
    Uart_ChkTxOver();
}

void Uart_WriteStr(char *str)
{
    Uart_WriteBuffer((u8 *)str, strlen(str));
    Uart_WriteByte('\r');
    Uart_WriteByte('\n');
}

void Uart_InitTxDma(u8 *pTxBuffer, u32 len)
{
    NVIC_InitTypeDef  NVIC_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};

    //DMA1通道5配置
    DMA_DeInit(UART_TXDMA_CH);
    //外设地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(UART_PORT->DR));
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

    DMA_Init(UART_TXDMA_CH, &DMA_InitStructure);

    DMA_ITConfig(UART_TXDMA_CH, DMA_IT_TC, ENABLE);  //open DMA send inttrupt
    Uart_DisableTxDma();// 关闭DMA
    
    NVIC_InitStructure.NVIC_IRQChannel = UART_TXDMA_INT;                                       //

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;    //
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void Uart_InitRxDma(u8 *pRxBuffer, u32 len)
{
    NVIC_InitTypeDef  NVIC_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};

    DMA_DeInit(UART_RXDMA_CH);
    //外设地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(UART_PORT->DR));

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
    
    DMA_Init(UART_RXDMA_CH, &DMA_InitStructure);
    DMA_ITConfig(UART_RXDMA_CH, DMA_IT_TC, ENABLE);  //open DMA send inttrupt
    Uart_DisableRxDma();// 关闭DMA
    
    //dma interrupt
    NVIC_InitStructure.NVIC_IRQChannel = UART_RXDMA_INT; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;    //
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

u8 g_nUartBuffer[UART_RX_FRAME_LEN] = {0};

void Uart_Init()
{
    Uart_InitInterface();
    Uart_InitRxDma(g_nUartBuffer, UART_RX_FRAME_LEN);
    Uart_EnableRxDma();
}


/*
size_t __write(int handle, const unsigned char *buf, size_t bufSize) {
		int sendsize;

    sendsize = uartSend((uint8_t *)buf,bufSize);

    return sendsize ;
}
*/
//映射至PRINTF
int fputc(int Byte, FILE *pF)
{
    Uart_WriteByte(*(u8 *)&Byte);
    return Byte;
}

//定时器，硬件为基，软件扩展
u32 FreeRTOSRunTimeTicks = 0;
void Tim_Init(u32 period, u32 prescaler)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    TIM_TimeBaseStructure.TIM_Period = period;             
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler;         
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;            
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;    
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);   

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE ); 
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 

    TIM_Cmd(TIM2, ENABLE);  
}

void ConfigureTimeForRunTimeStats()
{
   Tim_Init(10 - 1, 72 -1);
}

//-----------Static Idle Task Creat 
