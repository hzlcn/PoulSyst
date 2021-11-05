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


///* Private define ------------------------------------------------------------*/

//#define DBG_TRACE                                   0

//#if DBG_TRACE == 1
//    #include <stdio.h>
//    /*!
//     * Works in the same way as the printf function does.
//     */
//    #define DBG( ... )                               \
//        do                                           \
//        {                                            \
//            printf( __VA_ARGS__ );                   \
//        }while( 0 )
//#else
//    #define DBG( fmt, ... )
//#endif

/* Private macro -------------------------------------------------------------*/

/**
  * 显示网络图标类型
  */
#define DISPLAY_ICON_NWK_NONE            0x0000
#define DISPLAY_ICON_NWK_NET             0x0001
#define DISPLAY_ICON_NWK_SIM             0x0002

/* Private variables ---------------------------------------------------------*/

/**
  * 选择界面的选项参数 ( 页数 和 禽类索引 )
  */
static DispOpt_t *g_pOpt = &g_allData.temp.display.opt;

static Display_t *g_pDisp = &g_allData.temp.display;

/**
  * 登陆界面的显示文本信息
  */
static ShowTextInfo_t g_showText = {
	.type = TEXT_NULL,
	.text = g_allData.temp.display.wTxt,
};

ErrorText_t *g_errorText = g_allData.temp.display.err;

u8 g_testModeText[20] = { 
	0xB2, 0xE2, 0xCA, 0xD4, 0xC4, 0xA3, 0xCA, 0xBD, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};
/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  把文本(字节)设置为居中
  */
static void Display_SetByteMid(uint8_t *input, uint8_t *output);

/**
  * @brief  各个页面的按键处理
  */
static void Page_1_Pro(u16 button);	// 售卖页面
//static void Page_2_Pro(u16 button);	// 添加二页面
static void Page_3_Pro(u16 button);	// 选择页面
//static void Page_4_Pro(u16 button);	// 删除页面
//static void Page_5_Pro(u16 button);	// 超出页面
static void Page_6_Pro(u16 button);	// 管理员页面
//static void Page_7_Pro(u16 button);	// 添加一页面

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
//	LCD_Page_t page = LCD_GetPage();
//	uint16_t button = LCD_GetButton();
//	
//	if (button == 0) {
//		return ;
//	}
//	
//	switch (page) {
//		case LCD_PAGE_1:
//			Page_1_Pro(button);
//			break;
//		
////		case LCD_PAGE_2:
////			Page_2_Pro(button);
////			break;
//		
//		case LCD_PAGE_3:
//			Page_3_Pro(button);
//			break;
//		
////		case LCD_PAGE_4:
////			Page_4_Pro(button);
////			break;
//		
////		case LCD_PAGE_5:
////			Page_5_Pro(button);
////			break;
//		
//		case LCD_PAGE_6:
//			Page_6_Pro(button);
//			break;
//		
////		case LCD_PAGE_7:
////			Page_7_Pro(button);
////			break;
//		
//		default:
//			break;
//	}
}

/**
  * @brief  设置显示时间
  * @param  
  * @retval 
  */
void Disp_SetTime(void)
{
	// LCD地址缓冲区
	u16 wrAddr = 0;
	// 时间缓冲区
	char curDate[20] = { 0 };
	RTCTime_t *rtcTime = RTCTime_GetTime();
	
	// 当前时间
	wrAddr = LCD_BASE_ADDR + LCD_GetPage() * LCD_PAGE_STEP + 0x0006;
	
	sprintf(curDate, "20%02d-%02d-%02d %02d:%02d", 
		rtcTime->rtc.year, rtcTime->rtc.month, rtcTime->rtc.day, 
		rtcTime->rtc.hour, rtcTime->rtc.minute);
	LCD_WriteByte(wrAddr, curDate, 16);
}

/**
  * @brief  设置显示标签数
  * @param  
  * @retval 
  */
void Disp_SetLabel(void)
{
	LCD_Page_t page = LCD_GetPage();
	// 标签数缓冲区
	char number[4] = { 0 };
	// 剩余标签数缓冲区
	ModbusInfo_t *modbusInfo = Modbus_GetParam();
	u16 valibNum = modbusInfo->regList[MODBUS_REG_VALIB_LABEL].data;
	u16 remainnumb = LABEL_TOTAL_NUMBER - valibNum - modbusInfo->regList[MODBUS_REG_INVAL_LABEL].data;

	if (page == LCD_PAGE_1) {
		// 有效标签
		sprintf(number, "%04d", valibNum);
		LCD_WriteByte(LCD_ADDR_T_01_02_LABELNUM, number, 4);

		// 剩余标签
		memset(number, 0, 4);
		sprintf(number, "%04d", remainnumb);
		LCD_WriteByte(LCD_ADDR_T_01_03_REMAINNUM, number, 4);
	}
}

