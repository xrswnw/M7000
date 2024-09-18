#ifndef _LTE_H_
#define _LTE_H_

#include "LTE_HL.h"
#include "Uart_Receive.h"

extern UART_RCVFRAME g_sLTERcvFrame;

#define LTE_TX_MAX_LENTH                    256

#define LTE_ClearRxBuffer()                    memset(&g_sLTERcvFrame, 0,  sizeof(UART_RCVFRAME)) //这里一定要用memset

#define TESTTOKEN								"version=2018-10-31&res=products%2FmvKLMzGpI4&et=2039307162&method=md5&sign=ztZFHCgmoPHMM50%2BNTgiow%3D%3D"
#define LTE_PRDOCT_ID                          "mvKLMzGpI4"                                                                           //产品ID


#define LTE_IP_LEN                             4

#define LTE_PAR_ADDR_MOD_IP                    0
#define LTE_PAR_ADDR_MOD_URL                   1
#define LTE_PAR_ADDR_URL_LEN                   64

#define LTE_MQTT_TOPIC_HEART                   1
#define LTE_MQTT_TOPIC_REBAT                   2
#define LTE_MQTT_TOPIC_BWBAT                   3
#define LTE_MQTT_TOPIC_WARN                    4
#define LTE_MQTT_TOPIC_CMD                     5
#define LTE_MQTT_TOPIC_ERR                     6       
#define LTE_MQTT_TOPIC_OPERIPH                  8
#define LTE_RSP_JSON                           0x01
#define LTE_RSP_CMD                            0x02

// 2 + 2 + 2 + 64 + 1
typedef struct w232Par{
    u8 ip[LTE_IP_LEN];
    u16 port;
    char url[LTE_PAR_ADDR_URL_LEN + 1];
    u8 mode;
}LTE_PARAMS;

#define LTE_CNT_CMD_PWROFF                     0
#define LTE_CNT_CMD_PWRON                      1

#define LTE_CNT_OP_PWROFF                      0
#define LTE_CNT_OP_PWRON                       1
#define LTE_CNT_OP_KEYON                       2
#define LTE_CNT_OP_KEYOFF                      3
#define LTE_CNT_OP_CPIN                        4
#define LTE_CNT_OP_CREG                        5
#define LTE_CNT_OP_CGREG                       6
#define LTE_CNT_OP_APN                         7
#define LTE_CNT_OP_DEACT                       8
#define LTE_CNT_OP_ACT                         9
#define LTE_CNT_OP_OPEN                        10
#define LTE_CNT_OP_COMM                        11  //开机后，串口需要10s才能响应命令
#define LTE_CNT_OP_ATE0                        12
#define LTE_CNT_OP_IMEI                        13
#define LTE_CNT_OP_CCLK                        14
#define LTE_CNT_OP_WAIT                        15
#define LTE_CNT_OP_CLOSE                       16
#define LTE_CNT_OP_DTUOFF                      17
#define LTE_CNT_OP_DTR1                        18
#define LTE_CNT_OP_NTP                         19   //ntp同步系统时间，必须在联网之前操作
#define LTE_CNT_OP_CSQ                         20      
#define LTE_CNT_OP_QMTCFG                      21
#define LTE_CNT_OP_QMTOPEN                     22
#define LTE_CNT_OP_QMTCLOSE                    23
#define LTE_CNT_OP_QMTCONN                     24
#define LTE_CNT_OP_QMTSUB                      25
#define LTE_CNT_OP_QMTPUBEX                    26
#define LTE_CNT_OP_IMSI                        27
#define LTE_CNT_OP_QMTSUB_CMD                  28
#define LTE_CNT_OP_QMTSUB_JSON_ACCEPT          29
#define LTE_CNT_OP_QMTSUB_JSON_REJECT          30
#define LTE_CNT_OP_QMTCFG_VERSION              31       
#define LTE_CNT_OP_QMTCFG_TIME                 32
#define LTE_CNT_OP_QMTSUB_RESPONSE_CMD         33
#define LTE_CNT_OP_QMTSUB_OTA                  34
#define LTE_CNT_OP_GET_QSQ                     35
#define LTE_CNT_OP_OPEN_DEBUG                  36

#define LTE_CNT_OP_SUBSCRIBE_PROPERTY_POST_REPLY      37              //设备属性上报响应
//
#define LTE_CNT_OP_SUBSCRIBE_EVENT_POST_REPLY         38              //设备事件上报响应
//
#define LTE_CNT_OP_SUBSCRIBE_PROPERTY_SET             41                    //设置直连设备属性
#define LTE_CNT_OP_SUBSCRIBE_PROPERTY_SET_REPLY       42                   //设置直连设备属性响应
#define LTE_CNT_OP_SUBSCRIBE_PROPERTY_GET             43                   //获取设备属性
 
//
#define LTE_CNT_OP_SUBSCRIBE_OTA_INFORM             44                 //设备OTA

#define LTE_CNT_OP_STAT_IDLE                   0x0001
#define LTE_CNT_OP_STAT_TX                     0x0002
#define LTE_CNT_OP_STAT_RX                     0x0004
#define LTE_CNT_OP_STAT_STEP                   0x0008
#define LTE_CNT_OP_STAT_WAIT                   0x0020
#define LTE_CNT_OP_STAT_DTU                    0x0040
//#define LTE_CNT_OP_STAT_TEST                   0x0080
#define LTE_CNT_OP_STAT_TX_AT                  0x0100
#define LTE_CNT_OP_STAT_RX_AT                  0x0200
#define LTE_CNT_OP_STAT_ERR                    0x0400

#define LTE_CNT_TIME_1S                        200
#define LTE_CNT_TIME_500MS                    100
#define LTE_CNT_TIME_100MS                     20

