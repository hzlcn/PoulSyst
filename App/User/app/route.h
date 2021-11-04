#ifndef __ROUTE_H__
#define __ROUTE_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Private typedef -----------------------------------------------------------*/

/**
  * ����״̬
  */
typedef enum {
	ROUTE_INIT_MODE,
	ROUTE_DEVICE_LINK,
	ROUTE_MERCHANT_LOGIN,
	ROUTE_CUSTOMER_LOGIN,
	ROUTE_PRODUCT_SHOP,
	ROUTE_ADMIN_MODE,
}RouteStatus_t;

typedef struct {
	RouteStatus_t status;	// ����״̬
	bool loginStatus;
	bool trig;
	bool updateSoft;
}RouteInfo_t;


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ���̴���
  */
void        Route_Process       (void);

/**
  * @brief  ���̴���
  */
RouteInfo_t *Route_GetInfo(void);

/**
  * @brief  ��ȡ����״̬
  */
void        Route_SetLogin(void);

void Route_SetUpdate(void);

#endif /* __ROUTE_H__ */
