#include "drivers.h"

void Ex_WDI_Feed(void)
{
	HAL_GPIO_TogglePin(WDI_GPIO_Port, WDI_Pin);
}


void PrintHexBuffer(uint8_t *buffer, uint8_t len)
{
	uint8_t i;
	for (i = 0; i < len; i++) {
		printf(" %02x", buffer[i]);
		if ((i % 16 == 15) && (i != len - 1)) {
			printf("\r\n");
		}
	}
	printf("\r\n");
}

void PrintCharBuffer(char *buf, uint8_t len)
{
	uint8_t i;
	for (i = 0; i < len; i++) {
		if ((buf[i] == '\r') && (buf[i + 1] == '\n')) {
			break;
		}
		printf("%c", buf[i]);
		if ((i % 48 == 47) && (i != len - 1)) {
			printf("\r\n");
		}
		
	}
	printf("\r\n");
}

/*----------------------------- HAL FUNCTION END -----------------------------*/

/**
  * @名称  int fputc(int ch, FILE *f)
  * @说明  重定义fputc函数,加入以下代码,支持printf函数,而不需要选择use MicroLIB
  */
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;

};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}

//输出
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit( &huart3, (uint8_t *)&ch, 1, 1000 );
	return ch;
}
//#include "stdio.h"

//#ifdef __GNUC__

//int __io_putchar(int ch)
//{
//	HAL_UART_Transmit(&huart1,(uint8_t*)&ch, 1, HAL_MAX_DELAY);

//	return ch;
//}
//#endif

