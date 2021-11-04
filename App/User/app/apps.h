#ifndef __A_MAIN_H__
#define __A_MAIN_H__

#include "common.h"

#include "debug.h"
#include "network.h"
#include "modbus.h"
#include "communicate.h"
#include "display.h"
#include "ethernet.h"
#include "gps.h"
#include "store.h"



#define LED_TURN    HAL_GPIO_TogglePin( LED_GPIO_Port, LED_Pin )


#define READ_SP0    HAL_GPIO_ReadPin(SP0_IN_GPIO_Port, SP0_IN_Pin)

/**
  * @brief  初始化app层
  */
void Main_Init( void );

/**
  * @brief  处理app层
  */
void Main_Process( void );

#endif //__A_MAIN_H__
