#include "LTE.h"

UART_RCVFRAME g_sLTERcvFrame = {0};
LTE_CONNECT g_sLTEConnect = {0};
u8 g_nImei[LTE_IMEI_LEN + 2] = {0};
u8 g_nImeiStr[LTE_IMEI_LEN + 2] = {0};
u8 g_nLTEStatus = 0;

void LTE_Init()
{
    LTE_InitInterface(LTE_BAUDRARE);
    LTE_ConfigInt();
    LTE_EnableInt(ENABLE, DISABLE);
    
    g_nLTEStatus = LTE_STATUS_IDLE;
}

void LTE_ConnectInit(LTE_CONNECT *pCntOp, u8 cmd, LTE_PARAMS *pParams)
{
    u8 num = 0;

    memset(pCntOp, 0, sizeof(LTE_CONNECT));
    if(cmd == LTE_CNT_CMD_PWRON)
    {
        pCntOp->to[num] = LTE_CNT_TIME_100MS* 15;       pCntOp->repeat[num] = 1;    pCntOp->op[num++] = LTE_CNT_OP_PWRON;          //寮�鍚數婧?0ms
        pCntOp->to[num] = LTE_CNT_TIME_100MS * 15;      pCntOp->repeat[num] = 1;    pCntOp->op[num++] = LTE_CNT_OP_KEYON;          //寮�鏈?=500ms
        pCntOp->to[num] = LTE_CNT_TIME_500MS * 2;       pCntOp->repeat[num] = 25;   pCntOp->op[num++] = LTE_CNT_OP_COMM;           //寮�鏈?0s锛屼覆鍙ｆ湁鍝嶅簲
        pCntOp->to[num] = LTE_CNT_TIME_500MS  * 2;      pCntOp->repeat[num] = 2;    pCntOp->op[num++] = LTE_CNT_OP_ATE0;

        pCntOp->to[num] = LTE_CNT_TIME_500MS * 2;       pCntOp->repeat[num] = 10;    pCntOp->op[num++] = LTE_CNT_OP_IMEI;
        pCntOp->to[num] = LTE_CNT_TIME_500MS * 2;       pCntOp->repeat[num] = 2;    pCntOp->op[num++] = LTE_CNT_OP_DTR1;        
        pCntOp->to[num] = LTE_CNT_TIME_500MS * 2;       pCntOp->repeat[num] = 20;   pCntOp->op[num++] = LTE_CNT_OP_CPIN;           //鏌ヨ鎵嬫満鍗?=20s
        pCntOp->to[num] = LTE_CNT_TIME_500MS * 2;       pCntOp->repeat[num] = 80;  pCntOp->op[num++] = LTE_CNT_OP_CREG;           //鏌ヨ鏄惁娉ㄥ唽缃戠粶>=90s
        pCntOp->to[num] = LTE_CNT_TIME_500MS * 2;       pCntOp->repeat[num] = 80;  pCntOp->op[num++] = LTE_CNT_OP_CGREG;          //鏌ヨ鏄惁娉ㄥ唽缃戠粶>=60s
        pCntOp->to[num] = LTE_CNT_TIME_1S * 4;          pCntOp->repeat[num] = 2;    pCntOp->op[num++] = LTE_CNT_OP_DEACT;          //40s
        pCntOp->to[num] = LTE_CNT_TIME_1S * 15;         pCntOp->repeat[num] = 2;    pCntOp->op[num++] = LTE_CNT_OP_ACT;            //150s
        pCntOp->to[num] = LTE_CNT_TIME_1S * 4;          pCntOp->repeat[num] = 4;    pCntOp->op[num++] = LTE_CNT_OP_QMTCFG;          //閰嶇疆MQTT鍙傛暟
        pCntOp->to[num] = LTE_CNT_TIME_1S * 4;          pCntOp->repeat[num] = 4;    pCntOp->op[num++] = LTE_CNT_OP_QMTCFG_TIME;
        pCntOp->to[num] = LTE_CNT_TIME_1S * 4 ;         pCntOp->repeat[num] = 50;    pCntOp->op[num++] = LTE_CNT_OP_QMTCFG_VERSION;
        pCntOp->to[num] = LTE_CNT_TIME_1S * 4;          pCntOp->repeat[num] = 2;    pCntOp->op[num++] = LTE_CNT_OP_QMTOPEN;             //杩炴帴缃戠粶
        pCntOp->to[num] = LTE_CNT_TIME_1S * 4;          pCntOp->repeat[num] = 5;    pCntOp->op[num++] = LTE_CNT_OP_QMTCONN;           //杩炴帴MQTT瀹㈡埛绔?
        pCntOp->to[num] = LTE_CNT_TIME_1S * 4 ;         pCntOp->repeat[num] = 20;    pCntOp->op[num++] = LTE_CNT_OP_SUBSCRIBE_PROPERTY_SET;              //璁㈤槄涓婚
        pCntOp->to[num] = LTE_CNT_TIME_1S * 4 ;         pCntOp->repeat[num] = 20;    pCntOp->op[num++] = LTE_CNT_OP_SUBSCRIBE_OTA_INFORM; 
    }
    else
    {
        pCntOp->to[num] = LTE_CNT_TIME_1S * 10;  pCntOp->repeat[num] = 5;    pCntOp->op[num++] = LTE_CNT_OP_CLOSE;
        pCntOp->to[num] = LTE_CNT_TIME_100MS * 9;  pCntOp->repeat[num] = 1;    pCntOp->op[num++] = LTE_CNT_OP_KEYOFF;          //鍏虫満>=650ms
        pCntOp->to[num] = LTE_CNT_TIME_1S * 2;  pCntOp->repeat[num] = 1;    pCntOp->op[num++] = LTE_CNT_OP_WAIT; 
        pCntOp->to[num] = LTE_CNT_TIME_100MS;  pCntOp->repeat[num] = 1;    pCntOp->op[num++] = LTE_CNT_OP_PWROFF;
    }
    pCntOp->state = LTE_CNT_OP_STAT_TX;
    pCntOp->cmd = cmd;
    pCntOp->num = num;
    memcpy(&pCntOp->params, pParams, sizeof(LTE_PARAMS));
}

