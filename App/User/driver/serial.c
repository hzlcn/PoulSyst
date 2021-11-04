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
  * 串口开关状态标志
  */
bool     g_uart2_RxEnabled = false;

/**
  * 数据缓冲区
  */
uint8_t  g_uart2_RxBuf[UART2_BUFFSIZE];
uint16_t g_uart2_RxLen = 0;
uint8_t  g_uart3_RxTmp = 0;
uint8_t  g_uart3_RxBuf[UART3_BUFFSIZE];
uint16_t g_uart3_RxLen = 0;
uint8_t  g_uart4_RxTmp = 0;
uint8_t  g_uart4_RxBuf[UART4_BUFFSIZE];
uint16_t g_uart4_RxLen;
uint8_t  g_uart5_RxBuf[UART5_BUFFSIZE];
uint16_t g_uart5_RxLen = 0;
uint8_t  g_uart6_RxTmp = 0;
uint8_t  g_uart6_RxBuf[UART6_BUFFSIZE];
uint16_t g_uart6_RxLen = 0;
uint8_t  g_uart7_RxBuf[UART7_BUFFSIZE];
uint16_t g_uart7_RxLen = 0;
uint8_t  g_uart8_RxBuf[UART8_BUFFSIZE];
uint16_t g_uart8_RxLen = 0;

/**
  * 接收超时计数
  */
u32 g_uart3_RxTim;
u32 g_uart4_RxTim;
u32 g_uart6_RxTim;


u32 g_uartTick;

/**
  * @brief  开启或关闭串口接收
  */
void Uart_SetRxEnable(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2) {
		__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
		HAL_UART_Receive_DMA(&huart2, g_uart2_RxBuf, UART2_BUFFSIZE);
	} else if (huart->Instance == USART3) {
		HAL_UART_Receive_IT(&huart3, &g_uart3_RxTmp, 1);
	} else if (huart->Instance == UART4) {
		HAL_UART_Receive_IT(&huart4, &g_uart4_RxTmp, 1);
	} else if (huart->Instance == UART5) {
		__HAL_UART_ENABLE_IT(&huart5, UART_IT_IDLE);
		HAL_UART_Receive_DMA(&huart5, g_uart5_RxBuf, UART5_BUFFSIZE);
	} else if (huart->Instance == USART6) {
		HAL_UART_Receive_IT(&huart6, &g_uart6_RxTmp, 1);
	} else if (huart->Instance == UART7) {
		__HAL_UART_ENABLE_IT(&huart7, UART_IT_IDLE);
		HAL_UART_Receive_DMA(&huart7, g_uart7_RxBuf, UART7_BUFFSIZE);
	} else if (huart->Instance == UART8) {
		__HAL_UART_ENABLE_IT(&huart8, UART_IT_IDLE);
		HAL_UART_Receive_DMA(&huart8, g_uart8_RxBuf, UART8_BUFFSIZE);		
	}
}

/**
  * @brief  获取数据
  * @param  
  * @retval 
  */
