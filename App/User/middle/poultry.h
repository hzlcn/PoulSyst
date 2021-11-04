#ifndef __POULTRY_H__
#define __POULTRY_H__

#include "common.h"
#include "serial.h"

/* Private macro -------------------------------------------------------------*/

/**
  * 食品信息码大小
  */
#define N_POULTRY_QRCODE_BUFFER_SIZE    24

/**
  * 最大食品信息条数
  */
#define N_POULTRY_MAX_KIND              30

/**
  * 食品信息 - 种类序号长度[6] = 大类序号[2] + 小类序号[4]
  */
#define N_POULTRY_KIND_NUMS_SIZE        6

/* Private typedef -----------------------------------------------------------*/

/**
  * 食品列表类型
  */
typedef enum {
	POUL_LISTTYPE_NONE = 0,
	POUL_LISTTYPE_CUR  = 1,             // 食品信息列表
	POUL_LISTTYPE_ADD  = 2,             // 添加信息列表
}PoulListType_t;

/**
  * 食品类型
  */
typedef enum {
	POUL_LISTGROUPTYPE_CHICK = 0,       // 鸡
	POUL_LISTGROUPTYPE_DUCK  = 1,       // 鸭
	POUL_LISTGROUPTYPE_GOOSE = 2,       // 鹅
	POUL_LISTGROUPTYPE_OTHER = 3,       // 其它
	POUL_LISTGROUPTYPE_NONE  = 4,       // 无
}PoulListGroup_t;

/**
  * 食品种类信息结构体
  */
typedef struct {
	// 前2字节为PoulListGroup_t类型，后4字节为更细化的类别。(例如: 前2字节表示鸡，后4字节表示山地鸡。)
	u8 kindNum[N_POULTRY_KIND_NUMS_SIZE];
	
	// 食品种类的名称(GBK编码)
	u8 name[10];
}PoulKind_t;

/**
  * 食品种类库
  */
typedef PoulKind_t PoulKinds_t[N_POULTRY_MAX_KIND];

/**
  * 食品信息库结构体
  */
typedef struct {
	u8 keepFlag;
	u8 version[2];
	u8 sum;
	PoulKinds_t kinds;
}PoulBase_t;

/**
  * 食品数据列表
  */
typedef struct {
	// 当前列表类型
	PoulListType_t type;
	
	// 食品组类
	PoulListGroup_t group;
	
	// 信息总数
	u8 sum;
	
	// 食品信息库索引列表
	PoulKind_t *pKinds[N_POULTRY_MAX_KIND];
}PoulList_t;

/**
  * 食品编码信息结构体
  */
typedef struct {
	bool isReady;
	u8   qrcode[N_POULTRY_QRCODE_BUFFER_SIZE];
}PoulCode_t;

/**
  * 食品种类信息结构体
  */
typedef struct {
	PoulKind_t *pCurKind;		// 当前食品种类
	PoulCode_t  code;           // 食品二维码
	PoulList_t  list;           // 食品列表
	PoulBase_t *base[2];		// 食品种类库
}PoulInfo_t;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  初始化食品二维码模块
  */
void        Poul_InitModule(void);

/**
  * @brief  处理食品二维码数据(串口2)
  */
void        Poul_Process(void);

/**
  * @brief  获取食品类型信息参数地址指针
  */
PoulInfo_t *Poul_GetInfo(void);

/**
  * @brief  清除食品二维码
  */
void        Poul_ClrCode(void);

/**
  * @brief  更新食品种类指针列表
  */
void        Poul_SetList(PoulListGroup_t group, PoulListType_t type);

/**
  * @brief  设置当前食品种类
  */
void        Poul_SetKind(PoulKind_t *kind);

/**
  * @brief  添加单条食品信息到内部禽类库
  */
void        Poul_AddKind(PoulKind_t *kind);

/**
  * @brief  通过索引从内部禽类库中删除单条食品信息
  */
void        Poul_DelKind(PoulKind_t *kind);

/**
  * @brief  设置食品信息库为初始值
  */
void        Poul_DefBase(void);

#endif /* __PRODUCT_H__ */
