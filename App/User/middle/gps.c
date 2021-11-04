/**
  ******************************************************************************
  * @file           : gps.c
  * @brief          : Gps module function
  ******************************************************************************
  * @attention
  *
  * Gps function is used.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "apps.h"
#include "gps.h"
#include "drivers.h"
#include "param.h"

/* Private define ------------------------------------------------------------*/

#define DBG_TRACE                                   0

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

/**
  * @brief  GPSģ���Դ����
  */
#define     GPS_ON      HAL_GPIO_WritePin(GPS_CTL_GPIO_Port, GPS_CTL_Pin, GPIO_PIN_SET)
#define     GPS_OFF     HAL_GPIO_WritePin(GPS_CTL_GPIO_Port, GPS_CTL_Pin, GPIO_PIN_RESET)

/* Private typedef -----------------------------------------------------------*/

/**
  * GPS��Ϣ�ṹ��
  */
typedef struct {
	uint8_t latitude[10];   // γ��
	uint8_t NS;             // �ϱ�γ
	uint8_t longitude[10];  // ����
	uint8_t EW;             // ������
}GPS_LLParam_t;

/* Private define ------------------------------------------------------------*/

/**
  * ���Դ�ӡ������Ϣ
  */
#define DEBUG_GPS           0

/**
  * GPS��λ��������
  *
  * ����:
  *     $GPGLL,,,,,062611.76,V,N*49
  *     $GPGLL,2321.25865,N,11318.67101,E,062617.00,A,A*68
  */
#define GPS_LL_HEAD         "$GPGLL"   // ��Ϣ��ͷ

/**
  * GPS���ǽ�������
  *
  * ����:
  *     $GPGSV,4,4,15,30,,,23,32,48,119,37,50,59,148,40*7F
  */
#define GPS_SV_HEAD         "$GPGSV"

/**
  * ������������С
  */
#define GPS_MSG_UNIT_NUM    8          // 8����ϢԪ
#define GPS_MSG_UNIT_SIZE   20         // ������ϢԪ���20�ֽ�

/* Private macro -------------------------------------------------------------*/

/**
  * ���ݻ�����
  */
uint8_t g_gpsRxBuf[UART8_BUFFSIZE] = { 0 };
char    g_preSplitBuf[GPS_MSG_UNIT_NUM * GPS_MSG_UNIT_SIZE] = { 0 };   // ���ǰ��Ϣ������
char    g_aftSplitBuf[GPS_MSG_UNIT_NUM][GPS_MSG_UNIT_SIZE] = { 0 };    // ��ֺ���Ϣ������

/**
  * Gps��γ����Ϣ����
  */
GPS_LLParam_t g_GPS_LLInfo = {
	.latitude = "23.0",    // γ��
	.NS = 'N',             // �ϱ�γ
	.longitude = "113.0",  // ����
	.EW = 'E',             // ������
//	0
};

/**
  * Gps������Ϣ
  */
GPS_SVParam_t g_GPS_SVInfo;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ����������Ϣ
  */
static void GPS_ParseSV(void);

/**
  * @brief  ������λ��Ϣ
  */
static void GPS_ParseLocation(void);

#if (DBG_TRACE == 1)
/**
  * @brief  ��ӡ��λ��Ϣ
  */
static void GPS_ShowLocation(void);
#endif

/**
  * @brief  ����Ϣ���ֳɶ����ϢԪ
  */
static void GPS_Split(char *source, uint16_t srcLen, char *destination, char delim, uint8_t num, uint8_t size);


/**
  * @brief  ����GPS����
  * @param  state   ����״̬
  * @retval 
  */
void GPS_Init(void)
{
	GPS_ON;
	Uart_SetRxEnable(&huart8);
}

/**
  * @brief  ѭ������GPS����
  * @param  
  * @retval 
  */
void GPS_Process(void)
{
	uint16_t rxLen = Uart_GetData(&huart8, g_gpsRxBuf);
	if (rxLen == 0) {
		return ;
	}
	
	Error_Del(ERROR_GPS_UNCOMMUNICATE);
	
	DBG("%s", g_gpsRxBuf);
	
	// ����������Ϣ
	GPS_ParseSV();
	
	// ��ȡ��λ
	GPS_ParseLocation();
	
	// ��ʾ��λ
#if (DBG_TRACE == 1)
	GPS_ShowLocation();
#endif
	
	// ��ջ�����
	memset(g_gpsRxBuf, 0, UART8_BUFFSIZE);
	
	// ���½�������
	Uart_SetRxEnable(&huart8);
}

