#ifndef _RS485_H_
#define _RS485_H_

#include "Rs485_HL.h"



































void UART_QueueInit(void);
void vUartRxTask(void *pvParameters);

void vUartTxTask(void *pvParameters);
void UART_SendByte(uint8_t data);
void USART1_IRQHandler(void);
void Rs485_Init(u32 bud);



void UART_SendBuffer(u8 *pBuffer, u8 len);
#endif