u16 Uart_GetData(UART_HandleTypeDef *huart, u8 *pBuf)
{
	u8 *uartRxBuf = NULL;
	u16 *uartRxLen = NULL;
	u16 uartRxSize = 0;
	u32 uartRxTim = 0;
	u16 retLen = 0;

	if (huart->Instance == USART2) {
		uartRxBuf = g_uart2_RxBuf;
		uartRxLen = &g_uart2_RxLen;
		uartRxSize = UART2_BUFFSIZE;
	} else if (huart->Instance == USART3) {
		uartRxBuf = g_uart3_RxBuf;
		uartRxLen = &g_uart3_RxLen;
		uartRxSize = UART3_BUFFSIZE;
		uartRxTim = g_uart3_RxTim;
	} else if (huart->Instance == UART4) {
		uartRxBuf = g_uart4_RxBuf;
		uartRxLen = &g_uart4_RxLen;
		uartRxSize = UART4_BUFFSIZE;
		uartRxTim = g_uart4_RxTim;
	} else if (huart->Instance == UART5) {
		uartRxBuf = g_uart5_RxBuf;
		uartRxLen = &g_uart5_RxLen;
		uartRxSize = UART5_BUFFSIZE;
	} else if (huart->Instance == USART6) {
		uartRxBuf = g_uart6_RxBuf;
		uartRxLen = &g_uart6_RxLen;
		uartRxSize = UART6_BUFFSIZE;
		uartRxTim = g_uart6_RxTim;
	} else if (huart->Instance == UART7) {
		uartRxBuf = g_uart7_RxBuf;
		uartRxLen = &g_uart7_RxLen;
		uartRxSize = UART7_BUFFSIZE;
	} else if (huart->Instance == UART8) {
		uartRxBuf = g_uart8_RxBuf;
		uartRxLen = &g_uart8_RxLen;
		uartRxSize = UART8_BUFFSIZE;
	} else {
		return 0;
	}
	
	if (*uartRxLen > 0) {
		if ((*uartRxLen >= uartRxSize) || (uartRxTim == 0)) {
			if (huart->Instance == USART3) {
				__NOP();
			}
			
			// 复制数据
			memcpy(pBuf, uartRxBuf, *uartRxLen);
			retLen = *uartRxLen;
			
			// 清空缓冲区
			memset(uartRxBuf, 0, uartRxSize);
			*uartRxLen = 0;
		}

		if (huart->Instance == USART2) {
			__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
			HAL_UART_Receive_DMA(&huart2, g_uart2_RxBuf, UART2_BUFFSIZE);
		} else if (huart->Instance == UART4) {
			// 重新打开接收
			HAL_UART_Receive_IT(&huart4, &g_uart4_RxTmp, 1);
		} else if (huart->Instance == UART5) {
			// 重新打开接收
			__HAL_UART_ENABLE_IT(&huart5, UART_IT_IDLE);
			HAL_UART_Receive_DMA(&huart5, g_uart5_RxBuf, UART5_BUFFSIZE);
		} else if (huart->Instance == UART7) {
			// 开启串口7接收
			__HAL_UART_ENABLE_IT(&huart7, UART_IT_IDLE);
			HAL_UART_Receive_DMA(&huart7, g_uart7_RxBuf, UART7_BUFFSIZE);
		}

	}

	return retLen;
}

/**
  * @brief  处理定时
  * @param  
  * @retval 
  */
void Uart_Timer_Handler(void)
{
	if (g_uart3_RxTim > 0) {
		g_uart3_RxTim--;
	}
	if (g_uart4_RxTim > 0) {
		g_uart4_RxTim--;
	}
	if (g_uart6_RxTim > 0) {
		g_uart6_RxTim--;
	}
}

/**
  * @brief  获取串口开关状态
  * @param  
  * @retval 
  */
bool Uart2_IsRxEnable(void)
{
	return g_uart2_RxEnabled;
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

/**
  * @brief  发送串口数据
  * @param  
  * @retval 
  */
void Uart4_Send(uint8_t *pBuffer, uint16_t len)
{
	HAL_UART_Transmit(&huart4, pBuffer, len, 1000);
}

/**
  * @brief  发送串口数据
  */
void Uart5_Send(uint8_t *pBuf, uint16_t len)
{
	HAL_UART_Transmit(&huart5, pBuf, len, 1000);
}

/**
  * @brief  发送串口数据
  */
void Uart6_Send(uint8_t *pBuf, uint16_t txLen)
{
	HAL_UART_Transmit(&huart6, pBuf, txLen, 0xFFFF);
}

/**
  * @brief  发送串口数据
  * @param  
  * @retval 
  */
void Uart7_Send(uint8_t *pBuff, uint16_t len)
{
	RS485_RTS_ON;   // 发送模式
	HAL_UART_Transmit(&huart7, pBuff, len, 1000);
	RS485_RTS_OFF;  // 接收模式
}

/*---------------------------------- Common ----------------------------------*/

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huartx)
{
	if (huartx->Instance == USART3) {
		// 停止时间
		g_uart3_RxTim = 10;
		
		// 把数据添加到接收缓冲区
		if (g_uart3_RxLen < UART3_BUFFSIZE) {
			g_uart3_RxBuf[g_uart3_RxLen++] = g_uart3_RxTmp;
		}
		
		// 重新打开接收中断
		HAL_UART_Receive_IT(&huart3, &g_uart3_RxTmp, 1);
	}
	else if (huartx->Instance == UART4) {
		g_uart4_RxTim = 10;
		if (g_uart4_RxLen < UART4_BUFFSIZE) {
			g_uart4_RxBuf[g_uart4_RxLen++] = g_uart4_RxTmp;
		}
		HAL_UART_Receive_IT(&huart4, &g_uart4_RxTmp, 1);
	}
	else if (huartx->Instance == USART6) {
		g_uart6_RxTim = 10;
		if (g_uart6_RxLen < UART6_BUFFSIZE) {
			g_uart6_RxBuf[g_uart6_RxLen++] = g_uart6_RxTmp;
		}
		HAL_UART_Receive_IT(&huart6, &g_uart6_RxTmp, 1);
	}
}

