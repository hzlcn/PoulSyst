#ifndef __SERIAL_H__
#define __SERIAL_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/**
  * ���ջ�������С
  */
#define     UART3_BUFFSIZE      5120

/**
  * @brief  ������رմ��ڽ���
  */
void        Uart3_Init          (void);

/**
  * @brief  ��ȡ��������
  */
uint16_t    Uart3_GetData       (uint8_t *pBuffer);

/**
  * @brief  ���ʹ�������
  */
void        Uart3_Send          (uint8_t *pBuff, uint16_t len);


/*---------------------------------- Common ----------------------------------*/

/**
  * @brief  ����֡�жϴ�����
  */
void        UART_IDLE_IRQHandler(UART_HandleTypeDef *huart);

#endif //__SERIAL_H__
