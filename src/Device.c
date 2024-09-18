#include "Device.h"

const char DEVICE_VERSION[DEVICE_VERSION_SIZE]@0x08005000 = "M7000 24090401 G231200";

DEVICE_PARAMS g_sDeviceParams = {0};
DEVICE_RSPFRAME g_sDeviceRspFrame = {0};
DEVICE_NETRCVINFO g_sDeviceNetRcvInfo = {0};
DEVICE_DATASUBINFO g_sDeviceInfoSub = {0};
DEVICE_DATAPUBINFO g_sDeviceInfoPub = {0}; 
DEVICE_NETINFO g_sDeviceNetInfo = {0};
DEVICE_DATEFRAME g_sDeviceTemprFrame = {0};

u16 Device_ResponseFrame(u8 *pParam, u16 len, DEVICE_RSPFRAME *pRspFrame)
{
    u16 pos = 0;
    u16 crc = 0;

    pRspFrame->buffer[pos++] = UART_FRAME_FLAG_HEAD1; // frame head first byte
    pRspFrame->buffer[pos++] = UART_FRAME_FLAG_HEAD2; // frame haed second byte
    pRspFrame->buffer[pos++] = 0x00;   // length
    pRspFrame->buffer[pos++] = (g_sDeviceParams.addr >> 0) & 0xFF;
    pRspFrame->buffer[pos++] = (g_sDeviceParams.addr >> 8) & 0xFF;
    pRspFrame->buffer[pos++] = (pRspFrame->destAddr >> 0) & 0xFF;
    pRspFrame->buffer[pos++] = (pRspFrame->destAddr >> 8) & 0xFF;
    pRspFrame->buffer[pos++] = UART_FRAME_RESPONSE_FLAG;
    pRspFrame->buffer[pos++] = pRspFrame->cmd;               // cmd
    pRspFrame->buffer[pos++] = UART_FRAME_PARAM_RFU;     // RFU

    if(len > UART_FRAME_DATA_MAX_LEN)
    {
        pRspFrame->buffer[pos++] = (len >> 0) & 0xFF;
        pRspFrame->buffer[pos++] = (len >> 8) & 0xFF;

        memcpy(pRspFrame->buffer + pos, pParam, len);
        pos += len;
    }
    else
    {
        memcpy(pRspFrame->buffer + pos, pParam, len);
        pos += len;
        
        pRspFrame->buffer[pos++] = pRspFrame->flag;
        pRspFrame->buffer[pos++] = pRspFrame->err;
        
        pRspFrame->buffer[UART_FRAME_POS_LEN] = pos - 3 + 2; //减去帧头7E 55 LEN 的三个字节，加上CRC的两个字节
    }

    crc = a_GetCrc(pRspFrame->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    pRspFrame->buffer[pos++] = crc & 0xFF;
    pRspFrame->buffer[pos++] = (crc >> 8) & 0xFF;

    pRspFrame->len = pos;

    return pos;
}

u16 Device_ProcessUartFrame(u8 *pFrame, u16 len, u8 com)
{
    u8 cmd = 0;
    u16 destAddr = 0;
    u16 paramsLen = 0;
    u8 *pParams = NULL;

    destAddr = *(u16 *)(pFrame + UART_FRAME_POS_DESTADDR);
    
    if((destAddr != 0xFFFF) && (destAddr != g_sDeviceParams.addr))
    {
        return 0;
    }

    g_sDeviceRspFrame.destAddr = *((u16 *)(pFrame + UART_FRAME_POS_SRCADDR));
    g_sDeviceRspFrame.len = 0;
    cmd = *(pFrame + UART_FRAME_POS_CMD);

    g_sDeviceRspFrame.cmd = cmd;
    g_sDeviceRspFrame.flag = UART_FRAME_FLAG_OK;
    g_sDeviceRspFrame.err = UART_FRAME_RSP_NOERR;

    if(pFrame[UART_FRAME_POS_LEN] == 0)
    {
        paramsLen = *((u16 *)(pFrame + UART_FRAME_POS_PAR));
        pParams = pFrame + UART_FRAME_POS_PAR + 2;
    }
    else
    {
        paramsLen = len - UART_FRAME_MIN_LEN;
        pParams = pFrame + UART_FRAME_POS_PAR;
    }
    
    switch(cmd)
    {
        case DEVICE_FRAME_CMD_RST:
            if(paramsLen == 0)
            {            
                g_sDeviceRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDeviceRspFrame);
            }
        break;
        case DEVICE_FRAME_CMD_VER:
            if(paramsLen == 0)
            {            
                g_sDeviceRspFrame.len = Device_ResponseFrame((u8 *)DEVICE_VERSION, DEVICE_VERSION_SIZE, &g_sDeviceRspFrame);
            }
        break;
        default:
        break;
    }
    
    if(g_sDeviceRspFrame.len == 0)
    {
        g_sDeviceRspFrame.flag = UART_FRAME_FLAG_FAIL;
        g_sDeviceRspFrame.err = UART_FRAME_RSP_PARAMSERR;
        g_sDeviceRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDeviceRspFrame);
    }
    
    return g_sDeviceRspFrame.len;
}

