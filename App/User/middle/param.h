#ifndef __PARAM_H__
#define __PARAM_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"


/**
  * ����״̬����
  */
typedef bool ErrorStatus_t;

typedef enum {
	ERROR_NONE = 0,
	ERROR_DEVICE_UNCOMMUNICATE,         // PLC��ͨѶ
	ERROR_GPS_UNCOMMUNICATE,   // GPS����Ϣ
	ERROR_USERQRCODE_UNCOMMUNICATE,      // ��ɨ��������
	ERROR_POULQRCODE_UNCOMMUNICATE,      // ��ɨ��������
	ERROR_POULQRCODE_NO_CODE,         // ��ʳƷ��Ϣ
	ERROR_GPS_NO_SVDATA,       // ��������Ϣ
	ERROR_GPS_NO_LOCATION,       // �޶�λ��Ϣ
	ERROR_PLC_UNLOCK,       // δ����
	
}ErrStatus_t;

/* Private macro -------------------------------------------------------------*/

// �ܱ�ǩ�� �̶�ֵ  ��Ҫ�޸�
#define LABEL_TOTAL_NUMBER 									8888

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ��ȡ����״ָ̬��
  */
ErrorStatus_t *Param_GetErrorStatus(void);

/**
  * @brief  ��ʼ������״̬
  */
void        Error_Init(void);

/**
  * @brief  ��Ӵ���
  */
void        Error_Add(ErrStatus_t status);

/**
  * @brief  ɾ������
  */
void        Error_Del(ErrStatus_t status);

/**
  * @brief  ��ȡ����ֵ��ߵĴ���
  */
ErrStatus_t Error_GetError(void);

#endif /* __PARAM_H__ */
