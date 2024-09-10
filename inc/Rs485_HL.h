#ifndef _RS485_HL_H_
#define _RS485_HL_H_

#include "Config.h"

#define RS485_PORT                       USART1
#define Rs485_IRQHandler                 USART1_IRQHandler
#define RS485_BAUDRARE                   38400

#define RS485_TX_FRAME_LEN               (64 + 32)
#define RS485_RX_FRAME_LEN               (512 + 32)

#define Rs485_ReadByte()                ((u16)(RS485_PORT->DR & (u16)0x01FF))
#define Rs485_ChkTxOver()               while(((RS485_PORT)->SR & USART_FLAG_TC) == (u16)RESET)

extern const PORT_INF RS485_PORT_CTRL;
#define Rs485_EnableRx()                    (RS485_PORT_CTRL.Port->BRR = RS485_PORT_CTRL.Pin)
#define Rs485_EnableTx()                    (RS485_PORT_CTRL.Port->BSRR = RS485_PORT_CTRL.Pin)

extern const PORT_INF RS485_PORT_PWR;
#define Rs485_ClosePwr()                     (RS485_PORT_PWR.Port->BRR = RS485_PORT_PWR.Pin)
#define Rs485_OpenPwr()                      (RS485_PORT_PWR.Port->BSRR = RS485_PORT_PWR.Pin)

#define RS485_SR_IDLE                        0x0010  
#define RS485_DMA                            DMA1

#define RS485_TXDMA_CH                       DMA1_Channel4
#define RS485_TXDMA_INT                      DMA1_Channel4_IRQn
#define Rs485_TxDMAIRQHandler                DMA1_Channel4_IRQHandler
#define RS485_TXDMA_TC_FLAG                  DMA1_FLAG_TC4

#define Rs485_EnableTxDma(p, s)              do{\
                                                Rs485_EnableTx();\
                                                Rs485_Delayms(1);\
                                                (RS485_DMA)->IFCR = RS485_TXDMA_TC_FLAG; \
                                                (RS485_TXDMA_CH)->CMAR = ((u32)(p)); \
                                                (RS485_TXDMA_CH)->CNDTR = (s); \
                                                (RS485_TXDMA_CH)->CCR |= CCR_ENABLE_Set; \
                                             }while(0)
                                                                                             
#define Rs485_DisableTxDma()                 do{\
                                                (RS485_DMA)->IFCR = RS485_TXDMA_TC_FLAG;\
                                                (RS485_TXDMA_CH)->CCR &= CCR_ENABLE_Reset;\
                                                Rs485_ChkTxOver();\
                                                Rs485_EnableRx();\
                                            }while(0)

#define RS485_RXDMA_CH                       DMA1_Channel5
#define RS485_RXDMA_INT                      DMA1_Channel5_IRQn
#define RS485_RxDMAIRQHandler                DMA1_Channel5_IRQHandler
#define RS485_RXDMA_TC_FLAG                  DMA1_FLAG_TC5
#define Rs485_EnableRxDma()                  do{\
                                                (RS485_DMA)->IFCR = RS485_RXDMA_TC_FLAG; \
                                                (RS485_RXDMA_CH)->CNDTR = RS485_RX_FRAME_LEN; \
                                                (RS485_RXDMA_CH)->CCR |= CCR_ENABLE_Set; \
                                            }while(0)
                                              
#define Rs485_GetRxLen()                     (RS485_RX_FRAME_LEN - (RS485_RXDMA_CH)->CNDTR)  

#define Rs485_DisableRxDma()                 do{\
                                                (RS485_DMA)->IFCR = RS485_RXDMA_TC_FLAG;\
                                                (RS485_RXDMA_CH)->CCR &= CCR_ENABLE_Reset;\
                                            }while(0)

void Rs485_Delayms(u32 n);
void Rs485_InitInterface(u32 baudrate);
void Rs485_ConfigInt(void);
void Rs485_EnableInt(FunctionalState rxState, FunctionalState txState);
void Rs485_WriteByte(u8 ch);
void Rs485_WriteBuffer(u8 *pFrame, u16 len);
void Rs485_WriteWarStr(char *str);
void Rs485_InitFloating();

void Rs485_InitTxDma(u8 *pRxBuffer, u32 len);
void Rs485_InitRxDma(u8 *pRxBuffer, u32 len);
#endif



