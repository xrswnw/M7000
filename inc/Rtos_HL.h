#ifndef _RTOS_HL_
#define _RTOS_HL_

#include "stdio.h"

#if (defined(_ANYID_STM32_) || defined(_ANYID_GD32_))
    #include "stm32f10x.h"
    typedef struct Port_Inf
    {
        GPIO_TypeDef* Port;
        u16 Pin;
    } PORT_INF;
#endif

//HL
#define UART_PORT                           USART2
#define Uart_IRQHandler                     USART2_IRQHandler
#define UART_BAUDRARE                       115200

#define UART_TX_FRAME_LEN                   (64 + 32)
#define UART_RX_FRAME_LEN                   (256 + 32)

#define Uart_ReadByte()                     ((u16)(UART_PORT->DR & (u16)0x01FF))
#define Uart_ChkTxOver()                    while(((UART_PORT)->SR & USART_FLAG_TC) == (u16)RESET)

#define UART_SR_IDLE                        0x0010  
#define UART_DMA                            DMA1

#define UART_TXDMA_CH                       DMA1_Channel7
#define UART_TXDMA_INT                      DMA1_Channel7_IRQn
#define Uart_TxDMAIRQHandler                DMA1_Channel7_IRQHandler
#define UART_TXDMA_TC_FLAG                  DMA1_FLAG_TC7

#define Uart_EnableTxDma(p, s)              do{\
                                                (UART_DMA)->IFCR = UART_TXDMA_TC_FLAG; \
                                                (UART_TXDMA_CH)->CMAR = ((u32)(p)); \
                                                (UART_TXDMA_CH)->CNDTR = (s); \
                                                (UART_TXDMA_CH)->CCR |= CCR_ENABLE_Set; \
                                             }while(0)

#define Uart_DisableTxDma()                 do{\
                                                (UART_DMA)->IFCR = UART_TXDMA_TC_FLAG;\
                                                (UART_TXDMA_CH)->CCR &= CCR_ENABLE_Reset;\
                                                Uart_ChkTxOver();\
                                            }while(0)

#define UART_RXDMA_CH                       DMA1_Channel6
#define UART_RXDMA_INT                      DMA1_Channel6_IRQn
#define UART_RxDMAIRQHandler                DMA1_Channel6_IRQHandler
#define UART_RXDMA_TC_FLAG                  DMA1_FLAG_TC6
#define Uart_EnableRxDma()                  do{\
                                                (UART_DMA)->IFCR = UART_RXDMA_TC_FLAG; \
                                                (UART_RXDMA_CH)->CNDTR = UART_RX_FRAME_LEN; \
                                                (UART_RXDMA_CH)->CCR |= CCR_ENABLE_Set; \
                                            }while(0)

#define Uart_GetRxLen()                     (UART_RX_FRAME_LEN - (UART_RXDMA_CH)->CNDTR)  

#define Uart_DisableRxDma()                 do{\
                                                (UART_DMA)->IFCR = UART_RXDMA_TC_FLAG;\
                                                (UART_RXDMA_CH)->CCR &= CCR_ENABLE_Reset;\
                                            }while(0)
                                              
                                              
#define TIM_PSC                             (72000000 / 720)
#define TIM_TOTAL_1MS                       (TIM_PSC / 1000)
#define TIM_TOTAL_5MS                       (TIM_TOTAL_1MS * 5 + 1)                 //偏移一个Tick，减少偶然重发

void Uart_InitInterface();

void Uart_InitRxDma(u8 *pRxBuffer, u32 len);
void Uart_InitTxDma(u8 *pTxBuffer, u32 len);
void Uart_WriteStr(char *str);
void Uart_WriteBuffer(u8 *pFrame, u16 len);
void Uart_WriteByte(u8 ch);
void Uart_Delayms(u32 n);
void Uart_Init();

extern u32 FreeRTOSRunTimeTicks;
extern u32 g_nSysTick;

void Tim_Init(u32 period, u32 prescaler);
void ConfigureTimeForRunTimeStats();
#endif