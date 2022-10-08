#pragma once

#include "gnuboy.h"

typedef struct
{
	int rate, cycles;
	byte wave[16];
	struct {
		unsigned on, pos;
		int cnt, encnt, swcnt;
		int len, enlen, swlen;
		int swfreq, freq;
		int envol, endir;
	} ch[4];
} gb_snd_t;

gb_snd_t *sound_init(void);
void sound_write(byte r, byte b);
void sound_dirty(void);
void sound_reset(bool hard);
void sound_emulate(void);
#define sound_advance(count) hw.snd->cycles += (count)
