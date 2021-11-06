/**
  ******************************************************************************
  * @file           : ethernet.c
  * @brief          : Connect network by w5500
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* Private includes ----------------------------------------------------------*/


#include "w5500_ex.h"
#include "w5500.h"
#include "dhcp.h"
#include "socket.h"
#include "drivers.h"

/* Private typedef -----------------------------------------------------------*/

/**
  * TCP��Ϣ�ṹ��
  */
typedef struct {
    uint8_t serverIP[4];                // ����˵�ַ
	uint16_t serverPort;                // ����˶˿�
	uint16_t clientPort;                // �ͻ��˶˿�
	bool status;                        // TCP���ӳɹ���־
}Net_TcpServer_t;

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
  * ģ���Դ����
  */
#define W5500_PWR_ON    HAL_GPIO_WritePin(W5500_ONOFF_GPIO_Port, W5500_ONOFF_Pin, GPIO_PIN_SET)
#define W5500_PWR_OFF   HAL_GPIO_WritePin(W5500_ONOFF_GPIO_Port, W5500_ONOFF_Pin, GPIO_PIN_RESET)

/**
  * ģ�鸴λ����
  */
#define W5500_RST_ON    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET)
#define W5500_RST_OFF   HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET)

/* Private variables ---------------------------------------------------------*/

/**
  * DHCP��Ϣ����
  */
DhcpConfig_t ConfigDhcpMsg = {
    .mac   = { 0 },
    .lip   = { 0 },
    .sub   = { 0 },
    .gw    = { 0 },
    .dns   = { 0 },
};

/**
  * TCP��Ϣ����
  */
Net_TcpServer_t g_net_Server = {
    .serverIP = { 8, 129, 53, 188 },   //{ 8, 129, 53, 188 },  { 192, 168, 2, 92 }
    .serverPort = 9093,	// 0x07E6,	//0x1DB0, // 0x07E5
    .clientPort = 0x1388, 
    .status = false
};

/**
  * ��־
  */
bool g_dhcpStatus = false;              // DHCP�ɹ���־

/**
  * NET���ݻ�����
  */
uint8_t  g_net_TxBuf[4096];
uint8_t  g_net_RxBuf[4096];
uint16_t g_net_TxLen;
uint16_t g_net_RxLen;

/**
  * ��ʱ����
  */
uint32_t g_net_TimCount = 0xFFFFFFFF;
uint32_t g_net_While1sTime = 0xFFFFFFFF;

/* External variables --------------------------------------------------------*/

/**
  * DHCP��ϢԤ����
  */
extern uint8_t DHCP_allocated_ip[4];    // IP address from DHCP
extern uint8_t DHCP_allocated_gw[4];    // Gateway address from DHCP
extern uint8_t DHCP_allocated_sn[4];    // Subnet mask from DHCP
extern uint8_t DHCP_allocated_dns[4];   // DNS address from DHCP

/**
  * ������(����Net�߳��������̵߳�)
  */
extern osMutexId_t NetTxMutexHandle;
extern osMutexId_t NetRxMutexHandle;
extern osMutexId_t NetStatusMutexHandle;

extern osSemaphoreId_t NetStatusBinarySemHandle;


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ��λ����
  */
static void Ethernet_Reset(void);

/**
  * @brief  ����MAC��ַ(ͨ����ȡΨһ��ʶ��)
  */
static void Ethernet_SetMac(uint8_t *pBuffer);

/**
  * @brief  ����DHCP����
  */
static void Ethernet_SetDhcpConfig(uint8_t *ip, uint8_t *gw, uint8_t *sub, uint8_t *dns);

/**
  * @brief  �����������
  */
static void Ethernet_SetNetwork(void);

/**
  * @brief  ��ʼ������(����)
  * @param  
  * @retval 
  */
void Ethernet_Init(void)
{
	// 1.Ӳ����ʼ��
	W5500_PWR_ON;                       // ��Դ����
	Ethernet_Reset();                   // ��λw5500оƬ
	
	// 2.����MAC��ַ
	Ethernet_SetMac(ConfigDhcpMsg.mac); // ����MAC��ַ
	
	// 3.��ʼ������
	init_dhcp_client();                 // ��ʼ��DHCP����
}

/**
  * @brief  ����������Ϣ״̬(����)
  * @param  
  * @retval 
  */
