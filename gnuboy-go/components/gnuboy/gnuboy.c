#include <sys/param.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "gnuboy.h"
#include "hw.h"
#include "cpu.h"
#include "sound.h"
#include "lcd.h"

static void (*vblank_callback)(void);

int gnuboy_init(int samplerate, bool stereo, int pixformat, void *vblank_func)
{
	sound_init(samplerate, stereo);
	lcd.out.format = pixformat;
	vblank_callback = vblank_func;
	// gnuboy_reset(true);
	return 0;
}


/*
 * gnuboy_reset is called to initialize the state of the emulated
 * system. It should set cpu registers, hardware registers, etc. to
 * their appropriate values at power up time.
 */
void gnuboy_reset(bool hard)
{
	hw_reset(hard);
	lcd_reset(hard);
	cpu_reset(hard);
	sound_reset(hard);
}


/*
	Time intervals throughout the code, unless otherwise noted, are
	specified in double-speed machine cycles (2MHz), each unit
	roughly corresponds to 0.477us.

	For CPU each cycle takes 2dsc (0.954us) in single-speed mode
	and 1dsc (0.477us) in double speed mode.

	Although hardware gbc LCDC would operate at completely different
	and fixed frequency, for emulation purposes timings for it are
	also specified in double-speed cycles.

	line = 228 dsc (109us)
	frame (154 lines) = 35112 dsc (16.7ms)
	of which
		visible lines x144 = 32832 dsc (15.66ms)
		vblank lines x10 = 2280 dsc (1.08ms)
*/
void gnuboy_run(bool draw)
{
	lcd.out.enabled = draw;
	snd.output.pos = 0;

	/* FIXME: judging by the time specified this was intended
	to emulate through vblank phase which is handled at the
	end of the loop. */
	cpu_emulate(2280);

	/* FIXME: R_LY >= 0; comparsion to zero can also be removed
	altogether, R_LY is always 0 at this point */
	while (R_LY > 0 && R_LY < 144) {
		/* Step through visible line scanning phase */
		cpu_emulate(lcd.cycles);
	}

	/* VBLANK BEGIN */
	if (draw && vblank_callback) {
		(vblank_callback)();
	}

	hw_vblank();

	if (!(R_LCDC & 0x80)) {
		/* LCDC operation stopped */
		/* FIXME: judging by the time specified, this is
		intended to emulate through visible line scanning
		phase, even though we are already at vblank here */
		cpu_emulate(32832);
	}

	while (R_LY > 0) {
		/* Step through vblank phase */
		cpu_emulate(lcd.cycles);
	}
}


void gnuboy_set_pad(uint pad)
{
	if (hw.pad != pad)
	{
		hw_setpad(pad);
	}
}


void gnuboy_die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	abort();
}


int gnuboy_load_bios(const char *file)
{
	MESSAGE_INFO("Loading BIOS file: '%s'\n", file);

	FILE *fp = fopen(file, "rb");
	if (fp == NULL)
	{
		MESSAGE_ERROR("BIOS fopen failed");
		return -1;
	}

	hw.bios = malloc(0x900);
	if (!hw.bios)
		gnuboy_die("Out of memory");

	if (fread(hw.bios, 1, 0x900, fp) >= 0x100)
	{
		MESSAGE_INFO("BIOS loaded\n");
	}

	fclose(fp);

	return 0;
}


void gnuboy_load_bank(int bank)
{
	const size_t BANK_SIZE = 0x4000;
	const size_t OFFSET = bank * BANK_SIZE;

	if (!cart.rombanks[bank])
		cart.rombanks[bank] = malloc(BANK_SIZE);

	if (!cart.romFile)
		return;

	MESSAGE_INFO("loading bank %d.\n", bank);
	while (!cart.rombanks[bank])
	{
		int i = rand() & 0xFF;
		if (cart.rombanks[i])
		{
			MESSAGE_INFO("reclaiming bank %d.\n", i);
			cart.rombanks[bank] = cart.rombanks[i];
			cart.rombanks[i] = NULL;
			break;
		}
	}

	// Load the 16K page
	if (fseek(cart.romFile, OFFSET, SEEK_SET)
		|| !fread(cart.rombanks[bank], BANK_SIZE, 1, cart.romFile))
	{
		if (feof(cart.romFile))
			MESSAGE_ERROR("End of file reached, the cart's header is probably incorrect!\n");
		else
			gnuboy_die("ROM bank loading failed");
	}
}


