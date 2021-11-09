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

#define MAX_PROTOCOL_NUMBER                         10

#define IOT_TX_SIZE			  						1220

/**
  * ����֡��󳤶�
  */
#define PAYLOAD_SIZE	  							1200

/**
  * ���Ӧ��ʱ��(��λms)
  */
#define CMNC_MAX_ACK_TIMEOUT                        10000

/**
  * ����ط�����
  */
#define CMNC_MAX_RETRY_NUMBER                       3

/* Private variables ---------------------------------------------------------*/

/**
  * ���ݻ�����
  */
static uint8_t g_CMNC_TxBuf[IOT_TX_SIZE] = { 0 };	// ���緢�����ݻ�����
static uint16_t g_CMNC_TxLen;
static uint8_t g_CMNC_RxBuf[IOT_TX_SIZE] = { 0 };

/**
  * ָ���б�
  */
static uint8_t g_CMNC_List[MAX_PROTOCOL_NUMBER];
static uint8_t g_CMNC_Num = 0;
static CMNC_CmdType_t g_CMNC_CmdType = UL_FUNC_NONE;   // ��ǰ����

/**
  * ��ʱ��
  */
static uint32_t g_CMNCTick = 0;
static TickTimer_t g_CMNCAckTimeout;

/**
  * �ط�����
  */
uint8_t  g_CMNC_RetryCount = 0;

/**
  * ֡ͷ��֡β
  */
const uint8_t FrameHead[2] = { 0x88, 0xFB };
const uint8_t FrameTail[2] = { 0xFC, 0xFC };

CMNCSolve_t g_CMNCSolve[10] = {
	{ UL_DEVICE_JOIN,     CMNC_01_Solve },
	{ UL_USERMSG_UPDATE,  CMNC_02_Solve },
	{ UL_POULTYPE_UPDATE, CMNC_03_Solve },
	{ UL_FIRMWARE_UPDATE, CMNC_04_Solve },
	{ UL_USER_LOGIN,      CMNC_05_Solve },
	{ UL_HEARTBEAT,       CMNC_06_Solve },
	{ UL_FUNC_7,          CMNC_07_Solve },
	{ UL_FUNC_8,          CMNC_08_Solve },
	{ UL_FUNC_9,          CMNC_09_Solve },
	{ UL_FUNC_A,          CMNC_0A_Solve },
};

bool g_isServerJoined = false;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  �������ݵ�������
  */
static void CMNC_ULProcess(void);

/**
  * @brief  ��ָ����ж�ȡЭ��ָ��
  */
static uint8_t CMNC_GetCmd(void);

static void CMNC_Prepare(CMNC_CmdType_t type);

static void CMNC_SendCmd(uint8_t *pBuf, uint16_t len);

/**
  * @brief  ��ʼ��Э���
  * @param  
  * @retval 
  */
void CMNC_Init(void)
{
	g_CMNC_List[g_CMNC_Num++] = UL_DEVICE_JOIN;
}

/**
  * @brief  ����Э�������
  * @param  
  * @retval 
  */
void CMNC_Process(void)
{	
	g_CMNCTick = HAL_GetTick();

	// ������
	CMNC_ULProcess();

	// �����������
	CMNC_DLProcess();

	// Ӧ��ʱ
	if ((g_CMNCAckTimeout.count > 0) && (g_CMNCTick - g_CMNCAckTimeout.start > g_CMNCAckTimeout.count)) {
		g_CMNCAckTimeout.start = 0;
		g_CMNCAckTimeout.count = 0;
		
		if (g_CMNC_RetryCount < CMNC_MAX_RETRY_NUMBER) {
			g_CMNC_RetryCount++;
		} else {
			g_CMNC_RetryCount = 0;
			
			// �Ѵ������ָ��Ľṹ�����
			g_CMNC_CmdType = UL_FUNC_NONE;
			DBG("wait ack timeout!\r\n");
		}
	}
}

/**
  * @brief  ���Э��ָ�ָ�����
  * @param  cmd   Э��ָ��
  * @retval 
  */
