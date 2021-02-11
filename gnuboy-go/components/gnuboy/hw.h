#ifndef __HW_H__
#define __HW_H__


#include "emu.h"


#define PAD_RIGHT  0x01
#define PAD_LEFT   0x02
#define PAD_UP     0x04
#define PAD_DOWN   0x08
#define PAD_A      0x10
#define PAD_B      0x20
#define PAD_SELECT 0x40
#define PAD_START  0x80

#define IF_VBLANK 0x01
#define IF_STAT   0x02
#define IF_TIMER  0x04
#define IF_SERIAL 0x08
#define IF_PAD    0x10
#define IF_MASK   0x1F

typedef struct
{
	un32 ilines;
	un32 pad;
	un32 cgb;
	n32 hdma;
	n32 serial;
} hw_t;


extern hw_t hw;

void hw_hdma();
void hw_dma(byte b);
void hw_hdma_cmd(byte c);
void hw_reset(bool hard);
void pad_set(byte btn, int set);
void pad_refresh();
void hw_interrupt(byte i, int level);

#endif
