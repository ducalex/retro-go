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


/************************************************************************
 *                                                                      *
 *	Portions, but not all of this source file are based on MAME v0.60	*
 *	File "sn76496.c". All copyright goes to the original author.		*
 *	The remaining parts, including DAC processing, by neopop_uk			*
 *                                                                      *
 ************************************************************************/

#ifndef __GP32__
#include "StdAfx.h"
#endif

#include "neopopsound.h"


#include "main.h"
#include "memory.h"
//#include "menu.h"


//=============================================================================

SoundChip toneChip;
SoundChip noiseChip;

//==== DAC
#ifdef TARGET_WIN
#define DAC_BUFFERSIZE		(2560 * 1024) //at (256 * 1024) the PC version will crash on MS2 intro
#else
#define DAC_BUFFERSIZE		(16 * 1024) //at (256 * 1024) the PC version will crash on MS2 intro
#endif

int dacLBufferRead, dacLBufferWrite, dacLBufferCount;
_u16 dacBufferL[DAC_BUFFERSIZE];

#if !defined(__GP32__) && !defined(TARGET_PSP)
#define SDL_MIX_MAXVOLUME 99
int volume = SDL_MIX_MAXVOLUME;//SDL_MIX_MAXVOLUME / 2;

void increaseVolume()
{
    if(volume < SDL_MIX_MAXVOLUME)
        volume++;
}

void decreaseVolume()
{
    if(volume > 0)
        volume--;
}

#endif

//=============================================================================

#define SOUNDCHIPCLOCK	(3072000)	//Unverified / sounds correct

#define MAX_OUTPUT 0x7fff
#define STEP 0x10000		//Fixed point adjuster

#define MAX_OUTPUT_STEP 0x7fff0000
#define STEP_SHIFT 16

static _u32 VolTable[16];
static _u32 UpdateStep = 0;	//Number of steps during one sample.

/* Formulas for noise generator */
/* bit0 = output */

/* noise feedback for white noise mode (verified on real SN76489 by John Kortink) */
#define FB_WNOISE 0x14002	/* (16bits) bit16 = bit0(out) ^ bit2 ^ bit15 */

/* noise feedback for periodic noise mode */
#define FB_PNOISE 0x08000	/* 15bit rotate */

/* noise generator start preset (for periodic noise) */
#define NG_PRESET 0x0f35

#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

//=============================================================================

static _u16 sample_chip_tone(void)
{
	int i;

	int vol[3];
	unsigned int out;

	/* vol[] keeps track of how long each square wave stays */
	/* in the 1 position during the sample period. */
	vol[0] = vol[1] = vol[2] = /*vol[3] = */ 0;

	for (i = 0; i < 3; i++)
	{
		if (toneChip.Output[i]) vol[i] += toneChip.Count[i];
		toneChip.Count[i] -= STEP;

		/* Period[i] is the half period of the square wave. Here, in each */
		/* loop I add Period[i] twice, so that at the end of the loop the */
		/* square wave is in the same status (0 or 1) it was at the start. */
		/* vol[i] is also incremented by Period[i], since the wave has been 1 */
		/* exactly half of the time, regardless of the initial position. */
		/* If we exit the loop in the middle, Output[i] has to be inverted */
		/* and vol[i] incremented only if the exit status of the square */
		/* wave is 1. */

		while (toneChip.Count[i] <= 0)
		{
			toneChip.Count[i] += toneChip.Period[i];
			if (toneChip.Count[i] > 0)
			{
				toneChip.Output[i] ^= 1;
				if (toneChip.Output[i]) vol[i] += toneChip.Period[i];
				break;
			}
			toneChip.Count[i] += toneChip.Period[i];
			vol[i] += toneChip.Period[i];
		}
		if (toneChip.Output[i]) vol[i] -= toneChip.Count[i];
	}

	out = vol[0] * toneChip.Volume[0] + vol[1] * toneChip.Volume[1] +
		vol[2] * toneChip.Volume[2];

	//if (out > MAX_OUTPUT * STEP) out = MAX_OUTPUT * STEP;
	if (out > MAX_OUTPUT_STEP) out = MAX_OUTPUT_STEP;

	return out>>STEP_SHIFT;//out / STEP;
}

//=============================================================================