u16 LTE_ConnectTxCmd(LTE_CONNECT *pCntOp, u8 *pFrame)
{
    u8 op = 0;
    op = pCntOp->op[pCntOp->index];
    switch(op)
    {
        case LTE_CNT_OP_WAIT:   //涓嶉渶瑕佸彂閫侊紝绛夊緟鍗冲彲
            break;
        case LTE_CNT_OP_PWROFF: 
            LTE_KeyHigh();
        case LTE_CNT_OP_PWRON:
            LTE_KeyLow();
            vTaskDelay(200);
            LTE_KeyHigh();
            break;
        case LTE_CNT_OP_KEYON:
        case LTE_CNT_OP_KEYOFF:
            LTE_KeyLow();
            break;
        case LTE_CNT_OP_COMM:
            sprintf((char *)pFrame, "AT");
            break;
		case LTE_CNT_OP_OPEN_DEBUG:
            sprintf((char *)pFrame, "AT+QCFG=\"DBGCTL\",0");
            break;
        case LTE_CNT_OP_ATE0:
          sprintf((char *)pFrame, "ATE0");
            break;
        case LTE_CNT_OP_GET_QSQ:
          sprintf((char *)pFrame, "AT+CSQ");
            break;
        case LTE_CNT_OP_IMEI:
          sprintf((char *)pFrame, "AT+GSN=1");
            break;
        case LTE_CNT_OP_IMSI:
            sprintf((char *)pFrame, "AT+CIMI");
            break;
        case LTE_CNT_OP_CCLK:
          sprintf((char *)pFrame, "AT+CCLK?");
            break;
        case LTE_CNT_OP_DTR1:
          sprintf((char *)pFrame, "AT&D1");
            break;
        case LTE_CNT_OP_CPIN:
          sprintf((char *)pFrame, "AT+CPIN?");
            break;
        case LTE_CNT_OP_CREG:
          sprintf((char *)pFrame, "AT+CREG?");
            break;
        case LTE_CNT_OP_CGREG:
          sprintf((char *)pFrame, "AT+CGREG?");
            break;
        case LTE_CNT_OP_APN:
          sprintf((char *)pFrame, "AT+QICSGP=1,1,\"UNINET\",\"\",\"\",1");
            break;
        case LTE_CNT_OP_DEACT:
          sprintf((char *)pFrame, "AT+QIDEACT=1");
            break;
        case LTE_CNT_OP_ACT:
          sprintf((char *)pFrame, "AT+QIACT=1");
            break;
        case LTE_CNT_OP_NTP:
          sprintf((char *)pFrame, "AT+QNTP=1,\"202.112.10.36\",123");
            break;
        case LTE_CNT_OP_CLOSE:
            vTaskDelay(10);
            sprintf((char *)pFrame, "AT+QPOWD=0");
            break;
        case LTE_CNT_OP_OPEN:
            if(pCntOp->params.mode == LTE_PAR_ADDR_MOD_IP)
            {
                sprintf((char *)pFrame, "AT+QIOPEN=1,0,\"TCP\",\"%d.%d.%d.%d\",%d,0,2", pCntOp->params.ip[0], pCntOp->params.ip[1], pCntOp->params.ip[2], pCntOp->params.ip[3], pCntOp->params.port);
            }
            else
            {
                sprintf((char *)pFrame, "AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,2", pCntOp->params.url, pCntOp->params.port);
            }
            break;
        case LTE_CNT_OP_QMTCFG:

          sprintf((char *)pFrame, "AT+QMTCFG=\"qmtping\",0,60\r\nAT+QMTCFG=\"recv/mode\",0,0,1\r\nAT+QMTCFG=\"send/mode\",0,1");
            break;
        case  LTE_CNT_OP_QMTCFG_TIME:
          sprintf((char *)pFrame,"AT+QMTCFG=\"keepalive\",0,%d", 120);
          break;
        case LTE_CNT_OP_QMTCFG_VERSION:
            sprintf((char *)pFrame,"AT+QMTCFG=\"version\",0,4");
            break;
        case LTE_CNT_OP_QMTOPEN:
            sprintf((char *)pFrame,"AT+QMTOPEN=0,\"mqtts.heclouds.com\",1883");
            break;
        case LTE_CNT_OP_QMTCONN:
            sprintf((char *)pFrame, "AT+QMTCONN=0,\"%.15s\",\"%.10s\",\"%s\"", pCntOp->imeiStr, LTE_PRDOCT_ID, TESTTOKEN);
            break;
        case LTE_CNT_OP_QMTSUB_CMD:
            sprintf((char *)pFrame,"AT+QMTSUB=0,1,\"$sys/%.10s/%.15s/cmd/request/+\",1", LTE_PRDOCT_ID, pCntOp->imeiStr);
            break;
        case LTE_CNT_OP_QMTSUB_JSON_ACCEPT:
            sprintf((char *)pFrame, "AT+QMTSUB=0,1,\"$sys/%.10s/%.15s/dp/post/json/accepted\",1", LTE_PRDOCT_ID, pCntOp->imeiStr);
            break;
        case LTE_CNT_OP_QMTSUB_JSON_REJECT:
            sprintf((char *)pFrame,"AT+QMTSUB=0,1,\"$sys/%.10s/%.15s/dp/post/json/rejected\",1", LTE_PRDOCT_ID, pCntOp->imeiStr);
            break;
        case LTE_CNT_OP_QMTSUB_RESPONSE_CMD:
          sprintf((char *)pFrame,"AT+QMTSUB=0,1,\"$sys/%.10s/%.15s/cmd/response/+/accepted\",1", LTE_PRDOCT_ID, pCntOp->imeiStr);
            break;
        case LTE_CNT_OP_QMTSUB_OTA:
            sprintf((char *)pFrame,"AT+QMTSUB=0,1,\"$sys/%.10s/%.15s/ota/inform\",1", LTE_PRDOCT_ID, pCntOp->imeiStr);
            break;
        case LTE_CNT_OP_SUBSCRIBE_PROPERTY_SET:
            sprintf((char *)pFrame, "AT+QMTSUB=0,1,\"$sys/%.10s/%.15s/thing/property/set\",1", LTE_PRDOCT_ID, pCntOp->imeiStr);
            break;
        case LTE_CNT_OP_SUBSCRIBE_PROPERTY_GET:
            sprintf((char *)pFrame,"AT+QMTSUB=0,1,\"$sys/%.10s/%.15s/thing/property/get\",1", LTE_PRDOCT_ID, pCntOp->imeiStr);
            break;
        case LTE_CNT_OP_SUBSCRIBE_OTA_INFORM:
          sprintf((char *)pFrame,"AT+QMTSUB=0,1,\"$sys/%.10s/%.15s/ota/inform\",1", LTE_PRDOCT_ID, pCntOp->imeiStr);
          break;
    }
    return strlen((char *)pFrame);
}

