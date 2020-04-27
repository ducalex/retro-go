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
#include "debug.h"
#include "utils.h"
#include "romdb.h"

/* Variable section */

struct host_machine host;

uchar *ROM = NULL;
// ROM = the same thing as the ROM file (w/o header)

uint ROM_size, ROM_crc, ROM_idx;
// the number of block of 0x2000 bytes in the rom

const int scanlines_per_frame = 263;
//

const int BaseClock = 7800000;
//

const int IPeriod = 494;
// BaseClock / (scanlines_per_frame * 60);

bool PCERunning = false;
//

int UPeriod = 0;
// Number of frame to skip


/*****************************************************************************

		Function: LoadCard

		Description: load a card
		Parameters: char* name (the filename to load)
		Return: -1 on error else 0

*****************************************************************************/
int
LoadCard(char *name)
{
	MESSAGE_INFO("Opening %s...\n", name);

	if (ROM != NULL) {
		free(ROM);
	}

	FILE *fp = fopen(name, "rb");
	if (fp == NULL)
		return -1;

	// find file size
	fseek(fp, 0, SEEK_END);
	int fsize = ftell(fp);

	// ajust var if header present
	fseek(fp, fsize & 0x1fff, SEEK_SET);
	fsize &= ~0x1fff;

	// read ROM
	ROM = (uchar *)rg_alloc(fsize, MEM_SLOW);
	fread(ROM, 1, fsize, fp);

	fclose(fp);

	ROM_size = fsize / 0x2000;
	ROM_crc = CRC_buffer(ROM, ROM_size * 0x2000);
	ROM_idx = 0xFFFF;

	return 0;
}


void
ResetPCE(bool hard)
{
	// Zero reset RAM and all IO/sound
	if (hard) {
		hard_reset();
	}

	// Reset sprite cache
	memset(&SPR_CACHE, 0, sizeof(SPR_CACHE));

	// Reset IO lines
	io.vdc_status = 0;
	io.vdc_inc = 1;
	io.minline = 0;
	io.maxline = 255;
	io.irq_mask = 0;
	io.psg_volume = 0;
	io.psg_ch = 0;

	/* TEST */
	io.screen_w = 255;
	/* TEST normally 256 */

	/* TEST */
	// io.screen_h = 214;
	/* TEST */

	/* TEST */
	//  io.screen_h = 240;
	/* TEST */

	io.screen_h = 224;

	// Reset sound generator values
	for (int i = 0; i < 6; i++) {
		io.PSG[i][4] = 0x80;
	}

	// Reset memory banking
	bank_set(7, 0x00);
	bank_set(6, 0x05);
	bank_set(5, 0x04);
	bank_set(4, 0x03);
	bank_set(3, 0x02);
	bank_set(2, 0x01);
	bank_set(1, 0xF8);
	bank_set(0, 0xFF);

	// Reset CPU
	reg_a = reg_x = reg_y = 0x00;
	reg_p = FL_TIQ;
	reg_s = 0xFF;
	reg_pc = Read16(VEC_RESET);

	// Counters
	Scanline = 0;
	Cycles = 0;
	TotalCycles = 0;
	PrevTotalCycles = 0;
}


int
InitPCE(char *name)
{
	int i = 0, local_us_encoded_card = 0;

	if (LoadCard(name))
		return 1;

	gfx_init();
	hard_init();

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

	for (int i = 0; i < 0xFF; i++) {
		ROMMapR[i] = TRAPRAM;
		ROMMapW[i] = TRAPRAM;
	}

	// Hu Card with onboard RAM (Populous)
	if (ExtraRAM) {
		ROMMapR[0x40] = ROMMapW[0x40] = ExtraRAM;
		ROMMapR[0x41] = ROMMapW[0x41] = ExtraRAM + 0x2000;
		ROMMapR[0x42] = ROMMapW[0x42] = ExtraRAM + 0x4000;
		ROMMapR[0x43] = ROMMapW[0x43] = ExtraRAM + 0x6000;
	}

	// Backup RAM
	ROMMapR[0xF7] = SaveRAM;
	ROMMapW[0xF7] = SaveRAM;

    // Main RAM
	ROMMapR[0xF8] = RAM;
	ROMMapW[0xF8] = RAM;

	// Supergraphx
	if (SuperRAM) {
		ROMMapR[0xF9] = ROMMapW[0xF9] = SuperRAM;
		ROMMapR[0xFA] = ROMMapW[0xFA] = SuperRAM + 0x2000;
		ROMMapR[0xFB] = ROMMapW[0xFB] = SuperRAM + 0x4000;
	}

    // IO Area
	ROMMapR[0xFF] = IOAREA;
	ROMMapW[0xFF] = IOAREA;

    // Game ROM
    if (ROM && ROM_size) {
        int ROMmask = 1;
        while (ROMmask < ROM_size)
            ROMmask <<= 1;
        ROMmask--;

        MESSAGE_DEBUG("ROMmask=%02X, ROM_size=%02X\n", ROMmask, ROM_size);

        for (int i = 0; i < 0x80; i++) {
            if (ROM_size == 0x30) {
                switch (i & 0x70) {
                case 0x00:
                case 0x10:
                case 0x50:
                    ROMMapR[i] = ROM + (i & ROMmask) * 0x2000;
                    break;
                case 0x20:
                case 0x60:
                    ROMMapR[i] = ROM + ((i - 0x20) & ROMmask) * 0x2000;
                    break;
                case 0x30:
                case 0x70:
                    ROMMapR[i] = ROM + ((i - 0x10) & ROMmask) * 0x2000;
                    break;
                case 0x40:
                    ROMMapR[i] = ROM + ((i - 0x20) & ROMmask) * 0x2000;
                    break;
                }
            } else {
                ROMMapR[i] = ROM + (i & ROMmask) * 0x2000;
            }
        }
    }

	ResetPCE(false);

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

	FILE *fp = fopen(name, "rb");
	if (fp == NULL)
		return -1;

	fread(&PCE, sizeof(PCE), 1, fp);
	fread(&saved_gfx_context, sizeof(saved_gfx_context), 1, fp);
	fclose(fp);

	memset(&SPR_CACHE, 0, sizeof(SPR_CACHE));

	for (int i = 0; i < 8; i++) {
		bank_set(i, MMR[i]);
	}

	return 0;
}


int
SaveState(char *name)
{
	MESSAGE_INFO("Saving state to %s...\n", name);

	FILE *fp = fopen(name, "wb");
	if (fp == NULL)
		return -1;


	fwrite(&PCE, sizeof(PCE), 1, fp);
	fwrite(&saved_gfx_context, sizeof(saved_gfx_context), 1, fp);

	fclose(fp);

	return 0;
}


void
TrashPCE()
{
	// Set volume to zero
	io.psg_volume = 0;

	if (VRAMS) free(VRAMS);
	if (VRAM2) free(VRAM2);
	if (ROM) free(ROM);

	hard_term();
}
