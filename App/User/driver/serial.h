#ifndef __SERIAL_H__
#define __SERIAL_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/**
  * 接收缓冲区大小
  */
#define     UART2_BUFFSIZE      100
#define     UART3_BUFFSIZE      100
#define     UART4_BUFFSIZE      100
#define     UART5_BUFFSIZE      256
#define     UART6_BUFFSIZE      2048
#define     UART7_BUFFSIZE      20
#define     UART8_BUFFSIZE      4096

/**
  * @brief  获取串口开关状态
  */
bool        Uart2_IsRxEnable    (void);

/**
  * @brief  发送串口数据
  */
void        Uart3_Send          (uint8_t *pBuff, uint16_t len);

/**
  * @brief  发送串口数据
  */
void        Uart4_Send          (uint8_t *pBuffer, uint16_t len);

/**
  * @brief  发送串口数据
  */
void        Uart5_Send          (uint8_t *pBuf, uint16_t len);

/**
  * @brief  发送串口数据
  */
void        Uart6_Send          (uint8_t *pBuf, uint16_t txLen);

/**
  * @brief  发送串口数据
  */
void        Uart7_Send          (uint8_t *pBuff, uint16_t len);

/**
  * @brief  开启或关闭串口接收
  */
void        Uart_SetRxEnable    (UART_HandleTypeDef *huart);

/**
  * @brief  获取数据
  */
u16         Uart_GetData        (UART_HandleTypeDef *huart, u8 *pBuf);

/**
  * @brief  处理定时
  */
void        Uart_Timer_Handler  (void);

/**
  * @brief  空闲帧中断处理函数
  */
void        UART_IDLE_IRQHandler(UART_HandleTypeDef *huart);

#endif //__SERIAL_H__
