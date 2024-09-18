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

#define DEVICE_SERVER_AT_OP_TIME			120
#define DEVICE_SERVER_AT_PAYLOAD_TIME		200 * 20
#define DEVICE_SERVER_AT_REPAT_TICK			20
#define DEVICE_SERVER_PAYLOAD_REPAT_TICK    3
#define DEVICE_SERVER_DELAY_TIME			5

#define DEVICE_SERVER_PRIORITY_SUB          1
#define DEVICE_SERVER_PRIORITY_PUB          0

#define DEVICE_SERVER_RESULT_FAIL		    0
#define DEVICE_SERVER_RESULT_OK			    1

#define DEVICE_GET_REQUEST_STR_LEN          8
#define DEVICE_GET_MATT_STAT_STR_LEN        12


#define DEVICE_SERVER_STAT_IDLE			    0x00000000
#define DEVICE_SERVER_STAT_AT_TX		    0x00000001
#define DEVICE_SERVER_STAT_AT_WAIT		    0x00000002
#define DEVICE_SERVER_STAT_AT_RX		    0x00000004
#define DEVICE_SERVER_STAT_AT_OVER		    0x00000008
#define DEVICE_SERVER_STAT_PAYLOAD_TX	    0x00000010
#define DEVICE_SERVER_STAT_PAYLOAD_WAIT	    0x00000020
#define DEVICE_SERVER_STAT_PAYLOAD_RX	    0x00000040
#define DEVICE_SERVER_STAT_PAYLOAD_OVER	    0x00000080
#define DEVICE_SERVER_STAT_STEP     	    0x00000100
#define DEVICE_SERVER_STAT_OP       	    0x00000200
#define DEVICE_SERVER_STAT_DELAY       	    0x00000400

#define DEVICE_NET_EVENT_HEART          0x00
#define DEVICE_NET_EVENT_THING          0x01

#define DEVICE_DTU_SUBDEVICE            0x01

typedef struct deviceNetInfo{
    u8 dtu;
    u8 event;                       //事件类型
    u8 waterIm;                     //水浸
    u8 manCover;                    //井盖
    u8 angel;                       //开启角度
    u8 bird;
    u8 eventResult;
    u8 status;
	u8 result;
	u8 sigNum;
    u8 channelErrNum;
	u8 priority;
	u32 state;
	u32 time;
	u32 delayTick;
	u32 repat;
    u32 heartTime;
    u32 wakeTime;
}DEVICE_NETINFO;

#define DEVICE_DETA_MIN_FRAME_LEN       10
#define DEVICE_DETA_STR_1BYTE_LEN		2
#define DEVICE_DETA_FRAME_MIN_LEN       4
#define DEVICE_DETA_PAYLOADID_LEN		8
#define DEVICE_DETA_REQUESTID_LEN		36

#define DEVICE_DETA_AT_FRAME_LEN		256
#define DEVICE_DETA_FRAME_LEN			256
#define DEVICE_DATA_STR_MAX_LEN         256
#define DEVICE_DATE_PARAMS_LEN          16

#define DEVICE_NET_CMD_NULL             0x00
#define DEVICE_NET_CMD_SET_PROPERTY     0x01
#define DEVICE_NET_CMD_GET_PROPERTY     0x02
#define DEVICE_NET_CMD_OTA_INFORM       0x03

typedef struct deviceNetRcvInfo{
	u8 cmd;
	u8 addr;
    u16 code;
    u16 lenth;
    char id[DEVICE_DATE_PARAMS_LEN];
    char version[DEVICE_DATE_PARAMS_LEN];
    char msg[DEVICE_DETA_FRAME_LEN];
	u8 requestBuffer[DEVICE_DETA_FRAME_LEN];
	u8 rspBuffer[DEVICE_DETA_FRAME_LEN];
}DEVICE_NETRCVINFO;

extern DEVICE_NETRCVINFO g_sDeviceNetRcvInfo;

