#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* Begin FDS sound */
#include <string.h>
#include "fceu-types.h"
#include "x6502.h"
#include "fceu.h"
#include "fceu-sound.h"
#include "fceu-state.h"

#ifndef TARGET_GNW
#define FDSClock (1789772.7272727272727272 / 2)
#else
#define FDSClock (1789000 / 2) // this value allow to reduce audio glitches on G&W with FDS Games
#endif

typedef struct {
	int64 cycles;		/* Cycles per PCM sample */
	int64 count;		/* Cycle counter */
	int64 envcount;		/* Envelope cycle counter */
	uint32 b19shiftreg60;
	uint32 b24adder66;
	uint32 b24latch68;
	uint32 b17latch76;
	int32 clockcount;	/* Counter to divide frequency by 8. */
	uint8 b8shiftreg88;	/* Modulation register. */
	uint8 amplitude[2];	/* Current amplitudes. */
	uint8 speedo[2];
	uint8 mwcount;
	uint8 mwstart;
	uint8 mwave[0x20];	/* Modulation waveform */
	uint8 cwave[0x40];	/* Game-defined waveform(carrier) */
	uint8 SPSG[0xB];
} FDSSOUND;

static FDSSOUND fdso;

#define  SPSG  fdso.SPSG
#define b19shiftreg60  fdso.b19shiftreg60
#define b24adder66  fdso.b24adder66
#define b24latch68  fdso.b24latch68
#define b17latch76  fdso.b17latch76
#define b8shiftreg88  fdso.b8shiftreg88
#define clockcount  fdso.clockcount
#define amplitude  fdso.amplitude
#define speedo    fdso.speedo

void FDSSoundStateAdd(void) {
	AddExState(fdso.cwave, 64, 0, "WAVE");
	AddExState(fdso.mwave, 32, 0, "MWAV");
	AddExState(amplitude, 2, 0, "AMPL");
	AddExState(SPSG, 0xB, 0, "SPSG");

	AddExState(&b8shiftreg88, 1, 0, "B88");

	AddExState(&clockcount, 4, 1, "CLOC");
	AddExState(&b19shiftreg60, 4, 1, "B60");
	AddExState(&b24adder66, 4, 1, "B66");
	AddExState(&b24latch68, 4, 1, "B68");
	AddExState(&b17latch76, 4, 1, "B76");
}

DECLFR(FDSSRead) {
	switch (A & 0xF) {
	case 0x0: return(amplitude[0] | (X.DB & 0xC0));
	case 0x2: return(amplitude[1] | (X.DB & 0xC0));
	}
	return(X.DB);
}

static void RenderSound(void);
#ifndef TARGET_GNW
static void RenderSoundHQ(void);
#endif

DECLFW(FDSSWrite) {
	if (FSettings.SndRate) {
#ifndef TARGET_GNW
		if (FSettings.soundq >= 1)
			RenderSoundHQ();
		else
#endif
			RenderSound();
	}
	A -= 0x4080;
	switch (A) {
	case 0x0:
	case 0x4:
		if (V & 0x80)
			amplitude[(A & 0xF) >> 2] = V & 0x3F;
		break;
	case 0x7:
		b17latch76 = 0;
		SPSG[0x5] = 0;
		break;
	case 0x8:
		b17latch76 = 0;
		fdso.mwave[SPSG[0x5] & 0x1F] = V & 0x7;
		SPSG[0x5] = (SPSG[0x5] + 1) & 0x1F;
		break;
	}
	SPSG[A] = V;
}

/* $4080 - Fundamental wave amplitude data register 92
 * $4082 - Fundamental wave frequency data register 58
 * $4083 - Same as $4082($4083 is the upper 4 bits).
 *
 * $4084 - Modulation amplitude data register 78
 * $4086 - Modulation frequency data register 72
 * $4087 - Same as $4086($4087 is the upper 4 bits)
 */


static void DoEnv() {
	int x;

	for (x = 0; x < 2; x++)
		if (!(SPSG[x << 2] & 0x80) && !(SPSG[0x3] & 0x40)) {
			static int counto[2] = { 0, 0 };

			if (counto[x] <= 0) {
				if (!(SPSG[x << 2] & 0x80)) {
					if (SPSG[x << 2] & 0x40) {
						if (amplitude[x] < 0x3F)
							amplitude[x]++;
					} else {
						if (amplitude[x] > 0)
							amplitude[x]--;
					}
				}
				counto[x] = (SPSG[x << 2] & 0x3F);
			} else
				counto[x]--;
		}
}

DECLFR(FDSWaveRead) {
	return(fdso.cwave[A & 0x3f] | (X.DB & 0xC0));
}

DECLFW(FDSWaveWrite) {
	if (SPSG[0x9] & 0x80)
		fdso.cwave[A & 0x3f] = V & 0x3F;
}

