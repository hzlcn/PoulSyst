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
#include "ethernet.h"
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

/* Private variables ---------------------------------------------------------*/
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
	g_appTick = HAL_GetTick();
	
	
	// �����ά������
	Poul_Process();
	User_Process();

	// ���Կ���Ϣ����
	Debug_Process();
	
	// ��ʾLCD���ݴ���
	LCD_Process();
	Display_Process();

	// Modbus�ķ��ͽ��մ���
	Modbus_Process();
			
	// GPRS���ݽ���
	GPS_Process();
	
	// �������緢�͡����պͶ������
	Network_Process();
	if ((nwkStatus == NETWORK_STATUS_NET) || (nwkStatus == NETWORK_STATUS_SIM)) {
		// �������ͨѶ
		CMNC_Process();
	}

	// ��������: �豸��¼->�û���¼->�ͻ���¼->��Ʒ����
	Route_Process();

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
		
		// ��ȡ����״̬ + ��ǩ��
		Modbus_AddCmd(MODBUS_CMD_GETLOCKSTATUS);
		Modbus_AddCmd(MODBUS_CMD_READ_VALIBNUM);
		Modbus_AddCmd(MODBUS_CMD_READ_INVALNUM);
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
