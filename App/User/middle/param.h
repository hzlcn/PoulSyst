#ifndef __PARAM_H__
#define __PARAM_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"


/**
  * 错误状态类型
  */
typedef bool ErrorStatus_t;

typedef enum {
	ERROR_NONE = 0,
	ERROR_DEVICE_UNCOMMUNICATE,         // PLC无通讯
	ERROR_GPS_UNCOMMUNICATE,   // GPS无信息
	ERROR_USERQRCODE_UNCOMMUNICATE,      // 外扫码器出错
	ERROR_POULQRCODE_UNCOMMUNICATE,      // 内扫码器出错
	ERROR_POULQRCODE_NO_CODE,         // 无食品信息
	ERROR_GPS_NO_SVDATA,       // 无卫星信息
	ERROR_GPS_NO_LOCATION,       // 无定位信息
	ERROR_PLC_UNLOCK,       // 未关锁
	
}ErrStatus_t;

/* Private macro -------------------------------------------------------------*/

// 总标签量 固定值  需要修改
#define LABEL_TOTAL_NUMBER 									8888

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  获取错误状态指针
  */
ErrorStatus_t *Param_GetErrorStatus(void);

/**
  * @brief  初始化错误状态
  */
void        Error_Init(void);

/**
  * @brief  添加错误
  */
void        Error_Add(ErrStatus_t status);

/**
  * @brief  删除错误
  */
void        Error_Del(ErrStatus_t status);

/**
  * @brief  获取优先值最高的错误
  */
ErrStatus_t Error_GetError(void);

#endif /* __PARAM_H__ */
