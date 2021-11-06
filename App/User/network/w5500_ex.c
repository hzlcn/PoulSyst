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
  * TCP信息结构体
  */
typedef struct {
    uint8_t serverIP[4];                // 服务端地址
	uint16_t serverPort;                // 服务端端口
	uint16_t clientPort;                // 客户端端口
	bool status;                        // TCP连接成功标志
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
  * 模块电源开关
  */
#define W5500_PWR_ON    HAL_GPIO_WritePin(W5500_ONOFF_GPIO_Port, W5500_ONOFF_Pin, GPIO_PIN_SET)
#define W5500_PWR_OFF   HAL_GPIO_WritePin(W5500_ONOFF_GPIO_Port, W5500_ONOFF_Pin, GPIO_PIN_RESET)

/**
  * 模块复位开关
  */
#define W5500_RST_ON    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET)
#define W5500_RST_OFF   HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET)

/* Private variables ---------------------------------------------------------*/

/**
  * DHCP信息参数
  */
DhcpConfig_t ConfigDhcpMsg = {
    .mac   = { 0 },
    .lip   = { 0 },
    .sub   = { 0 },
    .gw    = { 0 },
    .dns   = { 0 },
};

/**
  * TCP信息参数
  */
Net_TcpServer_t g_net_Server = {
    .serverIP = { 8, 129, 53, 188 },   //{ 8, 129, 53, 188 },  { 192, 168, 2, 92 }
    .serverPort = 9093,	// 0x07E6,	//0x1DB0, // 0x07E5
    .clientPort = 0x1388, 
    .status = false
};

/**
  * 标志
  */
bool g_dhcpStatus = false;              // DHCP成功标志

/**
  * NET数据缓冲区
  */
uint8_t  g_net_TxBuf[4096];
uint8_t  g_net_RxBuf[4096];
uint16_t g_net_TxLen;
uint16_t g_net_RxLen;

/**
  * 定时计数
  */
uint32_t g_net_TimCount = 0xFFFFFFFF;
uint32_t g_net_While1sTime = 0xFFFFFFFF;

/* External variables --------------------------------------------------------*/

/**
  * DHCP信息预存区
  */
extern uint8_t DHCP_allocated_ip[4];    // IP address from DHCP
extern uint8_t DHCP_allocated_gw[4];    // Gateway address from DHCP
extern uint8_t DHCP_allocated_sn[4];    // Subnet mask from DHCP
extern uint8_t DHCP_allocated_dns[4];   // DNS address from DHCP

/**
  * 互斥锁(区分Net线程与其他线程的)
  */
extern osMutexId_t NetTxMutexHandle;
extern osMutexId_t NetRxMutexHandle;
extern osMutexId_t NetStatusMutexHandle;

extern osSemaphoreId_t NetStatusBinarySemHandle;


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  复位网口
  */
static void Ethernet_Reset(void);

/**
  * @brief  设置MAC地址(通过读取唯一标识符)
  */
static void Ethernet_SetMac(uint8_t *pBuffer);

/**
  * @brief  设置DHCP参数
  */
static void Ethernet_SetDhcpConfig(uint8_t *ip, uint8_t *gw, uint8_t *sub, uint8_t *dns);

/**
  * @brief  设置网络参数
  */
static void Ethernet_SetNetwork(void);

/**
  * @brief  初始化网络(网口)
  * @param  
  * @retval 
  */
void Ethernet_Init(void)
{
	// 1.硬件初始化
	W5500_PWR_ON;                       // 电源开启
	Ethernet_Reset();                   // 复位w5500芯片
	
	// 2.设置MAC地址
	Ethernet_SetMac(ConfigDhcpMsg.mac); // 生成MAC地址
	
	// 3.初始化参数
	init_dhcp_client();                 // 初始化DHCP参数
}

/**
  * @brief  处理网络信息状态(网口)
  * @param  
  * @retval 
  */
void Ethernet_Process(void)
{
	if (g_dhcpStatus == false) {
		
		// DHCP超时
		if ((g_net_TimCount == 0xFFFFFFFF) || (g_net_TimCount == 0)) {
			g_net_TimCount = 5000;
			close(SOCK_DHCP);
			init_dhcp_client();  // 初始化DHCP参数
		}
		
		// DHCP获取ip地址
		if (DHCP_run() == DHCP_IP_ASSIGN) {
			
			// 更新参数
			Ethernet_SetDhcpConfig(DHCP_allocated_ip, DHCP_allocated_gw,
                                   DHCP_allocated_sn, DHCP_allocated_dns);
			
			// 将参数写入芯片
			Ethernet_SetNetwork();  // 配置初始化IP信息并打印,初始化8个Socket
			
			g_dhcpStatus = true;
			g_net_TimCount = 0xFFFFFFFF;
		}

	} else {
		
		// 检查硬件连接状态
		if ((getPHYCFGR() & 0x01) == 0x00) {  // 重新连接
			if (g_net_Server.status == true) {
				g_net_Server.status = false;

				osSemaphoreRelease(NetStatusBinarySemHandle);
				
				close(SOCK_TCPC);
			}
		}
		
		// 监听TCP状态
		switch (getSn_SR(SOCK_TCPC)) {                                  // 获取socket0的状态
			
			case SOCK_INIT:                                             // Socket处于初始化完成(打开)状态
				connect(SOCK_TCPC, g_net_Server.serverIP, g_net_Server.serverPort);           // 配置Sn_CR为CONNECT，并向TCP服务器发出连接请求
				break;
			
			case SOCK_ESTABLISHED:                                      // Socket处于连接建立状态		
				if (getSn_IR(SOCK_TCPC) & Sn_IR_CON) {
					setSn_IR(SOCK_TCPC, Sn_IR_CON);                     // 清除接收中断标志位
					DBG("Tcp server is connected!\r\n");
					
					g_net_Server.status = true;
					osSemaphoreRelease(NetStatusBinarySemHandle);

				}
				
				if ((g_net_Server.status == true) && (g_net_RxLen == 0)) {
					/* Receive data from TCP server */
					// 上锁
					osMutexAcquire(NetRxMutexHandle, osWaitForever);
					
					g_net_RxLen = getSn_RX_RSR(SOCK_TCPC);
					if (g_net_RxLen > 0) {
						recv(SOCK_TCPC, g_net_RxBuf, g_net_RxLen);
					}

					// 解锁
					osMutexRelease(NetRxMutexHandle);

				}

				break;
				
			case SOCK_CLOSE_WAIT:                                       // Socket处于等待关闭状态
				close(SOCK_TCPC);
				break;
			
			case SOCK_CLOSED:                                           // Socket处于关闭状态
				socket(SOCK_TCPC, Sn_MR_TCP, g_net_Server.clientPort, Sn_MR_ND);   // 打开Socket0，并配置为TCP无延时模式，打开一个本地端口
				g_net_Server.status = false;
				osSemaphoreRelease(NetStatusBinarySemHandle);

				break;
			
			default:
				break;
		}
		
		if (g_net_Server.status == true) {
			// 上锁
			osMutexAcquire(NetTxMutexHandle, osWaitForever);
			
			if (g_net_TxLen > 0) {
				send(SOCK_TCPC, g_net_TxBuf, g_net_TxLen);
				
				memset(g_net_TxBuf, 0, g_net_TxLen);
				g_net_TxLen = 0;

			}
			
			// 解锁
			osMutexRelease(NetTxMutexHandle);

		}
	}
	
	if ((g_net_While1sTime == 0) || (g_net_While1sTime == 0xFFFFFFFF)) {
		g_net_While1sTime = 1000;
		
//		DBG("net: +1\r\n");
	}
}

/**
  * @brief  获取连接状态
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
  * @brief  接收网络数据(网口)
  * @param  
  * @retval 
  */
uint16_t Ethernet_GetRxData(uint8_t *pBuffer)
{
	uint16_t rxlen = 0;
	
	// 上锁
	if (osMutexAcquire(NetRxMutexHandle, 10) == osOK) {
	
		if (g_net_RxLen > 0) {
			// 读取数据
			memcpy(pBuffer, g_net_RxBuf, g_net_RxLen);
			rxlen = g_net_RxLen;
			
			// 清空缓冲区
			memset(g_net_RxBuf, 0, 4096);
			g_net_RxLen = 0;
		}

		// 解锁
		osMutexRelease(NetRxMutexHandle);
	}

	return rxlen;
}

/**
  * @brief  设置发送数据
  * @param  
  * @retval 
  */
void Ethernet_SetTxData(uint8_t *pBuf, uint16_t len)
{
	// 上锁
	if (osMutexAcquire(NetTxMutexHandle, 10) == osOK) {

		memcpy(g_net_TxBuf, pBuf, len);
		g_net_TxLen = len;
		
		// 解锁
		osMutexRelease(NetTxMutexHandle);
	}

}

/**
  * @brief  定时计数
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
  * @brief  复位网口
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
  * @brief  设置MAC地址(通过读取唯一标识符)
  * @param  pBuffer     MAC存储缓冲区
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
    mac[4]= ChipID[1] >> 8;//这里使用后16位	
    mac[5]= ChipID[1] >> 0; 
	
    if (mac[0] % 2) {    // 第一位为偶数
        mac[0]--;
    }
	
    memcpy(pBuffer, mac, 6);
	DBG("MAC=%02x:%02x:%02x:%02x:%02x:%02x\r\n",pBuffer[0],pBuffer[1],pBuffer[2],pBuffer[3],pBuffer[4],pBuffer[5]);
}

/**
  * @brief  设置DHCP参数
  * @param  ip      ip存储缓冲区
  *         gw      网关存储缓冲区
  *         sub     子网掩码存储缓冲区
  *         dns     dns存储缓冲区
  * @retval 
  */
static void Ethernet_SetDhcpConfig(uint8_t *ip, uint8_t *gw, uint8_t *sub, uint8_t *dns)
{
    /*复制定义的配置信息到配置结构体*/
    memcpy(ConfigDhcpMsg.lip, ip,  4);
    memcpy(ConfigDhcpMsg.gw,  gw,  4);
    memcpy(ConfigDhcpMsg.sub, sub, 4);
    memcpy(ConfigDhcpMsg.dns, dns, 4);	
}

/**
  * @brief  设置网络参数
  * @param  
  * @retval 
  */
static void Ethernet_SetNetwork(void)
{
    uint8_t ip[4];
    uint8_t txsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};   // 选择8个Socket每个Socket发送缓存的大小，在w5500.c的void sysinit()有设置过程
    uint8_t rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};   // 选择8个Socket每个Socket接收缓存的大小，在w5500.c的void sysinit()有设置过程

    setSHAR(ConfigDhcpMsg.mac);
    setSUBR(ConfigDhcpMsg.sub);
    setGAR (ConfigDhcpMsg.gw );
    setSIPR(ConfigDhcpMsg.lip);

    sysinit(txsize, rxsize);                            // 初始化8个socket
    setRTR(2000);                                       // 设置超时时间
    setRCR(3);                                          // 设置最大重新发送次数

    DBG("DHCP info: \r\n");
    getSIPR (ip);
    DBG("ip : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
    getSUBR(ip);
    DBG("sn : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
    getGAR(ip);
    DBG("gw : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
}

