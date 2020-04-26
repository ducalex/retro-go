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
struct hugo_options option;

uchar can_write_debug = 0;

uchar *PopRAM;
// Now dynamicaly allocated
// ( size of popRAMsize bytes )
// If someone could explain me why we need it
// the version I have works well without this trick

const uint32 PopRAMsize = 0x8000;
// I don't really know if it must be 0x8000 or 0x10000

#define ZW			64
//byte ZBuf[ZW*256];
//BOOL IsROM[8];

uchar *ROM = NULL;
// IOAREA = a pointer to the emulated IO zone
// vchange = array of boolean to know whether bg tiles have changed (i.e.
//      vchanges[5]==1 means the 6th tile have changed and VRAM2 should be updated)
//      [to check !]
// vchanges IDEM for sprites
// ROM = the same thing as the ROM file (w/o header)

int ROM_size;
// obvious, no ?
// actually, the number of block of 0x2000 bytes in the rom

extern int32 vmode;
// What is the favorite video mode to use

int32 smode;
// what sound card type should we use? (0 means the silent one,
// my favorite : the fastest!!! ; and -1 means AUTODETECT;
// later will avoid autodetection if wanted)

char silent = 1;
// a bit different from the previous one, even if asked to
// use a card, we could not be able to make sound...

/*
 * nb_joy no more used
 * uchar nb_joy = 1;
 * number of input to poll
 */

int Country;
/* Is this^ initialised anywhere ?
 * You may try to play with if some games don't want to start
 * it could be useful on some cases
 */

int IPeriod;
// Number of cycle between two interruption calls

uint32 TimerCount;
// int CycleOld;
// int TimerPeriod;
int scanlines_per_frame = 263;

//int MinLine = 0,MaxLine = 255;
//#define MAXDISP 227

int BaseClock = 7170000;

char cart_name[PCE_PATH_MAX];
// Name of the file containing the ROM

char short_cart_name[PCE_PATH_MAX];
// Just the filename without the extension (with a dot)
// you just have to add your own extension...

char rom_file_name[PCE_PATH_MAX];
// the name of the file containing the ROM (with path, ext)
// Now needed 'coz of ZIP archiving...

char *server_hostname = NULL;

uchar populus = 0;
// no more hasardous detection
// thanks to the CRC detection
// now, used to know whether the save file
// must contain extra info

uchar US_encoded_card = 0;
// Do we have to swap bit order in the rom

uint16 NO_ROM;
// Number of the ROM in the database or 0xFFFF if unknown

uchar debug_on_beginning = 0;
// Do we have to set a bp on the reset IP

int scroll = 0;

volatile char key_delay = 0;
// delay to avoid too many key strokes

static volatile uchar can_blit = 1;
// used to sync screen to 60 image/sec.

volatile uint32 message_delay = 0;
// if different of zero, we must display the message pointed by pmessage

char exit_message[256];
// What we must display at the end


// const
uchar binbcd[0x100] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
};

// const
uchar bcdbin[0x100] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0, 0, 0, 0,
		0, 0,
	0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0, 0, 0, 0,
		0, 0,
	0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0, 0, 0, 0,
		0, 0,
	0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0, 0, 0, 0,
		0, 0,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0, 0, 0, 0,
		0, 0,
	0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0, 0, 0, 0,
		0, 0,
	0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0, 0, 0, 0,
		0, 0,
	0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0, 0, 0, 0,
		0, 0,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0, 0, 0, 0,
		0, 0,
	0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0, 0, 0, 0,
		0, 0,
};

const char *joymap_reverse[J_MAX] = {
	"UP", "DOWN", "LEFT", "RIGHT",
	"I", "II", "SELECT", "RUN",
	"AUTOI", "AUTOII", "PI", "PII",
	"PSELECT", "PRUN", "PAUTOI", "PAUTOII",
	"PXAXIS", "PYAXIS"
};

uint32 timer_60 = 0;
// how many times do the interrupt have been called

int UPeriod = 0;
// Number of frame to skip

