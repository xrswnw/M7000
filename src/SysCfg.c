#include "SysCfg.h"

u32 g_nSysState = SYS_STAT_IDLE;
u32 g_nLedDelayTime = 0;

u32 g_nChkComTime = 0;

#if SYS_ENABLE_WDT
    #define SYS_ENABLE_TEST         0
#else
    #define SYS_ENABLE_TEST         1
#endif

void Sys_Delayms(u32 n)
{
    //72MHZ
    n *= 0x6000;
    n++;
    while(n--);
}


void Sys_CfgClock(void)
{
    ErrorStatus HSEStartUpStatus;

    RCC_DeInit();
    //Enable HSE
    RCC_HSEConfig(RCC_HSE_ON);

    //Wait till HSE is ready
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if(HSEStartUpStatus == SUCCESS)
    {

        //HCLK = SYSCLK = 72.0M
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        //PCLK2 = HCLK = 72.0M
        RCC_PCLK2Config(RCC_HCLK_Div1);

        //PCLK1 = HCLK/2 = 33.9M
        RCC_PCLK1Config(RCC_HCLK_Div2);

        //ADCCLK = PCLK2/2
        RCC_ADCCLKConfig(RCC_PCLK2_Div8);

        // Select USBCLK source 72 / 1.5 = 48M
        RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);

        //Flash 2 wait state
        FLASH_SetLatency(FLASH_Latency_2);

        //Enable Prefetch Buffer
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

		//12 * 6
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);    //PLL在最后设置

        //Enable PLL
        RCC_PLLCmd(ENABLE);

        //Wait till PLL is ready
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        //Select PLL as system clock source
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        //Wait till PLL is used as system clock source
        while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
}

void Sys_CfgPeriphClk(FunctionalState state)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_AFIO |
                           RCC_APB2Periph_USART1, state);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 |
                           RCC_APB1Periph_USART2 |
                           RCC_APB1Periph_USART3 |
                           RCC_APB1Periph_PWR  |
                           RCC_APB1Periph_BKP  |
                           RCC_APB1Periph_WWDG , state);
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, state);
}

void Sys_CfgNVIC(void)
{
    //NVIC_InitTypeDef NVIC_InitStructure;
#ifdef  VECT_TAB_RAM
    //Set the Vector Table base location at 0x20000000
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  //VECT_TAB_FLASH
    //Set the Vector Table base location at 0x08000000
#ifdef _ANYID_BOOT_GD32_
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4000);
#else
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
#endif
#endif

    //Configure the Priority Group to 2 bits
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}


void Sys_CtrlIOInit(void)
{
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}

void Sys_Init(void)
{
    Sys_CfgClock();
    Sys_CfgNVIC();
    Sys_CfgPeriphClk(ENABLE);
    Sys_Delayms(1000);
    Sys_Delayms(1000);
	#if SYS_ENABLE_WDT
    WDG_InitIWDG();
	#endif
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );			                //将所有优先级位都指定为抢占优先级位， 不保留任何优先级位作为子优先级位，方便调度管理 否则报错
    Sys_CtrlIOInit();
	RCC_ClocksTypeDef g_sSysclockFrep; RCC_GetClocksFreq(&g_sSysclockFrep);    //查看时钟频率
    
    Sys_EnableInt();
}

u32 g_nAlarmDelay = 0;
void Sys_LedTask(void)
{		
    if(a_CheckStateBit(g_nSysState, SYS_STAT_SLEEP))
    {
        return ;
    }
    
	if(a_CheckStateBit(g_nSysState, SYS_STAT_RUNLED))
	{	
		g_nLedDelayTime++;
        
		a_ClearStateBit(g_nSysState, SYS_STAT_RUNLED);
        #if SYS_ENABLE_WDT
        WDG_FeedIWDog();
        #endif
	}
}






























































//-----------------------------------------------
void Sys_TaskCreat()
{
    portENTER_CRITICAL();
    //Enter Ceitical

    #if FREE_RTOS_ENABLE_ERR_LOG
        Uart_Init();
    #endif
    
    //485
    Rs485_Init(RS485_BAUDRARE);
    xTaskCreate(Rs485TxTask, "Rs485TxTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &g_hRs485Tx);
    xTaskCreate(Sys_Rs485ProcessTask, "Sys_Rs485ProcessTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &g_hSysProcessRs485);
    //
    
    //
    #if configUSE_STATS_FORMATTING_FUNCTIONS
        xTaskCreate(Sys_RunTime, "Sys_RunTime", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);
    #endif
    //
        
    //
    LTE_Init();
    xTaskCreate(LTE_InitTask, "LTE_InitTask", configMINIMAL_STACK_SIZE, &g_sDeviceParams.lteParams, tskIDLE_PRIORITY + 1, &g_hLTEInit);
    //
    
    vTaskDelete(NULL);
    
    //Exti Ceitical
    portEXIT_CRITICAL();
}

TaskHandle_t g_hSysProcessRs485 = NULL;
void Sys_Rs485ProcessTask(void *p)
{
    UART_FRAME uartFrame = {0};
    while (1)
    {
        memset(&uartFrame, 0, sizeof(uartFrame)); 
        if(xQueueReceive(g_sRs485RxQueue, &uartFrame, portMAX_DELAY) == pdTRUE)
        {
            u16 pos = 0, lenth = 0, txLen = 0;
            
            lenth = Uart_UsrCheckFrame(uartFrame.frame, uartFrame.lenth, &pos);
            if(lenth >= UART_FRAME_MIN_LEN)
            {
                txLen = Device_ProcessUartFrame(uartFrame.frame + pos, lenth, 0);
                if(txLen && (txLen <= RS485_RX_FRAME_LEN))
                {
                    memcpy(uartFrame.frame, g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
                    uartFrame.lenth = g_sDeviceRspFrame.len;
                    if(xQueueSend(g_sRs485TxQueue, &uartFrame, portMAX_DELAY) != pdTRUE)                       //队列满待优化
                    {   //写入发送队列
                        
                    }
                }
            }
        }
    }
}
//监控各个任务所占时间，调试使用
void Sys_RunTime(void *p)
{
    char frame[256] = {0};          //堆栈区间过小可能导致越界
    while(1)
    {
        vTaskGetRunTimeStats(frame);
  
        printf("%s\r\n", frame);
        vTaskDelay(1000);
    }
}

