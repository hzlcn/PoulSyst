#ifndef __DISPLAY_H__
#define __DISPLAY_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "lcd.h"
#include "datastruct.h"

/* Private macro -------------------------------------------------------------*/

#define OPTION_NUMBER                    8

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  显示初始化
  */
void        Display_Init              (void);

/**
  * @brief  设置显示时间
  * @param  
  * @retval 
  */
int Disp_SetTime(void);

/**
  * @brief  设置显标签数
  * @param  
  * @retval 
  */
int Disp_SetLabel(void);

/**
  * @brief  设置显示GPS信号
  * @param  
  * @retval 
  */
int Disp_SetGPSSNR(void);

/**
  * @brief  设置显示网络信号
  * @param  
  * @retval 
  */
int Disp_SetNetIcon(void);

/**
  * @brief  设置显示错误提示
  * @param  
  * @retval 
  */
int Disp_SetErrTip(void);

/**
  * @brief  设置显示
  */
void        Display_SetShow           (Display_ShowType_t showType);

/**
  * @brief  设置显示登录界面文本
  */
void        Display_SetLoginText (ShowTextType_t textType);

/**
  * @brief  显示添加/选择/删除界面的按键文本
  */
void Display_ShowOptions(uint16_t addr);

void Disp_SetList(PoulListGroup_t group, PoulBase_t *base);

#endif //__DISPLAY_H__
