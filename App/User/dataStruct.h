#ifndef __DATASTRUCT_H__
#define __DATASTRUCT_H__

#include "common.h"
#include "serial.h"
// ��ͷ�ļ����ڶ����û����ݽṹ

/* Private define ------------------------------------------------------------*/

/**
  * ����û���Ϣ��
  */
#define N_MERCBASE_MAX_INFO 30

/**
  * �û�(������ͻ�)�Ķ�ά�볤��
  */
#define USER_CODE_SIZE      20

/**
  * ʳƷ��Ϣ���С
  */
#define N_POULTRY_QRCODE_BUFFER_SIZE    24

/**
  * ���ʳƷ��Ϣ����
  */
#define N_POULTRY_MAX_KIND              30

/**
  * ʳƷ��Ϣ - ������ų���[6] = �������[2] + С�����[4]
  */
#define N_POULTRY_KIND_NUMS_SIZE        6

/* Private typedef -----------------------------------------------------------*/
/**
  * ��ʾ�ı���������
  */
typedef uint16_t ShowText_t[5];

/**
  * ������Ϣ��������
  */
typedef uint8_t ErrorText_t[12];

/**
  * �û�Ȩ������
  */
typedef enum {
	USER_PERM_ADMIN         = 0,    // ����Ա
	USER_PERM_MAINTAIN      = 1,    // ά��Ա
	USER_PERM_MERCHANT      = 2,    // ��ͨ�û�
	USER_PERM_CUSTOMER      = 3,    // ��ͨ�ͻ�
	USER_PERM_NULL          = 0xFF, // ��Ȩ��
}UserPerm_t;

/**
  * ʳƷ�б�����
  */
typedef enum {
	POUL_LISTTYPE_NONE = 0,
	POUL_LISTTYPE_CUR  = 1,             // ʳƷ��Ϣ�б�
	POUL_LISTTYPE_ADD  = 2,             // �����Ϣ�б�
}PoulListType_t;

/**
  * ʳƷ����
  */
typedef enum {
	POUL_LISTGROUPTYPE_CHICK = 0,       // ��
	POUL_LISTGROUPTYPE_DUCK  = 1,       // Ѽ
	POUL_LISTGROUPTYPE_GOOSE = 2,       // ��
	POUL_LISTGROUPTYPE_OTHER = 3,       // ����
	POUL_LISTGROUPTYPE_NONE  = 4,       // ��
}PoulListGroup_t;

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
  * ����������Ϣ�ṹ��
  */
typedef struct {
	UserPerm_t perm;
	char code[USER_CODE_SIZE];
}Merchant_t;

/**
  * �����ͻ���Ϣ�ṹ��
  */
typedef struct {
	UserPerm_t perm;
	char code[USER_CODE_SIZE];
	u8 name[20];
}Customer_t;

/**
  * ������Ϣ��ṹ��
  */
typedef struct {
	uint8_t keepFlag;
	uint8_t Version[2];             // ��汾
	uint8_t Sum;                    // �����Ϣ��
	struct {
		Merchant_t Info[N_MERCBASE_MAX_INFO];
	}List;                          // �����Ϣ�б�
}MercBase_t;

/**
  * �û���ά������ṹ��
  */
typedef struct {
	bool isReady;
	uint8_t code[UART5_BUFFSIZE];
}RxQrcode_t;

/**
  * �û���Ϣ�ṹ��
  */
typedef struct {
	Merchant_t merc;
	Customer_t cust;
	RxQrcode_t recv;
	MercBase_t *mercBase;
}UserInfo_t;

/* Private typedef -----------------------------------------------------------*/

/**
  * ʳƷ������Ϣ�ṹ��
  */
typedef struct {
	// ǰ2�ֽ�ΪPoulListGroup_t���ͣ���4�ֽ�Ϊ��ϸ�������(����: ǰ2�ֽڱ�ʾ������4�ֽڱ�ʾɽ�ؼ���)
	u8 kindNum[N_POULTRY_KIND_NUMS_SIZE];
	
	// ʳƷ���������(GBK����)
	u8 name[10];
}PoulKind_t;

/**
  * ʳƷ�����
  */
typedef PoulKind_t PoulKinds_t[N_POULTRY_MAX_KIND];

/**
  * ʳƷ��Ϣ��ṹ��
  */
typedef struct {
	u8 keepFlag;
	u8 version[2];
	u8 sum;
	PoulKinds_t kinds;
}PoulBase_t;

/**
  * ʳƷ�����б�
  */
typedef struct {
	// ��ǰ�б�����
	PoulListType_t type;
	
	// ʳƷ����
	PoulListGroup_t group;
	
	// ��Ϣ����
	u8 sum;
	
	// ʳƷ��Ϣ�������б�
	PoulKind_t *pKinds[N_POULTRY_MAX_KIND];
}PoulList_t;

/**
  * ʳƷ������Ϣ�ṹ��
  */
typedef struct {
	bool isReady;
	u8   qrcode[N_POULTRY_QRCODE_BUFFER_SIZE];
}PoulCode_t;

/**
  * ʳƷ������Ϣ�ṹ��
  */
typedef struct {
	PoulKind_t *pCurKind;		// ��ǰʳƷ����
	PoulCode_t  code;           // ʳƷ��ά��
	PoulList_t  list;           // ʳƷ�б�
	PoulBase_t *base[2];		// ʳƷ�����
}PoulInfo_t;

/**
  * GPS������Ϣ�ṹ��
  */
typedef struct {
	char svNum[2];          // �ɼ�������
	char svSnr[2];          // �����
}GPS_SVParam_t;

/**
  * ��ʾ�ı���Ϣ�ṹ��
  */
typedef struct {
	ShowTextType_t type;
	ShowText_t text[16];
}ShowTextInfo_t;

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

/* Private typedef -----------------------------------------------------------*/
/**
  * ��ʱ���ݽṹ��
  */
typedef struct sTempPara{
	UserInfo_t user;
	PoulInfo_t poul;
	GPS_SVParam_t gps;
}TempPara_t;

/**
  * �洢���ݽṹ��
  */
typedef struct sSavePara{
	int flag;
}SavePara_t;

/**
  * ���ݽṹ��
  */
typedef struct {
	TempPara_t temp;
	SavePara_t save;
}DataStruct_t;

extern DataStruct_t g_allData;
extern SavePara_t *g_savPara;
extern TempPara_t *g_tmpPara;

#endif /* __DATASTRUCT_H__ */
