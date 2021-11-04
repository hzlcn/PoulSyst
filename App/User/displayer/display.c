/* Includes ------------------------------------------------------------------*/
#include "lcd.h"
#include "display.h"
#include "drivers.h"		
#include "network.h"        // 显示网络图标
#include "poultry.h"        // 显示禽类名称
#include "param.h"          // 显示错误码
#include "usermsg.h"        // 显示客户名称
#include "store.h"          // 显示机器ID和版本
#include "modbus.h"         // 显示标签数
#include "gps.h"            // 显示GPS卫星信息
#include "communicate.h"    // 发送信息到服务器


/* Private define ------------------------------------------------------------*/

#define DBG_TRACE                                   0

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

/* Private variables ---------------------------------------------------------*/

/**
  * 选择界面的选项参数 ( 页数 和 禽类索引 )
  */
static Display_Opt_t g_optionInfo = { 0, 0 };

/**
  * 登陆界面的显示文本信息
  */
static ShowTextInfo_t g_showText = {
	.type = TEXT_NULL,
	.text = {
		{ 0x2020, 0x2020, 0xCEDE, 0x2020, 0x2020 },		// TEXT_NULL                  “    无    ”
		{ 0xC7EB, 0xC9CC, 0xBBA7, 0xB5C7, 0xC2BC },		// TEXT_LOGIN                 “请商户登录”
		{ 0xC7EB, 0xBCEC, 0xB2E9, 0xCDF8, 0xC2E7 },		// TEXT_CHECK_NETWORK         “请检查网络”
		{ 0xC9E8, 0xB1B8, 0xCEB4, 0xB5C7, 0xC2BD },		// TEXT_DEVICE_UNLOGIN        “设备未登陆”
		{ 0xC7EB, 0xBFAA, 0xC6F4, 0xBBFA, 0xC6F7 },		// TEXT_CHECK_PLC             “请检查机器”
		{ 0xC7EB, 0xD6D8, 0xD0C2, 0xCBA2, 0xBFA8 },		// TEXT_RECARD                “请重新刷卡”
		{ 0x2020, 0xBBF1, 0xC8A1, 0xD6D0, 0x2020 },		// TEXT_GETTING               “  获取中  ”
		{ 0xC1AC, 0xBDD3, 0xCDF8, 0xC2E7, 0xD6D0 },		// TEXT_NULL                  “连接网络中”
		{ 0xC1AC, 0xBDD3, 0xC9E8, 0xB1B8, 0xD6D0 },		// TEXT_NULL                  “连接设备中”
		{ 0xC7EB, 0xCAE4, 0xC8EB, 0xBFCD, 0xBBA7 },		// TEXT_NULL                  “请输入客户”
		{ 0xB6A8, 0xCEBB, 0xCEB4, 0xBBF1, 0xC8A1 },     // TEXT_GPS_DISABLE           “定位未获取”
		{ 0xC9CC, 0xBBA7, 0xCEB4, 0xD7A2, 0xB2E1 },     // TEXT_MERCHANT_UNSIGNIN     “商户未注册”
		{ 0xBFCD, 0xBBA7, 0xCEB4, 0xD7A2, 0xB2E1 },     // TEXT_CUSTOMER_UNSIGNIN     “客户未注册”
		{ 0xC9CC, 0xBBA7, 0xB5C7, 0xC2BC, 0xD6D0 },     // TEXT_MERCHANT_LOGINING     “商户登录中”
		{ 0x20B5, 0xC7C2, 0xBCCA, 0xA7B0, 0xDC20 },     // TEXT_LOGIN_FAIL            “ 登录失败 ”
		{ 0xBFCD, 0xBBA7, 0xB5C7, 0xC2BC, 0xD6D0 },     //TEXT_CUSTOMER_LOGINING      “客户登录中”
	}
};

