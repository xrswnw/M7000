#ifndef _ANYID_DEVICE_
#define _ANYID_DEVICE_

#include "Uart_Receive.h"
#include "Rs485.h"
#include "LTE.h"

#define SLAVE_OP_MAX_BLOCK_NUM      2
typedef struct deviceParamenter{
	u8 addr;
    u8 mode;
    u8 debug;
    u8 dtu;
	u8 bud;
    LTE_PARAMS lteParams;
    
    u8 subDeviceNum;
    u16 heartPeriod; 
    u16 subDeviceOpPeriod;
    u8 subDeviceOpNum;
    u8 subDeviceOpAddr[SLAVE_OP_MAX_BLOCK_NUM];
    
    u16 wakeTime;
    u16 sleepTime;
    
	u32 rfu1;
    u32 rfu2;
	u32 crc;
}DEVICE_PARAMS;


#define DEVICE_RSPFRAME_LEN                 UART_BUFFER_MAX_LEN
typedef struct deviceRspFrame{
    u16 len;
    u8 buffer[DEVICE_RSPFRAME_LEN];
    u8 flag;
    u8 err;
    u16 destAddr;
    u8 cmd;
    u8 comType;
}DEVICE_RSPFRAME;

#define DEVICE_VERSION_SIZE                 50

#define DEVICE_FRAME_CMD_RST                0x04
#define DEVICE_FRAME_CMD_VER                0xF7


extern DEVICE_PARAMS g_sDeviceParams;
extern DEVICE_RSPFRAME g_sDeviceRspFrame;


u16 Device_ProcessUartFrame(u8 *pFrame, u16 len, u8 com);

u16 Device_ResponseFrame(u8 *pParam, u16 len, DEVICE_RSPFRAME *pRspFrame);
#endif