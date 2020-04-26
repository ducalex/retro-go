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

int ROM_size;
// the number of block of 0x2000 bytes in the rom

int IPeriod;
// Number of cycle between two interruption calls

int scanlines_per_frame = 263;

int BaseClock = 7170000;

char cart_name[PCE_PATH_MAX];
// Name of the file containing the ROM

char *server_hostname = NULL;

uint16 NO_ROM;
// Number of the ROM in the database or 0xFFFF if unknown

int scroll = 0;

bool PCERunning = false;

const char *joymap_reverse[J_MAX] = {
	"UP", "DOWN", "LEFT", "RIGHT",
	"I", "II", "SELECT", "RUN",
	"AUTOI", "AUTOII", "PI", "PII",
	"PSELECT", "PRUN", "PAUTOI", "PAUTOII",
	"PXAXIS", "PYAXIS"
};

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
	strcpy(cart_name, name);

	if (ROM != NULL) {
		free(ROM);
	}

	// Load PCE, we always load a PCE even with a cd (syscard)
	FILE *fp = fopen(name, "rb");
	// find file size
	fseek(fp, 0, SEEK_END);
	int fsize = ftell(fp);

	// ajust var if header present
	fseek(fp, fsize & 0x1fff, SEEK_SET);
	fsize &= ~0x1fff;

	// read ROM
	ROM = (uchar *)rg_alloc(fsize, MEM_SLOW);
	ROM_size = fsize / 0x2000;
	fread(ROM, 1, fsize, fp);

	fclose(fp);

	return 0;
}


int
ResetPCE()
{
	hard_reset();

	scanline = 0;
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

	for (int i = 0; i < 6; i++) {
		io.PSG[i][4] = 0x80;
	}

	bank_set(7, 0x00);
	bank_set(6, 0x05);
	bank_set(5, 0x04);
	bank_set(4, 0x03);
	bank_set(3, 0x02);
	bank_set(2, 0x01);
	bank_set(1, 0xF8);
	bank_set(0, 0xFF);

	reg_a = reg_x = reg_y = 0x00;
	reg_p = FL_TIQ;

	reg_s = 0xFF;

	reg_pc = Read8(VEC_RESET) + 256 * Read8(VEC_RESET + 1);

	return 0;
}


int
InitPCE(char *name)
{
	int i = 0, local_us_encoded_card = 0, ROMmask;

	if (LoadCard(name))
		return 1;

	// Set the base frequency
	BaseClock = 7800000;

	// Set the interruption period
	IPeriod = BaseClock / (scanlines_per_frame * 60);

	hard_init();

	/* TEST */
	io.screen_h = 224;
	/* TEST */
	io.screen_w = 256;

	for (i = 0; i < 0xFF; i++) {
		ROMMapR[i] = TRAPRAM;
		ROMMapW[i] = TRAPRAM;
	}

	// Backup RAM
	ROMMapR[0xF7] = SaveRAM;
	ROMMapW[0xF7] = SaveRAM;

	ROMMapR[0xF8] = RAM;
	ROMMapW[0xF8] = RAM;

	// supergraphx
	if (SuperRAM) {
		ROMMapR[0xF9] = ROMMapW[0xF9] = SuperRAM;
		ROMMapR[0xFA] = ROMMapW[0xFA] = SuperRAM + 0x2000;
		ROMMapR[0xFB] = ROMMapW[0xFB] = SuperRAM + 0x4000;
	}

	ROMMapR[0xFF] = IOAREA;
	ROMMapW[0xFF] = IOAREA;


	uint32 CRC = CRC_buffer(ROM, ROM_size * 0x2000);

	NO_ROM = 0xFFFF;

	for (int index = 0; index < KNOWN_ROM_COUNT; index++) {
		if (CRC == kKnownRoms[index].CRC) {
			NO_ROM = index;
			break;
		}
	}

	if (NO_ROM == 0xFFFF)
	{
		MESSAGE_INFO("ROM not in database: CRC=%lx\n", CRC);
	}
	else
	{
		MESSAGE_INFO("Rom Name: %s\n", kKnownRoms[NO_ROM].Name);
		MESSAGE_INFO("Publisher: %s\n", kKnownRoms[NO_ROM].Publisher);

		// US Encrypted
		if ((kKnownRoms[NO_ROM].Flags & US_ENCODED) || ROM[0x1FFF] < 0xE0)
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
		if (kKnownRoms[NO_ROM].Flags & TWO_PART_ROM)
			ROM_size = 0x30;

		// Hu Card with onboard RAM
		if (kKnownRoms[NO_ROM].Flags & POPULOUS)
		{
			MESSAGE_INFO("Special Rom: Populous detected!\n");
			if (!ExtraRAM)
				ExtraRAM = (uchar*)rg_alloc(0x8000, MEM_FAST);

			ROMMapR[0x40] = ROMMapW[0x40] = ExtraRAM;
			ROMMapR[0x41] = ROMMapW[0x41] = ExtraRAM + 0x2000;
			ROMMapR[0x42] = ROMMapW[0x42] = ExtraRAM + 0x4000;
			ROMMapR[0x43] = ROMMapW[0x43] = ExtraRAM + 0x6000;
		}
	}


	ROMmask = 1;
	while (ROMmask < ROM_size)
		ROMmask <<= 1;
	ROMmask--;

	MESSAGE_DEBUG("ROMmask=%02X, ROM_size=%02X\n", ROMmask, ROM_size);

	for (i = 0; i < 0x80; i++) {
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

	return 0;
}


int
RunPCE(void)
{
	if (!ResetPCE())
		exe_go();
	return 1;
}


int
LoadState(char *name)
{
	MESSAGE_INFO("Loading state from %s...\n", name);

	FILE *fp = fopen(name, "rb");
	fclose(fp);

	return 0;
}


int
SaveState(char *name)
{
	MESSAGE_INFO("Saving state to %s...\n", name);

	FILE *fp = fopen(name, "wb");
	fclose(fp);

	return 0;
}


void
TrashPCE()
{
	// Set volume to zero
	io.psg_volume = 0;

	if (ROM)
		free(ROM);

	if (ExtraRAM)
		free(ExtraRAM);

	hard_term();

	return;
}
