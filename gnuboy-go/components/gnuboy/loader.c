#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <ctype.h>

#include "emu.h"
#include "regs.h"
#include "mem.h"
#include "hw.h"
#include "lcd.h"
#include "rtc.h"
#include "cpu.h"
#include "sound.h"

static const byte mbc_table[256] =
{
	0, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 3,
	3, 3, 3, 3, 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 0,
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

#ifdef IS_LITTLE_ENDIAN
#define LIL(x) (x)
#else
#define LIL(x) ((x<<24)|((x&0xff00)<<8)|((x>>8)&0xff00)|(x>>24))
#endif

#define SAVE_VERSION 0x107

#define wavofs  (4096 - 784)
#define hiofs   (4096 - 768)
#define palofs  (4096 - 512)
#define oamofs  (4096 - 256)

#define I1(s, p) { 1, s, p }
#define I2(s, p) { 2, s, p }
#define I4(s, p) { 4, s, p }
#define END { 0, "\0\0\0\0", 0 }

typedef struct
{
	int len;
	char key[4];
	void *ptr;
} svar_t;

static un32 ver;

static svar_t svars[] =
{
	I4("GbSs", &ver),

	I2("PC  ", &PC),
	I2("SP  ", &SP),
	I2("BC  ", &BC),
	I2("DE  ", &DE),
	I2("HL  ", &HL),
	I2("AF  ", &AF),

	I4("IME ", &cpu.ime),
	I4("ima ", &cpu.ima),
	I4("spd ", &cpu.double_speed),
	I4("halt", &cpu.halted),
	I4("div ", &cpu.div),
	I4("tim ", &cpu.timer),
	I4("lcdc", &lcd.cycles),
	I4("snd ", &snd.cycles),

	I4("ints", &hw.ilines),
	I4("pad ", &hw.pad),
	I4("cgb ", &hw.cgb),
	I4("hdma", &hw.hdma),
	I4("seri", &hw.serial),

	I4("mbcm", &mbc.model),
	I4("romb", &mbc.rombank),
	I4("ramb", &mbc.rambank),
	I4("enab", &mbc.enableram),
	I4("batt", &mbc.batt),

	I4("rtcR", &rtc.sel),
	I4("rtcL", &rtc.latch),
	I4("rtcF", &rtc.flags),
	I4("rtcd", &rtc.d),
	I4("rtch", &rtc.h),
	I4("rtcm", &rtc.m),
	I4("rtcs", &rtc.s),
	I4("rtct", &rtc.ticks),
	I1("rtR8", &rtc.regs[0]),
	I1("rtR9", &rtc.regs[1]),
	I1("rtRA", &rtc.regs[2]),
	I1("rtRB", &rtc.regs[3]),
	I1("rtRC", &rtc.regs[4]),

	I4("S1on", &snd.ch[0].on),
	I4("S1p ", &snd.ch[0].pos),
	I4("S1c ", &snd.ch[0].cnt),
	I4("S1ec", &snd.ch[0].encnt),
	I4("S1sc", &snd.ch[0].swcnt),
	I4("S1sf", &snd.ch[0].swfreq),

	I4("S2on", &snd.ch[1].on),
	I4("S2p ", &snd.ch[1].pos),
	I4("S2c ", &snd.ch[1].cnt),
	I4("S2ec", &snd.ch[1].encnt),

	I4("S3on", &snd.ch[2].on),
	I4("S3p ", &snd.ch[2].pos),
	I4("S3c ", &snd.ch[2].cnt),

	I4("S4on", &snd.ch[3].on),
	I4("S4p ", &snd.ch[3].pos),
	I4("S4c ", &snd.ch[3].cnt),
	I4("S4ec", &snd.ch[3].encnt),

	END
};

/**
 * Save file format is:
 * GB:
 * 0x0000 - 0x0FFF: svars, oam, palette, hiram, wave
 * 0x1000 - 0x2FFF: RAM
 * 0x3000 - 0x4FFF: VRAM
 * 0x5000 - 0x...:  SRAM
 *
 * GBC:
 * 0x0000 - 0x0FFF: svars, oam, palette, hiram, wave
 * 0x1000 - 0x8FFF: RAM
 * 0x9000 - 0xCFFF: VRAM
 * 0xD000 - 0x...:  SRAM
 *
 */

int rom_loadbank(int bank)
{
	const size_t BANK_SIZE = 0x4000;
	const size_t OFFSET = bank * BANK_SIZE;

	if (rom.bank[bank])
	{
		printf("bank_load: bank %d already loaded!\n", bank);
		// return 0;
	}

	printf("bank_load: loading bank %d.\n", bank);
	rom.bank[bank] = (byte*)malloc(BANK_SIZE);
	if (rom.bank[bank] == NULL) {
		while (true) {
			uint8_t i = rand() & 0xFF;
			if (rom.bank[i]) {
				printf("bank_load: reclaiming bank %d.\n", i);
				rom.bank[bank] = rom.bank[i];
				rom.bank[i] = NULL;
				break;
			}
		}
	}

	// Make sure no transaction is running
	rg_spi_lock_acquire(SPI_LOCK_SDCARD);

	// Load the 16K page
	if (fseek(fpRomFile, OFFSET, SEEK_SET))
	{
		emu_die("ROM fseek failed");
	}

	if (fread(rom.bank[bank], BANK_SIZE, 1, fpRomFile) < 1)
	{
		emu_die("ROM fread failed");
	}

	rg_spi_lock_release(SPI_LOCK_SDCARD);

	return 0;
}


int rom_load(const char *file)
{
    printf("loader: Loading file: '%s'\n", file);

	fpRomFile = fopen(file, "rb");
	if (fpRomFile == NULL)
	{
		emu_die("ROM fopen failed");
	}

	rom_loadbank(0);

	byte *header = rom.bank[0];

	memcpy(rom.name, header + 0x0134, 16);
	rom.name[16] = 0;

	int tmp = *((int*)(header + 0x0140));
	byte c = tmp >> 24;
	hw.cgb = ((c == 0x80) || (c == 0xc0));

	tmp = *((int*)(header + 0x0144));
	c = (tmp >> 24) & 0xff;
	mbc.type = mbc_table[c];
	mbc.batt = batt_table[c];
	mbc.rtc = rtc_table[c];
	mbc.rumble = (c == 30 || c == 29 || c == 28);

	tmp = *((int*)(header + 0x0148));
	mbc.romsize = romsize_table[(tmp & 0xff)];
	mbc.ramsize = ramsize_table[((tmp >> 8) & 0xff)];
	rom.length = 16384 * mbc.romsize;

	memcpy(&rom.checksum, header + 0x014E, 2);

	if (!mbc.romsize) emu_die("ROM size == 0");
	if (!mbc.ramsize) emu_die("SRAM size == 0");

	const char* mbcName;
	switch (mbc.type)
	{
		case MBC_NONE:   mbcName = "MBC_NONE"; break;
		case MBC_MBC1:   mbcName = "MBC_MBC1"; break;
		case MBC_MBC2:   mbcName = "MBC_MBC2"; break;
		case MBC_MBC3:   mbcName = "MBC_MBC3"; break;
		case MBC_MBC5:   mbcName = "MBC_MBC5"; break;
		case MBC_HUC1:   mbcName = "MBC_HUC1"; break;
		case MBC_HUC3:   mbcName = "MBC_HUC3"; break;
		default:         mbcName = "(unknown)"; break;
	}

	printf("loader: rom.name='%s'\n", rom.name);
	printf("loader: mbc.type=%s, mbc.romsize=%d (%dK), mbc.ramsize=%d (%dK)\n",
		mbcName, mbc.romsize, rom.length / 1024, mbc.ramsize, mbc.ramsize * 8);

	// SRAM
	ram.sbank = rg_alloc(8192 * mbc.ramsize, MEM_FAST);
	ram.sram_dirty = 0;

	memset(ram.sbank, 0xff, 8192 * mbc.ramsize);
	memset(ram.ibank, 0xff, 4096 * 8);

	mbc.rombank = 1;
	mbc.rambank = 0;

	int preload = mbc.romsize < 64 ? mbc.romsize : 64;

	// Some games stutters too much if we don't fully preload it
	if (strncmp(rom.name, "RAYMAN", 6) == 0 || strncmp(rom.name, "NONAME", 6) == 0)
	{
		printf("loader: Special preloading for Rayman 1/2\n");
		preload = mbc.romsize - 8;
	}

	printf("loader: Preloading the first %d banks\n", preload);
	for (int i = 1; i < preload; i++)
	{
		rom_loadbank(i);
	}

	// Apply game-specific hacks
	if (strncmp(rom.name, "SIREN GB2 ", 11) == 0 || strncmp(rom.name, "DONKEY KONG", 11) == 0)
	{
		printf("loader: HACK: Window offset hack enabled\n");
		lcd.enable_window_offset_hack = 1;
	}

	return 0;
}


void rom_unload(void)
{
	for (int i = 0; i < 512; i++) {
		if (rom.bank[i]) {
			free(rom.bank[i]);
			rom.bank[i] = NULL;
		}
	}
	free(ram.sbank);

	mbc.type = mbc.romsize = mbc.ramsize = mbc.batt = mbc.rtc = 0;
	// ram.sbank = NULL;
}


int sram_load(const char *file)
{
	int ret = -1;
	FILE *f;

	if (!mbc.batt || !file || !*file) return -1;

	rg_spi_lock_acquire(SPI_LOCK_SDCARD);

	if ((f = fopen(file, "rb")))
	{
		printf("sram_load: Loading SRAM from '%s'\n", file);
		fread(ram.sbank, 8192, mbc.ramsize, f);
		rtc_load(f);
		fclose(f);
		ret = 0;
	}

	rg_spi_lock_release(SPI_LOCK_SDCARD);
	return ret;
}


int sram_save(const char *file)
{
	int ret = -1;
	FILE *f;

	if (!mbc.batt || !file || !mbc.ramsize) return -1;

	rg_spi_lock_acquire(SPI_LOCK_SDCARD);

	if ((f = fopen(file, "wb")))
	{
		printf("sram_save: Saving SRAM to '%s'\n", file);
		fwrite(ram.sbank, 8192, mbc.ramsize, f);
		rtc_save(f);
		fclose(f);
		ret = 0;
	}

	rg_spi_lock_release(SPI_LOCK_SDCARD);
	return ret;
}


int state_save(const char *file)
{
	byte *buf = calloc(1, 4096);
	if (!buf) return -2;

	FILE *f = fopen(file, "wb");
	if (!f) return -1;

	un32 (*header)[2] = (un32 (*)[2])buf;

	ver = SAVE_VERSION;

	for (int i = 0; svars[i].ptr; i++)
	{
		un32 d = 0;

		switch (svars[i].len)
		{
		case 1:
			d = *(byte *)svars[i].ptr;
			break;
		case 2:
			d = *(un16 *)svars[i].ptr;
			break;
		case 4:
			d = *(un32 *)svars[i].ptr;
			break;
		}

		header[i][0] = *(un32 *)svars[i].key;
		header[i][1] = LIL(d);
	}

	memcpy(buf+hiofs, ram.hi, sizeof ram.hi);
	memcpy(buf+palofs, lcd.pal, sizeof lcd.pal);
	memcpy(buf+oamofs, lcd.oam.mem, sizeof lcd.oam);
	memcpy(buf+wavofs, snd.wave, sizeof snd.wave);

	fwrite(buf, 4096, 1, f);
	fwrite(ram.ibank, 4096, hw.cgb ? 8 : 2, f);
	fwrite(lcd.vbank, 4096, hw.cgb ? 4 : 2, f);
	fwrite(ram.sbank, 4096, mbc.ramsize << 1, f);
	fclose(f);

	free(buf);

	return 0;
}


int state_load(const char *file)
{
	byte* buf = calloc(1, 4096);
	if (!buf) return -2;

	FILE *f = fopen(file, "rb");
	if (!f) return -1;

	fread(buf, 4096, 1, f);
	fread(ram.ibank, 4096, hw.cgb ? 8 : 2, f);
	fread(lcd.vbank, 4096, hw.cgb ? 4 : 2, f);
	fread(ram.sbank, 4096, mbc.ramsize << 1, f);
	fclose(f);

	un32 (*header)[2] = (un32 (*)[2])buf;

	for (int i = 0; svars[i].ptr; i++)
	{
		un32 d = 0;

		for (int j = 0; header[j][0]; j++)
		{
			if (header[j][0] == *(un32 *)svars[i].key)
			{
				d = LIL(header[j][1]);
				break;
			}
		}

		switch (svars[i].len)
		{
		case 1:
			*(byte *)svars[i].ptr = d;
			break;
		case 2:
			*(un16 *)svars[i].ptr = d;
			break;
		case 4:
			*(un32 *)svars[i].ptr = d;
			break;
		}
	}

	memcpy(ram.hi, buf+hiofs, sizeof ram.hi);
	memcpy(lcd.pal, buf+palofs, sizeof lcd.pal);
	memcpy(lcd.oam.mem, buf+oamofs, sizeof lcd.oam);
	memcpy(snd.wave, buf+wavofs, sizeof snd.wave);

	if (ver != SAVE_VERSION)
		printf("state_load: Save file version mismatch!\n");

	free(buf);

	pal_dirty();
	sound_dirty();
	mem_updatemap();

	return 0;
}
