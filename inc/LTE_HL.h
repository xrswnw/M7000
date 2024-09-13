#ifndef _LTE_HL_
#define _LTE_HL_

#include "Config.h"

#define LTE_PORT                       USART3
#define LTE_BAUDRARE                   115200
#define LTE_IRQHandler                 USART3_IRQHandler
#define LTE_DR_ADDRESS                 (LTE_PORT->DR)



extern const PORT_INF LTE_PORT_BOOT_CTR;
#define LTE_BootHigh()          LTE_PORT_BOOT_CTR.Port->BSRR = LTE_PORT_BOOT_CTR.Pin
#define LTE_BootLow()           LTE_PORT_BOOT_CTR.Port->BRR = LTE_PORT_BOOT_CTR.Pin

extern const PORT_INF LTE_PORT_KEY;
#define LTE_KeyHigh()          LTE_PORT_KEY.Port->BSRR = LTE_PORT_KEY.Pin
#define LTE_KeyLow()           LTE_PORT_KEY.Port->BRR = LTE_PORT_KEY.Pin

extern const PORT_INF LTE_PORT_RST;
#define LTE_CtrlHigh()         LTE_PORT_RST.Port->BSRR = LTE_PORT_RST.Pin
#define LTE_CtrlLow()          LTE_PORT_RST.Port->BRR = LTE_PORT_RST.Pin

#define LTE_ReadByte()                 ((u16)(LTE_PORT->DR & (u16)0x01FF))

void LTE_Delayms(u32 n);
#define LTE_Rest()          do{LTE_KeyHigh();\
                                LTE_CtrlHigh();\
                                LTE_Delayms(700);\
                                LTE_CtrlLow();\
                                LTE_KeyLow();}while(0)

void LTE_InitInterface(u32 baudrate);
void LTE_ConfigInt(void);
void LTE_ConfigDma(u8 *pTxBuffer, u32 size);
void LTE_EnableInt(FunctionalState rxState, FunctionalState txState);

void LTE_WriteByte(u8 ch);

void LTE_WriteBuffer(u8 *pFrame, u16 len);
void LTE_WriteCmd(char *str);

#endif

