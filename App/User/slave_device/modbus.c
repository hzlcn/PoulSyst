/* Private includes ----------------------------------------------------------*/
#include "modbus.h"
#include "serial.h"
#include "crc.h"
#include "apps.h"
#include "drivers.h"
#include "param.h"

/* Private define ------------------------------------------------------------*/

#define DBG_TRACE                                   0

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

/* Private variables ---------------------------------------------------------*/

ModbusInfo_t g_modbusInfo = { 
	.devJoin = false,
	.regList = {
		{ .addr = MODBUS_ADDR_MACHINE_RESET, .data = 0, },	// ������λ
		{ .addr = MODBUS_ADDR_CHEAR_LABEL, .data = 0, },	// 
		{ .addr = MODBUS_ADDR_CLEAR_RESET, .data = 0, },	// 
		{ .addr = MODBUS_ADDR_TRIGGER_LAB, .data = 0, },	// ���괥��
		{ .addr = MODBUS_ADDR_LABEL_ERROR, .data = 0, },	// �������
		{ .addr = MODBUS_ADDR_PRESS_ERROR, .data = 0, },	// 
		{ .addr = MODBUS_ADDR_CLAMP_ERROR, .data = 0, },
		{ .addr = MODBUS_ADDR_LOCK_STATUS, .data = 0, },	// ��״̬
		{ .addr = MODBUS_ADDR_WORK_STATUS, .data = 0, },	// ����״̬
		{ .addr = MODBUS_ADDR_LOCK_OPERATE, .data = 0, },	// ������
		{ .addr = MODBUS_ADDR_VALIB_LABEL, .data = 0, },	// ��Ч��ǩ��
		{ .addr = MODBUS_ADDR_INVAL_LABEL, .data = 0, },	// ��Ч��ǩ��
	},
};

/**
  * ���ݻ�����
  */
u8 g_RTUTxBuff[1024];
u8 g_RTURxBuff[1024];

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  �޸ļĴ���ֵ
  * @param  
  * @retval 1	�޸ĳɹ�
  *         0   δ�ҵ��Ĵ���
  */
static int RTU_SetReg(u16 addr, u16 data);

/**
  * @brief  ����������Ϊ01��Ӧ������
  * @param  
  * @retval 
  */
static int RTU_01_Parse(u8 *pData, u16 len, u16 addr);

/**
  * @brief  ����������Ϊ03��Ӧ������
  * @param  
  * @retval 
  */
static int RTU_03_Parse(u8 *pData, u16 len, u16 addr);

/**
  * @brief  ����������Ϊ05��Ӧ������
  * @param  
  * @retval 
  */
static int RTU_05_Parse(u8 *pData, u16 len, u16 addr);

/**
  * @brief  ����������Ϊ06��Ӧ������
  * @param  
  * @retval 
  */
static int RTU_06_Parse(u8 *pData, u16 len, u16 addr);

/**
  * @brief  ����Ӧ������
  * @param  
  * @retval 1	�����ɹ�
  *         0   δ�ҵ��Ĵ���
  *         -1	���ȴ���
  *         -2	���ݳ��ȴ���/�Ĵ�����ַ����
  *         -3	
  *         -4	�豸�����
  *         -5	���������
  *         -6	CRC����
  */
static int RTU_Parse(u8 *pRxBuf, u16 rxLen, u8 devCode, u8 funCode, u16 addr, u16 data);

/**
  * @brief  ����ָ��
  * @param  
  * @retval 1	�����ɹ�
  *         0   δ�ҵ��Ĵ���
  *         -1	���ȴ���
  *         -2	���ݳ��ȴ���/�Ĵ�����ַ����
  *         -3	
  *         -4	�豸�����
  *         -5	���������
  *         -6	CRC����
  *         -7	δ���յ�����
  */
static int RTU_SendCmd(u8 devCode, u8 funCode, u16 addr, u16 data);

/**
  * @brief  Modbus-RTUͨѶ��ʼ��
  * @param  
  * @retval 
  */
