#include "Rs485.h"

void Rs485_Init(u32 bud)
{
    Rs485_InitInterface(bud);
    Rs485_ConfigInt();
   // Rs485_InitRxDma(g_sRs485RcvFrame.buffer, SLAVE_RX_FRAME_LEN);
  //  Rs485_InitTxDma(g_sRs485Info.tx.frame, g_sRs485Info.tx.lenth);
    Rs485_EnableRxDma();
    
    UART_QueueInit();
}




#define UART_TX_BUFFER_SIZE 128
#define UART_RX_BUFFER_SIZE 128

QueueHandle_t xTxQueue;
QueueHandle_t xRxQueue;

void vUartTxTask(void *pvParameters)
{
    uint8_t txData;
    
    while (1)
    {
        if (xQueueReceive(xTxQueue, &txData, portMAX_DELAY) == pdTRUE)
        {
            Rs485_EnableTx();
            Rs485_Delayms(1);
            USART_SendData(USART1, txData);
            Rs485_EnableRx();
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        }
    }
}

void vUartRxTask(void *pvParameters)
{
    uint8_t rxData;
    
    while (1)
    {
        if (xQueueReceive(xRxQueue, &rxData, portMAX_DELAY) == pdTRUE)
        {
            printf("Received data: %c\n", rxData);
        }
    }
}


void UART_QueueInit(void)
{
    xTxQueue = xQueueCreate(UART_TX_BUFFER_SIZE, sizeof(uint8_t));
    xRxQueue = xQueueCreate(UART_RX_BUFFER_SIZE, sizeof(uint8_t));
}

// 从串口发送一个字节
void UART_SendByte(uint8_t data)
{
    if (xQueueSendToBack(xTxQueue, &data, 0) != pdTRUE)
    {

    }
}

void UART_SendBuffer(u8 *pBuffer, u8 len)
{
    u8 index = 0;
    
    for(index = 0; index < len; index++)
    {
        UART_SendByte(*(pBuffer + index));
    }
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        uint8_t rxData = USART_ReceiveData(USART1);
        
        if (xQueueSendToBackFromISR(xRxQueue, &rxData, NULL) != pdTRUE)
        {
            
        }
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

  
