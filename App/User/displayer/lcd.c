/* Private includes ----------------------------------------------------------*/
#include "lcd.h"
#include "drivers.h"
#include "dataStruct.h"

/**************************************************************************
*@写字指令 5A A5 0D 82 1000 2829 BAC3 B1B3 B1B4 B1B5
*@5A A5：帧头
*@0D:数据长度（写寄存器指令82-->结束）
*@82：写寄存器指令
*@1000：变量地址
*@后面直接接要显示的字符的GBK码,若要清空显示字符显示字符20
***************************************************************************/

/**************************************************************************
*@切换页面指令 5A A5 07 82 00 84 5A 01 00 03
*@5A A5：帧头
*@07:数据长度（写寄存器指令82-->结束）
*@82：写寄存器指令
*@0084：切换页面系统变量地址
*@5A 01:固定
*@后面直接接要切换的页面
***************************************************************************/

/**************************************************************************
*@按键返回指令 5A A5 06 83 10 00 01 00 00
*@5A A5：帧头
*@06:数据长度（读寄存器指令83-->结束）
*@83：读寄存器指令
*@1000:固变量地址
*@0000：键值，默认为0
***************************************************************************/

/* Private define ------------------------------------------------------------*/

#define DBG_TRACE                                   0

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

/* Private variables ---------------------------------------------------------*/

/**
  * 数据缓冲区
  */
uint8_t  g_LCD_RxBuf[UART4_BUFFSIZE];
uint8_t  g_LCD_TxBuf[UART4_BUFFSIZE];
uint16_t g_LCD_TxLen = 0;

/**
  * 当前按键地址
  */
uint16_t g_LCD_Button = 0;

/**
  * 页码
  */
static LCD_Page_t *g_LCD_Page = &g_allData.temp.display.page;

/**
  * 写入应答 - 固定数据
  */
const uint8_t g_LCD_Ack[6] = { 0x5a, 0xa5, 0x03, 0x82, 0x4f, 0x4b };

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  发送LCD数据
  * @param  
  * @retval 
  */
static void LCD_Send(uint8_t *pBuf, uint16_t len);

/**
  * @brief  等待应答
  */
static bool LCD_WaitAck(uint32_t timeout);

/**
  * @brief  打开显示器
  * @param  
  * @retval 
  */
void LCD_Init(void)
{
	LTDC_ON;
	Uart_SetRxEnable(&huart4);
}

/**
  * @brief  处理接收数据
  * @param  
  * @retval 
  */
void LCD_Process(void)
{
	uint16_t rxLen = Uart_GetData(&huart4, g_LCD_RxBuf);
	if (rxLen > 0) {
		if ((g_LCD_RxBuf[0] == 0x5A) && 
			(g_LCD_RxBuf[1] == 0xA5) && 
			(g_LCD_RxBuf[2] == 0x06) && 
			(g_LCD_RxBuf[3] == 0x83))
		{
			if (g_LCD_Button == 0) {
				g_LCD_Button = (g_LCD_RxBuf[4] << 8) | g_LCD_RxBuf[5];
			}
		}
		memset(g_LCD_RxBuf, 0, UART4_BUFFSIZE);
	}
}

/**
  * @brief  按字节写入数据
  * @param  waddr: 写入的地址;
  *         buffer: 数据缓冲区; 
  *         len: 字节长度
  * @retval 
  */
void LCD_WriteByte(uint16_t regAddr, char *pBuf, uint16_t len)
{
	memset(g_LCD_TxBuf, 0, UART4_BUFFSIZE);
	g_LCD_TxLen = 0;

	g_LCD_TxBuf[g_LCD_TxLen++] = 0x5A;
	g_LCD_TxBuf[g_LCD_TxLen++] = 0xA5;
	g_LCD_TxBuf[g_LCD_TxLen++] = len + 3;
	g_LCD_TxBuf[g_LCD_TxLen++] = LCD_WRITE_REG;
	g_LCD_TxBuf[g_LCD_TxLen++] = (regAddr & 0xFF00) >> 8;
	g_LCD_TxBuf[g_LCD_TxLen++] = (regAddr & 0x00FF) >> 0;
	for (uint16_t i = 0; i < len; i++) {
		g_LCD_TxBuf[g_LCD_TxLen++] = pBuf[i];
	}
	LCD_Send(g_LCD_TxBuf, g_LCD_TxLen);
#if (DBG_TRACE == 1)
	DBG("send(LCD):");
	PrintHexBuffer(g_LCD_TxBuf, g_LCD_TxLen);
#endif
}

/**
  * @brief  按字写入数据
  * @param  waddr: 写入的地址;
  *         buffer: 数据缓冲区; 
  *         len: 字长度
  * @retval 
  */
