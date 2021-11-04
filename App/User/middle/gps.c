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
  * @brief  GPS模块电源开关
  */
#define     GPS_ON      HAL_GPIO_WritePin(GPS_CTL_GPIO_Port, GPS_CTL_Pin, GPIO_PIN_SET)
#define     GPS_OFF     HAL_GPIO_WritePin(GPS_CTL_GPIO_Port, GPS_CTL_Pin, GPIO_PIN_RESET)

/* Private typedef -----------------------------------------------------------*/

/**
  * GPS信息结构体
  */
typedef struct {
	uint8_t latitude[10];   // 纬度
	uint8_t NS;             // 南北纬
	uint8_t longitude[10];  // 经度
	uint8_t EW;             // 东西经
}GPS_LLParam_t;

/* Private define ------------------------------------------------------------*/

/**
  * 调试打印所有信息
  */
#define DEBUG_GPS           0

/**
  * GPS定位解析参数
  *
  * 例如:
  *     $GPGLL,,,,,062611.76,V,N*49
  *     $GPGLL,2321.25865,N,11318.67101,E,062617.00,A,A*68
  */
#define GPS_LL_HEAD         "$GPGLL"   // 信息块头

/**
  * GPS卫星解析参数
  *
  * 例如:
  *     $GPGSV,4,4,15,30,,,23,32,48,119,37,50,59,148,40*7F
  */
#define GPS_SV_HEAD         "$GPGSV"

/**
  * 解析缓冲区大小
  */
#define GPS_MSG_UNIT_NUM    8          // 8个信息元
#define GPS_MSG_UNIT_SIZE   20         // 单个信息元最大20字节

/* Private macro -------------------------------------------------------------*/

/**
  * 数据缓冲区
  */
uint8_t g_gpsRxBuf[UART8_BUFFSIZE] = { 0 };
char    g_preSplitBuf[GPS_MSG_UNIT_NUM * GPS_MSG_UNIT_SIZE] = { 0 };   // 拆分前信息缓冲区
char    g_aftSplitBuf[GPS_MSG_UNIT_NUM][GPS_MSG_UNIT_SIZE] = { 0 };    // 拆分后信息缓冲区

/**
  * Gps经纬度信息参数
  */
GPS_LLParam_t g_GPS_LLInfo = {
	.latitude = "23.0",    // 纬度
	.NS = 'N',             // 南北纬
	.longitude = "113.0",  // 经度
	.EW = 'E',             // 东西经
//	0
};

/**
  * Gps卫星信息
  */
GPS_SVParam_t g_GPS_SVInfo;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  解析卫星信息
  */
static void GPS_ParseSV(void);

/**
  * @brief  解析定位信息
  */
static void GPS_ParseLocation(void);

#if (DBG_TRACE == 1)
/**
  * @brief  打印定位信息
  */
static void GPS_ShowLocation(void);
#endif

/**
  * @brief  把信息块拆分成多个信息元
  */
static void GPS_Split(char *source, uint16_t srcLen, char *destination, char delim, uint8_t num, uint8_t size);


/**
  * @brief  开关GPS功能
  * @param  state   开关状态
  * @retval 
  */
void GPS_Init(void)
{
	GPS_ON;
	Uart_SetRxEnable(&huart8);
}

/**
  * @brief  循环处理GPS数据
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
	
	// 解析卫星信息
	GPS_ParseSV();
	
	// 读取定位
	GPS_ParseLocation();
	
	// 显示定位
#if (DBG_TRACE == 1)
	GPS_ShowLocation();
#endif
	
	// 清空缓冲区
	memset(g_gpsRxBuf, 0, UART8_BUFFSIZE);
	
	// 重新接收数据
	Uart_SetRxEnable(&huart8);
}

/**
  * @brief  获取卫星信息
  * @param  
  * @retval 
  */
GPS_SVParam_t *GPS_GetSVDate(void)
{
	return &g_GPS_SVInfo;
}

/**
  * @brief  判断卫星信息是否已经获取
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
  * @brief  获取定位信息
  * @param  pBuffer     定位信息缓冲区，前10字节为经度，后10字节为纬度
  * @retval true  - 获取成功
  *         false - 获取失败
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
  * @brief  判断定位信息是否已经获取
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
  * @brief  解析卫星信息
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
	
	// 计算这个信息长度
	uint8_t msgSize = ptrEnd - ptrStart;
	
	// 清空缓冲区
	memset(g_preSplitBuf, 0, GPS_MSG_UNIT_NUM * GPS_MSG_UNIT_SIZE);
	memset(g_aftSplitBuf, 0, GPS_MSG_UNIT_NUM * GPS_MSG_UNIT_SIZE);
	
	// 复制$GPRMC的信息
	strncpy(g_preSplitBuf, ptrStart, msgSize);
	
	// 拆分该信息
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
  * @brief  解析定位信息
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
	
	// 计算这个信息长度
	uint8_t msgSize = ptrEnd - ptrStart;
	
	memset(g_preSplitBuf, 0, GPS_MSG_UNIT_NUM * GPS_MSG_UNIT_SIZE);
	memset(g_aftSplitBuf, 0, GPS_MSG_UNIT_NUM * GPS_MSG_UNIT_SIZE);
	
	// 复制$GPRMC的信息
	strncpy(g_preSplitBuf, ptrStart, msgSize);
	
	// 拆分该信息
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
  * @brief  打印定位信息
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
		// 显示经度
		for (i = 0; i < 10; i++) {
			printf("%c", g_GPS_LLInfo.longitude[i]);
		}
		printf(", ");
		// 显示纬度
		for (i = 0; i < 10; i++) {
			printf("%c", g_GPS_LLInfo.latitude[i]);
		}
		printf("\r\n");
	}
}
#endif

/**
  * @brief  把信息块拆分成多个信息元
  * @param  source          拆分前的信息缓冲区
  *         srcLen          拆分前的信息长度
  *         destination     拆分后的信息缓冲区
  *         delim           拆分依据的标志
  *         num             拆分的信息元数量
  *         size            拆分的信息元最大长度
  * @retval 
  */
static void GPS_Split(char *source, uint16_t srcLen, char *destination, char delim, uint8_t num, uint8_t size)
{
	// 切换到第一个信息元
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
		// 存储信息元
		if (next - cur < size) {                                 // 确保缓冲区能存储下该信息元
			if (i == 0) {                                        // 第1个信息元("$GPGLL")
				memcpy(destination, cur, next - cur);            // 
			} else {                                             // 中间的信息元(如",11318.67101")
				memcpy(destination, cur + 1, next - cur - 1);    // 去除delim标志
			}
			destination += size;
		}
	
		// 切换到下一个信息元
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
