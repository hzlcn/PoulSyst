#include "store.h"
#include "usermsg.h"
#include "poultry.h"
#include "w25qxx.h"
#include "param.h"
#include "communicate.h"
#include "drivers.h"
#include "modbus.h"     // 获取标签数

/* Private define ------------------------------------------------------------*/

#define DBG_TRACE                                   1

#if DBG_TRACE == 1
    #include <stdio.h>
    /*!
     * Works in the same way as the printf function does.
     */
    #define DBG( ... )                               \
        do                                           \
        {                                            \
            printf( __VA_ARGS__ );                   \
        }while( 0 )
#else
    #define DBG( fmt, ... )
#endif

/* Private macro -------------------------------------------------------------*/

/**
  * 数据存储地址
  */
#define STORE_CONFIG_ADDR  	    0x01FFB000
#define STORE_MERCBASE_ADDR     0x01FFC000
#define STORE_POULBASE_ADDR     0x01FFD000

/**
  * 配置存储标志
  */
#define STORE_CONFIG_FLAG       0x23
#define STORE_MERCBASE_FLAG     0x23
#define STORE_POULBASE_FLAG     0x23
#define STORE_RECORD_FLAG       0x23

/* External variables --------------------------------------------------------*/
/**
  * 配置参数缓冲区
  */
ConfInfo_t g_config = { 0 };

/**
  * 配置参数缓冲区
  */
ConfInfo_t g_defConfig = {
	.keepFlag = STORE_CONFIG_FLAG,
	.devAddr = { 3, 3, 3, 3, 3, 3 },
	.version = { 0, 0 },
};

/**
  * 交易信息记录
  */
static RecordInfo_t g_record_Info = { 0 };

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  获取配置参数指针
  * @param  
  * @retval 
  */
ConfInfo_t *Store_GetConfig(void)
{
	return &g_config;
}

/**
  * @brief  获取交易参数地址指针
  * @param  
  * @retval 
  */
RecordInfo_t *Store_GetRecord(void)
{
	return &g_record_Info;
}

/**
  * @brief  读取配置参数
  * @param  
  * @retval 
  */
void Store_Init(void)
{	
	MercBase_t *mercBase = User_GetInfo()->mercBase;
	PoulBase_t *poulBase = Poul_GetInfo()->base[0];

	// 加载配置参数
	W25QXX_Read((uint8_t *)&g_config, STORE_CONFIG_ADDR, sizeof(ConfInfo_t));
	if (g_config.keepFlag != STORE_CONFIG_FLAG) {
		memcpy(&g_config, &g_defConfig, sizeof(ConfInfo_t));
		Store_SaveConfig();
	}
	
	// 加载交易信息数量
	W25QXX_Read((uint8_t *)&g_record_Info, RECORD_NUMBER_ADDR, sizeof(RecordNumber_t));
	if (g_record_Info.num.keepFlag != STORE_RECORD_FLAG) {
		g_record_Info.num.keepFlag = STORE_RECORD_FLAG;
		g_record_Info.num.blockNum = 0;
		g_record_Info.num.syncNum = 0;
		Store_SaveRecordNum();
	}
	
	// 加载用户信息库
	W25QXX_Read((uint8_t *)mercBase, STORE_MERCBASE_ADDR, sizeof(MercBase_t));
	if (mercBase->keepFlag != STORE_MERCBASE_FLAG) {
		// 复位并存储用户信息
		User_InitMercBase();
		Store_SaveMercBase();
	}
	
	// 加载食品信息库
	W25QXX_Read((uint8_t *)poulBase, STORE_POULBASE_ADDR, sizeof(PoulBase_t));
	if (poulBase->keepFlag != STORE_POULBASE_FLAG) {
		// 复位并存储禽类信息
		Poul_DefBase();
		Store_SavePoulBase();
	}
	
	DBG("device address: %02x %02x %02x %02x %02x %02x\r\n", g_config.devAddr[0], g_config.devAddr[1], g_config.devAddr[2], 
	                                                         g_config.devAddr[3], g_config.devAddr[4], g_config.devAddr[5]);
}

/**
  * @brief  检查信息上报
  * @param  
  * @retval 
  */
void Store_Process(void)
{
	if (g_record_Info.num.blockNum > g_record_Info.num.syncNum) {
		// 上报交易信息
		CMNC_AddCmd(UL_FUNC_7);
	}	
}

/**
  * @brief  保存配置参数
  * @param  
  * @retval 
  */
void Store_SaveConfig(void)
{
	g_config.keepFlag = STORE_CONFIG_FLAG;

	W25QXX_Erase_Sector(STORE_CONFIG_ADDR >> 12);
	W25QXX_Write_NoCheck((uint8_t *)&g_config, STORE_CONFIG_ADDR, sizeof(ConfInfo_t));
}

/**
  * @brief  存储店主库信息
  * @param  
  * @retval 
  */
void Store_SaveMercBase(void)
{
	MercBase_t *mercBase = User_GetInfo()->mercBase;
	mercBase->keepFlag = STORE_MERCBASE_FLAG;
	
	W25QXX_Erase_Sector(STORE_MERCBASE_ADDR >> 12);
	W25QXX_Write_NoCheck((uint8_t *)mercBase, STORE_MERCBASE_ADDR, sizeof(MercBase_t));
}

