#ifndef __DRIVERS_H__
#define __DRIVERS_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "common.h"

#include "main.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"


#include "rtctime.h"
#include "serial.h"

/* Private define ------------------------------------------------------------*/

#define RS485_RTS_ON                    HAL_GPIO_WritePin(UART7_RTS_GPIO_Port, UART7_RTS_Pin ,GPIO_PIN_SET)
#define RS485_RTS_OFF                   HAL_GPIO_WritePin(UART7_RTS_GPIO_Port, UART7_RTS_Pin ,GPIO_PIN_RESET)

#define E3000_ON    HAL_GPIO_WritePin( E3000_ONOFF_GPIO_Port, E3000_ONOFF_Pin, GPIO_PIN_SET )
#define E3000_OFF   HAL_GPIO_WritePin( E3000_ONOFF_GPIO_Port, E3000_ONOFF_Pin, GPIO_PIN_RESET )

/*----------------------------------- UART -----------------------------------*/

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_uart4_rx;
extern DMA_HandleTypeDef hdma_uart5_rx;
extern DMA_HandleTypeDef hdma_usart6_rx;
extern DMA_HandleTypeDef hdma_uart7_rx;
extern DMA_HandleTypeDef hdma_uart8_rx;

void Ex_WDI_Feed(void);

void PrintHexBuffer( uint8_t *buffer, uint8_t len );
void PrintCharBuffer(char *buf, uint8_t len);

void Beep_Run(u16 timeout);

/**
  * @brief  W5500-SPI-NSS control
  */
void WIZ_CS(uint8_t val);

/**
  * @brief  SPI1 send one byte
  */
uint8_t SPI2_SendByte(uint8_t TxData);

#endif /* __DRIVERS_H__ */
