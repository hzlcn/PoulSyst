#ifndef __MODBUS_H__
#define __MODBUS_H__

#include "common.h"

/*******************************************/
//0x00----机械复位---写入
//0x01----手动清除废纸---写入
//0x02----清除废纸后返回---写入
//0x03----启动触发----写入
//0x04----标签出错----读取
//0x09----运行中----读取
//0x012C---有效标签---读取
//0x012E---无效标签---读取
/*******************************************/


/* Private macro -------------------------------------------------------------*/

#define MODBUS_LIST_MAX_NUMBER          10          // 指令列表容量
#define MODBUS_ACK_TIMEOUT_TIME         2000        // 单位是秒，代表2秒
#define MODBUS_TRIGGERRESET_TIME        5000        // 单位是秒，代表5秒
#define MODBUS_MAX_RETRY_NUMBER         5           // 最大重发次数
#define MODBUS_MAX_REGISTER_NUM         12          // 寄存器数量

/**
  * Modbus地址
  */
#define MODBUS_ADDR_MACHINE_RESET       0x0000
#define MODBUS_ADDR_CHEAR_LABEL         0x0001
#define MODBUS_ADDR_CLEAR_RESET         0x0002
#define MODBUS_ADDR_TRIGGER_LAB         0x0003
#define MODBUS_ADDR_LABEL_ERROR         0x0004
#define MODBUS_ADDR_PRESS_ERROR         0x0005
#define MODBUS_ADDR_CLAMP_ERROR         0x0006
#define MODBUS_ADDR_LOCK_STATUS         0x0007
#define MODBUS_ADDR_WORK_STATUS         0x0009
#define MODBUS_ADDR_LOCK_OPERATE        0x330C
#define MODBUS_ADDR_VALIB_LABEL         0x012C
#define MODBUS_ADDR_INVAL_LABEL         0x012E

/**
  * Modbus功能码
  */
#define MODBUS_FUN_01					0x01
#define MODBUS_FUN_03					0x03
#define MODBUS_FUN_05					0x05
#define MODBUS_FUN_06                   0x06

/* Private typedef -----------------------------------------------------------*/

/**
  * 指令类型
  */
typedef enum {
	MODBUS_CMD_NULL,
	MODBUS_CMD_TRIGGER_LABEL,
	MODBUS_CMD_TRIGGER_RESET,
	MODBUS_CMD_READ_VALIBNUM,
	MODBUS_CMD_READ_INVALNUM,
	MODBUS_CMD_MACHINE_RESET,
	MODBUS_CMD_CLEAR_RESET,
	MODBUS_CMD_UNLOCK_DEVICE,
	MODBUS_CMD_GETLOCKSTATUS,
}ModbusCmdType_t;

/**
  * 寄存器类型
  */
typedef enum {
    MODBUS_REG_MACHINE_RESET = 0,
	MODBUS_REG_CHEAR_LABEL   = 1,
	MODBUS_REG_CLEAR_RESET   = 2,
	MODBUS_REG_TRIGGER_LAB   = 3,
	MODBUS_REG_LABEL_ERROR   = 4,
	MODBUS_REG_PRESS_ERROR   = 5,
	MODBUS_REG_CLAMP_ERROR   = 6,
	MODBUS_REG_LOCK_STATUS   = 7,
	MODBUS_REG_WORK_STATUS   = 8,
	MODBUS_REG_LOCK_OPERATE  = 9,
	MODBUS_REG_VALIB_LABEL   = 10,
	MODBUS_REG_INVAL_LABEL   = 11,
}ModbusRegType_t;

/**
  * 指令参数结构体
  */
typedef struct sElement {
	uint8_t  devCode;
	uint8_t  funCode;
	uint16_t  addr;
	union {
		uint16_t wrData;
		uint16_t readLen;
	}data;
	uint16_t crc;
}ModbusData_t;

/**
  * 指令列表参数结构体
  */
typedef struct {
	ModbusCmdType_t list[MODBUS_LIST_MAX_NUMBER];    // 指令容器
	uint8_t num;                        // 指令数量
}ModbusList_t;

/**
  * 寄存器信息
  */
typedef struct {
	uint16_t addr;
	uint16_t data;
}ModbusReg_t;

/**
  * Modbus指令信息结构体
  */
typedef struct {
	ModbusList_t    cmdList;	// 指令列表
	ModbusData_t    cmdData;	// 指令数据
	ModbusCmdType_t cmdType;	// 指令内容
	uint8_t         reTxNum;	// 重发次数
	bool            devJoin;	// 连接状态
	ModbusReg_t     regList[MODBUS_MAX_REGISTER_NUM];
}ModbusInfo_t;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Modbus-RTU通讯初始化
  */
void        Modbus_Init(void);

/**
  * @brief  Modbus-RTU协议处理
  */
void        Modbus_Process(void);

/**
  * @brief  获取Modbus指令信息地址指针
  */
ModbusInfo_t *Modbus_GetParam(void);

/**
  * @brief  添加Modbus-RTU指令到指令队列
  */
void        Modbus_AddCmd(ModbusCmdType_t cmd);

#endif // __MODBUS_H__