/**
  * @brief  存储食品库信息
  * @param  
  * @retval 
  */
void Store_SavePoulBase(void)
{
	PoulBase_t *poulBase = Poul_GetInfo()->base[0];
	poulBase->keepFlag = STORE_POULBASE_FLAG;

	W25QXX_Erase_Sector(STORE_POULBASE_ADDR >> 12);
	W25QXX_Write_NoCheck((uint8_t *)poulBase, STORE_POULBASE_ADDR, sizeof(PoulBase_t));
}

/**
  * @brief  获取交易信息
  * @param  
  * @retval 
  */
void Store_SaveBlock(void)
{
	ModbusInfo_t *modbusInfo = Modbus_GetParam();
	PoulInfo_t *poulInfo = Poul_GetInfo();
	Customer_t *customer = &User_GetInfo()->cust;
	RTCTime_t *rtcTime = RTCTime_GetTime();
	uint16_t remainnumb = LABEL_TOTAL_NUMBER - modbusInfo->regList[MODBUS_REG_VALIB_LABEL].data - modbusInfo->regList[MODBUS_REG_INVAL_LABEL].data;
	uint32_t writeAddr = g_record_Info.num.blockNum * sizeof(RecordBlock_t);
	
	if (poulInfo->code.isReady == false) {	
		DBG("There is no poultry qrcode.\r\n");
		return ;
	}
	
	if (customer->perm == USER_PERM_NULL) {
		DBG("There is no customer qrcode.\r\n");
		return ;
	}

	// 添加当前禽类类型
	memcpy(g_record_Info.block.poulKind, poulInfo->pCurKind->kindNum, 6);
	
	// 添加食品信息
	memcpy(g_record_Info.block.poulCode, poulInfo->code.qrcode, N_POULTRY_QRCODE_BUFFER_SIZE);
	Poul_ClrCode();

	// 添加客户信息
	memcpy(g_record_Info.block.custCode, customer->code, USER_CODE_SIZE);
	
	// 添加交易时间
	memcpy(g_record_Info.block.rtcTime, rtcTime->value, SYSTEM_TIME_SIZE);
	g_record_Info.block.rtcTime[5] = ((g_record_Info.block.rtcTime[5] & 0xF0) >> 4) * 10 + (g_record_Info.block.rtcTime[5] & 0x0F);/* 秒位为16进制 */
	
	// 添加剩余标签数
	g_record_Info.block.remainNum[0] = remainnumb >> 8;
	g_record_Info.block.remainNum[1] = remainnumb;
	
	g_record_Info.num.blockNum++;
	W25QXX_Write((uint8_t *)&g_record_Info.block, writeAddr, sizeof(RecordBlock_t));
		
#if (DBG_TRACE == 1)
	DBG("################# product param ################\r\n");
	DBG("product QRcode : ");
	PrintCharBuffer((char *)g_record_Info.block.poulCode, N_POULTRY_QRCODE_BUFFER_SIZE);
	DBG("poultry type   : %02x %02x %02x %02x %02x %02x\r\n", g_record_Info.block.poulKind[0], g_record_Info.block.poulKind[1], 
		g_record_Info.block.poulKind[2], g_record_Info.block.poulKind[3], g_record_Info.block.poulKind[4], g_record_Info.block.poulKind[5]);
	DBG("customer QRcode: ");
	PrintCharBuffer((char *)g_record_Info.block.custCode, USER_CODE_SIZE);
	DBG("system time    : %04d-%02d-%02d %02d:%02d:%02d\r\n", g_record_Info.block.rtcTime[0] + 2000, g_record_Info.block.rtcTime[1], 
		g_record_Info.block.rtcTime[2], g_record_Info.block.rtcTime[3], g_record_Info.block.rtcTime[4], g_record_Info.block.rtcTime[5]);
	DBG("remain label   : %04d\r\n", ((g_record_Info.block.remainNum[0]) << 8 | g_record_Info.block.remainNum[1]));
	DBG("################################################\r\n");
#endif
}

/**
  * @brief  获取交易信息(最多10条)
  * @param  
  * @retval 数据长度
  */
uint8_t Store_ReadBlock(uint8_t *pBuffer)
{
	uint32_t readAddr = 0;
	uint16_t byteNum  = 0;
	
	if (g_record_Info.num.blockNum > g_record_Info.num.syncNum) {
		g_record_Info.upNum = MIN(g_record_Info.num.blockNum - g_record_Info.num.syncNum, 10);
//		g_record_Info.upNum = 1;

		// 读取多条交易信息
		readAddr = g_record_Info.num.syncNum * sizeof(RecordBlock_t); // 读取地址
		byteNum  = g_record_Info.upNum * sizeof(RecordBlock_t);      // 读取长度
		W25QXX_Read(pBuffer, readAddr, byteNum);
	}
	
	return g_record_Info.upNum;
}

/**
  * @brief  保存信息数量
  * @param  
  * @retval 
  */
void Store_SaveRecordNum(void)
{
	W25QXX_Erase_Sector(RECORD_NUMBER_ADDR >> 12);
	W25QXX_Write_NoCheck((uint8_t *)&g_record_Info.num, RECORD_NUMBER_ADDR, sizeof(RecordNumber_t));
}