static _u16 sample_chip_noise(void)
{
	//int vol[4];
	int vol3 = 0;
	unsigned int out;
	int left;

	/* vol[] keeps track of how long each square wave stays */
	/* in the 1 position during the sample period. */
	if (noiseChip.Volume[3])
	{
		left = STEP;
		do
		{
			int nextevent = min(noiseChip.Count[3],left);

			//if (noiseChip.Count[3] < left) nextevent = noiseChip.Count[3];
			//else nextevent = left;

			if (noiseChip.Output[3]) vol3 += noiseChip.Count[3];
			noiseChip.Count[3] -= nextevent;
			if (noiseChip.Count[3] <= 0)
			{
				if (noiseChip.RNG & 1) noiseChip.RNG ^= noiseChip.NoiseFB;
				noiseChip.RNG >>= 1;
				noiseChip.Output[3] = noiseChip.RNG & 1;
				noiseChip.Count[3] += noiseChip.Period[3];
				if (noiseChip.Output[3]) vol3 += noiseChip.Period[3];
			}
			if (noiseChip.Output[3]) vol3 -= noiseChip.Count[3];

			left -= nextevent;
		} while (left > 0);
	}
	out = vol3 * noiseChip.Volume[3];

	//if (out > MAX_OUTPUT * STEP) out = MAX_OUTPUT * STEP;
	if (out > MAX_OUTPUT_STEP) out = MAX_OUTPUT_STEP;

	return out>>STEP_SHIFT;//out / STEP;
}

//=============================================================================

void sound_update(_u16* chip_buffer, int length_bytes)
{
    length_bytes >>= 1;//turn it into words
	while (length_bytes)
	{
		//Mix a mono track out of: (Tone + Noise) >> 1
		//Write it to the sound buffer
		*(chip_buffer++) = (sample_chip_tone() + sample_chip_noise()) >> 1;

		length_bytes--;
	}
}

//=============================================================================

void WriteSoundChip(SoundChip* chip, _u8 data)
{
	//Command
	if (data & 0x80)
	{
		int r = (data & 0x70) >> 4;
		int c = r>>1;//r/2;

		chip->LastRegister = r;
		chip->Register[r] = (chip->Register[r] & 0x3f0) | (data & 0x0f);

		switch(r)
		{
		case 0:	/* tone 0 : frequency */
		case 2:	/* tone 1 : frequency */
		case 4:	/* tone 2 : frequency */
			chip->Period[c] = UpdateStep * chip->Register[r];
			if (chip->Period[c] == 0) chip->Period[c] = UpdateStep;
			if (r == 4)
			{
				/* update noise shift frequency */
				if ((chip->Register[6] & 0x03) == 0x03)
					chip->Period[3] = chip->Period[2]<<1;
			}
			break;

		case 1:	/* tone 0 : volume */
		case 3:	/* tone 1 : volume */
		case 5:	/* tone 2 : volume */
		case 7:	/* noise  : volume */
#ifdef NEOPOP_DEBUG
			if (filter_sound)
			{
				if (chip == &toneChip)
					system_debug_message("sound (T): Set Tone %d Volume to %d (0 = min, 15 = max)", c, 15 - (data & 0xF));
				else
					system_debug_message("sound (N): Set Tone %d Volume to %d (0 = min, 15 = max)", c, 15 - (data & 0xF));
			}
#endif
			chip->Volume[c] = VolTable[data & 0xF];
			break;

		case 6:	/* noise  : frequency, mode */
			{
				int n = chip->Register[6];
#ifdef NEOPOP_DEBUG
	if (filter_sound)
	{
		char *pm, *nm = "White";
		if ((n & 4)) nm = "Periodic";

		switch(n & 3)
		{
		case 0: pm = "N/512"; break;
		case 1: pm = "N/1024"; break;
		case 2: pm = "N/2048"; break;
		case 3: pm = "Tone#2"; break;
		}

		if (chip == &toneChip)
			system_debug_message("sound (T): Set Noise Mode to %s, Period = %s", nm, pm);
		else
			system_debug_message("sound (N): Set Noise Mode to %s, Period = %s", nm, pm);
	}
#endif
				chip->NoiseFB = (n & 4) ? FB_WNOISE : FB_PNOISE;
				n &= 3;
				/* N/512,N/1024,N/2048,Tone #2 output */
				chip->Period[3] = (n == 3) ? 2 * chip->Period[2] : (UpdateStep << (5+n));

				/* reset noise shifter */
				chip->RNG = NG_PRESET;
				chip->Output[3] = chip->RNG & 1;

			}
			break;
		}
	}
	else
	{
		int r = chip->LastRegister;
		int c = r/2;

		switch (r)
		{
			case 0:	/* tone 0 : frequency */
			case 2:	/* tone 1 : frequency */
			case 4:	/* tone 2 : frequency */
				chip->Register[r] = (chip->Register[r] & 0x0f) | ((data & 0x3f) << 4);
				chip->Period[c] = UpdateStep * chip->Register[r];
				if (chip->Period[c] == 0) chip->Period[c] = UpdateStep;
				if (r == 4)
				{
					/* update noise shift frequency */
					if ((chip->Register[6] & 0x03) == 0x03)
						chip->Period[3] = chip->Period[2]<<1;
				}
#ifdef NEOPOP_DEBUG
	if (filter_sound)
	{
		if (chip == &toneChip)
			system_debug_message("sound (T): Set Tone %d Frequency to %d", c, chip->Register[r]);
		else
			system_debug_message("sound (N): Set Tone %d Frequency to %d", c, chip->Register[r]);
	}
#endif
				break;
		}
	}
}