/**
  * @brief  设置显示GPS信号
  * @param  
  * @retval 
  */
void Disp_SetGPSSNR(void)
{
	LCD_Page_t page = LCD_GetPage();
	// 卫星信息缓冲区
	char gpsSvStr[12] = { 0 };
	GPS_SVParam_t *svInfo = GPS_GetSVDate();
	
	if (page == LCD_PAGE_0) {
		if (GPS_SVIsReady() == true) {
			sprintf(gpsSvStr, "GPS %c%c,%c%cdB ", svInfo->svNum[0], svInfo->svNum[1], 
				svInfo->svSnr[0], svInfo->svSnr[1]);
			LCD_WriteByte(LCD_ADDR_T_00_03_GPSRSSI, gpsSvStr, 12);
		}
	}
}

/**
  * @brief  设置显示网络信号
  * @param  
  * @retval 
  */
void Disp_SetNetIcon(void)
{
	LCD_Page_t page = LCD_GetPage();
	// LCD地址缓冲区
	u16 wrAddr = 0;
	NwkStatus_t nwkStatus = Network_GetStatus();
	// 网络图标缓冲区
	u16 netIcon = DISPLAY_ICON_NWK_NONE;
	
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
}

/**
  * @brief  设置显示错误提示
  * @param  
  * @retval 
  */
