#ifndef __DISPLAY_H__
#define __DISPLAY_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "lcd.h"

/* Private typedef -----------------------------------------------------------*/

/**
  * 登陆界面显示文本类型
  */
typedef enum {
	TEXT_NULL = 0,          // “    无    ”
	TEXT_LOGIN,             // "请商户登录"
	TEXT_CHECK_NETWORK,     // “请检查网络”
	TEXT_DEVICE_UNLOGIN,    // “设备未登陆”
	TEXT_CHECK_PLC,         // “请检查机器”
	TEXT_RECARD,            // “请重新刷卡”
	TEXT_GETTING,           // "  获取中  "
	TEXT_LINKING_NETWORK,
	TEXT_LINKING_DEVICER,
	TEXT_INPUT_CUSTOMER,
	TEXT_GPS_DISABLE,       // “定位未获取”
	TEXT_MERCHANT_UNSIGNIN, // “商户未注册”
	TEXT_CUSTOMER_UNSIGNIN, // “客户未注册”
	TEXT_MERCHANT_LOGINING, // “商户登录中”
	TEXT_LOGIN_FAIL,
	TEXT_CUSTOMER_LOGINING,	// “客户登录中”
}ShowTextType_t;

/**
  * 显示文本数据类型
  */
typedef uint16_t ShowText_t[5];

/**
  * 错误信息数据类型
  */
typedef uint8_t ErrorText_t[12];

/**
  * 显示文本信息结构体
  */
typedef struct {
	ShowTextType_t type;
	ShowText_t text[16];
}ShowTextInfo_t;

/**
  * 显示页面类型
  */
typedef enum {
	DISPLAY_PAGE_LOGIN      = 0,
	DISPLAY_PAGE_WORK       = 1,
	DISPLAY_PAGE_ADD        = 2,
	DISPLAY_PAGE_SELECT     = 3,
	DISPLAY_PAGE_DELETE     = 4,
	DISPLAY_PAGE_ADMIN      = 6,
	DISPLAY_PAGE_ERROR      = 8,
}Display_PageType_t;

/**
  * 显示类型
  */
typedef enum {
	DISPLAY_SHOW_RTCTIME,       // 当前时间
	DISPLAY_SHOW_LABELNUM,      // 标签数
	DISPLAY_SHOW_GPSSNR,        // GPS信号强度
	DISPLAY_SHOW_NETICON,       // 网络图标
	DISPLAY_SHOW_ERRCODE,       // 错误码
	DISPLAY_SHOW_WORKPAGE,      // 工作页面
	DISPLAY_SHOW_LOGINPAGE,     // 登录页面
	DISPLAY_SHOW_ADMINPAGE,     // 管理员页面
	DISPLAY_SHOW_ADDPAGE,       // 添加页面
}Display_ShowType_t;

/**
  * 选项显示的参数结构体
  */
typedef struct {
	/**
	  * 当前显示选项页码，从第0页开始
	  */
	uint8_t page;
	
	/**
      * 选项总页数
      */
	uint8_t pageSum;
}Display_Opt_t;

/* Private macro -------------------------------------------------------------*/

#define OPTION_NUMBER                    8

/**
  * 显示网络图标类型
  */
#define DISPLAY_ICON_NWK_NONE            0x0000
#define DISPLAY_ICON_NWK_NET             0x0001
#define DISPLAY_ICON_NWK_SIM             0x0002

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  显示初始化
  */
void        Display_Init              (void);

/**
  * @brief  显示数据处理
  */
void        Display_Process           (void);

/**
  * @brief  设置显示
  */
void        Display_SetShow           (Display_ShowType_t showType);

/**
  * @brief  读取显示页面
  */
Display_PageType_t Display_GetPage(void);

/**
  * @brief  设置显示登录界面文本
  */
void        Display_SetLoginText (ShowTextType_t textType);

#endif //__DISPLAY_H__
