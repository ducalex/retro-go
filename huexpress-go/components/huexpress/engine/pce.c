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
#include "iso_ent.h"
#include "zipmgr.h"
#include "utils.h"
#if defined(BSD_CD_HARDWARE_SUPPORT)
#include "pcecd.h"
#endif

#include "romdb.h"

#define LOG_NAME "huexpress.log"

#define CD_FRAMES 75
#define CD_SECS 60

/* Variable section */

struct host_machine host;
struct hugo_options option;

uchar minimum_bios_hooking = 0;

uchar can_write_debug = 0;

uchar *cd_buf = NULL;

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

uchar CDBIOS_replace[0x4d][2];
// Used to know what byte do we have replaced to hook bios functions so that
// we can restore them if needed

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

char cart_name[PCE_PATH_MAX] = "";
// Name of the file containing the ROM

char short_cart_name[PCE_PATH_MAX];
// Just the filename without the extension (with a dot)
// you just have to add your own extension...

char short_iso_name[PCE_PATH_MAX];
// Just the ISO filename without the extension (with a dot)
// you just have to add your own extension...

uchar hook_start_cd_system = 0;
// Do we hook CD system to avoid pressing start on main screen

uchar use_eagle = 0;
// eagle use ?

uchar use_scanline = 0;
// use scanline mode ?

char rom_file_name[PCE_PATH_MAX];
// the name of the file containing the ROM (with path, ext)
// Now needed 'coz of ZIP archiving...

char config_basepath[PCE_PATH_MAX];
// base path for all configs (in users home)

char sav_path[PCE_PATH_MAX];
// The filename for saving games

char sav_basepath[PCE_PATH_MAX];
// base path for saved games

char tmp_basepath[PCE_PATH_MAX];
// base path for temporary operations

char video_path[PCE_PATH_MAX];
// The place where to keep output pictures

char ISO_filename[PCE_PATH_MAX] = "";
// The name of the ISO file

uchar force_header = 1;
// Force the first sector of the code track to be the correct header

char *server_hostname = NULL;

char effectively_played = 0;
// Well, the name is enough I think...

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

uchar CD_emulation = 0;
// Do we emulate CD ( == 1)
//                      or  ISO file     ( == 2)
//                      or  ISQ file     ( == 3)
//                      or  plain BIN file ( == 4)
//                      or  HCD ( == 5)

uchar builtin_system_used = 0;
// Have we used the .dat included rom or no ?

int scroll = 0;

signed char snd_vol[6][32];
// cooked volume for each channel

Track CD_track[0x100];
// Track
// beg_min -> beginning in minutes since the begin of the CD(BCD)
// beg_sec -> beginning in seconds since the begin of the CD(BCD)
// beg_fr -> beginning in frames     since the begin of the CD(BCD)
// type -> 0 = audio, 4 = data
// beg_lsn -> beginning in number of sector (2048 bytes)
// length -> number of sector

volatile char key_delay = 0;
// delay to avoid too many key strokes

static volatile uchar can_blit = 1;
// used to sync screen to 60 image/sec.

volatile uint32 message_delay = 0;
// if different of zero, we must display the message pointed by pmessage

char exit_message[256] = "";
// What we must display at the end

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

uchar pce_cd_adpcm_trans_done = 0;

FILE *iso_FILE = NULL;

const char *joymap_reverse[J_MAX] = {
	"UP", "DOWN", "LEFT", "RIGHT",
	"I", "II", "SELECT", "RUN",
	"AUTOI", "AUTOII", "PI", "PII",
	"PSELECT", "PRUN", "PAUTOI", "PAUTOII",
	"PXAXIS", "PYAXIS"
};

uint32 packed_iso_filesize = 0;

uint32 ISQ_position = 0;

// struct cdrom_tocentry pce_cd_tocentry;

uchar nb_max_track = 24;		//(NO MORE BCD!!!!!)

//extern char    *pCartName;

//extern char snd_bSound;

uint32 timer_60 = 0;
// how many times do the interrupt have been called

int UPeriod = 0;
// Number of frame to skip

uchar video_driver = 0;
/* 0 => Normal driver, normal display
 * 1 => Eagle graphism
 * 2 => Scanline graphism
 */

// Pre declaration of reading function routines
void read_sector_dummy(uchar *, uint32);
void read_sector_CD(uchar *, uint32);
void read_sector_ISO(uchar *, uint32);
void read_sector_ISQ(uchar *, uint32);
void read_sector_BIN(uchar *, uint32);
void read_sector_HCD(uchar *, uint32);

void (*read_sector_method[6]) (uchar *, uint32) = {
read_sector_dummy,
		read_sector_CD,
		read_sector_ISO, read_sector_ISQ, read_sector_BIN,
		read_sector_HCD};

static char *
check_char(char *s, char c)
{
	while ((*s) && (*s != c))
		s++;

	return *s == c ? s : NULL;
}


uint32
interrupt_60hz(uint32 interval, void *param)
{
	/* Refresh freezed values in RAM */
	for (can_blit = 0; can_blit < current_freezed_values; can_blit++)
		RAM[list_to_freeze[can_blit].position] =
			list_to_freeze[can_blit].value;

	/* Make the system understand it can blit */
	can_blit = 1;

	/* If we've displayed a message recently, make it less recent */
	if (message_delay)
		message_delay--;

	/* number of call of this function */
	timer_60++;

	return interval;
};

/*****************************************************************************

		Function: init_log_file

		Description: destroy the current log file, and create another
		Parameters: none
		Return: nothin

*****************************************************************************/
void
init_log_file()
{
	unlink(log_filename);
	Log("Creating log file %s\n", __DATE__);
}


extern int op6502_nb;


