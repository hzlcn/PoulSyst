#ifndef __NETWORK_H__
#define __NETWORK_H__

/* Private includes ----------------------------------------------------------*/

#include "Types.h"
#include "common.h"

/* Private typedef -----------------------------------------------------------*/
/**
  * 网络状态
  */
typedef enum {
	NETWORK_STATUS_NONE,
	NETWORK_STATUS_NET,
	NETWORK_STATUS_SIM,
}NwkStatus_t;

extern NwkStatus_t g_nwk_status;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  初始化网络
  */
void Network_Init(void);

/**
  * @brief  检查网络连接
  */
void Network_Process(void);

/**
  * @brief  获取网络状态
  */
NwkStatus_t Network_GetStatus(void);

/**
  * @brief  获取数据
  */
uint16_t Network_GetRxData(uint8_t *pBuf);

/**
  * @brief  网络发送
  */
void Network_Send(uint8_t *pBuffer, uint16_t len);

#endif //__NETWORK_H__
