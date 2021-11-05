#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include "common.h"

/********************* w5500 ************************/
#define SOCK_TCPS               0
#define SOCK_HUMTEM             0
#define SOCK_PING               0
#define SOCK_TCPC               0
#define SOCK_UDPS               2
#define SOCK_WEIBO              2
#define SOCK_DHCP               3
#define SOCK_HTTPS              4
#define SOCK_DNS                5
#define SOCK_SMTP               6
#define SOCK_NTP                7
/****************************************************/

/**
  * 读取版本号
  */
#define GET_W5500_VRESION       IINCHIP_READ(VERSIONR)

/**
  * 判断网线是否被拔掉
  */
#define getPHYCFGR()            IINCHIP_READ(PHYCFGR)

/**
  * DHCP信息结构体
  */
typedef struct {
    uint8_t mac[6];             // MAC地址
    uint8_t lip[4];             // local IP本地IP地址
    uint8_t sub[4];             // 子网掩码
    uint8_t gw[4];              // 网关
    uint8_t dns[4];             // DNS服务器地址
}DhcpConfig_t;

/**
  * DHCP信息参数
  */
extern DhcpConfig_t ConfigDhcpMsg;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  初始化网络(网口)
  */
void        Ethernet_Init(void);

/**
  * @brief  轮询网络信息状态(网口)
  */
void        Ethernet_Process(void);

/**
  * @brief  获取连接状态
  */
bool        Ethernet_GetLinkStatus(void);

/**
  * @brief  接收网络数据(网口)
  */
uint16_t    Ethernet_GetRxData(uint8_t *pBuffer);

/**
  * @brief  设置发送数据
  */
void        Ethernet_SetTxData(uint8_t *pBuf, uint16_t len);

/**
  * @brief  定时计数
  */
void        Ethernet_Timer_Handler(void);

#endif /* __ETHERNET_H__ */
