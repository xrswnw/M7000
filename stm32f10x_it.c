/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    04/16/2010
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "SysCfg.h"


/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
/*
void SVC_Handler(void)
{
}
*/
/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
/*
void PendSV_Handler(void)
{
}
*/
/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/*
void SysTick_Handler(void)
{
    g_nSysTick++;
    if(g_nSysTick % 5 == 0)
	{
		a_SetStateBit(g_nSysState, SYS_STAT_RUNLED);
	}
	
	if(g_nSysTick % 21 == 0)
	{
		a_SetStateBit(g_nSysState,  SYS_STAT_ADDRECORD | SYS_STAT_AD | SYS_STAT_IN_SENSOR | SYS_STAT_CTLOUTPUT);
	}
}
*/

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */
    
void TIM2_IRQHandler()
{
    if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET) //溢出中断
	{
		FreeRTOSRunTimeTicks++;
        if((FreeRTOSRunTimeTicks % TIM_TOTAL_5MS) == 0) 
        {//5ms中断，可做超时接收，兼容旧工程
            g_nSysTick++;
            
            Uart_IncIdleTime(STICK_TIME_MS, g_sLTERcvFrame);
        }
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //清除中断标志位
	}
}

u16 g_nLogSr = 0;
u16 g_nLogDr = 0;
void USART2_IRQHandler(void)
{
    if(UART_PORT->SR & 0x0010)
    {    
        Uart_DisableRxDma();
    }

    g_nLogSr = UART_PORT->SR;                              //通过读取SR和DR清空中断标志
    g_nLogDr = UART_PORT->DR;    
}


u16 g_nLteSr = 0;
u16 g_nLteDr = 0;
void LTE_IRQHandler(void)
{
    if((LTE_PORT->SR) & USART_IT_RXNE)
    {
        g_nLteDr = LTE_ReadByte();
        g_sLTERcvFrame.buffer[g_sLTERcvFrame.index++] = g_nLteDr;
        if(g_sLTERcvFrame.index < UART_BUFFER_MAX_LEN)
        {
            g_sLTERcvFrame.state |= UART_FLAG_RCV;
        }
        else
        {
           g_sLTERcvFrame.state = UART_STAT_END; 
        }
        g_sLTERcvFrame.idleTime = 0;
    }
    else
    {
        LTE_ReadByte();
    }
	g_nLteSr = LTE_PORT->SR;				//通过读取清空寄存器的值
	g_nLteDr = LTE_PORT->DR;
}




















//485


void Rs485_RxDMAIRQHandler(void)
{
    Rs485_DisableRxDma();           //接收缓冲区满，一般不会出现这样的情况，如果有，就可能系统有故障
    g_sRs485TempFrame.lenth = Rs485_GetRxLen(); 
}

void Rs485_TxDMAIRQHandler(void)
{
    Rs485_DisableTxDma();           //DMA完成后，最后一个字节可能没有发送出去，需要在主循环中做判断处理
    Rs485_EnableRxDma();           //使能接收
}

u16 g_nRS485Sr = 0;
u16 g_nRS485Dr = 0;
void Rs485_IRQHandler(void)
{
    if(RS485_PORT->SR & RS485_SR_IDLE)
    {    
        Rs485_DisableRxDma();
        g_sRs485TempFrame.lenth = Rs485_GetRxLen(); 
        
        //加入校验，通过则写入队列.若数据处理繁琐，可在中断外处理
        if(1)
        {
            if (xQueueSendToBackFromISR(g_sRs485RxQueue, (void *)&g_sRs485TempFrame, NULL) == pdTRUE)
            {   //写入失败处理
              
            }
        }
        Rs485_RstFrame();
        Rs485_EnableRxDma();
    }
    g_nRS485Sr = RS485_PORT->SR;  //通过读取SR和DR清空中断标志
    g_nRS485Dr = RS485_PORT->DR;    
}
//


/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
