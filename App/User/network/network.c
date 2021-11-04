/* Private includes ----------------------------------------------------------*/
#include "network.h"
#include "ethernet.h"
#include "sim.h"
#include "drivers.h"

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

#define NETWORK_NET_MASK    0X01
#define NETWORK_SIM_MASK    0X02


/* Private variables ---------------------------------------------------------*/

/**
  * ��������״̬
  */
NwkStatus_t g_nwk_status = NETWORK_STATUS_NONE;

/**
  * ������������
  */
uint8_t g_net_Mask = 0;

/**
  * Net���ֵ�����״̬�ź���
  */
extern osSemaphoreId_t NetStatusBinarySemHandle;
extern osSemaphoreId_t SimStatusBinarySemHandle;


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ��ʼ������
  * @param  
  * @retval 
  */
void Network_Init(void)
{

}

/**
  * @brief  �����������
  * @param  
  * @retval 
  */
void Network_Process(void)
{
	if (osSemaphoreAcquire(NetStatusBinarySemHandle, 0) == osOK) {
		if (Ethernet_GetLinkStatus() == true) {
			g_net_Mask |= NETWORK_NET_MASK;
		} else {
			g_net_Mask &= ~NETWORK_NET_MASK;
		}
	}
	
	if (osSemaphoreAcquire(SimStatusBinarySemHandle, 0) == osOK) {
		if (Sim_GetLinkStatus() == true) {
			g_net_Mask |= NETWORK_SIM_MASK;
		} else {
			g_net_Mask &= ~NETWORK_SIM_MASK;
		}
	}

	if (g_net_Mask & NETWORK_STATUS_NET) {
		g_nwk_status = NETWORK_STATUS_NET;
	} else if (g_net_Mask & NETWORK_STATUS_SIM) {
		g_nwk_status = NETWORK_STATUS_SIM;
	} else {
		g_nwk_status = NETWORK_STATUS_NONE;
	}
}

/**
  * @brief  ��ȡ����״̬
  * @param  
  * @retval 
  */
NwkStatus_t Network_GetStatus(void)
{
	return g_nwk_status;
}

/**
  * @brief  ��ȡ����
  * @param  
  * @retval 
  */
uint16_t Network_GetRxData(uint8_t *pBuf)
{
	uint16_t rxLen = 0;

	if (g_nwk_status == NETWORK_STATUS_NET) {

		// ��������
		rxLen = Ethernet_GetRxData(pBuf);

	} else if (g_nwk_status == NETWORK_STATUS_SIM) {

		// ��������
		rxLen = Sim_GetData(pBuf);
	}

	return rxLen;
}

/**
  * @brief  ���緢��
  * @param  
  * @retval 
  */
void Network_Send(uint8_t *pBuffer, uint16_t len)
{
	if (g_nwk_status == NETWORK_STATUS_NET) {
		Ethernet_SetTxData(pBuffer, len);
		DBG("Net tx:\r\n");
	} else if (g_nwk_status == NETWORK_STATUS_SIM) {
		Sim_PutData(pBuffer, len);
		DBG("Sim tx:\r\n");
	} else {
		return ;
	}

	if (len & 0xFF00) {
		DBG("payload is too many.\r\n");
		for (uint16_t i = 0; i < len; i++) {
			DBG(" %02x", pBuffer[i]);
			if ((i % 16 == 15) && (i != len - 1)) {
				DBG("\r\n");
			}
		}
		DBG("\r\n");
	} else {
#if (DBG_TRACE == 1)
		PrintHexBuffer(pBuffer, len);
#endif
	}
}