void Ethernet_Process(void)
{
	if (g_dhcpStatus == false) {
		
		// DHCP��ʱ
		if ((g_net_TimCount == 0xFFFFFFFF) || (g_net_TimCount == 0)) {
			g_net_TimCount = 5000;
			close(SOCK_DHCP);
			init_dhcp_client();  // ��ʼ��DHCP����
		}
		
		// DHCP��ȡip��ַ
		if (DHCP_run() == DHCP_IP_ASSIGN) {
			
			// ���²���
			Ethernet_SetDhcpConfig(DHCP_allocated_ip, DHCP_allocated_gw,
                                   DHCP_allocated_sn, DHCP_allocated_dns);
			
			// ������д��оƬ
			Ethernet_SetNetwork();  // ���ó�ʼ��IP��Ϣ����ӡ,��ʼ��8��Socket
			
			g_dhcpStatus = true;
			g_net_TimCount = 0xFFFFFFFF;
		}

	} else {
		
		// ���Ӳ������״̬
		if ((getPHYCFGR() & 0x01) == 0x00) {  // ��������
			if (g_net_Server.status == true) {
				g_net_Server.status = false;

				osSemaphoreRelease(NetStatusBinarySemHandle);
				
				close(SOCK_TCPC);
			}
		}
		
		// ����TCP״̬
		switch (getSn_SR(SOCK_TCPC)) {                                  // ��ȡsocket0��״̬
			
			case SOCK_INIT:                                             // Socket���ڳ�ʼ�����(��)״̬
				connect(SOCK_TCPC, g_net_Server.serverIP, g_net_Server.serverPort);           // ����Sn_CRΪCONNECT������TCP������������������
				break;
			
			case SOCK_ESTABLISHED:                                      // Socket�������ӽ���״̬		
				if (getSn_IR(SOCK_TCPC) & Sn_IR_CON) {
					setSn_IR(SOCK_TCPC, Sn_IR_CON);                     // ��������жϱ�־λ
					DBG("Tcp server is connected!\r\n");
					
					g_net_Server.status = true;
					osSemaphoreRelease(NetStatusBinarySemHandle);

				}
				
				if ((g_net_Server.status == true) && (g_net_RxLen == 0)) {
					/* Receive data from TCP server */
					// ����
					osMutexAcquire(NetRxMutexHandle, osWaitForever);
					
					g_net_RxLen = getSn_RX_RSR(SOCK_TCPC);
					if (g_net_RxLen > 0) {
						recv(SOCK_TCPC, g_net_RxBuf, g_net_RxLen);
					}

					// ����
					osMutexRelease(NetRxMutexHandle);

				}

				break;
				
			case SOCK_CLOSE_WAIT:                                       // Socket���ڵȴ��ر�״̬
				close(SOCK_TCPC);
				break;
			
			case SOCK_CLOSED:                                           // Socket���ڹر�״̬
				socket(SOCK_TCPC, Sn_MR_TCP, g_net_Server.clientPort, Sn_MR_ND);   // ��Socket0��������ΪTCP����ʱģʽ����һ�����ض˿�
				g_net_Server.status = false;
				osSemaphoreRelease(NetStatusBinarySemHandle);

				break;
			
			default:
				break;
		}
		
		if (g_net_Server.status == true) {
			// ����
			osMutexAcquire(NetTxMutexHandle, osWaitForever);
			
			if (g_net_TxLen > 0) {
				send(SOCK_TCPC, g_net_TxBuf, g_net_TxLen);
				
				memset(g_net_TxBuf, 0, g_net_TxLen);
				g_net_TxLen = 0;

			}
			
			// ����
			osMutexRelease(NetTxMutexHandle);

		}
	}
	
	if ((g_net_While1sTime == 0) || (g_net_While1sTime == 0xFFFFFFFF)) {
		g_net_While1sTime = 1000;
		
//		DBG("net: +1\r\n");
	}
}

/**
  * @brief  ��ȡ����״̬
  * @param  
  * @retval 
  */
bool Ethernet_GetLinkStatus(void)
{
	bool status = false;
	
	status = g_net_Server.status;
		
	return status;
}

/**
  * @brief  ������������(����)
  * @param  
  * @retval 
  */
uint16_t Ethernet_GetRxData(uint8_t *pBuffer)
{
	uint16_t rxlen = 0;
	
	// ����
	if (osMutexAcquire(NetRxMutexHandle, 10) == osOK) {
	
		if (g_net_RxLen > 0) {
			// ��ȡ����
			memcpy(pBuffer, g_net_RxBuf, g_net_RxLen);
			rxlen = g_net_RxLen;
			
			// ��ջ�����
			memset(g_net_RxBuf, 0, 4096);
			g_net_RxLen = 0;
		}

		// ����
		osMutexRelease(NetRxMutexHandle);
	}

	return rxlen;
}