#define LTE_CNT_TIME_20S                       4000

#define LTE_CNT_REPAT_NULL                     1
#define LTE_CNT_REPAT_THREE                    3
#define LTE_CNT_OP_NUM                         32
#define LTE_CNT_TX_BUF_NUM						100
#define LTE_CNT_RESULT_OK                      0
#define LTE_CNT_RESULT_FAIL                    1
#define LTE_CNT_RESULT_TO                      2

#define LTE_RESPONES_NULL                      0x00
#define LTE_RESPONES_CMD_RESP                  0x02
#define LTE_RESPONES_CMD_GET                   0x01
#define LTE_RESPONES_CMD_RESP                  0x02
#define LTE_RESPONES_JSON_ACCEPT               0x04
#define LTE_RESPONES_JSON_REJECT               0x08

//

#define LTE_MQTT_STAT_OK						0xFF
#define LTE_MQTT_STAT_CONNECTING				0x00				//连接中
#define LTE_MQTT_STAT_S_DISCONNECT				0x01				//服务器主动断开MQTT连接
#define LTE_MQTT_STAT_OP_PING					0x02				//PING超时，应反激活PDP、重建MQTT连接
#define LTE_MQTT_STAT_OP_CONNECT				0x03				//发送连接超时，可能密码错误、或者被占用
#define LTE_MQTT_STAT_OP_CONNACK				0x04				//接收连接超时，可能密码错误、或者被占用
#define LTE_MQTT_STAT_C_DISCONNECT				0x05				//客户端主动发起disconnect包,客户端主动断开MQTT连接
#define LTE_MQTT_STAT_C_DATA_DISCONNECT		    0x06				//客户端发送数据多次错误，客户端主动断开MQTT连接。检查数据包格式
#define LTE_MQTT_STAT_S_FAIL					0x07				//链路异常或服务器不可用
#define LTE_MQTT_STAT_C_DISCONNECTS			    0x08				//客户端主动断开MQTT连接


#define LTE_CNT_MODE_CONNECT                   1
#define LTE_CNT_MODE_DISCONCT                  0
#define LTE_IMEI_LEN                           15
#define LTE_IMSI_LEN                           15
#define LTE_RTC_LEN                            6

#define LTE_RTC_DATA_ID_POS                    6
#define LTE_RTC_DATA_INDEX_POS                 37
#define LTE_RTC_DATA_CMD_POS                   39
#define LTE_RTC_DATA_LEN_POS                   41
#define LTE_RTC_DATA_BUFFER_POS                45

#define LTE_CNT_INDEX_LEN                      2
//#define LTE_TOKEN_LEN                          103//97//99
#define LTE_CNT_IMEI_SRT_LEN					8

#define LTE_HEART_OFFLINE_TIME                 4

#define LTE_SERVER_TX_TO_TIME                  100

#define LTE_STATUS_IDLE                         0x00
#define LTE_STATUS_INIT                         0x01
#define LTE_STATUS_PWR_OFF                      0x02
#define LTE_STATUS_CONNECT_OK                   0x04
#define LTE_STATUS_INIT_ERR                     0x80

typedef struct w232Connect{
    LTE_PARAMS params;
    u8 mode;
    u8 imei[LTE_IMEI_LEN + 2];
    u8 imeiStr[LTE_IMEI_LEN + 2];
    u8 imsi[LTE_IMSI_LEN + 2];
    u8 imSiStr[LTE_IMSI_LEN + 2];
    u8 rtc[LTE_RTC_LEN];
    u8 op[LTE_CNT_OP_NUM];
    u32 to[LTE_CNT_OP_NUM];
    u8 repeat[LTE_CNT_OP_NUM];
    u8 sigStr[2];
    u8 channelErrStr[2];
    u8 num;
    u8 index;
    u8 sigNum;
    u8 channelErrNum;
    u8 cmd;
    u16 state;
    u32 tick;
    u8 result;
}LTE_CONNECT;
extern LTE_CONNECT g_sLTEConnect;


#define LTE_CheckRtcOk(p)                      ((p)[0] >= 21)

extern u8 g_nImei[LTE_IMEI_LEN + 2];
extern u8 g_nImeiStr[LTE_IMEI_LEN + 2];
extern u8 g_nLTEStatus;

extern TaskHandle_t g_hLTEInit;

BOOL LTE_SendBuffer( u8 *pRxBuf);
BOOL LTE_ConnectCheckRsp(LTE_CONNECT *pCntOp, u8 *pRxBuf);
BOOL LTE_ConnectCheckClose(u8 *pBuffer);
BOOL LTE_ConnectGetCClk(u8 *pRxBuf, u8 len, u8 *pRtc);
BOOL LTECheckRsp(u8 *pRxBuf);
BOOL LTE_CheckRsp(LTE_CONNECT *pCntOp, u8 *pRxBuf);
BOOL LTE_FormatRsp(LTE_CONNECT *pCntOp);
u16 LTE_ConnectTxCmd(LTE_CONNECT *pCntOp, u8 *pFrame);
void LTE_Init();
void LTE_ConnectInit(LTE_CONNECT *pCntOp, u8 cmd, LTE_PARAMS *pParams);
void LTE_ConnectStep(LTE_CONNECT *pCntOp);
void LTE_PostRsp(LTE_CONNECT *pCntOp,u8 *pBuffer, u16 len);
void LTE_PostRtBatRsp(LTE_CONNECT *pCntOp,u8 *pBuffer, u8 addr, u32 id);
void LTE_PostBwBatRsp(LTE_CONNECT *pCntOp,u8 *pBuffer, u8 addr, u32 id);
void LTE_InitTask(void *p);
//void LTE_ConnectTxCmd(LTE_CONNECT *pCntOp, u32 sysTick);
#endif

