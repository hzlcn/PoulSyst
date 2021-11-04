#ifndef __GPS_H__
#define __GPS_H__

#include "common.h"

/**
  * GPS卫星信息结构体
  */
typedef struct {
	char svNum[2];          // 可见卫星数
	char svSnr[2];          // 信躁比
}GPS_SVParam_t;

/**
  * @brief  开关GPS功能
  */
void GPS_Init(void);

/**
  * @brief  循环处理GPS数据
  */
void GPS_Process(void);

/**
  * @brief  获取卫星信息
  */
GPS_SVParam_t *GPS_GetSVDate(void);

/**
  * @brief  判断卫星信息是否已经获取
  */
bool GPS_SVIsReady(void);

/**
  * @brief  获取定位信息
  */
bool GPS_GetLLData(uint8_t *pBuffer);

/**
  * @brief  判断定位信息是否已经获取
  */
bool GPS_LLIsReady(void);


#endif /* __GPS_H__ */
