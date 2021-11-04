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
  * TCP�������Ϣ�ṹ��
  */
typedef struct {
	char ip[15];                            // ��ַ
	uint32_t port;                          // �˿�
	bool status;                            // �������ӱ�־
}Sim_TcpServer_t;

// ����ָ������
typedef enum {
	SIM_REQ_AT = 1,                         // ����AT
	SIM_REQ_ATE0,                           // �رջ���(�����رգ�ģ��᷵�ؿ��ư巢�͵�ATָ��Ȼ���ٴ���ģ���Ӧ��)
	SIM_REQ_CGDCONT,                        // ����PDP�ı�
	SIM_REQ_CPIN,                           // ����PIN
	SIM_REQ_CREG,                           // ����Ǽ�
	SIM_REQ_CPSI,                           // ��ѯUEϵͳ��Ϣ
	SIM_REQ_CSQ,                            // �ź�ǿ��
	SIM_REQ_CIPMODE_0,                      // ��͸��ģʽ
	SIM_REQ_CSOCKSETPN,                     // ���û��PDP�����ĵ������ļ����
	SIM_REQ_NETOPEN,                        // ��������
	SIM_REQ_CIPOPEN,                        // connect
	SIM_REQ_NETOPEN_Q,                      // �鿴����״̬
	SIM_REQ_CIPSEND_0,                      // ����ָ����������(��͸��)
	SIM_REQ_DATA_0,                         //
	SIM_REQ_CIPMODE_1,                      // ͸��ģʽ
	SIM_REQ_CIPSEND,                        // ����͸������
	SIM_REQ_CRESET,                         // ��λģ��
}SIM_ReqList_t;

// ����ָ������
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

// ����ָ���б�
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

// ����Ӧ���б�
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
  * TCP�������Ϣ
  */
Sim_TcpServer_t g_sim_Server = { "8.129.53.188", 0x07E5, false };

/**
  * ��־
  */
bool g_sim_DebugPrintf = false;             // ���Դ�ӡ��־

/**
  * SIM���ݻ�����
  */
u8  g_sim_RxBuf[UART6_BUFFSIZE];
u16 g_sim_RxLen = 0;
u8  g_sim_DataRxBuf[UART6_BUFFSIZE];
u16 g_sim_DataRxLen;

/**
  * ��ʱ����
  */
uint32_t g_simTick;
TickTimer_t g_simWhile1sTime = { .start = 0, .count = 1000 };

/**
  * ���������ź���(����Sim�߳��������߳�)
  */
extern osMutexId_t SimMutexHandle;
extern osSemaphoreId_t SimStatusBinarySemHandle;

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  SIMģ�鸴λ
  */
static void Sim_Reset(void);

/**
  * @brief  SIM����
  */
static bool Sim_Config(void);

/**
  * @brief  SIM����TCP������
  */
static bool Sim_Tcp(void);

/**
  * @brief  �ȴ�Ӧ��
  */
static bool Sim_WaitAck(char *ackBuf, u16 timeout);

/**
  * @brief  ����ָ��ȴ�Ӧ��
  */
static bool Sim_SendReq(char *req, char *ack, u8 timeout);

/**
  * @brief  ����ָ��ȴ�Ӧ��
  */
static bool Sim2_SendReq(char *req, char *ack, u16 timeout);

/**
  * @brief  �ȴ�SIM����
  */
static bool Sim_WaitOpen(void);

/**
  * @brief  GSM��ʼ��
  * @param  
  * @retval 
  */
bool Sim_Init(void)
{
	u8 cntout = 20;
	
	// 1.Ӳ����ʼ��
	Sim_Reset();
		
	// 2.�ȴ��������
	if (Sim_WaitOpen() == false) {
		DBG("Sim open error.\r\n");
	}
		
	// 3.����ATͨѶ
	while (cntout > 0) {
		if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_AT], g_sim_AckList[SIM_ACK_AT], 20) == true) {
//			if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_ATE0], g_sim_AckList[SIM_ACK_ATE0], 20) == true) {
				// ����PDP��������Ϣ
				if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CGDCONT], g_sim_AckList[SIM_ACK_CGDCONT], 1000) == true) {
					// ���SIM��״̬
					if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CPIN], g_sim_AckList[SIM_ACK_CPIN], 1000) == true) {
						break;
					}
				}
