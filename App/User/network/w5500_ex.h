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
  * ��ȡ�汾��
  */
#define GET_W5500_VRESION       IINCHIP_READ(VERSIONR)

/**
  * �ж������Ƿ񱻰ε�
  */
#define getPHYCFGR()            IINCHIP_READ(PHYCFGR)

/**
  * DHCP��Ϣ�ṹ��
  */
typedef struct {
    uint8_t mac[6];             // MAC��ַ
    uint8_t lip[4];             // local IP����IP��ַ
    uint8_t sub[4];             // ��������
    uint8_t gw[4];              // ����
    uint8_t dns[4];             // DNS��������ַ
}DhcpConfig_t;

/**
  * DHCP��Ϣ����
  */
extern DhcpConfig_t ConfigDhcpMsg;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ��ʼ������(����)
  */
void        Ethernet_Init(void);

/**
  * @brief  ��ѯ������Ϣ״̬(����)
  */
void        Ethernet_Process(void);

/**
  * @brief  ��ȡ����״̬
  */
bool        Ethernet_GetLinkStatus(void);

/**
  * @brief  ������������(����)
  */
uint16_t    Ethernet_GetRxData(uint8_t *pBuffer);

/**
  * @brief  ���÷�������
  */
void        Ethernet_SetTxData(uint8_t *pBuf, uint16_t len);

/**
  * @brief  ��ʱ����
  */
void        Ethernet_Timer_Handler(void);

#endif /* __ETHERNET_H__ */
