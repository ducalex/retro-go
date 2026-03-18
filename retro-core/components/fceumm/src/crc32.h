#ifndef CRC32_H
#define CRC32_H

#include "fceu-crc32.h"
#include "fceu-types.h"


// CalcCRC32 is defined in fceu-crc32.c
uint32 CalcCRC32(uint32 crc, uint8 *buf, uint32 len);

#endif