uchar video_driver = 0;
/* 0 => Normal driver, normal display
 * 1 => Eagle graphism
 * 2 => Scanline graphism
 */


uint32
interrupt_60hz(uint32 interval, void *param)
{
	/* Make the system understand it can blit */
	can_blit = 1;

	/* If we've displayed a message recently, make it less recent */
	if (message_delay)
		message_delay--;

	/* number of call of this function */
	timer_60++;

	return interval;
};


uchar
TimerInt()
{
	if (io.timer_start) {
		io.timer_counter--;
		if (io.timer_counter > 128) {
			io.timer_counter = io.timer_reload;

			if (!(io.irq_mask & TIRQ)) {
				io.irq_status |= TIRQ;
				return INT_TIMER;
			}

		}
	}
	return INT_NONE;
}


/*****************************************************************************

		Function: CartLoad

		Description: load a card
		Parameters: char* name (the filename to load)
		Return: -1 on error else 0
		set rom_file_name or builtin_system

*****************************************************************************/
int
CartLoad(char *name)
{
	MESSAGE_INFO("Opening %s...\n", name);
	strcpy(rom_file_name, name);

	if (cart_name != name) {
		// Avoids warning when copying passing cart_name as parameter
		#warning find where this weird call is done
		strcpy(cart_name, name);
	}

	if (ROM != NULL) {
		// ROM was already loaded, likely from memory / zip file
		return 0;
	}

	// Load PCE, we always load a PCE even with a cd (syscard)
	FILE *fp = fopen(rom_file_name, "rb");
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
	int i;

	memset(SPRAM, 0, 64 * 8);

	TimerCount = TimerPeriod;
	hard_reset_io();
	scanline = 0;
	io.vdc_status = 0;
	io.vdc_inc = 1;
	io.minline = 0;
	io.maxline = 255;
	io.irq_mask = 0;
	io.psg_volume = 0;
	io.psg_ch = 0;

	zp_base = RAM;
	sp_base = RAM + 0x100;

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

	for (i = 0; i < 6; i++) {
		io.PSG[i][4] = 0x80;
	}

	mmr[7] = 0x00;
	bank_set(7, 0x00);

	mmr[6] = 0x05;
	bank_set(6, 0x05);

	mmr[5] = 0x04;
	bank_set(5, 0x04);

	mmr[4] = 0x03;
	bank_set(4, 0x03);

	mmr[3] = 0x02;
	bank_set(3, 0x02);

	mmr[2] = 0x01;
	bank_set(2, 0x01);

	mmr[1] = 0xF8;
	bank_set(1, 0xF8);

	mmr[0] = 0xFF;
	bank_set(0, 0xFF);

	reg_a = reg_x = reg_y = 0x00;
	reg_p = FL_TIQ;

	reg_s = 0xFF;

	reg_pc = Op6502(VEC_RESET) + 256 * Op6502(VEC_RESET + 1);

	//CycleNew = 0;
	//CycleOld = 0;

	if (debug_on_beginning) {
		Bp_list[GIVE_HAND_BP].position = reg_pc;
		Bp_list[GIVE_HAND_BP].original_op = Op6502(reg_pc);
		Bp_list[GIVE_HAND_BP].flag = ENABLED;
		Wr6502(reg_pc, 0xB + 0x10 * GIVE_HAND_BP);
	}

	return 0;
}


