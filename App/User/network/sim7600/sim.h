#ifndef __SIM_H__
#define __SIM_H__

#include "common.h"

#define PCIE_PWR_ON     HAL_GPIO_WritePin(MPCIE_PWR_GPIO_Port, MPCIE_PWR_Pin, GPIO_PIN_SET)
#define PCIE_PWR_OFF    HAL_GPIO_WritePin(MPCIE_PWR_GPIO_Port, MPCIE_PWR_Pin, GPIO_PIN_RESET)

// 非SIM线程调用函数
void Sim_PutData(uint8_t *pBuffer, uint16_t len);

uint16_t Sim_GetData(uint8_t *buffer);

bool Sim_GetLinkStatus(void);

// SIM线程调用函数
bool Sim_Init(void);
void Sim_Process(void);

#endif /* __SIM_H__ */
