#include "sim.h"
#include "serial.h"
#include "drivers.h"
#include "serial.h"

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

/**
  * TCP服务端信息结构体
  */
typedef struct {
	char ip[15];                            // 地址
	uint32_t port;                          // 端口
	bool status;                            // 网络连接标志
}Sim_TcpServer_t;

// 发送指令索引
typedef enum {
	SIM_REQ_AT = 1,                         // 测试AT
	SIM_REQ_ATE0,                           // 关闭回显(若不关闭，模块会返回控制板发送的AT指令然后再传输模块的应答)
	SIM_REQ_CGDCONT,                        // 定义PDP文本
	SIM_REQ_CPIN,                           // 进入PIN
	SIM_REQ_CREG,                           // 网络登记
	SIM_REQ_CPSI,                           // 查询UE系统信息
	SIM_REQ_CSQ,                            // 信号强度
	SIM_REQ_CIPMODE_0,                      // 非透传模式
	SIM_REQ_CSOCKSETPN,                     // 设置活动的PDP上下文的配置文件编号
	SIM_REQ_NETOPEN,                        // 开启网络
	SIM_REQ_CIPOPEN,                        // connect
	SIM_REQ_NETOPEN_Q,                      // 查看网络状态
	SIM_REQ_CIPSEND_0,                      // 发送指定长度数据(非透传)
	SIM_REQ_DATA_0,                         //
	SIM_REQ_CIPMODE_1,                      // 透传模式
	SIM_REQ_CIPSEND,                        // 进入透传发送
	SIM_REQ_CRESET,                         // 复位模块
}SIM_ReqList_t;

// 接收指令索引
typedef enum {
	SIM_ACK_PBDONE = 0,                    //
	SIM_ACK_AT,                             //
	SIM_ACK_ATE0,                           //
	SIM_ACK_CGDCONT,                        //
	SIM_ACK_CPIN,                           //
	SIM_ACK_CREG,                           //
	SIM_ACK_CPSI,                           //
	SIM_ACK_CSQ,                            //
	SIM_ACK_CIPMODE_0,                      //
	SIM_ACK_CSOCKSETPN,                     //
	SIM_ACK_NETOPEN,                        //
	SIM_ACK_CIPOPEN,                        //
	SIM_ACK_NETOPEN_Q,                      //
	SIM_ACK_CIPSEND_0,                      //
	SIM_ACK_DATA_0,                         //
	SIM_ACK_CIPMODE_1,                      //
	SIM_ACK_CIPSEND,
	SIM_ACK_CRESET,
}SIM_AckList_t;

// 发送指令列表
char g_sim_ReqList[18][30] = {
	"",                                     //
	"AT\r\n",                               //
	"ATE0\r\n",                             //
	"AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n",    //
	"AT+CPIN?\r\n",                         //
	"AT+CREG?\r\n",                         //
	"AT+CPSI?\r\n",                         //
	"AT+CSQ\r\n",                           //
	"AT+CIPMODE=0\r\n",                     //
	"AT+CSOCKSETPN=1\r\n",                  //
	"AT+NETOPEN\r\n",                       //
	"AT+CIPOPEN=0,\"TCP\",\"%s\",%d\r\n",   //
	"AT+NETOPEN?\r\n",                      //
	"AT+CIPSEND=0,%d\r\n",                  //
	"",                                     //
	"AT+CIPMODE=1\r\n",
	"AT+CIPSEND\r\n",
	"AT+CRESET\r\n"
};

// 接收应答列表
char g_sim_AckList[18][30] = {
	"PB DONE\r\n",                          //
	"OK",                                   //
	"OK",                                   //
	"OK",                                   //
	"OK",                                   //
	"OK",                                   //
	"LTE",                                  //
	"+CSQ: 99,99",                          //
	"OK",                                   //
	"OK",                                   //
	"OK",                                   //
	"CONNECT 115200",                       //
	"+NETOPEN: 1",                          //
	">",                                    //
	"+CIPSEND: 0,%d,%d",                    //
	"OK",
	"OK",
	"OK"
};

/**
  * TCP服务端信息
  */
Sim_TcpServer_t g_sim_Server = { "8.129.53.188", 0x07E5, false };

/**
  * 标志
  */
bool g_sim_DebugPrintf = false;             // 调试打印标志

/**
  * SIM数据缓冲区
  */
