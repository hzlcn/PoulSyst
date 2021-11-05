#include "route.h"
#include "param.h"
#include "apps.h"
#include "display.h"
#include "usermsg.h"
#include "poultry.h"
#include "drivers.h"
#include "sim7600.h"
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

static RoutePara_t *g_pRoute = &g_allData.temp.route;

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
static void Route_UserLogin(RouteStatus_t status);

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
	RouteStatus_t routeStatus = ROUTE_INIT_MODE;
	bool routeSellTrig = false;
	LCD_Page_t page = g_tmpPara->display.page;
	uint16_t button = LCD_GetButton();
	DispOpt_t *pDispOpt = &g_tmpPara->display.opt;

	g_routeTick = HAL_GetTick();

	switch (routeStatus) {
		case ROUTE_INIT_MODE:
			// 初始化
			routeStatus = ROUTE_DEVICE_LINK;
			Display_SetLoginText(TEXT_DEVICE_UNLOGIN);
			break;
		
		case ROUTE_DEVICE_LINK:
			if (modbusInfo->devJoin == true) {
				if (modbusInfo->regList[7].data == 0x0000) {
					routeStatus = ROUTE_MERCHANT_LOGIN;
					Display_SetLoginText(TEXT_LOGIN);   // 显示"请商户登录"
					User_InitMerc();                    // 初始化店主
				}
			}
			break;
		
		case ROUTE_MERCHANT_LOGIN:
			Route_UserLogin(routeStatus);
			if (g_pRoute->loginMerc == LOGIN_MERCHANT_SUCCESS) {
				// 登录成功
				User_ClearRecv();	// 清空用户缓冲区
				
				if ((merchant->perm == USER_PERM_ADMIN) || (merchant->perm == USER_PERM_MAINTAIN))  {
					// 进入管理员界面
					routeStatus = ROUTE_ADMIN_MODE;
					Display_SetShow(DISPLAY_SHOW_ADMINPAGE);	
					User_SetCustAdmin();
				} else if (merchant->perm == USER_PERM_MERCHANT) {
					// 进入客户登录
					routeStatus = ROUTE_CUSTOMER_LOGIN;
					Display_SetLoginText(TEXT_INPUT_CUSTOMER);	
				}
				
				g_pRoute->loginMerc = LOGIN_MERCHANT_READY;
			}
			break;
			
		case ROUTE_ADMIN_MODE:
			if (g_tmpPara->display.page == LCD_PAGE_6) {
				if (button == LCD_ADDR_B_06_02_FORCERUN) {
					Display_SetShow(DISPLAY_SHOW_WORKPAGE);

				} else if (button == LCD_ADDR_B_06_03_UNLOCK) {
					Modbus_AddCmd(MODBUS_CMD_UNLOCK_DEVICE);
					CMNC_AddCmd(UL_FUNC_9);
					
				} else if (button == LCD_ADDR_B_06_04_SPECIES) {
					LCD_SwitchPage(LCD_PAGE_7);

				} else if (button == LCD_ADDR_B_06_05_MAINTAIN) {
			//		LCD_SwitchPage(LCD_PAGE_8);
				}
			}
			if (g_tmpPara->display.page == LCD_PAGE_1) {
				// 进入售卖页面
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);				
				routeStatus = ROUTE_PRODUCT_SHOP;
			}
			break;
		

		case ROUTE_CUSTOMER_LOGIN:
			Route_UserLogin(routeStatus);
			if (g_pRoute->loginMerc == LOGIN_MERCHANT_SUCCESS) {
				// 登录成功，进入售卖页面
				User_ClearRecv();	// 清空用户缓冲区
				routeStatus = ROUTE_PRODUCT_SHOP;
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);
				g_pRoute->loginMerc = LOGIN_MERCHANT_READY;
			}
			break;
		
		case ROUTE_PRODUCT_SHOP:
			if (g_tmpPara->display.page == LCD_PAGE_1) {
				Merchant_t *merchant = &User_GetInfo()->merc;
				PoulListGroup_t group = POUL_LISTGROUPTYPE_NONE;
				switch (button) {
					case LCD_ADDR_B_01_07_CLEARLABEL:
						// 清除废纸
			//			if( PLCService.GetRTUStatus( ) != WRITE_MANUL_CLEAR_LABEL_ACK ) {
			//				PLCService.ClearWaste( );				//清除废纸标志位  全局变量
			//			}
						break;
					
					case LCD_ADDR_B_01_08_CHICK:	// 选项：鸡
					case LCD_ADDR_B_01_09_DUCK:		// 选项：鸭		
					case LCD_ADDR_B_01_10_GOOSE:	// 选项：鹅
					case LCD_ADDR_B_01_11_OTHERS:	// 选项：其它
						group = button - LCD_ADDR_B_01_08_CHICK;
						Disp_SetList(group, g_tmpPara->poul.base[0]);	// 设置选项列表
						LCD_SwitchPage(LCD_PAGE_3);							// 跳转选择页面
						Display_ShowOptions(LCD_ADDR_T_03_02_CHOOSE_11);	// 设置选项内容
						break;
					
					case LCD_ADDR_B_01_12_EXIT:
						// 退出登录
						if ((merchant->perm == USER_PERM_ADMIN) || 
							(merchant->perm == USER_PERM_MAINTAIN)) 
						{
							LCD_SwitchPage(LCD_PAGE_6);
						} else {
							LCD_SwitchPage(LCD_PAGE_0);
						}
						break;
						
					default:
						break;
				}

			} else if (g_tmpPara->display.page == LCD_PAGE_3) {
				PoulKind_t *poulKind = NULL;
				switch (button) {
					case LCD_ADDR_B_03_10_CHOOSE_11:
					case LCD_ADDR_B_03_11_CHOOSE_12:
					case LCD_ADDR_B_03_12_CHOOSE_13:
					case LCD_ADDR_B_03_13_CHOOSE_14:
					case LCD_ADDR_B_03_14_CHOOSE_21:
					case LCD_ADDR_B_03_15_CHOOSE_22:
					case LCD_ADDR_B_03_16_CHOOSE_23:
					case LCD_ADDR_B_03_17_CHOOSE_24:
						// 读取该项禽类信息
						poulKind = pDispOpt->pKinds[pDispOpt->page * OPTION_NUMBER + button - LCD_ADDR_B_03_10_CHOOSE_11];
						if (poulKind == NULL) {
							break;
						}
						
						// 设置该项禽类为选中禽类
						Poul_SetKind(poulKind);
						
						// 返回售卖页面
						Display_SetShow(DISPLAY_SHOW_WORKPAGE);
						break;

					case LCD_ADDR_B_03_18_HOME:
						// 返回售卖页面
						Display_SetShow(DISPLAY_SHOW_WORKPAGE);
						break;
					
					case LCD_ADDR_B_03_19_LAST:
						// 上一页
						if (pDispOpt->page > 0) {
							pDispOpt->page--;
							Display_ShowOptions(LCD_ADDR_T_03_02_CHOOSE_11);
			//			} else {
			//				LCD_SwitchPage(LCD_PAGE_5);
						}
						break;
						
					case LCD_ADDR_B_03_20_NEXT:
						// 下一页
						if (pDispOpt->page + 1 < pDispOpt->pageSum) {
							pDispOpt->page++;
							Display_ShowOptions(LCD_ADDR_T_03_02_CHOOSE_11);
			//			} else {
			//				LCD_SwitchPage(LCD_PAGE_5);
						}
						break;
						
					default:
						break;
				}

			}
			
			// 检查贴标触发
			if (READ_SP0 == GPIO_PIN_RESET) {
				if (routeSellTrig == true) {
					routeSellTrig = false;

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
					routeSellTrig = true;
				}
			}

			// 检查客户是否退出
			if (g_tmpPara->display.page == LCD_PAGE_0) {
				Display_SetLoginText(TEXT_LOGIN);

				g_routeTrigTimeout.start = 0;
				g_routeTrigTimeout.count = 0;

				routeStatus = ROUTE_CUSTOMER_LOGIN;
				
			} else if (g_tmpPara->display.page == LCD_PAGE_6) {
				routeStatus = ROUTE_ADMIN_MODE;
			}

			// 检查定时计数
			if ((g_routeTrigTimeout.count > 0) && (g_routeTick - g_routeTrigTimeout.start >= g_routeTrigTimeout.count)) {
				g_routeTrigTimeout.start = 0;
				g_routeTrigTimeout.count = 0;
			}
			break;
		
		
		default:
			break;
	}

	// 1秒处理
	if (g_routeTick - g_routeWhile1sTime.start >= g_routeWhile1sTime.count){
		g_routeWhile1sTime.start = g_routeTick;

		// 显示错误码
		Disp_SetErrTip();

		// 显示有效标签数
		Disp_SetLabel();

		// 处理网络图标
		Disp_SetNetIcon();
	}
	
	// 5s处理
	if (g_routeTick - g_routeWhile5sTime.start >= g_routeWhile5sTime.count){
		g_routeWhile5sTime.start = g_routeTick;
		
		// 显示GPS信息
		Disp_SetGPSSNR();
		
		// 显示时间
		Disp_SetTime();
				
		// 软件更新
		if (g_pRoute->updateSoft == true) {
			if (Upg_GetInfo()->firmIndex < Upg_GetInfo()->firmSize) {
				CMNC_AddCmd(UL_FUNC_A);
			} else {
				// 切换软件
			}
		}
	}	
}

