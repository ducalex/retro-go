#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "emu.h"
#include "regs.h"
#include "hw.h"
#include "mem.h"
#include "lcd.h"
#include "cpu.h"

#include "palettes.h"

typedef struct
{
	int pat, x, v, pal, pri;
} vissprite_t;

static int BG[64];
static int WND[64];
static byte BUF[0x100];
static byte PRI[0x100];
static un16 PAL[64];
static vissprite_t VS[16];

static int S; /* tilemap position */
static int T;
static int U; /* position within tile */
static int V;

static int WX;
static int WY;
static int WT;
static int WV;

#define CYCLES (lcd.cycles)

static uint16_t dmg_pal[4][4] = {
	GB_DEFAULT_PALETTE, GB_DEFAULT_PALETTE,
	GB_DEFAULT_PALETTE, GB_DEFAULT_PALETTE,
};
static int dmg_selected_pal = 0;

static byte *vdest;


lcd_t lcd;
fb_t fb;


#define priused(attr) ({un32 *a = (un32*)(attr); (int)((a[0]|a[1]|a[2]|a[3]|a[4]|a[5]|a[6]|a[7])&0x80808080);})

#define blendcpy(dest, src, b, cnt) {					\
	byte *s = (src), *d = (dest), _b = (b), c = (cnt); 	\
	while(c--) *(d + c) = *(s + c) | _b; 				\
}


/**
 * Drawing routines
 */

__attribute__((optimize("unroll-loops")))
static inline byte *get_patpix(int tile, int x)
{
	const byte *vram = lcd.vbank[0];

	static byte pix[8];

	if (tile & (1 << 11)) // Vertical Flip
		vram += ((tile & 0x3FF) << 4) | ((7 - x) << 1);
	else
		vram += ((tile & 0x3FF) << 4) | (x << 1);

	if (tile & (1 << 10)) // Horizontal Flip
		for (int k = 0; k < 8; ++k)
		{
			pix[k] = ((vram[0] >> k) & 1) | (((vram[1] >> k) & 1) << 1);
		}
	else
		for (int k = 0; k < 8; ++k)
		{
			pix[7 - k] = ((vram[0] >> k) & 1) | (((vram[1] >> k) & 1) << 1);
		}

	return pix;
}

static inline void tilebuf()
{
	int cnt, base;
	byte *tilemap, *attrmap;
	int *tilebuf;

	/* Background tiles */

	const int8_t wraptable[64] = {
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,-32
	};
	const int8_t *wrap = wraptable + S;

	base = ((R_LCDC&0x08)?0x1C00:0x1800) + (T<<5) + S;
	tilemap = lcd.vbank[0] + base;
	attrmap = lcd.vbank[1] + base;
	tilebuf = BG;
	cnt = ((WX + 7) >> 3) + 1;

	if (hw.cgb)
	{
		if (R_LCDC & 0x10)
			for (int i = cnt; i > 0; i--)
			{
				*(tilebuf++) = *tilemap
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*attrmap & 0x07) << 2);
				attrmap += *wrap + 1;
				tilemap += *(wrap++) + 1;
			}
		else
			for (int i = cnt; i > 0; i--)
			{
				*(tilebuf++) = (0x100 + ((n8)*tilemap))
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*attrmap & 0x07) << 2);
				attrmap += *wrap + 1;
				tilemap += *(wrap++) + 1;
			}
	}
	else
	{
		if (R_LCDC & 0x10)
			for (int i = cnt; i > 0; i--)
			{
				*(tilebuf++) = *(tilemap++);
				tilemap += *(wrap++);
			}
		else
			for (int i = cnt; i > 0; i--)
			{
				*(tilebuf++) = (0x100 + ((n8)*(tilemap++)));
				tilemap += *(wrap++);
			}
	}

	if (WX >= 160) return;

	/* Window tiles */

	base = ((R_LCDC&0x40)?0x1C00:0x1800) + (WT<<5);
	tilemap = lcd.vbank[0] + base;
	attrmap = lcd.vbank[1] + base;
	tilebuf = WND;
	cnt = ((160 - WX) >> 3) + 1;

	if (hw.cgb)
	{
		if (R_LCDC & 0x10)
			for (int i = cnt; i > 0; i--)
			{
				*(tilebuf++) = *(tilemap++)
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*(attrmap++)&0x7) << 2);
			}
		else
			for (int i = cnt; i > 0; i--)
			{
				*(tilebuf++) = (0x100 + ((n8)*(tilemap++)))
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*(attrmap++)&0x7) << 2);
			}
	}
	else
	{
		if (R_LCDC & 0x10)
			for (int i = cnt; i > 0; i--)
				*(tilebuf++) = *(tilemap++);
		else
			for (int i = cnt; i > 0; i--)
				*(tilebuf++) = (0x100 + ((n8)*(tilemap++)));
	}
}

