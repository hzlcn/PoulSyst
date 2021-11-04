#include "rtctime.h"
#include "rtc.h"
#include "param.h"

/* Private variables ---------------------------------------------------------*/

/**
  * RTCʱ�仺����
  */
RTCTime_t g_RTCTime;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ����RTCʱ��
  * @param  year    ��
  *         month   ��
  *         day     ��
  *         hour    ʱ
  *         minute  ��
  *         second  ��
  * @retval 
  */
void RTCTime_SetTime(uint8_t year, uint8_t month,  uint8_t date,
                      uint8_t hour, uint8_t minute, uint8_t second)
{
	RTC_TimeTypeDef timetmp = { 0 };
	RTC_DateTypeDef datatmp = { 0 };

	// �洢RTCʱ�����
	g_RTCTime.rtc.year   = year;
	g_RTCTime.rtc.month  = month;
	g_RTCTime.rtc.day    = date;
	g_RTCTime.rtc.hour   = hour;
	g_RTCTime.rtc.minute = minute;
	g_RTCTime.rtc.second = second;
	
	// ��������
	timetmp.Hours   = hour;
	timetmp.Minutes = minute;
	timetmp.Seconds = second;
	timetmp.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	timetmp.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &timetmp, RTC_FORMAT_BIN) != HAL_OK) {
		// Initialization Error
		Error_Handler();
	}

	// ����ʱ��
	datatmp.Year    = year;
	datatmp.Month   = month;
	datatmp.Date    = date;
	if (HAL_RTC_SetDate(&hrtc, &datatmp, RTC_FORMAT_BIN) != HAL_OK) {
		// Initialization Error
		Error_Handler();
	}
}

/**
  * @brief  ��ȡRTCʱ��
  * @param  
  * @retval 
  */
RTCTime_t *RTCTime_GetTime(void)
{
	RTC_DateTypeDef datatmp = { 0 };
	RTC_TimeTypeDef timetmp = { 0 };

	HAL_RTC_GetTime(&hrtc, &timetmp, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &datatmp, RTC_FORMAT_BIN);
	
	// �洢RTCʱ�����
	g_RTCTime.rtc.year   = datatmp.Year;
	g_RTCTime.rtc.month  = datatmp.Month;
	g_RTCTime.rtc.day    = datatmp.Date;
	g_RTCTime.rtc.hour   = timetmp.Hours;
	g_RTCTime.rtc.minute = timetmp.Minutes;
	g_RTCTime.rtc.second = timetmp.Seconds;
	
	return &g_RTCTime;
}

