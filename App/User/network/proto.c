/* Includes ------------------------------------------------------------------*/
#include "crc.h"
#include "apps.h"
#include "poultry.h"
#include "communicate.h"
#include "gps.h"
#include "route.h"
#include "param.h"
#include "drivers.h"
#include "store.h"
#include "usermsg.h"
#include "upgrade.h"
#include "dataStruct.h"

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


/* Private variables ---------------------------------------------------------*/
u8 g_protoTxBuf[1024] = { 0 };
u8 g_proRxBuf[1024] = { 0 };
u8 *g_pTxPayload = g_protoTxBuf + 11;
u8 *g_pRxPayload = g_proRxBuf + 11;

extern bool g_isServerJoined;


/* Private function prototypes -----------------------------------------------*/

static int Proto_PrepareFrame(u8 type, int txLen)
{
	u16 calcCrc = 0;
	
	// ��ջ�����
	memset(g_protoTxBuf, 0, 1024);
	
	// 1.֡ͷ
	g_protoTxBuf[0] = 0x88;
	g_protoTxBuf[1] = 0xFB;
	
	// 2.���ȣ��� ָ������ �� CRCУ�� ��������֡ͷ֡β�ͳ�����6���ֽڣ�
	g_protoTxBuf[2] = (txLen + 9) >> 8;
	g_protoTxBuf[3] = txLen + 9;
	
	// 3.ָ������
	g_protoTxBuf[4] = type;

	// 4.�豸��ַ
	uint8_t *devAddr = (uint8_t *)Store_GetConfig()->devAddr;
	memcpy(g_protoTxBuf + 5, devAddr, sizeof(DevAddr_t));

	// 5.У��
	calcCrc = CalCrc16(g_protoTxBuf + 2, txLen + 9, 0xFFFF);
	g_protoTxBuf[txLen + 11] = (calcCrc & 0xFF00) >> 8;
	g_protoTxBuf[txLen + 12] = (calcCrc & 0x00FF) >> 0;
	
	// 6.֡β
	g_protoTxBuf[txLen + 13] = 0xFC;
	g_protoTxBuf[txLen + 14] = 0xFC;
	
	return txLen;
}

int Proto_ParseFrame(u8 type, int rxLen)
{
	u16 calcCrc = 0, recvCrc = 0;
	u8 *devAddr = Store_GetConfig()->devAddr;

	// 1.����֡ͷ��֡β�Ƿ���ȷ
	if ((g_proRxBuf[0] != 0x88) || 
		(g_proRxBuf[1] != 0xFB) || 
		(g_proRxBuf[rxLen-1] != 0xFC) || 
		(g_proRxBuf[rxLen-2] != 0xFC)) 
	{
		DBG("Frame head or fail is wrong!\r\n");
		return -1;
	}
	
	// 2.����CRC�Ƿ���ȷ
	calcCrc = CalCrc16(g_proRxBuf + 2, rxLen - 6, 0xFFFF);
	recvCrc = ((g_proRxBuf[rxLen - 4] << 8) | g_proRxBuf[rxLen - 3]);
	if (calcCrc != recvCrc) {
		DBG("CRC error: calc crc: %04x, recv crc: %04x\r\n", calcCrc, recvCrc);
		return -2;
	}
	
	// 4.���ָ���Ƿ��Ӧ
	if (type != g_proRxBuf[4]) {
		DBG("Cmd is different!\r\n");
		return -3;
	}
	
	// 3.�����豸��ַ�Ƿ���ȷ
	if (memcmp(devAddr, g_proRxBuf + 5, 6) != 0) {
		return -4;
	}

	return 0;
}

/**
  * @brief  �豸��¼������
  * @param  timeout	��¼��ʱʱ��
  * @retval 
  */
