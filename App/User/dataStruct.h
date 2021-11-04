#ifndef __DATASTRUCT_H__
#define __DATASTRUCT_H__

#include "common.h"
#include "serial.h"
// 本头文件用于定义用户数据结构

/* Private define ------------------------------------------------------------*/

/**
  * 最大用户信息数
  */
#define N_MERCBASE_MAX_INFO 30

/**
  * 用户(店主或客户)的二维码长度
  */
#define USER_CODE_SIZE      20

/**
  * 食品信息码大小
  */
#define N_POULTRY_QRCODE_BUFFER_SIZE    24

/**
  * 最大食品信息条数
  */
#define N_POULTRY_MAX_KIND              30

/**
  * 食品信息 - 种类序号长度[6] = 大类序号[2] + 小类序号[4]
  */
#define N_POULTRY_KIND_NUMS_SIZE        6

/* Private typedef -----------------------------------------------------------*/
/**
  * 显示文本数据类型
  */
typedef uint16_t ShowText_t[5];

/**
  * 错误信息数据类型
  */
typedef uint8_t ErrorText_t[12];

/**
  * 用户权限类型
  */
typedef enum {
	USER_PERM_ADMIN         = 0,    // 管理员
	USER_PERM_MAINTAIN      = 1,    // 维护员
	USER_PERM_MERCHANT      = 2,    // 普通用户
	USER_PERM_CUSTOMER      = 3,    // 普通客户
	USER_PERM_NULL          = 0xFF, // 无权限
}UserPerm_t;

/**
  * 食品列表类型
  */
typedef enum {
	POUL_LISTTYPE_NONE = 0,
	POUL_LISTTYPE_CUR  = 1,             // 食品信息列表
	POUL_LISTTYPE_ADD  = 2,             // 添加信息列表
}PoulListType_t;

/**
  * 食品类型
  */
typedef enum {
	POUL_LISTGROUPTYPE_CHICK = 0,       // 鸡
	POUL_LISTGROUPTYPE_DUCK  = 1,       // 鸭
	POUL_LISTGROUPTYPE_GOOSE = 2,       // 鹅
	POUL_LISTGROUPTYPE_OTHER = 3,       // 其它
	POUL_LISTGROUPTYPE_NONE  = 4,       // 无
}PoulListGroup_t;

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
  * 单条店主信息结构体
  */
typedef struct {
	UserPerm_t perm;
	char code[USER_CODE_SIZE];
}Merchant_t;

/**
  * 单条客户信息结构体
  */
typedef struct {
	UserPerm_t perm;
	char code[USER_CODE_SIZE];
	u8 name[20];
}Customer_t;

/**
  * 店主信息库结构体
  */
typedef struct {
	uint8_t keepFlag;
	uint8_t Version[2];             // 库版本
	uint8_t Sum;                    // 库的信息数
	struct {
		Merchant_t Info[N_MERCBASE_MAX_INFO];
	}List;                          // 库的信息列表
}MercBase_t;

/**
  * 用户二维码参数结构体
  */
typedef struct {
	bool isReady;
	uint8_t code[UART5_BUFFSIZE];
}RxQrcode_t;

/**
  * 用户信息结构体
  */
typedef struct {
	Merchant_t merc;
	Customer_t cust;
	RxQrcode_t recv;
	MercBase_t *mercBase;
}UserInfo_t;

/* Private typedef -----------------------------------------------------------*/

/**
  * 食品种类信息结构体
  */
typedef struct {
	// 前2字节为PoulListGroup_t类型，后4字节为更细化的类别。(例如: 前2字节表示鸡，后4字节表示山地鸡。)
	u8 kindNum[N_POULTRY_KIND_NUMS_SIZE];
	
	// 食品种类的名称(GBK编码)
	u8 name[10];
}PoulKind_t;

/**
  * 食品种类库
  */
typedef PoulKind_t PoulKinds_t[N_POULTRY_MAX_KIND];

/**
  * 食品信息库结构体
  */
typedef struct {
	u8 keepFlag;
	u8 version[2];
	u8 sum;
	PoulKinds_t kinds;
}PoulBase_t;

/**
  * 食品数据列表
  */
typedef struct {
	// 当前列表类型
	PoulListType_t type;
	
	// 食品组类
	PoulListGroup_t group;
	
	// 信息总数
	u8 sum;
	
	// 食品信息库索引列表
	PoulKind_t *pKinds[N_POULTRY_MAX_KIND];
}PoulList_t;

/**
  * 食品编码信息结构体
  */
typedef struct {
	bool isReady;
	u8   qrcode[N_POULTRY_QRCODE_BUFFER_SIZE];
}PoulCode_t;

/**
  * 食品种类信息结构体
  */
typedef struct {
	PoulKind_t *pCurKind;		// 当前食品种类
	PoulCode_t  code;           // 食品二维码
	PoulList_t  list;           // 食品列表
	PoulBase_t *base[2];		// 食品种类库
}PoulInfo_t;

/**
  * GPS卫星信息结构体
  */
typedef struct {
	char svNum[2];          // 可见卫星数
	char svSnr[2];          // 信躁比
}GPS_SVParam_t;

/**
  * 显示文本信息结构体
  */
typedef struct {
	ShowTextType_t type;
	ShowText_t text[16];
}ShowTextInfo_t;

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

/* Private typedef -----------------------------------------------------------*/
/**
  * 临时数据结构体
  */
typedef struct sTempPara{
	UserInfo_t user;
	PoulInfo_t poul;
	GPS_SVParam_t gps;
}TempPara_t;

/**
  * 存储数据结构体
  */
typedef struct sSavePara{
	int flag;
}SavePara_t;

/**
  * 数据结构体
  */
typedef struct {
	TempPara_t temp;
	SavePara_t save;
}DataStruct_t;

extern DataStruct_t g_allData;
extern SavePara_t *g_savPara;
extern TempPara_t *g_tmpPara;

#endif /* __DATASTRUCT_H__ */
