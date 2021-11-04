/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RC522_RST_Pin GPIO_PIN_3
#define RC522_RST_GPIO_Port GPIOE
#define SPI4_NSS_Pin GPIO_PIN_4
#define SPI4_NSS_GPIO_Port GPIOE
#define UART7_RTS_Pin GPIO_PIN_1
#define UART7_RTS_GPIO_Port GPIOC
#define MPCIE_PWR_Pin GPIO_PIN_4
#define MPCIE_PWR_GPIO_Port GPIOA
#define SPI1_NSS_Pin GPIO_PIN_4
#define SPI1_NSS_GPIO_Port GPIOC
#define W5500_ONOFF_Pin GPIO_PIN_5
#define W5500_ONOFF_GPIO_Port GPIOC
#define E3000_ONOFF_Pin GPIO_PIN_2
#define E3000_ONOFF_GPIO_Port GPIOB
#define SPI2_NSS_Pin GPIO_PIN_12
#define SPI2_NSS_GPIO_Port GPIOB
#define NTRIG2_Pin GPIO_PIN_11
#define NTRIG2_GPIO_Port GPIOD
#define NTRIG1_Pin GPIO_PIN_12
#define NTRIG1_GPIO_Port GPIOD
#define W5500_RST_Pin GPIO_PIN_8
#define W5500_RST_GPIO_Port GPIOC
#define W5500_INT_Pin GPIO_PIN_9
#define W5500_INT_GPIO_Port GPIOC
#define WIFI_ONOFF_Pin GPIO_PIN_8
#define WIFI_ONOFF_GPIO_Port GPIOA
#define GSM_STA_Pin GPIO_PIN_15
#define GSM_STA_GPIO_Port GPIOA
#define BEEP_Pin GPIO_PIN_10
#define BEEP_GPIO_Port GPIOC
#define LTDC_ONOFF_Pin GPIO_PIN_11
#define LTDC_ONOFF_GPIO_Port GPIOC
#define LED_Pin GPIO_PIN_4
#define LED_GPIO_Port GPIOD
#define WDI_Pin GPIO_PIN_6
#define WDI_GPIO_Port GPIOB
#define SP0_IN_Pin GPIO_PIN_8
#define SP0_IN_GPIO_Port GPIOB
#define SP0_IN_EXTI_IRQn EXTI9_5_IRQn
#define GPS_CTL_Pin GPIO_PIN_9
#define GPS_CTL_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
