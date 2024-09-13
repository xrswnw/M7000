#ifndef _SYSCFG_H_
#define _SYSCFG_H_

#include "Device.h"

extern u32 g_nSysState;
#define SYS_NULL_TICK                   0

#define SYS_STAT_IDLE                   0x00000001	        //
#define SYS_STAT_RUNLED                 0x00000002
#define SYS_STAT_ALARMLED               0x00000004
#define SYS_STAT_ALARMDLY				0x00000008
#define SYS_STAT_LTE_INIT_INIT          0x00000010
#define SYS_STAT_LTE_LINK_OK            0x00000020
#define SYS_STAT_LTE_INIT_ERR           0x00000040
#define SYS_STAT_SLAVE_TX               0x00000100 
#define SYS_STAT_SI24R1_CHK             0x00000200
#define SYS_STAT_SI24R1_TX              0x00000400
#define SYS_STAT_AD                     0x00000800
#define SYS_STAT_LOG_TX                 0x00001000 
#define SYS_STAT_LTE_START_PWR_OFF      0x00002000
#define SYS_STAT_LTE_PWR_OFF            0x00004000


#define SYS_STAT_WEAK                   0x20000000
#define SYS_STAT_PRE_SLEEP              0x40000000
#define SYS_STAT_SLEEP                  0x80000000
#define SYS_STAT_PRE_WEAK               0x08000000

#define SYS_SYSTICK_ONE_SECOND          200 * 1
#define SYS_SYSTICK_ONE_MINUET          SYS_SYSTICK_ONE_SECOND * 58         //Êµ¼ÊÓÐÆ«²î£¬


void Sys_Delayms(u32 n);
void Sys_CfgClock(void);
void Sys_CfgPeriphClk(FunctionalState state);
void Sys_CfgNVIC(void);
void Sys_CtrlIOInit(void);
void Sys_Init(void);
void Sys_TaskCreat();





extern TaskHandle_t g_hSysProcessRs485;
void Sys_Rs485ProcessTask(void *p);

void Sys_RunTime(void *p);
#endif
