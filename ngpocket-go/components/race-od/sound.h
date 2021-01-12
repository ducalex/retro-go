//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

// sound.h: interface for the sound class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOUND_H__15E0A182_A0FD_11D4_8645_0050DA4EEEA0__INCLUDED_)
#define AFX_SOUND_H__15E0A182_A0FD_11D4_8645_0050DA4EEEA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StdAfx.h"
#include "main.h"

int initSound();
void soundCleanup();

#ifndef __GP32__
void setHwnd(HWND hWnd);
#endif

//
// stolen MAME things
//
int osd_start_audio_stream(int stereo);
void osd_stop_audio_stream(void);
int osd_update_audio_stream(short *buffer);
void osd_set_mastervolume(int _attenuation);
int osd_get_mastervolume(void);

//
// General sound system functions
//
void soundStep(int cycles);
void soundOutput();

#define NUM_CHANNELS 32
#if !defined(__GP32__) && !defined(TARGET_PSP) && !defined(TARGET_OD)
extern Mix_Chunk *chunks[NUM_CHANNELS];
#endif

//
// Gameboy sound system
//
void gbSoundInit();
void gbSoundUpdate();
void gbSoundWrite(int reg, unsigned char data);

///
/// Neogeo pocket sound functions
///
//ALEK void sound_update(void);
void ngpSoundStart();
void ngpSoundExecute();
void ngpSoundOff();
void ngpSoundInterrupt();

#endif // !defined(AFX_SOUND_H__15E0A182_A0FD_11D4_8645_0050DA4EEEA0__INCLUDED_)
