#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
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

static const char *mbc_names[16] = {
	"MBC_NONE", "MBC_MBC1", "MBC_MBC2", "MBC_MBC3",
	"MBC_MBC5", "MBC_MBC6", "MBC_MBC7", "MBC_HUC1",
	"MBC_HUC3", "MBC_MMM01", "INVALID", "INVALID",
	"INVALID", "INVALID", "INVALID", "INVALID",
};

static FILE* fpRomFile = NULL;
static FILE *fpSramFile = NULL;

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

typedef struct
{
	void *ptr;
	int len;
} sblock_t;

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


int rom_loadbank(int bank)
{
	const size_t BANK_SIZE = 0x4000;
	const size_t OFFSET = bank * BANK_SIZE;

	if (rom.bank[bank])
	{
		MESSAGE_INFO("bank %d already loaded!\n", bank);
		free(rom.bank[bank]);
		// return 0;
	}

	MESSAGE_INFO("loading bank %d.\n", bank);
	rom.bank[bank] = (byte*)malloc(BANK_SIZE);
	while (!rom.bank[bank])
	{
		int i = rand() & 0xFF;
		if (rom.bank[i])
		{
			MESSAGE_INFO("reclaiming bank %d.\n", i);
			rom.bank[bank] = rom.bank[i];
			rom.bank[i] = NULL;
			break;
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
    MESSAGE_INFO("Loading file: '%s'\n", file);

	fpRomFile = fopen(file, "rb");
	if (fpRomFile == NULL)
	{
		emu_die("ROM fopen failed");
	}

	rom_loadbank(0);

	byte *header = rom.bank[0];

	int type = header[0x0147];
	int romsize = header[0x0148];
	int ramsize = header[0x0149];

	hw.cgb = (header[0x0143] == 0x80 || header[0x0143] == 0xC0);

	memcpy(&rom.checksum, header + 0x014E, 2);
	memcpy(&rom.name, header + 0x0134, 16);
	rom.name[16] = 0;

	mbc.batt = (type == 3 || type == 6 || type == 9 || type == 13 || type == 15 ||
				type == 16 || type == 19 || type == 27 || type == 30 || type == 255);
	mbc.rtc  = (type == 15 || type == 16);
	mbc.rumble = (type == 28 || type == 29 || type == 30);
	mbc.sensor = (type == 34);

	if (type >= 1 && type <= 3)
		mbc.type = MBC_MBC1;
	else if (type >= 5 && type <= 6)
		mbc.type = MBC_MBC2;
	else if (type >= 11 && type <= 13)
		mbc.type = MBC_MMM01;
	else if (type >= 15 && type <= 19)
		mbc.type = MBC_MBC3;
	else if (type >= 25 && type <= 30)
		mbc.type = MBC_MBC5;
	else if (type == 32)
		mbc.type = MBC_MBC6;
	else if (type == 34)
		mbc.type = MBC_MBC7;
	else if (type == 254)
		mbc.type = MBC_HUC3;
	else if (type == 255)
		mbc.type = MBC_HUC1;
	else
		mbc.type = MBC_NONE;

	if (romsize < 9)
	{
		mbc.romsize = (2 << romsize);
	}
	else if (romsize > 0x51 && romsize < 0x55)
	{
		mbc.romsize = 128; // (2 << romsize) + 64;
	}
	else
	{
		emu_die("Invalid ROM size: %d\n", romsize);
	}

	if (ramsize < 6)
	{
		const byte ramsize_table[] = {1, 1, 1, 4, 16, 8};
		mbc.ramsize = ramsize_table[ramsize];
	}
	else
	{
		MESSAGE_ERROR("Invalid RAM size: %d\n", ramsize);
		mbc.ramsize = 1;
	}

	ram.sram = malloc(8192 * mbc.ramsize);

	if (!ram.sram)
	{
		emu_die("SRAM alloc failed");
	}

	MESSAGE_INFO("Cart loaded: name='%s', cgb=%d, mbc=%s, romsize=%dK, ramsize=%dK\n",
		rom.name, hw.cgb, mbc_names[mbc.type], mbc.romsize * 16, mbc.ramsize * 8);

	// Gameboy color games can be very large so we only load 1024K for faster boot
	// Also 4/8MB games do not fully fit, our bank manager takes care of swapping.

	int preload = mbc.romsize < 64 ? mbc.romsize : 64;

	if (strncmp(rom.name, "RAYMAN", 6) == 0 || strncmp(rom.name, "NONAME", 6) == 0)
	{
		MESSAGE_INFO("Special preloading for Rayman 1/2\n");
		preload = mbc.romsize - 8;
	}

	MESSAGE_INFO("Preloading the first %d banks\n", preload);
	for (int i = 1; i < preload; i++)
	{
		rom_loadbank(i);
	}

	// Apply game-specific hacks
	if (strncmp(rom.name, "SIREN GB2 ", 11) == 0 || strncmp(rom.name, "DONKEY KONG", 11) == 0)
	{
		MESSAGE_INFO("HACK: Window offset hack enabled\n");
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
	ram.sbank = NULL;

	if (fpRomFile)
	{
		fclose(fpRomFile);
		fpRomFile = NULL;
	}

	if (fpSramFile)
	{
		fclose(fpSramFile);
		fpSramFile = NULL;
	}

	memset(&mbc, 0, sizeof(mbc));
	memset(&rom, 0, sizeof(rom));
}


int sram_load(const char *file)
{
	int ret = -1;
	FILE *f;

	if (!mbc.batt || !mbc.ramsize || !file || !*file)
		return -1;

	rg_spi_lock_acquire(SPI_LOCK_SDCARD);

	if ((f = fopen(file, "rb")))
	{
		MESSAGE_INFO("Loading SRAM from '%s'\n", file);
		if (fread(ram.sbank, 8192, mbc.ramsize, f))
		{
			ram.sram_dirty = 0;
			rtc_load(f);
			ret = 0;
		}
		fclose(f);
	}

	rg_spi_lock_release(SPI_LOCK_SDCARD);
	return ret;
}


int sram_save(const char *file)
{
	int ret = -1;
	FILE *f;

	if (!mbc.batt || !mbc.ramsize || !file || !*file)
		return -1;

	rg_spi_lock_acquire(SPI_LOCK_SDCARD);

	if ((f = fopen(file, "wb")))
	{
		MESSAGE_INFO("Saving SRAM to '%s'\n", file);
		if (fwrite(ram.sbank, 8192, mbc.ramsize, f))
		{
			ram.sram_dirty = 0;
			rtc_save(f);
			ret = 0;
		}
		fclose(f);
	}

	rg_spi_lock_release(SPI_LOCK_SDCARD);
	return ret;
}


int sram_update(const char *file)
{
	if (!mbc.batt || !mbc.ramsize || !file || !*file)
		return -1;

	rg_spi_lock_acquire(SPI_LOCK_SDCARD);

	FILE *fp = fopen(file, "wb");
	if (!fp)
	{
		MESSAGE_ERROR("Unable to open SRAM file: %s", file);
		goto _cleanup;
	}

	for (int pos = 0; pos < (mbc.ramsize * 8192); pos += SRAM_SECTOR_SIZE)
	{
		int sector = pos / SRAM_SECTOR_SIZE;

		if (ram.sram_dirty_sector[sector])
		{
			// MESSAGE_INFO("Writing sram sector #%d @ %ld\n", sector, pos);

			if (fseek(fp, pos, SEEK_SET) != 0)
			{
				MESSAGE_ERROR("Failed to seek sram sector #%d\n", sector);
				goto _cleanup;
			}

			if (fwrite(&ram.sram[pos], SRAM_SECTOR_SIZE, 1, fp) != 1)
			{
				MESSAGE_ERROR("Failed to write sram sector #%d\n", sector);
				goto _cleanup;
			}

			ram.sram_dirty_sector[sector] = 0;
		}
	}

	if (fseek(fp, mbc.ramsize * 8192, SEEK_SET) == 0)
	{
		rtc_save(fp);
	}

_cleanup:
	// Keeping the file open between calls is dangerous unfortunately

	// if (mbc.romsize < 64)
	// {
	// 	fflush(fpSramFile);
	// }
	// else

	if (fclose(fp) == 0)
	{
		ram.sram_dirty = 0;
	}

	rg_spi_lock_release(SPI_LOCK_SDCARD);

	return 0;
}


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

int state_save(const char *file)
{
	byte *buf = calloc(1, 4096);
	if (!buf) return -2;

	FILE *fp = fopen(file, "wb");
	if (!fp) goto _error;

	sblock_t blocks[] = {
		{buf, 1},
		{ram.ibank, hw.cgb ? 8 : 2},
		{lcd.vbank, hw.cgb ? 4 : 2},
		{ram.sbank, mbc.ramsize * 2},
		{NULL, 0},
	};

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

	for (int i = 0; blocks[i].ptr != NULL; i++)
	{
		if (fwrite(blocks[i].ptr, 4096, blocks[i].len, fp) < 1)
		{
			MESSAGE_ERROR("Write error in block %d\n", i);
			goto _error;
		}
	}

	fclose(fp);
	free(buf);

	return 0;

_error:
	if (fp) fclose(fp);
	if (buf) free(buf);

	return -1;
}


int state_load(const char *file)
{
	byte* buf = calloc(1, 4096);
	if (!buf) return -2;

	FILE *fp = fopen(file, "rb");
	if (!fp) goto _error;

	sblock_t blocks[] = {
		{buf, 1},
		{ram.ibank, hw.cgb ? 8 : 2},
		{lcd.vbank, hw.cgb ? 4 : 2},
		{ram.sbank, mbc.ramsize * 2},
		{NULL, 0},
	};

	for (int i = 0; blocks[i].ptr != NULL; i++)
	{
		if (fread(blocks[i].ptr, 4096, blocks[i].len, fp) < 1)
		{
			MESSAGE_ERROR("Read error in block %d\n", i);
			goto _error;
		}
	}

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

	if (ver != SAVE_VERSION)
		MESSAGE_ERROR("Save file version mismatch!\n");

	memcpy(ram.hi, buf+hiofs, sizeof ram.hi);
	memcpy(lcd.pal, buf+palofs, sizeof lcd.pal);
	memcpy(lcd.oam.mem, buf+oamofs, sizeof lcd.oam);
	memcpy(snd.wave, buf+wavofs, sizeof snd.wave);

	fclose(fp);
	free(buf);

	pal_dirty();
	sound_dirty();
	mem_updatemap();

	return 0;

_error:
	if (fp) fclose(fp);
	if (buf) free(buf);

	return -1;
}
