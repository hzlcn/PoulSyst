#ifndef __LCD_H__
#define __LCD_H__

#include "common.h"

/* Private typedef -----------------------------------------------------------*/

/**
  * 显示器页码
  */
typedef enum {
	LCD_PAGE_0 = 0,
	LCD_PAGE_1,
	LCD_PAGE_2,
	LCD_PAGE_3,
	LCD_PAGE_4,
	LCD_PAGE_5,
	LCD_PAGE_6,
	LCD_PAGE_7,
	LCD_PAGE_8,
}LCD_Page_t;

/* Private define ------------------------------------------------------------*/

#define LTDC_ON             HAL_GPIO_WritePin(LTDC_ONOFF_GPIO_Port, LTDC_ONOFF_Pin ,GPIO_PIN_SET)
#define LTDC_OFF            HAL_GPIO_WritePin(LTDC_ONOFF_GPIO_Port, LTDC_ONOFF_Pin ,GPIO_PIN_RESET)

/* Private macro -------------------------------------------------------------*/
/**
  * Lcd_Ex address
  */
// 第0页
#define LCD_ADDR_T_00_00_ERRORCODE         0x5000
#define LCD_ADDR_T_00_01_DATE              0x5006
#define LCD_ADDR_T_00_02_SHOWTEXT          0x5010
#define LCD_ADDR_T_00_03_GPSRSSI           0x5018
#define LCD_ADDR_T_00_04_MACHINEID         0x501E
#define LCD_ADDR_T_00_05_FIRMWARE          0x5028
#define LCD_ADDR_T_00_06_NETWORKICON       0x5032

// 第1页
#define LCD_ADDR_T_01_00_ERRORCODE         0x5080
#define LCD_ADDR_T_01_01_DATE              0x5086
#define LCD_ADDR_T_01_02_LABELNUM          0x5090
#define LCD_ADDR_T_01_03_REMAINNUM         0x5092
#define LCD_ADDR_T_01_04_CUSTOMER          0x5094
#define LCD_ADDR_T_01_05_SPECIES           0x509E
#define LCD_ADDR_T_01_06_NETWORKICON       0x50A3

#define LCD_ADDR_B_01_07_CLEARLABEL        0x50C0
#define LCD_ADDR_B_01_08_CHICK             0x50C1
#define LCD_ADDR_B_01_09_DUCK              0x50C2
#define LCD_ADDR_B_01_10_GOOSE             0x50C3
#define LCD_ADDR_B_01_11_OTHERS            0x50C4
#define LCD_ADDR_B_01_12_EXIT              0x50C5

// 第2页
#define LCD_ADDR_T_02_00_ERRORCODE         0x5100
#define LCD_ADDR_T_02_01_DATE              0x5106
#define LCD_ADDR_T_02_02_CHOOSE_11         0x5110
#define LCD_ADDR_T_02_03_CHOOSE_12         0x5115
#define LCD_ADDR_T_02_04_CHOOSE_13         0x511A
#define LCD_ADDR_T_02_05_CHOOSE_14         0x511F
#define LCD_ADDR_T_02_06_CHOOSE_21         0x5124
#define LCD_ADDR_T_02_07_CHOOSE_22         0x5129
#define LCD_ADDR_T_02_08_CHOOSE_23         0x512E
#define LCD_ADDR_T_02_09_CHOOSE_24         0x5133

#define LCD_ADDR_B_02_10_CHOOSE_11         0x5140 // 1行1列
#define LCD_ADDR_B_02_11_CHOOSE_12         0x5141 // 2行1列
#define LCD_ADDR_B_02_12_CHOOSE_13         0x5142
#define LCD_ADDR_B_02_13_CHOOSE_14         0x5143
#define LCD_ADDR_B_02_14_CHOOSE_21         0x5144 // 1行3列
#define LCD_ADDR_B_02_15_CHOOSE_22         0x5145
#define LCD_ADDR_B_02_16_CHOOSE_23         0x5146
#define LCD_ADDR_B_02_17_CHOOSE_24         0x5147
#define LCD_ADDR_B_02_18_DELETE            0x5148
#define LCD_ADDR_B_02_19_LAST              0x5149
#define LCD_ADDR_B_02_20_NEXT              0x514A
#define LCD_ADDR_B_02_21_RETURN            0x514B

// 第3页
#define LCD_ADDR_T_03_00_ERRORCODE         0x5180
#define LCD_ADDR_T_03_01_DATE              0x5186
#define LCD_ADDR_T_03_02_CHOOSE_11         0x5190
#define LCD_ADDR_T_03_03_CHOOSE_12         0x5195
#define LCD_ADDR_T_03_04_CHOOSE_13         0x519A
#define LCD_ADDR_T_03_05_CHOOSE_14         0x519F
#define LCD_ADDR_T_03_06_CHOOSE_21         0x51A4
#define LCD_ADDR_T_03_07_CHOOSE_22         0x51A9
#define LCD_ADDR_T_03_08_CHOOSE_23         0x51AE
#define LCD_ADDR_T_03_09_CHOOSE_24         0x51B3

#define LCD_ADDR_B_03_10_CHOOSE_11         0x51C0
#define LCD_ADDR_B_03_11_CHOOSE_12         0x51C1
#define LCD_ADDR_B_03_12_CHOOSE_13         0x51C2
#define LCD_ADDR_B_03_13_CHOOSE_14         0x51C3
#define LCD_ADDR_B_03_14_CHOOSE_21         0x51C4
#define LCD_ADDR_B_03_15_CHOOSE_22         0x51C5
#define LCD_ADDR_B_03_16_CHOOSE_23         0x51C6
#define LCD_ADDR_B_03_17_CHOOSE_24         0x51C7
#define LCD_ADDR_B_03_18_HOME              0x51C8
#define LCD_ADDR_B_03_19_LAST              0x51C9
#define LCD_ADDR_B_03_20_NEXT              0x51CA