void CMNC_AddCmd(uint8_t cmd)
{
	u8 i;
	NwkStatus_t nwkStatus = Network_GetStatus();
	if ((nwkStatus == NETWORK_STATUS_NET) || (nwkStatus == NETWORK_STATUS_SIM)) {
		if ((g_isServerJoined == true) && (g_CMNC_Num < MAX_PROTOCOL_NUMBER)) {
			for (i = 0; i < g_CMNC_Num; i++) {
				if (g_CMNC_List[i] == cmd) {
					return ;
				}
			}
			g_CMNC_List[g_CMNC_Num++] = cmd;
		}
	}
}

/**
  * @brief  �������������
  * @param  
  * @retval 
  */
void CMNC_DLProcess(void)
{
	uint8_t *buffer = NULL;
	uint16_t len = 0;
	uint16_t calcCrc = 0;
	uint8_t i = 0;
	u8 *devAddr = Store_GetConfig()->devAddr;

	uint16_t rxLen = Network_GetRxData(g_CMNC_RxBuf);
	if (rxLen > 0) {
		DBG("Net rx:\r\n");
		PrintHexBuffer(g_CMNC_RxBuf, rxLen);
	
		buffer = g_CMNC_RxBuf;
		len = rxLen;

		// 1.����֡ͷ��֡β�Ƿ���ȷ
		if ((buffer[0] != 0x88) || 
			(buffer[1] != 0xFB) || 
			(buffer[len-1] != 0xFC) || 
			(buffer[len-2] != 0xFC)) 
		{
			DBG("Frame head or fail is wrong!\r\n");
			return ;
		}

		// 2.����CRC�Ƿ���ȷ
		calcCrc = CalCrc16( buffer + 2, len - 6, 0xFFFF );
		if (calcCrc != ((buffer[len - 4] << 8) | buffer[len - 3])) {
			DBG("CRC error: calc crc: %04x, recv crc: %04x\r\n", calcCrc, ( ( buffer[len - 3] << 8 ) | buffer[len - 4] ) );
			return ;
		}
		
		// 3.�����豸��ַ�Ƿ���ȷ
		if ((devAddr[0] != buffer[5]) || (devAddr[1] != buffer[6]) || (devAddr[2] != buffer[7]) ||
			(devAddr[3] != buffer[8]) || (devAddr[4] != buffer[9]) || (devAddr[5] != buffer[10]))
		{
			return ;
		}

		// 4.���ָ���Ƿ��Ӧ
		if (g_CMNC_CmdType != buffer[4]) {
			DBG("Cmd is different!\r\n");
			return ;
		}

		// 5.���ô�����
		for (i = 0; i < 10; i++) {
			if (g_CMNCSolve[i].cmdType == buffer[4]) {
				g_CMNCSolve[i].cmdSolve(buffer + 11, len - 15);
				
				g_CMNC_RetryCount = 0;
				g_CMNCAckTimeout.start = 0;
				g_CMNCAckTimeout.count = 0;
				g_CMNC_CmdType = UL_FUNC_NONE;
				break;
			}
		}
	}
}

/**
  * @brief  �������ݵ�������
  * @param  
  * @retval 
  */
static void CMNC_ULProcess(void)
{
	if (g_CMNCAckTimeout.count > 0) { /* �ȴ�Ӧ���� */
		return ;
	}

	// δ���ӷ�������ǰһ��ָ�û���յ���Ӧ���ڵȴ�Ӧ���򷵻�
	if (GPS_LLIsReady() == false) {
		return ;
	}

	if (g_CMNC_CmdType == UL_FUNC_NONE) {
		g_CMNC_CmdType = (CMNC_CmdType_t)CMNC_GetCmd();
	}
	
	if (g_CMNC_CmdType != UL_FUNC_NONE) {
		CMNC_Prepare(g_CMNC_CmdType);
		CMNC_SendCmd(g_CMNC_TxBuf, g_CMNC_TxLen);
		if ((g_CMNC_CmdType == UL_USER_LOGIN) || (g_CMNC_CmdType == UL_HEARTBEAT)) {
			g_CMNC_RetryCount += CMNC_MAX_RETRY_NUMBER;
		}
	}
}