void Disp_SetErrTip(void)
{
	// 错误码缓冲区
	ErrStatus_t errCode = ERROR_NONE;
	
	// LCD地址缓冲区
	u16 wrAddr = 0;
	
	char tempor2[20] = { 0 };
	
	errCode = Error_GetError();
	wrAddr = LCD_GetPage() * LCD_PAGE_STEP + LCD_BASE_ADDR;

	if (errCode != ERROR_NONE) {
		LCD_WriteByte(wrAddr, (char *)g_errorText[errCode], 12);
	} else {
		LCD_WriteByte(wrAddr, (char *)tempor2, 12);
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
	
	// 网络图标缓冲区
	u16 netIcon = DISPLAY_ICON_NWK_NONE;
		
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
		case DISPLAY_SHOW_WORKPAGE:
		{   
			// 显示售卖页面
			LCD_SwitchPage(LCD_PAGE_1);
			
			// 显示网络信号
			Disp_SetNetIcon();
			
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

			// 显示标签数
			Disp_SetLabel();
			break;
		}
		case DISPLAY_SHOW_LOGINPAGE:
		{
			// 显示登录页面
			LCD_SwitchPage(LCD_PAGE_0);
			
			// 显示网络信号
			Disp_SetNetIcon();
			
			// 设置登录页面文本显示
			Display_SetLoginText(g_showText.type);

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
//		case DISPLAY_SHOW_ADDPAGE:
//		{
//			// 重置食品列表，显示列表按键
//			if (LCD_GetPage() == LCD_PAGE_2) {
//				Disp_SetList(Poul_GetInfo()->list.group, g_allData.temp.poul.base[1]);
//				Display_ShowOptions(LCD_ADDR_T_02_02_CHOOSE_11);
//			}
//			break;
//		}
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

void Disp_SetList(PoulListGroup_t group, PoulBase_t *base)
{
	u8 i = 0;
	
	// 清空列表
	memset(g_pOpt, 0, sizeof(DispOpt_t));
	
	g_pOpt->group = group;
	for (i = 0; i < base->sum; i++) {
		if (base->kinds[i].kindNum[1] == group) {
			// 添加到指针列表
			g_pOpt->pKinds[g_pOpt->kindSum++] = &base->kinds[i];
		}
	}
	g_pOpt->pageSum = (g_pOpt->kindSum / OPTION_NUMBER) + ((g_pOpt->kindSum % OPTION_NUMBER) ? 1 : 0);
}


/**
  * @brief  显示食品信息库的按键
  * @param  
  * @retval 
  */
void Display_ShowOptions(uint16_t addr)
{
	uint8_t i;
	uint8_t transBuf[10] = { 0 };
	uint8_t gbk_NULL[10] = { 0x20, 0x20, 0x20, 0x20, 0xCE, 0xDE, 0x20, 0x20, 0x20, 0x20 };
	PoulList_t *poulList = &Poul_GetInfo()->list;
	
	// 根据信息列表从信息库中提取信息，并把信息显示到LCD的8个按键
	for (i = 0; i < OPTION_NUMBER; i++) {
		if ((g_pOpt->page * OPTION_NUMBER + i) < poulList->sum) {
			// 显示禽类名称
			Display_SetByteMid(poulList->pKinds[g_pOpt->page * OPTION_NUMBER + i]->name, transBuf);
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

/**
  * @brief  售卖页面按键处理
  * @param  
  * @retval 
  */
static void Page_1_Pro(u16 button)
{
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
}


/**
  * @brief  选择页面按键处理
  * @param  button	按键值
  * @retval 
  */
static void Page_3_Pro(u16 button)
{
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
			poulKind = g_pOpt->pKinds[g_pOpt->page * OPTION_NUMBER + button - LCD_ADDR_B_03_10_CHOOSE_11];
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
			if (g_pOpt->page > 0) {
				g_pOpt->page--;
				Display_ShowOptions(LCD_ADDR_T_03_02_CHOOSE_11);
//			} else {
//				LCD_SwitchPage(LCD_PAGE_5);
			}
			break;
			
		case LCD_ADDR_B_03_20_NEXT:
			// 下一页
			if (g_pOpt->page + 1 < g_pOpt->pageSum) {
				g_pOpt->page++;
				Display_ShowOptions(LCD_ADDR_T_03_02_CHOOSE_11);
//			} else {
//				LCD_SwitchPage(LCD_PAGE_5);
			}
			break;
			
		default:
			break;
	}
}

/**
  * @brief  管理员页面按键处理
  * @param  
  * @retval 
  */
static void Page_6_Pro(u16 button)
{
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

///**
//  * @brief  添加二页面按键处理
//  * @param  
//  * @retval 
//  */
//static void Page_2_Pro(u16 button)
//{
//	if (button == LCD_ADDR_B_02_10_CHOOSE_11) {
//		Poul_AddKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 0]);
//	} else if (button == LCD_ADDR_B_02_11_CHOOSE_12) {
//		Poul_AddKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 1]);
//	} else if (button == LCD_ADDR_B_02_12_CHOOSE_13) {
//		Poul_AddKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 2]);
//	} else if (button == LCD_ADDR_B_02_13_CHOOSE_14) {
//		Poul_AddKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 3]);
//	} else if (button == LCD_ADDR_B_02_14_CHOOSE_21) {
//		Poul_AddKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 4]);
//	} else if (button == LCD_ADDR_B_02_15_CHOOSE_22) {
//		Poul_AddKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 5]);
//	} else if (button == LCD_ADDR_B_02_16_CHOOSE_23) {
//		Poul_AddKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 6]);
//	} else if (button == LCD_ADDR_B_02_17_CHOOSE_24) {
//		Poul_AddKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 7]);
//	} else if (button == LCD_ADDR_B_02_18_DELETE) {
//		// 切换到第4页
//		LCD_SwitchPage(LCD_PAGE_4);
//		
//		// 显示禽类库
//		Disp_SetList(Poul_GetInfo()->list.group, g_allData.temp.poul.base[0]);
//		Display_ShowOptions(LCD_ADDR_T_04_02_CHOOSE_11);

//	} else if (button == LCD_ADDR_B_02_19_LAST) {
//		// 为0时页数固定为0
//		if (g_pOpt->page > 0) {
//			g_pOpt->page--;
//			Display_ShowOptions(LCD_ADDR_T_03_02_CHOOSE_11);
//		}

//	} else if (button == LCD_ADDR_B_02_20_NEXT) {
//		if (g_pOpt->page + 1 < g_pOpt->pageSum) {
//			g_pOpt->page++;
//			Display_ShowOptions(LCD_ADDR_T_03_02_CHOOSE_11);
//		}

//	} else if (button == LCD_ADDR_B_02_21_RETURN) {
//		LCD_SwitchPage(LCD_PAGE_7);
//	}
//}

///**
//  * @brief  删除页面按键处理
//  * @param  
//  * @retval 
//  */
//static void Page_4_Pro(u16 button)
//{
//	if (button == LCD_ADDR_B_04_10_CHOOSE_11) {
//		// 删除食品信息
//		Poul_DelKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 0]);
//		
//		// 重新显示删除后的食品列表按键
//		Display_ShowOptions(LCD_ADDR_T_04_02_CHOOSE_11);

//	} else if (button == LCD_ADDR_B_04_10_CHOOSE_12) {
//		Poul_DelKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 1]);
//		Display_ShowOptions(LCD_ADDR_T_04_02_CHOOSE_11);

//	} else if (button == LCD_ADDR_B_04_10_CHOOSE_13) {
//		Poul_DelKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 2]);
//		Display_ShowOptions(LCD_ADDR_T_04_02_CHOOSE_11);

//	} else if (button == LCD_ADDR_B_04_10_CHOOSE_14) {
//		Poul_DelKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 3]);
//		Display_ShowOptions(LCD_ADDR_T_04_02_CHOOSE_11);

//	} else if (button == LCD_ADDR_B_04_10_CHOOSE_21) {
//		Poul_DelKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 4]);
//		Display_ShowOptions(LCD_ADDR_T_04_02_CHOOSE_11);

//	} else if (button == LCD_ADDR_B_04_10_CHOOSE_22) {
//		Poul_DelKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 5]);
//		Display_ShowOptions(LCD_ADDR_T_04_02_CHOOSE_11);

//	} else if (button == LCD_ADDR_B_04_10_CHOOSE_23) {
//		Poul_DelKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 6]);
//		Display_ShowOptions(LCD_ADDR_T_04_02_CHOOSE_11);

//	} else if (button == LCD_ADDR_B_04_10_CHOOSE_24) {
//		Poul_DelKind(Poul_GetInfo()->list.pKinds[g_pOpt->page * OPTION_NUMBER + 7]);
//		Display_ShowOptions(LCD_ADDR_T_04_02_CHOOSE_11);

//	} else if (button == LCD_ADDR_B_04_10_LAST) {
//		if (g_pOpt->page > 0) {
//			g_pOpt->page--;
//			Display_ShowOptions(LCD_ADDR_T_04_02_CHOOSE_11);
//		}

//	} else if (button == LCD_ADDR_B_04_10_NEXT) {
//		if (g_pOpt->page + 1 < g_pOpt->pageSum) {
//			g_pOpt->page++;
//			Display_ShowOptions(LCD_ADDR_T_04_02_CHOOSE_11);
//		}

//	} else if (button == LCD_ADDR_B_04_10_RETURN) {
//		LCD_SwitchPage(LCD_PAGE_2);
//		
//		// 选项页码复位
//		g_pOpt->page = 0;
//		Disp_SetList(Poul_GetInfo()->list.group, g_allData.temp.poul.base[1]);
//		Display_ShowOptions(LCD_ADDR_T_02_02_CHOOSE_11);

//	}
//}

///**
//  * @brief  超出页面按键处理
//  * @param  
//  * @retval 
//  */
//static void Page_5_Pro(u16 button)
//{
//	if (button == LCD_ADDR_B_05_02_HOME) {
//		Display_SetShow(DISPLAY_SHOW_WORKPAGE);

//	} else if (button == LCD_ADDR_B_05_03_RETURN) {
//		LCD_SwitchPage(LCD_PAGE_3);
//	}
//}

///**
//  * @brief  添加一页面按键处理
//  * @param  
//  * @retval 
//  */
//static void Page_7_Pro(u16 button)
//{
//	if (button == LCD_ADDR_B_07_02_CHICK) {
//		Disp_SetList(POUL_LISTGROUPTYPE_CHICK, g_allData.temp.poul.base[1]);
//		
//		LCD_SwitchPage(LCD_PAGE_2);
//		
//		/* 查询服务器的新增禽类信息 */
//		CMNC_AddCmd(UL_FUNC_8);

//	} else if (button == LCD_ADDR_B_07_03_DUCK) {
//		Disp_SetList(POUL_LISTGROUPTYPE_DUCK, g_allData.temp.poul.base[1]);

//		LCD_SwitchPage(LCD_PAGE_2);
//		CMNC_AddCmd(UL_FUNC_8);

//	} else if (button == LCD_ADDR_B_07_04_GOOSE) {
//		Disp_SetList(POUL_LISTGROUPTYPE_GOOSE, g_allData.temp.poul.base[1]);

//		LCD_SwitchPage(LCD_PAGE_2);
//		CMNC_AddCmd(UL_FUNC_8);

//	} else if (button == LCD_ADDR_B_07_05_OTHERS) {
//		Disp_SetList(POUL_LISTGROUPTYPE_OTHER, g_allData.temp.poul.base[1]);

//		LCD_SwitchPage(LCD_PAGE_2);
//		CMNC_AddCmd(UL_FUNC_8);

//	} else if (button == LCD_ADDR_B_07_06_RETURN) {
//		LCD_SwitchPage(LCD_PAGE_6);
//		
//	}
//}
