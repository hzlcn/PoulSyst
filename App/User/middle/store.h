#ifndef __STORE_H__
#define __STORE_H__

#include "common.h"

#include "poultry.h"
#include "usermsg.h"


/* Private macro -------------------------------------------------------------*/

#define SYSTEM_TIME_SIZE       6
#define REMAIN_LABEL_NUM_SIZE  2
#define RESERVE_SIZE           (100 - N_POULTRY_QRCODE_BUFFER_SIZE - N_POULTRY_KIND_NUMS_SIZE - USER_CODE_SIZE - \
								SYSTEM_TIME_SIZE - REMAIN_LABEL_NUM_SIZE)

// W25Q256����ַ: 0x02000000
#define RECORD_NUMBER_ADDR     0x01FFE000
#define RECORD_BLOCK_MAX_ADDR  0x01D00000

/* Private typedef -----------------------------------------------------------*/

/**
  * �豸��ַ����
  */
typedef uint8_t DevAddr_t[6];

/**
  * ��Ӳ���汾����
  */
typedef struct {
	uint8_t hard;
	uint8_t soft;
}Version_t;

/**
  * ���ò���
  */
typedef struct {
	uint8_t   keepFlag;
	DevAddr_t devAddr;
	Version_t version;
}ConfInfo_t;

/**
  * ������Ϣ�ṹ��
  */
typedef struct
{
    uint8_t poulCode[N_POULTRY_QRCODE_BUFFER_SIZE];  // ��Ʒ��ά��
    uint8_t poulKind[N_POULTRY_KIND_NUMS_SIZE];      // ��Ʒ����
    uint8_t custCode[USER_CODE_SIZE];                // �ͻ���ά��
    uint8_t rtcTime[SYSTEM_TIME_SIZE];               // ����ʱ��
    uint8_t remainNum[REMAIN_LABEL_NUM_SIZE];        // ʣ��������
}RecordBlock_t;

/**
  * ���׼�¼�ṹ��
  */
typedef struct {
	u8 keepFlag;
	uint32_t blockNum;                               // ��Ϣ����
	uint32_t syncNum;                                // ���ϱ�����Ϣ����
}RecordNumber_t;

/**
  * ������Ϣ
  */
typedef struct {
	RecordNumber_t num;
	RecordBlock_t block;
	uint8_t upNum; // �����ϱ��Ľ��׼�¼����(�����ϱ��ɹ��������ϱ���Ľ�����Ϣ��)
}RecordInfo_t;


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ��ȡ���ò���ָ��
  */
ConfInfo_t *Store_GetConfig(void);

/**
  * @brief  ��ȡ���ײ�����ַָ��
  */
RecordInfo_t *Store_GetRecord(void);

/**
  * @brief  ��ȡ���ò���
  */
void        Store_Init(void);

/**
  * @brief  �����Ϣ�ϱ�
  */
void        Store_Process(void);

/**
  * @brief  �������ò���
  */
void        Store_SaveConfig(void);

/**
  * @brief  �洢��������Ϣ
  */
void        Store_SaveMercBase(void);

/**
  * @brief  �洢ʳƷ����Ϣ
  */
void        Store_SavePoulBase(void);

/**
  * @brief  ��ȡ������Ϣ
  */
void        Store_SaveBlock(void);

/**
  * @brief  ��ȡ���׼�¼(���10��)
  */
uint8_t     Store_ReadBlock(uint8_t *pBuffer);

/**
  * @brief  ������Ϣ����
  */
void        Store_SaveRecordNum(void);

#endif /* __STORE_H__ */
