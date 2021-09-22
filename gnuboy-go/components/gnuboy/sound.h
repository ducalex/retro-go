#pragma once

#include "gnuboy.h"

typedef struct
{
	int samplerate;
	bool stereo;

	struct {
		int pos, len;
		n16* buf;
	} output;

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

extern gb_snd_t snd;

void sound_init(int samplerate, bool stereo);
void sound_write(byte r, byte b);
void sound_dirty(void);
void sound_reset(bool hard);
void sound_emulate(void);
void sound_advance(int cycles);