/**
  * @brief  运行用户登录流程
  * @param  
  * @retval 
  */
static void Route_UserLogin(RouteStatus_t status)
{
	Merchant_t *merchant = &User_GetInfo()->merc;
	Customer_t *customer = &User_GetInfo()->cust;
	UserPerm_t userperm = USER_PERM_NULL;
	ShowTextType_t loginText = TEXT_NULL;
	ShowTextType_t initText = TEXT_NULL;
	
	if (status == ROUTE_MERCHANT_LOGIN) {
		userperm = merchant->perm;
		loginText = TEXT_MERCHANT_LOGINING;
		initText = TEXT_LOGIN;
	} else if (status == ROUTE_CUSTOMER_LOGIN) {
		userperm = customer->perm;
		loginText = TEXT_CUSTOMER_LOGINING;
		initText = TEXT_INPUT_CUSTOMER;
	}
	
	if (g_pRoute->loginMerc == LOGIN_MERCHANT_READY) {
		// 检查缓冲区
		if (g_tmpPara->user.cust.perm != USER_PERM_NULL) {
			memset(&g_tmpPara->user.cust, 0, sizeof(Customer_t));
		}
		
		// 检查登录
		if (g_tmpPara->user.recv.isReady == false) {
			return;
		}
		
		// 检索本地
		if (status == ROUTE_MERCHANT_LOGIN) {
			if (User_CheckLocal() == true) {
				g_pRoute->loginMerc = LOGIN_MERCHANT_SUCCESS;
				return;			
			}
		}
		
		// 远程登录
		CMNC_AddCmd(UL_USER_LOGIN);
		g_routeLogin1Time.start = g_routeTick;
		g_routeLogin1Time.count = ROUTE_LOGIN_TIMEOUT;
		Display_SetLoginText(loginText);
		g_pRoute->loginMerc = LOGIN_MERCHANT_RUNNING;
		
	} else if (g_pRoute->loginMerc == LOGIN_MERCHANT_RUNNING) {
		// 远程登录中
		if (userperm != USER_PERM_NULL) {
			// 登录成功
			g_routeLogin1Time.start = 0;
			g_routeLogin1Time.count = 0;
			
			g_pRoute->loginMerc = LOGIN_MERCHANT_SUCCESS;
			
		} else if (g_routeTick - g_routeLogin1Time.start >= g_routeLogin1Time.count) {
			// 登录失败
			g_routeLogin1Time.start = 0;
			g_routeLogin1Time.count = 0;
			
			g_pRoute->loginMerc = LOGIN_MERCHANT_FAILED;
		} 
	} else if (g_pRoute->loginMerc == LOGIN_MERCHANT_FAILED) {
		// 登录失败
		if (g_routeLogin2Time.count == 0) {
			g_routeLogin2Time.start = g_routeTick;
			g_routeLogin2Time.count = ROUTE_LOGIN_TIMEOUT;		
			User_ClearRecv();
			Display_SetLoginText(TEXT_LOGIN_FAIL);
		}
		
		if (g_routeTick - g_routeLogin2Time.start >= g_routeLogin2Time.count) {
			g_routeLogin2Time.start = 0;
			g_routeLogin2Time.count = 0;

			// 复位登录
			Display_SetLoginText(initText);
			g_pRoute->loginMerc = LOGIN_MERCHANT_READY;
		}

	}
}
