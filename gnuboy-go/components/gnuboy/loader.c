#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "esp_heap_caps.h"
#include "esp_system.h"

#ifndef GNUBOY_NO_MINIZIP
/*
** use http://www.winimage.com/zLibDll/minizip.html v1.1
** which needs zlib
*/
#include <unzip/unzip.h>
#endif /* GNUBOY_USE_MINIZIP */


#include "gnuboy.h"
#include "defs.h"
#include "regs.h"
#include "mem.h"
#include "hw.h"
#include "lcd.h"
#include "rtc.h"
#include "rc.h"
#include "sound.h"

#include "odroid_system.h"
#include "odroid_display.h"

static FILE* RomFile = NULL;

#ifndef GNUBOY_NO_MINIZIP
static int check_zip(char *filename);
static byte *loadzipfile(char *archive, int *filesize);
#endif /* GNUBOY_USE_MINIZIP */

static int mbc_table[256] =
{
	0, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 3,
	3, 3, 3, 3, 0, 0, 0, 0, 0, 5, 5, 5, MBC_RUMBLE, MBC_RUMBLE, MBC_RUMBLE, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, MBC_HUC3, MBC_HUC1
};

static int rtc_table[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0
};

static int batt_table[256] =
{
	0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
	1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0,
	0
};

static int romsize_table[256] =
{
	2, 4, 8, 16, 32, 64, 128, 256, 512,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 128, 128, 128
	/* 0, 0, 72, 80, 96  -- actual values but bad to use these! */
};

static int ramsize_table[256] =
{
	1, 1, 1, 4, 16,
	4 /* FIXME - what value should this be?! */
};


static char *romfile=NULL;
static char *sramfile=NULL;
static char *rtcfile=NULL;
static char *saveprefix=NULL;

static char *savename=NULL;
static char *savedir=NULL;

static int saveslot=0;

static int forcebatt=0, nobatt=0;
static int forcedmg=0, gbamode=0;

static int memfill = 0, memrand = -1;


static void initmem(void *mem, int size)
{
	char *p = mem;
	memset(p, 0xff /*memfill*/, size);
}

static byte *loadfile(FILE *f, int *len)
{
	int l = 0, c = 0;
	byte *d = NULL;
#ifdef GNUBOY_ENABLE_ORIGINAL_SLOW_INCREMENTAL_LOADER
	int p = 0;
	byte buf[512];

	for(;;)
	{
		c = fread(buf, 1, sizeof buf, f);
		if (c <= 0) break;
		l += c;
		d = realloc(d, l);
		if (!d) return 0;
		memcpy(d+p, buf, c);
		p += c;
	}
#else /* fast and no space check */
	/* alloc and read once - NOTE no sanity check on filesize */
	fseek(f, 0, SEEK_END);
	l = ftell(f);
	fseek(f, 0, SEEK_SET);
	d = (byte*) malloc(l);
	if (d != NULL)
	{
		c = fread((void *) d, (size_t) l, 1, f);
		if (c != 1)
		{
			l = 0;
			/* NOTE if this fails caller doesn't catch it (ditto the slow and "safe" version) */
		}
	}
#endif /* GNUBOY_ENABLE_ORIGINAL_SLOW_INCREMENTAL_LOADER */
	*len = l;
	return d;
}

static byte *inf_buf;
static int inf_pos, inf_len;

static void inflate_callback(byte b)
{
	if (inf_pos >= inf_len)
	{
		inf_len += 512;
		inf_buf = realloc(inf_buf, inf_len);
		if (!inf_buf) die("out of memory inflating file @ %d bytes\n", inf_pos);
	}
	inf_buf[inf_pos++] = b;
}

static byte *decompress(byte *data, int *len)
{
	long pos = 0;
	if (data[0] != 0x1f || data[1] != 0x8b)
		return data;
	inf_buf = 0;
	inf_pos = inf_len = 0;
	if (unzip(data, &pos, inflate_callback) < 0)
		return data;
	*len = inf_pos;
	return inf_buf;
}


int IRAM_ATTR rom_loadbank(short bank)
{
	const size_t BANK_SIZE = 0x4000;
	const size_t OFFSET = bank * BANK_SIZE;

	printf("bank_load: loading bank %d.\n", bank);
	rom.bank[bank] = (byte*)heap_caps_malloc(BANK_SIZE, MALLOC_CAP_SPIRAM);
	if (rom.bank[bank] == NULL) {
		while (true) {
			uint8_t i = esp_random() & 0xFF;
			if (rom.bank[i]) {
				printf("bank_load: reclaiming bank %d.\n", i);
				rom.bank[bank] = rom.bank[i];
				rom.bank[i] = NULL;
				break;
			}
		}
	}

	if (rom.bank[bank] == NULL) {
		abort();
	}

	// Stop the SPI bus
	odroid_display_lock();

	// Load the 16K page
	if (fseek(RomFile, OFFSET, SEEK_SET))
	{
		printf("bank_load: fseek failed. OFFSET=%d\n", OFFSET);
		odroid_display_show_error(ODROID_SD_ERR_BADFILE);
		odroid_system_halt();
	}

	if (fread(rom.bank[bank], BANK_SIZE, 1, RomFile) < 1)
	{
		printf("bank_load: fread failed. bank=%d\n", bank);
		odroid_display_show_error(ODROID_SD_ERR_BADFILE);
		odroid_system_halt();
	}

	odroid_display_unlock();

	return 0;
}