//=============================================================================

//#ifdef TARGET_PSP
void dac_writeL(unsigned char data)
{
    static int conv=5;

#if defined(TARGET_OD) || defined(TARGET_PSP)
    //pretend that conv=5.5 (44100/8000) conversion factor
    if(conv==5)
        conv=6;
    else
        conv=5;
#else
	conv=1;
#endif

    for(int i=0;i<conv;i++)
    {
        //Write to buffer
        dacBufferL[dacLBufferWrite++] = (data-0x80)<<8;

        //dacLBufferWrite++;
        if (dacLBufferWrite == DAC_BUFFERSIZE)
            dacLBufferWrite = 0;

        //Overflow?
        dacLBufferCount++;
        if (dacLBufferCount == DAC_BUFFERSIZE)
        {
            //dbg_printf("dac_write: DAC buffer overflow\nPlease report this to the author.");
            dacLBufferCount = 0;
        }
    }
}
//#endif

/*void dac_writeR(unsigned char data)
{
    SDL_LockAudio();
	//Write to buffer
	dacBufferR[dacRBufferWrite] = data;
	dacRBufferWrite++;
	if (dacRBufferWrite == DAC_BUFFERSIZE)
		dacRBufferWrite = 0;

	//Overflow?
	dacRBufferCount++;
	if (dacRBufferCount == DAC_BUFFERSIZE)
	{
		dbg_printf("dac_write: DAC buffer overflow\nPlease report this to the author.");
		dacRBufferCount = 0;
	}
    SDL_UnlockAudio();
}*/

void dac_mixer(_u16* stream, int length_bytes)
{
#if !defined(__GP32__) && !defined(TARGET_PSP) && !defined(TARGET_OD)
    int length_words = length_bytes>>1;
    length_bytes &= 0xFFFFFFFE; //make sure it's 16bit safe

    if(dacLBufferRead+length_words >= DAC_BUFFERSIZE)
    {
        SDL_MixAudio((Uint8*)stream, (Uint8*)&dacBufferL[dacLBufferRead], (DAC_BUFFERSIZE-dacLBufferRead)*2, volume); //mix it to the buffer
        SDL_MixAudio((Uint8*)&stream[DAC_BUFFERSIZE-dacLBufferRead], (Uint8*)dacBufferL, length_bytes-((DAC_BUFFERSIZE-dacLBufferRead)*2), volume); //mix it to the buffer
        dacLBufferRead = length_words-(DAC_BUFFERSIZE-dacLBufferRead);
    }
    else
    {
        SDL_MixAudio((Uint8*)stream, (Uint8*)&dacBufferL[dacLBufferRead], length_bytes, volume); //mix it to the buffer
        dacLBufferRead += length_words;
    }

    dacLBufferCount -= length_words; //need it in 16bits
#endif
}


void dac_update(_u16* dac_buffer, int length_bytes)
{
	while (length_bytes > 1)
	{
		//Copy then clear DAC data
#if defined(TARGET_PSP) || defined(TARGET_OD)
		*(dac_buffer++) |= dacBufferL[dacLBufferRead];
#else
		*(dac_buffer++) = dacBufferL[dacLBufferRead];
#endif
		dacBufferL[dacLBufferRead] = 0;  //silence?

		length_bytes -= 2;	// 1 byte = 8 bits

		if (dacLBufferCount > 0)
		{
			dacLBufferCount--;

			//Advance the DAC read
//			dacLBufferRead++;
			if (++dacLBufferRead == DAC_BUFFERSIZE)
				dacLBufferRead = 0;
		}
	}
}

//=============================================================================

