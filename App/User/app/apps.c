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
#include "delay.h"
#include "debug.h"
#include "common.h"
#include "network.h"
#include "ethernet.h"
#include "display.h"
#include "modbus.h"
#include "w25qxx.h"
#include "route.h"
#include "param.h"
#include "serial.h"
#include "gps.h"
#include "store.h"
#include "usermsg.h"
#include "drivers.h"
#include "lcd.h"
#include "poultry.h"
#include "stmflash.h"

/* Private variables ---------------------------------------------------------*/
/**
  * 定时计数
  */
static uint32_t g_appTick = 0;
static TickTimer_t g_app50msTime = { .start = 0, .count = 50 };
static TickTimer_t g_app1sTime = { .start = 0, .count = 1000 };
static TickTimer_t g_app10sTime = { .start = 0, .count = 10000 };
static TickTimer_t g_app60sTime = { .start = 0, .count = 60000 };

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Initial app layer
  * @param  
  * @retval 
  */
void Main_Init(void)
{
	uint8_t i = 0;
	
	printf( "\r\n###### ====== Poultry System v1.2.0 ======= ######\r\n" );
	
	delay_init(180);
	HAL_TIM_Base_Start_IT(&htim2);
	
	// 读取初始化配置
	W25QXX_Init();
	Store_Init();

	// 初始化二维码模块
	E3000_ON;
	Poul_InitModule();
	User_InitModule();

	// 初始化调试打印功能
	Debug_Init();
	
	// 初始化显示器显示功能
	Display_Init();	

	// 初始化贴标机通讯功能
	Modbus_Init();

	// 初始化Gps功能
	GPS_Init();

	// 初始化服务器通讯
	Network_Init();
	CMNC_Init();
	
	// 初始化错误码
	Error_Init();
	
	// 显示器开启延时5秒
	for	(i = 0; i < 100; i++) {
		osDelay(50);
		Ex_WDI_Feed();
	}
	
	// 更新登录界面显示
	Display_SetShow(DISPLAY_SHOW_LOGINPAGE);
}

/**
  * @brief  Process app layer
  * @param  
  * @retval 
  */
void Main_Process(void)
{
	NwkStatus_t nwkStatus = Network_GetStatus();
	g_appTick = HAL_GetTick();
	
	
	// 处理二维码数据
	Poul_Process();
	User_Process();

	// 调试口信息处理
	Debug_Process();
	
	// 显示LCD数据处理
	LCD_Process();
	Display_Process();

	// Modbus的发送接收处理
	Modbus_Process();
			
	// GPRS数据解析
	GPS_Process();
	
	// 包含网络发送、接收和断网检测
	Network_Process();
	if ((nwkStatus == NETWORK_STATUS_NET) || (nwkStatus == NETWORK_STATUS_SIM)) {
		// 与服务器通讯
		CMNC_Process();
	}

	// 运行流程: 设备登录->用户登录->客户登录->商品购买
	Route_Process();

	// 50ms处理
	if ((g_app50msTime.count > 0) && (g_appTick - g_app50msTime.start >= g_app50msTime.count)) {
		g_app50msTime.start = g_appTick;
		
		// 外部看门狗
		Ex_WDI_Feed();
	}
	
	// 1秒处理
	if ((g_app1sTime.count > 0) && (g_appTick - g_app1sTime.start >= g_app1sTime.count)) {
		g_app1sTime.start = g_appTick;
		
		// 运行状态灯
		LED_TURN;
		
		// 获取门锁状态 + 标签数
		Modbus_AddCmd(MODBUS_CMD_GETLOCKSTATUS);
		Modbus_AddCmd(MODBUS_CMD_READ_VALIBNUM);
		Modbus_AddCmd(MODBUS_CMD_READ_INVALNUM);
	}
	
	// 5秒处理
	if ((g_app10sTime.count > 0) && (g_appTick - g_app10sTime.start >= g_app10sTime.count)) {
		g_app10sTime.start = g_appTick;
		
		// 检查信息上报 --------------------------------------------------------
		Store_Process();

	}
	
	// 60秒处理
	if ((g_app60sTime.count > 0) && (g_appTick - g_app60sTime.start >= g_app60sTime.count)) {
		g_app60sTime.start = g_appTick;		
		
		// 服务器通讯 - 心跳包
		CMNC_AddCmd(UL_HEARTBEAT);
	}
}
