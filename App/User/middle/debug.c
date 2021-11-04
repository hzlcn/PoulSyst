#include "debug.h"
#include "w25qxx.h"
#include "display.h"
#include "drivers.h"
#include "ethernet.h"
#include "usermsg.h"
#include "lcd.h"
#include "param.h"
#include "network.h"
#include "store.h"

typedef enum {
	DEBUG_CMD_0,
	DEBUG_CMD_1,
	DEBUG_CMD_2,
	DEBUG_CMD_5 = 5,
	DEBUG_CMD_33,
}Debug_CmdType_t;

uint8_t  g_debug_RxBuf[UART3_BUFFSIZE];

static void Debug_Parse(uint8_t *pBuf, uint16_t len);

void Debug_Init(void)
{
	Uart_SetRxEnable(&huart3);
}

void Debug_Process(void)
{
    uint16_t rxLen = Uart_GetData(&huart3, g_debug_RxBuf);
	if (rxLen > 0) {
//		printf("%s", g_debug_RxBuf);
//		Uart6_Send(g_debug_RxBuf, rxLen);
		
		// 帧头 + 帧尾
		if ((g_debug_RxBuf[0] == 0xFB) && 
			(g_debug_RxBuf[rxLen - 1] == 0xFC))
		{
			// 处理数据
			Debug_Parse(g_debug_RxBuf + 1, rxLen - 2);
		}
		
		memset(g_debug_RxBuf, 0, UART3_BUFFSIZE);
	}
}

static void Debug_Parse(uint8_t *pBuf, uint16_t len)
{
	Merchant_t *merchant = NULL;
	Customer_t *customer = NULL;
	uint8_t *devAddr = NULL;
	
	switch(pBuf[0]) {
		case DEBUG_CMD_0:
		{	// LCD Debug
			if (pBuf[1] == 1) { // 翻页
				LCD_SwitchPage((LCD_Page_t)pBuf[2]);
			} else if (pBuf[1] == 2) {
				Display_SetLoginText(TEXT_NULL);
			} else if (pBuf[1] == 3) {
			}
			break;
		}
		case DEBUG_CMD_1:
		{	// Qrcode
			if (pBuf[1] == 1) { // 查询全部参数
				printf("Merchant: ");
				merchant = &User_GetInfo()->merc;
				if ((merchant->perm == USER_PERM_ADMIN) || 
					(merchant->perm == USER_PERM_MAINTAIN)) 
				{
					printf("%s\r\n", merchant->code);
				} else {
					printf("\r\n");
				}
				printf("Customer: ");
				customer = &User_GetInfo()->cust;
				if (customer->perm == USER_PERM_CUSTOMER) {
					printf("%s\r\n", customer->code);
				} else {
					printf("\r\n");
				}
			}
			break;
		}
		case DEBUG_CMD_2:
		{	// Ethernet
			if (pBuf[1] == 1) {
				Network_Send(pBuf + 3, pBuf[2]);
			}
			break;
		}
		case DEBUG_CMD_5:
		{	// 修改设备地址
			devAddr = (uint8_t *)Store_GetConfig()->devAddr;
			if (len > 6) {
				memcpy(devAddr, pBuf + 1, 6);
				Store_SaveConfig();
			}
			printf("device address: %02x %02x %02x %02x %02x %02x\r\n", devAddr[0], devAddr[1], devAddr[2], devAddr[3], devAddr[4], devAddr[5]);
			break;
		}
		case DEBUG_CMD_33:
		{	// 直接调用协议
			//CMNC_DLProcess();
		}
		default:
			break;
	}
}