void Modbus_Init( void )
{
	// �������ʼ��
	Uart_SetRxEnable(&huart7);

	Modbus_MechanicalReset();
}

/**
  * @brief  ��ȡModbusָ����Ϣ��ַָ��
  * @param  
  * @retval 
  */
ModbusInfo_t *Modbus_GetParam(void)
{
	return &g_modbusInfo;
}

/**
  * @brief  ��е��λ
  * @param  
  * @retval 0	��λ����
  *         -1	��λ��Ӧ��
  */
int Modbus_MechanicalReset(void)
{
	// ��е��λ
	if (RTU_SendCmd(0x01, MODBUS_FUN_05, MODBUS_ADDR_MACHINE_RESET, 0xFF00) != 1) {
		return -1;
	}
	return 0;
}

/**
  * @brief  ������
  * @param  
  * @retval 0	��������
  *         -1	��ѯ��״̬��Ӧ��
  *         -2  ��ѯ��Ч��ǩ����Ӧ��
  *         -3  ��ѯ��Ч��ǩ����Ӧ��
  */
int Modbus_HeartBeat(void)
{
	// ��״̬
	if (RTU_SendCmd(0x01, MODBUS_FUN_01, MODBUS_ADDR_LOCK_STATUS, 0x0001) != 1) {
		return -1;
	}
	
	// ��Ч��ǩ��
	if (RTU_SendCmd(0x01, MODBUS_FUN_03, MODBUS_ADDR_VALIB_LABEL, 0x0001) != 1) {
		return -2;
	}

	// ��Ч��ǩ��
	if (RTU_SendCmd(0x01, MODBUS_FUN_03, MODBUS_ADDR_INVAL_LABEL, 0x0001) != 1) {
		return -3;
	}
	return 0;
}

/**
  * @brief  ����
  * @param  
  * @retval 0	��������
  *         -1  ������Ӧ��
  *         -2  ���긴λ��Ӧ��
  */
int Modbus_LabelRun(void)
{
	// ����
	if (RTU_SendCmd(0x01, MODBUS_FUN_05, MODBUS_ADDR_TRIGGER_LAB, 0xFF00) != 1) {
		return -1;
	}
	
	osDelay(3000);
	
	// ���긴λ
	if (RTU_SendCmd(0x01, MODBUS_FUN_05, MODBUS_ADDR_TRIGGER_LAB, 0x0000) != 1) {
		return -2;
	}
	
	return 0;
}

/**
  * @brief  ����
  * @param  
  * @retval 0	��������
  *         -1  ��Ӧ��
  */
int Modbus_OpenLock(void)
{
	// ����
	if (RTU_SendCmd(0x01, MODBUS_FUN_05, MODBUS_ADDR_LOCK_OPERATE, 0xFF00) != 1) {
		return -1;
	}
	return 0;
}

/**
  * @brief  �޸ļĴ���ֵ
  * @param  
  * @retval 1	�޸ĳɹ�
  *         0   δ�ҵ��Ĵ���
  */