u8  g_sim_RxBuf[UART6_BUFFSIZE];
u16 g_sim_RxLen = 0;
u8  g_sim_DataRxBuf[UART6_BUFFSIZE];
u16 g_sim_DataRxLen;

/**
  * 定时计数
  */
uint32_t g_simTick;
TickTimer_t g_simWhile1sTime = { .start = 0, .count = 1000 };

/**
  * 互斥锁与信号量(区分Sim线程与其他线程)
  */
extern osMutexId_t SimMutexHandle;
extern osSemaphoreId_t SimStatusBinarySemHandle;

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  SIM模块复位
  */
static void Sim_Reset(void);

/**
  * @brief  SIM配置
  */
static bool Sim_Config(void);

/**
  * @brief  SIM连接TCP服务器
  */
static bool Sim_Tcp(void);

/**
  * @brief  等待应答
  */
static bool Sim_WaitAck(char *ackBuf, u16 timeout);

/**
  * @brief  发送指令并等待应答
  */
static bool Sim_SendReq(char *req, char *ack, u8 timeout);

/**
  * @brief  发送指令并等待应答
  */
static bool Sim2_SendReq(char *req, char *ack, u16 timeout);

/**
  * @brief  等待SIM开启
  */
static bool Sim_WaitOpen(void);

/**
  * @brief  GSM初始化
  * @param  
  * @retval 
  */
bool Sim_Init(void)
{
	u8 cntout = 20;
	
	// 1.硬件初始化
	Sim_Reset();
		
	// 2.等待开机完成
	if (Sim_WaitOpen() == false) {
		DBG("Sim open error.\r\n");
	}
		
	// 3.测试AT通讯
	while (cntout > 0) {
		if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_AT], g_sim_AckList[SIM_ACK_AT], 20) == true) {
//			if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_ATE0], g_sim_AckList[SIM_ACK_ATE0], 20) == true) {
				// 设置PDP上下文信息
				if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CGDCONT], g_sim_AckList[SIM_ACK_CGDCONT], 1000) == true) {
					// 检查SIM卡状态
					if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CPIN], g_sim_AckList[SIM_ACK_CPIN], 1000) == true) {
						break;
					}
				}
//			}
		}
		cntout--;
	}
	if (cntout > 0) {
		// 4.SIM配置
		if (Sim_Config() == true) {
			// 5.TCP连接
			if (Sim_Tcp() == true) {
				DBG("Sim tcp connect ok.\r\n");
				// TCP连接成功
				g_sim_Server.status = true;
				osSemaphoreRelease(SimStatusBinarySemHandle);
				return true;
			} else {
				DBG("Sim tcp connect error.\r\n");
			}
		} else {
			DBG("Sim config error.\r\n");
		}
	}

	return false;
}

/**
  * @brief  SIM轮询处理
  * @param  
  * @retval 
  */
void Sim_Process(void)
{
	u8 cntout = 20;
	uint16_t rxLen = 0;
	
	g_simTick = HAL_GetTick();
	
	// 上锁
	osMutexAcquire(SimMutexHandle, osWaitForever);

	if (g_sim_Server.status == true) {
		
		// 轮询接收数据
		rxLen = Uart_GetData(&huart6, g_sim_RxBuf);
		if (rxLen > 0) {
			if (g_sim_DebugPrintf == true) {
				printf("%s", g_sim_RxBuf);
			}
			
			if (memcmp(g_sim_RxBuf, "OK", 2) != 0) {
				g_sim_DataRxLen = rxLen;
				memcpy(g_sim_DataRxBuf, g_sim_RxBuf, rxLen);
			} else {
				g_sim_Server.status = false;
			}
			
			memset(g_sim_RxBuf, 0, UART6_BUFFSIZE);
		}
	} else {
		// 断网重连
		// 1.模块复位
		if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CRESET], g_sim_AckList[SIM_ACK_CRESET], 3000) == true) {
			// 2.等待开机完成
			if (Sim_WaitOpen() == false) {
				DBG("Sim open error.\r\n");
			}
			
			// 3.测试AT通讯
			while (cntout > 0) {
				if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_AT], g_sim_AckList[SIM_ACK_AT], 20) == true) {
		//			if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_ATE0], g_sim_AckList[SIM_ACK_ATE0], 20) == true) {
						// 设置PDP上下文信息
						if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CGDCONT], g_sim_AckList[SIM_ACK_CGDCONT], 1000) == true) {
							// 检查SIM卡状态
							if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CPIN], g_sim_AckList[SIM_ACK_CPIN], 1000) == true) {
								break;
							}
						}
		//			}
				}
				cntout--;
			}
			if (cntout > 0) {
				// 4.SIM配置
				if (Sim_Config() == true) {
					// 5.TCP连接
					if (Sim_Tcp() == true) {
						DBG("Sim tcp connect ok.\r\n");
						// TCP连接成功
						g_sim_Server.status = true;
						osSemaphoreRelease(SimStatusBinarySemHandle);

					} else {
						DBG("Sim tcp connect error.\r\n");
					}
				} else {
					DBG("Sim config error.\r\n");
				}
			}
		} else {
			DBG("Sim reset fail.\r\n");
		}
	}

	// 解锁
	osMutexRelease(SimMutexHandle);

	if (g_simTick - g_simWhile1sTime.start >= g_simWhile1sTime.count) {
		g_simWhile1sTime.start = g_simTick;

//		printf("sim + 1\r\n");
	}
}

