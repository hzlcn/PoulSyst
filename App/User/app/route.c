#include "route.h"
#include "param.h"
#include "apps.h"
#include "display.h"
#include "usermsg.h"
#include "poultry.h"
#include "drivers.h"
#include "sim.h"
#include "param.h"
#include "upgrade.h"

/* Private define ------------------------------------------------------------*/
#define DBG_TRACE                                   1

#if DBG_TRACE == 1
    #include <stdio.h>
    /*!
     * Works in the same way as the printf function does.
     */
    #define DBG( ... )                               \
        do                                           \
        {                                            \
            printf( __VA_ARGS__ );                   \
        }while( 0 )
#else
    #define DBG( fmt, ... )
#endif

/* Private macro -------------------------------------------------------------*/

#define ROUTE_TRIGGER_TIMEOUT    3000	// ����һ�κ���Ҫ�ȴ�4������ٴδ���
#define ROUTE_LOGIN_TIMEOUT      3000   // ��¼��������Ҫ�ȴ�3����ܵ�¼�ͻ�

/* Private variables ---------------------------------------------------------*/

/**
  * ���̲���
  */
static RouteInfo_t g_route_Info = { 
	.status = ROUTE_INIT_MODE,
	.loginStatus = false,
	.trig = false,
	.updateSoft = false,
};

/**
  * ��ʱ����
  */
static uint32_t g_routeTick = 0;
static TickTimer_t g_routeTrigTimeout;
static TickTimer_t g_routeLogin1Time;
static TickTimer_t g_routeLogin2Time;
static TickTimer_t g_routeWhile1sTime = { .start = 0, .count = 1000 };
static TickTimer_t g_routeWhile5sTime = { .start = 0, .count = 5000 };

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  �����û���¼����
  */
static void Route_UserLogin(void);

/**
  * @brief  ����ϵͳ����
  * @param  
  * @retval 
  */
void Route_Process(void)
{
	Merchant_t *merchant = &User_GetInfo()->merc;
	Customer_t *customer = &User_GetInfo()->cust;
	PoulCode_t *poulCode = &Poul_GetInfo()->code;
	ModbusInfo_t *modbusInfo = Modbus_GetParam();

	g_routeTick = HAL_GetTick();

	switch (g_route_Info.status) {
		case ROUTE_INIT_MODE:
		{   // ��ʼ��
			g_route_Info.status = ROUTE_DEVICE_LINK;
			Display_SetLoginText(TEXT_DEVICE_UNLOGIN);
			
//			g_route_Info.status = ROUTE_MERCHANT_LOGIN;
//			Display_SetLoginText(TEXT_LOGIN);   // ��ʾ"���̻���¼"
//			User_InitMerc();                    // ��ʼ������
			break;
		}
		case ROUTE_DEVICE_LINK:
		{
			if (modbusInfo->devJoin == true) {
				if (modbusInfo->regList[7].data == 0x0000) {
					g_route_Info.status = ROUTE_MERCHANT_LOGIN;
					Display_SetLoginText(TEXT_LOGIN);   // ��ʾ"���̻���¼"
					User_InitMerc();                    // ��ʼ������
				}
			}
			break;
		}
		case ROUTE_MERCHANT_LOGIN:
		{
			Route_UserLogin();
			break;
		}
		case ROUTE_CUSTOMER_LOGIN:
		{			
			Route_UserLogin();
			break;
		}
		case ROUTE_PRODUCT_SHOP:
		{
			// ������괥��
			if (READ_SP0 == GPIO_PIN_RESET) {
				if (g_route_Info.trig == true) {
					g_route_Info.trig = false;

					// 1.��������
					Beep_Run(50);

					// 2.�������������ǩ
					Modbus_AddCmd(MODBUS_CMD_TRIGGER_LABEL);

					// 3.���潻����Ϣ
					if (merchant->perm != USER_PERM_MAINTAIN)  {
						Store_SaveBlock();
						Store_SaveRecordNum();
					}

					// 4.�������ʱ��
					g_routeTrigTimeout.start = g_routeTick;
					g_routeTrigTimeout.count = ROUTE_TRIGGER_TIMEOUT;
				}
			} else {
				if ((poulCode->isReady == true) &&		// �����Ʒ��״̬
					(g_routeTrigTimeout.count == 0))		// �������ʱ���ʱ���
				{
					g_route_Info.trig = true;
				}
			}

			// ���ͻ��Ƿ��˳�
			if (customer->perm == USER_PERM_NULL) {
				User_InitCust();
				Display_SetLoginText(TEXT_LOGIN);

				g_routeTrigTimeout.start = 0;
				g_routeTrigTimeout.count = 0;

				g_route_Info.status = ROUTE_CUSTOMER_LOGIN;
			}

			// ��鶨ʱ����
			if ((g_routeTrigTimeout.count > 0) && (g_routeTick - g_routeTrigTimeout.start >= g_routeTrigTimeout.count)) {
				g_routeTrigTimeout.start = 0;
				g_routeTrigTimeout.count = 0;
			}
			break;
		}
		case ROUTE_ADMIN_MODE:
		{
			if (Display_GetPage() == DISPLAY_PAGE_WORK) {
				// ��ʾ����ҳ��
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);
				DBG("product shop.\r\n");
				
				g_route_Info.status = ROUTE_PRODUCT_SHOP;
			}
			break;
		}
		default:
			break;
	}

	// 1�봦��
	if (g_routeTick - g_routeWhile1sTime.start >= g_routeWhile1sTime.count){
		g_routeWhile1sTime.start = g_routeTick;

		// ��ʾ������ ----------------------------------------------------------
		Display_SetShow(DISPLAY_SHOW_ERRCODE);

		// ��ʾ��Ч��ǩ�� ------------------------------------------------------
		Display_SetShow(DISPLAY_SHOW_LABELNUM);

		// ��������ͼ�� --------------------------------------------------------
		Display_SetShow(DISPLAY_SHOW_NETICON);
	}
	
	// 5s����
	if (g_routeTick - g_routeWhile5sTime.start >= g_routeWhile5sTime.count){
		g_routeWhile5sTime.start = g_routeTick;
		
		// ��ʾGPS��Ϣ ---------------------------------------------------------
		Display_SetShow(DISPLAY_SHOW_GPSSNR);
		
		// ��ʾʱ�� ------------------------------------------------------------
		Display_SetShow(DISPLAY_SHOW_RTCTIME);
				
		// ������� ------------------------------------------------------------
		if (g_route_Info.updateSoft == true) {
			if (Upg_GetInfo()->firmIndex < Upg_GetInfo()->firmSize) {
				CMNC_AddCmd(UL_FUNC_A);
			} else {
				// �л����
			}
		}
	}	
}

