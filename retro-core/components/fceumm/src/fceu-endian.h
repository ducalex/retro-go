#ifndef _FCEU_ENDIAN_H
#define _FCEU_ENDIAN_H

#include "fceu-memory.h"

#ifndef TARGET_GNW
#include <streams/memory_stream.h>
#else
#include <nes_memory_stream.h>
#endif

int write32le_mem(uint32 b, memstream_t *mem);
int read32le_mem(uint32 *Bufo, memstream_t *mem);

void FlipByteOrder(uint8 *src, uint32 count);

void FCEU_en32lsb(uint8 *, uint32);
uint32 FCEU_de32lsb(const uint8 *);

#endif
