#include "Rs485.h"

UART_FRAME g_sRs485TempFrame = {0};

QueueHandle_t g_sRs485TxQueue = NULL;
QueueHandle_t g_sRs485RxQueue = NULL;

void Rs485_Init(u32 bud)
{
    Rs485_QueueInit();                   //队列进行收发
    
    Rs485_InitInterface(bud);
    Rs485_ConfigInt();
    Rs485_InitTxDma(g_sRs485TempFrame.frame, g_sRs485TempFrame.lenth);
    Rs485_InitRxDma(g_sRs485TempFrame.frame, g_sRs485TempFrame.lenth);
    Rs485_EnableRxDma();
}

void Rs485_QueueInit(void)
{
    g_sRs485TxQueue = xQueueCreate(UART_TX_QUEUE_NUM, sizeof(UART_FRAME));
    g_sRs485RxQueue = xQueueCreate(UART_RX_QUEUE_NUM, sizeof(UART_FRAME));
}

void Rs485TxTask(void *pvParameters)
{
    UART_FRAME uartFrame = {0};
    while (1)
    {
        memset(&uartFrame, 0, sizeof(uartFrame));
        if(xQueueReceive(g_sRs485TxQueue, &uartFrame, portMAX_DELAY) == pdTRUE)
        {
            Rs485_WriteBuffer(uartFrame.frame, uartFrame.lenth);
        }
    }
}

void Rs485RxTask(void *pvParameters)
{
    UART_FRAME uartFrame = {0};
    while (1)
    {
        memset(&uartFrame, 0, sizeof(uartFrame)); 
        if(xQueueReceive(g_sRs485RxQueue, &uartFrame, portMAX_DELAY) == pdTRUE)
        {  
            xQueueSend(g_sRs485TxQueue, &uartFrame, portMAX_DELAY);
        }
    }
}