typedef struct deviceDataFrame{
	u8 cmd;
	u16 lenth;
	u8 requestId[DEVICE_DETA_REQUESTID_LEN];
	char atFrame[DEVICE_DETA_AT_FRAME_LEN];	            
	char payloadframe[DEVICE_DETA_FRAME_LEN];
}DEVICE_DATEFRAME;


#define DEVICE_DATA_SUB_LEN					1               //命令数据下发遵循一问一答模式，一般情况缓存只有一条
typedef struct deviceDataInfoSub{
	u8 num;
	u8 flag;
	u8 index;
	DEVICE_DATEFRAME data[DEVICE_DATA_SUB_LEN];
}DEVICE_DATASUBINFO;

extern DEVICE_DATASUBINFO g_sDeviceInfoSub;

#define DEVICE_DATA_PUB_LEN					2

typedef struct deviceDataInfoPub{
	u8 num;
	u8 flag;
	u8 index;
	DEVICE_DATEFRAME data[DEVICE_DATA_PUB_LEN];
}DEVICE_DATAPUBINFO;

extern DEVICE_DATAPUBINFO g_sDeviceInfoPub;

#define Device_ClearSubInfo()       do{memset(&g_sDeviceInfoSub, 0, sizeof(DEVICE_DATASUBINFO));}while(0)
#define Device_ClearPubInfo()       do{memset(&g_sDeviceInfoPub, 0, sizeof(DEVICE_DATAPUBINFO));}while(0)

#define Device_ServerAddData(pData,step,mode)       do{\
															if(mode == DEVICE_SERVER_PRIORITY_PUB)\
															{\
																pData->cmd = step;\
																memcpy(pData->atFrame, g_aMqttAtBuf, DEVICE_DETA_AT_FRAME_LEN);\
																memcpy(pData->payloadframe, g_aMqttBuf, DEVICE_DETA_FRAME_LEN);\
																if(g_sDeviceInfoPub.num < (DEVICE_DATA_PUB_LEN - 1))\
																{\
																	memcpy(&(g_sDeviceInfoPub.data[g_sDeviceInfoPub.num++]), pData, sizeof(DEVICE_DATEFRAME));\
																}\
															}\
															else \
															{\
																pData->cmd = step;\
																memcpy(pData->atFrame, g_aMqttAtBuf, DEVICE_DETA_AT_FRAME_LEN);\
																memcpy(pData->payloadframe, g_aMqttBuf, DEVICE_DETA_FRAME_LEN);\
																if(g_sDeviceInfoSub.num < (DEVICE_DATA_SUB_LEN - 1))\
																{\
																	memcpy(&(g_sDeviceInfoSub.data[g_sDeviceInfoSub.num++]), pData, sizeof(DEVICE_DATEFRAME));\
																}\
															}\
                                                        }while(0)

                                                            
#define DEVICE_MATT_SERVER_TX_AT                0x00
#define DEVICE_MATT_SERVER_TX_PAYLOAD           0x01
                                                            
#define Device_ServerTxFrame(p, pFame, lenth, nextState, mode)			do{\
                                                                        (p)->time = g_nSysTick;\
                                                                        (p)->state = nextState;\
                                                                        (p)->result = DEVICE_SERVER_RESULT_FAIL;\
                                                                        if(mode == DEVICE_MATT_SERVER_TX_AT)\
                                                                        {\
                                                                            LTE_WriteCmd(pFame);\
                                                                        }\
                                                                        else if(mode == DEVICE_MATT_SERVER_TX_PAYLOAD)\
                                                                        {\
                                                                            LTE_WriteBuffer((u8 *)pFame, lenth);\
                                                                        }\
                                                                      }while(0)
                                                        
#define Device_ServerNetClear(p,nextState)			do{\
														(p)->repat = 0;\
														(p)->time = g_nSysTick;\
														(p)->state = nextState;\
														(p)->result = DEVICE_SERVER_RESULT_OK;\
													  }while(0)

#define DEVICE_MQTT_RSP_OK                                      200
#define DEVICE_MQTT_RSP_ERR                                     0
                                                        
