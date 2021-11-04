#include "rtctime.h"
#include "rtc.h"
#include "param.h"

/* Private variables ---------------------------------------------------------*/

/**
  * RTC时间缓冲区
  */
RTCTime_t g_RTCTime;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  设置RTC时间
  * @param  year    年
  *         month   月
  *         day     日
  *         hour    时
  *         minute  分
  *         second  秒
  * @retval 
  */
void RTCTime_SetTime(uint8_t year, uint8_t month,  uint8_t date,
                      uint8_t hour, uint8_t minute, uint8_t second)
{
	RTC_TimeTypeDef timetmp = { 0 };
	RTC_DateTypeDef datatmp = { 0 };

	// 存储RTC时间参数
	g_RTCTime.rtc.year   = year;
	g_RTCTime.rtc.month  = month;
	g_RTCTime.rtc.day    = date;
	g_RTCTime.rtc.hour   = hour;
	g_RTCTime.rtc.minute = minute;
	g_RTCTime.rtc.second = second;
	
	// 设置日期
	timetmp.Hours   = hour;
	timetmp.Minutes = minute;
	timetmp.Seconds = second;
	timetmp.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	timetmp.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &timetmp, RTC_FORMAT_BIN) != HAL_OK) {
		// Initialization Error
		Error_Handler();
	}

	// 设置时间
	datatmp.Year    = year;
	datatmp.Month   = month;
	datatmp.Date    = date;
	if (HAL_RTC_SetDate(&hrtc, &datatmp, RTC_FORMAT_BIN) != HAL_OK) {
		// Initialization Error
		Error_Handler();
	}
}

/**
  * @brief  获取RTC时间
  * @param  
  * @retval 
  */
RTCTime_t *RTCTime_GetTime(void)
{
	RTC_DateTypeDef datatmp = { 0 };
	RTC_TimeTypeDef timetmp = { 0 };

	HAL_RTC_GetTime(&hrtc, &timetmp, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &datatmp, RTC_FORMAT_BIN);
	
	// 存储RTC时间参数
	g_RTCTime.rtc.year   = datatmp.Year;
	g_RTCTime.rtc.month  = datatmp.Month;
	g_RTCTime.rtc.day    = datatmp.Date;
	g_RTCTime.rtc.hour   = timetmp.Hours;
	g_RTCTime.rtc.minute = timetmp.Minutes;
	g_RTCTime.rtc.second = timetmp.Seconds;
	
	return &g_RTCTime;
}

