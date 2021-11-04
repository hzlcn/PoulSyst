#include "upgrade.h"
#include "w25qxx.h"
#include "stmflash.h"
#include "iap.h"


/* Private variables ---------------------------------------------------------*/

/**
  * �̼�����
  */
static UpgInfo_t g_upgInfo = {
	.paraFlag = 0,
	.upgFlag  = false,
	.firmVer  = 0,
	.firmIndex = 0,
	.firmSize = 0,
	.firmCrc  = 0,
};

/**
  * ��Ƭ������
  */
static u8 g_upgFragBuf[UPGRADE_FRAG_SIZE] = { 0 };

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ��ʼ���̼�����
  * @param  
  * @retval 
  */
void Upg_Init(void)
{
	W25QXX_Init();
	
	// ��ȡ���²���
	W25QXX_Read((u8 *)&g_upgInfo, UPGRADE_PARA_ADDR, sizeof(UpgInfo_t));
	if (g_upgInfo.paraFlag != UPGRADE_PARA_FLAG) {
		memset(&g_upgInfo, 0, sizeof(UpgInfo_t));
	}

	if (g_upgInfo.upgFlag == true) {
		STMFLASH_Erase(ADDR_FLASH_SECTOR_1);
		FLASH_WaitForLastOperation(FLASH_WAITETIME);            //�ȴ��ϴβ������
		STMFLASH_Erase(ADDR_FLASH_SECTOR_2);
		FLASH_WaitForLastOperation(FLASH_WAITETIME);            //�ȴ��ϴβ������
		STMFLASH_Erase(ADDR_FLASH_SECTOR_3);
		FLASH_WaitForLastOperation(FLASH_WAITETIME);            //�ȴ��ϴβ������
		STMFLASH_Erase(ADDR_FLASH_SECTOR_4);
		FLASH_WaitForLastOperation(FLASH_WAITETIME);            //�ȴ��ϴβ������
		STMFLASH_Erase(ADDR_FLASH_SECTOR_5);
		FLASH_WaitForLastOperation(FLASH_WAITETIME);            //�ȴ��ϴβ������
	}
}

/**
  * @brief  ����̼�����(Boot)
  * @param  
  * @retval 
  */
void Upg_Process(void)
{
	if (g_upgInfo.firmIndex < g_upgInfo.firmSize) {
		Upg_ReadFrag();
	} else {
		g_upgInfo.upgFlag = false;
	}
}

/**
  * @brief  ��ȡ�̼�����ָ��
  * @param  
  * @retval 
  */
UpgInfo_t *Upg_GetInfo(void)
{
	return &g_upgInfo;
}

/**
  * @brief  ��ʼ�̼�����
  * @param  
  * @retval 
  */
void Upg_Start(u8 firmVer, u32 firmSize, u16 firmCrc)
{
	u8 i;
	
	// ���ò���
	g_upgInfo.firmVer = firmVer;
	g_upgInfo.firmSize = firmSize;
	g_upgInfo.firmCrc = firmCrc;
	
	// ����ⲿFlash�Ĵ洢����
	for (i = 0; i < (firmSize >> 12); i++) {
		W25QXX_Erase_Sector(UPGRADE_BASE_ADDR + (i << 12));
	}
}

/**
  * @brief  ��ȡ��Ƭ�����浽�ڲ�FLASH��
  * @param  
  * @retval 
  */
void Upg_ReadFrag(void)
{
	u16 packSize = 0;
	if (g_upgInfo.firmIndex < g_upgInfo.firmSize) {
		if (g_upgInfo.firmIndex == (g_upgInfo.firmSize & ~(UPGRADE_FRAG_SIZE - 1))) {
			packSize = (g_upgInfo.firmSize & (UPGRADE_FRAG_SIZE - 1));
		} else {
			packSize = UPGRADE_FRAG_SIZE;
		}

		// ���ⲿFlash��ȡ��Ƭ
		W25QXX_Read(g_upgFragBuf, UPGRADE_BASE_ADDR + g_upgInfo.firmIndex, packSize);
		
		// ���浽�ڲ�Flash
		STMFLASH_Write_Nocheck(APP_BASE_ADDR + g_upgInfo.firmIndex, (u32 *)g_upgFragBuf, packSize);
		g_upgInfo.firmIndex += packSize;
	}
}

/**
  * @brief  ������Ƭ
  * @param  
  * @retval 
  */
void Upg_SaveFrag(u8 *pBuf)
{
	u16 packSize = 0;
	if (g_upgInfo.firmIndex < g_upgInfo.firmSize) {
		if ((g_upgInfo.firmIndex  & (UPGRADE_FRAG_SIZE - 1)) == (g_upgInfo.firmSize & (UPGRADE_FRAG_SIZE - 1))) {
			packSize = (g_upgInfo.firmSize & (UPGRADE_FRAG_SIZE - 1));
		} else {
			packSize = UPGRADE_FRAG_SIZE;
		}

		// ����̼���Ƭ���ⲿFlash
		W25QXX_Write(pBuf, UPGRADE_BASE_ADDR + g_upgInfo.firmIndex, packSize);
		g_upgInfo.firmIndex += packSize;
	}

	if (g_upgInfo.firmIndex >= g_upgInfo.firmSize) {
		// ���ø��±�־
		g_upgInfo.upgFlag = true;
	}
}

