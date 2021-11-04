/* Includes ------------------------------------------------------------------*/
#include "usermsg.h"

/* Private macro -------------------------------------------------------------*/

/**
  * �û���ά���С
  */
#define     USERQRCODE_MIN_SIZE     20

/* Private variables ---------------------------------------------------------*/

/**
  * ��ʼ���û���Ϣ��
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
  * ������Ϣ��
  */
static MercBase_t g_mercBase = { 0 };

/**
  * �û���Ϣ������
  */
static UserInfo_t g_userInfo = {
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
};

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ��ȡ�û���Ϣ
  * @param  
  * @retval 
  */
UserInfo_t *User_GetInfo(void)
{
	return &g_userInfo;
}

/**
  * @brief  �����û���ά��ģ��
  * @param  
  * @retval 
  */
void User_InitModule(void)
{
	Uart_SetRxEnable(&huart5);
}

/**
  * @brief  �����û���ά������
  * @param  
  * @retval 
  */
void User_Process(void)
{
	u8 i, dataError = 0, userRxTmp[UART5_BUFFSIZE] = { 0 };
	u16 rxLen = Uart_GetData(&huart5, userRxTmp);
	if (rxLen > 0) {
		// ������ݴ���
		for (i = 0; i < rxLen; i++) {
			if (userRxTmp[i] == 0x00) {
				dataError = 1;
				break;
			}
		}
		
		if ((g_userInfo.recv.isReady == false) && (rxLen >= USERQRCODE_MIN_SIZE) && (dataError == 0)) {
			g_userInfo.recv.isReady = true;
			memcpy(g_userInfo.recv.code, userRxTmp, rxLen);
		}
		
		// ��ս��ջ�����
		memset(userRxTmp, 0, UART5_BUFFSIZE);
	}
}

/**
  * @brief  ���ý��յ��Ķ�ά��Ϊ�û���
  * @param  
  * @retval 
  */
void User_SetCode(UserPerm_t perm)
{
	if ((perm == USER_PERM_MERCHANT) || (perm == USER_PERM_ADMIN) || (perm == USER_PERM_MAINTAIN)) {
		g_userInfo.merc.perm = perm;
		memcpy(g_userInfo.merc.code, g_userInfo.recv.code, USER_CODE_SIZE);
	} else if (perm == USER_PERM_CUSTOMER) {
		g_userInfo.cust.perm = USER_PERM_CUSTOMER;
		memcpy(g_userInfo.cust.code, g_userInfo.recv.code, USER_CODE_SIZE);
	}
}

/**
  * @brief  ����û���ά������
  * @param  
  * @retval 
  */
void User_ClearRecv(void)
{
	memset(g_userInfo.recv.code, 0, USERQRCODE_MIN_SIZE);
	g_userInfo.recv.isReady = false;
}

/**
  * @brief  ��λ�û���ά��ģ��
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
  * @brief  ��ʼ��������Ϣ
  * @param  
  * @retval 
  */
void User_InitMerc(void)
{   // ��λ������Ϣ������
	g_userInfo.merc.perm = USER_PERM_NULL;
	memset(g_userInfo.merc.code, 0, 20);
}

/**
  * @brief  �����û���Ϣ
  * @param  
  * @retval true  - ���������û�
  *         false - δ���������û�
  */
bool User_CheckLocal(void)
{
	u8 i = 0;
	for (i = 0; i < g_userInfo.mercBase->Sum; i++) {
		if (memcmp(&g_userInfo.mercBase->List.Info[i].code, g_userInfo.recv.code, USER_CODE_SIZE) == 0) {
			memcpy(&g_userInfo.merc, &g_userInfo.mercBase->List.Info[i], sizeof(Merchant_t));
			return true;
		}
	}

	return false;
}

/**
  * @brief  �����û���Ϣ��Ϊ��ʼֵ
  * @param  
  * @retval 
  */
void User_InitMercBase(void)
{
	memcpy(g_userInfo.mercBase, &g_mercDefaultBase, sizeof(MercBase_t));
}

/**
  * @brief  ��ʼ���ͻ���Ϣ
  * @param  
  * @retval 
  */
void User_InitCust(void)
{   // ��λ�ͻ���Ϣ������
	memset(g_userInfo.cust.code, 0, USER_CODE_SIZE);
	memset(g_userInfo.cust.name, 0, 20);
	g_userInfo.cust.perm = USER_PERM_NULL;
}

/**
  * @brief  ���ÿͻ�����
  * @param  
  * @retval 
  */
void User_SetCust(u8 *name)
{
	memcpy(g_userInfo.cust.name, name, 20);
}

/**
  * @brief  ���ͻ�����Ϊ����(����Ա��ά��Աʹ��)
  * @param  
  * @retval 
  */
void User_SetCustAdmin(void)
{
	if (g_userInfo.merc.perm != USER_PERM_NULL) {
		g_userInfo.cust.perm = USER_PERM_CUSTOMER;
		memcpy(g_userInfo.cust.code, g_userInfo.merc.code, USER_CODE_SIZE);
	}
}