#define DEVICE_COM_485                  0x01
#define DEVICE_COM_LTE                  0x02        
#define DEVICE_COM_UART                  0x04 
                                                      
#define DEVICE_RESPONSE_NOERR           0x00
#define DEVICE_RESPONSE_ERR_LEN         0x01
#define DEVICE_RESPONSE_ERR_FREAM       0x02
#define DEVICE_RESPONSE_ERR_DEVICE      0x03                                                    
                                                        
//#define DEVICE_MQTT_TYPE_EVENT_REPORT                    1
//#define DEVICE_MQTT_TYPE_PROPERTY_REPORT                 2      
//#define DEVICE_MQTT_TYPE_SET_PROPERTY_REPLY              3
//#define DEVICE_MQTT_TYPE_GET_PROPERTY_REPLY              4   
#define DEVICE_MQTT_TYPE_HEART_REPORT                    5
#define DEVICE_MQTT_TYPE_DATA_REPORT                     6
#define DEVICE_MQTT_TYPE_CMD_REPORT                      7
#define DEVICE_MQTT_TYPE_OTA_REPORT                      8
                                                        
#define DEVICE_STR_INTERVAL_SET_PRO         "thing/property/set"
#define DEVICE_STR_INTERVAL_GET_PRO         "thing/property/get"
#define DEVICE_STR_INTERVAL_OTA_INFORM      "ota/inform"
#define DEVICE_STR_INTERVAL_CMD             "Cmd\":\""
#define DEVICE_STR_INTERVAL_FRAME_END       "\"}}\""
#define DEVICE_STR_INTERVAL_OTA_FRAME_END   "\"}\""
#define DEVICE_STR_INTERVAL_ID              "id\":\""
#define DEVICE_STR_INTERVAL_ITEM_END        "\",\""
#define DEVICE_STR_INTERVAL_VERSION         "version\":\""
                                                            
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

#define DEVICE_FRAME_CMD_HEART              0x21
#define DEVICE_FRAME_CMD_RFID               0x22
#define DEVICE_FRAME_CMD_BIRD_DM            0x23
#define DEVICE_FRAME_CMD_RFID_PWR_CTR       0x24
#define DEVICE_FRAME_CMD_SET_RFID_ADDR      0x25
#define DEVICE_FRAME_CMD_GET_MODE           0x26
#define DEVICE_FRAME_CMD_SET_MODE           0x27
#define DEVICE_FRAME_CMD_RST                0x04
#define DEVICE_FRAME_CMD_SET_LOG_PARAMS     0x30
#define DEVICE_FRAME_CMD_SET_PARAMS         0xF4
#define DEVICE_FRAME_CMD_GET_PARAMS         0xF5
#define DEVICE_FRAME_CMD_VER                0xF7
#define DEVICE_FRAME_CMD_GETCPUID           0xF8

extern DEVICE_PARAMS g_sDeviceParams;
extern DEVICE_RSPFRAME g_sDeviceRspFrame;

void Device_ServerComOkCallBack(void *pParms);
void Device_ServerProcessSubInfo(DEVICE_NETRCVINFO *pRcvInfo);

BOOL Device_CheckSubInfo(DEVICE_NETRCVINFO *pRcvInfo, u8 *pRxBuf);
BOOL Device_FomatAddData(u8 type);
BOOL Device_CheckPubInfo(DEVICE_NETRCVINFO *pRcvInfo, u8 *pRxBuf);
BOOL Device_CheckAtRsp(u8 type, u8 *pRxBuf);

u16 Device_ProcessUartFrame(u8 *pFrame, u16 len, u8 com);
u16 Device_ResponseFrame(u8 *pParam, u16 len, DEVICE_RSPFRAME *pRspFrame);
u16 Device_HeartFormatFrame(DEVICE_RSPFRAME *pRspFrame);
u16 Device_Str2Hex(char *pStr, u8 *pHexBuf, u16 len);



extern TaskHandle_t g_hDevicePMqtt;
void Device_ProcessMqtt(void *p);

#endif