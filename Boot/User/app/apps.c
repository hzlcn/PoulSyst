/**
  ******************************************************************************
  * @file           : a_main.c
  * @brief          : Application main program body
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "apps.h"
#include "upgrade.h"
#include "iap.h"
#include "delay.h"
#include "serial.h"
#include "drivers.h"

/* Private variables ---------------------------------------------------------*/
/**
  * 定时计数
  */
uint32_t g_appTick = 0;
static TickTimer_t g_app50msTime = { .start = 0, .count = 50 };

//static u8  g_rxBuf[5120];
//static u16 g_rxLen;

//static u8 g_mode;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Initial app layer
  * @param  
  * @retval 
  */
void Main_Init(void)
{
	printf("*************** Version 1.1.8 ***************\r\n");
	
	delay_init(180);
	
	Upg_Init();
	
//	Uart3_Init();
}

/**
  * @brief  Process app layer
  * @param  
  * @retval 
  */
void Main_Process(void)
{	
//	// 接收串口
//	g_rxLen = Uart3_GetData(g_rxBuf);
//	if (g_rxLen > 0) {
//		if (g_mode == 0) {
//			if ((g_rxBuf[0] == 0x55) && (g_rxBuf[g_rxLen - 1] == 0x55)) {
//				if (g_rxBuf[1] == 1) {
//					g_mode = g_rxBuf[2];
//				}
//			}
//		} else if (g_mode == 1) {
//			Upg_SaveFrag(g_rxBuf);
//		}
//	}
	
	// 更新处理
	if (Upg_GetInfo()->upgFlag == true) {
		Upg_Process();
	} else {
		// 跳转app
		iap_load_app(APP_BASE_ADDR);
	}
	
	g_appTick = HAL_GetTick();
	
	// 50ms处理
	if ((g_app50msTime.count > 0) && (g_appTick - g_app50msTime.start >= g_app50msTime.count)) {
		g_app50msTime.start = g_appTick;
		
		// 外部看门狗
		Ex_WDI_Feed();
	}	
}
