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



/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  ���̴���
  */
void        Route_Process       (void);

#endif /* __ROUTE_H__ */
