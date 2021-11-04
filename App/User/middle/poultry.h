#ifndef __POULTRY_H__
#define __POULTRY_H__

#include "common.h"
#include "serial.h"

/* Private macro -------------------------------------------------------------*/

/**
  * ʳƷ��Ϣ���С
  */
#define N_POULTRY_QRCODE_BUFFER_SIZE    24

/**
  * ���ʳƷ��Ϣ����
  */
#define N_POULTRY_MAX_KIND              30

/**
  * ʳƷ��Ϣ - ������ų���[6] = �������[2] + С�����[4]
  */
#define N_POULTRY_KIND_NUMS_SIZE        6

/* Private typedef -----------------------------------------------------------*/

/**
  * ʳƷ�б�����
  */
typedef enum {
	POUL_LISTTYPE_NONE = 0,
	POUL_LISTTYPE_CUR  = 1,             // ʳƷ��Ϣ�б�
	POUL_LISTTYPE_ADD  = 2,             // �����Ϣ�б�
}PoulListType_t;

/**
  * ʳƷ����
  */
typedef enum {
	POUL_LISTGROUPTYPE_CHICK = 0,       // ��
	POUL_LISTGROUPTYPE_DUCK  = 1,       // Ѽ
	POUL_LISTGROUPTYPE_GOOSE = 2,       // ��
	POUL_LISTGROUPTYPE_OTHER = 3,       // ����
	POUL_LISTGROUPTYPE_NONE  = 4,       // ��
}PoulListGroup_t;

/**
  * ʳƷ������Ϣ�ṹ��
  */
typedef struct {
	// ǰ2�ֽ�ΪPoulListGroup_t���ͣ���4�ֽ�Ϊ��ϸ�������(����: ǰ2�ֽڱ�ʾ������4�ֽڱ�ʾɽ�ؼ���)
	u8 kindNum[N_POULTRY_KIND_NUMS_SIZE];
	
	// ʳƷ���������(GBK����)
	u8 name[10];
}PoulKind_t;

/**
  * ʳƷ�����
  */
typedef PoulKind_t PoulKinds_t[N_POULTRY_MAX_KIND];

/**
  * ʳƷ��Ϣ��ṹ��
  */
typedef struct {
	u8 keepFlag;
	u8 version[2];
	u8 sum;
	PoulKinds_t kinds;
}PoulBase_t;

/**
  * ʳƷ�����б�
  */
typedef struct {
	// ��ǰ�б�����
	PoulListType_t type;
	
	// ʳƷ����
	PoulListGroup_t group;
	
	// ��Ϣ����
	u8 sum;
	
	// ʳƷ��Ϣ�������б�
	PoulKind_t *pKinds[N_POULTRY_MAX_KIND];
}PoulList_t;

/**
  * ʳƷ������Ϣ�ṹ��
  */
typedef struct {
	bool isReady;
	u8   qrcode[N_POULTRY_QRCODE_BUFFER_SIZE];
}PoulCode_t;

/**
  * ʳƷ������Ϣ�ṹ��
  */
typedef struct {
	PoulKind_t *pCurKind;		// ��ǰʳƷ����
	PoulCode_t  code;           // ʳƷ��ά��
	PoulList_t  list;           // ʳƷ�б�
	PoulBase_t *base[2];		// ʳƷ�����
}PoulInfo_t;

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