static inline void bg_scan()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX <= 0) return;

	cnt = WX;
	tile = BG;
	dest = BUF;

	src = get_patpix(*(tile++), V) + U;
	memcpy(dest, src, 8-U);
	dest += 8-U;
	cnt -= 8-U;

	while (cnt >= 0)
	{
		src = get_patpix(*(tile++), V);
		memcpy(dest, src, 8);
		dest += 8;
		cnt -= 8;
	}
}

static inline void wnd_scan()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	cnt = 160 - WX;
	tile = WND;
	dest = BUF + WX;

	while (cnt >= 0)
	{
		src = get_patpix(*(tile++), WV);
		memcpy(dest, src, 8);
		dest += 8;
		cnt -= 8;
	}
}

static inline void bg_scan_pri()
{
	int cnt, i;
	byte *src, *dest;

	if (WX <= 0) return;

	i = S;
	cnt = WX;
	dest = PRI;
	src = lcd.vbank[1] + ((R_LCDC&0x08)?0x1C00:0x1800) + (T<<5);

	if (!priused(src))
	{
		memset(dest, 0, cnt);
		return;
	}

	memset(dest, src[i++&31]&128, 8-U);
	dest += 8-U;
	cnt -= 8-U;

	if (cnt <= 0) return;

	while (cnt >= 8)
	{
		memset(dest, src[i++&31]&128, 8);
		dest += 8;
		cnt -= 8;
	}
	memset(dest, src[i&31]&128, cnt);
}

static inline void wnd_scan_pri()
{
	int cnt, i;
	byte *src, *dest;

	if (WX >= 160) return;

	i = 0;
	cnt = 160 - WX;
	dest = PRI + WX;
	src = lcd.vbank[1] + ((R_LCDC&0x40)?0x1C00:0x1800) + (WT<<5);

	if (!priused(src))
	{
		memset(dest, 0, cnt);
		return;
	}

	while (cnt >= 8)
	{
		memset(dest, src[i++]&128, 8);
		dest += 8;
		cnt -= 8;
	}

	memset(dest, src[i]&128, cnt);
}

static inline void bg_scan_color()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX <= 0) return;

	cnt = WX;
	tile = BG;
	dest = BUF;

	src = get_patpix(*(tile++), V) + U;
	blendcpy(dest, src, *(tile++), 8-U);
	dest += 8-U;
	cnt -= 8-U;

	while (cnt >= 0)
	{
		src = get_patpix(*(tile++), V);
		blendcpy(dest, src, *(tile++), 8);
		dest += 8;
		cnt -= 8;
	}
}

static inline void wnd_scan_color()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX >= 160) return;

	cnt = 160 - WX;
	tile = WND;
	dest = BUF + WX;

	while (cnt >= 0)
	{
		src = get_patpix(*(tile++), WV);
		blendcpy(dest, src, *(tile++), 8);
		dest += 8;
		cnt -= 8;
	}
}