/**
  * @brief  ��ȡ������Ϣ
  * @param  
  * @retval 
  */
GPS_SVParam_t *GPS_GetSVDate(void)
{
	return &g_GPS_SVInfo;
}

/**
  * @brief  �ж�������Ϣ�Ƿ��Ѿ���ȡ
  * @param  
  * @retval 
  */
bool GPS_SVIsReady(void)
{
	if (((g_GPS_SVInfo.svNum[0] != 0) || (g_GPS_SVInfo.svNum[1] != 0)) && 
		((g_GPS_SVInfo.svSnr[0] != 0) || (g_GPS_SVInfo.svSnr[1] != 0)))
	{
		return true;
	}
	return false;
}

/**
  * @brief  ��ȡ��λ��Ϣ
  * @param  pBuffer     ��λ��Ϣ��������ǰ10�ֽ�Ϊ���ȣ���10�ֽ�Ϊγ��
  * @retval true  - ��ȡ�ɹ�
  *         false - ��ȡʧ��
  */
bool GPS_GetLLData(uint8_t *pBuffer)
{
	bool status = false;
	if (((g_GPS_LLInfo.NS == 'N') || (g_GPS_LLInfo.NS == 'S')) && 
		((g_GPS_LLInfo.EW == 'E') || (g_GPS_LLInfo.EW == 'W')))
	{
		memcpy(pBuffer, g_GPS_LLInfo.longitude, 10);
		memcpy(pBuffer + 10, g_GPS_LLInfo.latitude, 10);
		status = true;
	}
	return status;
}

/**
  * @brief  �ж϶�λ��Ϣ�Ƿ��Ѿ���ȡ
  * @param  
  * @retval 
  */
bool GPS_LLIsReady(void)
{
	if (((g_GPS_LLInfo.NS == 'N') || (g_GPS_LLInfo.NS == 'S')) && 
		((g_GPS_LLInfo.EW == 'E') || (g_GPS_LLInfo.EW == 'W')))
	{
		return true;
	}
	return false;
}

/**
  * @brief  ����������Ϣ
  * @param  
  * @retval 
  */
static void GPS_ParseSV(void)
{
	char *ptrStart = strstr((char *)g_gpsRxBuf, GPS_SV_HEAD);
	char *ptrEnd = strstr(ptrStart + 6, "$GP");
	
	if (ptrStart == NULL) {
		return ;
	}
	
	if (ptrEnd == NULL) {
		ptrEnd = ((char *)g_gpsRxBuf + strlen((char *)g_gpsRxBuf));
	}
	
	// ���������Ϣ����
	uint8_t msgSize = ptrEnd - ptrStart;
	
	// ��ջ�����
	memset(g_preSplitBuf, 0, GPS_MSG_UNIT_NUM * GPS_MSG_UNIT_SIZE);
	memset(g_aftSplitBuf, 0, GPS_MSG_UNIT_NUM * GPS_MSG_UNIT_SIZE);
	
	// ����$GPRMC����Ϣ
	strncpy(g_preSplitBuf, ptrStart, msgSize);
	
	// ��ָ���Ϣ
	GPS_Split(g_preSplitBuf, strlen(g_preSplitBuf), (char *)g_aftSplitBuf, ',', GPS_MSG_UNIT_NUM, GPS_MSG_UNIT_SIZE);
	
	if (((g_aftSplitBuf[3][0] != 0) || (g_aftSplitBuf[3][1] != 0)) && 
		((g_aftSplitBuf[7][0] != 0) || (g_aftSplitBuf[7][1] != 0)))
	{
		Error_Del(ERROR_GPS_NO_SVDATA);
		g_GPS_SVInfo.svNum[0] = g_aftSplitBuf[3][0];
		g_GPS_SVInfo.svNum[1] = g_aftSplitBuf[3][1];
		g_GPS_SVInfo.svSnr[0] = g_aftSplitBuf[7][0];
		g_GPS_SVInfo.svSnr[1] = g_aftSplitBuf[7][1];
	}

	return ;
}

/**
  * @brief  ������λ��Ϣ
  * @param  
  * @retval 
  */
