#ifndef __DISPLAY_H__
#define __DISPLAY_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "lcd.h"

/* Private typedef -----------------------------------------------------------*/

/**
  * ��½������ʾ�ı�����
  */
typedef enum {
	TEXT_NULL = 0,          // ��    ��    ��
	TEXT_LOGIN,             // "���̻���¼"
	TEXT_CHECK_NETWORK,     // ���������硱
	TEXT_DEVICE_UNLOGIN,    // ���豸δ��½��
	TEXT_CHECK_PLC,         // �����������
	TEXT_RECARD,            // ��������ˢ����
	TEXT_GETTING,           // "  ��ȡ��  "
	TEXT_LINKING_NETWORK,
	TEXT_LINKING_DEVICER,
	TEXT_INPUT_CUSTOMER,
	TEXT_GPS_DISABLE,       // ����λδ��ȡ��
	TEXT_MERCHANT_UNSIGNIN, // ���̻�δע�ᡱ
	TEXT_CUSTOMER_UNSIGNIN, // ���ͻ�δע�ᡱ
	TEXT_MERCHANT_LOGINING, // ���̻���¼�С�
	TEXT_LOGIN_FAIL,
	TEXT_CUSTOMER_LOGINING,	// ���ͻ���¼�С�
}ShowTextType_t;

/**
  * ��ʾ�ı���������
  */
typedef uint16_t ShowText_t[5];

/**
  * ������Ϣ��������
  */
typedef uint8_t ErrorText_t[12];

/**
  * ��ʾ�ı���Ϣ�ṹ��
  */
typedef struct {
	ShowTextType_t type;
	ShowText_t text[16];
}ShowTextInfo_t;

/**
  * ��ʾҳ������
  */
typedef enum {
	DISPLAY_PAGE_LOGIN      = 0,
	DISPLAY_PAGE_WORK       = 1,
	DISPLAY_PAGE_ADD        = 2,
	DISPLAY_PAGE_SELECT     = 3,
	DISPLAY_PAGE_DELETE     = 4,
	DISPLAY_PAGE_ADMIN      = 6,
	DISPLAY_PAGE_ERROR      = 8,
}Display_PageType_t;

/**
  * ��ʾ����
  */
typedef enum {
	DISPLAY_SHOW_RTCTIME,       // ��ǰʱ��
	DISPLAY_SHOW_LABELNUM,      // ��ǩ��
	DISPLAY_SHOW_GPSSNR,        // GPS�ź�ǿ��
	DISPLAY_SHOW_NETICON,       // ����ͼ��
	DISPLAY_SHOW_ERRCODE,       // ������
	DISPLAY_SHOW_WORKPAGE,      // ����ҳ��
	DISPLAY_SHOW_LOGINPAGE,     // ��¼ҳ��
	DISPLAY_SHOW_ADMINPAGE,     // ����Աҳ��
	DISPLAY_SHOW_ADDPAGE,       // ���ҳ��
}Display_ShowType_t;

/**
  * ѡ����ʾ�Ĳ����ṹ��
  */
typedef struct {
	/**
	  * ��ǰ��ʾѡ��ҳ�룬�ӵ�0ҳ��ʼ
	  */
	uint8_t page;
	
	/**
      * ѡ����ҳ��
      */
	uint8_t pageSum;
}Display_Opt_t;

/* Private macro -------------------------------------------------------------*/

#define OPTION_NUMBER                    8

/**
  * ��ʾ����ͼ������
  */
#define DISPLAY_ICON_NWK_NONE            0x0000
#define DISPLAY_ICON_NWK_NET             0x0001
#define DISPLAY_ICON_NWK_SIM             0x0002

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