void CMNC_01_Solve(u8 *pBuf, u16 len)
{
	RTCTime_t rtcTime = { 0 };
	memcpy(rtcTime.value, pBuf + 2, 6);
	if (pBuf[0] == 0x11) {
		// �豸ʹ��λ�ò���ȷ��GPSλ�ò���ȷ
	} else if (pBuf[0] == 0x12) {
		// �豸������
	} else {
		g_isServerJoined = true;
	}
	
	// ���²���
	if ((pBuf[1] & 0x01) == 0x01) {
		CMNC_AddCmd(UL_USERMSG_UPDATE);
	}
	if ((pBuf[1] & 0x02) == 0x02) {
		CMNC_AddCmd(UL_POULTYPE_UPDATE);
	}
	if ((pBuf[1] & 0x04) == 0x04) {
		CMNC_AddCmd(UL_FIRMWARE_UPDATE);
	}
	
	// У��ϵͳʱ��
	RTCTime_SetTime(rtcTime.rtc.year, rtcTime.rtc.month,  rtcTime.rtc.day, 
					 rtcTime.rtc.hour, rtcTime.rtc.minute, rtcTime.rtc.second);
	
	DBG("################## Join device #################\r\n");
	DBG("func flag    : %02x %02x\r\n", pBuf[0], pBuf[1]);
	DBG("system time  : %02d-%02d-%02d %02d:%02d:%02d\r\n", rtcTime.rtc.year + 2000, rtcTime.rtc.month, 
		rtcTime.rtc.day, rtcTime.rtc.hour, rtcTime.rtc.minute, rtcTime.rtc.second);

	if (pBuf[0] != 0) {
		DBG("device link error.\r\n");
	}
}

void CMNC_02_Solve(u8 *pBuf, u16 len)
{
	u8 i = 0, j = 0;
	MercBase_t *userBase = User_GetInfo()->mercBase;
				
	// ������ݳ���
	if ((len - 2) / 20 > 30) {
		// ���ݳ���
		DBG("user base is out of range.\r\n");
	}
	
	// �û���Ϣ�����
	userBase->Version[0] = pBuf[0];
	userBase->Version[1] = pBuf[1];
	userBase->Sum = (len - 2) / MERCHANT_MESSAGE_SIZE;
	for (j = 0; j < userBase->Sum; j++) {
		userBase->List.Info[j].perm = (UserPerm_t)pBuf[2 + j * MERCHANT_MESSAGE_SIZE];
		memcpy(userBase->List.Info[j].code, pBuf + 3 + j * MERCHANT_MESSAGE_SIZE, USER_CODE_SIZE);
	}
	
	Store_SaveMercBase();
	
	DBG("################ Updata userbase ###############\r\n");
	DBG("user base ver: %02x %02x\r\n", pBuf[0], pBuf[1]);
	for (j = 0; j < (len - 2) / MERCHANT_MESSAGE_SIZE; j++) {
		DBG("list [%d]    : ", j);
		for (i = 0; i < MERCHANT_MESSAGE_SIZE; i++) {
			if (i == 0) {
				DBG("(lv)");
			} else if (i == 1) {
				DBG("(id)");
			} else if (i == 1 + USER_CODE_SIZE) {
				DBG("(res)");
			}
			DBG("%02x ", pBuf[j * MERCHANT_MESSAGE_SIZE + 2 + i]);
		}
		DBG("\r\n");
	}
}