BOOL LTE_ConnectCheckRsp(LTE_CONNECT *pCntOp, u8 *pRxBuf)
{
    u8 op = 0;
    BOOL bOK = FALSE;
    op = pCntOp->op[pCntOp->index];
    switch(op)
    {
        case LTE_CNT_OP_WAIT:
        case LTE_CNT_OP_PWROFF:
        case LTE_CNT_OP_PWRON:
        case LTE_CNT_OP_KEYON:
        case LTE_CNT_OP_OPEN_DEBUG:
            break;
        case LTE_CNT_OP_COMM:
        case LTE_CNT_OP_ATE0:
        case LTE_CNT_OP_DEACT:
        case LTE_CNT_OP_ACT:
        case LTE_CNT_OP_APN:
        case LTE_CNT_OP_DTR1:
        case LTE_CNT_OP_CLOSE:
            if(strstr((char const *)pRxBuf, "OK") != NULL)
            {
                bOK = TRUE;
            }
            break;
        case  LTE_CNT_OP_GET_QSQ:
            if(strstr((char const *)pRxBuf, "OK") != NULL)
            {
                bOK = TRUE;
                memcpy(pCntOp->sigStr, pRxBuf + 8, 2);
                memcpy(pCntOp->channelErrStr, pRxBuf + 11, 2);
                pCntOp->sigNum = a_atoi((u8 *)pCntOp->sigStr, 2, STD_LIB_FMT_DEC);
                pCntOp->channelErrNum = a_atoi((u8 *)pCntOp->channelErrStr, 2,STD_LIB_FMT_DEC);
            }
            break;
        case LTE_CNT_OP_IMEI:
            if(strstr((char const *)pRxBuf, "OK") != NULL)
            {
                memcpy(pCntOp->imeiStr, (strstr((char const *)pRxBuf, "GSN") + 5), LTE_IMEI_LEN);
                pCntOp->imeiStr[LTE_IMEI_LEN] = 0x30;
                pCntOp->imeiStr[LTE_IMEI_LEN + 1] = '\0';
                a_Str2Hex((char *)pCntOp->imeiStr, pCntOp->imei);
                bOK = TRUE;
                memcpy(g_nImei, pCntOp->imei, LTE_IMEI_LEN);                //淇濆瓨
                memcpy(g_nImeiStr, pCntOp->imeiStr, LTE_IMEI_LEN);
            }
            break;
        case LTE_CNT_OP_IMSI:
            if(strstr((char const *)pRxBuf, "OK") != NULL)
            {
                memcpy(pCntOp->imsi, pRxBuf + 2, LTE_IMEI_LEN + 2);
                bOK = TRUE;
            }
            break;
        case LTE_CNT_OP_NTP:
            if(strstr((char const *)pRxBuf, "OK") != NULL)
            {
                if(strstr((char const *)pRxBuf, "QNTP") != NULL)
                {
                    bOK = TRUE;
                }
            }
            break;
        case LTE_CNT_OP_CCLK:
            if(strstr((char const *)pRxBuf, "OK") != NULL)  //杩欓噷杩樿鑾峰彇鏃堕棿
            {
                if(LTE_ConnectGetCClk(pRxBuf, 28, pCntOp->rtc) == FALSE)
                {
                    memset(pCntOp->rtc, 0, LTE_RTC_LEN);
                }
                bOK = TRUE;
            }
            break;
        case LTE_CNT_OP_CPIN:
            if(strstr((char const *)pRxBuf, "READY") != NULL)
            {
                bOK = TRUE;
            }
            break;
        case LTE_CNT_OP_CREG:
            if(strstr((char const *)pRxBuf, "+CREG") != NULL)
            {
                char *p = strstr((char const *)pRxBuf, ",");
                if(p[1] == '1' || p[1] == '5')
                {
                    bOK = TRUE;
                }
            }
            break;
        case LTE_CNT_OP_CGREG:
            if(strstr((char const *)pRxBuf, "+CGREG") != NULL)
            {
                char *p = strstr((char const *)pRxBuf, ",");
                if(p[1] == '1' || p[1] == '5')
                {
                    bOK = TRUE;
                }
            }
            break;
        case LTE_CNT_OP_OPEN:
        case LTE_CNT_OP_QMTCFG:
        case LTE_CNT_OP_QMTCFG_TIME:
        case LTE_CNT_OP_QMTCFG_VERSION:
        case LTE_CNT_OP_QMTOPEN:
        case LTE_CNT_OP_QMTCONN:
        case LTE_CNT_OP_QMTSUB:
        case LTE_CNT_OP_QMTSUB_CMD:
        case LTE_CNT_OP_QMTSUB_JSON_ACCEPT:
        case LTE_CNT_OP_QMTSUB_JSON_REJECT:
        case LTE_CNT_OP_QMTSUB_RESPONSE_CMD:
        case LTE_CNT_OP_QMTSUB_OTA:
        case LTE_CNT_OP_SUBSCRIBE_PROPERTY_SET:
        case LTE_CNT_OP_SUBSCRIBE_PROPERTY_GET:
        case LTE_CNT_OP_SUBSCRIBE_OTA_INFORM:
            if(strstr((char const *)pRxBuf, "OK") != NULL)
            {
                bOK = TRUE;
            }
            break;
    }
    return bOK;
}


