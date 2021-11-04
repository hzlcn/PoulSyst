/* Includes ------------------------------------------------------------------*/
#include "param.h"

/* Private variables ---------------------------------------------------------*/

/**
  * 错误状态缓冲区
  */
ErrorStatus_t g_errorStatus = false;

/**
  * 错误列表
  */
uint8_t g_err_List[20] = { 0 };


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  获取错误状态指针
  * @param  
  * @retval 
  */
ErrorStatus_t *Param_GetErrorStatus(void)
{
	return &g_errorStatus;
}

/**
  * @brief  初始化错误状态
  * @param  
  * @retval 
  */
void Error_Init(void)
{
	Error_Add(ERROR_DEVICE_UNCOMMUNICATE);
	Error_Add(ERROR_GPS_UNCOMMUNICATE);
	Error_Add(ERROR_GPS_NO_SVDATA);
	Error_Add(ERROR_GPS_NO_LOCATION);
}

/**
  * @brief  添加错误
  * @param  
  * @retval 
  */
void Error_Add(ErrStatus_t status)
{
	// 查看该错误是否已经存在
	if (g_err_List[status >> 3] & (1 << (status & 0x07))) {
		return ;
	}
	
	g_err_List[status >> 3] |= (1 << (status & 0x07));
}

/**
  * @brief  删除错误
  * @param  
  * @retval 
  */
void Error_Del(ErrStatus_t status)
{
	// 查看该错误是否已经存在
	if (g_err_List[status >> 3] & (1 << (status & 0x07))) {
		g_err_List[status >> 3] &= ~(1 << (status & 0x07));
	}
}

/**
  * @brief  获取优先值最高的错误
  * @param  
  * @retval 
  */
ErrStatus_t Error_GetError(void)
{
	uint8_t i, j;
	for (i = 0; i < 20; i++) {
		for (j = 0; j < 8; j++) {
			if (g_err_List[i] & (1 << j)) {
				return (ErrStatus_t)(i * 8 + j);
			}
		}
	}
	return ERROR_NONE;
}