void
fill_cd_info()
{
	uchar Min, Sec, Fra;
	uchar current_track;

	// Track 1 is almost always a audio avertising track
	// 30 sec. seems usual

	CD_track[1].beg_min = binbcd[00];
	CD_track[1].beg_sec = binbcd[02];
	CD_track[1].beg_fra = binbcd[00];

	CD_track[1].type = 0;
	CD_track[1].beg_lsn = 0;	// Number of sector since the
	// beginning of track 1

	CD_track[1].length = 47 * CD_FRAMES + 65;	// Most common

	nb_sect2msf(CD_track[1].length, &Min, &Sec, &Fra);

	// Second track is the main code track

	CD_track[2].beg_min = binbcd[bcdbin[CD_track[1].beg_min] + Min];
	CD_track[2].beg_sec = binbcd[bcdbin[CD_track[1].beg_sec] + Sec];
	CD_track[2].beg_fra = binbcd[bcdbin[CD_track[1].beg_fra] + Fra];

	CD_track[2].type = 4;
	CD_track[2].beg_lsn
		=
		msf2nb_sect(bcdbin[CD_track[2].beg_min] -
					bcdbin[CD_track[1].beg_min],
					bcdbin[CD_track[2].beg_sec] -
					bcdbin[CD_track[1].beg_sec],
					bcdbin[CD_track[2].beg_fra] -
					bcdbin[CD_track[1].beg_fra]);

	switch (CD_emulation) {
	case 2:
		CD_track[0x02].length = filesize(iso_FILE) / 2048;
		break;
	case 3:
		CD_track[0x02].length = packed_iso_filesize / 2048;
		break;
	case 4:
		CD_track[0x02].length = 140000;
		break;
	default:
		break;
	}

	// Now most track are audio

	for (current_track = 3; current_track < bcdbin[nb_max_track];
		 current_track++) {

		Fra = CD_track[current_track - 1].length % CD_FRAMES;
		Sec = (CD_track[current_track - 1].length / CD_FRAMES) % CD_SECS;
		Min = (CD_track[current_track - 1].length / CD_FRAMES) / CD_SECS;

		CD_track[current_track].beg_min
			= binbcd[bcdbin[CD_track[current_track - 1].beg_min] + Min];
		CD_track[current_track].beg_sec
			= binbcd[bcdbin[CD_track[current_track - 1].beg_sec] + Sec];
		CD_track[current_track].beg_fra
			= binbcd[bcdbin[CD_track[current_track - 1].beg_fra] + Fra];

		CD_track[current_track].type = 0;
		CD_track[current_track].beg_lsn
			= msf2nb_sect(bcdbin[CD_track[current_track].beg_min]
						  - bcdbin[CD_track[1].beg_min],
						  bcdbin[CD_track[current_track].beg_sec]
						  - bcdbin[CD_track[1].beg_sec],
						  bcdbin[CD_track[current_track].beg_fra]
						  - bcdbin[CD_track[1].beg_fra]);
		// 1 min for all
		CD_track[current_track].length = 1 * CD_SECS * CD_FRAMES;
	}

	// And the last one is generally also code

	Fra = CD_track[nb_max_track - 1].length % CD_FRAMES;
	Sec = (CD_track[nb_max_track - 1].length / CD_FRAMES) % CD_SECS;
	Min = (CD_track[nb_max_track - 1].length / CD_FRAMES) / CD_SECS;

	CD_track[nb_max_track].beg_min
		= binbcd[bcdbin[CD_track[nb_max_track - 1].beg_min] + Min];
	CD_track[nb_max_track].beg_sec
		= binbcd[bcdbin[CD_track[nb_max_track - 1].beg_sec] + Sec];
	CD_track[nb_max_track].beg_fra
		= binbcd[bcdbin[CD_track[nb_max_track - 1].beg_fra] + Fra];

	CD_track[nb_max_track].type = 4;
	CD_track[nb_max_track].beg_lsn
		= msf2nb_sect(bcdbin[CD_track[nb_max_track].beg_min]
					  - bcdbin[CD_track[1].beg_min],
					  bcdbin[CD_track[nb_max_track].beg_sec]
					  - bcdbin[CD_track[1].beg_sec],
					  bcdbin[CD_track[nb_max_track].beg_fra]
					  - bcdbin[CD_track[1].beg_fra]);

	/* Thank to Nyef for having localised a little bug there */
	switch (CD_emulation) {
	case 2:
		CD_track[nb_max_track].length = filesize(iso_FILE) / 2048;
		break;
	case 3:
		CD_track[nb_max_track].length = packed_iso_filesize / 2048;
		break;
	case 4:
		CD_track[nb_max_track].length = 14000;
		break;
	default:
		break;
	}
}


void
read_sector_BIN(uchar * p, uint32 sector)
{
	static int first_read = 1;
	static long second_track_sector = 0;
	int result;


	if (first_read) {
		uchar found = 0, dummy;
		int index_in_header = 0;
		unsigned long position;

		fseek(iso_FILE, 0, SEEK_SET);

		while ((!found) && (!feof(iso_FILE))) {
			dummy = getc(iso_FILE);
			if (dummy == ISO_header[0]) {
				position = ftell(iso_FILE);
				index_in_header = 1;
				while ((index_in_header < 0x800)
					   && (getc(iso_FILE) ==
						   ISO_header[index_in_header++]));

				if (index_in_header == 0x800) {
					found = 1;
					second_track_sector = ftell(iso_FILE) - 0x800;
				}

				fseek(iso_FILE, position, SEEK_SET);
			}
		}

		first_read = 0;

	}

	for (result = bcdbin[nb_max_track]; result > 0x01; result--) {
		if ((sector >= CD_track[binbcd[result]].beg_lsn)
			&& (sector <= CD_track[binbcd[result]].beg_lsn
				+ CD_track[binbcd[result]].length))
			break;
	}

	if (result != 0x02) {
		MESSAGE_ERROR("Read on non-track 2\n");
		TRACE("Track %d asked, sector number: 0x%X\n",
			  result, pce_cd_sectoraddy);
		exit(-10);
	}
#if ENABLE_TRACING
	TRACE("BIN: Loading sector number %d\n", pce_cd_sectoraddy);
#endif

	fseek(iso_FILE, second_track_sector
		  + (sector - CD_track[binbcd[result]].beg_lsn) * 2352, SEEK_SET);
	fread(p, 2048, 1, iso_FILE);

}


void
read_sector_ISQ(uchar * p, uint32 sector)
{
	// Only for allegro?
}


#define CD_BUF_LENGTH 8
uint32 first_sector = 0;


void
read_sector_CD(uchar * p, uint32 sector)
{
	int i;
#if ENABLE_TRACING_CD
	TRACE("CDRom2: Reading sector %d\n", sector);
#endif

	if (cd_buf != NULL) {
		if ((sector >= first_sector)
			&& (sector <= first_sector + CD_BUF_LENGTH - 1)) {
			memcpy(p, cd_buf + 2048 * (sector - first_sector), 2048);
			return;
		} else {
			for (i = 0; i < CD_BUF_LENGTH; i++)
				osd_cd_read(cd_buf + 2048 * i, sector + i);
			first_sector = sector;
			memcpy(p, cd_buf, 2048);
		}
	} else {
		cd_buf = (uchar *) malloc(CD_BUF_LENGTH * 2048);
		for (i = 0; i < CD_BUF_LENGTH; i++)
			osd_cd_read(cd_buf + 2048 * i, sector + i);
		first_sector = sector;
		memcpy(p, cd_buf, 2048);
	}

}


