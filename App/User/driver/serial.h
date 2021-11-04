#ifndef __SERIAL_H__
#define __SERIAL_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/**
  * ���ջ�������С
  */
#define     UART2_BUFFSIZE      100
#define     UART3_BUFFSIZE      100
#define     UART4_BUFFSIZE      100
#define     UART5_BUFFSIZE      256
#define     UART6_BUFFSIZE      2048
#define     UART7_BUFFSIZE      20
#define     UART8_BUFFSIZE      4096

/**
  * @brief  ��ȡ���ڿ���״̬
  */
bool        Uart2_IsRxEnable    (void);

/**
  * @brief  ���ʹ�������
  */
void        Uart3_Send          (uint8_t *pBuff, uint16_t len);

/**
  * @brief  ���ʹ�������
  */
void        Uart4_Send          (uint8_t *pBuffer, uint16_t len);

/**
  * @brief  ���ʹ�������
  */
void        Uart5_Send          (uint8_t *pBuf, uint16_t len);

/**
  * @brief  ���ʹ�������
  */
void        Uart6_Send          (uint8_t *pBuf, uint16_t txLen);

/**
  * @brief  ���ʹ�������
  */
void        Uart7_Send          (uint8_t *pBuff, uint16_t len);

/**
  * @brief  ������رմ��ڽ���
  */
void        Uart_SetRxEnable    (UART_HandleTypeDef *huart);

/**
  * @brief  ��ȡ����
  */
u16         Uart_GetData        (UART_HandleTypeDef *huart, u8 *pBuf);

/**
  * @brief  ����ʱ
  */
void        Uart_Timer_Handler  (void);

/**
  * @brief  ����֡�жϴ�����
  */
void        UART_IDLE_IRQHandler(UART_HandleTypeDef *huart);

#endif //__SERIAL_H__
