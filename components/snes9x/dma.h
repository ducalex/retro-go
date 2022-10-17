/* This file is part of Snes9x. See LICENSE file. */

#ifndef _DMA_H_
#define _DMA_H_

void S9xResetDMA(void);
uint8_t S9xDoHDMA(uint8_t);
void S9xStartHDMA(void);
void S9xDoDMA(uint8_t);
#endif