/**
  * @brief  ��ȡ���̲���ָ��
  * @param  
  * @retval 
  */
RouteInfo_t *Route_GetInfo(void)
{
	return &g_route_Info;
}

/**
  * @brief  ��ȡ����״̬
  * @param  
  * @retval 
  */
void Route_SetLogin(void)
{
	g_route_Info.loginStatus = true;
}

void Route_SetUpdate(void)
{
	g_route_Info.updateSoft = true;
}

/**
  * @brief  �����û���¼����
  * @param  
  * @retval 
  */
static void Route_UserLogin(void)
{
	Merchant_t *merchant = &User_GetInfo()->merc;
	Customer_t *customer = &User_GetInfo()->cust;
	UserPerm_t userperm = USER_PERM_NULL;
	if (g_route_Info.status == ROUTE_MERCHANT_LOGIN) {
		userperm = merchant->perm;
	} else if (g_route_Info.status == ROUTE_CUSTOMER_LOGIN) {
		userperm = customer->perm;
	}
		
	if (g_routeLogin1Time.count == 0) {
		if (g_routeLogin2Time.count == 0) {
			// ����¼
			if (User_GetInfo()->recv.isReady == true) {
				
				// ���ؼ���
				if (g_route_Info.status == ROUTE_MERCHANT_LOGIN) {
					if (User_CheckLocal() == true) {
						g_route_Info.loginStatus = true;
					} else {
						// Զ�̵�¼
						CMNC_AddCmd(UL_USER_LOGIN);

						g_routeLogin1Time.start = g_routeTick;
						g_routeLogin1Time.count = ROUTE_LOGIN_TIMEOUT;
						Display_SetLoginText(TEXT_MERCHANT_LOGINING);
					}
				} else if (g_route_Info.status == ROUTE_CUSTOMER_LOGIN) {
					// Զ�̵�¼
					CMNC_AddCmd(UL_USER_LOGIN);
					g_routeLogin1Time.start = g_routeTick;
					g_routeLogin1Time.count = ROUTE_LOGIN_TIMEOUT;
					Display_SetLoginText(TEXT_CUSTOMER_LOGINING); 
				}
			}
		} else {
			if (g_routeTick - g_routeLogin2Time.start >= g_routeLogin2Time.count) {
				g_routeLogin2Time.start = 0;
				g_routeLogin2Time.count = 0;

				// ��λ��¼
				if (g_route_Info.status == ROUTE_MERCHANT_LOGIN) {
					Display_SetLoginText(TEXT_LOGIN);
				} else if (g_route_Info.status == ROUTE_CUSTOMER_LOGIN) {
					Display_SetLoginText(TEXT_INPUT_CUSTOMER);
				}
			}
		}
	} else {
		// ������¼��
		
		if (userperm != USER_PERM_NULL) {
			// ��¼�ɹ�
			g_routeLogin1Time.start = 0;
			g_routeLogin1Time.count = 0;
			
			// ����û�������
			User_ClearRecv();
			if (g_route_Info.status == ROUTE_MERCHANT_LOGIN) {
				if ((merchant->perm == USER_PERM_ADMIN) || (merchant->perm == USER_PERM_MAINTAIN))  {
					g_route_Info.status = ROUTE_ADMIN_MODE;
					Display_SetShow(DISPLAY_SHOW_ADMINPAGE);// ��ʾ����Ա����
					User_SetCustAdmin();
				} else if (merchant->perm == USER_PERM_MERCHANT) {
					g_route_Info.status = ROUTE_CUSTOMER_LOGIN;
					Display_SetLoginText(TEXT_INPUT_CUSTOMER);// ��ʾ�ͻ���¼
					User_InitCust();
				}
			} else if (g_route_Info.status == ROUTE_CUSTOMER_LOGIN) {
				g_route_Info.status = ROUTE_PRODUCT_SHOP;
				Display_SetShow(DISPLAY_SHOW_WORKPAGE);
			}
		} else if (g_routeTick - g_routeLogin1Time.start >= g_routeLogin1Time.count) {
			g_routeLogin1Time.start = 0;
			g_routeLogin1Time.count = 0;
			
			// ��¼��ʱʧ�ܣ�����û�����������λ��¼״̬
			g_routeLogin2Time.start = g_routeTick;
			g_routeLogin2Time.count = ROUTE_LOGIN_TIMEOUT;		
			User_ClearRecv();
			Display_SetLoginText(TEXT_LOGIN_FAIL);
		} 
	}
}
