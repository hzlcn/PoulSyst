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
  * @brief  ��ʾ��ʼ��
  */
void        Display_Init              (void);

/**
  * @brief  ������ʾʱ��
  * @param  
  * @retval 
  */
int Disp_SetTime(void);

/**
  * @brief  �����Ա�ǩ��
  * @param  
  * @retval 
  */
int Disp_SetLabel(void);

/**
  * @brief  ������ʾGPS�ź�
  * @param  
  * @retval 
  */
int Disp_SetGPSSNR(void);

/**
  * @brief  ������ʾ�����ź�
  * @param  
  * @retval 
  */
int Disp_SetNetIcon(void);

/**
  * @brief  ������ʾ������ʾ
  * @param  
  * @retval 
  */
int Disp_SetErrTip(void);

/**
  * @brief  ������ʾ
  */
void        Display_SetShow           (Display_ShowType_t showType);

/**
  * @brief  ������ʾ��¼�����ı�
  */
void        Display_SetLoginText (ShowTextType_t textType);

/**
  * @brief  ��ʾ���/ѡ��/ɾ������İ����ı�
  */
void Display_ShowOptions(uint16_t addr);

void Disp_SetList(PoulListGroup_t group, PoulBase_t *base);

#endif //__DISPLAY_H__
