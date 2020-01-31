#ifndef __HW_H__
#define __HW_H__


#include "defs.h"


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

struct hw
{
	byte ilines;
	byte pad;
	int cgb, gba;
	int hdma;
};


extern struct hw hw;

void hw_hdma();
void hw_dma(byte b);
void hw_hdma_cmd(byte c);
void hw_reset();
void pad_set(byte k, int st);
void pad_refresh();

#endif
