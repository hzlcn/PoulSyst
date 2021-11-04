#ifndef __SERIAL_H__
#define __SERIAL_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/**
  * 接收缓冲区大小
  */
#define     UART3_BUFFSIZE      5120

/**
  * @brief  开启或关闭串口接收
  */
void        Uart3_Init          (void);

/**
  * @brief  获取串口数据
  */
uint16_t    Uart3_GetData       (uint8_t *pBuffer);

/**
  * @brief  发送串口数据
  */
void        Uart3_Send          (uint8_t *pBuff, uint16_t len);


/*---------------------------------- Common ----------------------------------*/

/**
  * @brief  空闲帧中断处理函数
  */
void        UART_IDLE_IRQHandler(UART_HandleTypeDef *huart);

#endif //__SERIAL_H__
