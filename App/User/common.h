/**
  ******************************************************************************
  * @file           : common.h
  * @brief          : Header for application file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMON_H__
#define __COMMON_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
//#include "iwdg.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;

typedef volatile uint8_t  vu8;
typedef volatile uint16_t vu16;
typedef volatile uint32_t vu32;

typedef enum {
    FUN_OK = 1,
    FUN_ERROR = 0,
}Fun_Status_t;

/**
  * 定时计数器
  */
typedef struct {
	uint32_t start;
	uint32_t count;
}TickTimer_t;

/* Private define ------------------------------------------------------------*/

/*!
 * \brief Returns the minimum value between a and b
 *
 * \param [IN] a 1st value
 * \param [IN] b 2nd value
 * \retval minValue Minimum value
 */
#ifndef MIN
#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#endif

/*!
 * \brief Returns the maximum value between a and b
 *
 * \param [IN] a 1st value
 * \param [IN] b 2nd value
 * \retval maxValue Maximum value
 */
#ifndef MAX
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif

#define DEBUG_CODE    0

#endif /* __COMMON_H__ */