void LTE_ConnectStep(LTE_CONNECT *pCntOp)
{
    u8 op = 0;

    op = pCntOp->op[pCntOp->index];
    switch(op)
    {
        case LTE_CNT_OP_KEYON:
        case LTE_CNT_OP_KEYOFF:
            //LTE_KeyLow();
            pCntOp->repeat[pCntOp->index] = 0;
            pCntOp->index++;
            pCntOp->result = LTE_CNT_RESULT_OK;
            break;
        case LTE_CNT_OP_WAIT:
        case LTE_CNT_OP_PWROFF:
        case LTE_CNT_OP_PWRON:
        case LTE_CNT_OP_CLOSE:
        case LTE_CNT_OP_OPEN_DEBUG:
            pCntOp->repeat[pCntOp->index] = 0;
            pCntOp->index++;
            pCntOp->result = LTE_CNT_RESULT_OK;
            break;
        case LTE_CNT_OP_COMM:
        case LTE_CNT_OP_ATE0:
        case LTE_CNT_OP_DEACT:
        case LTE_CNT_OP_ACT:
        case LTE_CNT_OP_APN:
        case LTE_CNT_OP_GET_QSQ:
        case LTE_CNT_OP_IMEI:
        case LTE_CNT_OP_IMSI:
        case LTE_CNT_OP_CCLK:
        case LTE_CNT_OP_CPIN:
        case LTE_CNT_OP_CREG:
        case LTE_CNT_OP_CGREG:
        case LTE_CNT_OP_OPEN:
        case LTE_CNT_OP_DTR1:
        case LTE_CNT_OP_NTP:
        case LTE_CNT_OP_QMTCFG:
        case LTE_CNT_OP_QMTCFG_VERSION:
        case LTE_CNT_OP_QMTCFG_TIME:
        case LTE_CNT_OP_QMTOPEN:
        case LTE_CNT_OP_QMTCONN:
        case LTE_CNT_OP_QMTSUB:
        case LTE_CNT_OP_QMTSUB_CMD:
        case LTE_CNT_OP_QMTSUB_JSON_ACCEPT:
        case LTE_CNT_OP_QMTSUB_JSON_REJECT:
        case LTE_CNT_OP_QMTSUB_RESPONSE_CMD:
        case LTE_CNT_OP_QMTSUB_OTA:
        case LTE_CNT_OP_SUBSCRIBE_PROPERTY_SET:
        case LTE_CNT_OP_SUBSCRIBE_PROPERTY_GET:  
        case LTE_CNT_OP_SUBSCRIBE_OTA_INFORM:
            if(pCntOp->result == LTE_CNT_RESULT_OK)
            {
                pCntOp->repeat[pCntOp->index] = 0;
                pCntOp->index++;
            }
            break;
    }
}

