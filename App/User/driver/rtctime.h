#ifndef __RTCTIME_H__
#define __RTCTIME_H__

#include "common.h"


/**
  * RTC时间结构体
  */
typedef union {
	uint8_t value[6];
	struct {
		uint8_t year;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
	}rtc;
}RTCTime_t;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  设置RTC时间
  */
void        RTCTime_SetTime(uint8_t year, uint8_t month, uint8_t date,
                            uint8_t hour, uint8_t minute, uint8_t second);

/**
  * @brief  获取RTC时间
  */
RTCTime_t * RTCTime_GetTime(void);


#endif /* __RTCTIME_H__ */
