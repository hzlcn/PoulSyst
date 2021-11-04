/* Includes ------------------------------------------------------------------*/
#include "poultry.h"
#include "serial.h"
#include "drivers.h"
#include "param.h"

/* Private macro -------------------------------------------------------------*/

/**
  * 食品二维码偏移量
  */
#define POUL_CODE_OFFSET    48

/**
  * 食品二维码最小长度
  */
#define POUL_CODE_MIN_SIZE  POUL_CODE_OFFSET

/* Private variables ---------------------------------------------------------*/

/**
  * 初始的食品信息库
  */
static PoulBase_t g_poulDefaultBase = {
	.keepFlag = 0,
	.version = { 0, 0 },
	.sum = 8,
	.kinds[0] = { { 0, 0, 0, 0, 0, 1 }, { 0xC9, 0xBD, 0xB5, 0xD8, 0xBC, 0xA6, 0xFF, 0xFF, 0xFF, 0xFF } },
	.kinds[1] = { { 0, 0, 0, 0, 0, 2 }, { 0xC9, 0xBD, 0xB5, 0xD8, 0xBC, 0xA6, 0xFF, 0xFF, 0xFF, 0xFF } },
	.kinds[2] = { { 0, 1, 0, 0, 0, 3 }, { 0xC9, 0xBD, 0xB5, 0xD8, 0xD1, 0xBC, 0xFF, 0xFF, 0xFF, 0xFF } },
	.kinds[3] = { { 0, 1, 0, 0, 0, 4 }, { 0xC9, 0xBD, 0xB5, 0xD8, 0xD1, 0xBC, 0xFF, 0xFF, 0xFF, 0xFF } },
	.kinds[4] = { { 0, 2, 0, 0, 0, 5 }, { 0xC9, 0xBD, 0xB5, 0xD8, 0xB6, 0xEC, 0xFF, 0xFF, 0xFF, 0xFF } },
	.kinds[5] = { { 0, 2, 0, 0, 0, 6 }, { 0xC9, 0xBD, 0xB5, 0xD8, 0xB6, 0xEC, 0xFF, 0xFF, 0xFF, 0xFF } },
	.kinds[6] = { { 0, 3, 0, 0, 0, 7 }, { 0xC9, 0xBD, 0xB5, 0xD8, 0xC4, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF } },
	.kinds[7] = { { 0, 3, 0, 0, 0, 8 }, { 0xC9, 0xBD, 0xB5, 0xD8, 0xC4, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF } },
	
};

/**
  * 食品信息库
  */
static PoulBase_t g_poulCurBase = { 0 };

/**
  * 添加信息库
  */
static PoulBase_t g_poulAddBase = { 0 };

/**
  * 食品种类信息
  */
static PoulInfo_t g_poulInfo = {	
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
};

/**
  * 接收缓冲区
  */
static uint8_t g_poulRxBuf[UART2_BUFFSIZE];

/**
  * @brief  检查接收数据
  */
static bool Poul_RxCheck(u8 *pBuf, u16 len);

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  获取食品类型信息参数地址指针
  * @param  
  * @retval 
  */
PoulInfo_t *Poul_GetInfo(void)
{
	return &g_poulInfo;
}

/**
  * @brief  初始化食品二维码模块
  * @param  
  * @retval 
  */
void Poul_InitModule(void)
{
	// 食品二维码模块初始化
	Uart_SetRxEnable(&huart2);
}

/**
  * @brief  处理食品二维码数据(串口2)
  * @param  
  * @retval 
  */
void Poul_Process(void)
{	
	u16 i;
	u16 rxLen = Uart_GetData(&huart2, g_poulRxBuf);
	if (rxLen > 0) {
		if ((g_poulInfo.code.isReady == false) && (rxLen > POUL_CODE_MIN_SIZE)) {
			if (Poul_RxCheck(g_poulRxBuf + POUL_CODE_OFFSET, MIN((rxLen - POUL_CODE_OFFSET) , N_POULTRY_QRCODE_BUFFER_SIZE)) == true) {
				g_poulInfo.code.isReady = true;
				memset(g_poulInfo.code.qrcode, '-', N_POULTRY_QRCODE_BUFFER_SIZE);
				memcpy(g_poulInfo.code.qrcode, g_poulRxBuf + POUL_CODE_OFFSET, MIN((rxLen - POUL_CODE_OFFSET) , N_POULTRY_QRCODE_BUFFER_SIZE));

				Beep_Run(50);
			}
		}
		
		// 清空缓冲区
		memset(g_poulRxBuf, 0, UART2_BUFFSIZE);
	}
}

