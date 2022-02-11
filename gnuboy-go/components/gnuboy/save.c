#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gnuboy.h"
#include "hw.h"
#include "lcd.h"
#include "cpu.h"
#include "sound.h"

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

// This is the format used by VBA, maybe others
typedef struct
{
	un32 s, m, h, d, flags;
	un32 regs[5];
	un64 rt;
} srtc_t;

static un32 ver;

static const svar_t svars[] =
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
	I4("hdma", &hw.hdma),
	I4("seri", &hw.serial),

	I4("mbcm", &cart.bankmode),
	I4("romb", &cart.rombank),
	I4("ramb", &cart.rambank),
	I4("enab", &cart.enableram),

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

// Set in the far future for VBA-M support
#define RTC_BASE 1893456000


int sram_load(const char *file)
{
	if (!cart.has_battery || !cart.ramsize || !file || !*file)
		return -1;

	FILE *f = fopen(file, "rb");
	if (!f)
		return -2;

	MESSAGE_INFO("Loading SRAM from '%s'\n", file);

	cart.sram_dirty = 0;
	cart.sram_saved = 0;

	for (int i = 0; i < cart.ramsize; i++)
	{
		if (fseek(f, i * 8192, SEEK_SET) == 0 && fread(cart.rambanks[i], 8192, 1, f) == 1)
		{
			MESSAGE_INFO("Loaded SRAM bank %d.\n", i);
			cart.sram_saved = (1 << i);
		}
	}

	if (cart.has_rtc)
	{
		srtc_t rtc_buf;

		if (fseek(f, cart.ramsize * 8192, SEEK_SET) == 0 && fread(&rtc_buf, 48, 1, f) == 1)
		{
			rtc.s = rtc_buf.s;
			rtc.m = rtc_buf.m;
			rtc.h = rtc_buf.h;
			rtc.d = rtc_buf.d;
			rtc.flags = rtc_buf.flags;
			rtc.regs[0] = rtc_buf.regs[0];
			rtc.regs[1] = rtc_buf.regs[1];
			rtc.regs[2] = rtc_buf.regs[2];
			rtc.regs[3] = rtc_buf.regs[3];
			rtc.regs[4] = rtc_buf.regs[4];
			MESSAGE_INFO("Loaded RTC section %03d %02d:%02d:%02d.\n", rtc.d, rtc.h, rtc.m, rtc.s);
		}
	}

	fclose(f);

	return cart.sram_saved ? 0 : -1;
}


/**
 * If quick_save is set to true, sram_save will only save the sectors that
 * changed + the RTC. If set to false then a full sram file is created.
 */
int sram_save(const char *file, bool quick_save)
{
	if (!cart.has_battery || !cart.ramsize || !file || !*file)
		return -1;

	FILE *f = fopen(file, "wb");
	if (!f)
		return -2;

	MESSAGE_INFO("Saving SRAM to '%s'...\n", file);

	// Mark everything as dirty and unsaved (do a full save)
	if (!quick_save)
	{
		cart.sram_dirty = (1 << cart.ramsize) - 1;
		cart.sram_saved = 0;
	}

	for (int i = 0; i < cart.ramsize; i++)
	{
		if (!(cart.sram_saved & (1 << i)) || (cart.sram_dirty & (1 << i)))
		{
			if (fseek(f, i * 8192, SEEK_SET) == 0 && fwrite(cart.rambanks[i], 8192, 1, f) == 1)
			{
				MESSAGE_INFO("Saved SRAM bank %d.\n", i);
				cart.sram_dirty &= ~(1 << i);
				cart.sram_saved |= (1 << i);
			}
		}
	}

	if (cart.has_rtc)
	{
		srtc_t rtc_buf = {
			rtc.s, rtc.m, rtc.h, rtc.d, rtc.flags,
			rtc.regs[0], rtc.regs[1], rtc.regs[2],
			rtc.regs[3], rtc.regs[4],
			RTC_BASE + rtc.s + (rtc.m * 60) + (rtc.h * 3600) + (rtc.d * 86400),
		};
		if (fseek(f, cart.ramsize * 8192, SEEK_SET) == 0 && fwrite(&rtc_buf, 48, 1, f) == 1)
		{
			MESSAGE_INFO("Saved RTC section.\n");
		}
	}

	fclose(f);

	return cart.sram_dirty ? -1 : 0;
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

	bool is_cgb = hw.hwtype == GB_HW_CGB;

	sblock_t blocks[] = {
		{buf, 1},
		{hw.rambanks, is_cgb ? 8 : 2},
		{lcd.vbank, is_cgb ? 4 : 2},
		{cart.rambanks, cart.ramsize * 2},
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

	memcpy(buf+hiofs, hw.ioregs, sizeof hw.ioregs);
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

	bool is_cgb = hw.hwtype == GB_HW_CGB;

	sblock_t blocks[] = {
		{buf, 1},
		{hw.rambanks, is_cgb ? 8 : 2},
		{lcd.vbank, is_cgb ? 4 : 2},
		{cart.rambanks, cart.ramsize * 2},
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

	memcpy(hw.ioregs, buf+hiofs, sizeof hw.ioregs);
	memcpy(lcd.pal, buf+palofs, sizeof lcd.pal);
	memcpy(lcd.oam.mem, buf+oamofs, sizeof lcd.oam);
	memcpy(snd.wave, buf+wavofs, sizeof snd.wave);

	fclose(fp);
	free(buf);

	// Disable BIOS. This is a hack to support old saves
	R_BIOS = 0x1;

	// Older saves might overflow this
	cart.rambank &= (cart.ramsize - 1);

	lcd_rebuildpal();
	sound_dirty();
	hw_updatemap();

	return 0;

_error:
	if (fp) fclose(fp);
	if (buf) free(buf);

	return -1;
}
