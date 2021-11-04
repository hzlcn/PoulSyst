#ifndef __GPS_H__
#define __GPS_H__

#include "common.h"

/**
  * GPS������Ϣ�ṹ��
  */
typedef struct {
	char svNum[2];          // �ɼ�������
	char svSnr[2];          // �����
}GPS_SVParam_t;

/**
  * @brief  ����GPS����
  */
void GPS_Init(void);

/**
  * @brief  ѭ������GPS����
  */
void GPS_Process(void);

/**
  * @brief  ��ȡ������Ϣ
  */
GPS_SVParam_t *GPS_GetSVDate(void);

/**
  * @brief  �ж�������Ϣ�Ƿ��Ѿ���ȡ
  */
bool GPS_SVIsReady(void);

/**
  * @brief  ��ȡ��λ��Ϣ
  */
bool GPS_GetLLData(uint8_t *pBuffer);

/**
  * @brief  �ж϶�λ��Ϣ�Ƿ��Ѿ���ȡ
  */
bool GPS_LLIsReady(void);


#endif /* __GPS_H__ */
