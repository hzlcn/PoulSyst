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
	.cmdList = { MODBUS_CMD_NULL },
	.cmdData = { 0 },
	.cmdType = MODBUS_CMD_NULL,
	.reTxNum = 0,
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
static uint8_t  g_modbusRxBuf[UART7_BUFFSIZE] = { 0 };
static uint16_t g_modbusRxLen = 0;

/**
  * ��ʱ����
  */
static uint32_t g_modbusTick = 0;
static TickTimer_t g_modbusAckTimeout;// Ӧ��ʱ
static TickTimer_t g_modbusClearReset;// �����λ
static TickTimer_t g_modbusTrigReset; // ������λ

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ����ָ���
  */
static void Modbus_TxProcess(void);

/**
  * @brief  ���մ���
  */
static void Modbus_RxProcess(void);

/**
  * @brief  ����ʱ��
  */
static void Modbus_TimProcess(void);

/**
  * @brief  ��������Ϊ01��ָ��
  */
static void Modbus_01_Pro(void);

/**
  * @brief  ��������Ϊ03��ָ��
  */
static void Modbus_03_Pro(void);

/**
  * @brief  ��������Ϊ05��ָ��
  */
static void Modbus_05_Pro(void);

/**
  * @brief  ��������Ϊ06��ָ��
  */
static void Modbus_06_Pro(void);

/**
  * @brief  ����ָ��
  */
static void Modbus_SendCmd(ModbusData_t *info);

/**
  * @brief  Modbus-RTUͨѶ��ʼ��
  * @param  
  * @retval 
  */
void Modbus_Init( void )
{
	// �������ʼ��
	Uart_SetRxEnable(&huart7);

	Modbus_AddCmd(MODBUS_CMD_MACHINE_RESET);
}

/**
  * @brief  Modbus-RTUЭ�鴦��
  * @param  
  * @retval 
  */
