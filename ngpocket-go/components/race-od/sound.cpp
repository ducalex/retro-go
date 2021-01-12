//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

// sound.cpp: implementation of the sound class.
//
//////////////////////////////////////////////////////////////////////

//Flavor - Convert from DirectSound to SDL

#include "StdAfx.h"
#include "main.h"
#include "sound.h"
#include "memory.h"

#ifdef DRZ80
#include "DrZ80_support.h"
#else
#if defined(CZ80)
#include "cz80_support.h"
#else
#include "z80.h"
#endif
#endif

#include <math.h>

#define Machine (&m_emuInfo)


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int sndCycles = 0;

void soundStep(int cycles)
{
	sndCycles+= cycles;
}

/////////////////////////////////////////////////////////////////////////////////
///
/// Neogeo Pocket Sound system
///
/////////////////////////////////////////////////////////////////////////////////

unsigned int	ngpRunning;

void ngpSoundStart()
{
	ngpRunning = 1;	// ?
#if defined(DRZ80) || defined(CZ80)
    Z80_Reset();
#else
	z80Init();
	z80SetRunning(1);
#endif
}

/// Execute all gained cycles (divided by 2)
void ngpSoundExecute()
{
#if defined(DRZ80) || defined(CZ80)
    int toRun = sndCycles/2;
    if(ngpRunning)
    {
        Z80_Execute(toRun);
    }
    //timer_add_cycles(toRun);
    sndCycles -= toRun;
#else
	int		elapsed;
	while(sndCycles > 0)
	{
		elapsed = z80Step();
		sndCycles-= (2*elapsed);
		//timer_add_cycles(elapsed);
	}
#endif
}

/// Switch sound system off
void ngpSoundOff() {
	ngpRunning = 0;
#if defined(DRZ80) || defined(CZ80)

#else
	z80SetRunning(0);
#endif
}

// Generate interrupt to ngp sound system
void ngpSoundInterrupt() {
	if (ngpRunning)
	{
#if defined(DRZ80) || defined(CZ80)
        Z80_Cause_Interrupt(0x100);//Z80_IRQ_INT???
#else
		z80Interrupt(0);
#endif
	}
}