int rom_load()
{
    printf("loader: Loading file: %s\n", romfile);

	RomFile = fopen(romfile, "rb");
	if (RomFile == NULL)
	{
		printf("loader: fopen failed.\n");
		odroid_display_show_error(ODROID_SD_ERR_BADFILE);
		odroid_system_halt();
	}

	rom_loadbank(0);

	byte *header = rom.bank[0];

	memcpy(rom.name, header + 0x0134, 16);
	rom.name[16] = 0;
	printf("loader: rom.name='%s'\n", rom.name);

	int tmp = *((int*)(header + 0x0140));
	byte c = tmp >> 24;
	hw.cgb = ((c == 0x80) || (c == 0xc0)) && !forcedmg;
	hw.gba = (hw.cgb && gbamode);

	tmp = *((int*)(header + 0x0144));
	c = (tmp >> 24) & 0xff;
	mbc.type = mbc_table[c];
	mbc.batt = (batt_table[c] && !nobatt) || forcebatt;
	rtc.batt = rtc_table[c];

	tmp = *((int*)(header + 0x0148));
	mbc.romsize = romsize_table[(tmp & 0xff)];
	mbc.ramsize = ramsize_table[((tmp >> 8) & 0xff)];
	rom.length = 16384 * mbc.romsize;

	if (!mbc.romsize) die("unknown ROM size %02X\n", header[0x0148]);
	if (!mbc.ramsize) die("unknown SRAM size %02X\n", header[0x0149]);

	const char* mbcName;
	switch (mbc.type)
	{
		case MBC_NONE:
			mbcName = "MBC_NONE";
			break;

		case MBC_MBC1:
			mbcName = "MBC_MBC1";
			break;

		case MBC_MBC2:
			mbcName = "MBC_MBC2";
			break;

		case MBC_MBC3:
			mbcName = "MBC_MBC3";
			break;

		case MBC_MBC5:
			mbcName = "MBC_MBC5";
			break;

		case MBC_RUMBLE:
			mbcName = "MBC_RUMBLE";
			break;

		case MBC_HUC1:
			mbcName = "MBC_HUC1";
			break;

		case MBC_HUC3:
			mbcName = "MBC_HUC3";
			break;

		default:
			mbcName = "(unknown)";
			break;
	}

	int sram_length = 8192 * mbc.ramsize;
	printf("loader: mbc.type=%s, mbc.romsize=%d (%dK), mbc.ramsize=%d (%dK)\n", mbcName, mbc.romsize, rom.length / 1024, mbc.ramsize, sram_length / 1024);

	// SRAM
	ram.sram_dirty = 1;
	ram.sbank = malloc(sram_length);

	initmem(ram.sbank, sram_length);
	initmem(ram.ibank, 4096 * 8);

	mbc.rombank = 1;
	mbc.rambank = 0;

	int preload = mbc.romsize < 32 ? mbc.romsize : 32;

	printf("loader: Preloading the first %d banks\n", preload);
	for (int i = 1; i < preload; i++) {
		rom_loadbank(i);
	}

	return 0;
}


int sram_load()
{
	if (!mbc.batt) return -1;

	/* Consider sram loaded at this point, even if file doesn't exist */
	ram.loaded = 1;

	return 0;
}


int sram_save()
{
	/* If we crash before we ever loaded sram, DO NOT SAVE! */
	if (!mbc.batt || !ram.loaded || !mbc.ramsize)
		return -1;

	return 0;
}


void state_save(int n)
{
	FILE *f;
	char *name;

	if (n < 0) n = saveslot;
	if (n < 0) n = 0;
	name = malloc(strlen(saveprefix) + 5);
	sprintf(name, "%s.%03d", saveprefix, n);

	if ((f = fopen(name, "wb")))
	{
		savestate(f);
		fclose(f);
	}
	free(name);
}


void state_load(int n)
{
	FILE *f;
	char *name;

	if (n < 0) n = saveslot;
	if (n < 0) n = 0;
	name = malloc(strlen(saveprefix) + 5);
	sprintf(name, "%s.%03d", saveprefix, n);

	if ((f = fopen(name, "rb")))
	{
		loadstate(f);
		fclose(f);
		vram_dirty();
		pal_dirty();
		sound_dirty();
		mem_updatemap();
	}
	free(name);
}

void rtc_save()
{
	FILE *f;
	if (!rtc.batt) return;
	if (!(f = fopen(rtcfile, "wb"))) return;
	rtc_save_internal(f);
	fclose(f);
}

void rtc_load()
{
	//FILE *f;
	if (!rtc.batt) return;
	//if (!(f = fopen(rtcfile, "r"))) return;
	//rtc_load_internal(f);
	//fclose(f);
}


