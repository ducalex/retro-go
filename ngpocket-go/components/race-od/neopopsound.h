// Flavor modified sound.c and sound.h from NEOPOP
//  which was originally based on sn76496.c from MAME
//  some ideas also taken from NeoPop-SDL code

//---------------------------------------------------------------------------
// Originally from
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------


#ifndef __NEOPOPSOUND__
#define __NEOPOPSOUND__
//=============================================================================

#ifdef __GP32__
#include "main.h"
#endif
#include "StdAfx.h"

typedef struct
{
	int LastRegister;	/* last register written */
	int Register[8];	/* registers */
	int Volume[4];
	int Period[4];
	int Count[4];
	int Output[4];

	unsigned int RNG;	/* noise generator      */
	int NoiseFB;		/* noise feedback mask */

} SoundChip;

//=============================================================================

extern SoundChip toneChip;
extern SoundChip noiseChip;

void WriteSoundChip(SoundChip* chip, _u8 data);

int sound_system_init();
BOOL system_sound_init(void);
void system_sound_chipreset(void);



//void system_sound_update(void);
void system_sound_update(int nframes);
void system_VBL(void);

#define Write_SoundChipTone(VALUE)		(WriteSoundChip(&toneChip, VALUE))
#define Write_SoundChipNoise(VALUE)		(WriteSoundChip(&noiseChip, VALUE))

//=============================================================================

//void dac_writeL(unsigned char);
//void dac_writeR(unsigned char);

void sound_init(int SampleRate);

extern BOOL mute;

void dac_update(_u16* dac_buffer, int length_bytes);
void sound_update(_u16* chip_buffer, int length_bytes);

#ifndef __GP32__
void increaseVolume();
void decreaseVolume();
#endif

//#define dac_writeL dac_write
//#define dac_writeR dac_write
void dac_writeL(unsigned char);
//void dac_writeR(unsigned char);
void dac_write(unsigned char);

//=============================================================================
#endif
