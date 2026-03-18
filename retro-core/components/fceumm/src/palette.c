#ifdef TARGET_GNW
#include "build/config.h"
#include "build/mappers.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002,2003 Xodnizel
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

#include <stdlib.h>
#include <math.h>

#ifndef TARGET_GNW
#include <string/stdstring.h>
#include <file/file_path.h>
#include <streams/file_stream.h>
#endif

#include "fceu-types.h"
#include "fceu.h"
#include "general.h"
#include "driver.h"

#include "palette.h"
#include "palettes/palettes.h"

/* These are dynamically filled/generated palettes: */
pal palettei[64];		/* Custom palette for an individual game. */
pal palettec[64];		/* Custom "global" palette. */

static void ChoosePalette(void);
static void WritePalette(void);
extern uint8 pale;

pal *palo;
static pal *palpoint[8] =
{
	palette,
	rp2c04_0001,
	rp2c04_0002,
	rp2c04_0003,
	rp2c04_0004,
	rp2c03,
};

void FCEUI_SetPaletteArray(uint8 *pal) {
	if (!pal)
		palpoint[0] = palette;
	else {
		int x;
		palpoint[0] = palettec;
		for (x = 0; x < 64; x++) {
			palpoint[0][x].r = *((uint8*)pal + x + x + x);
			palpoint[0][x].g = *((uint8*)pal + x + x + x + 1);
			palpoint[0][x].b = *((uint8*)pal + x + x + x + 2);
		}
	}
	FCEU_ResetPalette();
}

static uint8 lastd = 0;
void SetNESDeemph(uint8 d, int force) {
	static uint16 rtmul[7] = { 32768 * 1.239, 32768 * .794, 32768 * 1.019, 32768 * .905, 32768 * 1.023, 32768 * .741, 32768 * .75 };
	static uint16 gtmul[7] = { 32768 * .915, 32768 * 1.086, 32768 * .98, 32768 * 1.026, 32768 * .908, 32768 * .987, 32768 * .75 };
	static uint16 btmul[7] = { 32768 * .743, 32768 * .882, 32768 * .653, 32768 * 1.277, 32768 * .979, 32768 * .101, 32768 * .75 };
	uint32 r, g, b;
	int x;

	/* If it's not forced(only forced when the palette changes),
	don't waste cpu time if the same deemphasis bits are set as the last call. */
	if (!force) {
		if (d == lastd)
			return;
	} else {	/* Only set this when palette has changed. */
		r = rtmul[6];
		g = rtmul[6];
		b = rtmul[6];

		for (x = 0; x < 0x40; x++) {
			uint32 m, n, o;
			m = palo[x].r;
			n = palo[x].g;
			o = palo[x].b;
			m = (m * r) >> 15;
			n = (n * g) >> 15;
			o = (o * b) >> 15;
			if (m > 0xff) m = 0xff;
			if (n > 0xff) n = 0xff;
			if (o > 0xff) o = 0xff;
			FCEUD_SetPalette(x | 0xC0, m, n, o);
		}
	}
	if (!d) return;	/* No deemphasis, so return. */

	r = rtmul[d - 1];
	g = gtmul[d - 1];
	b = btmul[d - 1];

	for (x = 0; x < 0x40; x++) {
		uint32 m, n, o;

		m = palo[x].r;
		n = palo[x].g;
		o = palo[x].b;
		m = (m * r) >> 15;
		n = (n * g) >> 15;
		o = (o * b) >> 15;
		if (m > 0xff) m = 0xff;
		if (n > 0xff) n = 0xff;
		if (o > 0xff) o = 0xff;

		FCEUD_SetPalette(x | 0x40, m, n, o);
	}

	lastd = d;
}

int ipalette = 0;

#ifndef TARGET_GNW
void FCEU_LoadGamePalette(void) {
	uint8 ptmp[192];
	RFILE *fp = NULL;
	char *fn = NULL;

	ipalette = 0;

	fn = FCEU_MakeFName(FCEUMKF_PALETTE, 0, 0);

	if (!string_is_empty(fn) && path_is_valid(fn))
		fp = filestream_open(fn,
				RETRO_VFS_FILE_ACCESS_READ,
				RETRO_VFS_FILE_ACCESS_HINT_NONE);

	if (fp) {
		int x;
		filestream_read(fp, ptmp, 192);
		filestream_close(fp);
		for (x = 0; x < 64; x++) {
			palettei[x].r = ptmp[x + x + x];
			palettei[x].g = ptmp[x + x + x + 1];
			palettei[x].b = ptmp[x + x + x + 2];
		}
		ipalette = 1;
	}
	free(fn);
}
#endif

void FCEU_ResetPalette(void) {
	if (GameInfo) {
		ChoosePalette();
		WritePalette();
	}
}

static void ChoosePalette(void) {
#if !defined(TARGET_GNW) || defined(NES_MAPPER_NSF)
	if (GameInfo->type == GIT_NSF)
		palo = 0;
	else
#endif
	if (ipalette)
		palo = palettei;
	else
		palo = palpoint[pale];
}

void WritePalette(void) {
	int x;

	for (x = 0; x < 7; x++)
		FCEUD_SetPalette(x, unvpalette[x].r, unvpalette[x].g, unvpalette[x].b);
#if !defined(TARGET_GNW) || defined(NES_MAPPER_NSF)
	if (GameInfo->type == GIT_NSF) {
	} else
#endif
	{
		for (x = 0; x < 64; x++)
			FCEUD_SetPalette(128 + x, palo[x].r, palo[x].g, palo[x].b);
		SetNESDeemph(lastd, 1);
	}
}
#endif