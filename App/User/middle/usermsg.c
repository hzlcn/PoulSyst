/* Includes ------------------------------------------------------------------*/
#include "usermsg.h"

/* Private variables ---------------------------------------------------------*/

/**
  * 初始的用户信息库
  */
static MercBase_t g_mercDefaultBase = {
	.keepFlag = 0,
	.Version = { 0, 0 },
	.Sum = 3,
	.List = {
		.Info[0] = { USER_PERM_ADMIN, { "ML9lcb8vjLRHKwaj2u9G" } },
		.Info[1] = { USER_PERM_MERCHANT,  { "MJUsVhJZ2NGuHSKJdIiS" } },
		.Info[2] = { USER_PERM_MERCHANT,  { "https://u.wechat.com" } },
	}
};

/**
  * 用户信息缓冲区
  */
static UserInfo_t *g_userInfo = &g_allData.temp.user;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  获取用户信息
  * @param  
  * @retval 
  */
UserInfo_t *User_GetInfo(void)
{
	return g_userInfo;
}

/**
  * @brief  开启用户二维码模块
  * @param  
  * @retval 
  */
void User_InitModule(void)
{
	Uart_SetRxEnable(&huart5);
}

/**
  * @brief  处理用户二维码数据
  * @param  
  * @retval 
  */
void User_Process(void)
{
	u8 i, userRxTmp[UART5_BUFFSIZE] = { 0 };
	u16 rxLen = Uart_GetData(&huart5, userRxTmp);
	if (rxLen == 0) {
		return;
	}
	
	// 检查数据内容
	for (i = 0; i < rxLen; i++) {
		if (userRxTmp[i] == 0x00) {
			return;
		}
	}
	
	// 检查数据长度
	if (rxLen < USER_CODE_SIZE) {
		return;
	}
	
	// 检查缓冲区是否空闲
	if (g_userInfo->recv.isReady == true) {
		return;
	}
	
	// 设置完成接收数据标志
	g_userInfo->recv.isReady = true;
	memcpy(g_userInfo->recv.code, userRxTmp, rxLen);
}

/**
  * @brief  设置接收到的二维码为用户码
  * @param  
  * @retval 
  */
void User_SetCode(UserPerm_t perm)
{
	if ((perm == USER_PERM_MERCHANT) || (perm == USER_PERM_ADMIN) || (perm == USER_PERM_MAINTAIN)) {
		g_userInfo->merc.perm = perm;
		memcpy(g_userInfo->merc.code, g_userInfo->recv.code, USER_CODE_SIZE);
		
	} else if (perm == USER_PERM_CUSTOMER) {
		g_userInfo->cust.perm = perm;
		memcpy(g_userInfo->cust.code, g_userInfo->recv.code, USER_CODE_SIZE);
	}
}

/**
  * @brief  清除用户二维码数据
  * @param  
  * @retval 
  */
void User_ClearRecv(void)
{
	memset(g_userInfo->recv.code, 0, USER_CODE_SIZE);
	g_userInfo->recv.isReady = false;
}

/**
  * @brief  复位用户二维码模块
  * @param  
  * @retval 
  */
void User_ResetModule(void)
{
//	uint8_t closeQR[11] = { 0x57, 0x00, 0x5D, 0x00, 0x01, 0x00, 0x00, 0x9A, 0xC8, 0x50, 0x00 };
//	uint8_t openQR[11] = { 0x57, 0x00, 0x5D, 0x00, 0x01, 0x00, 0x01, 0x5B, 0x08, 0x50, 0x00 };
//	
//	Uart5_Send(closeQR, 11);
//	osDelay(10);
//	Uart5_Send(openQR, 11);
	
//	E3000_OFF;
//	osDelay(2000);
//	E3000_ON;
}

/**
  * @brief  初始化店主信息
  * @param  
  * @retval 
  */
void User_InitMerc(void)
{   // 复位店主信息缓冲区
	g_userInfo->merc.perm = USER_PERM_NULL;
	memset(g_userInfo->merc.code, 0, 20);
}

/**
  * @brief  检索用户信息
  * @param  
  * @retval true  - 检索到该用户
  *         false - 未检索到该用户
  */
bool User_CheckLocal(void)
{
	u8 i = 0;
	
	// 逐个检索用户
	for (i = 0; i < g_userInfo->mercBase->Sum; i++) {
		if (memcmp(&g_userInfo->mercBase->List.Info[i].code, g_userInfo->recv.code, USER_CODE_SIZE) == 0) {
			// 检查成功
			memcpy(&g_userInfo->merc, &g_userInfo->mercBase->List.Info[i], sizeof(Merchant_t));
			g_userInfo->merc.perm += 1;
			return true;
		}
	}

	return false;
}

/**
  * @brief  设置用户信息库为初始值
  * @param  
  * @retval 
  */
void User_InitMercBase(void)
{
	memcpy(g_userInfo->mercBase, &g_mercDefaultBase, sizeof(MercBase_t));
}

/**
  * @brief  设置客户名称
  * @param  
  * @retval 
  */
void User_SetCust(u8 *name)
{
	memcpy(g_userInfo->cust.name, name, 20);
}

/**
  * @brief  将客户设置为店主(管理员或维护员使用)
  * @param  
  * @retval 
  */
void User_SetCustAdmin(void)
{
	if (g_userInfo->merc.perm != USER_PERM_NULL) {
		g_userInfo->cust.perm = USER_PERM_CUSTOMER;
		memcpy(g_userInfo->cust.code, g_userInfo->merc.code, USER_CODE_SIZE);
	}
}