int Proto_Join(u32 timeout)
{
	int payloadLen = 0;
	int txLen = 0;
	u16 rxLen = 0;
	
	Version_t *version = &Store_GetConfig()->version;
	RTCTime_t rtcTime = { 0 };
	
	// ��Ӳ���汾
	g_pTxPayload[payloadLen++] = version->soft;
	g_pTxPayload[payloadLen++] = version->hard;

	// ϵͳʱ��
//	data[g_CMNC_TxLen++] = rtcTime->rtc.year;
//	data[g_CMNC_TxLen++] = rtcTime->rtc.month;
//	data[g_CMNC_TxLen++] = rtcTime->rtc.day;
//	data[g_CMNC_TxLen++] = rtcTime->rtc.hour;
	
	// �û���Ϣ��汾
	MercBase_t *mercBase = User_GetInfo()->mercBase;
	g_pTxPayload[payloadLen++] = mercBase->Version[0];
	g_pTxPayload[payloadLen++] = mercBase->Version[1];

	// ʳƷ��Ϣ��汾
	PoulBase_t *poulBase = Poul_GetInfo()->base[0];
	g_pTxPayload[payloadLen++] = poulBase->version[0];
	g_pTxPayload[payloadLen++] = poulBase->version[1];

	// ��ǩ
	memset(g_pTxPayload + payloadLen, 0x31, 20);
	payloadLen += 20;
	
	// GPS��Ϣ
	GPS_GetLLData(g_pTxPayload + payloadLen);
	payloadLen += 20;

	txLen = Proto_PrepareFrame(UL_DEVICE_JOIN, payloadLen);
	Network_Send(g_protoTxBuf, txLen);
	
	// �ȴ���������
	while (timeout--) {
		rxLen = Network_GetRxData(g_proRxBuf);
		if (rxLen == 0) {
			osDelay(1);
			continue;
		}
		
		if (Proto_ParseFrame(UL_DEVICE_JOIN, rxLen) == 0) {
			
			if (g_pRxPayload[0] == 0x11) {
				// �豸ʹ��λ�ò���ȷ��GPSλ�ò���ȷ
			} else if (g_pRxPayload[0] == 0x12) {
				// �豸������
			} else {
				g_isServerJoined = true;
			}
			
			// ���²���
			if ((g_pRxPayload[1] & 0x01) == 0x01) {
				CMNC_AddCmd(UL_USERMSG_UPDATE);
			}
			if ((g_pRxPayload[1] & 0x02) == 0x02) {
				CMNC_AddCmd(UL_POULTYPE_UPDATE);
			}
			if ((g_pRxPayload[1] & 0x04) == 0x04) {
				CMNC_AddCmd(UL_FIRMWARE_UPDATE);
			}
			
			// У��ϵͳʱ��
			memcpy(rtcTime.value, g_pRxPayload + 2, 6);
			RTCTime_SetTime(rtcTime.rtc.year, rtcTime.rtc.month,  rtcTime.rtc.day, 
				 rtcTime.rtc.hour, rtcTime.rtc.minute, rtcTime.rtc.second);
			
			DBG("################## Join device #################\r\n");
			DBG("func flag    : %02x %02x\r\n", g_pRxPayload[0], g_pRxPayload[1]);
			DBG("system time  : %02d-%02d-%02d %02d:%02d:%02d\r\n", rtcTime.rtc.year + 2000, rtcTime.rtc.month, 
				rtcTime.rtc.day, rtcTime.rtc.hour, rtcTime.rtc.minute, rtcTime.rtc.second);

			if (g_pRxPayload[0] != 0) {
				DBG("device link error.\r\n");
			}
			
			return 0;
		}
		
		
	}
	return -10;
}