ErrorText_t g_errorText[10] = {
	{ 0x50, 0x4C, 0x43, 0xCE, 0xDE, 0xCD, 0xA8, 0xD1, 0xB6 },					// PLC无通讯
	{ 0x47, 0x50, 0x53, 0xCE, 0xDE, 0xD0, 0xC5, 0xCF, 0xA2 },					// GPS无信息
	{ 0xCD, 0xE2, 0xC9, 0xA8, 0xC2, 0xEB, 0xC6, 0xF7, 0xB3, 0xF6, 0xB4, 0xED }, // 外扫码器出错
	{ 0xC4, 0xDA, 0xC9, 0xA8, 0xC2, 0xEB, 0xC6, 0xF7, 0xB3, 0xF6, 0xB4, 0xED },	// 内扫码器出错
	{ 0xCE, 0xDE, 0xCA, 0xB3, 0xC6, 0xB7, 0xD0, 0xC5, 0xCF, 0xA2 },				// 无食品信息
	{ 0xCE, 0xDE, 0xCE, 0xC0, 0xD0, 0xC7, 0xD0, 0xC5, 0xCF, 0xA2 },				// 无卫星信息
	{ 0xCE, 0xDE, 0xB6, 0xA8, 0xCE, 0xBB, 0xD0, 0xC5, 0xCF, 0xA2 },				// 无定位信息
	{ 0xCE, 0xB4, 0xB9, 0xD8, 0xCB, 0xF8 },                                     // 未关锁
};

