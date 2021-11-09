#ifndef __COMMUNICATE_H__
#define __COMMUNICATE_H__

/* Includes ------------------------------------------------------------------*/

#include "common.h"

/* Private typedef -----------------------------------------------------------*/

// ������ͨѶЭ�� ����ѡ��
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
  * @brief  ��ʼ��Э���
  */
void CMNC_Init(void);

/**
  * @brief  ����Э�������
  */
void CMNC_Process(void);

/**
  * @brief  �������������
  */
void CMNC_DLProcess(void);

/**
  * @brief  ���Э��ָ�ָ�����
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
  * @brief  �豸��¼������
  * @param  timeout	��¼��ʱʱ��
  * @retval 
  */
int Proto_Join(u32 timeout);


#endif /*__COMMUNICATE_H__ */
