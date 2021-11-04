/**
  ******************************************************************************
  * \File Name         : serial.c
  *
  * \Description       : This file provides personal serial implement method 
  *                      by HuZhuoli, it is contain serial with DMA and serial 
  *                      without DMA.
  *
  * \Version           : V1.4.0
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "serial.h"
#include "drivers.h"

/**
  * 接收缓冲区
  */
u8  g_uart3_RxTmp = 0;

/**
  * 接收超时计数
  */
TickTimer_t g_uart3_RxTim;

/**
  * 数据缓冲区
  */
u8  g_uart3_RxBuf[UART3_BUFFSIZE];
u16 g_uart3_RxLen = 0;

/**
  * @brief  开启或关闭串口接收
  * @param  status  true  - 开启串口接收
  *                 false - 关闭串口接收
  * @retval 
  */
void Uart3_Init(void)
{
	HAL_UART_Receive_IT(&huart3, &g_uart3_RxTmp, 1);
}

/**
  * @brief  获取串口数据
  * @param  
  * @retval 
  */
uint16_t Uart3_GetData(uint8_t *pBuffer)
{
	u16 rxLen = 0;
	u16 cpyBase = 0;
	u16 cpyLen = 0;
	// 接收完成或接收超时
	if (g_uart3_RxLen > 0) {
		if ((g_uart3_RxTim.count > 0) && (HAL_GetTick() - g_uart3_RxTim.start >= g_uart3_RxTim.count)) {
			g_uart3_RxTim.start = 0;
			g_uart3_RxTim.count = 0;
			
			rxLen = g_uart3_RxLen;
			
			// 复制数据
			while (g_uart3_RxLen > 1024) {
				memcpy(pBuffer + cpyBase, g_uart3_RxBuf + cpyBase, 1024);
				g_uart3_RxLen -= 1024;
				cpyBase += 1024;
			}
			memcpy(pBuffer, g_uart3_RxBuf, g_uart3_RxLen);
			
			// 清空缓冲区
			memset(g_uart3_RxBuf, 0, UART3_BUFFSIZE);
			g_uart3_RxLen = 0;
		}
	}
	return rxLen;
}

/**
  * @brief  发送串口数据
  * @param  
  * @retval 
  */
void Uart3_Send(uint8_t *pBuff, uint16_t len)
{
	HAL_UART_Transmit(&huart3, pBuff, len, 0xFFFF);
}

/*---------------------------------- Common ----------------------------------*/

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huartx)
{
	if (huartx->Instance == USART3) {
		// 停止时间
		g_uart3_RxTim.start = HAL_GetTick();
		if (g_uart3_RxTim.count == 0) {
			g_uart3_RxTim.count = 20;
		}
		
		// 把数据添加到接收缓冲区
		if (g_uart3_RxLen < UART3_BUFFSIZE) {
			g_uart3_RxBuf[g_uart3_RxLen++] = g_uart3_RxTmp;
		}
		
		// 重新打开接收中断
		HAL_UART_Receive_IT(&huart3, &g_uart3_RxTmp, 1);
	}
}

