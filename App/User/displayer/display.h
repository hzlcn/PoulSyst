#ifndef __DISPLAY_H__
#define __DISPLAY_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "lcd.h"
#include "dataStruct.h"

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ��ʾ��ʼ��
  */
void        Display_Init              (void);

/**
  * @brief  ��ʾ���ݴ���
  */
void        Display_Process           (void);

/**
  * @brief  ������ʾ
  */
void        Display_SetShow           (Display_ShowType_t showType);

/**
  * @brief  ��ȡ��ʾҳ��
  */
Display_PageType_t Display_GetPage(void);

/**
  * @brief  ������ʾ��¼�����ı�
  */
void        Display_SetLoginText (ShowTextType_t textType);

#endif //__DISPLAY_H__