u8 g_testModeText[20] = { 0xB2, 0xE2, 0xCA, 0xD4, 0xC4, 0xA3, 0xCA, 0xBD, 0x00, 0x00, 
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  显示添加/选择/删除界面的按键文本
  */
static void Display_ShowButton(uint16_t addr);

/**
  * @brief  把文本(字节)设置为居中
  */
static void Display_SetByteMid(uint8_t *input, uint8_t *output);

/**
  * @brief  显示初始化
  * @param  
  * @retval 
  */
void Display_Init(void)
{
	LCD_Init();
}

/**
  * @brief  显示数据处理
  * @param  
  * @retval 
  */
void Display_Process(void)
{

	PoulInfo_t *poulInfo = NULL;
	
	LCD_Page_t page = LCD_GetPage();
	uint16_t button = LCD_GetButton();
	
	if (button == 0) {
		return ;
	}
	
	switch (page) {
		case LCD_PAGE_1:
		{
			if (button == LCD_ADDR_B_01_07_CLEARLABEL) {
				// 清除废纸
//			if( PLCService.GetRTUStatus( ) != WRITE_MANUL_CLEAR_LABEL_ACK ) {
//				PLCService.ClearWaste( );				//清除废纸标志位  全局变量
//			}

			} else if (button == LCD_ADDR_B_01_08_CHICK) {
				// 选项：鸡
				LCD_SwitchPage(LCD_PAGE_3);
				Poul_SetList(POUL_LISTGROUPTYPE_CHICK, POUL_LISTTYPE_CUR);
				Display_ShowButton(LCD_ADDR_T_03_02_CHOOSE_11);
			} else if (button == LCD_ADDR_B_01_09_DUCK) {
				// 选项：鸭
				LCD_SwitchPage(LCD_PAGE_3);
				Poul_SetList(POUL_LISTGROUPTYPE_DUCK, POUL_LISTTYPE_CUR);
				Display_ShowButton(LCD_ADDR_T_03_02_CHOOSE_11);

			} else if (button == LCD_ADDR_B_01_10_GOOSE) {
				// 选项：鹅
				LCD_SwitchPage(LCD_PAGE_3);
				Poul_SetList(POUL_LISTGROUPTYPE_GOOSE, POUL_LISTTYPE_CUR);
				Display_ShowButton(LCD_ADDR_T_03_02_CHOOSE_11);

			} else if (button == LCD_ADDR_B_01_11_OTHERS) {
				// 选项：其它
				LCD_SwitchPage(LCD_PAGE_3);
				Poul_SetList(POUL_LISTGROUPTYPE_OTHER, POUL_LISTTYPE_CUR);
				Display_ShowButton(LCD_ADDR_T_03_02_CHOOSE_11);

			} else if (button == LCD_ADDR_B_01_12_EXIT) {
				Merchant_t *merchant = &User_GetInfo()->merc;
				if ((merchant->perm == USER_PERM_ADMIN) || 
					(merchant->perm == USER_PERM_MAINTAIN)) 
				{
					// 退出登录
					LCD_SwitchPage(LCD_PAGE_6);
				} else {
					// 退出登录
					User_InitCust();
					LCD_SwitchPage(LCD_PAGE_0);
				}

			}

			break;
		}
		case LCD_PAGE_2:
		{
			if (button == LCD_ADDR_B_02_10_CHOOSE_11) {
				Poul_AddKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 0]);
			} else if (button == LCD_ADDR_B_02_11_CHOOSE_12) {
				Poul_AddKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 1]);
			} else if (button == LCD_ADDR_B_02_12_CHOOSE_13) {
				Poul_AddKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 2]);
			} else if (button == LCD_ADDR_B_02_13_CHOOSE_14) {
				Poul_AddKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 3]);
			} else if (button == LCD_ADDR_B_02_14_CHOOSE_21) {
				Poul_AddKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 4]);
			} else if (button == LCD_ADDR_B_02_15_CHOOSE_22) {
				Poul_AddKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 5]);
			} else if (button == LCD_ADDR_B_02_16_CHOOSE_23) {
				Poul_AddKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 6]);
			} else if (button == LCD_ADDR_B_02_17_CHOOSE_24) {
				Poul_AddKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 7]);
			} else if (button == LCD_ADDR_B_02_18_DELETE) {
				// 切换到第4页
				LCD_SwitchPage(LCD_PAGE_4);
				
				// 显示禽类库
				Poul_SetList(Poul_GetInfo()->list.group, POUL_LISTTYPE_CUR);
				Display_ShowButton(LCD_ADDR_T_04_02_CHOOSE_11);

			} else if (button == LCD_ADDR_B_02_19_LAST) {
				// 为0时页数固定为0
				if (g_optionInfo.page > 0) {
					g_optionInfo.page--;
					Display_ShowButton(LCD_ADDR_T_03_02_CHOOSE_11);
				}

			} else if (button == LCD_ADDR_B_02_20_NEXT) {
				if (g_optionInfo.page + 1 < g_optionInfo.pageSum) {
					g_optionInfo.page++;
					Display_ShowButton(LCD_ADDR_T_03_02_CHOOSE_11);
				}

			} else if (button == LCD_ADDR_B_02_21_RETURN) {
				LCD_SwitchPage(LCD_PAGE_7);
			}
			break;
		}
		case LCD_PAGE_3:
		{
			if (button == LCD_ADDR_B_03_10_CHOOSE_11) {
				poulInfo = Poul_GetInfo();
				if (poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 0] != NULL) {
					Poul_SetKind(poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 0]);
				}
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);

			} else if (button == LCD_ADDR_B_03_11_CHOOSE_12) {
				poulInfo = Poul_GetInfo();
				if (poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 1] != NULL) {
					Poul_SetKind(poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 1]);
				}
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);

			} else if (button == LCD_ADDR_B_03_12_CHOOSE_13) {
				poulInfo = Poul_GetInfo();
				if (poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 2] != NULL) {
					Poul_SetKind(poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 2]);
				}
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);

			} else if (button == LCD_ADDR_B_03_13_CHOOSE_14) {
				poulInfo = Poul_GetInfo();
				if (poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 3] != NULL) {
					Poul_SetKind(poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 3]);
				}
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);

			} else if (button == LCD_ADDR_B_03_14_CHOOSE_21) {
				poulInfo = Poul_GetInfo();
				if (poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 4] != NULL) {
					Poul_SetKind(poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 4]);
				}
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);

			} else if (button == LCD_ADDR_B_03_15_CHOOSE_22) {
				poulInfo = Poul_GetInfo();
				if (poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 5] != NULL) {
					Poul_SetKind(poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 5]);
				}
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);

			} else if (button == LCD_ADDR_B_03_16_CHOOSE_23) {
				poulInfo = Poul_GetInfo();
				if (poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 6] != NULL) {
					Poul_SetKind(poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 6]);
				}
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);

			} else if (button == LCD_ADDR_B_03_17_CHOOSE_24) {
				poulInfo = Poul_GetInfo();
				if (poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 7] != NULL) {
					Poul_SetKind(poulInfo->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 7]);
				}
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);

			} else if (button == LCD_ADDR_B_03_18_HOME) {
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);

			} else if (button == LCD_ADDR_B_03_19_LAST) {
				/* 上一页 */
				if (g_optionInfo.page > 0) {
					g_optionInfo.page--;
					Display_ShowButton(LCD_ADDR_T_03_02_CHOOSE_11);
				} else {
					LCD_SwitchPage(LCD_PAGE_5);
				}

			} else if (button == LCD_ADDR_B_03_20_NEXT) {
				if (g_optionInfo.page + 1 < g_optionInfo.pageSum) {
					g_optionInfo.page++;
					Display_ShowButton(LCD_ADDR_T_03_02_CHOOSE_11);
				} else {
					LCD_SwitchPage(LCD_PAGE_5);
				}

			}

			break;
		}
		case LCD_PAGE_4:
		{
			if (button == LCD_ADDR_B_04_10_CHOOSE_11) {
				// 删除食品信息
				Poul_DelKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 0]);
				
				// 重新显示删除后的食品列表按键
				Display_ShowButton(LCD_ADDR_T_04_02_CHOOSE_11);

			} else if (button == LCD_ADDR_B_04_10_CHOOSE_12) {
				Poul_DelKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 1]);
				Display_ShowButton(LCD_ADDR_T_04_02_CHOOSE_11);

			} else if (button == LCD_ADDR_B_04_10_CHOOSE_13) {
				Poul_DelKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 2]);
				Display_ShowButton(LCD_ADDR_T_04_02_CHOOSE_11);

			} else if (button == LCD_ADDR_B_04_10_CHOOSE_14) {
				Poul_DelKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 3]);
				Display_ShowButton(LCD_ADDR_T_04_02_CHOOSE_11);

			} else if (button == LCD_ADDR_B_04_10_CHOOSE_21) {
				Poul_DelKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 4]);
				Display_ShowButton(LCD_ADDR_T_04_02_CHOOSE_11);

			} else if (button == LCD_ADDR_B_04_10_CHOOSE_22) {
				Poul_DelKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 5]);
				Display_ShowButton(LCD_ADDR_T_04_02_CHOOSE_11);

			} else if (button == LCD_ADDR_B_04_10_CHOOSE_23) {
				Poul_DelKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 6]);
				Display_ShowButton(LCD_ADDR_T_04_02_CHOOSE_11);

			} else if (button == LCD_ADDR_B_04_10_CHOOSE_24) {
				Poul_DelKind(Poul_GetInfo()->list.pKinds[g_optionInfo.page * OPTION_NUMBER + 7]);
				Display_ShowButton(LCD_ADDR_T_04_02_CHOOSE_11);

			} else if (button == LCD_ADDR_B_04_10_LAST) {
				if (g_optionInfo.page > 0) {
					g_optionInfo.page--;
					Display_ShowButton(LCD_ADDR_T_04_02_CHOOSE_11);
				}

			} else if (button == LCD_ADDR_B_04_10_NEXT) {
				if (g_optionInfo.page + 1 < g_optionInfo.pageSum) {
					g_optionInfo.page++;
					Display_ShowButton(LCD_ADDR_T_04_02_CHOOSE_11);
				}

			} else if (button == LCD_ADDR_B_04_10_RETURN) {
				LCD_SwitchPage(LCD_PAGE_2);
				
				// 选项页码复位
				g_optionInfo.page = 0;
				Poul_SetList(Poul_GetInfo()->list.group, POUL_LISTTYPE_ADD);
				Display_ShowButton(LCD_ADDR_T_02_02_CHOOSE_11);

			}
			break;
		}
		case LCD_PAGE_5:
		{
			if (button == LCD_ADDR_B_05_02_HOME) {
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);

			} else if (button == LCD_ADDR_B_05_03_RETURN) {
				LCD_SwitchPage(LCD_PAGE_3);

			}

			break;
		}
		case LCD_PAGE_6:
		{
			if (button == LCD_ADDR_B_06_02_FORCERUN) {
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);

			} else if (button == LCD_ADDR_B_06_03_UNLOCK) {
				Modbus_AddCmd(MODBUS_CMD_UNLOCK_DEVICE);
				CMNC_AddCmd(UL_FUNC_9);
				
			} else if (button == LCD_ADDR_B_06_04_SPECIES) {
				LCD_SwitchPage(LCD_PAGE_7);

			} else if (button == LCD_ADDR_B_06_05_MAINTAIN) {
//				LCD_SwitchPage(LCD_PAGE_8);
			}

			break;
		}
		case LCD_PAGE_7:
		{
			if (button == LCD_ADDR_B_07_02_CHICK) {
				Poul_SetList(POUL_LISTGROUPTYPE_CHICK, POUL_LISTTYPE_ADD);
				
				LCD_SwitchPage(LCD_PAGE_2);
				
				/* 查询服务器的新增禽类信息 */
				CMNC_AddCmd(UL_FUNC_8);

			} else if (button == LCD_ADDR_B_07_03_DUCK) {
				Poul_SetList(POUL_LISTGROUPTYPE_DUCK, POUL_LISTTYPE_ADD);

				LCD_SwitchPage(LCD_PAGE_2);
				CMNC_AddCmd(UL_FUNC_8);

			} else if (button == LCD_ADDR_B_07_04_GOOSE) {
				Poul_SetList(POUL_LISTGROUPTYPE_GOOSE, POUL_LISTTYPE_ADD);

				LCD_SwitchPage(LCD_PAGE_2);
				CMNC_AddCmd(UL_FUNC_8);

			} else if (button == LCD_ADDR_B_07_05_OTHERS) {
				Poul_SetList(POUL_LISTGROUPTYPE_OTHER, POUL_LISTTYPE_ADD);

				LCD_SwitchPage(LCD_PAGE_2);
				CMNC_AddCmd(UL_FUNC_8);

			} else if (button == LCD_ADDR_B_07_06_RETURN) {
				LCD_SwitchPage(LCD_PAGE_6);
				
			}

			break;
		}
		default:
			break;
	}
}

