#ifndef __POULTRY_H__
#define __POULTRY_H__

#include "common.h"
#include "serial.h"
#include "dataStruct.h"


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  初始化食品二维码模块
  */
void        Poul_InitModule(void);

/**
  * @brief  处理食品二维码数据(串口2)
  */
void        Poul_Process(void);

/**
  * @brief  获取食品类型信息参数地址指针
  */
PoulInfo_t *Poul_GetInfo(void);

/**
  * @brief  清除食品二维码
  */
void        Poul_ClrCode(void);

/**
  * @brief  更新食品种类指针列表
  */
void        Poul_SetList(PoulListGroup_t group, PoulListType_t type);

/**
  * @brief  设置当前食品种类
  */
void        Poul_SetKind(PoulKind_t *kind);

/**
  * @brief  添加单条食品信息到内部禽类库
  */
void        Poul_AddKind(PoulKind_t *kind);

/**
  * @brief  通过索引从内部禽类库中删除单条食品信息
  */
void        Poul_DelKind(PoulKind_t *kind);

/**
  * @brief  设置食品信息库为初始值
  */
void        Poul_DefBase(void);

#endif /* __PRODUCT_H__ */
