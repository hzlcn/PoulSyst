#ifndef __UPGRADE_H__
#define __UPGRADE_H__

/* Includes ------------------------------------------------------------------*/

#include "common.h"
#include "stmflash.h"

/* Private typedef -----------------------------------------------------------*/

typedef struct {
	u8   paraFlag;  // 参数标志
	bool upgFlag;   // 更新标志
	u8   firmVer;   // 固件版本
	u32  firmIndex; // 固件地址指针
	u32  firmSize;  // 固件大小
	u16  firmCrc;   // 固件校验CRC
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
  * @brief  初始化固件更新
  */
void        Upg_Init(void);

/**
  * @brief  处理固件更新(Boot)
  */
void        Upg_Process(void);

/**
  * @brief  获取固件参数指针
  */
UpgInfo_t * Upg_GetInfo(void);

/**
  * @brief  开始固件更新
  */
void        Upg_Start(u8 firmVer, u32 firmSize, u16 firmCrc);

/**
  * @brief  读取碎片并保存到内部FLASH中
  */
void        Upg_ReadFrag(void);

/**
  * @brief  保存碎片
  */
void        Upg_SaveFrag(u8 *pBuf);

#endif /* __UPGRADE_H__ */