void CMNC_03_Solve(u8 *pBuf, u16 len)
{
	u8 i = 0, j = 0;
	PoulBase_t *poulBase = NULL;

	// ����������Ϣ��
	poulBase = Poul_GetInfo()->base[0];
	poulBase->version[0] = pBuf[0];
	poulBase->version[1] = pBuf[1];
	poulBase->sum = (len - 17) / 16;
	for (i = 0; i < poulBase->sum; i++) {
		memcpy(poulBase->kinds[i].kindNum, pBuf + 2 + i * 16, 6);
		memcpy(poulBase->kinds[i].name, pBuf + 8 + i * 16, 10);
	}
	Store_SavePoulBase();
	
	DBG("################ Updata poulbase ###############\r\n");
	DBG("poul base ver: %02x %02x\r\n", pBuf[0], pBuf[1]);
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
			DBG("%02x ", pBuf[i * 16 + 2 + j]);
		}
		DBG("\r\n");
	}
}

void CMNC_04_Solve(u8 *pBuf, u16 len)
{
	u16 fragSum = (pBuf[1] << 8) | pBuf[2];
	u32 firmSize = ((pBuf[3] << 24) | (pBuf[4] << 16) | (pBuf[5] << 8) | pBuf[6]);
	if (fragSum == (firmSize / UPGRADE_FRAG_SIZE) + (firmSize & (UPGRADE_FRAG_SIZE - 1)) ? 1 : 0) {
		Upg_Start(pBuf[0], firmSize, (pBuf[7] << 8) | pBuf[8]);
		g_tmpPara->route.updateSoft = true;
	}
}

void CMNC_05_Solve(u8 *pBuf, u16 len)
{	
	if (pBuf[0] == 0x00) {	// ���û�
		User_SetCode(USER_PERM_NULL);
	} else if (pBuf[0] == 0x01) {	// ��ͨ����
		User_SetCode(USER_PERM_MERCHANT);
	} else if (pBuf[0] == 0x02) {	// ����Ա
		User_SetCode(USER_PERM_ADMIN);
	} else if (pBuf[0] == 0x03) {	// ά��Ա
		User_SetCode(USER_PERM_MAINTAIN);
	} else if (pBuf[0] == 0x04) {	// ��ͨ�ͻ�
		User_SetCode(USER_PERM_CUSTOMER);
		User_SetCust(pBuf + 1);
	}
	
	DBG("################## User login ##################\r\n");
	if ((pBuf[0] == 0x01) || (pBuf[0] == 0x02) || (pBuf[0] == 0x03) || (pBuf[0] == 0x04)) {
		DBG("Login success.\r\n");
	} else {
		DBG("Login fail.\r\n");
	}
}

void CMNC_06_Solve(u8 *pBuf, u16 len)
{
	u16 status = pBuf[0] << 8 | pBuf[1];
	if (status == 0x0003) {
		CMNC_AddCmd(UL_POULTYPE_UPDATE);
	}
}

void CMNC_07_Solve(u8 *pBuf, u16 len)
{
	RecordInfo_t *recdInfo = NULL;
	if ((pBuf[0] == 0x00) || (pBuf[0] == 0x01) || (pBuf[0] == 0x02)) {
		// �������ϱ��Ľ��׼�¼����
		recdInfo = Store_GetRecord();
		if (recdInfo->upNum > 0) {
			recdInfo->num.syncNum += recdInfo->upNum;
			recdInfo->upNum = 0;
		}
		
		Store_SaveRecordNum();
	} else if (pBuf[0] == 0x02) {

	}
	
	DBG("################ Product message upload ###############\r\n");
	if ((pBuf[0] == 0x00) || (pBuf[0] == 0x01)) {
		DBG("Upload product message success.\r\n");
	} else {
		DBG("Upload product message fail.\r\n");
	}
}

void CMNC_08_Solve(u8 *pBuf, u16 len)
{
	u8 i = 0, j = 0;
	PoulBase_t *poulBase = Poul_GetInfo()->base[1];

	DBG("################ Updata new add poulbase ###############\r\n");
	poulBase->sum = (len - 15) / 16;
	for (i = 0; i < poulBase->sum; i++) {
#if (DBG_TRACE == 1)
		DBG("list [%d]    : ", i);
		for (j = 0; j < 16; j++) {
			if (j == 0) {
				DBG("(class)");
			} else if (j == 2) {
				DBG("(group)");
			} else if (j == 6) {
				DBG("(name_gbk)");
			}
			DBG("%02x ", pBuf[i * 16 + j]);
		}
		DBG("\r\n");
#endif

		memcpy(poulBase->kinds[i].kindNum, pBuf + 0 + i * 16, 6);
		memcpy(poulBase->kinds[i].name, pBuf + 6 + i * 16, 10);
	}

	Display_SetShow(DISPLAY_SHOW_ADDPAGE);
}