void
read_sector_ISO(uchar * p, uint32 sector)
{
	int result;

	for (result = nb_max_track; result > 0x01; result--) {
		if ((sector >= CD_track[result].beg_lsn)
			&& (sector <=
				CD_track[result].beg_lsn + CD_track[result].length))
			break;
	}

#if ENABLE_TRACING_CD
	TRACE("CDRom2: Loading ISO sector %d...\n"
		  "           AX=%02x%02x; BX=%02x%02x; CX=%02x%02x; DX=%02x%02x\n"
		  "           Track #%d begins at %d\n",
		  pce_cd_sectoraddy,
		  RAM[0xf9], RAM[0xf8], RAM[0xfb], RAM[0xfa],
		  RAM[0xfd], RAM[0xfc], RAM[0xff], RAM[0xfe],
		  result, CD_track[result].beg_lsn);
	/*
	   Log
	   ("Loading sector n�%d.\nAX=%02x%02x\nBX=%02x%02x\nCX=%02x%02x\nDX=%02x%02x\n\n",
	   pce_cd_sectoraddy, RAM[0xf9], RAM[0xf8], RAM[0xfb], RAM[0xfa], RAM[0xfd],
	   RAM[0xfc], RAM[0xff], RAM[0xfe]);
	   Log("temp+2-5 = %x %x %x\ntemp + 1 = %02x\n",RAM[5], RAM[6], RAM[7], RAM[4]);
	   Log("ISO : seek at %d\n", (sector - CD_track[result].beg_lsn) * 2048);
	   Log("Track n�%d begin at %d\n", result, CD_track[result].beg_lsn);
	 */
#endif

	if (result != 0x02) {
		int i;
		MESSAGE_ERROR("Read on non-track 2\n");
		TRACE("Track %d asked\nsector : 0x%X", result, pce_cd_sectoraddy);

		/* exit(-10);
		 * Don't quit anymore but fill the reading buffer with garbage
		 * easily recognizable
		 */

		for (i = 0; i < 2048; i += 4)
			*(uint32 *) & p[i] = 0xDEADBEEF;
		return;
	}

	if (sector == CD_track[result].beg_lsn) {
		/* We're reading the first sector, the header */
		if (force_header) {
			memcpy(p, ISO_header, 0x800);
			return;
		}
	}

	fseek(iso_FILE, (sector - CD_track[result].beg_lsn) * 2048, SEEK_SET);
	fread(p, 2048, 1, iso_FILE);

}


void
read_sector_dummy(uchar * p, uint32 sector)
{
	return;
}


void
pce_cd_read_sector(void)
{
	// Reduce volume to avoid sound stutter during CD access
	osd_snd_set_volume(0);

#if ENABLE_TRACING_CD
	TRACE("CDRom2: %s reading sector %d (via CDEmulation mode %d)\n",
		  __func__, pce_cd_sectoraddy, CD_emulation);
#endif

	(*read_sector_method[CD_emulation]) (cd_sector_buffer,
										 pce_cd_sectoraddy);

	pce_cd_sectoraddy++;

#if 0
	for (result = 0; result < 2048; result++) {
		if ((result & 15) == 0) {
			fprintf(stderr, "%03x: ", result);
		}
		fprintf(stderr, "%02x", cd_sector_buffer[result]);
		if ((result & 15) == 15) {
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, " ");
		}
	}
#endif

#ifndef FINAL_RELEASE
#if 0
	{

		FILE *g = fopen("read.cd", "at");
		int result;

		fprintf(g, "sector #%x\n", pce_cd_sectoraddy - 1);

		for (result = 0; result < 2048; result++) {
			if ((result & 15) == 0) {
				fprintf(g, "%03x: ", result);
			}
			fprintf(g, "%02x", cd_sector_buffer[result]);
			if ((result & 15) == 15) {
				fprintf(g, "\n");
			} else {
				fprintf(g, " ");
			}
		}

		fclose(g);

	}
#endif
#endif

	pce_cd_read_datacnt = 2048;
	cd_read_buffer = cd_sector_buffer;

	// raise volume back to original value
	osd_snd_set_volume(gen_vol);
}


void
issue_ADPCM_dma(void)
{
#ifndef FINAL_RELEASE
	fprintf(stderr, "Will make DMA transfer\n");
	Log("ADPCM DMA will begin\n");
#endif

	while (cd_sectorcnt--) {
		memcpy(PCM + io.adpcm_dmaptr, cd_read_buffer, pce_cd_read_datacnt);
		cd_read_buffer = NULL;
		io.adpcm_dmaptr += pce_cd_read_datacnt;
		pce_cd_read_datacnt = 0;
		pce_cd_read_sector();
	}

	pce_cd_read_datacnt = 0;
	pce_cd_adpcm_trans_done = 1;
	cd_read_buffer = NULL;
}

/*
 *	convert logical_block_address to m-s-f_number (3 bytes only)
 *	lifted from the cdrom test program in the Linux kernel docs.
 *	hacked up to convert to BCD.
 */
#ifndef LINUX
#define CD_MSF_OFFSET 150
#endif

void
lba2msf(int lba, uchar * msf)
{
	lba += CD_MSF_OFFSET;
	msf[0] = binbcd[lba / (CD_SECS * CD_FRAMES)];
	lba %= CD_SECS * CD_FRAMES;
	msf[1] = binbcd[lba / CD_FRAMES];
	msf[2] = binbcd[lba % CD_FRAMES];
}


uint32
msf2nb_sect(uchar min, uchar sec, uchar frm)
{
	uint32 result = frm;
	result += sec * CD_FRAMES;
	result += min * CD_FRAMES * CD_SECS;
	return result;
}


void
nb_sect2msf(uint32 lsn, uchar * min, uchar * sec, uchar * frm)
{

	(*frm) = lsn % CD_FRAMES;
	lsn /= CD_FRAMES;
	(*sec) = lsn % CD_SECS;
	(*min) = lsn / CD_SECS;

	return;
}