void Modbus_Process(void)
{
	g_modbusTick = HAL_GetTick();
	
	Modbus_TxProcess();     // ����ָ��
	Modbus_RxProcess();     // ���մ���
	Modbus_TimProcess();    // ��ʱ������
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
  * @brief  ���Modbus-RTUָ�ָ�����
  * @param  cmd   Modbus-RTUָ��
  * @retval 
  */
void Modbus_AddCmd(ModbusCmdType_t cmd)
{
	u8 i = 0;
	for (i = 0; i < g_modbusInfo.cmdList.num; i++) {
		if (cmd == g_modbusInfo.cmdList.list[i]) {
			return ;
		}
	}
	if (g_modbusInfo.cmdList.num < MODBUS_LIST_MAX_NUMBER) {
		g_modbusInfo.cmdList.list[g_modbusInfo.cmdList.num++] = cmd;
	}
}

/**
  * @brief  ����ָ���
  * @remark ���͵���ָ�����ֱ���ڷ���ʱ���ü��ɣ�������ָ����Ҫ�ȴ�Ӧ���ط���
  *         ��ʱҪ����ָ��ͻ��ͻ����˴���ָ���б��ڵȴ�Ӧ��ʱ����Ҫ����ָ��
  *         �ʹ洢��ָ���б��У��ȵ�ǰָ���ȡ��Ӧ��򳬹��ط����������ٷ���
  *         ָ���б��е�ָ�ָ���б���Ҫ����һ��ָ�����̽������ܷ��ͣ���˷���
  *         ������Ҫ����mainѭ����һֱ�鿴��
  */
static void Modbus_TxProcess(void)
{
	if (g_modbusAckTimeout.count == 0) {    // modbus���ڿ���
		// ָ�����ݴ���
		if (g_modbusInfo.cmdType == MODBUS_CMD_NULL) {
			if (g_modbusInfo.cmdList.num > 0) {
				// ��ȡָ��
				g_modbusInfo.cmdType = g_modbusInfo.cmdList.list[0];
				g_modbusInfo.cmdList.num--;
				
				// ɾ��ָ��
				memcpy(g_modbusInfo.cmdList.list, g_modbusInfo.cmdList.list + 1, g_modbusInfo.cmdList.num);
				g_modbusInfo.cmdList.list[g_modbusInfo.cmdList.num] = MODBUS_CMD_NULL;
			}
		}
		
		// �����һ��ָ������
		memset(&g_modbusInfo.cmdData, 0, sizeof(ModbusData_t));
		
		switch (g_modbusInfo.cmdType) {
			case MODBUS_CMD_TRIGGER_LABEL:
			{
				g_modbusInfo.cmdData.devCode = 0x01;
				g_modbusInfo.cmdData.funCode = MODBUS_FUN_05;
				g_modbusInfo.cmdData.addr = MODBUS_ADDR_TRIGGER_LAB;
				g_modbusInfo.cmdData.data.wrData = 0xFF00;
				Modbus_SendCmd(&g_modbusInfo.cmdData);
				
				g_modbusTrigReset.start = g_modbusTick;
				g_modbusTrigReset.count = MODBUS_TRIGGERRESET_TIME;   // 5���λ
				break;
			}
			case MODBUS_CMD_TRIGGER_RESET:
			{
				g_modbusInfo.cmdData.devCode = 0x01;
				g_modbusInfo.cmdData.funCode = MODBUS_FUN_05;
				g_modbusInfo.cmdData.addr = MODBUS_ADDR_TRIGGER_LAB;
				g_modbusInfo.cmdData.data.wrData = 0x0000;
				Modbus_SendCmd(&g_modbusInfo.cmdData);
				break;
			}
			case MODBUS_CMD_READ_VALIBNUM:
			{
				g_modbusInfo.cmdData.devCode = 0x01;
				g_modbusInfo.cmdData.funCode = MODBUS_FUN_03;
				g_modbusInfo.cmdData.addr = MODBUS_ADDR_VALIB_LABEL;
				g_modbusInfo.cmdData.data.readLen = 0x0001;
				Modbus_SendCmd(&g_modbusInfo.cmdData);
				break;
			}
			case MODBUS_CMD_READ_INVALNUM:
			{
				g_modbusInfo.cmdData.devCode = 0x01;
				g_modbusInfo.cmdData.funCode = MODBUS_FUN_03;
				g_modbusInfo.cmdData.addr = MODBUS_ADDR_INVAL_LABEL;
				g_modbusInfo.cmdData.data.readLen = 0x0001;
				Modbus_SendCmd(&g_modbusInfo.cmdData);
				break;
			}
			case MODBUS_CMD_MACHINE_RESET:
			{
				g_modbusInfo.cmdData.devCode = 0x01;
				g_modbusInfo.cmdData.funCode = MODBUS_FUN_05;
				g_modbusInfo.cmdData.addr = MODBUS_ADDR_MACHINE_RESET;
				g_modbusInfo.cmdData.data.wrData = 0xFF00;
				Modbus_SendCmd(&g_modbusInfo.cmdData);
				break;
			}
			case MODBUS_CMD_CLEAR_RESET:
			{
				g_modbusInfo.cmdData.devCode = 0x01;
				g_modbusInfo.cmdData.funCode = MODBUS_FUN_05;
				g_modbusInfo.cmdData.addr = MODBUS_ADDR_CLEAR_RESET;
				g_modbusInfo.cmdData.data.wrData = 0xFF00;
				Modbus_SendCmd(&g_modbusInfo.cmdData);
				break;
			}
			case MODBUS_CMD_UNLOCK_DEVICE:
			{
				g_modbusInfo.cmdData.devCode = 0x01;
				g_modbusInfo.cmdData.funCode = MODBUS_FUN_05;
				g_modbusInfo.cmdData.addr = MODBUS_ADDR_LOCK_OPERATE;
				g_modbusInfo.cmdData.data.wrData = 0xFF00;
				Modbus_SendCmd(&g_modbusInfo.cmdData);
				break;
			}
			case MODBUS_CMD_GETLOCKSTATUS:
			{
				g_modbusInfo.cmdData.devCode = 0x01;
				g_modbusInfo.cmdData.funCode = MODBUS_FUN_01;
				g_modbusInfo.cmdData.addr = MODBUS_ADDR_LOCK_STATUS;
				g_modbusInfo.cmdData.data.readLen = 0x0001;
				Modbus_SendCmd(&g_modbusInfo.cmdData);
				break;
			}
			default:
				break;
		}
	}
}

/**
  * @brief  ���մ���
  * @param  
  * @retval 
  */
static void Modbus_RxProcess(void)
{
	uint16_t calcCRC = 0, recvCRC = 0;
	g_modbusRxLen = Uart_GetData(&huart7, g_modbusRxBuf);   // ��ȡRTU��������
	if (g_modbusRxLen > 0) {
		// ��ӡ����
		#if (DBG_TRACE == 1)
			DBG("recv(PLC): ");
			PrintHexBuffer(g_modbusRxBuf, g_modbusRxLen);
		#endif
		
		// 1.�����е��
		if (g_modbusRxBuf[0] == 0x01) {
			Error_Del(ERROR_DEVICE_UNCOMMUNICATE);

			// 2.���鹦����
			if (g_modbusRxBuf[1] == g_modbusInfo.cmdData.funCode) {
				
				// 3.У��CRC
				calcCRC = CalCrc16(g_modbusRxBuf, g_modbusRxLen - 2, 0xFFFF);
				recvCRC = ((uint16_t)g_modbusRxBuf[g_modbusRxLen - 1] << 8) | g_modbusRxBuf[g_modbusRxLen - 2];
				/***********�ж�CRCУ����ȷ*************/
				if (calcCRC == recvCRC) { //CRCУ����ȷ
					
					if (g_modbusInfo.cmdData.funCode == g_modbusRxBuf[1]) {
						switch (g_modbusRxBuf[1]) {
							case 1:
								Modbus_01_Pro();
								break;
							case 3:
								Modbus_03_Pro();
								break;
							case 5:
								Modbus_05_Pro();
								break;
							case 6:
								Modbus_06_Pro();
								break;
							default:
								break;
						}	
					}
				}
			}
		}
		
		memset(g_modbusRxBuf, 0, UART7_BUFFSIZE);
	}
}

/**
  * @brief  ����ʱ��
  * @param  
  * @retval 
  */
static void Modbus_TimProcess(void)
{
	// Ӧ��ʱ
	if ((g_modbusAckTimeout.count > 0) && (g_modbusTick - g_modbusAckTimeout.start >= g_modbusAckTimeout.count)) {
		g_modbusAckTimeout.start = 0;
		g_modbusAckTimeout.count = 0;
				
		if (g_modbusInfo.reTxNum < MODBUS_MAX_RETRY_NUMBER) {// �ط�ָ��
			g_modbusInfo.reTxNum++;
		} else {		// ��������ط������������ط�
			g_modbusInfo.reTxNum = 0;
			g_modbusInfo.cmdType = MODBUS_CMD_NULL;
		}
	}
	
	// �����λ
	if ((g_modbusClearReset.count > 0) && (g_modbusTick - g_modbusClearReset.start >= g_modbusClearReset.count)) {
		g_modbusClearReset.start = 0;
		g_modbusClearReset.count = 0;

		// ���������λָ��
		Modbus_AddCmd(MODBUS_CMD_CLEAR_RESET);
    }
	
	// ������λ
	if ((g_modbusTrigReset.count > 0) && (g_modbusTick - g_modbusTrigReset.start >= g_modbusTrigReset.count)) {
		g_modbusTrigReset.start = 0;
		g_modbusTrigReset.count = 0;

		// ���ʹ�����λָ��
		Modbus_AddCmd(MODBUS_CMD_TRIGGER_RESET);
	}

}

/**
  * @brief  ��������Ϊ01��ָ��
  * @param  
  * @retval 
  */
static void Modbus_01_Pro(void)
{
	uint8_t i = 0;
	if (g_modbusRxLen >= 6) {   // ������ȷ
		if (g_modbusRxBuf[2] == 1) {    // �غɳ�����ȷ
			for (i = 0; i < MODBUS_MAX_REGISTER_NUM; i++) {
				if (g_modbusInfo.regList[i].addr == g_modbusInfo.cmdData.addr) {
					g_modbusInfo.regList[i].data = g_modbusRxBuf[3];
					g_modbusInfo.cmdType = MODBUS_CMD_NULL;
					g_modbusAckTimeout.start = 0;
					g_modbusAckTimeout.count = 0;
					
					break;
				}
			}
		}
	}
}

/**
  * @brief  ��������Ϊ03��ָ��
  * @param  
  * @retval 
  */
static void Modbus_03_Pro(void)
{
	uint8_t i = 0;
	if (g_modbusRxLen >= 7) {   // ������ȷ
		if (g_modbusRxBuf[2] == 2) {    // �غɳ�����ȷ
			for (i = 0; i < MODBUS_MAX_REGISTER_NUM; i++) {
				if (g_modbusInfo.regList[i].addr == g_modbusInfo.cmdData.addr) {
					g_modbusInfo.regList[i].data = (g_modbusRxBuf[3] << 8) | g_modbusRxBuf[4];
					g_modbusInfo.cmdType = MODBUS_CMD_NULL;
					g_modbusAckTimeout.start = 0;
					g_modbusAckTimeout.count = 0;
					break;
				}
			}
		}
	}
}

/**
  * @brief  ��������Ϊ05��ָ��
  * @param  
  * @retval 
  */
static void Modbus_05_Pro(void)
{
	uint8_t i = 0;
	uint16_t startAddr = 0, data = 0;
	if (g_modbusRxLen >= 8) {   // ������ȷ
		startAddr = ((uint16_t)g_modbusRxBuf[2] << 8) | g_modbusRxBuf[3];
		data = ((uint16_t)g_modbusRxBuf[4] << 8) | g_modbusRxBuf[5];
		if (startAddr == g_modbusInfo.cmdData.addr) {   // ��ַ��ȷ
			for (i = 0; i < MODBUS_MAX_REGISTER_NUM; i++) {
				if (g_modbusInfo.regList[i].addr == startAddr) {
					g_modbusInfo.regList[i].data = data;
					g_modbusInfo.cmdType = MODBUS_CMD_NULL;
					g_modbusAckTimeout.start = 0;
					g_modbusAckTimeout.count = 0;
					
					if (g_modbusInfo.devJoin == false) {// ModbusͨѶ����
						if (startAddr == MODBUS_ADDR_MACHINE_RESET) {
							g_modbusInfo.devJoin = true;
						}
					}
					break;
				}
			}
		}
	}
}

/**
  * @brief  ��������Ϊ06��ָ��
  * @param  
  * @retval 
  */
static void Modbus_06_Pro(void)
{
	uint8_t i = 0;
	uint16_t startAddr = 0, data = 0;
	if (g_modbusRxLen >= 8) {   // ������ȷ
		startAddr = ((uint16_t)g_modbusRxBuf[2] << 8) | g_modbusRxBuf[3];
		data = ((uint16_t)g_modbusRxBuf[4] << 8) | g_modbusRxBuf[5];
		if (startAddr == g_modbusInfo.cmdData.addr) {   // ��ַ��ȷ
			for (i = 0; i < MODBUS_MAX_REGISTER_NUM; i++) {
				if (g_modbusInfo.regList[i].addr == startAddr) {
					g_modbusInfo.regList[i].data = data;
					g_modbusInfo.cmdType = MODBUS_CMD_NULL;
					g_modbusAckTimeout.start = 0;
					g_modbusAckTimeout.count = 0;
					break;
				}
			}
		}
	}
}

/**
  * @brief  ����ָ��
  * @param  
  * @retval 
  */
static void Modbus_SendCmd(ModbusData_t *info)
{
	uint8_t  txBuf[UART7_BUFFSIZE] = { 0 };
	uint16_t txLen = 0;

	txBuf[txLen++] = info->devCode;
	txBuf[txLen++] = info->funCode;
	txBuf[txLen++] = info->addr >> 8;
	txBuf[txLen++] = info->addr;
	txBuf[txLen++] = info->data.wrData >> 8;
	txBuf[txLen++] = info->data.wrData;
	
	info->crc = CalCrc16(txBuf, 6, 0xFFFF);
	txBuf[txLen++] = (info->crc & 0x00FF) >> 0;
	txBuf[txLen++] = (info->crc & 0xFF00) >> 8;

	Uart7_Send(txBuf, txLen);
	
	//��ӡ��������
#if (DBG_TRACE == 1)
	DBG("send(PLC): ");
	PrintHexBuffer(txBuf, txLen);
#endif

	// ���ó�ʱ����
	g_modbusAckTimeout.start = g_modbusTick;
	g_modbusAckTimeout.count = MODBUS_ACK_TIMEOUT_TIME;
}


