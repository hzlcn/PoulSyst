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
	
	// 清空缓冲区
	memset(g_protoTxBuf, 0, 1024);
	
	// 1.帧头
	g_protoTxBuf[0] = 0x88;
	g_protoTxBuf[1] = 0xFB;
	
	// 2.长度（从 指令类型 到 CRC校验 ，不包括帧头帧尾和长度这6个字节）
	g_protoTxBuf[2] = (txLen + 9) >> 8;
	g_protoTxBuf[3] = txLen + 9;
	
	// 3.指令类型
	g_protoTxBuf[4] = type;

	// 4.设备地址
	uint8_t *devAddr = (uint8_t *)Store_GetConfig()->devAddr;
	memcpy(g_protoTxBuf + 5, devAddr, sizeof(DevAddr_t));

	// 5.校验
	calcCrc = CalCrc16(g_protoTxBuf + 2, txLen + 9, 0xFFFF);
	g_protoTxBuf[txLen + 11] = (calcCrc & 0xFF00) >> 8;
	g_protoTxBuf[txLen + 12] = (calcCrc & 0x00FF) >> 0;
	
	// 6.帧尾
	g_protoTxBuf[txLen + 13] = 0xFC;
	g_protoTxBuf[txLen + 14] = 0xFC;
	
	return txLen;
}

int Proto_ParseFrame(u8 type, int rxLen)
{
	u16 calcCrc = 0, recvCrc = 0;
	u8 *devAddr = Store_GetConfig()->devAddr;

	// 1.检验帧头或帧尾是否正确
	if ((g_proRxBuf[0] != 0x88) || 
		(g_proRxBuf[1] != 0xFB) || 
		(g_proRxBuf[rxLen-1] != 0xFC) || 
		(g_proRxBuf[rxLen-2] != 0xFC)) 
	{
		DBG("Frame head or fail is wrong!\r\n");
		return -1;
	}
	
	// 2.检验CRC是否正确
	calcCrc = CalCrc16(g_proRxBuf + 2, rxLen - 6, 0xFFFF);
	recvCrc = ((g_proRxBuf[rxLen - 4] << 8) | g_proRxBuf[rxLen - 3]);
	if (calcCrc != recvCrc) {
		DBG("CRC error: calc crc: %04x, recv crc: %04x\r\n", calcCrc, recvCrc);
		return -2;
	}
	
	// 4.检查指令是否对应
	if (type != g_proRxBuf[4]) {
		DBG("Cmd is different!\r\n");
		return -3;
	}
	
	// 3.检验设备地址是否正确
	if (memcmp(devAddr, g_proRxBuf + 5, 6) != 0) {
		return -4;
	}

	return 0;
}

/**
  * @brief  设备登录服务器
  * @param  timeout	登录超时时间
  * @retval 
  */
