#ifndef __A_MAIN_H__
#define __A_MAIN_H__

#include "common.h"

#define LED_TURN    HAL_GPIO_TogglePin( LED_GPIO_Port, LED_Pin )


#define READ_SP0    HAL_GPIO_ReadPin(SP0_IN_GPIO_Port, SP0_IN_Pin)

/**
  * @brief  ??ʼ??app??
  */
void Main_Init( void );

/**
  * @brief  ????app??
  */
void Main_Process( void );

#endif //__A_MAIN_H__