static int RTU_SetReg(u16 addr, u16 data)
{
	int ret = 0;
	// �Ѷ�ȡ��������д��Ĵ���
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
  * @brief  ����������Ϊ01��Ӧ������
  * @param  
  * @retval 1	�����ɹ�
  *         0   δ�ҵ��Ĵ���
  *         -1	���ȴ���
  *         -2	���ݳ��ȴ���
  */
static int RTU_01_Parse(u8 *pData, u16 len, u16 addr)
{
	int i;
	
	// ��鳤��
	if (len < 2) {
		return -1;
	}
	
	// ������ݳ���
	if (pData[0] != 1) {
		return -2;
	}
	
	// �Ѷ�ȡ��������д��Ĵ���
	return RTU_SetReg(addr, pData[1]);
}

/**
  * @brief  ����������Ϊ03��Ӧ������
  * @param  
  * @retval 1	�����ɹ�
  *         0   δ�ҵ��Ĵ���
  *         -1	���ȴ���
  *         -2	���ݳ��ȴ���
  */
static int RTU_03_Parse(u8 *pData, u16 len, u16 addr)
{
	int i;
	
	// ��鳤��
	if (len < 3) {
		return -1;
	}
	
	// ������ݳ���
	if (pData[0] != 2) {
		return -2;
	}
	
	// �Ѷ�ȡ��������д��Ĵ���
	return RTU_SetReg(addr, (pData[1] << 8) | pData[2]);
}

/**
  * @brief  ����������Ϊ05��Ӧ������
  * @param  
  * @retval 1	�����ɹ�
  *         0   δ�ҵ��Ĵ���
  *         -1	���ȴ���
  *         -2	�Ĵ�����ַ����
  */
static int RTU_05_Parse(u8 *pData, u16 len, u16 addr)
{
	int i = 0, ret = 0;
	uint16_t startAddr = 0, data = 0;
	
	// ��鳤��
	if (len < 4) {
		return -1;
	}
	
	// ���Ĵ�����ַ
	startAddr = ((uint16_t)pData[0] << 8) | pData[1];
	if (startAddr != addr) {
		return -2;
	}
	
	// ����豸�Ƿ�����
	if (g_modbusInfo.devJoin == false && addr == MODBUS_ADDR_MACHINE_RESET) {
		g_modbusInfo.devJoin = true;
	}
	
	// �Ѷ�ȡ��������д��Ĵ���
	data = ((uint16_t)pData[2] << 8) | pData[3];
	return RTU_SetReg(addr, data);
}

/**
  * @brief  ����������Ϊ06��Ӧ������
  * @param  
  * @retval 1	�����ɹ�
  *         0   δ�ҵ��Ĵ���
  *         -1	���ȴ���
  *         -2	�Ĵ�����ַ����
  */
static int RTU_06_Parse(u8 *pData, u16 len, u16 addr)
{
	int i = 0, ret = 0;
	uint16_t startAddr = 0, data = 0;
	
	// ��鳤��
	if (len < 4) {
		return -1;
	}
	
	// ���Ĵ�����ַ
	startAddr = ((uint16_t)pData[0] << 8) | pData[1];
	if (startAddr != addr) {
		return -2;
	}
	
	// �Ѷ�ȡ��������д��Ĵ���
	data = ((uint16_t)pData[2] << 8) | pData[3];
	return RTU_SetReg(addr, data);
}

/**
  * @brief  ����Ӧ������
  * @param  
  * @retval 1	�����ɹ�
  *         0   δ�ҵ��Ĵ���
  *         -1	���ȴ���
  *         -2	���ݳ��ȴ���/�Ĵ�����ַ����
  *         -3	
  *         -4	�豸�����
  *         -5	���������
  *         -6	CRC����
  */
static int RTU_Parse(u8 *pRxBuf, u16 rxLen, u8 devCode, u8 funCode, u16 addr, u16 data)
{
	int ret = 0;
	u16 calcCrc = 0, recvCrc = 0;
	
	// ����豸��
	if (pRxBuf[0] != devCode) {
		return -4;
	}
	
	// ��鹦����
	if (pRxBuf[1] != funCode) {
		return -5;
	}
	
	// ���CRC
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
  * @brief  ����ָ��
  * @param  
  * @retval 1	�����ɹ�
  *         0   δ�ҵ��Ĵ���
  *         -1	���ȴ���
  *         -2	���ݳ��ȴ���/�Ĵ�����ַ����
  *         -3	
  *         -4	�豸�����
  *         -5	���������
  *         -6	CRC����
  *         -7	δ���յ�����
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
	
	// ��ս�������
	Uart_GetData(&huart7, g_RTURxBuff);
	
	// ����ָ��
	Uart7_Send(pTxBuf, txLen);
	
	// �ȴ���������
	while (timeout--) {
		rxLen = Uart_GetData(&huart7, g_RTURxBuff);
		if (rxLen > 0) {
			return RTU_Parse(g_RTURxBuff, rxLen, devCode, funCode, addr, data);
		}
		osDelay(1);
	}
	return -7;
}