TaskHandle_t g_hDevicePMqtt = NULL;
void Device_ProcessMqtt(void *p)
{
    static DEVICE_NETINFO *pNetInfo = {0};
    static DEVICE_DATASUBINFO *pSubInfo = {0};
    static DEVICE_DATAPUBINFO *pPubInfo = {0};
    while(1)
    {
        pNetInfo = &g_sDeviceNetInfo;
        pSubInfo = &g_sDeviceInfoSub;	
        pPubInfo = &g_sDeviceInfoPub;
        
        //串口错误处理:重新初始化
        if(g_nLTEStatus == LTE_STATUS_CONNECT_OK)
        {
            if(USART_GetFlagStatus(LTE_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
            {
                USART_ClearFlag(LTE_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE);
                LTE_InitInterface(LTE_BAUDRARE);
                LTE_ConfigInt();
                LTE_EnableInt(ENABLE, DISABLE);
            }

            if(Uart_IsRcvFrame(g_sLTERcvFrame))
            {
                if(g_sLTERcvFrame.index > 0)
                {
                    if(pNetInfo->state == DEVICE_SERVER_STAT_AT_WAIT)				//AT接收
                    {
                        if(Device_CheckAtRsp(pSubInfo->data[pSubInfo->index].cmd, g_sLTERcvFrame.buffer) && 
                           (pNetInfo->priority == DEVICE_SERVER_PRIORITY_SUB))
                        {
                            Device_ServerNetClear(pNetInfo, DEVICE_SERVER_STAT_PAYLOAD_TX);
                            g_sDeviceNetInfo.repat = 0;
                        }
                        else if(pNetInfo->priority == DEVICE_SERVER_PRIORITY_PUB)
                        {
                            if(Device_CheckAtRsp(pPubInfo->data[pPubInfo->index].cmd, g_sLTERcvFrame.buffer))
                            {
                                Device_ServerNetClear(pNetInfo, DEVICE_SERVER_STAT_PAYLOAD_TX);
                                g_sDeviceNetInfo.repat = 0;
                            }
                            else if(pPubInfo->data[pPubInfo->index].cmd == LTE_CNT_OP_GET_QSQ)
                            {//未收到AT返回？？？\直接处理的AT，如QSA
                                Device_ServerNetClear(pNetInfo, DEVICE_SERVER_STAT_STEP);
                                g_sDeviceNetInfo.repat = 0;
                            }
                        }
                    }
                    else if(pNetInfo->state == DEVICE_SERVER_STAT_PAYLOAD_WAIT)		//payload接收
                    {
                        if(Device_CheckPubInfo(&g_sDeviceNetRcvInfo, g_sLTERcvFrame.buffer))
                        {
                            Device_ServerNetClear(pNetInfo, DEVICE_SERVER_STAT_STEP);
                            g_sDeviceNetInfo.repat = 0;
                        }
                        else
                        {
                            
                        }
                    }
                    
                    if(Device_CheckSubInfo(&g_sDeviceNetRcvInfo, g_sLTERcvFrame.buffer))//cmd接收
                    {
                        Device_ServerProcessSubInfo(&g_sDeviceNetRcvInfo);
                    }
                }                                                     
                memset(&g_sLTERcvFrame, 0, sizeof(UART_RCVFRAME));
            }
            
            if(pNetInfo->state == DEVICE_SERVER_STAT_IDLE)
            {
                if(pSubInfo->num)
                {
                    if(pSubInfo->index < pSubInfo->num)
                    {
                        pNetInfo->state = DEVICE_SERVER_STAT_AT_TX;
                        pNetInfo->priority = DEVICE_SERVER_PRIORITY_SUB;
                    }
                }
                else if(pPubInfo->num)
                {
                    if(pPubInfo->index < pPubInfo->num)
                    {
                        pNetInfo->state = DEVICE_SERVER_STAT_AT_TX;
                        pNetInfo->priority = DEVICE_SERVER_PRIORITY_PUB;
                    }
                }
            }
            else if(pNetInfo->state == DEVICE_SERVER_STAT_AT_TX)
            {
                if(pNetInfo->priority == DEVICE_SERVER_PRIORITY_SUB)
                {
                    Device_ServerTxFrame(pNetInfo, pSubInfo->data[pSubInfo->index].atFrame, 0, DEVICE_SERVER_STAT_AT_WAIT, DEVICE_MATT_SERVER_TX_AT);
                }
                else
                {
                    Device_ServerTxFrame(pNetInfo, pPubInfo->data[pPubInfo->index].atFrame, 0, DEVICE_SERVER_STAT_AT_WAIT, DEVICE_MATT_SERVER_TX_AT);
                }
            }
            else if(pNetInfo->state == DEVICE_SERVER_STAT_AT_WAIT)
            {
                if(pNetInfo->time + DEVICE_SERVER_AT_OP_TIME < g_nSysTick)
                {
                    if(pNetInfo->repat > DEVICE_SERVER_AT_REPAT_TICK)			//AT命令无返回，进行下步操作。多次是否重启LTE模块
                    {
                        pNetInfo->state = DEVICE_SERVER_STAT_STEP;
                        pNetInfo->result = DEVICE_SERVER_RESULT_FAIL;
                        //超时，重启模块    AT超时短，连续二十次AT指令（近一千二字节,）无返回，代表LTE模块挂或陷入假死状态
                        if(pNetInfo->priority == DEVICE_SERVER_PRIORITY_PUB)
                        {
                            if(pPubInfo->data[pPubInfo->index].cmd == DEVICE_MQTT_TYPE_HEART_REPORT || 
                               pPubInfo->data[pPubInfo->index].cmd == DEVICE_MQTT_TYPE_DATA_REPORT)
                            {
                                vTaskResume(g_hLTEInit);
                                g_nLTEStatus = LTE_STATUS_INIT;
                            }
                        }
                    }
                    else
                    {
                        pNetInfo->state = DEVICE_SERVER_STAT_AT_TX;
                        pNetInfo->repat++;
                    }
                }
            }
            else if(pNetInfo->state == DEVICE_SERVER_STAT_PAYLOAD_TX)
            {
                if(pNetInfo->priority == DEVICE_SERVER_PRIORITY_SUB)
                {
                    Device_ServerTxFrame(pNetInfo, pSubInfo->data[pSubInfo->index].payloadframe, pSubInfo->data[pSubInfo->index].lenth, DEVICE_SERVER_STAT_PAYLOAD_WAIT, DEVICE_MATT_SERVER_TX_PAYLOAD);
                }
                else
                {
                    Device_ServerTxFrame(pNetInfo, pPubInfo->data[pPubInfo->index].payloadframe, pPubInfo->data[pPubInfo->index].lenth, DEVICE_SERVER_STAT_PAYLOAD_WAIT, DEVICE_MATT_SERVER_TX_PAYLOAD);
                }
            }
            else if(pNetInfo->state == DEVICE_SERVER_STAT_PAYLOAD_WAIT)
            {
                if(pNetInfo->time + DEVICE_SERVER_AT_PAYLOAD_TIME < g_nSysTick)
                {
                    if(pNetInfo->repat > DEVICE_SERVER_PAYLOAD_REPAT_TICK)			//payload数据无返回，进行下步操作。多次是否重启LTE模块
                    {
                        pNetInfo->state = DEVICE_SERVER_STAT_STEP;
                        pNetInfo->result = DEVICE_SERVER_RESULT_FAIL;
                        //超时，重启模块    AT超时短，连续二十次AT指令（近一千二字节,）无返回，代表LTE模块挂或陷入假死状态
                        if(pNetInfo->priority == DEVICE_SERVER_PRIORITY_PUB)
                        {
                            if(pPubInfo->data[pPubInfo->index].cmd == DEVICE_MQTT_TYPE_HEART_REPORT || 
                               pPubInfo->data[pPubInfo->index].cmd == DEVICE_MQTT_TYPE_DATA_REPORT)
                            {
                                vTaskResume(g_hLTEInit);
                                g_nLTEStatus = LTE_STATUS_INIT;
                            }
                        }
                    }
                    else
                    {
                        pNetInfo->state = DEVICE_SERVER_STAT_AT_TX;//DEVICE_SERVER_STAT_PAYLOAD_TX;  //重新執行AT
                        pNetInfo->repat++;
                    }
                }
            }
            else if(pNetInfo->state == DEVICE_SERVER_STAT_STEP)
            {
                if(pNetInfo->priority == DEVICE_SERVER_PRIORITY_SUB)
                {
                    if((pSubInfo->num) && 
                       (pSubInfo->index < DEVICE_DATA_SUB_LEN) && 
                       (pSubInfo->index < pSubInfo->num))
                    {
                        pSubInfo->index++;
                        if(pSubInfo->index >= pSubInfo->num)
                        {
                            Device_ClearSubInfo();
                        }
                    }
                    else
                    {
                        Device_ClearSubInfo();
                    }
                }
                else
                {	
                    if((pPubInfo->num) && 
                       (pPubInfo->index < DEVICE_DATA_PUB_LEN) && 
                       (pPubInfo->index < pPubInfo->num))
                    {
                        pPubInfo->index++;
                        if(pPubInfo->index >= pPubInfo->num)
                        {
                            Device_ClearPubInfo();
                        }
                    }
                    else
                    {
                        Device_ClearPubInfo();
                    }
                }
                pNetInfo->state = DEVICE_SERVER_STAT_DELAY;
            }
            else if(pNetInfo->state == DEVICE_SERVER_STAT_DELAY)
            {
                if(pNetInfo->delayTick + DEVICE_SERVER_DELAY_TIME < g_nSysTick)
                {
                    pNetInfo->state = DEVICE_SERVER_STAT_IDLE;
                }
            }
        }
        else if(g_nLTEStatus == 0xFF)
        {
            vTaskSuspend(NULL);
        }
    }
}


BOOL Device_CheckAtRsp(u8 type, u8 *pRxBuf)
{
    BOOL bOK = FALSE;
    switch(type)
    {
        case  LTE_CNT_OP_GET_QSQ:
            if(strstr((char const *)pRxBuf, "OK") != NULL)
            {
                g_sDeviceNetInfo.sigNum = a_atoi(pRxBuf + 8, 2, STD_LIB_FMT_DEC);
                g_sDeviceNetInfo.channelErrNum = a_atoi(pRxBuf + 11, 2,STD_LIB_FMT_DEC);
            }
            break;
        case DEVICE_MQTT_TYPE_HEART_REPORT:
        case DEVICE_MQTT_TYPE_DATA_REPORT:
        case DEVICE_MQTT_TYPE_CMD_REPORT:
        case DEVICE_MQTT_TYPE_OTA_REPORT:
            if(strstr((char const *)pRxBuf, ">") != NULL)
            {
                bOK = TRUE;
            }
            break;
        default:
            break;
    }
    return bOK;
}

void Device_ServerProcessSubInfo(DEVICE_NETRCVINFO *pRcvInfo)               //处理服务器下发数据
{
    if(pRcvInfo->cmd == DEVICE_NET_CMD_SET_PROPERTY)
    {
        u16 crc1 = 0, crc2 = 0;
        u16 len = 0;
        
        crc1 = Uart_GetFrameCrc(pRcvInfo->requestBuffer, pRcvInfo->lenth);
        crc2 = a_GetCrc(pRcvInfo->requestBuffer + UART_FRAME_POS_LEN, pRcvInfo->lenth - 4);
        
        if(crc1 == crc2)
        {
            len = Device_ProcessUartFrame(pRcvInfo->requestBuffer, pRcvInfo->lenth, DEVICE_COM_LTE);
            if(len)
            {
                Device_FomatAddData(DEVICE_MQTT_TYPE_CMD_REPORT);
            }
        }
    }
    else if(pRcvInfo->cmd == DEVICE_NET_CMD_OTA_INFORM)
    {
        Device_FomatAddData(DEVICE_MQTT_TYPE_OTA_REPORT);
    }
}

u16 Device_Str2Hex(char *pStr, u8 *pHexBuf, u16 len)
{
    u16 i = 0, index = 0;
    u8 t = 0;
    
    if(pStr != NULL)
    {
        index = len & 0xFFFE;
        for(i = 0; i < index; i += 2)
        {
            t = g_aAsicto[pStr[i + 0]];
            t <<= 4;
            t |= g_aAsicto[pStr[i + 1]];
            
            *pHexBuf = t;
            pHexBuf++;
        }
        index >>= 1;
    }
    return index;
}

BOOL Device_CheckPubInfo(DEVICE_NETRCVINFO *pRcvInfo, u8 *pRxBuf)
{
    BOOL bOK = FALSE;
    u8 flag = 0;
    
	if(strstr((char const *)pRxBuf, "+QMTPUBEX: 0,1,0") != NULL)		//由模块判定，消息发布成功并且接收到服务器ACK
	{
		bOK = TRUE;

		Device_ServerComOkCallBack(&flag);
    }
	else if(strstr((char const *)pRxBuf, "+QMTPUBEX: 0,1,2") != NULL)     //数据包发送失败，主要是心跳包
	{
		bOK = FALSE;
	}
	else if(strstr((char const *)pRxBuf, "+QMTPING") != NULL) 			//1.5 * keepLiveTime无数据传输，服务器断开
	{
		bOK = FALSE;
	}
    return bOK;
}

char g_aMqttAtBuf[DEVICE_DETA_AT_FRAME_LEN] = {0};
char g_aMqttBuf[DEVICE_DETA_FRAME_LEN] = {0};
BOOL Device_FomatAddData(u8 type)
{
    u16 lenth = 0;
	BOOL bOk = FALSE;
	DEVICE_DATEFRAME *pDateFrame = {0};
    DEVICE_NETRCVINFO *pRcvInfo = NULL;
    char frameStr[DEVICE_DATA_STR_MAX_LEN] = {0};    
    
    pRcvInfo = &g_sDeviceNetRcvInfo;
    if(memcmp(g_sLTEConnect.imeiStr, g_nImeiStr, LTE_IMSI_LEN) == 0)            //校验IMEI，防止被更改   
    {   
        pDateFrame = &g_sDeviceTemprFrame;
        memset(pDateFrame, 0,sizeof(DEVICE_DATEFRAME));
        memset(g_aMqttAtBuf, 0, DEVICE_DETA_AT_FRAME_LEN);
        memset(g_aMqttBuf, 0, DEVICE_DETA_FRAME_LEN);
        switch(type)
        {
            case DEVICE_MQTT_TYPE_HEART_REPORT:
                    if(!bOk)
                    {   
                        sprintf(g_aMqttAtBuf,"AT+CSQ");
                        //Device_ServerAddData(pDateFrame, LTE_CNT_OP_GET_QSQ, DEVICE_SERVER_PRIORITY_PUB);
                        
                        g_sDeviceRspFrame.cmd = DEVICE_FRAME_CMD_HEART;
                        lenth = Device_HeartFormatFrame(&g_sDeviceRspFrame);
                        if(lenth <= DEVICE_DATA_STR_MAX_LEN)
                        {     
                            a_Hex2Str(frameStr, (u8 *)g_sDeviceRspFrame.buffer, lenth);
                            sprintf(g_aMqttBuf, "{\"id\":\"%.8d\",\"version\":\"1.0\",\"params\": {\"Heart\":{\"value\":\"%s\"}}}", g_nSysTick, frameStr);
                            pDateFrame->lenth = strlen(g_aMqttBuf);
                            sprintf(g_aMqttAtBuf,"AT+QMTPUBEX=0,1,1,0,\"$sys/%.10s/%.15s/thing/property/post\",%d", LTE_PRDOCT_ID, g_sLTEConnect.imeiStr, pDateFrame->lenth); 
                            
                            if(pDateFrame->lenth)
                            {
                                //Device_ServerAddData(pDateFrame, DEVICE_MQTT_TYPE_HEART_REPORT, DEVICE_SERVER_PRIORITY_PUB);
                                bOk =TRUE;
                            }
                        }
                    }
              break;
            case DEVICE_MQTT_TYPE_DATA_REPORT:
                    if(!bOk)
                    {
                        g_sDeviceRspFrame.cmd = DEVICE_FRAME_CMD_RFID;
                        //lenth = Device_SlaveFormatFrame(&g_sDeviceRspFrame);
                        if(lenth <= DEVICE_DATA_STR_MAX_LEN)
                        {     
                            a_Hex2Str(frameStr, g_sDeviceRspFrame.buffer, lenth);
                            sprintf(g_aMqttBuf, "{\"id\":\"%.8d\",\"version\":\"1.0\",\"params\": {\"Data1\":{\"value\":\"%s\"}}}", g_nSysTick, frameStr);
                            pDateFrame->lenth = strlen(g_aMqttBuf);
                            sprintf(g_aMqttAtBuf,"AT+QMTPUBEX=0,1,1,0,\"$sys/%.10s/%.15s/thing/property/post\",%d", LTE_PRDOCT_ID, g_sLTEConnect.imeiStr, pDateFrame->lenth);
                            
                            if(pDateFrame->lenth)
                            {
                                //Device_ServerAddData(pDateFrame, DEVICE_MQTT_TYPE_DATA_REPORT, DEVICE_SERVER_PRIORITY_PUB);
                                bOk =TRUE;
                            }
                        }
                    }
              break;
            case DEVICE_MQTT_TYPE_CMD_REPORT:
                    if(!bOk)
                    {
                        lenth = g_sDeviceRspFrame.len;
                        if(lenth <= DEVICE_DATA_STR_MAX_LEN)
                        {     
                            a_Hex2Str(frameStr, g_sDeviceRspFrame.buffer, lenth);
                            sprintf(g_aMqttBuf, "{\"id\":\"%s\",\"code\":\"%d\",\"msg\": \"%s\"}", pRcvInfo->id, DEVICE_MQTT_RSP_OK, frameStr);
                            pDateFrame->lenth = strlen(g_aMqttBuf);
                            sprintf(g_aMqttAtBuf,"AT+QMTPUBEX=0,1,1,0,\"$sys/%.10s/%.15s/thing/property/set_reply\",%d", LTE_PRDOCT_ID, g_sLTEConnect.imeiStr, pDateFrame->lenth);
                            if(pDateFrame->lenth)
                            {
                                //Device_ServerAddData(pDateFrame, DEVICE_MQTT_TYPE_CMD_REPORT, DEVICE_SERVER_PRIORITY_SUB);
                                bOk =TRUE;
                            }
                        }
                    }
              break;
            case DEVICE_MQTT_TYPE_OTA_REPORT:
                    if(!bOk)
                    {
                        sprintf(g_aMqttBuf, "{\"id\":\"%s\",\"code\":\"%d\",\"version\": \"%s\"}", pRcvInfo->id, DEVICE_MQTT_RSP_OK, pRcvInfo->version);
                        pDateFrame->lenth = strlen(g_aMqttBuf);
                        sprintf(g_aMqttAtBuf,"AT+QMTPUBEX=0,1,1,0,\"$sys/%.10s/%.15s/ota/inform_reply\",%d", LTE_PRDOCT_ID, g_sLTEConnect.imeiStr, pDateFrame->lenth);

                        if(pDateFrame->lenth)
                        {
                            //Device_ServerAddData(pDateFrame, DEVICE_MQTT_TYPE_OTA_REPORT, DEVICE_SERVER_PRIORITY_SUB);
                            bOk =TRUE;
                        }
                    }
              break;
              default:
              break;
        }
    }
    else
    {
        memcpy(g_sLTEConnect.imeiStr, g_nImeiStr, LTE_IMSI_LEN);
    }
    
	return bOk;
}

//检测订阅数据，主要是onenet_cmd数据
BOOL Device_CheckSubInfo(DEVICE_NETRCVINFO *pRcvInfo, u8 *pRxBuf)		
{
    char *pStart = NULL, *pEnd = NULL;
    BOOL bOK = FALSE;
    u16 lenth = 0;
    
	if(strstr((char const *)pRxBuf, "+QMTSTAT") != NULL)	                                                                        //MQTT状态发生变化
	{
        g_sDeviceNetInfo.status = (u8)*(strstr((char const *)pRxBuf, "+QMTSTAT") + DEVICE_GET_MATT_STAT_STR_LEN) - '0';
	}
    else if(strstr((char const *)pRxBuf, DEVICE_STR_INTERVAL_SET_PRO) != NULL)
    {
        pStart = strstr((char *)pRxBuf, DEVICE_STR_INTERVAL_ID) + strlen(DEVICE_STR_INTERVAL_ID);
        pEnd = strstr((char *)pStart, DEVICE_STR_INTERVAL_ITEM_END);
        if((pStart != NULL) && (pEnd != NULL))
        {
            memcpy(pRcvInfo->id, pStart, pEnd - pStart);
        }
        else
        {
            return bOK;
        
        }
        
        pStart = strstr((char *)pRxBuf, DEVICE_STR_INTERVAL_VERSION) + strlen(DEVICE_STR_INTERVAL_VERSION);
        pEnd = strstr((char *)pStart, DEVICE_STR_INTERVAL_ITEM_END);
        if((pStart != NULL) && (pEnd != NULL))
        {
            memcpy(pRcvInfo->version, pStart, pEnd - pStart);
        }
        else
        {
            return bOK;
        
        }
      
        pStart = strstr((char *)pRxBuf, DEVICE_STR_INTERVAL_CMD) + strlen(DEVICE_STR_INTERVAL_CMD);
        pEnd = strstr((char *)pRxBuf, DEVICE_STR_INTERVAL_FRAME_END);
        if((pStart != NULL) && (pEnd != NULL))
        {
            lenth = (pEnd - pStart);
            if((lenth <= DEVICE_DATA_STR_MAX_LEN) && 
               (( lenth % 2) == 0) && 
               (lenth >= UART_FRAME_DATA_MIN_LEN))                    //过滤数据
            {
                Device_Str2Hex(pStart, pRcvInfo->requestBuffer, lenth);
                pRcvInfo->lenth = (lenth >> 1);
                pRcvInfo->cmd = DEVICE_NET_CMD_SET_PROPERTY;
                bOK = TRUE;
            }
            
        }
    }
    else if(strstr((char const *)pRxBuf, DEVICE_STR_INTERVAL_OTA_INFORM) != NULL)
    {
        pStart = strstr((char *)pRxBuf, DEVICE_STR_INTERVAL_ID) + strlen(DEVICE_STR_INTERVAL_ID);
        pEnd = strstr((char *)pStart, DEVICE_STR_INTERVAL_OTA_FRAME_END);
        if((pStart != NULL) && (pEnd != NULL))
        {
            memcpy(pRcvInfo->id, pStart, pEnd - pStart);
        }
        else
        {
            return bOK;
        
        }
        
        pStart = strstr((char *)pRxBuf, DEVICE_STR_INTERVAL_VERSION) + strlen(DEVICE_STR_INTERVAL_VERSION);
        pEnd = strstr((char *)pStart, DEVICE_STR_INTERVAL_ITEM_END);
        if((pStart != NULL) && (pEnd != NULL))
        {
            memcpy(pRcvInfo->version, pStart, pEnd - pStart);
        }
        else
        {
            return bOK;
        
        }
        
        if(strstr((char const *)pRxBuf, "STUDIO-OTA-C") != NULL)
        {   //OTA任务通知
            pRcvInfo->cmd = DEVICE_NET_CMD_OTA_INFORM;
            bOK =TRUE;
        
        }
    }
        
    return bOK;
}

void Device_ServerComOkCallBack(void *pParms)
{
    /*DEVICE_NETRCVINFO *pRcvInfo = NULL;    
      
    pRcvInfo = &g_sDeviceNetRcvInfo;
    if(pRcvInfo->cmd == DEVICE_NET_CMD_SET_PROPERTY)
    {
        if(g_sDeviceRspFrame.cmd == DEVICE_FRAME_CMD_RST)				//复位
        {
            Sys_SoftReset();
        }
        g_sDeviceNetInfo.wakeTime = g_sRtcTime.counter;                 //后台有命令下来，更新清醒时间
        
    }
    else if(pRcvInfo->cmd == DEVICE_NET_CMD_OTA_INFORM)             //OTA通知完成，进入BOOT
    {       
      //  g_sFramBootParamenter.appState = FRAM_BOOT_APP_DATA_DOWD;
        //if(Fram_WriteBootParamenter())
        {
        //    Sys_SoftReset();
        }
    }
    
    pRcvInfo->cmd = DEVICE_NET_CMD_NULL;
    Device_ClearProcessBuffer();
    */
}

u16 Device_HeartFormatFrame(DEVICE_RSPFRAME *pRspFrame)
{
    u16 pos = 0;
   /* u16 crc = 0;

    pRspFrame->buffer[pos++] = UART_FRAME_FLAG_HEAD1; // frame head first byte
    pRspFrame->buffer[pos++] = UART_FRAME_FLAG_HEAD2; // frame haed second byte
    pRspFrame->buffer[pos++] = 0x00;   // length
    pRspFrame->buffer[pos++] = (g_sDeviceParams.addr >> 0) & 0xFF;
    pRspFrame->buffer[pos++] = (g_sDeviceParams.addr >> 8) & 0xFF;
    pRspFrame->buffer[pos++] = (pRspFrame->destAddr >> 0) & 0xFF;
    pRspFrame->buffer[pos++] = (pRspFrame->destAddr >> 8) & 0xFF;
    pRspFrame->buffer[pos++] = UART_FRAME_RESPONSE_FLAG;
    pRspFrame->buffer[pos++] = pRspFrame->cmd;               // cmd
    pRspFrame->buffer[pos++] = UART_FRAME_PARAM_RFU;     // RFU

    *(pRspFrame->buffer + (pos++)) = g_sDeviceNetInfo.event;
    
    memcpy(pRspFrame->buffer + pos, &g_sDeviceParams.heartPeriod, sizeof(g_sDeviceParams.heartPeriod));
    pos += sizeof(g_sDeviceParams.heartPeriod);
    
    *(pRspFrame->buffer + (pos++)) = g_sDeviceParams.subDeviceNum;
    
    memcpy(pRspFrame->buffer + pos, &g_sDeviceParams.subDeviceOpPeriod, sizeof(g_sDeviceParams.subDeviceOpPeriod));
    pos += sizeof(g_sDeviceParams.subDeviceOpPeriod);

    memcpy(pRspFrame->buffer + pos, &g_sDeviceParams.subDeviceOpAddr, sizeof(g_sDeviceParams.subDeviceOpAddr));
    pos += sizeof(g_sDeviceParams.subDeviceOpAddr);
    
    memcpy(pRspFrame->buffer + pos, &g_sDeviceParams.subDeviceOpNum, sizeof(g_sDeviceParams.subDeviceOpNum));
    pos += sizeof(g_sDeviceParams.subDeviceOpNum);

    *(pRspFrame->buffer + (pos++)) = g_sDeviceNetInfo.waterIm;
    *(pRspFrame->buffer + (pos++)) = g_sDeviceNetInfo.manCover;
    *(pRspFrame->buffer + (pos++)) = g_sDeviceNetInfo.angel;
    
    *(pRspFrame->buffer + (pos++)) = g_sDeviceNetInfo.sigNum;
    
    memcpy(pRspFrame->buffer + pos, &g_sBatWinAvgInfo.voltage, sizeof(g_sBatWinAvgInfo.voltage));
    pos += sizeof(g_sBatWinAvgInfo.voltage);

    pRspFrame->buffer[pos++] = pRspFrame->flag;
    pRspFrame->buffer[pos++] = pRspFrame->err;
    pRspFrame->buffer[UART_FRAME_POS_LEN] = pos - 3 + 2; //减去帧头7E 55 LEN 的三个字节，加上CRC的两个字节
    crc = a_GetCrc(pRspFrame->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    pRspFrame->buffer[pos++] = crc & 0xFF;
    pRspFrame->buffer[pos++] = (crc >> 8) & 0xFF;

    pRspFrame->len = pos;
*/
    return pos;
}