/**
  * @brief  发送数据
  * @param  
  * @retval 
  */
void Sim_PutData(uint8_t *pBuffer, uint16_t len)
{
	// 上锁
	osMutexAcquire(SimMutexHandle, osWaitForever);

	// 透传模式
	Uart6_Send(pBuffer, len);
	
	// 解锁
	osMutexRelease(SimMutexHandle);
}

/**
  * @brief  获取接收到的数据
  * @param  buffer  接收数据缓冲区
  * @retval 接收到的数据长度
  */
uint16_t Sim_GetData(uint8_t *pBuffer)
{
	uint16_t rxLen = 0;
	if (g_sim_DataRxLen > 0) {
		// 上锁
		osMutexAcquire(SimMutexHandle, osWaitForever);

		// 复制数据
		memcpy(pBuffer, g_sim_DataRxBuf, g_sim_DataRxLen);
		rxLen = g_sim_DataRxLen;
		
		// 清空缓冲区
		memset(g_sim_DataRxBuf, 0, UART6_BUFFSIZE);
		g_sim_DataRxLen = 0;
		
		// 解锁
		osMutexRelease(SimMutexHandle);
	}
	return rxLen;
}

/**
  * @brief  获取SIM网络连接状态
  * @param  
  * @retval true  - 网络连接成功
  *         false - 网络连接失败
  */
bool Sim_GetLinkStatus(void)
{
	bool status = false;
	
	// 上锁
	osMutexAcquire(SimMutexHandle, osWaitForever);

	status = g_sim_Server.status;
	
	// 解锁
	osMutexRelease(SimMutexHandle);
	
	return status;
}

/**
  * @brief  SIM模块复位
  * @param  
  * @retval 
  */
static void Sim_Reset(void)
{
	// SIM状态灯初始化
	HAL_GPIO_WritePin(GSM_STA_GPIO_Port, GSM_STA_Pin, GPIO_PIN_RESET);

	// SIM模块复位
	HAL_GPIO_WritePin(MPCIE_PWR_GPIO_Port, MPCIE_PWR_Pin, GPIO_PIN_SET);
	osDelay(1000);
	HAL_GPIO_WritePin(MPCIE_PWR_GPIO_Port, MPCIE_PWR_Pin, GPIO_PIN_RESET);

	// SIM通讯初始化
	Uart_SetRxEnable(&huart6);
}

/**
  * @brief  SIM配置
  * @param  
  * @retval 
  */
static bool Sim_Config(void)
{
	u8 cntout = 20;
	while (cntout > 0) {
		// 1.网络登记
		if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CREG], g_sim_AckList[SIM_ACK_CREG], 1000) == true) {
			// 2.查询网络注册信息
			if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CPSI], g_sim_AckList[SIM_ACK_CPSI], 1000) == true) {
				return true;
			}
		}
		cntout--;
	}
	return false;
}

/**
  * @brief  SIM连接TCP服务器
  * @param  
  * @retval 
  */
