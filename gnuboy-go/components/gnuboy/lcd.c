#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "gnuboy.h"
#include "hw.h"
#include "lcd.h"
#include "cpu.h"

static const uint16_t colorization_palettes[48][3][4] = {
// 0x00 - 0x1F: Game-specific colorization palettes extracted from GBC's BIOS
	{{0x7FFF, 0x01DF, 0x0112, 0x0000}, {0x7FFF, 0x7EEB, 0x001F, 0x7C00}, {0x7FFF, 0x42B5, 0x3DC8, 0x0000}},
	{{0x231F, 0x035F, 0x00F2, 0x0009}, {0x7FFF, 0x421F, 0x1CF2, 0x0000}, {0x4FFF, 0x7ED2, 0x3A4C, 0x1CE0}},
	{{0x7FFF, 0x7FFF, 0x7E8C, 0x7C00}, {0x7FFF, 0x32BF, 0x00D0, 0x0000}, {0x03ED, 0x7FFF, 0x255F, 0x0000}},
	{{0x7FFF, 0x7FFF, 0x7E8C, 0x7C00}, {0x7FFF, 0x421F, 0x1CF2, 0x0000}, {0x036A, 0x021F, 0x03FF, 0x7FFF}},
	{{0x7FFF, 0x421F, 0x1CF2, 0x0000}, {0x7FFF, 0x421F, 0x1CF2, 0x0000}, {0x7FFF, 0x03EF, 0x01D6, 0x0000}},
	{{0x7FFF, 0x421F, 0x1CF2, 0x0000}, {0x7FFF, 0x7EEB, 0x001F, 0x7C00}, {0x7FFF, 0x03EA, 0x011F, 0x0000}},
	{{0x7FFF, 0x421F, 0x1CF2, 0x0000}, {0x7FFF, 0x7EEB, 0x001F, 0x7C00}, {0x7FFF, 0x027F, 0x001F, 0x0000}},
	{{0x7FFF, 0x7E8C, 0x7C00, 0x0000}, {0x7FFF, 0x7EEB, 0x001F, 0x7C00}, {0x7FFF, 0x03FF, 0x001F, 0x0000}},
	{{0x299F, 0x001A, 0x000C, 0x0000}, {0x7C00, 0x7FFF, 0x3FFF, 0x7E00}, {0x7E74, 0x03FF, 0x0180, 0x0000}},
	{{0x7FFF, 0x01DF, 0x0112, 0x0000}, {0x7FFF, 0x7E8C, 0x7C00, 0x0000}, {0x67FF, 0x77AC, 0x1A13, 0x2D6B}},
	{{0x0000, 0x7FFF, 0x421F, 0x1CF2}, {0x0000, 0x7FFF, 0x421F, 0x1CF2}, {0x7ED6, 0x4BFF, 0x2175, 0x0000}},
	{{0x7FFF, 0x421F, 0x1CF2, 0x0000}, {0x7FFF, 0x3FFF, 0x7E00, 0x001F}, {0x7FFF, 0x7E8C, 0x7C00, 0x0000}},
	{{0x231F, 0x035F, 0x00F2, 0x0009}, {0x7FFF, 0x7EEB, 0x001F, 0x7C00}, {0x7FFF, 0x6E31, 0x454A, 0x0000}},
	{{0x7FFF, 0x421F, 0x1CF2, 0x0000}, {0x7FFF, 0x32BF, 0x00D0, 0x0000}, {0x7FFF, 0x6E31, 0x454A, 0x0000}},
	{{0x7FFF, 0x421F, 0x1CF2, 0x0000}, {0x7FFF, 0x7E8C, 0x7C00, 0x0000}, {0x7FFF, 0x1BEF, 0x0200, 0x0000}},
	{{0x7FFF, 0x7E8C, 0x7C00, 0x0000}, {0x7FFF, 0x1BEF, 0x0200, 0x0000}, {0x7FFF, 0x32BF, 0x00D0, 0x0000}},
	{{0x7FFF, 0x1BEF, 0x0200, 0x0000}, {0x7FFF, 0x7E8C, 0x7C00, 0x0000}, {0x7FFF, 0x421F, 0x1CF2, 0x0000}},
	{{0x7FFF, 0x03E0, 0x0206, 0x0120}, {0x7FFF, 0x7E8C, 0x7C00, 0x0000}, {0x7FFF, 0x421F, 0x1CF2, 0x0000}},
	{{0x7FFF, 0x1BEF, 0x0200, 0x0000}, {0x7FFF, 0x7E8C, 0x7C00, 0x0000}, {0x7FFF, 0x32BF, 0x00D0, 0x0000}},
	{{0x7FFF, 0x421F, 0x1CF2, 0x0000}, {0x7FFF, 0x1BEF, 0x0200, 0x0000}, {0x0000, 0x4200, 0x037F, 0x7FFF}},
	{{0x03FF, 0x001F, 0x000C, 0x0000}, {0x7FFF, 0x1BEF, 0x0200, 0x0000}, {0x7FFF, 0x7E8C, 0x7C00, 0x0000}},
	{{0x7FFF, 0x32BF, 0x00D0, 0x0000}, {0x7FFF, 0x7E8C, 0x7C00, 0x0000}, {0x7FFF, 0x42B5, 0x3DC8, 0x0000}},
	{{0x7FFF, 0x5294, 0x294A, 0x0000}, {0x7FFF, 0x5294, 0x294A, 0x0000}, {0x7FFF, 0x5294, 0x294A, 0x0000}},
	{{0x7FFF, 0x1BEF, 0x0200, 0x0000}, {0x7FFF, 0x7E8C, 0x7C00, 0x0000}, {0x53FF, 0x4A5F, 0x7E52, 0x0000}},
	{{0x7FFF, 0x421F, 0x1CF2, 0x0000}, {0x7FFF, 0x1BEF, 0x0200, 0x0000}, {0x7FFF, 0x7E8C, 0x7C00, 0x0000}},
	{{0x7FFF, 0x32BF, 0x00D0, 0x0000}, {0x7FFF, 0x32BF, 0x00D0, 0x0000}, {0x639F, 0x4279, 0x15B0, 0x04CB}},
	{{0x7FFF, 0x7E8C, 0x7C00, 0x0000}, {0x7FFF, 0x1BEF, 0x0200, 0x0000}, {0x7FFF, 0x03FF, 0x012F, 0x0000}},
	{{0x7FFF, 0x033F, 0x0193, 0x0000}, {0x7FFF, 0x033F, 0x0193, 0x0000}, {0x7FFF, 0x033F, 0x0193, 0x0000}},
	{{0x7FFF, 0x421F, 0x1CF2, 0x0000}, {0x7FFF, 0x7E8C, 0x7C00, 0x0000}, {0x7FFF, 0x1BEF, 0x6180, 0x0000}},
	{{0x2120, 0x8022, 0x8281, 0x1110}, {0xFF7F, 0xDF7F, 0x1201, 0x0001}, {0xFF00, 0xFF7F, 0x1F03, 0x0000}},
	{{0x7FFF, 0x32BF, 0x00D0, 0x0000}, {0x7FFF, 0x32BF, 0x00D0, 0x0000}, {0x7FFF, 0x32BF, 0x00D0, 0x0000}},
	{{0x7FFF, 0x32BF, 0x00D0, 0x0000}, {0x7FFF, 0x32BF, 0x00D0, 0x0000}, {0x7FFF, 0x32BF, 0x00D0, 0x0000}},

// 0x20 - 0x24 : Console-based colorization palettes
	{{0x0272, 0x0DCA, 0x0D45, 0x0102}, {0x0272, 0x0DCA, 0x0D45, 0x0102}, {0x0272, 0x0DCA, 0x0D45, 0x0102}}, // GB_PALETTE_DMG
	{{0x7FFF, 0x5AD6, 0x318C, 0x0000}, {0x7FFF, 0x5AD6, 0x318C, 0x0000}, {0x7FFF, 0x5AD6, 0x318C, 0x0000}}, // GB_PALETTE_MGB0
	{{0x36D5, 0x260E, 0x1D47, 0x18C4}, {0x36D5, 0x260E, 0x1D47, 0x18C4}, {0x36D5, 0x260E, 0x1D47, 0x18C4}}, // GB_PALETTE_MGB1
	{{0x6BDD, 0x3ED4, 0x1D86, 0x0860}, {0x6BDD, 0x3ED4, 0x1D86, 0x0860}, {0x6BDD, 0x3ED4, 0x1D86, 0x0860}}, // GB_PALETTE_CGB (real colors calculated in code)
	{{0x6BDD, 0x3ED4, 0x1D86, 0x0860}, {0x6BDD, 0x3ED4, 0x1D86, 0x0860}, {0x6BDD, 0x3ED4, 0x1D86, 0x0860}}, // GB_PALETTE_SGB (real colors calculated in code)

// 0x25 - 0x2F : Other misc palettes
	{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // Bleh
};

#define BG (lcd.BG)
#define WND (lcd.WND)
#define BUF (lcd.BUF)
#define PRI (lcd.PRI)
#define VS (lcd.VS)
#define S lcd.S /* tilemap position */
#define T lcd.T
#define U lcd.U /* position within tile */
#define V lcd.V
#define WX lcd.WX
#define WY lcd.WY
#define WT lcd.WT
#define WV lcd.WV

gb_lcd_t lcd;

#define priused(attr) ({uint32_t *a = (uint32_t *)(attr); (int)((a[0]|a[1]|a[2]|a[3]|a[4]|a[5]|a[6]|a[7])&0x80808080);})

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

	if (hw.hwtype == GB_HW_CGB)
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
				*(tilebuf++) = (0x100 + ((int8_t)*tilemap))
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
				*(tilebuf++) = (0x100 + ((int8_t)*(tilemap++)));
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

	if (hw.hwtype == GB_HW_CGB)
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
				*(tilebuf++) = (0x100 + ((int8_t)*(tilemap++)))
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
				*(tilebuf++) = (0x100 + ((int8_t)*(tilemap++)));
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

	gb_vs_t ts[10];
	int line = R_LY;
	int NS = 0;

	for (int i = 0; i < 40; ++i)
	{
		gb_obj_t *obj = &lcd.oam.obj[i];
		int v, pat;

		if (line >= obj->y || line + 16 < obj->y)
			continue;
		if (line + 8 >= obj->y && !(R_LCDC & 0x04))
			continue;

		VS[NS].x = (int)obj->x - 8;
		v = line - (int)obj->y + 16;

		if (hw.hwtype == GB_HW_CGB)
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
	if (hw.hwtype != GB_HW_CGB)
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
	gb_vs_t *vs;
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
		else if (hw.hwtype == GB_HW_CGB)
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

void lcd_reset(bool hard)
{
	if (hard)
	{
		memset(&lcd.vbank, 0, sizeof(lcd.vbank));
		memset(&lcd.oam, 0, sizeof(lcd.oam));
		memset(&lcd.pal, 0, sizeof(lcd.pal));
	}

	memset(BG, 0, sizeof(BG));
	memset(WND, 0, sizeof(WND));
	memset(BUF, 0, sizeof(BUF));
	memset(PRI, 0, sizeof(PRI));
	memset(VS, 0, sizeof(VS));

	WX = WY = WT = WV = 0;
	S = T = U = V = 0;

	WY = R_WY;
	lcd.pal_dirty = 1;

	/* set lcdc ahead of cpu by 19us; see A
			Set lcdc ahead of cpu by 19us (matches minimal hblank duration according
			to some docs). Value from lcd.cycles (when positive) is used to drive CPU,
			setting some ahead-time at startup is necessary to begin emulation.
	FIXME: leave value at 0, use lcd_emulate() to actually send lcdc ahead
	*/
	lcd.cycles = 40;
}


/**
 * LCD Controller routines
 */

/*
 * stat_change is called when a transition results in a change to the
 * LCD STAT condition (the low 2 bits of R_STAT).  It raises or lowers
 * the VBLANK interrupt line appropriately and calls lcd_stat_trigger to
 * update the STAT interrupt line.
 * FIXME: function now will only lower vblank interrupt, description does not match anymore
 */
static void inline stat_change(int stat)
{
	R_STAT = (R_STAT & 0x7C) | (stat & 3);
	if (stat != 1)
		hw_interrupt(IF_VBLANK, 0);
	lcd_stat_trigger();
}


/*
 * lcd_stat_trigger updates the STAT interrupt line to reflect whether any
 * of the conditions set to be tested (by bits 3-6 of R_STAT) are met.
 * This function should be called whenever any of the following occur:
 * 1) LY or LYC changes.
 * 2) A state transition affects the low 2 bits of R_STAT (see below).
 * 3) The program writes to the upper bits of R_STAT.
 * lcd_stat_trigger also updates bit 2 of R_STAT to reflect whether LY=LYC.
 */
void lcd_stat_trigger()
{
	int condbits[4] = { 0x08, 0x10, 0x20, 0x00 };
	int mask = condbits[R_STAT & 3];

	if (R_LY == R_LYC)
		R_STAT |= 0x04;
	else
		R_STAT &= ~0x04;

	hw_interrupt(IF_STAT, (R_LCDC & 0x80) && ((R_STAT & 0x44) == 0x44 || (R_STAT & mask)));
}


void lcd_lcdc_change(byte b)
{
	byte old = R_LCDC;
	R_LCDC = b;
	if ((R_LCDC ^ old) & 0x80) /* lcd on/off change */
	{
		R_LY = 0;
		stat_change(2);
		lcd.cycles = 40;  // Correct value seems to be 38
		WY = R_WY;
	}
}


static inline void sync_palette(void)
{
	MESSAGE_DEBUG("Syncing palette...\n");

	if (hw.hwtype != GB_HW_CGB)
	{
		int palette = host.video.colorize % GB_PALETTE_COUNT;
		int flags = 0b110;
		const uint16_t *bgp, *obp0, *obp1;

		if (palette == GB_PALETTE_CGB && cart.colorize)
		{
			palette = cart.colorize & 0x1F;
			flags = (cart.colorize & 0xE0) >> 5;
		}

		bgp = colorization_palettes[palette][2];
		obp0 = colorization_palettes[palette][(flags & 1) ? 0 : 1];
		obp1 = colorization_palettes[palette][(flags & 2) ? 0 : 1];

		// Some special cases
		if (!(flags & 4)) {
			obp1 = colorization_palettes[palette][2];
		}

		for (int j = 0; j < 8; j += 2)
		{
			lcd.palette[(0+j) >> 1] = bgp[(R_BGP >> j) & 3];
			lcd.palette[(8+j) >> 1] = bgp[(R_BGP >> j) & 3];
			lcd.palette[(64+j) >> 1] = obp0[(R_OBP0 >> j) & 3];
			lcd.palette[(72+j) >> 1] = obp1[(R_OBP1 >> j) & 3];
		}
	}

	for (int i = 0; i < 64; ++i)
	{
		int c = lcd.palette[i];   // Int is fine, we won't run in sign issues
		int r = c & 0x1f;         // bit 0-4 red
		int g = (c >> 5) & 0x1f;  // bit 5-9 green
		int b = (c >> 10) & 0x1f; // bit 10-14 blue

		int out = (r << 11) | (g << 6) | (b);

		if (host.video.format == GB_PIXEL_565_BE)
			host.video.palette[i] = (out << 8) | (out >> 8);
		else
			host.video.palette[i] = out;
	}

	lcd.pal_dirty = false;
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
static inline void lcd_renderline()
{
	if (!host.video.enabled || !host.video.buffer)
		return;

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
	if (lcd.window_offset_hack && (R_LCDC & 0x20))
	{
		WT %= lcd.window_offset_hack;
	}

	NS = spr_enum();
	tilebuf();

	if (hw.hwtype == GB_HW_CGB)
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

	// Real hardware allows palette change to occur between each scanline but very few games take
	// advantage of this. So we can switch to once per frame if performance becomes a problem...
	if (lcd.pal_dirty) // && SL == 0)
	{
		sync_palette();
	}

	if (host.video.format == GB_PIXEL_PALETTED)
	{
		memcpy(host.video.buffer8 + SL * 160 , BUF, 160);
	}
	else
	{
		uint16_t *dst = host.video.buffer16 + SL * 160;
		uint16_t *pal = host.video.palette;

		for (int i = 0; i < 160; ++i)
			dst[i] = pal[BUF[i]];
	}
}

void lcd_emulate(int cycles)
{
	lcd.cycles -= cycles;

	if (lcd.cycles > 0)
		return;

	/* LCD disabled */
	if (!(R_LCDC & 0x80))
	{
		/* LCDC operation disabled (short route) */
		while (lcd.cycles <= 0)
		{
			switch (R_STAT & 3)
			{
			case 0: /* hblank */
			case 1: /* vblank */
				// lcd_renderline();
				stat_change(2);
				lcd.cycles += 40;
				break;
			case 2: /* search */
				stat_change(3);
				lcd.cycles += 86;
				break;
			case 3: /* transfer */
				stat_change(0);
				/* FIXME: check docs; HDMA might require operating LCDC */
				if (hw.hdma & 0x80)
					hw_hdma_cont();
				else
					lcd.cycles += 102;
				break;
			}
			return;
		}
	}

	while (lcd.cycles <= 0)
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
					lcd.cycles += 228;
				}
				else lcd.cycles += 10;
				stat_change(1); /* -> vblank */
				break;
			}

			// Hack for Worms Armageddon
			if (R_STAT == 0x48)
				hw_interrupt(IF_STAT, 0);

			stat_change(2); /* -> search */
			lcd.cycles += 40;
			break;
		case 1:
			/* vblank -> */
			if (!(hw.ilines & IF_VBLANK))
			{
				hw_interrupt(IF_VBLANK, 1);
				lcd.cycles += 218;
				break;
			}
			if (R_LY == 0)
			{
				WY = R_WY;
				stat_change(2); /* -> search */
				lcd.cycles += 40;
				break;
			}
			else if (R_LY < 152)
				lcd.cycles += 228;
			else if (R_LY == 152)
				/* Handling special case on the last line; see
				docs/HACKING */
				lcd.cycles += 28;
			else
			{
				R_LY = -1;
				lcd.cycles += 200;
			}
			R_LY++;
			lcd_stat_trigger();
			break;
		case 2:
			/* search -> */
			lcd_renderline();
			stat_change(3); /* -> transfer */
			lcd.cycles += 86;
			break;
		case 3:
			/* transfer -> */
			stat_change(0); /* -> hblank */
			if (hw.hdma & 0x80)
				hw_hdma_cont();
			/* FIXME -- how much of the hblank does hdma use?? */
			/* else */
			lcd.cycles += 102;
			break;
		}
	}
}
