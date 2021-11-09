#ifndef __COMMUNICATE_H__
#define __COMMUNICATE_H__

/* Includes ------------------------------------------------------------------*/

#include "common.h"

/* Private typedef -----------------------------------------------------------*/

// 服务器通讯协议 功能选项
typedef enum {
	UL_FUNC_NONE = 0,
	UL_DEVICE_JOIN,
	UL_USERMSG_UPDATE,
	UL_POULTYPE_UPDATE,
	UL_FIRMWARE_UPDATE,
	UL_USER_LOGIN,
	UL_HEARTBEAT,
	UL_FUNC_7,
	UL_FUNC_8,
	UL_FUNC_9,
	UL_FUNC_A,
}CMNC_CmdType_t;

typedef struct {
	CMNC_CmdType_t cmdType;
	void (*cmdSolve)(u8 *pBuf, u16 len);
}CMNCSolve_t;

/* Private macro -------------------------------------------------------------*/

#define MERCHANT_MESSAGE_SIZE  24

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  初始化协议层
  */
void CMNC_Init(void);

/**
  * @brief  处理协议层数据
  */
void CMNC_Process(void);

/**
  * @brief  处理服务器数据
  */
void CMNC_DLProcess(void);

/**
  * @brief  添加协议指令到指令队列
  */
void CMNC_AddCmd(uint8_t cmd);

void CMNC_01_Solve(u8 *pBuf, u16 len);
void CMNC_02_Solve(u8 *pBuf, u16 len);
void CMNC_03_Solve(u8 *pBuf, u16 len);
void CMNC_04_Solve(u8 *pBuf, u16 len);
void CMNC_05_Solve(u8 *pBuf, u16 len);
void CMNC_06_Solve(u8 *pBuf, u16 len);
void CMNC_07_Solve(u8 *pBuf, u16 len);
void CMNC_08_Solve(u8 *pBuf, u16 len);
void CMNC_09_Solve(u8 *pBuf, u16 len);
void CMNC_0A_Solve(u8 *pBuf, u16 len);

/**
  * @brief  设备登录服务器
  * @param  timeout	登录超时时间
  * @retval 
  */
int Proto_Join(u32 timeout);


#endif /*__COMMUNICATE_H__ */
