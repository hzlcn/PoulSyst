#ifndef __MODBUS_H__
#define __MODBUS_H__

#include "common.h"

/*******************************************/
//0x00----��е��λ---д��
//0x01----�ֶ������ֽ---д��
//0x02----�����ֽ�󷵻�---д��
//0x03----��������----д��
//0x04----��ǩ����----��ȡ
//0x09----������----��ȡ
//0x012C---��Ч��ǩ---��ȡ
//0x012E---��Ч��ǩ---��ȡ
/*******************************************/


/* Private macro -------------------------------------------------------------*/

#define MODBUS_LIST_MAX_NUMBER          10          // ָ���б�����
#define MODBUS_ACK_TIMEOUT_TIME         2000        // ��λ���룬����2��
#define MODBUS_TRIGGERRESET_TIME        5000        // ��λ���룬����5��
#define MODBUS_MAX_RETRY_NUMBER         5           // ����ط�����
#define MODBUS_MAX_REGISTER_NUM         12          // �Ĵ�������

/**
  * Modbus��ַ
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
  * Modbus������
  */
#define MODBUS_FUN_01					0x01
#define MODBUS_FUN_03					0x03
#define MODBUS_FUN_05					0x05
#define MODBUS_FUN_06                   0x06

/* Private typedef -----------------------------------------------------------*/

/**
  * ָ������
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
  * �Ĵ�������
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
  * ָ������ṹ��
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
  * ָ���б�����ṹ��
  */
typedef struct {
	ModbusCmdType_t list[MODBUS_LIST_MAX_NUMBER];    // ָ������
	uint8_t num;                        // ָ������
}ModbusList_t;

/**
  * �Ĵ�����Ϣ
  */
typedef struct {
	uint16_t addr;
	uint16_t data;
}ModbusReg_t;

/**
  * Modbusָ����Ϣ�ṹ��
  */
typedef struct {
	ModbusList_t    cmdList;	// ָ���б�
	ModbusData_t    cmdData;	// ָ������
	ModbusCmdType_t cmdType;	// ָ������
	uint8_t         reTxNum;	// �ط�����
	bool            devJoin;	// ����״̬
	ModbusReg_t     regList[MODBUS_MAX_REGISTER_NUM];
}ModbusInfo_t;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Modbus-RTUͨѶ��ʼ��
  */
void        Modbus_Init(void);

/**
  * @brief  Modbus-RTUЭ�鴦��
  */
void        Modbus_Process(void);

/**
  * @brief  ��ȡModbusָ����Ϣ��ַָ��
  */
ModbusInfo_t *Modbus_GetParam(void);

/**
  * @brief  ���Modbus-RTUָ�ָ�����
  */
void        Modbus_AddCmd(ModbusCmdType_t cmd);

#endif // __MODBUS_H__
