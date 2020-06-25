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
 *
 *  Created  1998 by BERO bero@geocities.co.jp
 *  Modified 1998 by hmmx hmmx@geocities.co.jp
 *	Modified 1999-2005 by Zeograd (Olivier Jolly) zeograd@zeograd.com
 *	Modified 2011-2013 by Alexander von Gluck kallisti5@unixzen.com
 */

// pce.c - Entry file to start/stop/reset/save emulation
//
#include "pce.h"
#include "romdb.h"
#include "rom/crc.h"

struct host_machine host;

const uint ScanlinesPerFrame = 263;
const uint BaseClock = 7800000;
const uint IPeriod = 494; // BaseClock / (ScanlinesPerFrame * 60);
const char SAVESTATE_HEADER[8] = "PCE_V001";

/**
 * Describes what is saved in a save state. Changing the order will break
 * previous saves so add a place holder if necessary. Eventually we could use
 * the keys to make order irrelevant...
 */
const svar_t SaveStateVars[] =
{
	// Arrays
	SVAR_A("RAM", RAM),       SVAR_A("BRAM", BackupRAM), SVAR_A("VRAM", VRAM),
	SVAR_A("SPRAM", SPRAM),   SVAR_A("PAL", Palette),    SVAR_A("MMR", MMR),

	// CPU registers
	SVAR_2("CPU.PC", reg_pc), SVAR_1("CPU.A", reg_a),    SVAR_1("CPU.X", reg_x),
	SVAR_1("CPU.Y", reg_y),   SVAR_1("CPU.P", reg_p),    SVAR_1("CPU.S", reg_s),

	// Counters
	SVAR_2("Scanline", Scanline),              SVAR_2("Cycles", Cycles),
	SVAR_4("TotalCycles", TotalCycles),        SVAR_4("PrevTCycles", PrevTotalCycles),

	// PSG
	SVAR_A("PSG", io.PSG),                     SVAR_A("PSG_WAVE", io.PSG_WAVE),
	SVAR_A("psg_da_data", io.psg_da_data),     SVAR_A("psg_da_count", io.psg_da_count),
	SVAR_A("psg_da_index", io.psg_da_index),   SVAR_1("psg_ch", io.psg_ch),
	SVAR_1("psg_volume", io.psg_volume),       SVAR_1("psg_lfo_freq", io.psg_lfo_freq),
	SVAR_1("psg_lfo_ctrl", io.psg_lfo_ctrl),

	// IO
	SVAR_A("VCE", io.VCE),                     SVAR_2("vce_reg", io.vce_reg),

	SVAR_A("VDC", io.VDC),                     SVAR_1("vdc_inc", io.vdc_inc),
	SVAR_1("vdc_reg", io.vdc_reg),             SVAR_1("vdc_status", io.vdc_status),
	SVAR_1("vdc_ratch", io.vdc_ratch),         SVAR_1("vdc_satb", io.vdc_satb),
	SVAR_1("vdc_satb_c", io.vdc_satb_counter), SVAR_1("vdc_pendvsync", io.vdc_pendvsync),
	SVAR_2("vdc_minline", io.vdc_minline),     SVAR_2("vdc_maxline", io.vdc_maxline),
	SVAR_2("screen_w", io.screen_w),           SVAR_2("screen_h", io.screen_h),
	SVAR_2("bg_w", io.bg_w),                   SVAR_2("bg_h", io.bg_h),

	SVAR_1("timer_reload", io.timer_reload),   SVAR_1("timer_start", io.timer_start),
	SVAR_1("timer_counter", io.timer_counter), SVAR_1("irq_mask", io.irq_mask),
	SVAR_1("irq_status", io.irq_status),

	SVAR_END
};


/**
 * Load card into memory and set its memory map
 */
