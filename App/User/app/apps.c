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
#include "w5500_ex.h"
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
#include "datastruct.h"

/* Private variables ---------------------------------------------------------*/
/**
  * 店主信息库
  */
static MercBase_t g_mercBase = { 0 };

/**
  * 食品信息库
  */
static PoulBase_t g_poulCurBase = { 0 };

/**
  * 添加信息库
  */
static PoulBase_t g_poulAddBase = { 0 };

DataStruct_t g_allData = {
	.temp = {
		.user = {
			.merc = {
				.perm = USER_PERM_NULL,
				.code = { 0 },
			},
			.cust = {
				.perm = USER_PERM_NULL,
				.code = { 0 },
				.name = { 0 },
			},
			.recv = {
				.isReady = false, 
				.code = { 0 },
			},
			.mercBase = &g_mercBase,
		},
		.poul =  {
			.pCurKind = &g_poulCurBase.kinds[0],
			.code = {
				.isReady = false, 
				.qrcode = { 0 },
			},
			.list = { 
				.type = POUL_LISTTYPE_NONE,
				.group = POUL_LISTGROUPTYPE_CHICK,
				.sum = 0,
				.pKinds = { 0 },
			},
			.base = {
				[0] = &g_poulCurBase,
				[1] = &g_poulAddBase,
			},
		},
		.gps = { 0 },
		.route = { 0 },
		.display = {
			.err = {
				[0] = { 0 },
				[1] = { 0x50, 0x4C, 0x43, 0xCE, 0xDE, 0xCD, 0xA8, 0xD1, 0xB6 },					// PLC无通讯
				[2] = { 0x47, 0x50, 0x53, 0xCE, 0xDE, 0xD0, 0xC5, 0xCF, 0xA2 },					// GPS无信息
				[3] = { 0xCD, 0xE2, 0xC9, 0xA8, 0xC2, 0xEB, 0xC6, 0xF7, 0xB3, 0xF6, 0xB4, 0xED }, // 外扫码器出错
				[4] = { 0xC4, 0xDA, 0xC9, 0xA8, 0xC2, 0xEB, 0xC6, 0xF7, 0xB3, 0xF6, 0xB4, 0xED },	// 内扫码器出错
				[5] = { 0xCE, 0xDE, 0xCA, 0xB3, 0xC6, 0xB7, 0xD0, 0xC5, 0xCF, 0xA2 },				// 无食品信息
				[6] = { 0xCE, 0xDE, 0xCE, 0xC0, 0xD0, 0xC7, 0xD0, 0xC5, 0xCF, 0xA2 },				// 无卫星信息
				[7] = { 0xCE, 0xDE, 0xB6, 0xA8, 0xCE, 0xBB, 0xD0, 0xC5, 0xCF, 0xA2 },				// 无定位信息
				[8] = { 0xCE, 0xB4, 0xB9, 0xD8, 0xCB, 0xF8 },                                     // 未关锁
			},
			.wTxt = {
				[0] = { 0x2020, 0x2020, 0xCEDE, 0x2020, 0x2020 },		// TEXT_NULL                  “    无    ”
				[1] = { 0xC7EB, 0xC9CC, 0xBBA7, 0xB5C7, 0xC2BC },		// TEXT_LOGIN                 “请商户登录”
				[2] = { 0xC7EB, 0xBCEC, 0xB2E9, 0xCDF8, 0xC2E7 },		// TEXT_CHECK_NETWORK         “请检查网络”
				[3] = { 0xC9E8, 0xB1B8, 0xCEB4, 0xB5C7, 0xC2BD },		// TEXT_DEVICE_UNLOGIN        “设备未登陆”
				[4] = { 0xC7EB, 0xBFAA, 0xC6F4, 0xBBFA, 0xC6F7 },		// TEXT_CHECK_PLC             “请检查机器”
				[5] = { 0xC7EB, 0xD6D8, 0xD0C2, 0xCBA2, 0xBFA8 },		// TEXT_RECARD                “请重新刷卡”
				[6] = { 0x2020, 0xBBF1, 0xC8A1, 0xD6D0, 0x2020 },		// TEXT_GETTING               “  获取中  ”
				[7] = { 0xC1AC, 0xBDD3, 0xCDF8, 0xC2E7, 0xD6D0 },		// TEXT_NULL                  “连接网络中”
				[8] = { 0xC1AC, 0xBDD3, 0xC9E8, 0xB1B8, 0xD6D0 },		// TEXT_NULL                  “连接设备中”
				[9] = { 0xC7EB, 0xCAE4, 0xC8EB, 0xBFCD, 0xBBA7 },		// TEXT_NULL                  “请输入客户”
				[10] = { 0xB6A8, 0xCEBB, 0xCEB4, 0xBBF1, 0xC8A1 },     // TEXT_GPS_DISABLE           “定位未获取”
				[11] = { 0xC9CC, 0xBBA7, 0xCEB4, 0xD7A2, 0xB2E1 },     // TEXT_MERCHANT_UNSIGNIN     “商户未注册”
				[12] = { 0xBFCD, 0xBBA7, 0xCEB4, 0xD7A2, 0xB2E1 },     // TEXT_CUSTOMER_UNSIGNIN     “客户未注册”
				[13] = { 0xC9CC, 0xBBA7, 0xB5C7, 0xC2BC, 0xD6D0 },     // TEXT_MERCHANT_LOGINING     “商户登录中”
				[14] = { 0x20B5, 0xC7C2, 0xBCCA, 0xA7B0, 0xDC20 },     // TEXT_LOGIN_FAIL            “ 登录失败 ”
				[15] = { 0xBFCD, 0xBBA7, 0xB5C7, 0xC2BC, 0xD6D0 },     //TEXT_CUSTOMER_LOGINING      “客户登录中”
			},
			.opt = { 0 },
		}
	},
};
SavePara_t *g_savPara = &g_allData.save;
TempPara_t *g_tmpPara = &g_allData.temp;

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
	
	// 处理二维码数据
	Poul_Process();
	User_Process();

	// 调试口信息处理
	Debug_Process();
	
	// 显示LCD数据处理
	LCD_Process();

	// Modbus的发送接收处理
	//Modbus_Process();
			
	// GPRS数据解析
	GPS_Process();
	
	// 包含网络发送、接收和断网检测
	Network_Process();
	if ((nwkStatus == NETWORK_STATUS_NET) || (nwkStatus == NETWORK_STATUS_SIM)) {
		// 与服务器通讯
		CMNC_Process();
	}

	g_appTick = HAL_GetTick();
	
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
