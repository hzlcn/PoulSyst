/* Private includes ----------------------------------------------------------*/
#include "modbus.h"
#include "serial.h"
#include "crc.h"
#include "apps.h"
#include "drivers.h"
#include "param.h"
#include "dataStruct.h"

///* Private define ------------------------------------------------------------*/

//#define DBG_TRACE                                   0

//#if DBG_TRACE == 1
//    #include <stdio.h>
//    /*!
//     * Works in the same way as the printf function does.
//     */
//    #define DBG( ... )                               \
//        do                                           \
//        {                                            \
//            printf( __VA_ARGS__ );                   \
//        }while( 0 )
//#else
//    #define DBG( fmt, ... )
//#endif

/* Private variables ---------------------------------------------------------*/

extern ModbusInfo_t g_modbusInfo;
u8 g_RTUTxBuff[1024];
u8 g_RTURxBuff[1024];

/**
  * @brief  修改寄存器值
  * @param  
  * @retval 1	修改成功
  *         0   未找到寄存器
  */
static int RTU_SetReg(u16 addr, u16 data);

/**
  * @brief  解析功能码为01的应答数据
  * @param  
  * @retval 
  */
static int RTU_01_Parse(u8 *pData, u16 len, u16 addr);

/**
  * @brief  解析功能码为03的应答数据
  * @param  
  * @retval 
  */
static int RTU_03_Parse(u8 *pData, u16 len, u16 addr);

/**
  * @brief  解析功能码为05的应答数据
  * @param  
  * @retval 
  */
static int RTU_05_Parse(u8 *pData, u16 len, u16 addr);

/**
  * @brief  解析功能码为06的应答数据
  * @param  
  * @retval 
  */
static int RTU_06_Parse(u8 *pData, u16 len, u16 addr);

static int RTU_Parse(u8 *pRxBuf, u16 rxLen, u8 devCode, u8 funCode, u16 addr, u16 data);

static int RTU_SendCmd(u8 devCode, u8 funCode, u16 addr, u16 data);

int Modbus_HeartBeat(void)
{
	// 锁状态
	if (RTU_SendCmd(0x01, MODBUS_FUN_01, MODBUS_ADDR_LOCK_STATUS, 0x0001) != 1) {
		return -1;
	}
	
	// 有效标签数
	if (RTU_SendCmd(0x01, MODBUS_FUN_03, MODBUS_ADDR_VALIB_LABEL, 0x0001) != 1) {
		return -2;
	}

	// 无效标签数
	if (RTU_SendCmd(0x01, MODBUS_FUN_03, MODBUS_ADDR_INVAL_LABEL, 0x0001) != 1) {
		return -3;
	}
	return 0;
}

int Modbus_LabelRun(void)
{
	// 贴标
	if (RTU_SendCmd(0x01, MODBUS_FUN_05, MODBUS_ADDR_TRIGGER_LAB, 0xFF00) != 1) {
		return -1;
	}
	return 0;
}

int Modbus_OpenLock(void)
{
	// 开锁
	if (RTU_SendCmd(0x01, MODBUS_FUN_05, MODBUS_ADDR_LOCK_OPERATE, 0xFF00) != 1) {
		return -1;
	}
	return 0;
}

/**
  * @brief  修改寄存器值
  * @param  
  * @retval 1	修改成功
  *         0   未找到寄存器
  */
static int RTU_SetReg(u16 addr, u16 data)
{
	int ret = 0;
	// 把读取到的数据写入寄存器
	for (int i = 0; i < MODBUS_MAX_REGISTER_NUM; i++) {
		if (g_modbusInfo.regList[i].addr == addr) {
			g_modbusInfo.regList[i].data = data;
			ret = 1;
			break;
		}
	}
	return ret;
}

/**
  * @brief  解析功能码为01的应答数据
  * @param  
  * @retval 1	解析成功
  *         0   未找到寄存器
  *         -1	长度错误
  *         -2	数据长度错误
  */
static int RTU_01_Parse(u8 *pData, u16 len, u16 addr)
{
	int i;
	
	// 检查长度
	if (len < 2) {
		return -1;
	}
	
	// 检查数据长度
	if (pData[0] != 1) {
		return -2;
	}
	
	// 把读取到的数据写入寄存器
	return RTU_SetReg(addr, pData[1]);
}

/**
  * @brief  解析功能码为03的应答数据
  * @param  
  * @retval 1	解析成功
  *         0   未找到寄存器
  *         -1	长度错误
  *         -2	数据长度错误
  */
static int RTU_03_Parse(u8 *pData, u16 len, u16 addr)
{
	int i;
	
	// 检查长度
	if (len < 3) {
		return -1;
	}
	
	// 检查数据长度
	if (pData[0] != 2) {
		return -2;
	}
	
	// 把读取到的数据写入寄存器
	return RTU_SetReg(addr, (pData[1] << 8) | pData[2]);
}

