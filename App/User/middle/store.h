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

// W25Q256最大地址: 0x02000000
#define RECORD_NUMBER_ADDR     0x01FFE000
#define RECORD_BLOCK_MAX_ADDR  0x01D00000

/* Private typedef -----------------------------------------------------------*/

/**
  * 设备地址类型
  */
typedef uint8_t DevAddr_t[6];

/**
  * 软硬件版本类型
  */
typedef struct {
	uint8_t hard;
	uint8_t soft;
}Version_t;

/**
  * 配置参数
  */
typedef struct {
	uint8_t   keepFlag;
	DevAddr_t devAddr;
	Version_t version;
}ConfInfo_t;

/**
  * 交易信息结构体
  */
typedef struct
{
    uint8_t poulCode[N_POULTRY_QRCODE_BUFFER_SIZE];  // 商品二维码
    uint8_t poulKind[N_POULTRY_KIND_NUMS_SIZE];      // 商品种类
    uint8_t custCode[USER_CODE_SIZE];                // 客户二维码
    uint8_t rtcTime[SYSTEM_TIME_SIZE];               // 交易时间
    uint8_t remainNum[REMAIN_LABEL_NUM_SIZE];        // 剩余条码数
}RecordBlock_t;

/**
  * 交易记录结构体
  */
typedef struct {
	u8 keepFlag;
	uint32_t blockNum;                               // 信息总数
	uint32_t syncNum;                                // 已上报的信息数量
}RecordNumber_t;

/**
  * 交易信息
  */
typedef struct {
	RecordNumber_t num;
	RecordBlock_t block;
	uint8_t upNum; // 正在上报的交易记录数量(用于上报成功后增加上报后的交易信息数)
}RecordInfo_t;


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  获取配置参数指针
  */
ConfInfo_t *Store_GetConfig(void);

/**
  * @brief  获取交易参数地址指针
  */
RecordInfo_t *Store_GetRecord(void);

/**
  * @brief  读取配置参数
  */
void        Store_Init(void);

/**
  * @brief  检查信息上报
  */
void        Store_Process(void);

/**
  * @brief  保存配置参数
  */
void        Store_SaveConfig(void);

/**
  * @brief  存储店主库信息
  */
void        Store_SaveMercBase(void);

/**
  * @brief  存储食品库信息
  */
void        Store_SavePoulBase(void);

/**
  * @brief  获取交易信息
  */
void        Store_SaveBlock(void);

/**
  * @brief  获取交易记录(最多10条)
  */
uint8_t     Store_ReadBlock(uint8_t *pBuffer);

/**
  * @brief  保存信息数量
  */
void        Store_SaveRecordNum(void);

#endif /* __STORE_H__ */
