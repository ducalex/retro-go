#pragma once

#include "gnuboy.h"

typedef struct
{
	int rate, cycles;
	byte wave[16];
	struct {
		int on;
		unsigned pos;
		int cnt, encnt, swcnt;
		int len, enlen, swlen;
		int swfreq;
		int freq;
		int envol, endir;
	} ch[4];
} gb_snd_t;

typedef struct
{
	int hz, len;
	int stereo;
	n16* buf;
	int pos;
} gb_pcm_t;

extern gb_pcm_t pcm;
extern gb_snd_t snd;

void sound_write(byte r, byte b);
byte sound_read(byte r);
void sound_dirty();
void sound_reset(bool hard);
void sound_mix();
