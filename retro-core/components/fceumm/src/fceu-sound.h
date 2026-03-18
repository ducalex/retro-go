/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef _FCEU_SOUND_H
#define _FCEU_SOUND_H
#include "fceu.h"

typedef struct {
  void (*Fill)(int Count); /* Low quality ext sound. */

  void (*NeoFill)(int32 *Wave, int Count);
  void (*HiFill)(void);
  void (*HiSync)(int32 ts);

  void (*RChange)(void);
  void (*Kill)(void);
} EXPSOUND;

extern EXPSOUND GameExpSound;

extern int32 nesincsize;

void SetSoundVariables(void);

int GetSoundBuffer(int32 **W);
int FlushEmulateSound(void);
extern int32 *Wave;
extern int32 *WaveFinal;
extern int32 *WaveHi;

#define WAVE_ARRAY_SIZE (2048 + 512)
#define WAVEHI_ARRAY_SIZE 40000
extern uint32 soundtsinc;

extern uint32 soundtsoffs;
#define SOUNDTS (sound_timestamp + soundtsoffs)

void SetNESSoundMap(void);
void FrameSoundUpdate(void);

void FCEUSND_Power(void);
void FCEUSND_Reset(void);
void FCEUSND_SaveState(void);
void FCEUSND_LoadState(int version);

void FASTAPASS(1) FCEU_SoundCPUHook(int);

DECLFR(StatusRead);
DECLFW(Write_PSG);
DECLFW(Write_DMCRegs);
DECLFW(StatusWrite);
DECLFW(Write_IRQFM);

#endif