static void GPS_ParseLocation(void)
{
	char *ptrStart = strstr((char *)g_gpsRxBuf, GPS_LL_HEAD);
	char *ptrEnd = strstr(ptrStart + 6, "$GP");
	
	if (ptrStart == NULL) {
		return ;
	}
	
	if (ptrEnd == NULL) {
		ptrEnd = ((char *)g_gpsRxBuf + strlen((char *)g_gpsRxBuf));
	}
	
	// ���������Ϣ����
	uint8_t msgSize = ptrEnd - ptrStart;
	
	memset(g_preSplitBuf, 0, GPS_MSG_UNIT_NUM * GPS_MSG_UNIT_SIZE);
	memset(g_aftSplitBuf, 0, GPS_MSG_UNIT_NUM * GPS_MSG_UNIT_SIZE);
	
	// ����$GPRMC����Ϣ
	strncpy(g_preSplitBuf, ptrStart, msgSize);
	
	// ��ָ���Ϣ
	GPS_Split(g_preSplitBuf, strlen(g_preSplitBuf), (char *)g_aftSplitBuf, ',', GPS_MSG_UNIT_NUM, GPS_MSG_UNIT_SIZE);
	
	if (((g_aftSplitBuf[2][0] == 'N') || (g_aftSplitBuf[2][0] == 'S')) && 
		((g_aftSplitBuf[4][0] == 'E') || (g_aftSplitBuf[4][0] == 'W')))
	{
		Error_Del(ERROR_GPS_NO_LOCATION);
		memcpy(g_GPS_LLInfo.latitude, g_aftSplitBuf[1], 10);
		g_GPS_LLInfo.NS = g_aftSplitBuf[2][0];
		memcpy(g_GPS_LLInfo.longitude, g_aftSplitBuf[3], 10);
		g_GPS_LLInfo.EW = g_aftSplitBuf[4][0];
	}

	return ;
}

#if (DBG_TRACE == 1)
/**
  * @brief  ��ӡ��λ��Ϣ
  * @param  
  * @retval 
  */
static void GPS_ShowLocation(void)
{
	if ((g_GPS_SVInfo.svNum[1] != 0) || (g_GPS_SVInfo.svSnr[1] != 0)) {
		printf("Satellites: %c%c, %c%cdB\r\n", g_GPS_SVInfo.svNum[0], g_GPS_SVInfo.svNum[1], 
		    g_GPS_SVInfo.svSnr[0], g_GPS_SVInfo.svSnr[1]);
	}
	
	if (((g_GPS_LLInfo.NS == 'N') || (g_GPS_LLInfo.NS == 'S')) && 
		((g_GPS_LLInfo.EW == 'E') || (g_GPS_LLInfo.EW == 'W')))
	{
		printf("gps: ");
		uint8_t i = 0;
		// ��ʾ����
		for (i = 0; i < 10; i++) {
			printf("%c", g_GPS_LLInfo.longitude[i]);
		}
		printf(", ");
		// ��ʾγ��
		for (i = 0; i < 10; i++) {
			printf("%c", g_GPS_LLInfo.latitude[i]);
		}
		printf("\r\n");
	}
}
#endif

/**
  * @brief  ����Ϣ���ֳɶ����ϢԪ
  * @param  source          ���ǰ����Ϣ������
  *         srcLen          ���ǰ����Ϣ����
  *         destination     ��ֺ����Ϣ������
  *         delim           ������ݵı�־
  *         num             ��ֵ���ϢԪ����
  *         size            ��ֵ���ϢԪ��󳤶�
  * @retval 
  */
static void GPS_Split(char *source, uint16_t srcLen, char *destination, char delim, uint8_t num, uint8_t size)
{
	// �л�����һ����ϢԪ
	char *cur = source;
	char *next = source;
	uint8_t i = 0;
	while (*next != delim) {
		if (next - source <= srcLen - 1) {
			next++;
		} else {
			break;
		}
	}

	for (i = 0; i < num; i++) {
		// �洢��ϢԪ
		if (next - cur < size) {                                 // ȷ���������ܴ洢�¸���ϢԪ
			if (i == 0) {                                        // ��1����ϢԪ("$GPGLL")
				memcpy(destination, cur, next - cur);            // 
			} else {                                             // �м����ϢԪ(��",11318.67101")
				memcpy(destination, cur + 1, next - cur - 1);    // ȥ��delim��־
			}
			destination += size;
		}
	
		// �л�����һ����ϢԪ
		cur = next++;
		if ((*cur == '\n') && (*next == '\r')) {
			break;
		}
		while ((*next != delim) && ((*next != '\r') && (*(next + 1) != '\n'))) {
			if (next - source <= srcLen - 1) {
				next++;
			} else {
				break;
			}
		}
	}
}