/**
  * @brief  解析功能码为05的应答数据
  * @param  
  * @retval 1	解析成功
  *         0   未找到寄存器
  *         -1	长度错误
  *         -2	寄存器地址错误
  */
static int RTU_05_Parse(u8 *pData, u16 len, u16 addr)
{
	int i = 0, ret = 0;
	uint16_t startAddr = 0, data = 0;
	
	// 检查长度
	if (len < 4) {
		return -1;
	}
	
	// 检查寄存器地址
	startAddr = ((uint16_t)pData[0] << 8) | pData[1];
	if (startAddr != addr) {
		return -2;
	}
	
	// 检查设备是否连接
	if (g_modbusInfo.devJoin == false && addr == MODBUS_ADDR_MACHINE_RESET) {
		g_modbusInfo.devJoin = true;
	}
	
	// 把读取到的数据写入寄存器
	data = ((uint16_t)pData[2] << 8) | pData[3];
	return RTU_SetReg(addr, data);
}

/**
  * @brief  解析功能码为06的应答数据
  * @param  
  * @retval 1	解析成功
  *         0   未找到寄存器
  *         -1	长度错误
  *         -2	寄存器地址错误
  */
static int RTU_06_Parse(u8 *pData, u16 len, u16 addr)
{
	int i = 0, ret = 0;
	uint16_t startAddr = 0, data = 0;
	
	// 检查长度
	if (len < 4) {
		return -1;
	}
	
	// 检查寄存器地址
	startAddr = ((uint16_t)pData[0] << 8) | pData[1];
	if (startAddr != addr) {
		return -2;
	}
	
	// 把读取到的数据写入寄存器
	data = ((uint16_t)pData[2] << 8) | pData[3];
	return RTU_SetReg(addr, data);
}

/**
  * @brief  解析应答数据
  * @param  
  * @retval 1	解析成功
  *         0   未找到寄存器
  *         -1	长度错误
  *         -2	数据长度错误/寄存器地址错误
  *         -3	
  *         -4	设备码错误
  *         -5	功能码错误
  *         -6	CRC错误
  */
static int RTU_Parse(u8 *pRxBuf, u16 rxLen, u8 devCode, u8 funCode, u16 addr, u16 data)
{
	int ret = 0;
	u16 calcCrc = 0, recvCrc = 0;
	
	// 检查设备码
	if (pRxBuf[0] != devCode) {
		return -4;
	}
	
	// 检查功能码
	if (pRxBuf[1] != funCode) {
		return -5;
	}
	
	// 检查CRC
	calcCrc = CalCrc16(pRxBuf, rxLen - 2, 0xFFFF);
	recvCrc = ((uint16_t)pRxBuf[rxLen - 1] << 8) | pRxBuf[rxLen - 2];
	if (calcCrc != recvCrc) {
		return -6;
	}
	
	switch (funCode) {
		case 1:
			ret = RTU_01_Parse(pRxBuf + 2, rxLen - 4, addr);
			break;
		
		case 3:
			ret = RTU_03_Parse(pRxBuf + 2, rxLen - 4, addr);
			break;
		
		case 5:
			ret = RTU_05_Parse(pRxBuf + 2, rxLen - 4, addr);
			break;
		
		case 6:
			ret = RTU_06_Parse(pRxBuf + 2, rxLen - 4, addr);
			break;
		
		default:
			break;
	}
	return ret;
}

/**
  * @brief  发送指令
  * @param  
  * @retval 1	解析成功
  *         0   未找到寄存器
  *         -1	长度错误
  *         -2	数据长度错误/寄存器地址错误
  *         -3	
  *         -4	设备码错误
  *         -5	功能码错误
  *         -6	CRC错误
  *         -7	未接收到数据
  */
static int RTU_SendCmd(u8 devCode, u8 funCode, u16 addr, u16 data)
{
	u8 *pTxBuf = g_RTUTxBuff;
	u16 txLen = 0, calcCrc = 0;
	u16 rxLen = 0, timeout = 50;

	pTxBuf[txLen++] = devCode;
	pTxBuf[txLen++] = funCode;
	pTxBuf[txLen++] = addr >> 8;
	pTxBuf[txLen++] = addr;
	pTxBuf[txLen++] = data >> 8;
	pTxBuf[txLen++] = data;
	
	calcCrc = CalCrc16(pTxBuf, 6, 0xFFFF);
	pTxBuf[txLen++] = (calcCrc & 0x00FF) >> 0;
	pTxBuf[txLen++] = (calcCrc & 0xFF00) >> 8;
	
	Uart7_Send(pTxBuf, txLen);
	
	while (timeout--) {
		rxLen = Uart_GetData(&huart7, g_RTURxBuff);
		if (rxLen > 0) {
			return RTU_Parse(g_RTURxBuff, rxLen, devCode, funCode, addr, data);
		}
		memset(g_RTURxBuff, 0, UART4_BUFFSIZE);
		osDelay(1);
	}
	return -7;
}