/**
  * @brief  设置显示
  * @param  
  * @retval 
  */
void Display_SetShow(Display_ShowType_t showType)
{
	// 获取参数
	LCD_Page_t page = LCD_GetPage();
	NwkStatus_t nwkStatus = Network_GetStatus();
	RTCTime_t *rtcTime = RTCTime_GetTime();
	PoulInfo_t *poulInfo = Poul_GetInfo();
	Merchant_t *merchant = &User_GetInfo()->merc;
	Customer_t *customer = &User_GetInfo()->cust;
	GPS_SVParam_t *svInfo = GPS_GetSVDate();
	ModbusInfo_t *modbusInfo = Modbus_GetParam();
	
	

	// LCD地址缓冲区
	u16 wrAddr = 0;
	
	// 网络图标缓冲区
	u16 netIcon = DISPLAY_ICON_NWK_NONE;
	
	// 错误码缓冲区
	ErrStatus_t errCode = ERROR_NONE;
	
	// 时间缓冲区
	char curDate[20] = { 0 };
	
	// 卫星信息缓冲区
	char gpsSvStr[12] = { 0 };
	
	// 标签数缓冲区
	char number[4] = { 0 };
	
	// 剩余标签数缓冲区
	u16 valibNum = modbusInfo->regList[MODBUS_REG_VALIB_LABEL].data;
	u16 remainnumb = LABEL_TOTAL_NUMBER - valibNum - modbusInfo->regList[MODBUS_REG_INVAL_LABEL].data;
	
	// 机器ID缓冲区
	uint8_t devAddr[6] = { 0 };
	
	// 其他数据缓冲区
	uint8_t tempor1[10] = { 0 };
	char tempor2[20] = { 0 };

	switch (showType) {
		case DISPLAY_SHOW_RTCTIME:
		{   // 当前时间
			wrAddr = LCD_BASE_ADDR + LCD_GetPage() * LCD_PAGE_STEP + 0x0006;
			
			sprintf(curDate, "20%02d-%02d-%02d %02d:%02d", 
				rtcTime->rtc.year, rtcTime->rtc.month, rtcTime->rtc.day, 
				rtcTime->rtc.hour, rtcTime->rtc.minute);
			LCD_WriteByte(wrAddr, curDate, 16);
			break;
		}
		case DISPLAY_SHOW_LABELNUM:
		{	// 标签数
			if (page == LCD_PAGE_1) {
				// 有效标签
				sprintf(number, "%04d", valibNum);
				LCD_WriteByte(LCD_ADDR_T_01_02_LABELNUM, number, 4);

				// 剩余标签
				memset(number, 0, 4);
				sprintf(number, "%04d", remainnumb);
				LCD_WriteByte(LCD_ADDR_T_01_03_REMAINNUM, number, 4);
			}
			break;
		}
		case DISPLAY_SHOW_GPSSNR:
		{   // GPS信号强度
			if (page == LCD_PAGE_0) {
				if (GPS_SVIsReady() == true) {
					sprintf(gpsSvStr, "GPS %c%c,%c%cdB ", svInfo->svNum[0], svInfo->svNum[1], 
						svInfo->svSnr[0], svInfo->svSnr[1]);
					LCD_WriteByte(LCD_ADDR_T_00_03_GPSRSSI, gpsSvStr, 12);
				}
			}
			break;
		}
		case DISPLAY_SHOW_NETICON:
		{   // 网络信号图标
			if (page == LCD_PAGE_0) {		// 登录页面
				wrAddr = LCD_ADDR_T_00_06_NETWORKICON;
			} else if (page == LCD_PAGE_1) {// 工作页面
				wrAddr = LCD_ADDR_T_01_06_NETWORKICON;
			}
			
			if (nwkStatus == NETWORK_STATUS_NET) {
				netIcon = DISPLAY_ICON_NWK_NET;
				LCD_WriteWord(wrAddr, &netIcon, 1);
			} else if (nwkStatus == NETWORK_STATUS_SIM) {
				netIcon = DISPLAY_ICON_NWK_SIM;
				LCD_WriteWord(wrAddr, &netIcon, 1);
			} else {
				LCD_WriteWord(wrAddr, &netIcon, 1);
			}
			break;
		}
		case DISPLAY_SHOW_ERRCODE:
		{
			errCode = Error_GetError();
			wrAddr = LCD_GetPage() * LCD_PAGE_STEP + LCD_BASE_ADDR;

			if (errCode != ERROR_NONE) {
				LCD_WriteByte(wrAddr, (char *)g_errorText[errCode], 12);
			} else {
				LCD_WriteByte(wrAddr, (char *)tempor2, 12);
			}
			break;
		}
		case DISPLAY_SHOW_WORKPAGE:
		{   // 显示工作页面
			LCD_SwitchPage(LCD_PAGE_1);
			
			// 设置网络图标
			if (nwkStatus == NETWORK_STATUS_NET) {
				netIcon = DISPLAY_ICON_NWK_NET;
				LCD_WriteWord(LCD_ADDR_T_01_06_NETWORKICON, &netIcon, 1);
			} else if (nwkStatus == NETWORK_STATUS_SIM) {
				netIcon = DISPLAY_ICON_NWK_SIM;
				LCD_WriteWord(LCD_ADDR_T_01_06_NETWORKICON, &netIcon, 1);
			} else {
				LCD_WriteWord(LCD_ADDR_T_01_06_NETWORKICON, &netIcon, 1);
			}
			
			// 显示当前禽类名称
			Display_SetByteMid(poulInfo->pCurKind->name, tempor1);
			LCD_WriteByte(LCD_ADDR_T_01_05_SPECIES, (char *)tempor1, 10);

			// 显示客户
			if ((merchant->perm == USER_PERM_MAINTAIN) || (merchant->perm == USER_PERM_ADMIN)) {
				LCD_WriteByte(LCD_ADDR_T_01_04_CUSTOMER, (char *)g_testModeText, 20);
			} else {
				if (customer->perm == USER_PERM_CUSTOMER) {
					LCD_WriteByte(LCD_ADDR_T_01_04_CUSTOMER, (char *)customer->name, 20);
				}
			}

			// 有效标签
			sprintf(number, "%04d", valibNum);
			LCD_WriteByte(LCD_ADDR_T_01_02_LABELNUM, number, 4);

			// 剩余标签
			memset(number, 0, 4);
			sprintf(number, "%04d", remainnumb);
			LCD_WriteByte(LCD_ADDR_T_01_03_REMAINNUM, number, 4);

			break;
		}
		case DISPLAY_SHOW_LOGINPAGE:
		{
			// 显示登录页面
			LCD_SwitchPage(LCD_PAGE_0);
			
			// 设置网络标志
			if (nwkStatus == NETWORK_STATUS_NET) {
				netIcon = DISPLAY_ICON_NWK_NET;
				LCD_WriteWord(LCD_ADDR_T_00_06_NETWORKICON, &netIcon, 1);
			} else if (nwkStatus == NETWORK_STATUS_SIM) {
				netIcon = DISPLAY_ICON_NWK_SIM;
				LCD_WriteWord(LCD_ADDR_T_00_06_NETWORKICON, &netIcon, 1);
			} else {
				LCD_WriteWord(LCD_ADDR_T_00_06_NETWORKICON, &netIcon, 1);
			}
			
			// 设置登录页面文本显示
			ShowText_t textBuf = { 0 };			
			memcpy(textBuf, g_showText.text[g_showText.type], sizeof(ShowText_t));
			LCD_WriteWord(LCD_ADDR_T_00_02_SHOWTEXT, textBuf, 5);

			// 设置机器ID
			memcpy(devAddr, Store_GetConfig()->devAddr, sizeof(DevAddr_t));
			sprintf(tempor2, "%02x-%02x-%02x-%02x-%02x-%02x   ", devAddr[0], devAddr[1], devAddr[2], devAddr[3], devAddr[4], devAddr[5]);
			LCD_WriteByte(LCD_ADDR_T_00_04_MACHINEID, tempor2, 20);

			// 设置版本号
			memset(tempor2, 0, 20);
			Version_t *version = &Store_GetConfig()->version;
			sprintf(tempor2, "soft - %02x, hard - %02x", version->soft, version->hard);
			LCD_WriteByte(LCD_ADDR_T_00_05_FIRMWARE, tempor2, 20);
			break;
		}
		case DISPLAY_SHOW_ADMINPAGE:
		{
			LCD_SwitchPage(LCD_PAGE_6);
			break;
		}
		case DISPLAY_SHOW_ADDPAGE:
		{
			// 重置食品列表，显示列表按键
			if (LCD_GetPage() == LCD_PAGE_2) {
				Poul_SetList(poulInfo->list.group, POUL_LISTTYPE_ADD);
				Display_ShowButton(LCD_ADDR_T_02_02_CHOOSE_11);
			}
			break;
		}
		default:
			break;
	}
}