IRAM_ATTR void
IO_write(uint16 A, uchar V)
{
    //printf("w%04x,%02x ",A&0x3FFF,V);

    if ((A >= 0x800) && (A < 0x1800))   // We keep the io buffer value
        io.io_buffer = V;

#ifndef FINAL_RELEASE
    if ((A & 0x1F00) == 0x1A00)
        Log("AC Write %02x at %04x\n", V, A);
#endif

    switch (A & 0x1F00) {
    case 0x0000:                /* VDC */
        switch (A & 3) {
        case 0:
            IO_VDC_active_set(V & 31)
            return;
        case 1:
            return;
        case 2:
            //printf("vdc_l%d,%02x ",io.vdc_reg,V);
            switch (io.vdc_reg) {
            case VWR:           /* Write to video */
                io.vdc_ratch = V;
                return;
            case HDR:           /* Horizontal Definition */
                {
                    typeof(io.screen_w) old_value = io.screen_w;
                    io.screen_w = (V + 1) * 8;

                    if (io.screen_w == old_value)
                        break;

                    // (*init_normal_mode[video_driver]) ();
                    gfx_need_video_mode_change = 1;
                    {
                        uint32 x, y =
                            (WIDTH - io.screen_w) / 2 - 512 * WIDTH;
                        for (x = 0; x < 1024; x++) {
                            spr_init_pos[x] = y;
                            y += WIDTH;
                        }
                    }
                }
                break;

            case MWR:           /* size of the virtual background screen */
                {
                    static uchar bgw[] = { 32, 64, 128, 128 };
                    io.bg_h = (V & 0x40) ? 64 : 32;
                    io.bg_w = bgw[(V >> 4) & 3];
                }
                break;

            case BYR:           /* Vertical screen offset */
                /*
                   if (IO_VDC_08_BYR.B.l == V)
                   return;
                 */

                save_gfx_context(0);

                if (!scroll) {
                    oldScrollX = ScrollX;
                    oldScrollY = ScrollY;
                    oldScrollYDiff = ScrollYDiff;
                }
                IO_VDC_08_BYR.B.l = V;
                scroll = 1;
                ScrollYDiff = scanline - 1;
                ScrollYDiff -= IO_VDC_0C_VPR.B.h + IO_VDC_0C_VPR.B.l;

#if ENABLE_TRACING_DEEP_GFX
                TRACE("ScrollY = %d (l), ", ScrollY);
#endif
                return;
            case BXR:           /* Horizontal screen offset */

                /*
                   if (IO_VDC_07_BXR.B.l == V)
                   return;
                 */

                save_gfx_context(0);

                if (!scroll) {
                    oldScrollX = ScrollX;
                    oldScrollY = ScrollY;
                    oldScrollYDiff = ScrollYDiff;
                }
                IO_VDC_07_BXR.B.l = V;
                scroll = 1;
                return;

            case CR:
                if (IO_VDC_active.B.l == V)
                    return;
                save_gfx_context(0);
                IO_VDC_active.B.l = V;
                return;

            case VCR:
                IO_VDC_active.B.l = V;
                gfx_need_video_mode_change = 1;
                return;

            case HSR:
                IO_VDC_active.B.l = V;
                gfx_need_video_mode_change = 1;
                return;

            case VPR:
                IO_VDC_active.B.l = V;
                gfx_need_video_mode_change = 1;
                return;

            case VDW:
                IO_VDC_active.B.l = V;
                gfx_need_video_mode_change = 1;
                return;
            }

            IO_VDC_active.B.l = V;
            // all others reg just need to get the value, without additional stuff

#if ENABLE_TRACING_DEEP_GFX
            TRACE("VDC[%02x]=0x%02x, ", io.vdc_reg, V);
#endif

#ifndef FINAL_RELEASE
            if (io.vdc_reg > 19) {
                fprintf(stderr, "ignore write lo vdc%d,%02x\n", io.vdc_reg,
                        V);
            }
#endif

            return;
        case 3:
            switch (io.vdc_reg) {
            case VWR:           /* Write to mem */
                /* Writing to hi byte actually perform the action */
                VRAM[IO_VDC_00_MAWR.W * 2] = io.vdc_ratch;
                VRAM[IO_VDC_00_MAWR.W * 2 + 1] = V;

                vchange[IO_VDC_00_MAWR.W / 16] = 1;
                vchanges[IO_VDC_00_MAWR.W / 64] = 1;

                IO_VDC_00_MAWR.W += io.vdc_inc;

                /* vdc_ratch shouldn't be reset between writes */
                // io.vdc_ratch = 0;
                return;

            case VCR:
                IO_VDC_active.B.h = V;
                gfx_need_video_mode_change = 1;
                return;

            case HSR:
                IO_VDC_active.B.h = V;
                gfx_need_video_mode_change = 1;
                return;

            case VPR:
                IO_VDC_active.B.h = V;
                gfx_need_video_mode_change = 1;
                return;

            case VDW:           /* screen height */
                IO_VDC_active.B.h = V;
                gfx_need_video_mode_change = 1;
                return;

            case LENR:          /* DMA transfert */

                IO_VDC_12_LENR.B.h = V;

                {               // black-- 's code

                    int sourcecount = (IO_VDC_0F_DCR.W & 8) ? -1 : 1;
                    int destcount = (IO_VDC_0F_DCR.W & 4) ? -1 : 1;

                    int source = IO_VDC_10_SOUR.W * 2;
                    int dest = IO_VDC_11_DISTR.W * 2;

                    int i;

                    for (i = 0; i < (IO_VDC_12_LENR.W + 1) * 2; i++) {
                        *(VRAM + dest) = *(VRAM + source);
                        dest += destcount;
                        source += sourcecount;
                    }

                    /*
                       IO_VDC_10_SOUR.W = source;
                       IO_VDC_11_DISTR.W = dest;
                     */
                    // Erich Kitzmuller fix follows
                    IO_VDC_10_SOUR.W = source / 2;
                    IO_VDC_11_DISTR.W = dest / 2;

                }

                IO_VDC_12_LENR.W = 0xFFFF;

                memset(vchange, 1, VRAMSIZE / 32);
                memset(vchanges, 1, VRAMSIZE / 128);


                /* TODO: check whether this flag can be ignored */
                io.vdc_status |= VDC_DMAfinish;

                return;

            case CR:            /* Auto increment size */
                {
                    static uchar incsize[] = { 1, 32, 64, 128 };
                    /*
                       if (IO_VDC_05_CR.B.h == V)
                       return;
                     */
                    save_gfx_context(0);

                    io.vdc_inc = incsize[(V >> 3) & 3];
                    IO_VDC_05_CR.B.h = V;
                }
                break;
            case HDR:           /* Horizontal display end */
                /* TODO : well, maybe we should implement it */
                //io.screen_w = (io.VDC_ratch[HDR]+1)*8;
                //TRACE0("HDRh\n");
#if ENABLE_TRACING_DEEP_GFX
                TRACE("VDC[HDR].h = %d, ", V);
#endif
                break;

            case BYR:           /* Vertical screen offset */

                /*
                   if (IO_VDC_08_BYR.B.h == (V & 1))
                   return;
                 */

                save_gfx_context(0);

                if (!scroll) {
                    oldScrollX = ScrollX;
                    oldScrollY = ScrollY;
                    oldScrollYDiff = ScrollYDiff;
                }
                IO_VDC_08_BYR.B.h = V & 1;
                scroll = 1;
                ScrollYDiff = scanline - 1;
                ScrollYDiff -= IO_VDC_0C_VPR.B.h + IO_VDC_0C_VPR.B.l;
#if ENABLE_TRACING_GFX
                if (ScrollYDiff < 0)
                    TRACE("ScrollYDiff went negative when substraction VPR.h/.l (%d,%d)\n",
                        IO_VDC_0C_VPR.B.h, IO_VDC_0C_VPR.B.l);
#endif

#if ENABLE_TRACING_DEEP_GFX
                TRACE("ScrollY = %d (h), ", ScrollY);
#endif

                return;

            case SATB:          /* DMA from VRAM to SATB */
                IO_VDC_13_SATB.B.h = V;
                io.vdc_satb = 1;
                io.vdc_status &= ~VDC_SATBfinish;
                return;

            case BXR:           /* Horizontal screen offset */

                if (IO_VDC_07_BXR.B.h == (V & 3))
                    return;

                save_gfx_context(0);

                if (!scroll) {
                    oldScrollX = ScrollX;
                    oldScrollY = ScrollY;
                    oldScrollYDiff = ScrollYDiff;
                }

                IO_VDC_07_BXR.B.h = V & 3;
                scroll = 1;
                return;
            }
            IO_VDC_active.B.h = V;

#ifndef FINAL_RELEASE
            if (io.vdc_reg > 19) {
                fprintf(stderr, "ignore write hi vdc%d,%02x\n", io.vdc_reg,
                        V);
            }
#endif

            return;
        }
        break;

    case 0x0400:                /* VCE */
        switch (A & 7) {
        case 0:
            /*TRACE("VCE 0, V=%X\n", V); */
            return;

            /* Choose color index */
        case 2:
            io.vce_reg.B.l = V;
            return;
        case 3:
            io.vce_reg.B.h = V & 1;
            return;

            /* Set RGB components for current choosen color */
        case 4:
            io.VCE[io.vce_reg.W].B.l = V;
            {
                uchar c;
                int i, n;
                n = io.vce_reg.W;
                c = io.VCE[n].W >> 1;
                if (n == 0) {
                    for (i = 0; i < 256; i += 16)
                        Pal[i] = c;
                } else if (n & 15)
                    Pal[n] = c;
            }
            return;

        case 5:
            io.VCE[io.vce_reg.W].B.h = V;
            {
                uchar c;
                int i, n;
                n = io.vce_reg.W;
                c = io.VCE[n].W >> 1;
                if (n == 0) {
                    for (i = 0; i < 256; i += 16)
                        Pal[i] = c;
                } else if (n & 15)
                    Pal[n] = c;
            }
            io.vce_reg.W = (io.vce_reg.W + 1) & 0x1FF;
            return;
        }
        break;


    case 0x0800:                /* PSG */

        switch (A & 15) {

            /* Select PSG channel */
        case 0:
            io.psg_ch = V & 7;
            return;

            /* Select global volume */
        case 1:
            io.psg_volume = V;
            return;

            /* Frequency setting, 8 lower bits */
        case 2:
            io.PSG[io.psg_ch][2] = V;
            break;

            /* Frequency setting, 4 upper bits */
        case 3:
            io.PSG[io.psg_ch][3] = V & 15;
            break;

        case 4:
            io.PSG[io.psg_ch][4] = V;
#if ENABLE_TRACING_AUDIO
            if ((V & 0xC0) == 0x40)
                io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] = 0;
#endif
            break;

            /* Set channel specific volume */
        case 5:
            io.PSG[io.psg_ch][5] = V;
            break;

            /* Put a value into the waveform or direct audio buffers */
        case 6:
            if (io.PSG[io.psg_ch][PSG_DDA_REG] & PSG_DDA_DIRECT_ACCESS) {
                io.psg_da_data[io.psg_ch][io.psg_da_index[io.psg_ch]] = V;
                io.psg_da_index[io.psg_ch] =
                    (io.psg_da_index[io.psg_ch] + 1) & 0x3FF;
                if (io.psg_da_count[io.psg_ch]++ >
                    (PSG_DIRECT_ACCESS_BUFSIZE - 1)) {
                    if (!io.psg_channel_disabled[io.psg_ch])
                        MESSAGE_INFO
                            ("Audio being put into the direct access buffer faster than it's being played.\n");
                    io.psg_da_count[io.psg_ch] = 0;
                }
            } else {
                io.wave[io.psg_ch][io.PSG[io.psg_ch][PSG_DATA_INDEX_REG]] =
                    V;
                io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] =
                    (io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] + 1) & 0x1F;
            }
            break;

        case 7:
            io.PSG[io.psg_ch][7] = V;
            break;

        case 8:
            io.psg_lfo_freq = V;
            break;

        case 9:
            io.psg_lfo_ctrl = V;
            break;