static inline int spr_enum()
{
	if (!(R_LCDC & 0x02))
		return 0;

	vissprite_t ts[10];
	int line = R_LY;
	int NS = 0;

	for (int i = 0; i < 40; ++i)
	{
		obj_t *obj = &lcd.oam.obj[i];
		int v, pat;

		if (line >= obj->y || line + 16 < obj->y)
			continue;
		if (line + 8 >= obj->y && !(R_LCDC & 0x04))
			continue;

		VS[NS].x = (int)obj->x - 8;
		v = line - (int)obj->y + 16;

		if (hw.cgb)
		{
			pat = obj->pat | (((int)obj->flags & 0x60) << 5)
				| (((int)obj->flags & 0x08) << 6);
			VS[NS].pal = 32 + ((obj->flags & 0x07) << 2);
		}
		else
		{
			pat = obj->pat | (((int)obj->flags & 0x60) << 5);
			VS[NS].pal = 32 + ((obj->flags & 0x10) >> 2);
		}

		VS[NS].pri = (obj->flags & 0x80) >> 7;

		if ((R_LCDC & 0x04))
		{
			pat &= ~1;
			if (v >= 8)
			{
				v -= 8;
				pat++;
			}
			if (obj->flags & 0x40) pat ^= 1;
		}

		VS[NS].pat = pat;
		VS[NS].v = v;

		if (++NS == 10) break;
	}

	// Sort sprites
	if (!hw.cgb)
	{
		/* not quite optimal but it finally works! */
		for (int i = 0; i < NS; ++i)
		{
			int l = 0;
			int x = VS[0].x;
			for (int j = 1; j < NS; ++j)
			{
				if (VS[j].x < x)
				{
					l = j;
					x = VS[j].x;
				}
			}
			ts[i] = VS[l];
			VS[l].x = 160;
		}

		memcpy(VS, ts, sizeof(ts));
	}

	return NS;
}

static inline void spr_scan(int ns)
{
	byte *src, *dest, *bg, *pri;
	int i, b, x, pal;
	vissprite_t *vs;
	byte bgdup[256];

	memcpy(bgdup, BUF, 256);

	vs = &VS[ns-1];

	for (; ns; ns--, vs--)
	{
		pal = vs->pal;
		x = vs->x;

		if (x >= 160 || x <= -8)
			continue;

		src = get_patpix(vs->pat, vs->v);
		dest = BUF;

		if (x < 0)
		{
			src -= x;
			i = 8 + x;
		}
		else
		{
			dest += x;
			if (x > 152) i = 160 - x;
			else i = 8;
		}

		if (vs->pri)
		{
			bg = bgdup + (dest - BUF);
			while (i--)
			{
				b = src[i];
				if (b && !(bg[i]&3)) dest[i] = pal|b;
			}
		}
		else if (hw.cgb)
		{
			bg = bgdup + (dest - BUF);
			pri = PRI + (dest - BUF);
			while (i--)
			{
				b = src[i];
				if (b && (!pri[i] || !(bg[i]&3)))
					dest[i] = pal|b;
			}
		}
		else
		{
			while (i--) if (src[i]) dest[i] = pal|src[i];
		}
	}
}

static inline void lcd_beginframe()
{
	vdest = fb.buffer;
	WY = R_WY;
}

void lcd_reset(bool hard)
{
	if (hard)
	{
		memset(&lcd, 0, sizeof(lcd));
	}

	memset(BG, 0, sizeof(BG));
	memset(WND, 0, sizeof(WND));
	memset(BUF, 0, sizeof(BUF));
	memset(PRI, 0, sizeof(PRI));
	memset(PAL, 0, sizeof(PAL));
	memset(VS, 0, sizeof(VS));

	WX = WY = WT = WV = 0;
	S = T = U = V = 0;

	lcd_beginframe();
	pal_set_dmg(dmg_selected_pal);
}