void LCD_WriteWord(uint16_t regAddr, uint16_t *pBuf, uint16_t len)
{
	uint16_t i;
	
	memset(g_LCD_TxBuf, 0, UART4_BUFFSIZE);
	g_LCD_TxLen = 0;

	g_LCD_TxBuf[g_LCD_TxLen++] = 0x5A;
	g_LCD_TxBuf[g_LCD_TxLen++] = 0xA5;
	g_LCD_TxBuf[g_LCD_TxLen++] = len * 2 + 3;
	g_LCD_TxBuf[g_LCD_TxLen++] = LCD_WRITE_REG;
	g_LCD_TxBuf[g_LCD_TxLen++] = (regAddr & 0xFF00) >> 8;
	g_LCD_TxBuf[g_LCD_TxLen++] = (regAddr & 0x00FF) >> 0;
	for (i = 0; i < len; i++) {
		g_LCD_TxBuf[g_LCD_TxLen++] = (pBuf[i] & 0xFF00) >> 8;
		g_LCD_TxBuf[g_LCD_TxLen++] = (pBuf[i] & 0x00FF) >> 0;
	}
	LCD_Send(g_LCD_TxBuf, g_LCD_TxLen);
#if (DBG_TRACE == 1)
	DBG("send(LCD):");
	PrintHexBuffer(g_LCD_TxBuf, g_LCD_TxLen);
#endif
}

/**
  * @brief  切换界面
  * @param  page: 切换的页数;
  *         type: 自动切换还是指令切换
  * @retval 
  */
void LCD_SwitchPage(LCD_Page_t page)
{
	memset(g_LCD_TxBuf, 0, UART4_BUFFSIZE);
	g_LCD_TxLen = 0;

	g_LCD_TxBuf[g_LCD_TxLen++] = 0x5A;
	g_LCD_TxBuf[g_LCD_TxLen++] = 0xA5;
	g_LCD_TxBuf[g_LCD_TxLen++] = 7;
	g_LCD_TxBuf[g_LCD_TxLen++] = LCD_WRITE_REG;
	g_LCD_TxBuf[g_LCD_TxLen++] = (LCD_TURN_REG & 0xFF00) >> 8;
	g_LCD_TxBuf[g_LCD_TxLen++] = (LCD_TURN_REG & 0x00FF) >> 0;
	g_LCD_TxBuf[g_LCD_TxLen++] = 0x5A;
	g_LCD_TxBuf[g_LCD_TxLen++] = 0x01;
	g_LCD_TxBuf[g_LCD_TxLen++] = 0x00;
	g_LCD_TxBuf[g_LCD_TxLen++] = page;
	
	LCD_Send(g_LCD_TxBuf, g_LCD_TxLen);
#if (DBG_TRACE == 1)
	DBG("send(LCD):");
	PrintHexBuffer(g_LCD_TxBuf, 10);
#endif

	*g_LCD_Page = page;
}

/**
  * @brief  获取页码
  * @param  
  * @retval 
  */
LCD_Page_t LCD_GetPage(void)
{
	return *g_LCD_Page;
}

/**
  * @brief  获取按键状态
  * @param  
  * @retval 
  */
uint16_t LCD_GetButton(void)
{
	uint16_t button = g_LCD_Button;
	g_LCD_Button = 0;
	return button;
}

/**
  * @brief  发送LCD数据
  * @param  
  * @retval 
  */
static void LCD_Send(uint8_t *pBuf, uint16_t len)
{
	Uart4_Send(pBuf, len);
	if (LCD_WaitAck(10) == false) {
		Uart4_Send(pBuf, len);
	}
}

/**
  * @brief  等待应答
  * @param  
  * @retval 
  */
static bool LCD_WaitAck(uint32_t timeout)
{
	bool ret = false;
	uint16_t rxLen = 0;
	while (timeout--) {
		rxLen = Uart_GetData(&huart4, g_LCD_RxBuf);
		if (rxLen > 0) {
			// 写入应答
			if ((g_LCD_RxBuf[0] == g_LCD_Ack[0]) && 
				(g_LCD_RxBuf[1] == g_LCD_Ack[1]) && 
				(g_LCD_RxBuf[2] == g_LCD_Ack[2]) && 
				(g_LCD_RxBuf[3] == g_LCD_Ack[3]) && 
				(g_LCD_RxBuf[4] == g_LCD_Ack[4]) && 
				(g_LCD_RxBuf[5] == g_LCD_Ack[5])) 
			{
				ret = true;
				break;
			}
			// 按键返回
			else if ((g_LCD_RxBuf[0] == 0x5A) && 
				(g_LCD_RxBuf[1] == 0xA5) && 
				(g_LCD_RxBuf[2] == 0x06) && 
				(g_LCD_RxBuf[3] == 0x83))
			{
				if (g_LCD_Button == 0) {
					g_LCD_Button = (g_LCD_RxBuf[4] << 8) | g_LCD_RxBuf[5];
				}
				break;
			}

		}
		memset(g_LCD_RxBuf, 0, UART4_BUFFSIZE);
		osDelay(1);
	}
	return ret;
}

