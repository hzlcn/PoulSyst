#include "route.h"
#include "param.h"
#include "apps.h"
#include "display.h"
#include "usermsg.h"
#include "poultry.h"
#include "drivers.h"
#include "sim.h"
#include "param.h"
#include "upgrade.h"

/* Private define ------------------------------------------------------------*/
#define DBG_TRACE                                   1

#if DBG_TRACE == 1
    #include <stdio.h>
    /*!
     * Works in the same way as the printf function does.
     */
    #define DBG( ... )                               \
        do                                           \
        {                                            \
            printf( __VA_ARGS__ );                   \
        }while( 0 )
#else
    #define DBG( fmt, ... )
#endif

/* Private macro -------------------------------------------------------------*/

#define ROUTE_TRIGGER_TIMEOUT    3000	// 触发一次后，需要等待4秒才能再次触发
#define ROUTE_LOGIN_TIMEOUT      3000   // 登录店主后，需要等待3秒才能登录客户

/* Private variables ---------------------------------------------------------*/

/**
  * 流程参数
  */
static RouteInfo_t g_route_Info = { 
	.status = ROUTE_INIT_MODE,
	.loginStatus = false,
	.trig = false,
	.updateSoft = false,
};

/**
  * 定时计数
  */
static uint32_t g_routeTick = 0;
static TickTimer_t g_routeTrigTimeout;
static TickTimer_t g_routeLogin1Time;
static TickTimer_t g_routeLogin2Time;
static TickTimer_t g_routeWhile1sTime = { .start = 0, .count = 1000 };
static TickTimer_t g_routeWhile5sTime = { .start = 0, .count = 5000 };

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  运行用户登录流程
  */
static void Route_UserLogin(void);

/**
  * @brief  处理系统流程
  * @param  
  * @retval 
  */
void Route_Process(void)
{
	Merchant_t *merchant = &User_GetInfo()->merc;
	Customer_t *customer = &User_GetInfo()->cust;
	PoulCode_t *poulCode = &Poul_GetInfo()->code;
	ModbusInfo_t *modbusInfo = Modbus_GetParam();

	g_routeTick = HAL_GetTick();

	switch (g_route_Info.status) {
		case ROUTE_INIT_MODE:
		{   // 初始化
			g_route_Info.status = ROUTE_DEVICE_LINK;
			Display_SetLoginText(TEXT_DEVICE_UNLOGIN);
			
//			g_route_Info.status = ROUTE_MERCHANT_LOGIN;
//			Display_SetLoginText(TEXT_LOGIN);   // 显示"请商户登录"
//			User_InitMerc();                    // 初始化店主
			break;
		}
		case ROUTE_DEVICE_LINK:
		{
			if (modbusInfo->devJoin == true) {
				if (modbusInfo->regList[7].data == 0x0000) {
					g_route_Info.status = ROUTE_MERCHANT_LOGIN;
					Display_SetLoginText(TEXT_LOGIN);   // 显示"请商户登录"
					User_InitMerc();                    // 初始化店主
				}
			}
			break;
		}
		case ROUTE_MERCHANT_LOGIN:
		{
			Route_UserLogin();
			break;
		}
		case ROUTE_CUSTOMER_LOGIN:
		{			
			Route_UserLogin();
			break;
		}
		case ROUTE_PRODUCT_SHOP:
		{
			// 检查贴标触发
			if (READ_SP0 == GPIO_PIN_RESET) {
				if (g_route_Info.trig == true) {
					g_route_Info.trig = false;

					// 1.发出鸣响
					Beep_Run(50);

					// 2.控制贴标机贴标签
					Modbus_AddCmd(MODBUS_CMD_TRIGGER_LABEL);

					// 3.保存交易信息
					if (merchant->perm != USER_PERM_MAINTAIN)  {
						Store_SaveBlock();
						Store_SaveRecordNum();
					}

					// 4.触发间隔时间
					g_routeTrigTimeout.start = g_routeTick;
					g_routeTrigTimeout.count = ROUTE_TRIGGER_TIMEOUT;
				}
			} else {
				if ((poulCode->isReady == true) &&		// 检查商品码状态
					(g_routeTrigTimeout.count == 0))		// 触发间隔时间计时完成
				{
					g_route_Info.trig = true;
				}
			}

			// 检查客户是否退出
			if (customer->perm == USER_PERM_NULL) {
				User_InitCust();
				Display_SetLoginText(TEXT_LOGIN);

				g_routeTrigTimeout.start = 0;
				g_routeTrigTimeout.count = 0;

				g_route_Info.status = ROUTE_CUSTOMER_LOGIN;
			}

			// 检查定时计数
			if ((g_routeTrigTimeout.count > 0) && (g_routeTick - g_routeTrigTimeout.start >= g_routeTrigTimeout.count)) {
				g_routeTrigTimeout.start = 0;
				g_routeTrigTimeout.count = 0;
			}
			break;
		}
		case ROUTE_ADMIN_MODE:
		{
			if (Display_GetPage() == DISPLAY_PAGE_WORK) {
				// 显示工作页面
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);
				DBG("product shop.\r\n");
				
				g_route_Info.status = ROUTE_PRODUCT_SHOP;
			}
			break;
		}
		default:
			break;
	}

	// 1秒处理
	if (g_routeTick - g_routeWhile1sTime.start >= g_routeWhile1sTime.count){
		g_routeWhile1sTime.start = g_routeTick;

		// 显示错误码 ----------------------------------------------------------
		Display_SetShow(DISPLAY_SHOW_ERRCODE);

		// 显示有效标签数 ------------------------------------------------------
		Display_SetShow(DISPLAY_SHOW_LABELNUM);

		// 处理网络图标 --------------------------------------------------------
		Display_SetShow(DISPLAY_SHOW_NETICON);
	}
	
	// 5s处理
	if (g_routeTick - g_routeWhile5sTime.start >= g_routeWhile5sTime.count){
		g_routeWhile5sTime.start = g_routeTick;
		
		// 显示GPS信息 ---------------------------------------------------------
		Display_SetShow(DISPLAY_SHOW_GPSSNR);
		
		// 显示时间 ------------------------------------------------------------
		Display_SetShow(DISPLAY_SHOW_RTCTIME);
				
		// 软件更新 ------------------------------------------------------------
		if (g_route_Info.updateSoft == true) {
			if (Upg_GetInfo()->firmIndex < Upg_GetInfo()->firmSize) {
				CMNC_AddCmd(UL_FUNC_A);
			} else {
				// 切换软件
			}
		}
	}	
}