#ifdef EXTRA_CHECKING
        default:
            fprintf(stderr, "ignored PSG write\n");
#endif
        }
        return;

    case 0x0c00:                /* timer */
        //TRACE("Timer Access: A=%X,V=%X\n", A, V);
        switch (A & 1) {
        case 0:
            io.timer_reload = V & 127;
            return;
        case 1:
            V &= 1;
            if (V && !io.timer_start)
                io.timer_counter = io.timer_reload;
            io.timer_start = V;
            return;
        }
        break;

    case 0x1000:                /* joypad */
//              TRACE("V=%02X\n", V);
        io.joy_select = V & 1;
        //io.joy_select = V;
        if (V & 2)
            io.joy_counter = 0;
        return;

    case 0x1400:                /* IRQ */
        switch (A & 15) {
        case 2:
            io.irq_mask = V;    /*TRACE("irq_mask = %02X\n", V); */
            return;
        case 3:
            io.irq_status = (io.irq_status & ~TIRQ) | (V & 0xF8);
            return;
        }
        break;

    case 0x1A00:
        {

            if ((A & 0x1AF0) == 0x1AE0) {
                switch (A & 15) {
                case 0:
                    io.ac_shift = (io.ac_shift & 0xffffff00) | V;
                    break;
                case 1:
                    io.ac_shift = (io.ac_shift & 0xffff00ff) | (V << 8);
                    break;
                case 2:
                    io.ac_shift = (io.ac_shift & 0xff00ffff) | (V << 16);
                    break;
                case 3:
                    io.ac_shift = (io.ac_shift & 0x00ffffff) | (V << 24);
                    break;
                case 4:
                    io.ac_shiftbits = V & 0x0f;
                    if (io.ac_shiftbits != 0) {
                        if (io.ac_shiftbits < 8) {
                            io.ac_shift <<= io.ac_shiftbits;
                        } else {
                            io.ac_shift >>= (16 - io.ac_shiftbits);
                        }
                    }
                default:
                    break;
                }
                return;
            } else {
                uchar ac_port = (A >> 4) & 3;
                switch (A & 15) {
                case 0:
                case 1:

                    if (io.ac_control[ac_port] & AC_USE_OFFSET) {
                        ac_extra_mem[((io.ac_base[ac_port]
                                       +
                                       io.
                                       ac_offset[ac_port]) & 0x1fffff)] =
                            V;
                    } else {
                        ac_extra_mem[((io.ac_base[ac_port]) & 0x1fffff)] =
                            V;
                    }

                    if (io.ac_control[ac_port] & AC_ENABLE_INC) {
                        if (io.ac_control[ac_port] & AC_INCREMENT_BASE)
                            io.ac_base[ac_port] =
                                (io.ac_base[ac_port] +
                                 io.ac_incr[ac_port]) & 0xffffff;
                        else
                            io.ac_offset[ac_port] =
                                (io.ac_offset[ac_port] +
                                 io.ac_incr[ac_port]) & 0xffffff;
                    }

                    return;
                case 2:
                    io.ac_base[ac_port] =
                        (io.ac_base[ac_port] & 0xffff00) | V;
                    return;
                case 3:
                    io.ac_base[ac_port] =
                        (io.ac_base[ac_port] & 0xff00ff) | (V << 8);
                    return;
                case 4:
                    io.ac_base[ac_port] =
                        (io.ac_base[ac_port] & 0x00ffff) | (V << 16);
                    return;
                case 5:
                    io.ac_offset[ac_port] =
                        (io.ac_offset[ac_port] & 0xff00) | V;
                    return;
                case 6:
                    io.ac_offset[ac_port] =
                        (io.ac_offset[ac_port] & 0x00ff) | (V << 8);
                    if (io.ac_control[ac_port] & (AC_ENABLE_OFFSET_BASE_6))
                        io.ac_base[ac_port] =
                            (io.ac_base[ac_port] +
                             io.ac_offset[ac_port]) & 0xffffff;
                    return;
                case 7:
                    io.ac_incr[ac_port] =
                        (io.ac_incr[ac_port] & 0xff00) | V;
                    return;
                case 8:
                    io.ac_incr[ac_port] =
                        (io.ac_incr[ac_port] & 0x00ff) | (V << 8);
                    return;
                case 9:
                    io.ac_control[ac_port] = V;
                    return;
                case 0xa:
                    if ((io.ac_control[ac_port]
                         & (AC_ENABLE_OFFSET_BASE_A |
                            AC_ENABLE_OFFSET_BASE_6))
                        == (AC_ENABLE_OFFSET_BASE_A |
                            AC_ENABLE_OFFSET_BASE_6))
                        io.ac_base[ac_port] =
                            (io.ac_base[ac_port] +
                             io.ac_offset[ac_port]) & 0xffffff;
                    return;
                default:
                    Log("Unknown AC write %d into 0x%04X\n", V, A);
                }

            }
        }
        break;

    case 0x1800:                /* CD-ROM extention */