int gnuboy_load_rom(const char *file)
{
	// Memory Bank Controller names
	const char *mbc_names[16] = {
		"MBC_NONE", "MBC_MBC1", "MBC_MBC2", "MBC_MBC3",
		"MBC_MBC5", "MBC_MBC6", "MBC_MBC7", "MBC_HUC1",
		"MBC_HUC3", "MBC_MMM01", "INVALID", "INVALID",
		"INVALID", "INVALID", "INVALID", "INVALID",
	};
	const char *hw_types[] = {
		"DMG", "CGB", "SGB", "AGB", "???"
	};

	MESSAGE_INFO("Loading file: '%s'\n", file);

	byte header[0x200];

	cart.romFile = fopen(file, "rb");
	if (cart.romFile == NULL)
	{
		gnuboy_die("ROM fopen failed");
	}

	if (fread(&header, 0x200, 1, cart.romFile) != 1)
	{
		gnuboy_die("ROM fread failed");
	}

	int type = header[0x0147];
	int romsize = header[0x0148];
	int ramsize = header[0x0149];

	if (header[0x0143] == 0x80 || header[0x0143] == 0xC0)
		hw.hwtype = GB_HW_CGB; // Game supports CGB mode so we go for that
	else if (header[0x0146] == 0x03)
		hw.hwtype = GB_HW_SGB; // Game supports SGB features
	else
		hw.hwtype = GB_HW_DMG; // Games supports DMG only

	memcpy(&cart.checksum, header + 0x014E, 2);
	memcpy(&cart.name, header + 0x0134, 16);
	cart.name[16] = 0;

	cart.has_battery = (type == 3 || type == 6 || type == 9 || type == 13 || type == 15 ||
						type == 16 || type == 19 || type == 27 || type == 30 || type == 255);
	cart.has_rtc  = (type == 15 || type == 16);
	cart.has_rumble = (type == 28 || type == 29 || type == 30);
	cart.has_sensor = (type == 34);
	cart.colorize = 0;

	if (type >= 1 && type <= 3)
		cart.mbc = MBC_MBC1;
	else if (type >= 5 && type <= 6)
		cart.mbc = MBC_MBC2;
	else if (type >= 11 && type <= 13)
		cart.mbc = MBC_MMM01;
	else if (type >= 15 && type <= 19)
		cart.mbc = MBC_MBC3;
	else if (type >= 25 && type <= 30)
		cart.mbc = MBC_MBC5;
	else if (type == 32)
		cart.mbc = MBC_MBC6;
	else if (type == 34)
		cart.mbc = MBC_MBC7;
	else if (type == 254)
		cart.mbc = MBC_HUC3;
	else if (type == 255)
		cart.mbc = MBC_HUC1;
	else
		cart.mbc = MBC_NONE;

	if (romsize < 9)
	{
		cart.romsize = (2 << romsize);
	}
	else if (romsize > 0x51 && romsize < 0x55)
	{
		cart.romsize = 128; // (2 << romsize) + 64;
	}
	else
	{
		gnuboy_die("Invalid ROM size: %d\n", romsize);
	}

	if (ramsize < 6)
	{
		const byte ramsize_table[] = {1, 1, 1, 4, 16, 8};
		cart.ramsize = ramsize_table[ramsize];
	}
	else
	{
		MESSAGE_ERROR("Invalid RAM size: %d\n", ramsize);
		cart.ramsize = 1;
	}

	cart.rambanks = malloc(8192 * cart.ramsize);
	if (!cart.rambanks)
	{
		gnuboy_die("SRAM alloc failed");
	}

	// Detect colorization palette that the real GBC would be using
	if (hw.hwtype != GB_HW_CGB)
	{
		//
		// The following algorithm was adapted from visualboyadvance-m at
		// https://github.com/visualboyadvance-m/visualboyadvance-m/blob/master/src/gb/GB.cpp
		//

		// Title checksums that are treated specially by the CGB boot ROM
		const uint8_t col_checksum[79] = {
			0x00, 0x88, 0x16, 0x36, 0xD1, 0xDB, 0xF2, 0x3C, 0x8C, 0x92, 0x3D, 0x5C,
			0x58, 0xC9, 0x3E, 0x70, 0x1D, 0x59, 0x69, 0x19, 0x35, 0xA8, 0x14, 0xAA,
			0x75, 0x95, 0x99, 0x34, 0x6F, 0x15, 0xFF, 0x97, 0x4B, 0x90, 0x17, 0x10,
			0x39, 0xF7, 0xF6, 0xA2, 0x49, 0x4E, 0x43, 0x68, 0xE0, 0x8B, 0xF0, 0xCE,
			0x0C, 0x29, 0xE8, 0xB7, 0x86, 0x9A, 0x52, 0x01, 0x9D, 0x71, 0x9C, 0xBD,
			0x5D, 0x6D, 0x67, 0x3F, 0x6B, 0xB3, 0x46, 0x28, 0xA5, 0xC6, 0xD3, 0x27,
			0x61, 0x18, 0x66, 0x6A, 0xBF, 0x0D, 0xF4
		};

		// The fourth character of the game title for disambiguation on collision.
		const uint8_t col_disambig_chars[29] = {
			'B', 'E', 'F', 'A', 'A', 'R', 'B', 'E',
			'K', 'E', 'K', ' ', 'R', '-', 'U', 'R',
			'A', 'R', ' ', 'I', 'N', 'A', 'I', 'L',
			'I', 'C', 'E', ' ', 'R'
		};

		// Palette ID | (Flags << 5)
		const uint8_t col_palette_info[94] = {
			0x7C, 0x08, 0x12, 0xA3, 0xA2, 0x07, 0x87, 0x4B, 0x20, 0x12, 0x65, 0xA8,
			0x16, 0xA9, 0x86, 0xB1, 0x68, 0xA0, 0x87, 0x66, 0x12, 0xA1, 0x30, 0x3C,
			0x12, 0x85, 0x12, 0x64, 0x1B, 0x07, 0x06, 0x6F, 0x6E, 0x6E, 0xAE, 0xAF,
			0x6F, 0xB2, 0xAF, 0xB2, 0xA8, 0xAB, 0x6F, 0xAF, 0x86, 0xAE, 0xA2, 0xA2,
			0x12, 0xAF, 0x13, 0x12, 0xA1, 0x6E, 0xAF, 0xAF, 0xAD, 0x06, 0x4C, 0x6E,
			0xAF, 0xAF, 0x12, 0x7C, 0xAC, 0xA8, 0x6A, 0x6E, 0x13, 0xA0, 0x2D, 0xA8,
			0x2B, 0xAC, 0x64, 0xAC, 0x6D, 0x87, 0xBC, 0x60, 0xB4, 0x13, 0x72, 0x7C,
			0xB5, 0xAE, 0xAE, 0x7C, 0x7C, 0x65, 0xA2, 0x6C, 0x64, 0x85
		};

		uint8_t infoIdx = 0;
		uint8_t checksum = 0;

		// Calculate the checksum over 16 title bytes.
		for (int i = 0; i < 16; i++)
		{
			checksum += header[0x0134 + i];
		}

		// Check if the checksum is in the list.
		for (size_t idx = 0; idx < 79; idx++)
		{
			if (col_checksum[idx] == checksum)
			{
				infoIdx = idx;

				// Indexes above 0x40 have to be disambiguated.
				if (idx <= 0x40)
					break;

				// No idea how that works. But it works.
				for (size_t i = idx - 0x41, j = 0; i < 29; i += 14, j += 14) {
					if (header[0x0137] == col_disambig_chars[i]) {
						infoIdx += j;
						break;
					}
				}
				break;
			}
		}

		cart.colorize = col_palette_info[infoIdx];
	}

	MESSAGE_INFO("Cart loaded: name='%s', hw=%s, mbc=%s, romsize=%dK, ramsize=%dK, colorize=%d\n",
		cart.name, hw_types[hw.hwtype], mbc_names[cart.mbc], cart.romsize * 16, cart.ramsize * 8, cart.colorize);

	// Gameboy color games can be very large so we only load 1024K for faster boot
	// Also 4/8MB games do not fully fit, our bank manager takes care of swapping.

	int preload = cart.romsize < 64 ? cart.romsize : 64;

	if (cart.romsize > 64 && (strncmp(cart.name, "RAYMAN", 6) == 0 || strncmp(cart.name, "NONAME", 6) == 0))
	{
		MESSAGE_INFO("Special preloading for Rayman 1/2\n");
		preload = cart.romsize - 40;
	}

	MESSAGE_INFO("Preloading the first %d banks\n", preload);
	for (int i = 0; i < preload; i++)
	{
		gnuboy_load_bank(i);
	}

	// Apply game-specific hacks
	if (strncmp(cart.name, "SIREN GB2 ", 11) == 0 || strncmp(cart.name, "DONKEY KONG", 11) == 0)
	{
		MESSAGE_INFO("HACK: Window offset hack enabled\n");
		lcd.enable_window_offset_hack = 1;
	}

	return 0;
}


