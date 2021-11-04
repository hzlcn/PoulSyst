#ifndef __USERMSG_H__
#define __USERMSG_H__

#include "common.h"
#include "serial.h"

/* Private define ------------------------------------------------------------*/

/**
  * 最大用户信息数
  */
#define N_MERCBASE_MAX_INFO 30

/**
  * 用户(店主或客户)的二维码长度
  */
#define USER_CODE_SIZE      20

/* Private typedef -----------------------------------------------------------*/

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

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  开启用户二维码模块
  */
void        User_InitModule(void);

/**
  * @brief  循环处理用户二维码数据
  */
void        User_Process(void);

/**
  * @brief  设置接收到的数据为用户码
  */
void        User_SetCode(UserPerm_t perm);

/**
  * @brief  清除接收数据
  */
void        User_ClearRecv(void);

/**
  * @brief  复位用户二维码模块
  */
void        User_ResetModule(void);

/**
  * @brief  获取用户信息
  */
UserInfo_t *User_GetInfo(void);

/**
  * @brief  初始化店主信息
  */
void        User_InitMerc(void);

/**
  * @brief  检索用户信息
  */
bool        User_CheckLocal(void);

/**
  * @brief  设置用户信息库为初始值
  */
void        User_InitMercBase(void);

/**
  * @brief  初始化客户信息
  */
void        User_InitCust(void);

/**
  * @brief  设置客户名称
  */
void        User_SetCust(u8 *name);

/**
  * @brief  设置为商户(管理员或维护员使用)
  */
void        User_SetCustAdmin(void);


#endif /* __USERMSG_H__ */
