#ifndef __NETWORK_H__
#define __NETWORK_H__

/* Private includes ----------------------------------------------------------*/

#include "Types.h"
#include "common.h"

/* Private typedef -----------------------------------------------------------*/
/**
  * ����״̬
  */
typedef enum {
	NETWORK_STATUS_NONE,
	NETWORK_STATUS_NET,
	NETWORK_STATUS_SIM,
}NwkStatus_t;

extern NwkStatus_t g_nwk_status;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ��ʼ������
  */
void Network_Init(void);

/**
  * @brief  �����������
  */
void Network_Process(void);

/**
  * @brief  ��ȡ����״̬
  */
NwkStatus_t Network_GetStatus(void);

/**
  * @brief  ��ȡ����
  */
uint16_t Network_GetRxData(uint8_t *pBuf);

/**
  * @brief  ���緢��
  */
void Network_Send(uint8_t *pBuffer, uint16_t len);

#endif //__NETWORK_H__