//			}
		}
		cntout--;
	}
	if (cntout > 0) {
		// 4.SIM����
		if (Sim_Config() == true) {
			// 5.TCP����
			if (Sim_Tcp() == true) {
				DBG("Sim tcp connect ok.\r\n");
				// TCP���ӳɹ�
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
  * @brief  SIM��ѯ����
  * @param  
  * @retval 
  */
void Sim_Process(void)
{
	u8 cntout = 20;
	uint16_t rxLen = 0;
	
	g_simTick = HAL_GetTick();
	
	// ����
	osMutexAcquire(SimMutexHandle, osWaitForever);

	if (g_sim_Server.status == true) {
		
		// ��ѯ��������
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
		// ��������
		// 1.ģ�鸴λ
		if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CRESET], g_sim_AckList[SIM_ACK_CRESET], 3000) == true) {
			// 2.�ȴ��������
			if (Sim_WaitOpen() == false) {
				DBG("Sim open error.\r\n");
			}
			
			// 3.����ATͨѶ
			while (cntout > 0) {
				if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_AT], g_sim_AckList[SIM_ACK_AT], 20) == true) {
		//			if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_ATE0], g_sim_AckList[SIM_ACK_ATE0], 20) == true) {
						// ����PDP��������Ϣ
						if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CGDCONT], g_sim_AckList[SIM_ACK_CGDCONT], 1000) == true) {
							// ���SIM��״̬
							if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CPIN], g_sim_AckList[SIM_ACK_CPIN], 1000) == true) {
								break;
							}
						}
		//			}
				}
				cntout--;
			}
			if (cntout > 0) {
				// 4.SIM����
				if (Sim_Config() == true) {
					// 5.TCP����
					if (Sim_Tcp() == true) {
						DBG("Sim tcp connect ok.\r\n");
						// TCP���ӳɹ�
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

	// ����
	osMutexRelease(SimMutexHandle);

	if (g_simTick - g_simWhile1sTime.start >= g_simWhile1sTime.count) {
		g_simWhile1sTime.start = g_simTick;

//		printf("sim + 1\r\n");
	}
}

/**
  * @brief  ��������
  * @param  
  * @retval 
  */
void Sim_PutData(uint8_t *pBuffer, uint16_t len)
{
	// ����
	osMutexAcquire(SimMutexHandle, osWaitForever);

	// ͸��ģʽ
	Uart6_Send(pBuffer, len);
	
	// ����
	osMutexRelease(SimMutexHandle);
}

/**
  * @brief  ��ȡ���յ�������
  * @param  buffer  �������ݻ�����
  * @retval ���յ������ݳ���
  */
uint16_t Sim_GetData(uint8_t *pBuffer)
{
	uint16_t rxLen = 0;
	if (g_sim_DataRxLen > 0) {
		// ����
		osMutexAcquire(SimMutexHandle, osWaitForever);

		// ��������
		memcpy(pBuffer, g_sim_DataRxBuf, g_sim_DataRxLen);
		rxLen = g_sim_DataRxLen;
		
		// ��ջ�����
		memset(g_sim_DataRxBuf, 0, UART6_BUFFSIZE);
		g_sim_DataRxLen = 0;
		
		// ����
		osMutexRelease(SimMutexHandle);
	}
	return rxLen;
}

/**
  * @brief  ��ȡSIM��������״̬
  * @param  
  * @retval true  - �������ӳɹ�
  *         false - ��������ʧ��
  */
bool Sim_GetLinkStatus(void)
{
	bool status = false;
	
	// ����
	osMutexAcquire(SimMutexHandle, osWaitForever);

	status = g_sim_Server.status;
	
	// ����
	osMutexRelease(SimMutexHandle);
	
	return status;
}

/**
  * @brief  SIMģ�鸴λ
  * @param  
  * @retval 
  */
static void Sim_Reset(void)
{
	// SIM״̬�Ƴ�ʼ��
	HAL_GPIO_WritePin(GSM_STA_GPIO_Port, GSM_STA_Pin, GPIO_PIN_RESET);

	// SIMģ�鸴λ
	HAL_GPIO_WritePin(MPCIE_PWR_GPIO_Port, MPCIE_PWR_Pin, GPIO_PIN_SET);
	osDelay(1000);
	HAL_GPIO_WritePin(MPCIE_PWR_GPIO_Port, MPCIE_PWR_Pin, GPIO_PIN_RESET);

	// SIMͨѶ��ʼ��
	Uart_SetRxEnable(&huart6);
}

/**
  * @brief  SIM����
  * @param  
  * @retval 
  */
static bool Sim_Config(void)
{
	u8 cntout = 20;
	while (cntout > 0) {
		// 1.����Ǽ�
		if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CREG], g_sim_AckList[SIM_ACK_CREG], 1000) == true) {
			// 2.��ѯ����ע����Ϣ
			if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CPSI], g_sim_AckList[SIM_ACK_CPSI], 1000) == true) {
				return true;
			}
		}
		cntout--;
	}
	return false;
}