#if defined(BSD_CD_HARDWARE_SUPPORT)
        pce_cd_handle_write_1800(A, V);
#else
        gpl_pce_cd_handle_write_1800(A, V);
#endif
        break;
    }
#ifndef FINAL_RELEASE
    fprintf(stderr,
            "ignore I/O write %04x,%02x\tBase adress of port %X\nat PC = %04X\n",
            A, V, A & 0x1CC0,
            reg_pc);
#endif
//          DebugDumpTrace(4, 1);
}

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


char syscard_filename[PCE_PATH_MAX];

/*****************************************************************************

		Function: search_possible_syscard

		Description: Search for a system card rom
		Parameters: none
		Return: NULL if none found, else a pointer to a static area containing
							its name

*****************************************************************************/
char *
search_possible_syscard()
{
	MESSAGE_INFO("We need a syscard to load a CD, begining search...\n");
	FILE *f;

	char config_path[PCE_PATH_MAX];
	snprintf(config_path, PCE_PATH_MAX, "%s/", config_basepath);

#if defined(__haiku__)
#define POSSIBLE_LOCATION_COUNT 2
	const char *POSSIBLE_LOCATION[POSSIBLE_LOCATION_COUNT] = {
		"./", config_path
	};
#else
#define POSSIBLE_LOCATION_COUNT 3
	const char *POSSIBLE_LOCATION[POSSIBLE_LOCATION_COUNT] = {
		"./", config_path, "/usr/share/huexpress/syscard"
	};
#endif

#define POSSIBLE_FILENAME_COUNT	4
	const char *POSSIBLE_FILENAME[POSSIBLE_FILENAME_COUNT] = {
		"syscard.pce", "syscard3.pce", "syscard30.pce", "cd-rom~1.pce"
	};

	int location, filename;
	char temp_buffer[PCE_PATH_MAX];

	for (location = 0; location <= POSSIBLE_LOCATION_COUNT; location++)
		for (filename = 0; filename < POSSIBLE_FILENAME_COUNT; filename++) {

			if (location < POSSIBLE_LOCATION_COUNT)
				strcpy(temp_buffer, POSSIBLE_LOCATION[location]);
			else {
				strcpy(temp_buffer, config_basepath);
				strcat(temp_buffer, "/");
			}

			strcat(temp_buffer, POSSIBLE_FILENAME[filename]);
			TRACE("Checking for CD syscard at : %s\n", temp_buffer);
			if ((f = fopen(temp_buffer, "rb")) != NULL) {
				fclose(f);
				strncpy(syscard_filename, temp_buffer,
						sizeof(syscard_filename));
				MESSAGE_INFO("Found CD system card at %s\n",
							 syscard_filename);
				return syscard_filename;
			}
		}

	return NULL;
}


/*****************************************************************************

		Function: search_syscard

		Description: Search for a system card rom
		Parameters: none

*****************************************************************************/
void
search_syscard()
{
	char *syscard_location;

	syscard_location = search_possible_syscard();

	if (NULL == syscard_location) {
		MESSAGE_ERROR
			("No CD system cards were found, can not continue.\n");
	} else {
		CartInit(syscard_location);
	}
}