//Resets the sound chips, also used whenever sound options are changed
void sound_init(int SampleRate)
{
	int i;
	double out;

	/* the base clock for the tone generators is the chip clock divided by 16; */
	/* for the noise generator, it is clock / 256. */
	/* Here we calculate the number of steps which happen during one sample */
	/* at the given sample rate. No. of events = sample rate / (clock/16). */
	/* STEP is a multiplier used to turn the fraction into a fixed point */
	/* number. */
	UpdateStep = (_u32)(((double)STEP * SampleRate * 16) / SOUNDCHIPCLOCK);

	//Initialise Left Chip
	memset(&toneChip, 0, sizeof(SoundChip));

	//Initialise Right Chip
	memset(&noiseChip, 0, sizeof(SoundChip));

	//Default register settings
	for (i = 0;i < 8;i+=2)
	{
		toneChip.Register[i] = 0;
		toneChip.Register[i + 1] = 0x0f;	/* volume = 0 */
		noiseChip.Register[i] = 0;
		noiseChip.Register[i + 1] = 0x0f;	/* volume = 0 */
	}

	for (i = 0;i < 4;i++)
	{
		toneChip.Output[i] = 0;
		toneChip.Period[i] = toneChip.Count[i] = UpdateStep;
		noiseChip.Output[i] = 0;
		noiseChip.Period[i] = noiseChip.Count[i] = UpdateStep;
	}

	//Build the volume table
	out = MAX_OUTPUT / 3;

	/* build volume table (2dB per step) */
	for (i = 0;i < 15;i++)
	{
		VolTable[i] = (_u32)out;
		out /= 1.258925412;	/* = 10 ^ (2/20) = 2dB */
	}
	VolTable[15] = 0;


	//Clear the DAC buffer
	for (i = 0; i < DAC_BUFFERSIZE; i++)
		dacBufferL[i] = 0;

	dacLBufferCount = 0;
	dacLBufferRead = 0;
	dacLBufferWrite = 0;
}

//=============================================================================




#if defined(TARGET_PSP) || defined(TARGET_OD)
#define NGPC_CHIP_FREQUENCY		44100
#else
#define NGPC_CHIP_FREQUENCY		8000
#endif
int chip_freq=NGPC_CHIP_FREQUENCY;//what we'd prefer

#define CHIPBUFFERLENGTH	35280

#define UNDEFINED		0xFFFFFF

// ====== Chip sound =========
//static LPDIRECTSOUNDBUFFER chipBuffer = NULL;	// Chip Buffer
// static int lastChipWrite = 0, chipWrite = UNDEFINED;	//Write Cursor

// ====== DAC sound =========
//static LPDIRECTSOUNDBUFFER dacBuffer = NULL;	// DAC Buffer
// static int lastDacWrite = 0, dacWrite = UNDEFINED;		//Write Cursor


_u8 blockSound[CHIPBUFFERLENGTH], blockDAC[CHIPBUFFERLENGTH];		// Gets filled with sound data.
unsigned int blockSoundWritePtr = 0;
unsigned int blockSoundReadPtr = 0;

void system_sound_chipreset(void)
{
	//Initialises sound chips, matching frequncies
	sound_init(chip_freq);
}

#ifdef __GP32__

int audioCallback(unsigned short* sndBuf,int len) {
	int i,smp;
	for (i=0;i<len;i++)
	{
		smp = (sample_chip_tone() + sample_chip_noise()) >> 1;
		smp = (smp + dacBufferL[dacLBufferRead]) >> 1;

		*(sndBuf++) = smp;
		*(sndBuf++) = smp; // stereo ?
		dacBufferL[dacLBufferRead] = 0;  //silence?
		if (dacLBufferCount > 0)
		{
			dacLBufferCount--;
			if (++dacLBufferRead == DAC_BUFFERSIZE)
				dacLBufferRead = 0;
		}
	}
	return 1;
}

#endif

#ifndef TARGET_PSP
void mixaudioCallback(void *userdata, unsigned char *stream, int len)
{
    sound_update((_u16*)stream, len); //Get sound data
    dac_update((_u16*)stream, len);
#if 0
#ifndef __GP32__
    sound_update((_u16*)blockSound, len);	//Get sound data
//    memcpy(stream, blockSound, len);        //put it in the buffer
    SDL_MixAudio(stream, blockSound, len, volume); //mix it to the buffer

    if(dacLBufferCount >= len)
    {
        //dac_update((_u16*)blockDAC, len);	//Get DAC data
        //SDL_MixAudio(stream, blockDAC, len, volume); //mix it to the buffer

        dac_mixer((_u16*)stream, len);	//Get DAC data
    }
#endif
#endif
}
#endif /* TARGET_PSP */

int sound_system_init()
{
#if !defined(__GP32__) && !defined(TARGET_PSP) && 0
    //set up SDL sound here?
    SDL_AudioSpec fmt, retFmt;

    fmt.freq = chip_freq;  //11025 is good for dac_ sound
    fmt.format = AUDIO_S16;
    fmt.channels = 1;
#ifdef TARGET_PSP
    fmt.samples = 512;
#else
    fmt.samples = 512;
#endif
    fmt.callback = mixaudioCallback;
    fmt.userdata = NULL;

    /* Open the audio device and start playing sound! */
    if ( SDL_OpenAudio(&fmt, &retFmt) < 0 ) {
        fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
        exit(1);
    }

    chip_freq = retFmt.freq;

	system_sound_chipreset();	//Resets chips

    //SDL_PauseAudio(0);
#else
	system_sound_chipreset();	//Resets chips
#endif
  return 1;
}


//call this every so often to update the sound output
void system_sound_update(void)
{
}