void CMNC_09_Solve(u8 *pBuf, u16 len)
{

}

void CMNC_0A_Solve(u8 *pBuf, u16 len)
{
	UpgInfo_t *upgInfo = Upg_GetInfo();
	u16 fragSum = (upgInfo->firmSize / UPGRADE_FRAG_SIZE) + (upgInfo->firmSize & (UPGRADE_FRAG_SIZE - 1)) ? 1 : 0;
	if ((upgInfo->firmVer == pBuf[0]) && (fragSum == ((pBuf[1] << 8) | pBuf[2]))) {
		Upg_SaveFrag(pBuf + 3);
	}
}

/**
  * @brief  ��ָ����ж�ȡЭ��ָ��
  * @param  
  * @retval Э��ָ��
  */
static uint8_t CMNC_GetCmd(void)
{
	uint8_t ret = 0;
	if (g_CMNC_Num > 0) {
		ret = g_CMNC_List[0];
		memcpy(g_CMNC_List, g_CMNC_List + 1, g_CMNC_Num - 1);
		g_CMNC_Num--;
	}
	return ret;
}

static void CMNC_Prepare(CMNC_CmdType_t type)
{
	uint8_t *data = g_CMNC_TxBuf;
	uint16_t calcCrc = 0;
	u8 retNum = 0;
	
	// ��ջ�����
	memset(g_CMNC_TxBuf, 0, IOT_TX_SIZE);
	g_CMNC_TxLen = 0;
	
	// 1.֡ͷ
	data[g_CMNC_TxLen++] = FrameHead[0];
	data[g_CMNC_TxLen++] = FrameHead[1];
	
	// 2.���ȣ��� ָ������ �� CRCУ�� ��������֡ͷ֡β�ͳ�����6���ֽڣ�
	data[g_CMNC_TxLen++] = 0;
	data[g_CMNC_TxLen++] = 35;

	// 3.ָ������
	data[g_CMNC_TxLen++] = type;

	// 4.�豸��ַ
	uint8_t *devAddr = (uint8_t *)Store_GetConfig()->devAddr;
	memcpy(data + g_CMNC_TxLen, devAddr, sizeof(DevAddr_t));
	g_CMNC_TxLen += 6;

	if (type == UL_DEVICE_JOIN) {
		RTCTime_t *rtcTime = RTCTime_GetTime();
		Version_t *version = &Store_GetConfig()->version;
		
		// ��Ӳ���汾
		data[g_CMNC_TxLen++] = version->soft;
		data[g_CMNC_TxLen++] = version->hard;

		// ϵͳʱ��
//		data[g_CMNC_TxLen++] = rtcTime->rtc.year;
//		data[g_CMNC_TxLen++] = rtcTime->rtc.month;
//		data[g_CMNC_TxLen++] = rtcTime->rtc.day;
//		data[g_CMNC_TxLen++] = rtcTime->rtc.hour;
		
		// �û���Ϣ��汾
		MercBase_t *mercBase = User_GetInfo()->mercBase;
		data[g_CMNC_TxLen++] = mercBase->Version[0];
		data[g_CMNC_TxLen++] = mercBase->Version[1];

		// ʳƷ��Ϣ��汾
		PoulBase_t *poulBase = Poul_GetInfo()->base[0];
		data[g_CMNC_TxLen++] = poulBase->version[0];
		data[g_CMNC_TxLen++] = poulBase->version[1];

		// ��ǩ
		memset(data + g_CMNC_TxLen, 0x31, 20);
		g_CMNC_TxLen += 20;
		
		// GPS��Ϣ
		GPS_GetLLData(data + g_CMNC_TxLen);
		g_CMNC_TxLen += 20;
	} else if (type == UL_USERMSG_UPDATE) {
		// �û���Ϣ��汾
		MercBase_t *mercBase = User_GetInfo()->mercBase;
		data[g_CMNC_TxLen++] = mercBase->Version[0];
		data[g_CMNC_TxLen++] = mercBase->Version[1];
	} else if (type == UL_POULTYPE_UPDATE) {
		// ʳƷ��Ϣ��汾
		PoulBase_t *poulBase = Poul_GetInfo()->base[0];
		data[g_CMNC_TxLen++] = poulBase->version[0];
		data[g_CMNC_TxLen++] = poulBase->version[1];
	} else if (type == UL_FIRMWARE_UPDATE) {
		// �̼�����
		Version_t *version = &Store_GetConfig()->version;
		data[g_CMNC_TxLen++] = version->soft;
	} else if (type == UL_USER_LOGIN) {
		// �û���Ϣ(������Ҫ��¼���ͻ�����Ҫ��¼)
		memcpy(data + g_CMNC_TxLen, User_GetInfo()->recv.code, USER_CODE_SIZE);
		g_CMNC_TxLen += USER_CODE_SIZE;
	} else if (type == UL_HEARTBEAT) {
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
		data[g_CMNC_TxLen++] = errorstatus;
		
		// ʣ���ǩ
		data[g_CMNC_TxLen++] = ( remainnumb & 0xFF00 ) >> 8;
		data[g_CMNC_TxLen++] = ( remainnumb & 0x00FF ) >> 0;
	} else if (type == UL_FUNC_7) {
		// ������Ϣ
		retNum = Store_ReadBlock(data + g_CMNC_TxLen);
		g_CMNC_TxLen += (sizeof(RecordBlock_t) * retNum);
	} else if (type == UL_FUNC_8) {
		PoulInfo_t *poulInfo = Poul_GetInfo();
		data[g_CMNC_TxLen++] = poulInfo->list.group;
	} else if (type == UL_FUNC_9) {
		// �û���Ϣ(������Ҫ��¼���ͻ�����Ҫ��¼)
		memcpy(data + g_CMNC_TxLen, User_GetInfo()->recv.code, USER_CODE_SIZE);
		g_CMNC_TxLen += USER_CODE_SIZE;
	} else if (type == UL_FUNC_A) {
		UpgInfo_t *upgInfo = Upg_GetInfo();
		u16 fragSum = (upgInfo->firmSize / UPGRADE_FRAG_SIZE) + (upgInfo->firmSize & (UPGRADE_FRAG_SIZE - 1)) ? 1 : 0;
		data[g_CMNC_TxLen++] = upgInfo->firmVer;
		data[g_CMNC_TxLen++] = (fragSum >> 8);
		data[g_CMNC_TxLen++] = fragSum;
		data[g_CMNC_TxLen++] = ((upgInfo->firmIndex & (UPGRADE_FRAG_SIZE - 1)) >> 8);
		data[g_CMNC_TxLen++] = (upgInfo->firmIndex & (UPGRADE_FRAG_SIZE - 1));
	}

	// ����
	data[2] = (g_CMNC_TxLen - 2) >> 8;
	data[3] = g_CMNC_TxLen - 2;
	
	// 6.У��
	calcCrc = CalCrc16(data + 2, g_CMNC_TxLen - 2, 0xFFFF);
	data[g_CMNC_TxLen++] = (calcCrc & 0xFF00) >> 8;
	data[g_CMNC_TxLen++] = (calcCrc & 0x00FF) >> 0;
	
	// 7.֡β
	data[g_CMNC_TxLen++] = FrameTail[0];
	data[g_CMNC_TxLen++] = FrameTail[1];
}

static void CMNC_SendCmd(uint8_t *pBuf, uint16_t len)
{				
	// ����ָ��
	Network_Send(pBuf, len);

	// ����Ӧ��ʱ��ʱ����
	g_CMNCAckTimeout.start = g_CMNCTick;
	g_CMNCAckTimeout.count = 10000;
}
