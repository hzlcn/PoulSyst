#ifndef __USERMSG_H__
#define __USERMSG_H__

#include "common.h"
#include "serial.h"
#include "datastruct.h"

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
  * @brief  设置客户名称
  */
void        User_SetCust(u8 *name);

/**
  * @brief  设置为商户(管理员或维护员使用)
  */
void        User_SetCustAdmin(void);


#endif /* __USERMSG_H__ */