static int ta;
static INLINE void ClockRise(void) {
	if (!clockcount) {
		ta++;

		b19shiftreg60 = (SPSG[0x2] | ((SPSG[0x3] & 0xF) << 8));
		b17latch76 = (SPSG[0x6] | ((SPSG[0x07] & 0xF) << 8)) + b17latch76;

		if (!(SPSG[0x7] & 0x80)) {
			int t = fdso.mwave[(b17latch76 >> 13) & 0x1F] & 7;
			int t2 = amplitude[1];
			int adj = 0;

			if ((t & 3)) {
				if ((t & 4))
					adj -= (t2 * ((4 - (t & 3))));
				else
					adj += (t2 * ((t & 3)));
			}
			adj *= 2;
			if (adj > 0x7F) adj = 0x7F;
			if (adj < -0x80) adj = -0x80;
			b8shiftreg88 = 0x80 + adj;
		} else {
			b8shiftreg88 = 0x80;
		}
	} else {
		b19shiftreg60 <<= 1;
		b8shiftreg88 >>= 1;
	}
	b24adder66 = (b24latch68 + b19shiftreg60) & 0x1FFFFFF;
}

static INLINE void ClockFall(void) {
	if ((b8shiftreg88 & 1))
		b24latch68 = b24adder66;
	clockcount = (clockcount + 1) & 7;
}

static INLINE int32 FDSDoSound(void) {
	fdso.count += fdso.cycles;
	if (fdso.count >= ((int64)1 << 40)) {
 dogk:
		fdso.count -= (int64)1 << 40;
		ClockRise();
		ClockFall();
		fdso.envcount--;
		if (fdso.envcount <= 0) {
			fdso.envcount += SPSG[0xA] * 3;
			DoEnv();
		}
	}
	if (fdso.count >= 32768) goto dogk;

	/* Might need to emulate applying the amplitude to the waveform a bit better... */
	{
		int k = amplitude[0];
		if (k > 0x20) k = 0x20;
		return (fdso.cwave[b24latch68 >> 19] * k) * 4 / ((SPSG[0x9] & 0x3) + 2);
	}
}

static int32 FBC = 0;

static void RenderSound(void) {
	int32 end, start;
	int32 x;

	start = FBC;
	end = (SOUNDTS << 16) / soundtsinc;
	if (end <= start)
		return;
	FBC = end;

	if (!(SPSG[0x9] & 0x80))
		for (x = start; x < end; x++) {
			uint32 t = FDSDoSound();
			t += t >> 1;
			t >>= 4;
			Wave[x >> 4] += t;	/* (t>>2)-(t>>3); */ /* >>3; */
		}
}

#ifndef TARGET_GNW
static void RenderSoundHQ(void) {
	uint32 x;

	if (!(SPSG[0x9] & 0x80))
		for (x = FBC; x < SOUNDTS; x++) {
			uint32 t = FDSDoSound();
			t += t >> 1;
			WaveHi[x] += t;	/* (t<<2)-(t<<1); */
		}
	FBC = SOUNDTS;
}

static void HQSync(int32 ts) {
	FBC = ts;
}
#endif

void FDSSound(int c) {
	RenderSound();
	FBC = c;
}

static void FDS_ESI(void) {
	if (FSettings.SndRate) {
#ifndef TARGET_GNW
		if (FSettings.soundq >= 1) {
			fdso.cycles = (int64)1 << 39;
		} else
#endif
		{
			fdso.cycles = ((int64)1 << 40) * FDSClock;
			fdso.cycles /= FSettings.SndRate * 16;
		}
	}
#ifndef FCEU_LOW_RAM
	SetReadHandler(0x4040, 0x407f, FDSWaveRead);
	SetWriteHandler(0x4040, 0x407f, FDSWaveWrite);
	SetWriteHandler(0x4080, 0x408A, FDSSWrite);
	SetReadHandler(0x4090, 0x4092, FDSSRead);
#endif
}

void FDSSoundReset(void) {
	memset(&fdso, 0, sizeof(fdso));
	FDS_ESI();
#ifndef TARGET_GNW
	GameExpSound.HiSync = HQSync;
	GameExpSound.HiFill = RenderSoundHQ;
#endif
	GameExpSound.Fill = FDSSound;
	GameExpSound.RChange = FDS_ESI;
}

uint8 FDSSoundRead(uint32 A) {
	if (A >= 0x4040 && A < 0x4080) return FDSWaveRead(A);
	if (A >= 0x4090 && A < 0x4093) return FDSSRead(A);
	return X.DB;
}

void FDSSoundWrite(uint32 A, uint8 V) {
	if (A >= 0x4040 && A < 0x4080) FDSWaveWrite(A, V);
	else if (A >= 0x4080 && A < 0x408B) FDSSWrite(A, V);
}

void FDSSoundPower(void) {
	FDSSoundReset();
	FDSSoundStateAdd();
}
#endif