BOOL LTE_ConnectCheckClose(u8 *pBuffer)
{
    if((strstr((char const *)pBuffer, "closed\r\n") != NULL) || (strstr((char const *)pBuffer, "pdpdeact\r\n") != NULL))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



BOOL LTE_ConnectGetCClk(u8 *pRxBuf, u8 len, u8 *pRtc)
{
    int pos = 0;
    u8 rtcPos = 0;
    BOOL b = TRUE;
    u8 t = 0;

    // +CCLK: "21/07/23,08:09:17+00"
    //骞?
    pos = a_IndexOf(pRxBuf, pos, len, '"');
    if(b && pos >= 0)
    {
        b = FALSE;
        t = a_atoi(pRxBuf + pos + 1, 2, STD_LIB_FMT_DEC);
        pos += 3;
        if(21 <= t && t <= 99)
        {
            pRtc[rtcPos++] = t;
            b = TRUE;
        }
    }
    //鏈?
    pos = a_IndexOf(pRxBuf, pos, len, '/');
    if(b && pos >= 0)
    {
        b = FALSE;
        t = a_atoi(pRxBuf + pos + 1, 2, STD_LIB_FMT_DEC);
        pos += 3;
        if(1 <= t && t <= 12)
        {
            pRtc[rtcPos++] = t;
            b = TRUE;
        }
    }
    //鏃?
    pos = a_IndexOf(pRxBuf, pos, len, '/');
    if(b && pos >= 0)
    {
        b = FALSE;
        t = a_atoi(pRxBuf + pos + 1, 2, STD_LIB_FMT_DEC);
        pos += 3;
        if(1 <= t && t <= 31)
        {
            pRtc[rtcPos++] = t;
            b = TRUE;
        }
    }
    //鏃?
    pos = a_IndexOf(pRxBuf, pos, len, ',');
    if(b && pos >= 0)
    {
        b = FALSE;
        t = a_atoi(pRxBuf + pos + 1, 2, STD_LIB_FMT_DEC);
        pos += 3;
        if(t <= 23)
        {
            pRtc[rtcPos++] = t;
            b = TRUE;
        }
    }
    //鍒?
    pos = a_IndexOf(pRxBuf, pos, len, ':');
    if(b && pos >= 0)
    {
        b = FALSE;
        t = a_atoi(pRxBuf + pos + 1, 2, STD_LIB_FMT_DEC);
        pos += 3;
        if(t <= 59)
        {
            pRtc[rtcPos++] = t;
            b = TRUE;
        }
    }
    //绉?
    pos = a_IndexOf(pRxBuf, pos, len, ':');
    if(b && pos >= 0)
    {
        b = FALSE;
        t = a_atoi(pRxBuf + pos + 1, 2, STD_LIB_FMT_DEC);
        pos += 3;
        if(t <= 59)
        {
            pRtc[rtcPos++] = t;
            b = TRUE;
        }
    }
    
    if(b)   //鏍￠獙鏄惁鏄悎娉曟椂闂?
    {
        
    }

    return b;
}


TaskHandle_t g_hLTEInit = NULL;
void LTE_InitTask(void *p)
{
    LTE_PARAMS *pParams = NULL;
    u8 txFrameStr[LTE_TX_MAX_LENTH] = {0};             //256即可，可优化
    while(1)
    {
        if(g_nLTEStatus == LTE_STATUS_IDLE)
        {
            LTE_ConnectInit(&g_sLTEConnect, LTE_CNT_CMD_PWRON, pParams);
            g_nLTEStatus = LTE_STATUS_INIT;
        }
        else if(g_nLTEStatus ^ LTE_STATUS_CONNECT_OK)
        {
            pParams = (LTE_PARAMS *)p;
            if(USART_GetFlagStatus(LTE_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
            {
                USART_ClearFlag(LTE_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE);
                LTE_InitInterface(LTE_BAUDRARE);
                LTE_ConfigInt();
                LTE_EnableInt(ENABLE, DISABLE);
            }

            if(Uart_IsRcvFrame(g_sLTERcvFrame))
            {      
                if(strstr((const char *)g_sLTERcvFrame.buffer, (const char *)"POWERED DOWN") ||
                   strstr((const char *)g_sLTERcvFrame.buffer, (const char *)"+CME ERROR: 13"))  
                {
                    if(g_sLTEConnect.cmd == LTE_CNT_CMD_PWRON)
                    {
                        LTE_ConnectInit(&g_sLTEConnect, LTE_CNT_CMD_PWRON, pParams);
                    }
                    else if(g_sLTEConnect.cmd == LTE_CNT_CMD_PWROFF)
                    {
                        g_nLTEStatus = LTE_STATUS_PWR_OFF;
                        vTaskSuspend(NULL);
                    }
                }
                else if(strstr((const char *)g_sLTERcvFrame.buffer, (const char *)"RDY"))
                {
                    if(g_sLTEConnect.cmd == LTE_CNT_CMD_PWROFF)
                    {
                        LTE_ConnectInit(&g_sLTEConnect, LTE_CNT_CMD_PWROFF, pParams);
                    }
                }
                
                if(a_CheckStateBit(g_sLTEConnect.state, LTE_CNT_OP_STAT_RX))
                {          
                    if(LTE_ConnectCheckRsp(&g_sLTEConnect, g_sLTERcvFrame.buffer))   //如果校验响应帧失败，就继续接收，否则复位接收缓冲区
                    {
                        g_sLTEConnect.result = LTE_CNT_RESULT_OK;
                        a_SetState(g_sLTEConnect.state, LTE_CNT_OP_STAT_STEP);
                        LTE_ClearRxBuffer();
                    }
                    else
                    {
                        g_sLTERcvFrame.state = UART_FLAG_RCV;                          //继续接收
                        g_sLTERcvFrame.idleTime = 0;
                    }
                }
            }
            
            if(a_CheckStateBit(g_sLTEConnect.state, LTE_CNT_OP_STAT_TX))      //发送AT指令
            {
                if(g_sLTEConnect.index < g_sLTEConnect.num)
                {
                    if(LTE_ConnectTxCmd(&g_sLTEConnect, txFrameStr) <= LTE_TX_MAX_LENTH)
                    {
                        LTE_ClearRxBuffer();
                        g_sLTEConnect.tick = g_nSysTick;
                        LTE_WriteCmd((char *)txFrameStr);
                        a_SetState(g_sLTEConnect.state, LTE_CNT_OP_STAT_RX | LTE_CNT_OP_STAT_WAIT);
                    }
                    else
                    {
                        a_SetState(g_sLTEConnect.state, LTE_CNT_OP_STAT_STEP | LTE_CNT_OP_STAT_ERR);
                    }
                }
                a_ClearStateBit(g_sLTEConnect.state, LTE_CNT_OP_STAT_TX);
            }

            if(a_CheckStateBit(g_sLTEConnect.state, LTE_CNT_OP_STAT_WAIT))    //判断超时，每条指令的超时时间都不一样
            {
                if(g_sLTEConnect.tick + g_sLTEConnect.to[g_sLTEConnect.index] < g_nSysTick)
                {
                    g_sLTEConnect.repeat[g_sLTEConnect.index]--;              //有些指令允许多次操作
                    if(g_sLTEConnect.repeat[g_sLTEConnect.index] == 0)
                    {
                        a_ClearStateBit(g_sLTEConnect.state, LTE_CNT_OP_STAT_WAIT);
                        a_SetState(g_sLTEConnect.state, LTE_CNT_OP_STAT_STEP);
                        g_sLTEConnect.result = LTE_CNT_RESULT_TO;             //当前操作超时，GPRS执行流程错误
                    }
                    else
                    {
                        g_sLTEConnect.state = LTE_CNT_OP_STAT_TX;             //重复发送
                    }
                }
            }

            if(a_CheckStateBit(g_sLTEConnect.state, LTE_CNT_OP_STAT_STEP))    //下一步逻辑处理
            {
                LTE_ConnectStep(&g_sLTEConnect);                              //操作步骤处理
                if(g_sLTEConnect.result == LTE_CNT_RESULT_OK)                 //所有流程执行完成，并且成功，表示进入DTU模式
                {
                    g_sLTEConnect.state = LTE_CNT_OP_STAT_TX;
                    if(g_sLTEConnect.index == g_sLTEConnect.num)              //如果是开机操作执行完成，表示进入DTU，或者关闭
                    {
                        if(g_sLTEConnect.cmd == LTE_CNT_CMD_PWRON)            
                        {
                            g_sLTEConnect.state = LTE_CNT_OP_STAT_IDLE;
                            g_nLTEStatus = LTE_STATUS_CONNECT_OK;
                        }
                        else
                        {
                            g_sLTEConnect.state = LTE_CNT_OP_STAT_IDLE;
                            g_nLTEStatus = LTE_STATUS_PWR_OFF;
                        }
                    }
                }
                else													//初始化失败关机？？？、、、
                {
                    g_sLTEConnect.state = LTE_CNT_OP_STAT_IDLE;
                    g_nLTEStatus = LTE_STATUS_INIT_ERR;       
                    vTaskDelay(500);
                    LTE_ConnectInit(&g_sLTEConnect, LTE_CNT_CMD_PWRON, pParams);
                }
                a_ClearStateBit(g_sLTEConnect.state, LTE_CNT_OP_STAT_STEP);
            } 
        }
        else
        {
            vTaskSuspend(NULL);
        }
    }
}