/**
  * @brief  清除食品二维码
  * @param  
  * @retval 
  */
void Poul_ClrCode(void)
{
//	memset(g_poulInfo.code.qrcode, '-', N_POULTRY_QRCODE_BUFFER_SIZE);
	g_poulInfo.code.isReady = false;
}

/**
  * @brief  更新列表
  * @param  
  * @retval 
  */
void Poul_SetList(PoulListGroup_t group, PoulListType_t type)
{
	u8 i = 0;
	PoulBase_t *base = NULL;
	
	// 清空列表
	memset(&g_poulInfo.list, 0, sizeof(PoulList_t));

	g_poulInfo.list.group = group;
	g_poulInfo.list.type = type;
	
	if (type == POUL_LISTTYPE_CUR) {
		base = &g_poulCurBase;
	} else if (type == POUL_LISTTYPE_ADD) {
		base = &g_poulAddBase;
	} else {
		return ;
	}
	
	for (i = 0; i < base->sum; i++) {
		if (base->kinds[i].kindNum[1] == g_poulInfo.list.group) {
			// 添加到指针列表
			g_poulInfo.list.pKinds[g_poulInfo.list.sum++] = &base->kinds[i];
		}
	}
}

/**
  * @brief  设置当前食品种类
  * @param  
  * @retval 
  */
void Poul_SetKind(PoulKind_t *data)
{
	g_poulInfo.pCurKind = data;
}

/**
  * @brief  添加单条食品信息到信息库
  * @param  info    要添加的食品信息
  * @retval 
  */
void Poul_AddKind(PoulKind_t *kind)
{
	uint8_t i = 0;
	PoulBase_t *base = NULL;     // 信息库指针

	base = &g_poulCurBase;
	
	// 检查信息是否已经存在
	for (i = 0; i < base->sum; i++) {
		if (memcmp(&base->kinds[i], kind, sizeof(PoulKind_t)) == 0) {
			return ;
		}
	}
	
	// 判断信息总数是否已经达到最大值
	if (base->sum < 30) {
		memcpy(&base->kinds[base->sum], kind, sizeof(PoulKind_t));
		base->sum++;
	}
}

/**
  * @brief  通过索引从食品信息库中删除单条食品信息
  * @param  
  * @retval 
  */
void Poul_DelKind(PoulKind_t *data)
{
	PoulBase_t *base = &g_poulCurBase;     // 信息库指针
	uint8_t index = (data - &base->kinds[0]);

	if (data == NULL) {
		return ;
	}
	
	// 删除信息
	memcpy(data, data + 1, (base->sum - 1 - index) * sizeof(PoulKind_t));
	base->sum--;
	memset(&base->kinds[base->sum], 0, sizeof(PoulKind_t));
	
	// 更新信息列表
	Poul_SetList(g_poulInfo.list.group, g_poulInfo.list.type);
}

/**
  * @brief  设置食品信息库为初始值
  * @param  
  * @retval 
  */
void Poul_DefBase(void)
{
	memcpy(&g_poulCurBase, &g_poulDefaultBase, sizeof(PoulBase_t));
}

/**
  * @brief  检查接收数据
  * @param  
  * @retval 
  */
static bool Poul_RxCheck(u8 *pBuf, u16 len)
{
	u16 i;
	// 1.检查数据是否出现0x00或0xFF
	for (i = 0; i < len; i++) {
		if ((pBuf[i] == 0x00) || (pBuf[i] == 0xFF)) {
			return false;
		}
	}
	
	// 2.检查数据是否重复
	for (i = 0; i < len; i++) {
		if (pBuf[i] != g_poulInfo.code.qrcode[i]) {
			return true;
		}
	}
	return false;
}

