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

gb_snd_t *gb_sound_init(void);
void gb_sound_write(byte r, byte b);
void gb_sound_dirty(void);
void gb_sound_reset(bool hard);
void gb_sound_emulate(void);
#define gb_sound_advance(count) GB.snd->cycles += (count)