int Proto_Join(u32 timeout)
{
	int payloadLen = 0;
	int txLen = 0;
	u16 rxLen = 0;
	
	Version_t *version = &Store_GetConfig()->version;
	RTCTime_t rtcTime = { 0 };
	
	// 软硬件版本
	g_pTxPayload[payloadLen++] = version->soft;
	g_pTxPayload[payloadLen++] = version->hard;

	// 系统时间
//	data[g_CMNC_TxLen++] = rtcTime->rtc.year;
//	data[g_CMNC_TxLen++] = rtcTime->rtc.month;
//	data[g_CMNC_TxLen++] = rtcTime->rtc.day;
//	data[g_CMNC_TxLen++] = rtcTime->rtc.hour;
	
	// 用户信息库版本
	MercBase_t *mercBase = User_GetInfo()->mercBase;
	g_pTxPayload[payloadLen++] = mercBase->Version[0];
	g_pTxPayload[payloadLen++] = mercBase->Version[1];

	// 食品信息库版本
	PoulBase_t *poulBase = Poul_GetInfo()->base[0];
	g_pTxPayload[payloadLen++] = poulBase->version[0];
	g_pTxPayload[payloadLen++] = poulBase->version[1];

	// 标签
	memset(g_pTxPayload + payloadLen, 0x31, 20);
	payloadLen += 20;
	
	// GPS信息
	GPS_GetLLData(g_pTxPayload + payloadLen);
	payloadLen += 20;

	txLen = Proto_PrepareFrame(UL_DEVICE_JOIN, payloadLen);
	Network_Send(g_protoTxBuf, txLen);
	
	// 等待接收数据
	while (timeout--) {
		rxLen = Network_GetRxData(g_proRxBuf);
		if (rxLen == 0) {
			osDelay(1);
			continue;
		}
		
		if (Proto_ParseFrame(UL_DEVICE_JOIN, rxLen) == 0) {
			
			if (g_pRxPayload[0] == 0x11) {
				// 设备使用位置不正确，GPS位置不正确
			} else if (g_pRxPayload[0] == 0x12) {
				// 设备被禁用
			} else {
				g_isServerJoined = true;
			}
			
			// 更新参数
			if ((g_pRxPayload[1] & 0x01) == 0x01) {
				CMNC_AddCmd(UL_USERMSG_UPDATE);
			}
			if ((g_pRxPayload[1] & 0x02) == 0x02) {
				CMNC_AddCmd(UL_POULTYPE_UPDATE);
			}
			if ((g_pRxPayload[1] & 0x04) == 0x04) {
				CMNC_AddCmd(UL_FIRMWARE_UPDATE);
			}
			
			// 校正系统时间
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
	
	// 用户信息库版本
	g_pTxPayload[payloadLen++] = mercBase->Version[0];
	g_pTxPayload[payloadLen++] = mercBase->Version[1];

	txLen = Proto_PrepareFrame(UL_USERMSG_UPDATE, payloadLen);
	Network_Send(g_protoTxBuf, txLen);
	
	// 等待接收数据
	while (timeout--) {
		rxLen = Network_GetRxData(g_proRxBuf);
		if (rxLen == 0) {
			osDelay(1);
			continue;
		}
		
		if (Proto_ParseFrame(UL_USERMSG_UPDATE, rxLen) == 0) {			
			// 检查数据长度
			if ((payLen - 2) / 20 > 30) {
				// 数据超出
				DBG("user base is out of range.\r\n");
			}
			
			// 用户信息库更新
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
	
	// 食品信息库版本
	g_pTxPayload[payloadLen++] = poulBase->version[0];
	g_pTxPayload[payloadLen++] = poulBase->version[1];

	txLen = Proto_PrepareFrame(UL_POULTYPE_UPDATE, payloadLen);
	Network_Send(g_protoTxBuf, txLen);
	
	// 等待接收数据
	while (timeout--) {
		rxLen = Network_GetRxData(g_proRxBuf);
		if (rxLen == 0) {
			osDelay(1);
			continue;
		}
		
		if (Proto_ParseFrame(UL_POULTYPE_UPDATE, rxLen) == 0) {			
			// 更新禽类信息库
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
	
	// 用户信息(店主需要登录，客户不需要登录)
	memcpy(g_pTxPayload + payloadLen, User_GetInfo()->recv.code, USER_CODE_SIZE);
	payloadLen += USER_CODE_SIZE;

	txLen = Proto_PrepareFrame(UL_USER_LOGIN, payloadLen);
	Network_Send(g_protoTxBuf, txLen);
	
	// 等待接收数据
	while (timeout--) {
		rxLen = Network_GetRxData(g_proRxBuf);
		if (rxLen == 0) {
			osDelay(1);
			continue;
		}
		
		if (Proto_ParseFrame(UL_USER_LOGIN, rxLen) == 0) {
			if (g_pRxPayload[0] == 0x00) {	// 空用户
				User_SetCode(USER_PERM_NULL);
			} else if (g_pRxPayload[0] == 0x01) {	// 普通店主
				User_SetCode(USER_PERM_MERCHANT);
			} else if (g_pRxPayload[0] == 0x02) {	// 管理员
				User_SetCode(USER_PERM_ADMIN);
			} else if (g_pRxPayload[0] == 0x03) {	// 维护员
				User_SetCode(USER_PERM_MAINTAIN);
			} else if (g_pRxPayload[0] == 0x04) {	// 普通客户
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

	// 错误状态
	g_pTxPayload[payloadLen++] = errorstatus;
	
	// 剩余标签
	g_pTxPayload[payloadLen++] = ( remainnumb & 0xFF00 ) >> 8;
	g_pTxPayload[payloadLen++] = ( remainnumb & 0x00FF ) >> 0;

	txLen = Proto_PrepareFrame(UL_HEARTBEAT, payloadLen);
	Network_Send(g_protoTxBuf, txLen);
	
	// 等待接收数据
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