uchar
CartInit(char* name)
{
	MESSAGE_INFO("Opening %s...\n", name);
	uchar CDemulation = 0;

	if (CD_emulation == 1 || strstr(name, "/dev/disk/atapi/")
		|| strstr(name, "/dev/sr")) {
		CDemulation = 1;
		MESSAGE_INFO("Using Hardware CD Device to load CDRom2\n");
		strcpy(ISO_filename, name);
		search_syscard();
	} else if (strcasestr(name, ".PCE")) {
		// ROM Image or CD system card
		CDemulation = 0;
		strcpy(rom_file_name, name);
	} else if (strcasestr(name, ".HCD")) {
		// HuGO! CD definition provided
		CDemulation = 5;
		MESSAGE_INFO("Using Hu-Go! CD definition emulation\n");

		// Load correct ISO filename
		strcpy(ISO_filename, name);

		if (!fill_HCD_info(name))
			return 1;

		search_syscard();
	} else if (strcasestr(name, ".ISO")) {
		// Enable ISO support
		CDemulation = 2;
		MESSAGE_INFO("Using CD ISO emulation\n");

		// Load correct ISO filename
		strcpy(ISO_filename, name);

		search_syscard();
	} else if (strcasestr(name, ".ISQ")) {
		// Enable ISQ support
		CDemulation = 3;
		MESSAGE_INFO("Using CD ISQ emulation\n");

		// Load correct ISO filename
		strcpy(ISO_filename, name);

		search_syscard();
	} else if (strcasestr(name, ".BIN")) {
		// Enable BIN support
		CDemulation = 4;
		MESSAGE_INFO("Using CD BIN emulation\n");

		// Load correct ISO filename
		strcpy(ISO_filename, name);

		search_syscard();
	} else if (strcasestr(name, ".ZIP")) {
		char filename_in_archive[PCE_PATH_MAX];

		MESSAGE_INFO("Parsing possible ZIP archive\n");

		Log("Testing archive %s\n", name);

		uint32 result = zipmgr_probe_file(name, filename_in_archive);

		if (result == ZIP_ERROR) {
			MESSAGE_ERROR("ZIP file error!\n");
			return 1;
		} else if (result == ZIP_HAS_NONE) {
			MESSAGE_ERROR("No valid game files found in ZIP file!\n");
			return 1;
		}

		if (strcmp(filename_in_archive, "")) {
			Log("Found %s in %s\n", filename_in_archive, name);
			if (result == ZIP_HAS_PCE) {
				size_t unzipped_rom_size = 0;
				char* unzipped_rom = zipmgr_extract_to_memory(name,
					filename_in_archive, &unzipped_rom_size);

				if (unzipped_rom == NULL || unzipped_rom_size == 0) {
					MESSAGE_ERROR("Error expanding rom to memory!\n");
					return 1;
				}

				MESSAGE_INFO("unzipped rom size: %d\n", unzipped_rom_size);

				ROM_size = unzipped_rom_size / 0x2000;

				if ((unzipped_rom_size & 0x1FFF) == 0) {
					/* No header */
					ROM = unzipped_rom;
				} else {
					ROM = malloc(unzipped_rom_size & ~0x1FFF);
					memcpy(ROM, unzipped_rom + (unzipped_rom_size & 0x1FFF),
						unzipped_rom_size & ~0x1FFF);
					free(unzipped_rom);
				}
				return 0;
			} else {
				if (zipmgr_extract_to_disk(name, tmp_basepath)) {
					MESSAGE_ERROR("Error eixtracting zipfile\n");
					return 1;
				}
				char tmpGame[PCE_PATH_MAX];
				snprintf(tmpGame, PCE_PATH_MAX, "%s%s%s", tmp_basepath, PATH_SLASH,
					filename_in_archive);
				CDemulation = CartInit(tmpGame);
			}
		}
		/*
		   strcpy (rom_file_name, tmp_path);
		   fp = fopen (tmp_path, "rb");
		 */
	}
	/*else {
	   // unknown media format
	   CDemulation = 0;
	   strcpy (rom_file_name, name);
	   fp = fopen(name, "rb");
	   }
	 */
	return CDemulation;
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
	CD_emulation = CartInit(name);

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
	ROM = (uchar *)malloc(fsize);
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

	{
		uint32 x, y = (WIDTH - io.screen_w) / 2 - 512 * WIDTH;
		for (x = 0; x < 1024; x++) {
			spr_init_pos[x] = y;
			y += WIDTH;
		}
		// pos = WIDTH*(HEIGHT-FC_H)/2+(WIDTH-FC_W)/2+WIDTH*y+x;
	}

	for (i = 0; i < 6; i++) {
		io.PSG[i][4] = 0x80;
	}

#if !defined(TEST_ROM_RELOCATED)
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
#else
	mmr[7] = 0x68;
	bank_set(7, 0x68);

	mmr[6] = 0x05;
	bank_set(6, 0x05 + 0x68);

	mmr[5] = 0x04;
	bank_set(5, 0x04 + 0x68);

	mmr[4] = 0x03;
	bank_set(4, 0x03 + 0x68);

	mmr[3] = 0x02;
	bank_set(3, 0x02 + 0x68);

	mmr[2] = 0x01;
	bank_set(2, 0x01 + 0x68);
#endif							/* TEST_ROM_RELOCATED */

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

	if (((CD_emulation >= 2) && (CD_emulation <= 5))
		&& (!strcmp(ISO_filename, ""))) {
		CD_emulation = 0;
		// if no ISO name given, give up the emulation
	}


	if ((CD_emulation == 2) || (CD_emulation == 4)) {

		if (!(iso_FILE = fopen(ISO_filename, "rb"))) {
			MESSAGE_ERROR("Couldn't read ISO at %s\n", ISO_filename);
			return 1;
		}

		fill_cd_info();
	}

	TRACE("CD Emulation is %d\n", CD_emulation);

	if (CD_emulation) {
		// We set illegal opcodes to handle CD Bios functions
		uint16 x;

		TRACE("Will hook CD functions\n");

		/* TODO : reenable minimum_bios_hooking when bios hooking rewritten */

		if (!minimum_bios_hooking)
			for (x = 0x01; x < 0x4D; x++)
				if (x != 0x22) {
					// the 0x22th jump is special, points to a one byte routine
					uint16 dest;
					dest = Op6502(0xE000 + x * 3 + 1);
					dest += 256 * Op6502(0xE000 + x * 3 + 2);

					CDBIOS_replace[x][0] = Op6502(dest);
					CDBIOS_replace[x][1] = Op6502(dest + 1);

					Wr6502(dest, 0xFC);
					Wr6502(dest + 1, x);
				}

	}

	return 0;
}


int
InitPCE(char *name)
{
	int i = 0, ROMmask;
	char *tmp_dummy;
	char local_us_encoded_card = 0;

	if ((!strcmp(name, "")) && (CD_emulation != 1))
		return 1;

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

	if (!(tmp_dummy = (char *) (strrchr(ISO_filename, '\\'))))
		tmp_dummy = &ISO_filename[0];
	else
		tmp_dummy++;


	memset(short_iso_name, 0, 80);
	i = 0;
	while ((tmp_dummy[i]) && (tmp_dummy[i] != '.')) {
		short_iso_name[i] = tmp_dummy[i];
		i++;
	}

	if (strlen(short_iso_name)) {
		if (short_iso_name[strlen(short_iso_name) - 1] != '.') {
			short_iso_name[strlen(short_iso_name) + 1] = 0;
			short_iso_name[strlen(short_iso_name)] = '.';
		}
	}

	switch (CD_emulation) {
	case 0:
		sprintf(sav_path, "%s/%ssav", config_basepath,
				short_cart_name);
		break;
	case 1:
		sprintf(sav_path, "%s/cd_sav", config_basepath);
		break;
	case 2:
	case 3:
	case 4:
	case 5:
		sprintf(sav_path, "%s/cd.svi", config_basepath);
		break;
	}

	MESSAGE_INFO("Saved path is %s\n", sav_path);

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

	memset(VRAM, 0, VRAMSIZE);

	memset(VRAM2, 0, VRAMSIZE);

	memset(VRAMS, 0, VRAMSIZE);

	IOAREA = (uchar *) malloc(0x2000);
	memset(IOAREA, 0xFF, 0x2000);

	memset(vchange, 1, VRAMSIZE / 32);

	memset(vchanges, 1, VRAMSIZE / 128);

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
/*
	if (CD_emulation)
		{

			cd_extra_mem = (uchar *) malloc (0x10000);
			memset (cd_extra_mem, 0, 0x10000);

			cd_extra_super_mem = (uchar *) malloc (0x30000);
			memset (cd_extra_super_mem, 0, 0x30000);

			ac_extra_mem = (uchar *) malloc (0x200000);
			memset (ac_extra_mem, 0, 0x200000);

			cd_sector_buffer = (uchar *) malloc (0x2000);

			// cd_read_buffer = (uchar *)malloc(0x2000);

		}
*/

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
		ROMMapR[i] = trap_ram_read;
		ROMMapW[i] = trap_ram_write;
	}