int
InitPCE(char *name)
{
	int i = 0, ROMmask;
	char *tmp_dummy;
	char local_us_encoded_card = 0;

	option.want_fullscreen_aspect = 0;
	option.want_supergraphx_emulation = 1;

	if (CartLoad(name))
		return 1;

	if (!(tmp_dummy = (char *) (strrchr(cart_name, '/'))))
		tmp_dummy = &cart_name[0];
	else
		tmp_dummy++;

	memset(short_cart_name, 0, 80);
	while ((tmp_dummy[i]) && (tmp_dummy[i] != '.')) {
		short_cart_name[i] = tmp_dummy[i];
		i++;
	}

	if (strlen(short_cart_name))
		if (short_cart_name[strlen(short_cart_name) - 1] != '.') {
			short_cart_name[strlen(short_cart_name) + 1] = 0;
			short_cart_name[strlen(short_cart_name)] = '.';
		}

	// Set the base frequency
	BaseClock = 7800000;

	// Set the interruption period
	IPeriod = BaseClock / (scanlines_per_frame * 60);

	hard_init();

	/* TEST */
	io.screen_h = 224;
	/* TEST */
	io.screen_w = 256;

	uint32 CRC = CRC_buffer(ROM, ROM_size * 0x2000);

	/* I'm doing it only here 'coz cartload set
	   true_file_name       */

	NO_ROM = 0xFFFF;

	int index;
	for (index = 0; index < KNOWN_ROM_COUNT; index++) {
		if (CRC == kKnownRoms[index].CRC)
			NO_ROM = index;
	}

	if (NO_ROM == 0xFFFF)
		printf("ROM not in database: CRC=%lx\n", CRC);

	memset(WRAM, 0, 0x2000);
	WRAM[0] = 0x48;				/* 'H' */
	WRAM[1] = 0x55;				/* 'U' */
	WRAM[2] = 0x42;				/* 'B' */
	WRAM[3] = 0x4D;				/* 'M' */
	WRAM[5] = 0xA0;				/* WRAM[4-5] = 0xA000, end of free mem ? */
	WRAM[6] = 0x10;				/* WRAM[6-7] = 0x8010, beginning of free mem ? */
	WRAM[7] = 0x80;

	local_us_encoded_card = US_encoded_card;

	if ((NO_ROM != 0xFFFF) && (kKnownRoms[NO_ROM].Flags & US_ENCODED))
		local_us_encoded_card = 1;

	if (ROM[0x1FFF] < 0xE0) {
		Log("This rom is probably US encrypted, decrypting...\n");
		local_us_encoded_card = 1;
	}

	if (local_us_encoded_card) {
		uint32 x;
		uchar inverted_nibble[16] = { 0, 8, 4, 12,
			2, 10, 6, 14,
			1, 9, 5, 13,
			3, 11, 7, 15
		};

		for (x = 0; x < ROM_size * 0x2000; x++) {
			uchar temp;

			temp = ROM[x] & 15;

			ROM[x] &= ~0x0F;
			ROM[x] |= inverted_nibble[ROM[x] >> 4];

			ROM[x] &= ~0xF0;
			ROM[x] |= inverted_nibble[temp] << 4;

		}
	}

	// For example with Devil Crush 512Ko
	if ((NO_ROM != 0xFFFF) && (kKnownRoms[NO_ROM].Flags & TWO_PART_ROM))
		ROM_size = 0x30;

	ROMmask = 1;
	while (ROMmask < ROM_size)
		ROMmask <<= 1;
	ROMmask--;

#ifndef FINAL_RELEASE
	fprintf(stderr, "ROMmask=%02X, ROM_size=%02X\n", ROMmask, ROM_size);
#endif

	for (i = 0; i < 0xFF; i++) {
		ROMMapR[i] = TRAPRAM;
		ROMMapW[i] = TRAPRAM;
	}

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

	if (NO_ROM != 0xFFFF) {
		MESSAGE_INFO("Rom Name: %s\n",
			(kKnownRoms[NO_ROM].Name) ? kKnownRoms[NO_ROM].Name : "Unknown");
		MESSAGE_INFO("Publisher: %s\n",
			(kKnownRoms[NO_ROM].Publisher) ? kKnownRoms[NO_ROM].Publisher : "Unknown");
	} else {
		MESSAGE_ERROR("Unknown ROM\n");
	}

	if ((NO_ROM != 0xFFFF) && (kKnownRoms[NO_ROM].Flags & POPULOUS)) {
		populus = 1;

		MESSAGE_INFO("Special Rom: Populous detected!\n");
		if (!(PopRAM = (uchar*)malloc(PopRAMsize)))
			perror("Populous: Not enough memory!");

		ROMMapW[0x40] = PopRAM;
		ROMMapW[0x41] = PopRAM + 0x2000;
		ROMMapW[0x42] = PopRAM + 0x4000;
		ROMMapW[0x43] = PopRAM + 0x6000;

		ROMMapR[0x40] = PopRAM;
		ROMMapR[0x41] = PopRAM + 0x2000;
		ROMMapR[0x42] = PopRAM + 0x4000;
		ROMMapR[0x43] = PopRAM + 0x6000;

	} else {
		populus = 0;
		PopRAM = NULL;
	}

	ROMMapR[0xF7] = WRAM;
	ROMMapW[0xF7] = WRAM;

	ROMMapR[0xF8] = RAM;
	ROMMapW[0xF8] = RAM;

	if (option.want_supergraphx_emulation) {
		ROMMapW[0xF9] = RAM + 0x2000;
		ROMMapW[0xFA] = RAM + 0x4000;
		ROMMapW[0xFB] = RAM + 0x6000;

		ROMMapR[0xF9] = RAM + 0x2000;
		ROMMapR[0xFA] = RAM + 0x4000;
		ROMMapR[0xFB] = RAM + 0x6000;
	}

	/*
	   #warning REMOVE ME
	   // ROMMapR[0xFC] = RAM + 0x6000;
	   ROMMapW[0xFC] = NULL;
	 */

	ROMMapR[0xFF] = IOAREA;
	ROMMapW[0xFF] = IOAREA;

	{
		// char backupmem[PCE_PATH_MAX];
		// snprintf(backupmem, PCE_PATH_MAX, "%s/backupmem.bin", config_basepath);

		// FILE *fp;
		// fp = fopen(backupmem, "rb");

		// if (fp == NULL)
		// 	fprintf(stderr, "Can't open %s\n", backupmem);
		// else {
		// 	fread(WRAM, 0x2000, 1, fp);
		// 	fclose(fp);
		// }

	}

	if ((NO_ROM != 0xFFFF) && (kKnownRoms[NO_ROM].Flags & CD_SYSTEM)) {
		uint16 offset = 0;
		uchar new_val = 0;

		switch(kKnownRoms[NO_ROM].CRC) {
			case 0X3F9F95A4:
				// CD-ROM SYSTEM VER. 1.00
				offset = 56254;
				new_val = 17;
				break;
			case 0X52520BC6:
			case 0X283B74E0:
				// CD-ROM SYSTEM VER. 2.00
				// CD-ROM SYSTEM VER. 2.10
				offset = 51356;
				new_val = 128;
				break;
			case 0XDD35451D:
			case 0XE6F16616:
				// CD ROM 2 SYSTEM 3.0
				// SUPER CD-ROM2 SYSTEM VER. 3.00
				// SUPER CD-ROM2 SYSTEM VER. 3.00
				offset = 51401;
				new_val = 128;
				break;
		}

		if (offset > 0)
			ROMMapW[0xE1][offset & 0x1fff] = new_val;
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

void
TrashPCE()
{
	FILE *fp;
	char *tmp_buf = (char *) alloca(256);

	// char backupmem[PCE_PATH_MAX];
	// snprintf(backupmem, PCE_PATH_MAX, "%s/backupmem.bin", config_basepath);

	// Save the backup ram into file
	// if (!(fp = fopen(backupmem, "wb"))) {
	// 	memset(WRAM, 0, 0x2000);
	// 	MESSAGE_ERROR("Can't open %s for saving RAM\n", backupmem);
	// } else {
	// 	fwrite(WRAM, 0x2000, 1, fp);
	// 	fclose(fp);
	// 	MESSAGE_INFO("%s used for saving RAM\n", backupmem);
	// }

	// Set volume to zero
	io.psg_volume = 0;

	if (IOAREA)
		free(IOAREA);

	if (ROM)
		free(ROM);

	if (PopRAM)
		free(PopRAM);

	hard_term();

	return;
}
