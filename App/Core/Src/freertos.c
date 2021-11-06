/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sim7600.h"
#include "apps.h"
#include "w5500_ex.h"
#include "drivers.h"
#include "route.h"
#include "debug.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for WireTask */
osThreadId_t WireTaskHandle;
const osThreadAttr_t WireTask_attributes = {
  .name = "WireTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for MobileTask */
osThreadId_t MobileTaskHandle;
const osThreadAttr_t MobileTask_attributes = {
  .name = "MobileTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for RouteTask04 */
osThreadId_t RouteTask04Handle;
const osThreadAttr_t RouteTask04_attributes = {
  .name = "RouteTask04",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for NetTxMutex */
osMutexId_t NetTxMutexHandle;
const osMutexAttr_t NetTxMutex_attributes = {
  .name = "NetTxMutex"
};
/* Definitions for NetRxMutex */
osMutexId_t NetRxMutexHandle;
const osMutexAttr_t NetRxMutex_attributes = {
  .name = "NetRxMutex"
};
/* Definitions for SimTxMutex */
osMutexId_t SimTxMutexHandle;
const osMutexAttr_t SimTxMutex_attributes = {
  .name = "SimTxMutex"
};
/* Definitions for SimRxMutex */
osMutexId_t SimRxMutexHandle;
const osMutexAttr_t SimRxMutex_attributes = {
  .name = "SimRxMutex"
};
/* Definitions for SimMutex */
osMutexId_t SimMutexHandle;
const osMutexAttr_t SimMutex_attributes = {
  .name = "SimMutex"
};
/* Definitions for NetStatusMutex */
osMutexId_t NetStatusMutexHandle;
const osMutexAttr_t NetStatusMutex_attributes = {
  .name = "NetStatusMutex"
};
/* Definitions for NetStatusBinarySem */
osSemaphoreId_t NetStatusBinarySemHandle;
const osSemaphoreAttr_t NetStatusBinarySem_attributes = {
  .name = "NetStatusBinarySem"
};
/* Definitions for SimStatusBinarySem */
osSemaphoreId_t SimStatusBinarySemHandle;
const osSemaphoreAttr_t SimStatusBinarySem_attributes = {
  .name = "SimStatusBinarySem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);
void StartTask04(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of NetTxMutex */
  NetTxMutexHandle = osMutexNew(&NetTxMutex_attributes);

  /* creation of NetRxMutex */
  NetRxMutexHandle = osMutexNew(&NetRxMutex_attributes);

  /* creation of SimTxMutex */
  SimTxMutexHandle = osMutexNew(&SimTxMutex_attributes);

  /* creation of SimRxMutex */
  SimRxMutexHandle = osMutexNew(&SimRxMutex_attributes);

  /* creation of SimMutex */
  SimMutexHandle = osMutexNew(&SimMutex_attributes);

  /* creation of NetStatusMutex */
  NetStatusMutexHandle = osMutexNew(&NetStatusMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of NetStatusBinarySem */
  NetStatusBinarySemHandle = osSemaphoreNew(1, 1, &NetStatusBinarySem_attributes);

  /* creation of SimStatusBinarySem */
  SimStatusBinarySemHandle = osSemaphoreNew(1, 1, &SimStatusBinarySem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of WireTask */
  WireTaskHandle = osThreadNew(StartTask02, NULL, &WireTask_attributes);

  /* creation of MobileTask */
  MobileTaskHandle = osThreadNew(StartTask03, NULL, &MobileTask_attributes);

  /* creation of RouteTask04 */
  RouteTask04Handle = osThreadNew(StartTask04, NULL, &RouteTask04_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
//  HAL_TIM_Base_Start_IT(&htim2);
//	Debug_Init();
  Main_Init();
  /* Infinite loop */
  for(;;)
  {
    Main_Process();
//	  Debug_Process();
//    Ex_WDI_Feed();
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the WireTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  Ethernet_Init();
  /* Infinite loop */
  for(;;)
  {
    Ethernet_Process();
    osDelay(1);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the MobileTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  Sim_Init();
  /* Infinite loop */
  for(;;)
  {
    Sim_Process();
    osDelay(1);
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief Function implementing the RouteTask04 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask04 */
void StartTask04(void *argument)
{
  /* USER CODE BEGIN StartTask04 */
  osDelay(1000);
  /* Infinite loop */
  for(;;)
  {
    // 运行流程: 设备登录->用户登录->客户登录->商品购买
	Route_Process();
    osDelay(1);
  }
  /* USER CODE END StartTask04 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
