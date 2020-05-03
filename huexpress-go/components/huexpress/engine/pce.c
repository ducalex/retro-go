/*
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU Library General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/************************************************************************/
/*   'Portable' PC-Engine Emulator Source file							*/
/*                                                                      */
/*	1998 by BERO bero@geocities.co.jp                                   */
/*                                                                      */
/*  Modified 1998 by hmmx hmmx@geocities.co.jp                          */
/*	Modified 1999-2005 by Zeograd (Olivier Jolly) zeograd@zeograd.com   */
/*	Modified 2011-2013 by Alexander von Gluck kallisti5@unixzen.com     */
/************************************************************************/

/* Header section */

#include "pce.h"
#include "utils.h"
#include "romdb.h"

/* Variable section */

struct host_machine host;

uchar *ROM = NULL;

uint ROM_size, ROM_crc, ROM_idx;

const uint ScanlinesPerFrame = 263;
const uint BaseClock = 7800000;
const uint IPeriod = 494; // BaseClock / (ScanlinesPerFrame * 60);

const char *SAVESTATE_HEADER = "PCE_V001";

static svar_t svars[] =
{
	// Memory and arrays
	SVAR_A("RAM ", &RAM, RAMSIZE),
	SVAR_A("BRAM", &BackupRAM, BRAMSIZE),
	SVAR_A("VRAM", &VRAM, VRAMSIZE),
	SVAR_A("SPRAM", &VRAM, VRAMSIZE),
	SVAR_A("PAL", &Palette, 512),
	SVAR_A("MMR", &MMR, 8),

	// CPU registers
	SVAR_2("CPU.PC", &reg_pc),
	SVAR_1("CPU.A", &reg_a),
	SVAR_1("CPU.X", &reg_x),
	SVAR_1("CPU.Y", &reg_y),
	SVAR_1("CPU.P", &reg_p),
	SVAR_1("CPU.S", &reg_s),

	// Counters
	SVAR_2("Scanline", &Scanline),
	SVAR_2("Cycles", &Cycles),
	SVAR_2("TCycles", &TotalCycles),
	SVAR_2("PTCycles", &PrevTotalCycles),

	// IO

	SVAR_END
};

