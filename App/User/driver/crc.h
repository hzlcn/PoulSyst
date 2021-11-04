#ifndef _CRC_H
#define	_CRC_H

#include "stdint.h"

uint16_t CRC16InRam(uint8_t *BufPtr, uint16_t Length,uint16_t CRC16);
//u16 CalCrc16(u8 *buff, uint16_t ulLen);
uint16_t CalCrc16(uint8_t *buff, uint16_t ulLen,uint16_t crc);
#endif