/**
  * @brief  空闲帧中断处理函数
  * @param  
  * @retval 
  */
void UART_IDLE_IRQHandler(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2) {
		if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) == true) {
			__HAL_UART_CLEAR_IDLEFLAG(&huart2); // 清除标志
			
			// 停止接收
			__HAL_UART_DISABLE_IT(&huart2, UART_IT_IDLE);
			HAL_UART_DMAStop(&huart2);
			
			// 获取数据长度
			g_uart2_RxLen = UART2_BUFFSIZE - hdma_usart2_rx.Instance->NDTR;
			if (g_uart2_RxLen == 0) {
				// 重新开始接收
				__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
				HAL_UART_Receive_DMA(&huart2, g_uart2_RxBuf, UART2_BUFFSIZE);
			}
		}
	} else if (huart->Instance == UART5) {
		if (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_IDLE) == true) {
			__HAL_UART_CLEAR_IDLEFLAG(&huart5);
			__HAL_UART_DISABLE_IT(&huart5, UART_IT_IDLE);
			HAL_UART_DMAStop(&huart5);
			g_uart5_RxLen = UART5_BUFFSIZE - hdma_uart5_rx.Instance->NDTR;
			if (g_uart5_RxLen == 0) {
				__HAL_UART_ENABLE_IT(&huart5, UART_IT_IDLE);
				HAL_UART_Receive_DMA(&huart5, g_uart5_RxBuf, UART5_BUFFSIZE);
			}
		}
	} else if (huart->Instance == UART7) {
		if (__HAL_UART_GET_FLAG(&huart7, UART_FLAG_IDLE) == true) {
			__HAL_UART_CLEAR_IDLEFLAG(&huart7);
			__HAL_UART_DISABLE_IT(&huart7, UART_IT_IDLE);
			HAL_UART_DMAStop(&huart7);
			g_uart7_RxLen = UART7_BUFFSIZE - hdma_uart7_rx.Instance->NDTR;
			if (g_uart7_RxLen == 0) {
				__HAL_UART_ENABLE_IT(&huart7, UART_IT_IDLE);
				HAL_UART_Receive_DMA(&huart7, g_uart7_RxBuf, UART7_BUFFSIZE);
			}
		}
	} else if (huart->Instance == UART8) {
		if (__HAL_UART_GET_FLAG(&huart8, UART_FLAG_IDLE) == true) {
			__HAL_UART_CLEAR_IDLEFLAG(&huart8);
			__HAL_UART_DISABLE_IT(&huart8, UART_IT_IDLE);
			HAL_UART_DMAStop(&huart8);
			g_uart8_RxLen = UART8_BUFFSIZE - hdma_uart8_rx.Instance->NDTR;
			if (g_uart8_RxLen == 0) {
				__HAL_UART_ENABLE_IT(&huart8, UART_IT_IDLE);
				HAL_UART_Receive_DMA(&huart8, g_uart8_RxBuf, UART8_BUFFSIZE);
			}
		}
	}
}