/*****************************************************************************

		Function: LoadCard

		Description: load a card
		Parameters: char* name (the filename to load)
		Return: -1 on error else 0

*****************************************************************************/
int
LoadCard(char *name)
{
	if (ROM != NULL) {
		free(ROM); ROM = NULL;
	}

	MESSAGE_INFO("Opening %s...\n", name);

	FILE *fp = fopen(name, "rb");

	if (fp == NULL)
	{
		MESSAGE_ERROR("Failed to open %s!\n", name);
		return -1;
	}

	// find file size
	fseek(fp, 0, SEEK_END);
	int fsize = ftell(fp);

	// ajust var if header present
	fseek(fp, fsize & 0x1fff, SEEK_SET);
	fsize &= ~0x1fff;

	// read ROM
	ROM = (uchar *)rg_alloc(fsize, MEM_SLOW);
	if (ROM == NULL)
	{
		MESSAGE_ERROR("Failed to allocate ROM buffer!\n");
		return -1;
	}

	fread(ROM, 1, fsize, fp);

	fclose(fp);

	ROM_size = fsize / 0x2000;
	ROM_crc = CRC_buffer(ROM, ROM_size * 0x2000);
	ROM_idx = 0xFFFF;

	for (int index = 0; index < KNOWN_ROM_COUNT; index++) {
		if (ROM_crc == kKnownRoms[index].CRC) {
			ROM_idx = index;
			break;
		}
	}

	if (ROM_idx == 0xFFFF)
	{
		MESSAGE_INFO("ROM not in database: CRC=%lx\n", ROM_crc);
	}
	else
	{
		MESSAGE_INFO("Rom Name: %s\n", kKnownRoms[ROM_idx].Name);
		MESSAGE_INFO("Publisher: %s\n", kKnownRoms[ROM_idx].Publisher);

		// US Encrypted
		if ((kKnownRoms[ROM_idx].Flags & US_ENCODED) || ROM[0x1FFF] < 0xE0)
		{
			MESSAGE_INFO("This rom is probably US encrypted, decrypting...\n");

			uchar inverted_nibble[16] = {
				0, 8, 4, 12, 2, 10, 6, 14,
				1, 9, 5, 13, 3, 11, 7, 15
			};

			for (uint32 x = 0; x < ROM_size * 0x2000; x++) {
				uchar temp = ROM[x] & 15;

				ROM[x] &= ~0x0F;
				ROM[x] |= inverted_nibble[ROM[x] >> 4];

				ROM[x] &= ~0xF0;
				ROM[x] |= inverted_nibble[temp] << 4;
			}
		}

		// For example with Devil Crush 512Ko
		if (kKnownRoms[ROM_idx].Flags & TWO_PART_ROM)
			ROM_size = 0x30;

		// Hu Card with onboard RAM
		if (kKnownRoms[ROM_idx].Flags & POPULOUS)
		{
			MESSAGE_INFO("Special Rom: Populous detected!\n");
			if (!ExtraRAM)
				ExtraRAM = (uchar*)rg_alloc(0x8000, MEM_FAST);
		}
	}

	// Hu Card with onboard RAM (Populous)
	if (ExtraRAM) {
		MemoryMapR[0x40] = MemoryMapW[0x40] = ExtraRAM;
		MemoryMapR[0x41] = MemoryMapW[0x41] = ExtraRAM + 0x2000;
		MemoryMapR[0x42] = MemoryMapW[0x42] = ExtraRAM + 0x4000;
		MemoryMapR[0x43] = MemoryMapW[0x43] = ExtraRAM + 0x6000;
	}

    // Game ROM
	int ROMmask = 1;
	while (ROMmask < ROM_size)
		ROMmask <<= 1;
	ROMmask--;

	MESSAGE_INFO("ROMmask=%02X, ROM_size=%02X\n", ROMmask, ROM_size);

	for (int i = 0; i < 0x80; i++) {
		if (ROM_size == 0x30) {
			switch (i & 0x70) {
			case 0x00:
			case 0x10:
			case 0x50:
				MemoryMapR[i] = ROM + (i & ROMmask) * 0x2000;
				break;
			case 0x20:
			case 0x60:
				MemoryMapR[i] = ROM + ((i - 0x20) & ROMmask) * 0x2000;
				break;
			case 0x30:
			case 0x70:
				MemoryMapR[i] = ROM + ((i - 0x10) & ROMmask) * 0x2000;
				break;
			case 0x40:
				MemoryMapR[i] = ROM + ((i - 0x20) & ROMmask) * 0x2000;
				break;
			}
		} else {
			MemoryMapR[i] = ROM + (i & ROMmask) * 0x2000;
		}
	}

	return 0;
}


void
ResetPCE(bool hard)
{
	hard_reset();
}


int
InitPCE(char *name)
{
	if (osd_init_input())
		return 1;

	if (gfx_init())
		return 1;

	if (snd_init())
		return 1;

	if (hard_init())
		return 1;

	if (LoadCard(name))
		return 1;

	ResetPCE(0);

	return 0;
}


void
RunPCE(void)
{
	exe_go();
}


int
LoadState(char *name)
{
	MESSAGE_INFO("Loading state from %s...\n", name);

	char *buffer[1024];

	FILE *fp = fopen(name, "rb");
	if (fp == NULL)
		return -1;

	fread(&buffer, sizeof(SAVESTATE_HEADER), 1, fp);

	if (memcmp(SAVESTATE_HEADER, buffer, sizeof(SAVESTATE_HEADER)) != 0)
	{
		goto load_failed;
	}

	// fread(&PCE, sizeof(PCE), 1, fp);
	// fread(&saved_gfx_context, sizeof(saved_gfx_context), 1, fp);
	fclose(fp);

	memset(&SPR_CACHE, 0, sizeof(SPR_CACHE));

	for (int i = 0; i < 8; i++) {
		bank_set(i, MMR[i]);
	}

	return 0;

load_failed:
	fclose(fp);
	return -1;
}


int
SaveState(char *name)
{
	MESSAGE_INFO("Saving state to %s...\n", name);

	FILE *fp = fopen(name, "wb");
	if (fp == NULL)
		return -1;

	fwrite(&SAVESTATE_HEADER, sizeof(SAVESTATE_HEADER), 1, fp);

	// hard_save_state(fp);
	// gfx_save_state(fp);
	// snd_save_state(fp);

	fclose(fp);

	return 0;
}


void
ShutdownPCE()
{
	gfx_term();
	snd_term();
	hard_term();

	exit(0);
}
