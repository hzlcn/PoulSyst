#ifndef __USERMSG_H__
#define __USERMSG_H__

#include "common.h"
#include "serial.h"

/* Private define ------------------------------------------------------------*/

/**
  * ����û���Ϣ��
  */
#define N_MERCBASE_MAX_INFO 30

/**
  * �û�(������ͻ�)�Ķ�ά�볤��
  */
#define USER_CODE_SIZE      20

/* Private typedef -----------------------------------------------------------*/

/**
  * �û�Ȩ������
  */
typedef enum {
	USER_PERM_ADMIN         = 0,    // ����Ա
	USER_PERM_MAINTAIN      = 1,    // ά��Ա
	USER_PERM_MERCHANT      = 2,    // ��ͨ�û�
	USER_PERM_CUSTOMER      = 3,    // ��ͨ�ͻ�
	USER_PERM_NULL          = 0xFF, // ��Ȩ��
}UserPerm_t;

/**
  * ����������Ϣ�ṹ��
  */
typedef struct {
	UserPerm_t perm;
	char code[USER_CODE_SIZE];
}Merchant_t;

/**
  * �����ͻ���Ϣ�ṹ��
  */
typedef struct {
	UserPerm_t perm;
	char code[USER_CODE_SIZE];
	u8 name[20];
}Customer_t;

/**
  * ������Ϣ��ṹ��
  */
typedef struct {
	uint8_t keepFlag;
	uint8_t Version[2];             // ��汾
	uint8_t Sum;                    // �����Ϣ��
	struct {
		Merchant_t Info[N_MERCBASE_MAX_INFO];
	}List;                          // �����Ϣ�б�
}MercBase_t;

/**
  * �û���ά������ṹ��
  */
typedef struct {
	bool isReady;
	uint8_t code[UART5_BUFFSIZE];
}RxQrcode_t;

/**
  * �û���Ϣ�ṹ��
  */
typedef struct {
	Merchant_t merc;
	Customer_t cust;
	RxQrcode_t recv;
	MercBase_t *mercBase;
}UserInfo_t;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  �����û���ά��ģ��
  */
void        User_InitModule(void);

/**
  * @brief  ѭ�������û���ά������
  */
void        User_Process(void);

/**
  * @brief  ���ý��յ�������Ϊ�û���
  */
void        User_SetCode(UserPerm_t perm);

/**
  * @brief  �����������
  */
void        User_ClearRecv(void);

/**
  * @brief  ��λ�û���ά��ģ��
  */
void        User_ResetModule(void);

/**
  * @brief  ��ȡ�û���Ϣ
  */
UserInfo_t *User_GetInfo(void);

/**
  * @brief  ��ʼ��������Ϣ
  */
void        User_InitMerc(void);

/**
  * @brief  �����û���Ϣ
  */
bool        User_CheckLocal(void);

/**
  * @brief  �����û���Ϣ��Ϊ��ʼֵ
  */
void        User_InitMercBase(void);

/**
  * @brief  ��ʼ���ͻ���Ϣ
  */
void        User_InitCust(void);

/**
  * @brief  ���ÿͻ�����
  */
void        User_SetCust(u8 *name);

/**
  * @brief  ����Ϊ�̻�(����Ա��ά��Աʹ��)
  */
void        User_SetCustAdmin(void);


#endif /* __USERMSG_H__ */