#if ! defined(TEST_ROM_RELOCATED)
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
#else
	for (i = 0x68; i < 0x88; i++) {
		if (ROM_size == 0x30) {
			switch (i & 0x70) {
			case 0x00:
			case 0x10:
			case 0x50:
				ROMMapR[i] = ROM + ((i - 0x68) & ROMmask) * 0x2000;
				ROMMapW[i] = ROM + ((i - 0x68) & ROMmask) * 0x2000;
				break;
			case 0x20:
			case 0x60:
				ROMMapR[i] =
					ROM + (((i - 0x68) - 0x20) & ROMmask) * 0x2000;
				ROMMapW[i] =
					ROM + (((i - 0x68) - 0x20) & ROMmask) * 0x2000;
				break;
			case 0x30:
			case 0x70:
				ROMMapR[i] =
					ROM + (((i - 0x68) - 0x10) & ROMmask) * 0x2000;
				ROMMapW[i] =
					ROM + (((i - 0x68) - 0x10) & ROMmask) * 0x2000;
				break;
			case 0x40:
				ROMMapR[i] =
					ROM + (((i - 0x68) - 0x20) & ROMmask) * 0x2000;
				ROMMapW[i] =
					ROM + (((i - 0x68) - 0x20) & ROMmask) * 0x2000;
				break;
			}
		} else {
			ROMMapR[i] = ROM + ((i - 0x68) & ROMmask) * 0x2000;
			ROMMapW[i] = ROM + ((i - 0x68) & ROMmask) * 0x2000;
		}
	}
#endif

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

	if (CD_emulation) {
		ROMMapR[0x80] = cd_extra_mem;
		ROMMapR[0x81] = cd_extra_mem + 0x2000;
		ROMMapR[0x82] = cd_extra_mem + 0x4000;
		ROMMapR[0x83] = cd_extra_mem + 0x6000;
		ROMMapR[0x84] = cd_extra_mem + 0x8000;
		ROMMapR[0x85] = cd_extra_mem + 0xA000;
		ROMMapR[0x86] = cd_extra_mem + 0xC000;
		ROMMapR[0x87] = cd_extra_mem + 0xE000;

		ROMMapW[0x80] = cd_extra_mem;
		ROMMapW[0x81] = cd_extra_mem + 0x2000;
		ROMMapW[0x82] = cd_extra_mem + 0x4000;
		ROMMapW[0x83] = cd_extra_mem + 0x6000;
		ROMMapW[0x84] = cd_extra_mem + 0x8000;
		ROMMapW[0x85] = cd_extra_mem + 0xA000;
		ROMMapW[0x86] = cd_extra_mem + 0xC000;
		ROMMapW[0x87] = cd_extra_mem + 0xE000;

		for (i = 0x68; i < 0x80; i++) {
			ROMMapR[i] = cd_extra_super_mem + 0x2000 * (i - 0x68);
			ROMMapW[i] = cd_extra_super_mem + 0x2000 * (i - 0x68);
		}

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
		char backupmem[PCE_PATH_MAX];
		snprintf(backupmem, PCE_PATH_MAX, "%s/backupmem.bin", config_basepath);

		FILE *fp;
		fp = fopen(backupmem, "rb");

		if (fp == NULL)
			fprintf(stderr, "Can't open %s\n", backupmem);
		else {
			fread(WRAM, 0x2000, 1, fp);
			fclose(fp);
		}

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
#ifdef MY_REALLOC_MEMORY_SIDEARMS
	{ // SIDE Arms: 46,5 -> 47
    // exe_go: bank_set: 7,0 -> 3F89BDA4
    void *mem_fast = rg_alloc(0x2000, MEM_FAST);
    int nr = 7;
    memcpy(mem_fast, ROMMapR[0], 0x2000);
    printf("FAST MEM: %X -> %X\n", ROMMapR[0], mem_fast);
    ROMMapR[0] = mem_fast;
    ROMMapW[0] = mem_fast;
    }
#endif

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

	char backupmem[PCE_PATH_MAX];
	snprintf(backupmem, PCE_PATH_MAX, "%s/backupmem.bin", config_basepath);

	// Save the backup ram into file
	if (!(fp = fopen(backupmem, "wb"))) {
		memset(WRAM, 0, 0x2000);
		MESSAGE_ERROR("Can't open %s for saving RAM\n", backupmem);
	} else {
		fwrite(WRAM, 0x2000, 1, fp);
		fclose(fp);
		MESSAGE_INFO("%s used for saving RAM\n", backupmem);
	}

	// Set volume to zero
	io.psg_volume = 0;

	sprintf(tmp_buf, "rm -rf %s/tmp/*", config_basepath);
	system(tmp_buf);

	sprintf(tmp_buf, "%s/tmp", config_basepath);
	rmdir(tmp_buf);

	if (CD_emulation == 1)
		osd_cd_close();

	if ((CD_emulation == 2) || (CD_emulation == 4))
		fclose(iso_FILE);

	if (CD_emulation == 5)
		HCD_shutdown();

	if (IOAREA)
		free(IOAREA);

	if (ROM)
		free(ROM);

	if (PopRAM)
		free(PopRAM);
/*
	if (CD_emulation)
		{

			if (cd_extra_mem)
	free (cd_extra_mem);
			if (cd_sector_buffer)
	free (cd_sector_buffer);
			if (cd_extra_super_mem)
	free (cd_extra_super_mem);
			if (cd_buf)
	free (cd_buf);

		}
*/
	hard_term();

	return;
}


#ifdef CHRONO
unsigned nb_used[256], time_used[256];
#endif

#ifndef FINAL_RELEASE
extern int mseq(unsigned *);
extern void mseq_end();
extern void WriteBuffer_end();
extern void write_psg_end();
extern void WriteBuffer(char *, int, unsigned);
#endif


FILE *out_snd;


#ifdef MY_VDC_VARS

// DRAM_ATTR
// WORD_ALIGNED_ATTR

#define VAR_PREFIX DRAM_ATTR WORD_ALIGNED_ATTR

VAR_PREFIX pair IO_VDC_00_MAWR ;
VAR_PREFIX pair IO_VDC_01_MARR ;
VAR_PREFIX pair IO_VDC_02_VWR  ;
VAR_PREFIX pair IO_VDC_03_vdc3 ;
VAR_PREFIX pair IO_VDC_04_vdc4 ;
VAR_PREFIX pair IO_VDC_05_CR   ;
VAR_PREFIX pair IO_VDC_06_RCR  ;
VAR_PREFIX pair IO_VDC_07_BXR  ;
VAR_PREFIX pair IO_VDC_08_BYR  ;
VAR_PREFIX pair IO_VDC_09_MWR  ;
VAR_PREFIX pair IO_VDC_0A_HSR  ;
VAR_PREFIX pair IO_VDC_0B_HDR  ;
VAR_PREFIX pair IO_VDC_0C_VPR  ;
VAR_PREFIX pair IO_VDC_0D_VDW  ;
VAR_PREFIX pair IO_VDC_0E_VCR  ;
VAR_PREFIX pair IO_VDC_0F_DCR  ;
VAR_PREFIX pair IO_VDC_10_SOUR ;
VAR_PREFIX pair IO_VDC_11_DISTR;
VAR_PREFIX pair IO_VDC_12_LENR ;
VAR_PREFIX pair IO_VDC_13_SATB ;
VAR_PREFIX pair IO_VDC_14      ;
VAR_PREFIX pair *IO_VDC_active_ref;
#endif
