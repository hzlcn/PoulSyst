#ifndef __UPGRADE_H__
#define __UPGRADE_H__

/* Includes ------------------------------------------------------------------*/

#include "common.h"
#include "stmflash.h"

/* Private typedef -----------------------------------------------------------*/

typedef struct {
	u8   paraFlag;  // ������־
	bool upgFlag;   // ���±�־
	u8   firmVer;   // �̼��汾
	u32  firmIndex; // �̼���ַָ��
	u32  firmSize;  // �̼���С
	u16  firmCrc;   // �̼�У��CRC
}UpgInfo_t;

/* Private macro -------------------------------------------------------------*/

#define BOOT_BASE_ADDR              ADDR_FLASH_SECTOR_0
#define APP_BASE_ADDR               ADDR_FLASH_SECTOR_1

#define UPGRADE_FRAG_SIZE           0x400
#define UPGRADE_BASE_ADDR           0x01E01000
#define UPGRADE_PARA_ADDR           0x01EFF000
#define UPGRADE_PARA_FLAG           0x21

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ��ʼ���̼�����
  */
void        Upg_Init(void);

/**
  * @brief  ����̼�����(Boot)
  */
void        Upg_Process(void);

/**
  * @brief  ��ȡ�̼�����ָ��
  */
UpgInfo_t * Upg_GetInfo(void);

/**
  * @brief  ��ʼ�̼�����
  */
void        Upg_Start(u8 firmVer, u32 firmSize, u16 firmCrc);

/**
  * @brief  ��ȡ��Ƭ�����浽�ڲ�FLASH��
  */
void        Upg_ReadFrag(void);

/**
  * @brief  ������Ƭ
  */
void        Upg_SaveFrag(u8 *pBuf);

#endif /* __UPGRADE_H__ */