static inline void lcd_renderline()
{
	if (!fb.enabled)
		return;

	if (!(R_LCDC & 0x80))
		return; /* should not happen... */

	int SX, SY, SL, NS;

	SL = R_LY;
	SX = R_SCX;
	SY = (R_SCY + SL) & 0xff;
	S = SX >> 3;
	T = SY >> 3;
	U = SX & 7;
	V = SY & 7;

	WX = R_WX - 7;
	if (WY>SL || WY<0 || WY>143 || WX<-7 || WX>160 || !(R_LCDC&0x20))
		WX = 160;
	WT = (SL - WY) >> 3;
	WV = (SL - WY) & 7;

	// Fix for Fushigi no Dungeon - Fuurai no Shiren GB2 and Donkey Kong
	// This is a hack, the real problem is elsewhere
	if (lcd.enable_window_offset_hack && (R_LCDC & 0x20))
	{
		WT %= 12;
	}

	NS = spr_enum();
	tilebuf();

	if (hw.cgb)
	{
		bg_scan_color();
		wnd_scan_color();
		if (NS)
		{
			bg_scan_pri();
			wnd_scan_pri();
		}
	}
	else
	{
		bg_scan();
		wnd_scan();
		blendcpy(BUF+WX, BUF+WX, 0x04, 160-WX);
	}

	spr_scan(NS);

	if (fb.format == GB_PIXEL_PALETTED)
	{
		memcpy(vdest, BUF, 160);
		vdest += 160;
	}
	else
	{
		un16* dst = (un16*)vdest;

		for (int i = 0; i < 160; ++i)
			dst[i] = PAL[BUF[i]];

		vdest += 160 * 2;
	}
}

static inline void pal_update(byte i)
{
	int low = lcd.pal[i << 1];
	int high = lcd.pal[(i << 1) | 1];

	int c = (low | (high << 8)) & 0x7fff;

	int r = c & 0x1f;         // bit 0-4 red
	int g = (c >> 5) & 0x1f;  // bit 5-9 green
	int b = (c >> 10) & 0x1f; // bit 10-14 blue

	PAL[i] = (r << 11) | (g << (5 + 1)) | (b);

	if (fb.format == GB_PIXEL_565_BE) {
		PAL[i] = (PAL[i] << 8) | (PAL[i] >> 8);
	}
}

static inline void pal_detect_dmg()
{
	const uint16_t *bgp, *obp0, *obp1;
    uint8_t infoIdx = 0;
	uint8_t checksum = 0;

	// Calculate the checksum over 16 title bytes.
	for (int i = 0; i < 16; i++)
	{
		checksum += readb(0x0134 + i);
	}

	// Check if the checksum is in the list.
	for (size_t idx = 0; idx < sizeof(colorization_checksum); idx++)
	{
		if (colorization_checksum[idx] == checksum)
		{
			infoIdx = idx;

			// Indexes above 0x40 have to be disambiguated.
			if (idx > 0x40) {
				// No idea how that works. But it works.
				for (size_t i = idx - 0x41, j = 0; i < sizeof(colorization_disambig_chars); i += 14, j += 14) {
					if (readb(0x0137) == colorization_disambig_chars[i]) {
						infoIdx += j;
						break;
					}
				}
			}
			break;
		}
	}

    uint8_t palette = colorization_palette_info[infoIdx] & 0x1F;
    uint8_t flags = (colorization_palette_info[infoIdx] & 0xE0) >> 5;

	bgp  = dmg_game_palettes[palette][2];
    obp0 = dmg_game_palettes[palette][(flags & 1) ? 0 : 1];
    obp1 = dmg_game_palettes[palette][(flags & 2) ? 0 : 1];

    if (!(flags & 4)) {
        obp1 = dmg_game_palettes[palette][2];
    }

	MESSAGE_INFO("Using GBC palette %d\n", palette);

	memcpy(&dmg_pal[0], bgp, 8); // BGP
	memcpy(&dmg_pal[1], bgp, 8); // BGP
	memcpy(&dmg_pal[2], obp0, 8); // OBP0
	memcpy(&dmg_pal[3], obp1, 8); // OBP1
}

