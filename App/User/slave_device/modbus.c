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
		{ .addr = MODBUS_ADDR_MACHINE_RESET, .data = 0, },	// 机器复位
		{ .addr = MODBUS_ADDR_CHEAR_LABEL, .data = 0, },	// 
		{ .addr = MODBUS_ADDR_CLEAR_RESET, .data = 0, },	// 
		{ .addr = MODBUS_ADDR_TRIGGER_LAB, .data = 0, },	// 贴标触发
		{ .addr = MODBUS_ADDR_LABEL_ERROR, .data = 0, },	// 贴标错误
		{ .addr = MODBUS_ADDR_PRESS_ERROR, .data = 0, },	// 
		{ .addr = MODBUS_ADDR_CLAMP_ERROR, .data = 0, },
		{ .addr = MODBUS_ADDR_LOCK_STATUS, .data = 0, },	// 锁状态
		{ .addr = MODBUS_ADDR_WORK_STATUS, .data = 0, },	// 工作状态
		{ .addr = MODBUS_ADDR_LOCK_OPERATE, .data = 0, },	// 锁操作
		{ .addr = MODBUS_ADDR_VALIB_LABEL, .data = 0, },	// 有效标签数
		{ .addr = MODBUS_ADDR_INVAL_LABEL, .data = 0, },	// 无效标签数
	},
};

/**
  * 数据缓冲区
  */
static uint8_t  g_modbusRxBuf[UART7_BUFFSIZE] = { 0 };
static uint16_t g_modbusRxLen = 0;

/**
  * 定时计数
  */
static uint32_t g_modbusTick = 0;
static TickTimer_t g_modbusAckTimeout;// 应答超时
static TickTimer_t g_modbusClearReset;// 清除复位
static TickTimer_t g_modbusTrigReset; // 触发复位

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  发送指令处理
  */
static void Modbus_TxProcess(void);

/**
  * @brief  接收处理
  */
static void Modbus_RxProcess(void);

/**
  * @brief  处理定时器
  */
static void Modbus_TimProcess(void);

/**
  * @brief  处理功能码为01的指令
  */
static void Modbus_01_Pro(void);

/**
  * @brief  处理功能码为03的指令
  */
static void Modbus_03_Pro(void);

/**
  * @brief  处理功能码为05的指令
  */
static void Modbus_05_Pro(void);

/**
  * @brief  处理功能码为06的指令
  */
static void Modbus_06_Pro(void);

/**
  * @brief  发送指令
  */
static void Modbus_SendCmd(ModbusData_t *info);

/**
  * @brief  Modbus-RTU通讯初始化
  * @param  
  * @retval 
  */
void Modbus_Init( void )
{
	// 贴标机初始化
	Uart_SetRxEnable(&huart7);

	Modbus_AddCmd(MODBUS_CMD_MACHINE_RESET);
}

/**
  * @brief  Modbus-RTU协议处理
  * @param  
  * @retval 
  */
void Modbus_Process(void)
{
	g_modbusTick = HAL_GetTick();
	
	Modbus_TxProcess();     // 发送指令
	Modbus_RxProcess();     // 接收处理
	Modbus_TimProcess();    // 定时器处理
}

/**
  * @brief  获取Modbus指令信息地址指针
  * @param  
  * @retval 
  */
ModbusInfo_t *Modbus_GetParam(void)
{
	return &g_modbusInfo;
}

/**
  * @brief  添加Modbus-RTU指令到指令队列
  * @param  cmd   Modbus-RTU指令
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
  * @brief  发送指令处理
  * @remark 发送单条指令可以直接在发送时调用即可，但由于指令需要等待应答并重发，
  *         此时要发送指令就会冲突，因此创建指令列表，在等待应答时，需要发送指令
  *         就存储到指令列表中，等当前指令获取到应答或超过重发最大次数后，再发送
  *         指令列表中的指令。指令列表需要在上一条指令流程结束才能发送，因此发送
  *         处理需要放在main循环中一直查看。
  */