/**
  * @brief  ���÷�������
  * @param  
  * @retval 
  */
void Ethernet_SetTxData(uint8_t *pBuf, uint16_t len)
{
	// ����
	if (osMutexAcquire(NetTxMutexHandle, 10) == osOK) {

		memcpy(g_net_TxBuf, pBuf, len);
		g_net_TxLen = len;
		
		// ����
		osMutexRelease(NetTxMutexHandle);
	}

}

/**
  * @brief  ��ʱ����
  * @param  
  * @retval 
  */
void Ethernet_Timer_Handler(void)
{
	if ((g_net_TimCount != 0xFFFFFFFF) && (g_net_TimCount != 0)) {
		g_net_TimCount--;
	}
	
	if ((g_net_While1sTime > 0) && (g_net_While1sTime < 0xFFFFFFFF)) {
		g_net_While1sTime--;
	}
}

/**
  * @brief  ��λ����
  * @param  
  * @retval 
  */
static void Ethernet_Reset(void)
{
    HAL_GPIO_WritePin( W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET );
	osDelay( 2 );  
    HAL_GPIO_WritePin( W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET );
	osDelay( 1600 );
}

/**
  * @brief  ����MAC��ַ(ͨ����ȡΨһ��ʶ��)
  * @param  pBuffer     MAC�洢������
  * @retval 
  */
static void Ethernet_SetMac(uint8_t *pBuffer)
{
    uint32_t ChipID[3];
    uint8_t mac[6] = {0};

	ChipID[0] = *(volatile uint32_t*)(0x1FFF7A18);
	ChipID[1] = *(volatile uint32_t*)(0x1FFF7A14);
	ChipID[2] = *(volatile uint32_t*)(0x1FFF7A10);
	
    mac[0]= ChipID[0] >> 24;
    mac[1]= ChipID[0] >> 16;
    mac[2]= ChipID[0] >> 8;	
    mac[3]= ChipID[0] >> 0;	
    mac[4]= ChipID[1] >> 8;//����ʹ�ú�16λ	
    mac[5]= ChipID[1] >> 0; 
	
    if (mac[0] % 2) {    // ��һλΪż��
        mac[0]--;
    }
	
    memcpy(pBuffer, mac, 6);
	DBG("MAC=%02x:%02x:%02x:%02x:%02x:%02x\r\n",pBuffer[0],pBuffer[1],pBuffer[2],pBuffer[3],pBuffer[4],pBuffer[5]);
}

/**
  * @brief  ����DHCP����
  * @param  ip      ip�洢������
  *         gw      ���ش洢������
  *         sub     ��������洢������
  *         dns     dns�洢������
  * @retval 
  */
static void Ethernet_SetDhcpConfig(uint8_t *ip, uint8_t *gw, uint8_t *sub, uint8_t *dns)
{
    /*���ƶ����������Ϣ�����ýṹ��*/
    memcpy(ConfigDhcpMsg.lip, ip,  4);
    memcpy(ConfigDhcpMsg.gw,  gw,  4);
    memcpy(ConfigDhcpMsg.sub, sub, 4);
    memcpy(ConfigDhcpMsg.dns, dns, 4);	
}

/**
  * @brief  �����������
  * @param  
  * @retval 
  */
static void Ethernet_SetNetwork(void)
{
    uint8_t ip[4];
    uint8_t txsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};   // ѡ��8��Socketÿ��Socket���ͻ���Ĵ�С����w5500.c��void sysinit()�����ù���
    uint8_t rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};   // ѡ��8��Socketÿ��Socket���ջ���Ĵ�С����w5500.c��void sysinit()�����ù���

    setSHAR(ConfigDhcpMsg.mac);
    setSUBR(ConfigDhcpMsg.sub);
    setGAR (ConfigDhcpMsg.gw );
    setSIPR(ConfigDhcpMsg.lip);

    sysinit(txsize, rxsize);                            // ��ʼ��8��socket
    setRTR(2000);                                       // ���ó�ʱʱ��
    setRCR(3);                                          // ����������·��ʹ���

    DBG("DHCP info: \r\n");
    getSIPR (ip);
    DBG("ip : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
    getSUBR(ip);
    DBG("sn : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
    getGAR(ip);
    DBG("gw : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
}

