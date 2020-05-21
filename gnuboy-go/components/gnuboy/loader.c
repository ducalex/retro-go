#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "defs.h"
#include "regs.h"
#include "mem.h"
#include "hw.h"
#include "lcd.h"
#include "rtc.h"
#include "sound.h"

#include "esp_heap_caps.h"
#include "odroid_system.h"

static const byte mbc_table[256] =
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

static const byte rtc_table[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0
};

static const byte batt_table[256] =
{
	0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
	1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0,
	0
};

static const short romsize_table[256] =
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

static const byte ramsize_table[256] =
{
	1, 1, 1, 4, 16,
	4 /* FIXME - what value should this be?! */
};


static FILE* fpRomFile = NULL;

static char *romfile=NULL;
static char *sramfile=NULL;
// static char *rtcfile=NULL;
static char *saveprefix=NULL;

static int forcebatt=0, nobatt=0;
static int forcedmg=0, gbamode=0;


static inline void initmem(void *mem, int size)
{
	memset(mem, 0xff, size);
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
		odroid_system_panic("Out of memory");
	}

	// Make sure no transaction is running
	odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD);

	// Load the 16K page
	if (fseek(fpRomFile, OFFSET, SEEK_SET))
	{
		printf("bank_load: fseek failed. OFFSET=%d\n", OFFSET);
		odroid_system_panic("ROM fseek failed");
	}

	if (fread(rom.bank[bank], BANK_SIZE, 1, fpRomFile) < 1)
	{
		printf("bank_load: fread failed. bank=%d\n", bank);
		odroid_system_panic("ROM fread failed");
	}

	odroid_system_spi_lock_release(SPI_LOCK_SDCARD);

	return 0;
}


int rom_load()
{
    printf("loader: Loading file: %s\n", romfile);

	fpRomFile = fopen(romfile, "rb");
	if (fpRomFile == NULL)
	{
		printf("loader: fopen failed.\n");
		odroid_system_panic("ROM fopen failed");
	}

	rom_loadbank(0);

	byte *header = rom.bank[0];

	memcpy(rom.name, header + 0x0134, 16);
	rom.name[16] = 0;

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

	memcpy(&rom.checksum, header + 0x014E, 2);

	if (!mbc.romsize) die("unknown ROM size %02X\n", header[0x0148]);
	if (!mbc.ramsize) die("unknown SRAM size %02X\n", header[0x0149]);

	const char* mbcName;
	switch (mbc.type)
	{
		case MBC_NONE:   mbcName = "MBC_NONE"; break;
		case MBC_MBC1:   mbcName = "MBC_MBC1"; break;
		case MBC_MBC2:   mbcName = "MBC_MBC2"; break;
		case MBC_MBC3:   mbcName = "MBC_MBC3"; break;
		case MBC_MBC5:   mbcName = "MBC_MBC5"; break;
		case MBC_RUMBLE: mbcName = "MBC_RUMBLE"; break;
		case MBC_HUC1:   mbcName = "MBC_HUC1"; break;
		case MBC_HUC3:   mbcName = "MBC_HUC3"; break;
		default:         mbcName = "(unknown)"; break;
	}

	printf("loader: rom.name='%s'\n", rom.name);
	printf("loader: mbc.type=%s, mbc.romsize=%d (%dK), mbc.ramsize=%d (%dK)\n",
		mbcName, mbc.romsize, rom.length / 1024, mbc.ramsize, mbc.ramsize * 8);

	// SRAM
	ram.sbank = heap_caps_malloc_prefer(8192 * mbc.ramsize, MALLOC_CAP_INTERNAL, MALLOC_CAP_SPIRAM);
	ram.sram_dirty = 0;

	initmem(ram.sbank, 8192 * mbc.ramsize);
	initmem(ram.ibank, 4096 * 8);

	mbc.rombank = 1;
	mbc.rambank = 0;

	int preload = mbc.romsize < 64 ? mbc.romsize : 64;

	if (strncmp(rom.name, "RAYMAN", 6) == 0) {
		printf("loader: Special preloading for Rayman 1/2\n");
		preload = mbc.romsize - 8;
	}

	printf("loader: Preloading the first %d banks\n", preload);
	for (int i = 1; i < preload; i++) {
		rom_loadbank(i);
	}

	return 0;
}


int sram_load()
{
	int ret = -1;
	FILE *f;

	if (!mbc.batt || !sramfile || !*sramfile) return -1;

	odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD);

	if ((f = fopen(sramfile, "rb")))
	{
		printf("sram_load: Loading SRAM\n");
		fread(ram.sbank, 8192, mbc.ramsize, f);
		rtc_load_internal(f); // Temporary hack, hopefully
		fclose(f);
		ret = 0;
	}

	odroid_system_spi_lock_release(SPI_LOCK_SDCARD);
	return ret;
}


int sram_save()
{
	int ret = -1;
	FILE *f;

	if (!mbc.batt || !sramfile || !mbc.ramsize) return -1;

	odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD);

	if ((f = fopen(sramfile, "wb")))
	{
		printf("sram_load: Saving SRAM\n");
		fwrite(ram.sbank, 8192, mbc.ramsize, f);
		rtc_save_internal(f); // Temporary hack, hopefully
		fclose(f);
		ret = 0;
	}

	odroid_system_spi_lock_release(SPI_LOCK_SDCARD);
	return ret;
}


int state_save(char *name)
{
	FILE *f;

	if ((f = fopen(name, "wb")))
	{
		savestate(f);
		rtc_save_internal(f);
		fclose(f);
		return 0;
	}

	return -1;
}


int state_load(char *name)
{
	FILE *f;

	if ((f = fopen(name, "rb")))
	{
		loadstate(f);
		rtc_load_internal(f);
		fclose(f);
		vram_dirty();
		pal_dirty();
		sound_dirty();
		mem_updatemap();
		return 0;
	}

	return -1;
}

void rtc_save()
{
	if (!rtc.batt) return;
	// FILE *f;
	// if (!(f = fopen(rtcfile, "wb"))) return;
	// rtc_save_internal(f);
	// fclose(f);
}

void rtc_load()
{
	if (!rtc.batt) return;
	// FILE *f;
	// if (!(f = fopen(rtcfile, "r"))) return;
	// rtc_load_internal(f);
	// fclose(f);
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
	romfile = sramfile = saveprefix = NULL;
	// ram.sbank = NULL;
}

void loader_init(char *s)
{
	romfile  = odroid_system_get_path(ODROID_PATH_ROM_FILE);
	sramfile = odroid_system_get_path(ODROID_PATH_SAVE_SRAM);

	rom_load();
	rtc_load();
	// sram_load();
}