int Proto_ReadUserBase(u32 timeout)
{
	int payloadLen = 0;
	int txLen = 0;
	u16 rxLen = 0;
	u8 i = 0, j = 0;

	MercBase_t *mercBase = User_GetInfo()->mercBase;
	
	// �û���Ϣ��汾
	g_pTxPayload[payloadLen++] = mercBase->Version[0];
	g_pTxPayload[payloadLen++] = mercBase->Version[1];

	txLen = Proto_PrepareFrame(UL_USERMSG_UPDATE, payloadLen);
	Network_Send(g_protoTxBuf, txLen);
	
	// �ȴ���������
	while (timeout--) {
		rxLen = Network_GetRxData(g_proRxBuf);
		if (rxLen == 0) {
			osDelay(1);
			continue;
		}
		
		if (Proto_ParseFrame(UL_USERMSG_UPDATE, rxLen) == 0) {			
			// ������ݳ���
			if ((payLen - 2) / 20 > 30) {
				// ���ݳ���
				DBG("user base is out of range.\r\n");
			}
			
			// �û���Ϣ�����
			mercBase->Version[0] = g_pRxPayload[0];
			mercBase->Version[1] = g_pRxPayload[1];
			mercBase->Sum = (rxLen - 15 - 2) / MERCHANT_MESSAGE_SIZE;
			for (j = 0; j < mercBase->Sum; j++) {
				mercBase->List.Info[j].perm = (UserPerm_t)g_pRxPayload[2 + j * MERCHANT_MESSAGE_SIZE];
				memcpy(mercBase->List.Info[j].code, g_pRxPayload + 3 + j * MERCHANT_MESSAGE_SIZE, USER_CODE_SIZE);
			}
			
			Store_SaveMercBase();
			
			DBG("################ Updata userbase ###############\r\n");
			DBG("user base ver: %02x %02x\r\n", g_pRxPayload[0], g_pRxPayload[1]);
			for (j = 0; j < (rxLen - 15 - 2) / MERCHANT_MESSAGE_SIZE; j++) {
				DBG("list [%d]    : ", j);
				for (i = 0; i < MERCHANT_MESSAGE_SIZE; i++) {
					if (i == 0) {
						DBG("(lv)");
					} else if (i == 1) {
						DBG("(id)");
					} else if (i == 1 + USER_CODE_SIZE) {
						DBG("(res)");
					}
					DBG("%02x ", g_pRxPayload[j * MERCHANT_MESSAGE_SIZE + 2 + i]);
				}
				DBG("\r\n");
			}
			return 0;
		}
		
		
	}
	return -10;
}

int Proto_ReadPoulBase(u32 timeout)
{
	int payloadLen = 0;
	int txLen = 0;
	u16 rxLen = 0;
	u8 i = 0, j = 0;

	PoulBase_t *poulBase = Poul_GetInfo()->base[0];
	
	// ʳƷ��Ϣ��汾
	g_pTxPayload[payloadLen++] = poulBase->version[0];
	g_pTxPayload[payloadLen++] = poulBase->version[1];

	txLen = Proto_PrepareFrame(UL_POULTYPE_UPDATE, payloadLen);
	Network_Send(g_protoTxBuf, txLen);
	
	// �ȴ���������
	while (timeout--) {
		rxLen = Network_GetRxData(g_proRxBuf);
		if (rxLen == 0) {
			osDelay(1);
			continue;
		}
		
		if (Proto_ParseFrame(UL_POULTYPE_UPDATE, rxLen) == 0) {			
			// ����������Ϣ��
			poulBase = Poul_GetInfo()->base[0];
			poulBase->version[0] = g_pRxPayload[0];
			poulBase->version[1] = g_pRxPayload[1];
			poulBase->sum = (rxLen - 15 - 17) / 16;
			for (i = 0; i < poulBase->sum; i++) {
				memcpy(poulBase->kinds[i].kindNum, g_pRxPayload + 2 + i * 16, 6);
				memcpy(poulBase->kinds[i].name, g_pRxPayload + 8 + i * 16, 10);
			}
			Store_SavePoulBase();
			
			DBG("################ Updata poulbase ###############\r\n");
			DBG("poul base ver: %02x %02x\r\n", g_pRxPayload[0], g_pRxPayload[1]);
			for (i = 0; i < poulBase->sum; i++) {
				DBG("list [%02d]    : ", i);
				for (j = 0; j < 16; j++) {
					if (j == 0) {
						DBG("(class)");
					} else if (j == 2) {
						DBG("(group)");
					} else if (j == 6) {
						DBG("(name_gbk)");
					}
					DBG("%02x ", g_pRxPayload[i * 16 + 2 + j]);
				}
				DBG("\r\n");
			}

			return 0;
		}
		
		
	}
	return -10;
}