// 第4页
#define LCD_ADDR_T_04_00_ERRORCODE         0x5200
#define LCD_ADDR_T_04_01_DATE              0x5206
#define LCD_ADDR_T_04_02_CHOOSE_11         0x5210
#define LCD_ADDR_T_04_03_CHOOSE_12         0x5215
#define LCD_ADDR_T_04_04_CHOOSE_13         0x521A
#define LCD_ADDR_T_04_05_CHOOSE_14         0x521F
#define LCD_ADDR_T_04_06_CHOOSE_21         0x5224
#define LCD_ADDR_T_04_07_CHOOSE_22         0x5229
#define LCD_ADDR_T_04_08_CHOOSE_23         0x522E
#define LCD_ADDR_T_04_09_CHOOSE_24         0x5233

#define LCD_ADDR_B_04_10_CHOOSE_11         0x5240
#define LCD_ADDR_B_04_10_CHOOSE_12         0x5241
#define LCD_ADDR_B_04_10_CHOOSE_13         0x5242
#define LCD_ADDR_B_04_10_CHOOSE_14         0x5243
#define LCD_ADDR_B_04_10_CHOOSE_21         0x5244
#define LCD_ADDR_B_04_10_CHOOSE_22         0x5245
#define LCD_ADDR_B_04_10_CHOOSE_23         0x5246
#define LCD_ADDR_B_04_10_CHOOSE_24         0x5247
#define LCD_ADDR_B_04_10_LAST              0x5248
#define LCD_ADDR_B_04_10_NEXT              0x5249
#define LCD_ADDR_B_04_10_RETURN            0x524A

// 第5页
#define LCD_ADDR_T_05_00_ERRORCODE         0x5280
#define LCD_ADDR_T_05_01_DATE              0x5286

#define LCD_ADDR_B_05_02_HOME              0x52C0
#define LCD_ADDR_B_05_03_RETURN            0x52C1

// 第6页
#define LCD_ADDR_T_06_00_ERRORCODE         0x5300
#define LCD_ADDR_T_06_01_DATE              0x5306

#define LCD_ADDR_B_06_02_FORCERUN          0x5340
#define LCD_ADDR_B_06_03_UNLOCK            0x5341
#define LCD_ADDR_B_06_04_SPECIES           0x5342
#define LCD_ADDR_B_06_05_MAINTAIN          0x5342

// 第7页
#define LCD_ADDR_T_07_00_ERRORCODE         0x5380
#define LCD_ADDR_T_07_01_DATE              0x5386

#define LCD_ADDR_B_07_02_CHICK             0x53C0
#define LCD_ADDR_B_07_03_DUCK              0x53C1
#define LCD_ADDR_B_07_04_GOOSE             0x53C2
#define LCD_ADDR_B_07_05_OTHERS            0x53C3
#define LCD_ADDR_B_07_06_RETURN            0x53C4

// 第8页
#define LCD_ADDR_T_08_00_ERRORCODE         0x5400
#define LCD_ADDR_T_08_01_DATE              0x5406
#define LCD_ADDR_T_08_02_CHOOSE_11         0x5410
#define LCD_ADDR_T_08_03_CHOOSE_12         0x5415
#define LCD_ADDR_T_08_04_CHOOSE_13         0x541A
#define LCD_ADDR_T_08_05_CHOOSE_14         0x541F
#define LCD_ADDR_T_08_06_CHOOSE_21         0x5424
#define LCD_ADDR_T_08_07_CHOOSE_22         0x5429
#define LCD_ADDR_T_08_08_CHOOSE_23         0x542E
#define LCD_ADDR_T_08_09_CHOOSE_24         0x5433

#define LCD_ADDR_B_08_10_CHOOSE_11         0x5440 // 1行1列
#define LCD_ADDR_B_08_11_CHOOSE_12         0x5441 // 2行1列
#define LCD_ADDR_B_08_12_CHOOSE_13         0x5442
#define LCD_ADDR_B_08_13_CHOOSE_14         0x5443
#define LCD_ADDR_B_08_14_CHOOSE_21         0x5444 // 1行3列
#define LCD_ADDR_B_08_15_CHOOSE_22         0x5445
#define LCD_ADDR_B_08_16_CHOOSE_23         0x5446
#define LCD_ADDR_B_08_17_CHOOSE_24         0x5447
#define LCD_ADDR_B_08_21_RETURN            0x544B

/**
  * LCD parameter
  */
#define LCD_BASE_ADDR       0x5000
#define LCD_PAGE_STEP       0x0080

/**
  * LCD function code
  */
#define LCD_WRITE_REG       0x82
#define LCD_READ_REG        0x83
#define LCD_TURN_REG        0x0084

/**
  * @brief  打开显示器
  */
void        LCD_Init        (void);

/**
  * @brief  处理接收数据
  */
void        LCD_Process     (void);

/**
  * @brief  按字节写入数据
  */
int         LCD_WriteBytes  (uint16_t regAddr, char *pBuf, uint16_t len);

/**
  * @brief  按字写入数据
  */
int         LCD_WriteWords  (uint16_t regAddr, uint16_t *pBuf, uint16_t len);

/**
  * @brief  切换界面
  */
int         LCD_SwitchPage  (LCD_Page_t page);

/**
  * @brief  获取按键状态
  */
uint16_t    LCD_GetButton   (void);

#endif //__LCD_H__
