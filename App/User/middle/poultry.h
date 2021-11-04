#ifndef __POULTRY_H__
#define __POULTRY_H__

#include "common.h"
#include "serial.h"
#include "dataStruct.h"


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ��ʼ��ʳƷ��ά��ģ��
  */
void        Poul_InitModule(void);

/**
  * @brief  ����ʳƷ��ά������(����2)
  */
void        Poul_Process(void);

/**
  * @brief  ��ȡʳƷ������Ϣ������ַָ��
  */
PoulInfo_t *Poul_GetInfo(void);

/**
  * @brief  ���ʳƷ��ά��
  */
void        Poul_ClrCode(void);

/**
  * @brief  ����ʳƷ����ָ���б�
  */
void        Poul_SetList(PoulListGroup_t group, PoulListType_t type);

/**
  * @brief  ���õ�ǰʳƷ����
  */
void        Poul_SetKind(PoulKind_t *kind);

/**
  * @brief  ��ӵ���ʳƷ��Ϣ���ڲ������
  */
void        Poul_AddKind(PoulKind_t *kind);

/**
  * @brief  ͨ���������ڲ��������ɾ������ʳƷ��Ϣ
  */
void        Poul_DelKind(PoulKind_t *kind);

/**
  * @brief  ����ʳƷ��Ϣ��Ϊ��ʼֵ
  */
void        Poul_DefBase(void);

#endif /* __PRODUCT_H__ */