/**
  * @brief  设置显示登录界面文本
  * @param  
  * @retval 
  */
void Display_SetLoginText(ShowTextType_t textType)
{   // 显示文本
	g_showText.type = textType;
	LCD_WriteWord(LCD_ADDR_T_00_02_SHOWTEXT, g_showText.text[textType], 5);
}

/**
  * @brief  读取显示参数
  * @param  
  * @retval 
  */
Display_PageType_t Display_GetPage(void)
{
	LCD_Page_t page = LCD_GetPage();
	if (page == LCD_PAGE_0) {
		return DISPLAY_PAGE_LOGIN;
	} else if (page == LCD_PAGE_1) {
		return DISPLAY_PAGE_WORK;
	} else if (page == LCD_PAGE_2) {
		return DISPLAY_PAGE_ADD;
	} else if (page == LCD_PAGE_3) {
		return DISPLAY_PAGE_SELECT;
	} else if (page == LCD_PAGE_4) {
		return DISPLAY_PAGE_DELETE;
	} else if (page == LCD_PAGE_6) {
		return DISPLAY_PAGE_ADMIN;
	} else {
		return DISPLAY_PAGE_ERROR;
	}
}	

/**
  * @brief  显示食品信息库的按键
  * @param  
  * @retval 
  */
static void Display_ShowButton(uint16_t addr)
{
	uint8_t i;
	uint8_t transBuf[10] = { 0 };
	uint8_t gbk_NULL[10] = { 0x20, 0x20, 0x20, 0x20, 0xCE, 0xDE, 0x20, 0x20, 0x20, 0x20 };
	PoulList_t *poulList = &Poul_GetInfo()->list;
	
	// 根据信息列表从信息库中提取信息，并把信息显示到LCD的8个按键
	for (i = 0; i < 8; i++) {
		if (((g_optionInfo.page * 8) + i) < poulList->sum) {
			// 显示禽类名称
			Display_SetByteMid(poulList->pKinds[g_optionInfo.page * OPTION_NUMBER + i]->name, transBuf);
			LCD_WriteByte(addr, (char *)transBuf, 10);
		} else {
			// 显示“无”
			Display_SetByteMid(gbk_NULL, transBuf);
			LCD_WriteByte(addr, (char *)transBuf, 10);
		}
		addr += 5;
	}
}

/**
  * @brief  把文本(字节)设置为居中
  * @param  
  * @retval 
  */
static void Display_SetByteMid(uint8_t *input, uint8_t *output)
{
	uint8_t i, j;
	uint8_t wordlen = 0;
		
	memset(output, ' ', 10);

	/* 设置文字居中 */
	while ((input[wordlen] != 0xFF) && (wordlen < 10)) {
		wordlen++;
	}
	for (i = 0, j = 0; i < 10; i++) {
		if ((i >= (10 - wordlen) / 2) && (i < (10 + wordlen) / 2)) {
			output[i] = input[j++];
		}
	}
}
