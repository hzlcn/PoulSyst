#ifndef __DISPLAY_H__
#define __DISPLAY_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "lcd.h"
#include "dataStruct.h"

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  显示初始化
  */
void        Display_Init              (void);

/**
  * @brief  显示数据处理
  */
void        Display_Process           (void);

/**
  * @brief  设置显示
  */
void        Display_SetShow           (Display_ShowType_t showType);

/**
  * @brief  读取显示页面
  */
Display_PageType_t Display_GetPage(void);

/**
  * @brief  设置显示登录界面文本
  */
void        Display_SetLoginText (ShowTextType_t textType);

#endif //__DISPLAY_H__
