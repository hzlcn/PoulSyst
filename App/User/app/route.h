#ifndef __ROUTE_H__
#define __ROUTE_H__

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Private typedef -----------------------------------------------------------*/

/**
  * 流程状态
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
  * @brief  流程处理
  */
void        Route_Process       (void);

#endif /* __ROUTE_H__ */
