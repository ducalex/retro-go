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
#include "cpu.h"
#include "sound.h"

#include <odroid_system.h>

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


#ifdef IS_LITTLE_ENDIAN
#define LIL(x) (x)
#else
#define LIL(x) ((x<<24)|((x&0xff00)<<8)|((x>>8)&0xff00)|(x>>24))
#endif

#define I1(s, p) { 1, s, p }
#define I2(s, p) { 2, s, p }
#define I4(s, p) { 4, s, p }
#define R(r) I1(#r, &R_##r)
#define NOSAVE { -1, "\0\0\0\0", 0 }
#define END { 0, "\0\0\0\0", 0 }

typedef struct
{
	int len;
	char key[4];
	void *ptr;
} svar_t;

static int ver;
static int sramblock, iramblock, vramblock;
static int hiofs, palofs, oamofs, wavofs;

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
	I4("spd ", &cpu.speed),
	I4("halt", &cpu.halt),
	I4("div ", &cpu.div),
	I4("tim ", &cpu.timer),
	I4("lcdc", &lcd.cycles),
	I4("snd ", &snd.cycles),

	I1("ints", &hw.ilines),
	I1("pad ", &hw.pad),
	I4("cgb ", &hw.cgb),
	I4("gba ", &hw.gba),

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

	I4("hdma", &hw.hdma),

	I4("sram", &sramblock),
	I4("iram", &iramblock),
	I4("vram", &vramblock),
	I4("hi  ", &hiofs),
	I4("pal ", &palofs),
	I4("oam ", &oamofs),
	I4("wav ", &wavofs),

	END
};


