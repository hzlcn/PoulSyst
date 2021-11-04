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
  * ���ջ�����
  */
u8  g_uart3_RxTmp = 0;

/**
  * ���ճ�ʱ����
  */
TickTimer_t g_uart3_RxTim;

/**
  * ���ݻ�����
  */
u8  g_uart3_RxBuf[UART3_BUFFSIZE];
u16 g_uart3_RxLen = 0;

/**
  * @brief  ������رմ��ڽ���
  * @param  status  true  - �������ڽ���
  *                 false - �رմ��ڽ���
  * @retval 
  */
void Uart3_Init(void)
{
	HAL_UART_Receive_IT(&huart3, &g_uart3_RxTmp, 1);
}

/**
  * @brief  ��ȡ��������
  * @param  
  * @retval 
  */
uint16_t Uart3_GetData(uint8_t *pBuffer)
{
	u16 rxLen = 0;
	u16 cpyBase = 0;
	u16 cpyLen = 0;
	// ������ɻ���ճ�ʱ
	if (g_uart3_RxLen > 0) {
		if ((g_uart3_RxTim.count > 0) && (HAL_GetTick() - g_uart3_RxTim.start >= g_uart3_RxTim.count)) {
			g_uart3_RxTim.start = 0;
			g_uart3_RxTim.count = 0;
			
			rxLen = g_uart3_RxLen;
			
			// ��������
			while (g_uart3_RxLen > 1024) {
				memcpy(pBuffer + cpyBase, g_uart3_RxBuf + cpyBase, 1024);
				g_uart3_RxLen -= 1024;
				cpyBase += 1024;
			}
			memcpy(pBuffer, g_uart3_RxBuf, g_uart3_RxLen);
			
			// ��ջ�����
			memset(g_uart3_RxBuf, 0, UART3_BUFFSIZE);
			g_uart3_RxLen = 0;
		}
	}
	return rxLen;
}

/**
  * @brief  ���ʹ�������
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
		// ֹͣʱ��
		g_uart3_RxTim.start = HAL_GetTick();
		if (g_uart3_RxTim.count == 0) {
			g_uart3_RxTim.count = 20;
		}
		
		// ��������ӵ����ջ�����
		if (g_uart3_RxLen < UART3_BUFFSIZE) {
			g_uart3_RxBuf[g_uart3_RxLen++] = g_uart3_RxTmp;
		}
		
		// ���´򿪽����ж�
		HAL_UART_Receive_IT(&huart3, &g_uart3_RxTmp, 1);
	}
}

