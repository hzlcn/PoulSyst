#include "upgrade.h"
#include "w25qxx.h"
#include "stmflash.h"
#include "iap.h"


/* Private variables ---------------------------------------------------------*/

/**
  * 固件参数
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
  * 碎片缓冲区
  */
static u8 g_upgFragBuf[UPGRADE_FRAG_SIZE] = { 0 };

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  初始化固件更新
  * @param  
  * @retval 
  */
void Upg_Init(void)
{
	W25QXX_Init();
	
	// 读取更新参数
	W25QXX_Read((u8 *)&g_upgInfo, UPGRADE_PARA_ADDR, sizeof(UpgInfo_t));
	if (g_upgInfo.paraFlag != UPGRADE_PARA_FLAG) {
		memset(&g_upgInfo, 0, sizeof(UpgInfo_t));
	}

	if (g_upgInfo.upgFlag == true) {
		STMFLASH_Erase(ADDR_FLASH_SECTOR_1);
		FLASH_WaitForLastOperation(FLASH_WAITETIME);            //等待上次操作完成
		STMFLASH_Erase(ADDR_FLASH_SECTOR_2);
		FLASH_WaitForLastOperation(FLASH_WAITETIME);            //等待上次操作完成
		STMFLASH_Erase(ADDR_FLASH_SECTOR_3);
		FLASH_WaitForLastOperation(FLASH_WAITETIME);            //等待上次操作完成
		STMFLASH_Erase(ADDR_FLASH_SECTOR_4);
		FLASH_WaitForLastOperation(FLASH_WAITETIME);            //等待上次操作完成
		STMFLASH_Erase(ADDR_FLASH_SECTOR_5);
		FLASH_WaitForLastOperation(FLASH_WAITETIME);            //等待上次操作完成
	}
}

/**
  * @brief  处理固件更新(Boot)
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
  * @brief  获取固件参数指针
  * @param  
  * @retval 
  */
UpgInfo_t *Upg_GetInfo(void)
{
	return &g_upgInfo;
}

/**
  * @brief  开始固件更新
  * @param  
  * @retval 
  */
void Upg_Start(u8 firmVer, u32 firmSize, u16 firmCrc)
{
	u8 i;
	
	// 设置参数
	g_upgInfo.firmVer = firmVer;
	g_upgInfo.firmSize = firmSize;
	g_upgInfo.firmCrc = firmCrc;
	
	// 清空外部Flash的存储扇区
	for (i = 0; i < (firmSize >> 12); i++) {
		W25QXX_Erase_Sector(UPGRADE_BASE_ADDR + (i << 12));
	}
}

/**
  * @brief  读取碎片并保存到内部FLASH中
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

		// 从外部Flash读取碎片
		W25QXX_Read(g_upgFragBuf, UPGRADE_BASE_ADDR + g_upgInfo.firmIndex, packSize);
		
		// 保存到内部Flash
		STMFLASH_Write_Nocheck(APP_BASE_ADDR + g_upgInfo.firmIndex, (u32 *)g_upgFragBuf, packSize);
		g_upgInfo.firmIndex += packSize;
	}
}

/**
  * @brief  保存碎片
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

		// 保存固件碎片到外部Flash
		W25QXX_Write(pBuf, UPGRADE_BASE_ADDR + g_upgInfo.firmIndex, packSize);
		g_upgInfo.firmIndex += packSize;
	}

	if (g_upgInfo.firmIndex >= g_upgInfo.firmSize) {
		// 设置更新标志
		g_upgInfo.upgFlag = true;
	}
}