/**
  * @brief  SIM����TCP������
  * @param  
  * @retval 
  */
static bool Sim_Tcp(void)
{
	u8 timeout = 20;
	char tmpReq[50] = { 0 };
	
	while (timeout > 0) {
		// 0.���RF�ź�ǿ��
		if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CSQ], g_sim_AckList[SIM_ACK_CSQ], 1000) == false) {
			// 1.���û��PDP�����ĵ������ļ����
			if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CSOCKSETPN], g_sim_AckList[SIM_ACK_CSOCKSETPN], 1000) == true) {
				// ��ѯ����״̬
				if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_NETOPEN_Q], g_sim_AckList[SIM_ACK_NETOPEN_Q], 1000) == true) {
					// ����TCP������
					sprintf((char *)tmpReq, g_sim_ReqList[SIM_REQ_CIPOPEN], g_sim_Server.ip, g_sim_Server.port);
					if (Sim2_SendReq((char *)tmpReq, g_sim_AckList[SIM_ACK_CIPOPEN], 10000) == true) {
						return true;
					} else {
						osDelay(500);
					}
				} else {
					// ���봫��ģʽ(��ѡ͸�����͸��)
					if (Sim2_SendReq(g_sim_ReqList[SIM_REQ_CIPMODE_1], g_sim_AckList[SIM_ACK_CIPMODE_1], 1000) == true) {
						// ����TCPIP����
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
  * @brief  �ȴ�Ӧ��
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
		// ��������
		ackLen = Uart_GetData(&huart6, g_sim_RxBuf + g_sim_RxLen);	
		if (ackLen > 0) {
			// �����һ�����ַ�
			if ((g_sim_RxBuf[0] == 0) && (ackLen == 1)) {
				ackLen = 0;
				continue;
			}
			
			// ���Դ�ӡ
			if (g_sim_DebugPrintf == true) {
				printf("%s", g_sim_RxBuf + g_sim_RxLen);
			}
			g_sim_RxLen += ackLen;
			
			// ���Ӧ��
			ptr = strstr((char *)g_sim_RxBuf, ackBuf);
			if (ptr != NULL) {
				// Ӧ����ȷ
				ret = true;
				timeout = 0;
			}
			
			ackLen = 0;
			
			// ȥ��һ����\r\n�����ķ���
			if ((g_sim_RxBuf[g_sim_RxLen - 2] == 0x0D) && (g_sim_RxBuf[g_sim_RxLen - 1] == 0x0A)) {
				memset(g_sim_RxBuf, 0, g_sim_RxLen);
				g_sim_RxLen = 0;
			}
		}

		osDelay(1);
	}
	
	// ��λӦ�𻺳���
	memset(g_sim_RxBuf, 0, UART6_BUFFSIZE);
	g_sim_RxLen = 0;
	
	return ret;
}

/**
  * @brief  ����ָ��ȴ�Ӧ��
  * @param  
  * @retval 
  */
static bool Sim_SendReq(char *req, char *ack, u8 timeout)
{
	bool ret = false;
	while (timeout-- > 0) {
		// ����ָ��
		Uart6_Send((u8 *)req, strlen(req));
		
		// �ȴ�Ӧ��1000ms
		if (Sim_WaitAck(ack, 1000) == true) {
			ret = true;
			break;
		}
	}
	return ret;
}


/**
  * @brief  ����ָ��ȴ�Ӧ��
  * @param  
  * @retval 
  */
static bool Sim2_SendReq(char *req, char *ack, u16 timeout)
{
	bool ret = false;
	
	// ����ָ��
	Uart6_Send((u8 *)req, strlen(req));
	
	// �ȴ�Ӧ��1000ms
	if (Sim_WaitAck(ack, timeout) == true) {
		ret = true;
	}
	
	return ret;
}

/**
  * @brief  �ȴ�SIM����
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