void gnuboy_free_rom(void)
{
	for (int i = 0; i < 512; i++)
	{
		if (cart.rombanks[i]) {
			free(cart.rombanks[i]);
			cart.rombanks[i] = NULL;
		}
	}
	free(cart.rambanks);
	cart.rambanks = NULL;

	if (cart.romFile)
	{
		fclose(cart.romFile);
		cart.romFile = NULL;
	}

	if (cart.sramFile)
	{
		fclose(cart.sramFile);
		cart.sramFile = NULL;
	}

	memset(&cart, 0, sizeof(cart));
}


void gnuboy_get_time(int *day, int *hour, int *minute, int *second)
{
	if (day) *day = rtc.d;
	if (hour) *hour = rtc.h;
	if (minute) *minute = rtc.m;
	if (second) *second = rtc.s;
}


void gnuboy_set_time(int day, int hour, int minute, int second)
{
	rtc.d = MIN(MAX(day, 0), 365);
	rtc.h = MIN(MAX(hour, 0), 24);
	rtc.m = MIN(MAX(minute, 0), 60);
	rtc.s = MIN(MAX(second, 0), 60);
	rtc.ticks = 0;
	rtc.dirty = 0;
}


int gnuboy_get_hwtype(void)
{
	return hw.hwtype;
}


void gnuboy_set_hwtype(gb_hwtype_t type)
{
	// nothing for now
}


int gnuboy_get_palette(void)
{
	return lcd.out.colorize;
}


void gnuboy_set_palette(gb_palette_t pal)
{
	if (lcd.out.colorize != pal)
	{
		lcd.out.colorize = pal;
		lcd_rebuildpal();
	}
}


bool gnuboy_sram_dirty(void)
{
	return cart.sram_dirty != 0;
}