int
LoadCard(const char *name)
{
	int fsize, offset;

	MESSAGE_INFO("Opening %s...\n", name);

	FILE *fp = fopen(name, "rb");

	if (fp == NULL)
	{
		MESSAGE_ERROR("Failed to open %s!\n", name);
		return -1;
	}

	if (ROM != NULL) {
		free(ROM);
	}

	// find file size
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	offset = fsize & 0x1fff;

	// read ROM
	ROM = (uchar *)rg_alloc(fsize, MEM_SLOW);
	ROM_SIZE = (fsize - offset) / 0x2000;
	ROM_PTR = ROM + offset;

	if (ROM == NULL)
	{
		MESSAGE_ERROR("Failed to allocate ROM buffer!\n");
		return -1;
	}

	fseek(fp, 0, SEEK_SET);
	fread(ROM, 1, fsize, fp);

	fclose(fp);

	uint32 CRC = crc32_le(0, ROM, fsize);
	uint16 IDX = 0;
	uint32 ROM_MASK = 1;

	while (ROM_MASK < ROM_SIZE) ROM_MASK <<= 1;
	ROM_MASK--;

	MESSAGE_INFO("ROM LOADED: OFFSET=%d, BANKS=%d, MASK=%03X, CRC=%08X\n", offset, ROM_SIZE, ROM_MASK, CRC);

	for (int index = 0; index < KNOWN_ROM_COUNT; index++) {
		if (CRC == romFlags[index].CRC) {
			IDX = index;
			break;
		}
	}

	MESSAGE_INFO("Game Name: %s\n", romFlags[IDX].Name);
	MESSAGE_INFO("Game Region: %s\n", (romFlags[IDX].Flags & JAP) ? "Japan" : "USA");

	// US Encrypted
	if ((romFlags[IDX].Flags & US_ENCODED) || ROM_PTR[0x1FFF] < 0xE0)
	{
		MESSAGE_INFO("This rom is probably US encrypted, decrypting...\n");

		uchar inverted_nibble[16] = {
			0, 8, 4, 12, 2, 10, 6, 14,
			1, 9, 5, 13, 3, 11, 7, 15
		};

		for (uint32 x = 0; x < ROM_SIZE * 0x2000; x++) {
			uchar temp = ROM_PTR[x] & 15;

			ROM_PTR[x] &= ~0x0F;
			ROM_PTR[x] |= inverted_nibble[ROM_PTR[x] >> 4];

			ROM_PTR[x] &= ~0xF0;
			ROM_PTR[x] |= inverted_nibble[temp] << 4;
		}
	}

	// For example with Devil Crush 512Ko
	if (romFlags[IDX].Flags & TWO_PART_ROM)
		ROM_SIZE = 0x30;

    // Game ROM
	for (int i = 0; i < 0x80; i++) {
		if (ROM_SIZE == 0x30) {
			switch (i & 0x70) {
			case 0x00:
			case 0x10:
			case 0x50:
				MemoryMapR[i] = ROM_PTR + (i & ROM_MASK) * 0x2000;
				break;
			case 0x20:
			case 0x60:
				MemoryMapR[i] = ROM_PTR + ((i - 0x20) & ROM_MASK) * 0x2000;
				break;
			case 0x30:
			case 0x70:
				MemoryMapR[i] = ROM_PTR + ((i - 0x10) & ROM_MASK) * 0x2000;
				break;
			case 0x40:
				MemoryMapR[i] = ROM_PTR + ((i - 0x20) & ROM_MASK) * 0x2000;
				break;
			}
		} else {
			MemoryMapR[i] = ROM_PTR + (i & ROM_MASK) * 0x2000;
		}
		MemoryMapW[i] = TRAPRAM;
	}

	// Always allocate extra RAM if size <= 512K (Populous and some homebrews)
	if (ROM_SIZE <= 0x40) {
		ExtraRAM = ExtraRAM ?: (uchar*)rg_alloc(0x8000, MEM_FAST);
		MemoryMapR[0x40] = MemoryMapW[0x40] = ExtraRAM;
		MemoryMapR[0x41] = MemoryMapW[0x41] = ExtraRAM + 0x2000;
		MemoryMapR[0x42] = MemoryMapW[0x42] = ExtraRAM + 0x4000;
		MemoryMapR[0x43] = MemoryMapW[0x43] = ExtraRAM + 0x6000;
	}

    // Mapper for roms >= 1.5MB (SF2, homebrews)
    if (ROM_SIZE >= 192)
        MemoryMapW[0x00] = IOAREA;

	return 0;
}


/**
 * Reset the emulator
 */
void
ResetPCE(bool hard)
{
	hard_reset();
}


/**
 * Initialize the emulator (allocate memory, call osd_init* functions)
 */
int
InitPCE(const char *name)
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


/**
 * Start the emulation
 */
void
RunPCE(void)
{
	exe_go();
}


/**
 * Load saved state
 */
int
LoadState(char *name)
{
	MESSAGE_INFO("Loading state from %s...\n", name);

	char buffer[512];

	FILE *fp = fopen(name, "rb");
	if (fp == NULL)
		return -1;

	fread(&buffer, 8, 1, fp);

	if (memcmp(&buffer, SAVESTATE_HEADER, 8) != 0)
	{
		MESSAGE_ERROR("Loading state failed: Header mismatch\n");
		fclose(fp);
		return -1;
	}

	for (int i = 0; SaveStateVars[i].len > 0; i++)
	{
		MESSAGE_INFO("Loading %s (%d)\n", SaveStateVars[i].key, SaveStateVars[i].len);
		fread(SaveStateVars[i].ptr, SaveStateVars[i].len, 1, fp);
	}

	for(int i = 0; i < 8; i++)
	{
		BankSet(i, MMR[i]);
	}

	memset(&SPR_CACHE, 0, sizeof(SPR_CACHE));

	osd_gfx_set_mode(io.screen_w, io.screen_h);

	fclose(fp);

	return 0;
}


/**
 * Save current state
 */
int
SaveState(char *name)
{
	MESSAGE_INFO("Saving state to %s...\n", name);

	FILE *fp = fopen(name, "wb");
	if (fp == NULL)
		return -1;

	fwrite(SAVESTATE_HEADER, sizeof(SAVESTATE_HEADER), 1, fp);

	for (int i = 0; SaveStateVars[i].len > 0; i++)
	{
		MESSAGE_INFO("Saving %s (%d)\n", SaveStateVars[i].key, SaveStateVars[i].len);
		fwrite(SaveStateVars[i].ptr, SaveStateVars[i].len, 1, fp);
	}

	fclose(fp);

	return 0;
}


/**
 * Cleanup and quit (not used in retro-go)
 */
void
ShutdownPCE()
{
	gfx_term();
	snd_term();
	hard_term();

	exit(0);
}