int IRAM_ATTR rom_loadbank(short bank)
{
	const size_t BANK_SIZE = 0x4000;
	const size_t OFFSET = bank * BANK_SIZE;

	printf("bank_load: loading bank %d.\n", bank);
	rom.bank[bank] = (byte*)malloc(BANK_SIZE);
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

	// Make sure no transaction is running
	odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD);

	// Load the 16K page
	if (fseek(fpRomFile, OFFSET, SEEK_SET))
	{
		RG_PANIC("ROM fseek failed");
	}

	if (fread(rom.bank[bank], BANK_SIZE, 1, fpRomFile) < 1)
	{
		RG_PANIC("ROM fread failed");
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
		emu_die("ROM fopen failed");
	}

	rom_loadbank(0);

	byte *header = rom.bank[0];

	memcpy(rom.name, header + 0x0134, 16);
	rom.name[16] = 0;

	int tmp = *((int*)(header + 0x0140));
	byte c = tmp >> 24;
	hw.cgb = ((c == 0x80) || (c == 0xc0));
	hw.gba = (hw.cgb && 0);

	tmp = *((int*)(header + 0x0144));
	c = (tmp >> 24) & 0xff;
	mbc.type = mbc_table[c];
	mbc.batt = batt_table[c];
	mbc.rtc = rtc_table[c];

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
		case MBC_RUMBLE: mbcName = "MBC_RUMBLE"; break;
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

	// RAYMAN stutters too much if we don't fully preload it
	if (strncmp(rom.name, "RAYMAN", 6) == 0)
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
		enable_window_offset_hack = 1;
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
		rtc_load(f);
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
		rtc_save(f);
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
		int i;
		byte* buf = malloc(4096);
		if (!buf) abort();

		un32 (*header)[2] = (un32 (*)[2])buf;
		un32 d = 0;
		int irl = hw.cgb ? 8 : 2;
		int vrl = hw.cgb ? 4 : 2;
		int srl = mbc.ramsize << 1;

		ver = 0x106;
		iramblock = 1;
		vramblock = 1+irl;
		sramblock = 1+irl+vrl;
		wavofs = 4096 - 784;
		hiofs = 4096 - 768;
		palofs = 4096 - 512;
		oamofs = 4096 - 256;
		memset(buf, 0, 4096);

		for (i = 0; svars[i].len > 0; i++)
		{
			header[i][0] = *(un32 *)svars[i].key;
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
			header[i][1] = LIL(d);
		}
		header[i][0] = header[i][1] = 0;

		memcpy(buf+hiofs, ram.hi, sizeof ram.hi);
		memcpy(buf+palofs, lcd.pal, sizeof lcd.pal);
		memcpy(buf+oamofs, lcd.oam.mem, sizeof lcd.oam);
		memcpy(buf+wavofs, snd.wave, sizeof snd.wave);

		fseek(f, 0, SEEK_SET);
		fwrite(buf, 4096, 1, f);

		fseek(f, iramblock<<12, SEEK_SET);
		fwrite(ram.ibank, 4096, irl, f);

		fseek(f, vramblock<<12, SEEK_SET);
		fwrite(lcd.vbank, 4096, vrl, f);

		fseek(f, sramblock<<12, SEEK_SET);

		byte* tmp = (byte*)ram.sbank;
		for (int j = 0; j < srl; ++j)
		{
			memcpy(buf, (void*)tmp, 4096);
			size_t count = fwrite(buf, 4096, 1, f);
			printf("state_save: wrote sram addr=%p, size=0x%x, count=%d\n", (void*)tmp, 4096, count);
			tmp += 4096;
		}

		free(buf);
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
		int i, j;
		byte* buf = malloc(4096);
		if (!buf) abort();

		un32 (*header)[2] = (un32 (*)[2])buf;
		un32 d;
		int irl = hw.cgb ? 8 : 2;
		int vrl = hw.cgb ? 4 : 2;
		int srl = mbc.ramsize << 1;

		ver = hiofs = palofs = oamofs = wavofs = 0;

		fseek(f, 0, SEEK_SET);
		fread(buf, 4096, 1, f);

		for (j = 0; header[j][0]; j++)
		{
			for (i = 0; svars[i].ptr; i++)
			{
				if (header[j][0] != *(un32 *)svars[i].key)
					continue;
				d = LIL(header[j][1]);
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
				break;
			}
		}

		if (hiofs) memcpy(ram.hi, buf+hiofs, sizeof ram.hi);
		if (palofs) memcpy(lcd.pal, buf+palofs, sizeof lcd.pal);
		if (oamofs) memcpy(lcd.oam.mem, buf+oamofs, sizeof lcd.oam);

		if (wavofs) memcpy(snd.wave, buf+wavofs, sizeof snd.wave);
		else memcpy(snd.wave, ram.hi+0x30, 16); /* patch data from older files */

		iramblock = 1;
		vramblock = 1+irl;
		sramblock = 1+irl+vrl;

		fseek(f, iramblock<<12, SEEK_SET);
		fread(ram.ibank, 4096, irl, f);

		fseek(f, vramblock<<12, SEEK_SET);
		fread(lcd.vbank, 4096, vrl, f);

		fseek(f, sramblock<<12, SEEK_SET);

		size_t count = fread(ram.sbank, 4096, srl, f);

		printf("state_load: read sram addr=%p, size=0x%x, count=%d\n", (void*)ram.sbank, 4096 * srl, count);

		free(buf);
		fclose(f);
		pal_dirty();
		sound_dirty();
		mem_updatemap();
		return 0;
	}

	return -1;
}


void loader_unload()
{
	sram_save();
	if (romfile) free(romfile);
	if (sramfile) free(sramfile);
	if (ram.sbank) free(ram.sbank);

	for (int i = 0; i < 512; i++) {
		if (rom.bank[i]) {
			free(rom.bank[i]);
			rom.bank[i] = NULL;
		}
	}

	mbc.type = mbc.romsize = mbc.ramsize = mbc.batt = mbc.rtc = 0;
	romfile = sramfile = NULL;
	// ram.sbank = NULL;
}


void loader_init(char *s)
{
	romfile  = odroid_system_get_path(ODROID_PATH_ROM_FILE, 0);
	sramfile = odroid_system_get_path(ODROID_PATH_SAVE_SRAM, 0);

	rom_load();
	// sram_load();
}