void pal_write(byte i, byte b)
{
	if (lcd.pal[i] == b) return;
	lcd.pal[i] = b;
	pal_update(i >> 1);
}

void pal_write_dmg(byte i, byte mapnum, byte d)
{
	mapnum &= 3;
	for (int j = 0; j < 8; j += 2)
	{
		int c = dmg_pal[mapnum][(d >> j) & 3];
		/* FIXME - handle directly without faking cgb */
		pal_write(i+j, c & 0xff);
		pal_write(i+j+1, c >> 8);
	}
}

void pal_set_dmg(int palette)
{
	dmg_selected_pal = palette % (pal_count_dmg() + 1);

	if (dmg_selected_pal == 0) {
		pal_detect_dmg();
	} else {
		memcpy(&dmg_pal[0], dmg_palettes[dmg_selected_pal - 1], 8); // BGP
		memcpy(&dmg_pal[1], dmg_palettes[dmg_selected_pal - 1], 8); // BGP
		memcpy(&dmg_pal[2], dmg_palettes[dmg_selected_pal - 1], 8); // OBP0
		memcpy(&dmg_pal[3], dmg_palettes[dmg_selected_pal - 1], 8); // OBP1
	}

	pal_dirty();
}

int pal_get_dmg()
{
	return dmg_selected_pal;
}

int pal_count_dmg()
{
	return sizeof(dmg_palettes) / sizeof(dmg_palettes[0]);
}

void pal_dirty()
{
	if (!hw.cgb)
	{
		pal_write_dmg(0, 0, R_BGP);
		pal_write_dmg(8, 1, R_BGP);
		pal_write_dmg(64, 2, R_OBP0);
		pal_write_dmg(72, 3, R_OBP1);
	}

	for (int i = 0; i < 64; i++)
	{
		pal_update(i);
	}
}


/**
 * LCD Controller routines
 */

/*
 * stat_trigger updates the STAT interrupt line to reflect whether any
 * of the conditions set to be tested (by bits 3-6 of R_STAT) are met.
 * This function should be called whenever any of the following occur:
 * 1) LY or LYC changes.
 * 2) A state transition affects the low 2 bits of R_STAT (see below).
 * 3) The program writes to the upper bits of R_STAT.
 * stat_trigger also updates bit 2 of R_STAT to reflect whether LY=LYC.
 */

void stat_trigger()
{
	const byte condbits[4] = { 0x08, 0x10, 0x20, 0x00 };
	int level = 0;

	if (R_LY == R_LYC)
	{
		R_STAT |= 0x04;
		if (R_STAT & 0x40) level = 1;
	}
	else R_STAT &= ~0x04;

	if (R_STAT & condbits[R_STAT&3]) level = 1;

	if (!(R_LCDC & 0x80)) level = 0;

	hw_interrupt(IF_STAT, level);
}


/*
 * stat_change is called when a transition results in a change to the
 * LCD STAT condition (the low 2 bits of R_STAT).  It raises or lowers
 * the VBLANK interrupt line appropriately and calls stat_trigger to
 * update the STAT interrupt line.
 * FIXME: function now will only lower vblank interrupt, description does not match anymore
 */
static void inline stat_change(int stat)
{
	stat &= 3;
	R_STAT = (R_STAT & 0x7C) | stat;

	if (stat != 1) hw_interrupt(IF_VBLANK, 0);
	stat_trigger();
}


void lcdc_change(byte b)
{
	byte old = R_LCDC;
	R_LCDC = b;
	if ((R_LCDC ^ old) & 0x80) /* lcd on/off change */
	{
		R_LY = 0;
		stat_change(2);
		CYCLES = 40;  // Correct value seems to be 38
		lcd_beginframe();
	}
}