static bool Sim_Tcp(void)
{
	u8 timeout = 20;
	char tmpReq[50] = { 0 };
	
	while (timeout > 0) {
		// 0.检查RF信号强度
		if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CSQ], g_sim_AckList[SIM_ACK_CSQ], 1000) == false) {
			// 1.设置活动的PDP上下文的配置文件编号
			if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CSOCKSETPN], g_sim_AckList[SIM_ACK_CSOCKSETPN], 1000) == true) {
				// 查询网络状态
				if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_NETOPEN_Q], g_sim_AckList[SIM_ACK_NETOPEN_Q], 1000) == true) {
					// 连接TCP服务器
					sprintf((char *)tmpReq, g_sim_ReqList[SIM_REQ_CIPOPEN], g_sim_Server.ip, g_sim_Server.port);
					if (Sim2_SendReq((char *)tmpReq, g_sim_AckList[SIM_ACK_CIPOPEN], 10000) == true) {
						return true;
					} else {
						osDelay(500);
					}
				} else {
					// 进入传输模式(可选透传或非透传)
					if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CIPMODE_1], g_sim_AckList[SIM_ACK_CIPMODE_1], 1000) == true) {
						// 启动TCPIP服务
						Uart6_Send((uint8_t *)g_sim_ReqList[SIM_REQ_NETOPEN], strlen(g_sim_ReqList[SIM_REQ_NETOPEN]));
					} else {
						osDelay(500);
					}
				}
			} else {
				osDelay(500);
			}
		} else {
			osDelay(500);
		}
		timeout--;
	}
	return false;
}

/**
  * @brief  等待应答
  * @param  ackBuf
  *         timeout
  * @retval 
  */
static bool Sim_WaitAck(char *ackBuf, uint16_t timeout)
{
	bool ret = false;
	void *ptr = NULL;
	uint16_t ackLen = 0;
		
	while (timeout--) {
		// 接收数据
		ackLen = Uart_GetData(&huart6, g_sim_RxBuf + g_sim_RxLen);	
		if (ackLen > 0) {
			// 清除第一个空字符
			if ((g_sim_RxBuf[0] == 0) && (ackLen == 1)) {
				ackLen = 0;
				continue;
			}
			
			// 调试打印
			if (g_sim_DebugPrintf == true) {
				printf("%s", g_sim_RxBuf + g_sim_RxLen);
			}
			g_sim_RxLen += ackLen;
			
			// 检查应答
			ptr = strstr((char *)g_sim_RxBuf, ackBuf);
			if (ptr != NULL) {
				// 应答正确
				ret = true;
				timeout = 0;
			}
			
			ackLen = 0;
			
			// 去除一条以\r\n结束的反馈
			if ((g_sim_RxBuf[g_sim_RxLen - 2] == 0x0D) && (g_sim_RxBuf[g_sim_RxLen - 1] == 0x0A)) {
				memset(g_sim_RxBuf, 0, g_sim_RxLen);
				g_sim_RxLen = 0;
			}
		}

		osDelay(1);
	}
	
	// 复位应答缓冲区
	memset(g_sim_RxBuf, 0, UART6_BUFFSIZE);
	g_sim_RxLen = 0;
	
	return ret;
}

/**
  * @brief  发送指令并等待应答
  * @param  
  * @retval 
  */
static bool Sim_SendReq(char *req, char *ack, u8 timeout)
{
	bool ret = false;
	while (timeout-- > 0) {
		// 发送指令
		Uart6_Send((u8 *)req, strlen(req));
		
		// 等待应答1000ms
		if (Sim_WaitAck(ack, 1000) == true) {
			ret = true;
			break;
		}
	}
	return ret;
}


/**
  * @brief  发送指令并等待应答
  * @param  
  * @retval 
  */
static bool Sim2_SendReq(char *req, char *ack, u16 timeout)
{
	bool ret = false;
	
	// 发送指令
	Uart6_Send((u8 *)req, strlen(req));
	
	// 等待应答1000ms
	if (Sim_WaitAck(ack, timeout) == true) {
		ret = true;
	}
	
	return ret;
}

/**
  * @brief  等待SIM开启
  * @param  
  * @retval 
  */
static bool Sim_WaitOpen(void)
{
	char prepare[30] = { 0 };

	sprintf(prepare, "RDY\r\n");
	if (Sim_WaitAck(prepare, 30000) == true) {
		sprintf(prepare, "+CPIN: READY\r\n");
		if (Sim_WaitAck(prepare, 30000) == true) {
			sprintf(prepare, "SMS DONE\r\n");
			if (Sim_WaitAck(prepare, 30000) == true) {
				sprintf(prepare, "PB DONE\r\n");
				if (Sim_WaitAck(prepare, 30000) == true) {
					return true;
				}
			}
		}
	}
	
	return false;
}
