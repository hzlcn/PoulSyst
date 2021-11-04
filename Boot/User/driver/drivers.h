#ifndef __DRIVERS_H__
#define __DRIVERS_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "common.h"

#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "usart.h"

/* Private define ------------------------------------------------------------*/

#define RS485_RTS_ON                    HAL_GPIO_WritePin(UART7_RTS_GPIO_Port, UART7_RTS_Pin ,GPIO_PIN_SET)
#define RS485_RTS_OFF                   HAL_GPIO_WritePin(UART7_RTS_GPIO_Port, UART7_RTS_Pin ,GPIO_PIN_RESET)

#define E3000_ON    HAL_GPIO_WritePin( E3000_ONOFF_GPIO_Port, E3000_ONOFF_Pin, GPIO_PIN_SET )
#define E3000_OFF   HAL_GPIO_WritePin( E3000_ONOFF_GPIO_Port, E3000_ONOFF_Pin, GPIO_PIN_RESET )


void Ex_WDI_Feed(void);

void PrintHexBuffer( uint8_t *buffer, uint8_t len );
void PrintCharBuffer(char *buf, uint8_t len);

/**
  * @brief  W5500-SPI-NSS control
  */
void WIZ_CS(uint8_t val);

/**
  * @brief  SPI1 send one byte
  */
uint8_t SPI2_SendByte(uint8_t TxData);

#endif /* __DRIVERS_H__ */
