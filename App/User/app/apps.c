/**
  ******************************************************************************
  * @file           : a_main.c
  * @brief          : Application main program body
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "apps.h"
#include "delay.h"
#include "debug.h"
#include "common.h"
#include "network.h"
#include "w5500_ex.h"
#include "display.h"
#include "modbus.h"
#include "w25qxx.h"
#include "route.h"
#include "param.h"
#include "serial.h"
#include "gps.h"
#include "store.h"
#include "usermsg.h"
#include "drivers.h"
#include "lcd.h"
#include "poultry.h"
#include "stmflash.h"
#include "datastruct.h"

/* Private variables ---------------------------------------------------------*/
/**
  * ������Ϣ��
  */
static MercBase_t g_mercBase = { 0 };

/**
  * ʳƷ��Ϣ��
  */
static PoulBase_t g_poulCurBase = { 0 };

/**
  * �����Ϣ��
  */
static PoulBase_t g_poulAddBase = { 0 };

DataStruct_t g_allData = {
	.temp = {
		.user = {
			.merc = {
				.perm = USER_PERM_NULL,
				.code = { 0 },
			},
			.cust = {
				.perm = USER_PERM_NULL,
				.code = { 0 },
				.name = { 0 },
			},
			.recv = {
				.isReady = false, 
				.code = { 0 },
			},
			.mercBase = &g_mercBase,
		},
		.poul =  {
			.pCurKind = &g_poulCurBase.kinds[0],
			.code = {
				.isReady = false, 
				.qrcode = { 0 },
			},
			.list = { 
				.type = POUL_LISTTYPE_NONE,
				.group = POUL_LISTGROUPTYPE_CHICK,
				.sum = 0,
				.pKinds = { 0 },
			},
			.base = {
				[0] = &g_poulCurBase,
				[1] = &g_poulAddBase,
			},
		},
		.gps = { 0 },
		.route = { 0 },
		.display = {
			.err = {
				[0] = { 0 },
				[1] = { 0x50, 0x4C, 0x43, 0xCE, 0xDE, 0xCD, 0xA8, 0xD1, 0xB6 },					// PLC��ͨѶ
				[2] = { 0x47, 0x50, 0x53, 0xCE, 0xDE, 0xD0, 0xC5, 0xCF, 0xA2 },					// GPS����Ϣ
				[3] = { 0xCD, 0xE2, 0xC9, 0xA8, 0xC2, 0xEB, 0xC6, 0xF7, 0xB3, 0xF6, 0xB4, 0xED }, // ��ɨ��������
				[4] = { 0xC4, 0xDA, 0xC9, 0xA8, 0xC2, 0xEB, 0xC6, 0xF7, 0xB3, 0xF6, 0xB4, 0xED },	// ��ɨ��������
				[5] = { 0xCE, 0xDE, 0xCA, 0xB3, 0xC6, 0xB7, 0xD0, 0xC5, 0xCF, 0xA2 },				// ��ʳƷ��Ϣ
				[6] = { 0xCE, 0xDE, 0xCE, 0xC0, 0xD0, 0xC7, 0xD0, 0xC5, 0xCF, 0xA2 },				// ��������Ϣ
				[7] = { 0xCE, 0xDE, 0xB6, 0xA8, 0xCE, 0xBB, 0xD0, 0xC5, 0xCF, 0xA2 },				// �޶�λ��Ϣ
				[8] = { 0xCE, 0xB4, 0xB9, 0xD8, 0xCB, 0xF8 },                                     // δ����
			},
			.wTxt = {
				[0] = { 0x2020, 0x2020, 0xCEDE, 0x2020, 0x2020 },		// TEXT_NULL                  ��    ��    ��
				[1] = { 0xC7EB, 0xC9CC, 0xBBA7, 0xB5C7, 0xC2BC },		// TEXT_LOGIN                 �����̻���¼��
				[2] = { 0xC7EB, 0xBCEC, 0xB2E9, 0xCDF8, 0xC2E7 },		// TEXT_CHECK_NETWORK         ���������硱
				[3] = { 0xC9E8, 0xB1B8, 0xCEB4, 0xB5C7, 0xC2BD },		// TEXT_DEVICE_UNLOGIN        ���豸δ��½��
				[4] = { 0xC7EB, 0xBFAA, 0xC6F4, 0xBBFA, 0xC6F7 },		// TEXT_CHECK_PLC             �����������
				[5] = { 0xC7EB, 0xD6D8, 0xD0C2, 0xCBA2, 0xBFA8 },		// TEXT_RECARD                ��������ˢ����
				[6] = { 0x2020, 0xBBF1, 0xC8A1, 0xD6D0, 0x2020 },		// TEXT_GETTING               ��  ��ȡ��  ��
				[7] = { 0xC1AC, 0xBDD3, 0xCDF8, 0xC2E7, 0xD6D0 },		// TEXT_NULL                  �����������С�
				[8] = { 0xC1AC, 0xBDD3, 0xC9E8, 0xB1B8, 0xD6D0 },		// TEXT_NULL                  �������豸�С�
				[9] = { 0xC7EB, 0xCAE4, 0xC8EB, 0xBFCD, 0xBBA7 },		// TEXT_NULL                  ��������ͻ���
				[10] = { 0xB6A8, 0xCEBB, 0xCEB4, 0xBBF1, 0xC8A1 },     // TEXT_GPS_DISABLE           ����λδ��ȡ��
				[11] = { 0xC9CC, 0xBBA7, 0xCEB4, 0xD7A2, 0xB2E1 },     // TEXT_MERCHANT_UNSIGNIN     ���̻�δע�ᡱ
				[12] = { 0xBFCD, 0xBBA7, 0xCEB4, 0xD7A2, 0xB2E1 },     // TEXT_CUSTOMER_UNSIGNIN     ���ͻ�δע�ᡱ
				[13] = { 0xC9CC, 0xBBA7, 0xB5C7, 0xC2BC, 0xD6D0 },     // TEXT_MERCHANT_LOGINING     ���̻���¼�С�
				[14] = { 0x20B5, 0xC7C2, 0xBCCA, 0xA7B0, 0xDC20 },     // TEXT_LOGIN_FAIL            �� ��¼ʧ�� ��
				[15] = { 0xBFCD, 0xBBA7, 0xB5C7, 0xC2BC, 0xD6D0 },     //TEXT_CUSTOMER_LOGINING      ���ͻ���¼�С�
			},
			.opt = { 0 },
		}
	},
};
SavePara_t *g_savPara = &g_allData.save;
TempPara_t *g_tmpPara = &g_allData.temp;

