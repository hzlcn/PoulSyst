#ifndef __USERMSG_H__
#define __USERMSG_H__

#include "common.h"
#include "serial.h"
#include "datastruct.h"

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
  * @brief  ���ÿͻ�����
  */
void        User_SetCust(u8 *name);

/**
  * @brief  ����Ϊ�̻�(����Ա��ά��Աʹ��)
  */
void        User_SetCustAdmin(void);


#endif /* __USERMSG_H__ */