static void Modbus_TxProcess(void)
{
	if (g_modbusAckTimeout.count == 0) {    // modbus处于空闲
		// 指令内容处理
		if (g_modbusInfo.cmdType == MODBUS_CMD_NULL) {
			if (g_modbusInfo.cmdList.num > 0) {
				// 读取指令
				g_modbusInfo.cmdType = g_modbusInfo.cmdList.list[0];
				g_modbusInfo.cmdList.num--;
				
				// 删除指令
				memcpy(g_modbusInfo.cmdList.list, g_modbusInfo.cmdList.list + 1, g_modbusInfo.cmdList.num);
				g_modbusInfo.cmdList.list[g_modbusInfo.cmdList.num] = MODBUS_CMD_NULL;
			}
		}
		
		// 清除上一条指令数据
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
				g_modbusTrigReset.count = MODBUS_TRIGGERRESET_TIME;   // 5秒后复位
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
  * @brief  接收处理
  * @param  
  * @retval 
  */
static void Modbus_RxProcess(void)
{
	uint16_t calcCRC = 0, recvCRC = 0;
	g_modbusRxLen = Uart_GetData(&huart7, g_modbusRxBuf);   // 获取RTU串口数据
	if (g_modbusRxLen > 0) {
		// 打印数据
		#if (DBG_TRACE == 1)
			DBG("recv(PLC): ");
			PrintHexBuffer(g_modbusRxBuf, g_modbusRxLen);
		#endif
		
		// 1.检验机械码
		if (g_modbusRxBuf[0] == 0x01) {
			Error_Del(ERROR_DEVICE_UNCOMMUNICATE);

			// 2.检验功能码
			if (g_modbusRxBuf[1] == g_modbusInfo.cmdData.funCode) {
				
				// 3.校验CRC
				calcCRC = CalCrc16(g_modbusRxBuf, g_modbusRxLen - 2, 0xFFFF);
				recvCRC = ((uint16_t)g_modbusRxBuf[g_modbusRxLen - 1] << 8) | g_modbusRxBuf[g_modbusRxLen - 2];
				/***********判断CRC校验正确*************/
				if (calcCRC == recvCRC) { //CRC校验正确
					
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
  * @brief  处理定时器
  * @param  
  * @retval 
  */
static void Modbus_TimProcess(void)
{
	// 应答超时
	if ((g_modbusAckTimeout.count > 0) && (g_modbusTick - g_modbusAckTimeout.start >= g_modbusAckTimeout.count)) {
		g_modbusAckTimeout.start = 0;
		g_modbusAckTimeout.count = 0;
				
		if (g_modbusInfo.reTxNum < MODBUS_MAX_RETRY_NUMBER) {// 重发指令
			g_modbusInfo.reTxNum++;
		} else {		// 超过最大重发次数，不再重发
			g_modbusInfo.reTxNum = 0;
			g_modbusInfo.cmdType = MODBUS_CMD_NULL;
		}
	}
	
	// 清除复位
	if ((g_modbusClearReset.count > 0) && (g_modbusTick - g_modbusClearReset.start >= g_modbusClearReset.count)) {
		g_modbusClearReset.start = 0;
		g_modbusClearReset.count = 0;

		// 发送清除复位指令
		Modbus_AddCmd(MODBUS_CMD_CLEAR_RESET);
    }
	
	// 触发复位
	if ((g_modbusTrigReset.count > 0) && (g_modbusTick - g_modbusTrigReset.start >= g_modbusTrigReset.count)) {
		g_modbusTrigReset.start = 0;
		g_modbusTrigReset.count = 0;

		// 发送触发复位指令
		Modbus_AddCmd(MODBUS_CMD_TRIGGER_RESET);
	}

}

/**
  * @brief  处理功能码为01的指令
  * @param  
  * @retval 
  */
static void Modbus_01_Pro(void)
{
	uint8_t i = 0;
	if (g_modbusRxLen >= 6) {   // 长度正确
		if (g_modbusRxBuf[2] == 1) {    // 载荷长度正确
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
  * @brief  处理功能码为03的指令
  * @param  
  * @retval 
  */
static void Modbus_03_Pro(void)
{
	uint8_t i = 0;
	if (g_modbusRxLen >= 7) {   // 长度正确
		if (g_modbusRxBuf[2] == 2) {    // 载荷长度正确
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
  * @brief  处理功能码为05的指令
  * @param  
  * @retval 
  */
static void Modbus_05_Pro(void)
{
	uint8_t i = 0;
	uint16_t startAddr = 0, data = 0;
	if (g_modbusRxLen >= 8) {   // 长度正确
		startAddr = ((uint16_t)g_modbusRxBuf[2] << 8) | g_modbusRxBuf[3];
		data = ((uint16_t)g_modbusRxBuf[4] << 8) | g_modbusRxBuf[5];
		if (startAddr == g_modbusInfo.cmdData.addr) {   // 地址正确
			for (i = 0; i < MODBUS_MAX_REGISTER_NUM; i++) {
				if (g_modbusInfo.regList[i].addr == startAddr) {
					g_modbusInfo.regList[i].data = data;
					g_modbusInfo.cmdType = MODBUS_CMD_NULL;
					g_modbusAckTimeout.start = 0;
					g_modbusAckTimeout.count = 0;
					
					if (g_modbusInfo.devJoin == false) {// Modbus通讯正常
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
  * @brief  处理功能码为06的指令
  * @param  
  * @retval 
  */
static void Modbus_06_Pro(void)
{
	uint8_t i = 0;
	uint16_t startAddr = 0, data = 0;
	if (g_modbusRxLen >= 8) {   // 长度正确
		startAddr = ((uint16_t)g_modbusRxBuf[2] << 8) | g_modbusRxBuf[3];
		data = ((uint16_t)g_modbusRxBuf[4] << 8) | g_modbusRxBuf[5];
		if (startAddr == g_modbusInfo.cmdData.addr) {   // 地址正确
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
  * @brief  发送指令
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
	
	//打印接收数据
#if (DBG_TRACE == 1)
	DBG("send(PLC): ");
	PrintHexBuffer(txBuf, txLen);
#endif

	// 设置超时计数
	g_modbusAckTimeout.start = g_modbusTick;
	g_modbusAckTimeout.count = MODBUS_ACK_TIMEOUT_TIME;
}