/*
	LCD controller operates with 154 lines per frame, of which lines
	#0..#143 are visible and lines #144..#153 are processed in vblank
	state.

	lcd_emulate() performs cyclic switching between lcdc states (OAM
	search/data transfer/hblank/vblank), updates system state and time
	counters accordingly. Control is returned to the caller immediately
	after a step that sets LCDC ahead of CPU, so that LCDC is always
	ahead of CPU by one state change. Once CPU reaches same point in
	time, LCDC is advanced through the next step.

	For each visible line LCDC goes through states 2 (search), 3
	(transfer) and then 0 (hblank). At the first line of vblank LCDC
	is switched to state 1 (vblank) and remains there till line #0 is
	reached (execution is still interrupted after each line so that
	function could return if it ran out of time).

	Irregardless of state switches per line, time spent in each line
	adds up to exactly 228 double-speed cycles (109us).

	LCDC emulation begins with R_LCDC set to "operation enabled", R_LY
	set to line #0 and R_STAT set to state-hblank. lcd.cycles is also
	set to zero, to begin emulation we call lcd_emulate() once to
	force-advance LCD through the first iteration.

	Docs aren't entirely accurate about time intervals within single
	line; besides that, intervals will vary depending on number of
	sprites on the line and probably other factors. States 1, 2 and 3
	do not require precise sub-line CPU-LCDC sync, but state 0 might do.
*/
void lcd_emulate()
{
	/* LCD disabled */
	if (!(R_LCDC & 0x80))
	{
		/* LCDC operation disabled (short route) */
		while (CYCLES <= 0)
		{
			switch (R_STAT & 3)
			{
			case 0: /* hblank */
			case 1: /* vblank */
				// lcd_renderline();
				stat_change(2);
				CYCLES += 40;
				break;
			case 2: /* search */
				stat_change(3);
				CYCLES += 86;
				break;
			case 3: /* transfer */
				stat_change(0);
				/* FIXME: check docs; HDMA might require operating LCDC */
				if (hw.hdma & 0x80)
					hw_hdma();
				else
					CYCLES += 102;
				break;
			}
			return;
		}
	}

	while (CYCLES <= 0)
	{
		switch (R_STAT & 3)
		{
		case 0:
			/* hblank -> */
			if (++R_LY >= 144)
			{
				/* FIXME: pick _one_ place to trigger vblank interrupt
				this better be done here or within stat_change(),
				otherwise CPU will have a chance to run	for some time
				before interrupt is triggered */
				if (cpu.halted)
				{
					hw_interrupt(IF_VBLANK, 1);
					CYCLES += 228;
				}
				else CYCLES += 10;
				stat_change(1); /* -> vblank */
				break;
			}

			// Hack for Worms Armageddon
			if (R_STAT == 0x48)
				hw_interrupt(IF_STAT, 0);

			stat_change(2); /* -> search */
			CYCLES += 40;
			break;
		case 1:
			/* vblank -> */
			if (!(hw.ilines & IF_VBLANK))
			{
				hw_interrupt(IF_VBLANK, 1);
				CYCLES += 218;
				break;
			}
			if (R_LY == 0)
			{
				lcd_beginframe();
				stat_change(2); /* -> search */
				CYCLES += 40;
				break;
			}
			else if (R_LY < 152)
				CYCLES += 228;
			else if (R_LY == 152)
				/* Handling special case on the last line; see
				docs/HACKING */
				CYCLES += 28;
			else
			{
				R_LY = -1;
				CYCLES += 200;
			}
			R_LY++;
			stat_trigger();
			break;
		case 2:
			/* search -> */
			lcd_renderline();
			stat_change(3); /* -> transfer */
			CYCLES += 86;
			break;
		case 3:
			/* transfer -> */
			stat_change(0); /* -> hblank */
			if (hw.hdma & 0x80)
				hw_hdma();
			/* FIXME -- how much of the hblank does hdma use?? */
			/* else */
			CYCLES += 102;
			break;
		}
	}
}