/**
  * ��ʱ����
  */
static uint32_t g_appTick = 0;
static TickTimer_t g_app50msTime = { .start = 0, .count = 50 };
static TickTimer_t g_app1sTime = { .start = 0, .count = 1000 };
static TickTimer_t g_app10sTime = { .start = 0, .count = 10000 };
static TickTimer_t g_app60sTime = { .start = 0, .count = 60000 };

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Initial app layer
  * @param  
  * @retval 
  */
void Main_Init(void)
{
	uint8_t i = 0;
	
	printf( "\r\n###### ====== Poultry System v1.2.0 ======= ######\r\n" );
	
	delay_init(180);
	HAL_TIM_Base_Start_IT(&htim2);
	
	// ��ȡ��ʼ������
	W25QXX_Init();
	Store_Init();

	// ��ʼ����ά��ģ��
	E3000_ON;
	Poul_InitModule();
	User_InitModule();

	// ��ʼ�����Դ�ӡ����
	Debug_Init();
	
	// ��ʼ����ʾ����ʾ����
	Display_Init();	

	// ��ʼ�������ͨѶ����
	Modbus_Init();

	// ��ʼ��Gps����
	GPS_Init();

	// ��ʼ��������ͨѶ
	Network_Init();
	CMNC_Init();
	
	// ��ʼ��������
	Error_Init();
	
	// ��ʾ��������ʱ5��
	for	(i = 0; i < 100; i++) {
		osDelay(50);
		Ex_WDI_Feed();
	}
	
	// ���µ�¼������ʾ
	Display_SetShow(DISPLAY_SHOW_LOGINPAGE);
}

/**
  * @brief  Process app layer
  * @param  
  * @retval 
  */
void Main_Process(void)
{
	NwkStatus_t nwkStatus = Network_GetStatus();
	
	// �����ά������
	Poul_Process();
	User_Process();

	// ���Կ���Ϣ����
	Debug_Process();
	
	// ��ʾLCD���ݴ���
	LCD_Process();

	// Modbus�ķ��ͽ��մ���
	//Modbus_Process();
			
	// GPRS���ݽ���
	GPS_Process();
	
	// �������緢�͡����պͶ������
	Network_Process();
	if ((nwkStatus == NETWORK_STATUS_NET) || (nwkStatus == NETWORK_STATUS_SIM)) {
		// �������ͨѶ
		CMNC_Process();
	}

	g_appTick = HAL_GetTick();
	
	// 50ms����
	if ((g_app50msTime.count > 0) && (g_appTick - g_app50msTime.start >= g_app50msTime.count)) {
		g_app50msTime.start = g_appTick;
		
		// �ⲿ���Ź�
		Ex_WDI_Feed();
	}
	
	// 1�봦��
	if ((g_app1sTime.count > 0) && (g_appTick - g_app1sTime.start >= g_app1sTime.count)) {
		g_app1sTime.start = g_appTick;
		
		// ����״̬��
		LED_TURN;
	}
	
	// 5�봦��
	if ((g_app10sTime.count > 0) && (g_appTick - g_app10sTime.start >= g_app10sTime.count)) {
		g_app10sTime.start = g_appTick;
		
		// �����Ϣ�ϱ� --------------------------------------------------------
		Store_Process();

	}
	
	// 60�봦��
	if ((g_app60sTime.count > 0) && (g_appTick - g_app60sTime.start >= g_app60sTime.count)) {
		g_app60sTime.start = g_appTick;		
		
		// ������ͨѶ - ������
		CMNC_AddCmd(UL_HEARTBEAT);
	}
}