/**
  * @brief  获取流程参数指针
  * @param  
  * @retval 
  */
RouteInfo_t *Route_GetInfo(void)
{
	return &g_route_Info;
}

/**
  * @brief  获取流程状态
  * @param  
  * @retval 
  */
void Route_SetLogin(void)
{
	g_route_Info.loginStatus = true;
}

void Route_SetUpdate(void)
{
	g_route_Info.updateSoft = true;
}

/**
  * @brief  运行用户登录流程
  * @param  
  * @retval 
  */
static void Route_UserLogin(void)
{
	Merchant_t *merchant = &User_GetInfo()->merc;
	Customer_t *customer = &User_GetInfo()->cust;
	UserPerm_t userperm = USER_PERM_NULL;
	if (g_route_Info.status == ROUTE_MERCHANT_LOGIN) {
		userperm = merchant->perm;
	} else if (g_route_Info.status == ROUTE_CUSTOMER_LOGIN) {
		userperm = customer->perm;
	}
		
	if (g_routeLogin1Time.count == 0) {
		if (g_routeLogin2Time.count == 0) {
			// 检查登录
			if (User_GetInfo()->recv.isReady == true) {
				
				// 本地检索
				if (g_route_Info.status == ROUTE_MERCHANT_LOGIN) {
					if (User_CheckLocal() == true) {
						g_route_Info.loginStatus = true;
					} else {
						// 远程登录
						CMNC_AddCmd(UL_USER_LOGIN);

						g_routeLogin1Time.start = g_routeTick;
						g_routeLogin1Time.count = ROUTE_LOGIN_TIMEOUT;
						Display_SetLoginText(TEXT_MERCHANT_LOGINING);
					}
				} else if (g_route_Info.status == ROUTE_CUSTOMER_LOGIN) {
					// 远程登录
					CMNC_AddCmd(UL_USER_LOGIN);
					g_routeLogin1Time.start = g_routeTick;
					g_routeLogin1Time.count = ROUTE_LOGIN_TIMEOUT;
					Display_SetLoginText(TEXT_CUSTOMER_LOGINING); 
				}
			}
		} else {
			if (g_routeTick - g_routeLogin2Time.start >= g_routeLogin2Time.count) {
				g_routeLogin2Time.start = 0;
				g_routeLogin2Time.count = 0;

				// 复位登录
				if (g_route_Info.status == ROUTE_MERCHANT_LOGIN) {
					Display_SetLoginText(TEXT_LOGIN);
				} else if (g_route_Info.status == ROUTE_CUSTOMER_LOGIN) {
					Display_SetLoginText(TEXT_INPUT_CUSTOMER);
				}
			}
		}
	} else {
		// 店主登录中
		
		if (userperm != USER_PERM_NULL) {
			// 登录成功
			g_routeLogin1Time.start = 0;
			g_routeLogin1Time.count = 0;
			
			// 清空用户缓冲区
			User_ClearRecv();
			if (g_route_Info.status == ROUTE_MERCHANT_LOGIN) {
				if ((merchant->perm == USER_PERM_ADMIN) || (merchant->perm == USER_PERM_MAINTAIN))  {
					g_route_Info.status = ROUTE_ADMIN_MODE;
					Display_SetShow(DISPLAY_SHOW_ADMINPAGE);// 显示管理员界面
					User_SetCustAdmin();
				} else if (merchant->perm == USER_PERM_MERCHANT) {
					g_route_Info.status = ROUTE_CUSTOMER_LOGIN;
					Display_SetLoginText(TEXT_INPUT_CUSTOMER);// 显示客户登录
					User_InitCust();
				}
			} else if (g_route_Info.status == ROUTE_CUSTOMER_LOGIN) {
				g_route_Info.status = ROUTE_PRODUCT_SHOP;
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);
			}
		} else if (g_routeTick - g_routeLogin1Time.start >= g_routeLogin1Time.count) {
			g_routeLogin1Time.start = 0;
			g_routeLogin1Time.count = 0;
			
			// 登录超时失败，清空用户缓冲区并复位登录状态
			g_routeLogin2Time.start = g_routeTick;
			g_routeLogin2Time.count = ROUTE_LOGIN_TIMEOUT;		
			User_ClearRecv();
			Display_SetLoginText(TEXT_LOGIN_FAIL);
		} 
	}
}
