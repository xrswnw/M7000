#ifndef _RS485_H_
#define _RS485_H_

#include "Rs485_HL.h"

typedef struct uartFrame{
    u16 lenth;
    u16 index;
    u8 frame[RS485_RX_FRAME_LEN];
}UART_FRAME;


extern UART_FRAME g_sRs485TempFrame;
extern QueueHandle_t g_sRs485TxQueue;
extern QueueHandle_t g_sRs485RxQueue;

extern TaskHandle_t g_hRs485Rx;
extern TaskHandle_t g_hRs485Tx;

#define UART_TX_QUEUE_NUM           4
#define UART_RX_QUEUE_NUM           4

#define Rs485_RstFrame()        do{memset(&g_sRs485TempFrame, 0, sizeof(g_sRs485TempFrame));}while(0)


void Rs485_Init(u32 bud);
void Rs485_QueueInit(void);
void Rs485RxTask(void *pvParameters);
void Rs485TxTask(void *pvParameters);
#endif