void loader_unload()
{
	sram_save();
	if (romfile) free(romfile);
	if (sramfile) free(sramfile);
	if (saveprefix) free(saveprefix);
	if (ram.sbank) free(ram.sbank);

	for (int i = 0; i < 512; i++) {
		if (rom.bank[i]) {
			free(rom.bank[i]);
			rom.bank[i] = NULL;
		}
	}

	mbc.type = mbc.romsize = mbc.ramsize = mbc.batt = 0;
	ram.sbank = romfile = sramfile = saveprefix = 0;
}

/* basename/dirname like function */
static char *base(char *s)
{
	char *p;
	p = (char *) strrchr((unsigned char)s, DIRSEP_CHAR);
	if (p) return p+1;
	return s;
}

static char *ldup(char *s)
{
	int i;
	char *n, *p;
	p = n = malloc(strlen(s));
	for (i = 0; s[i]; i++) if (isalnum((unsigned char)s[i])) *(p++) = tolower((unsigned char)s[i]);
	*p = 0;
	return n;
}

static void cleanup()
{
	sram_save();
	rtc_save();
	/* IDEA - if error, write emergency savestate..? */
}

void loader_init(char *s)
{
	romfile = s;

	rom_load();
	rtc_load();

	//atexit(cleanup);
}

rcvar_t loader_exports[] =
{
	RCV_STRING("savedir", &savedir),
	RCV_STRING("savename", &savename),
	RCV_INT("saveslot", &saveslot),
	RCV_BOOL("forcebatt", &forcebatt),
	RCV_BOOL("nobatt", &nobatt),
	RCV_BOOL("forcedmg", &forcedmg),
	RCV_BOOL("gbamode", &gbamode),
	RCV_INT("memfill", &memfill),
	RCV_INT("memrand", &memrand),
	RCV_END
};

#ifndef GNUBOY_NO_MINIZIP
/*
** Simplistic zip support, only loads the first file in a zip file
** with no check as to the type (or filename/extension).
*/

/*
**  returns 1 if a filename is a zip file
*/
static int check_zip(char *filename)
{
    char buf[2];
    FILE *fd = NULL;
    fd = fopen(filename, "rb");
    if(!fd) return (0);
    fread(buf, 2, 1, fd);
    fclose(fd);
    if(memcmp(buf, "PK", 2) == 0) return (1);
    return (0);
}

static byte *loadzipfile(char *archive, int *filesize)
{
    char name[256];
    unsigned char *buffer=NULL;
    int zerror = UNZ_OK;
    unzFile zhandle;
    unz_file_info zinfo;
    char tmp_header[0x200];
    unsigned char rom_found=0;

    zhandle = unzOpen(archive);
    if(!zhandle) return (NULL);

#ifdef IS_LITTLE_ENDIAN
#define GAMEBOY_HEADER_MAGIC 0x6666EDCE
#else /* BIG ENDIAN */
#define GAMEBOY_HEADER_MAGIC 0xCEED6666
#endif /* IS_LITTLE_ENDIAN */

    /* Find first gameboy rom, do not use file extension, look for magic bytes/fingerprint */
    /* Seek to first file in archive */
    zerror = unzGoToFirstFile(zhandle);
    if(zerror != UNZ_OK)
    {
        unzClose(zhandle);
        return (NULL);
    }

    do
    {
        unzOpenCurrentFile(zhandle);
        unzReadCurrentFile(zhandle, tmp_header, sizeof(tmp_header));
        unzCloseCurrentFile(zhandle);
        if ((*((unsigned long *)(tmp_header + 0x104))) == GAMEBOY_HEADER_MAGIC)
        {
            /* Gameboy Rom found! */
            rom_found++;
            break;
        }
    } while (unzGoToNextFile(zhandle) != UNZ_END_OF_LIST_OF_FILE);

    if (rom_found == 0)
        return (NULL);

    /* Get information about the file */
    unzGetCurrentFileInfo(zhandle, &zinfo, &name[0], 0xff, NULL, 0, NULL, 0);
    *filesize = zinfo.uncompressed_size;

    /* Error: file size is zero */
    if(*filesize <= 0)
    {
        unzClose(zhandle);
        return (NULL);
    }

    /* Open current file */
    zerror = unzOpenCurrentFile(zhandle);
    if(zerror != UNZ_OK)
    {
        unzClose(zhandle);
        return (NULL);
    }

    /* Allocate buffer and read in file */
    buffer = malloc(*filesize);
    if(!buffer) return (NULL);
    zerror = unzReadCurrentFile(zhandle, buffer, *filesize);

    /* Internal error: free buffer and close file */
    if(zerror < 0 || zerror != *filesize)
    {
        free(buffer);
        buffer = NULL;
        unzCloseCurrentFile(zhandle);
        unzClose(zhandle);
        return (NULL);
    }

    /* Close current file and archive file */
    unzCloseCurrentFile(zhandle);
    unzClose(zhandle);

    return (buffer);
}
#endif /* GNUBOY_USE_MINIZIP */