int Proto_LoginUser(u32 timeout)
{
	int payloadLen = 0;
	int txLen = 0;
	u16 rxLen = 0;
	u8 i = 0, j = 0;

	PoulBase_t *poulBase = Poul_GetInfo()->base[0];
	
	// �û���Ϣ(������Ҫ��¼���ͻ�����Ҫ��¼)
	memcpy(g_pTxPayload + payloadLen, User_GetInfo()->recv.code, USER_CODE_SIZE);
	payloadLen += USER_CODE_SIZE;

	txLen = Proto_PrepareFrame(UL_USER_LOGIN, payloadLen);
	Network_Send(g_protoTxBuf, txLen);
	
	// �ȴ���������
	while (timeout--) {
		rxLen = Network_GetRxData(g_proRxBuf);
		if (rxLen == 0) {
			osDelay(1);
			continue;
		}
		
		if (Proto_ParseFrame(UL_USER_LOGIN, rxLen) == 0) {
			if (g_pRxPayload[0] == 0x00) {	// ���û�
				User_SetCode(USER_PERM_NULL);
			} else if (g_pRxPayload[0] == 0x01) {	// ��ͨ����
				User_SetCode(USER_PERM_MERCHANT);
			} else if (g_pRxPayload[0] == 0x02) {	// ����Ա
				User_SetCode(USER_PERM_ADMIN);
			} else if (g_pRxPayload[0] == 0x03) {	// ά��Ա
				User_SetCode(USER_PERM_MAINTAIN);
			} else if (g_pRxPayload[0] == 0x04) {	// ��ͨ�ͻ�
				User_SetCode(USER_PERM_CUSTOMER);
				User_SetCust(g_pRxPayload + 1);
			}
			
			DBG("################## User login ##################\r\n");
			if ((g_pRxPayload[0] == 0x01) || (g_pRxPayload[0] == 0x02) || (g_pRxPayload[0] == 0x03) || (g_pRxPayload[0] == 0x04)) {
				DBG("Login success.\r\n");
			} else {
				DBG("Login fail.\r\n");
			}

			return 0;
		}
		
		
	}
	return -10;
}

int Proto_HeartBeat(u32 timeout)
{
	int payloadLen = 0;
	int txLen = 0;
	u16 rxLen = 0;
	u8 i = 0, j = 0;
	u16 status = 0;

	uint8_t errorstatus = 0;
	ModbusInfo_t *modbusInfo = Modbus_GetParam();
	u16 valibNum = modbusInfo->regList[MODBUS_REG_VALIB_LABEL].data;
	uint16_t remainnumb = LABEL_TOTAL_NUMBER - valibNum - modbusInfo->regList[MODBUS_REG_INVAL_LABEL].data;
	ErrorStatus_t *error = Param_GetErrorStatus();
	errorstatus = (*error == true) ? 0x23 : 0;
	if (remainnumb <= 1) {
		if (errorstatus == 0) {
			errorstatus = 0x24;
		}
	}

	// ����״̬
	g_pTxPayload[payloadLen++] = errorstatus;
	
	// ʣ���ǩ
	g_pTxPayload[payloadLen++] = ( remainnumb & 0xFF00 ) >> 8;
	g_pTxPayload[payloadLen++] = ( remainnumb & 0x00FF ) >> 0;

	txLen = Proto_PrepareFrame(UL_HEARTBEAT, payloadLen);
	Network_Send(g_protoTxBuf, txLen);
	
	// �ȴ���������
	while (timeout--) {
		rxLen = Network_GetRxData(g_proRxBuf);
		if (rxLen == 0) {
			osDelay(1);
			continue;
		}
		
		if (Proto_ParseFrame(UL_HEARTBEAT, rxLen) == 0) {
			status = g_pRxPayload[0] << 8 | g_pRxPayload[1];
			if (status == 0x0003) {
				CMNC_AddCmd(UL_POULTYPE_UPDATE);
			}

			return 0;
		}
	}
	return -10;
}

