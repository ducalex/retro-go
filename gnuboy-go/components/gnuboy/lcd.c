#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "defs.h"
#include "regs.h"
#include "hw.h"
#include "mem.h"
#include "lcd.h"
#include "cpu.h"

#include "palettes.h"

struct fb fb;
struct lcd lcd;
struct scan scan;

#define BG (scan.bg)
#define WND (scan.wnd)
#define BUF (scan.buf)
#define PRI (scan.pri)

#define PAL2 (scan.pal2)

#define VS (scan.vs) /* vissprites */
#define NS (scan.ns)

#define SL (scan.l) /* line */
#define SX (scan.x) /* screen position */
#define SY (scan.y)
#define S (scan.s) /* tilemap position */
#define T (scan.t)
#define U (scan.u) /* position within tile */
#define V (scan.v)

#define WX (scan.wx)
#define WY (scan.wy)
#define WT (scan.wt)
#define WV (scan.wv)

#define CYCLES (lcd.cycles)

static uint16_t dmg_pal[4][4] = {
	GB_DEFAULT_PALETTE, GB_DEFAULT_PALETTE,
	GB_DEFAULT_PALETTE, GB_DEFAULT_PALETTE,
};

static int dmg_selected_pal = 0;

static byte *vdest;
static byte pix[8];

// Fix for Fushigi no Dungeon - Fuurai no Shiren GB2 and Donkey Kong
int enable_window_offset_hack = 0;


/**
 * Helper macros
 */

#define priused(attr) ({un32 *a = (un32*)(attr); (int)((a[0]|a[1]|a[2]|a[3]|a[4]|a[5]|a[6]|a[7])&0x80808080);})

#define blendcpy(dest, src, b, cnt) {					\
	byte *s = (src), *d = (dest), _b = (b), c = (cnt); 	\
	while(c--) *(d + c) = *(s + c) | _b; 				\
}


/**
 * Drawing routines
 */

__attribute__((optimize("unroll-loops")))
static inline byte* get_patpix(int i, int x)
{
	const int index = i & 0x3ff; // 1024 entries
	const int rotation = (i >> 10) & 3; // / 1024;

	int a, c, j;
	const byte* vram = lcd.vbank[0];

	switch (rotation)
	{
		case 0:
			a = ((index << 4) | (x << 1));

			for (byte k = 0; k < 8; k++)
			{
				c = vram[a] & (1 << k) ? 1 : 0;
				c |= vram[a+1] & (1 << k) ? 2 : 0;
				pix[7 - k] = c;
			}
			break;

		case 1:
			a = ((index << 4) | (x << 1));

			for (byte k = 0; k < 8; k++)
			{
				c = vram[a] & (1 << k) ? 1 : 0;
				c |= vram[a+1] & (1 << k) ? 2 : 0;
				pix[k] = c;
			}
			break;

		case 2:
			j = 7 - x;
			a = ((index << 4) | (j << 1));

			for (byte k = 0; k < 8; k++)
			{
				c = vram[a] & (1 << k) ? 1 : 0;
				c |= vram[a+1] & (1 << k) ? 2 : 0;
				pix[7 - k] = c;
			}
			break;

		case 3:
			j = 7 - x;
			a = ((index << 4) | (j << 1));

			for (byte k = 0; k < 8; k++)
			{
				c = vram[a] & (1 << k) ? 1 : 0;
				c |= vram[a+1] & (1 << k) ? 2 : 0;
				pix[k] = c;
			}
			break;
	}

	return pix;
}

static inline void tilebuf()
{
	int i, cnt, base;
	byte *tilemap, *attrmap;
	int *tilebuf;
	short const *wrap;

	static const short wraptable[64] = {
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,-32
	};

	/* Background tiles */

	base = ((R_LCDC&0x08)?0x1C00:0x1800) + (T<<5) + S;
	tilemap = lcd.vbank[0] + base;
	attrmap = lcd.vbank[1] + base;
	tilebuf = BG;
	wrap = wraptable + S;
	cnt = ((WX + 7) >> 3) + 1;

	if (hw.cgb)
	{
		if (R_LCDC & 0x10)
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = *tilemap
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*attrmap & 0x07) << 2);
				attrmap += *wrap + 1;
				tilemap += *(wrap++) + 1;
			}
		else
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = (256 + ((n8)*tilemap))
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
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = *(tilemap++);
				tilemap += *(wrap++);
			}
		else
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = (256 + ((n8)*(tilemap++)));
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
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = *(tilemap++)
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*(attrmap++)&7) << 2);
			}
		else
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = (256 + ((n8)*(tilemap++)))
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*(attrmap++)&7) << 2);
			}
	}
	else
	{
		if (R_LCDC & 0x10)
			for (i = cnt; i > 0; i--)
				*(tilebuf++) = *(tilemap++);
		else
			for (i = cnt; i > 0; i--)
				*(tilebuf++) = (256 + ((n8)*(tilemap++)));
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

	src = get_patpix(*(tile++),V) + U;
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

static inline void spr_enum()
{
	int i, j, l, x, v, pat;
	struct obj *o;
	struct vissprite ts[10];

	NS = 0;
	if (!(R_LCDC & 0x02)) return;

	o = lcd.oam.obj;

	for (i = 40; i; i--, o++)
	{
		if (SL >= o->y || SL + 16 < o->y)
			continue;
		if (SL + 8 >= o->y && !(R_LCDC & 0x04))
			continue;

		VS[NS].x = (int)o->x - 8;
		v = SL - (int)o->y + 16;

		if (hw.cgb)
		{
			pat = o->pat | (((int)o->flags & 0x60) << 5)
				| (((int)o->flags & 0x08) << 6);
			VS[NS].pal = 32 + ((o->flags & 0x07) << 2);
		}
		else
		{
			pat = o->pat | (((int)o->flags & 0x60) << 5);
			VS[NS].pal = 32 + ((o->flags & 0x10) >> 2);
		}

		VS[NS].pri = (o->flags & 0x80) >> 7;

		if ((R_LCDC & 0x04))
		{
			pat &= ~1;
			if (v >= 8)
			{
				v -= 8;
				pat++;
			}
			if (o->flags & 0x40) pat ^= 1;
		}

		VS[NS].pat = pat;
		VS[NS].v = v;

		if (++NS == 10) break;
	}

	// Sort sprites
	if (!hw.cgb)
	{
		/* not quite optimal but it finally works! */
		for (i = 0; i < NS; i++)
		{
			l = 0;
			x = VS[0].x;
			for (j = 1; j < NS; j++)
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
}

static inline void spr_scan()
{
	byte *src, *dest, *bg, *pri;
	int i, b, ns, x, pal;
	struct vissprite *vs;
	static byte bgdup[256];

	if (!NS) return;

	memcpy(bgdup, BUF, 256);

	ns = NS;
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
	vdest = fb.ptr;
	WY = R_WY;
}

void lcd_reset()
{
	memset(&lcd, 0, sizeof lcd);
	lcd_beginframe();
	pal_dirty();
}

static inline void lcd_renderline()
{
	if (!fb.enabled)
		return;

	if (!(R_LCDC & 0x80))
		return; /* should not happen... */

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
	if (enable_window_offset_hack && (R_LCDC & 0x20))
	{
		WT %= 12;
	}

	spr_enum();
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
	spr_scan();

	int cnt = 160;
	un16* dst = (un16*)vdest;
	byte* src = BUF;

	while (cnt--) *(dst++) = PAL2[*(src++)];

	vdest += fb.pitch;
}

static inline void pal_update(byte i)
{
	short c, r, g, b; //, y, u, v, rr, gg;

	short low = lcd.pal[i << 1];
	short high = lcd.pal[(i << 1) | 1];

	c = (low | (high << 8)) & 0x7fff;

	r = c & 0x1f;         // bit 0-4 red
	g = (c >> 5) & 0x1f;  // bit 5-9 green
	b = (c >> 10) & 0x1f; // bit 10-14 blue

	PAL2[i] = (r << 11) | (g << (5 + 1)) | (b);

	if (fb.byteorder == 1) {
		PAL2[i] = (PAL2[i] << 8) | (PAL2[i] >> 8);
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

	printf("pal_detect_dmg: Using GBC palette %d\n", palette);

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

void IRAM_ATTR stat_trigger()
{
	static const byte condbits[4] = { 0x08, 0x10, 0x20, 0x00 };
	byte flag = 0;

	if (R_LY == R_LYC)
	{
		R_STAT |= 0x04;
		if (R_STAT & 0x40) flag = IF_STAT;
	}
	else R_STAT &= ~0x04;

	if (R_STAT & condbits[R_STAT&3]) flag = IF_STAT;

	if (!(R_LCDC & 0x80)) flag = 0;

	hw_interrupt(flag, IF_STAT);
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

	if (stat != 1) hw_interrupt(0, IF_VBLANK);
	/* hw_interrupt((stat == 1) ? IF_VBLANK : 0, IF_VBLANK); */
	stat_trigger();
}


void IRAM_ATTR lcdc_change(byte b)
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
void IRAM_ATTR lcd_emulate()
{
	/* LCD disabled */
	if (!(R_LCDC & 0x80))
	{
		/* LCDC operation disabled (short route) */
		while (CYCLES <= 0)
		{
			switch ((byte)(R_STAT & 3))
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
		switch ((byte)(R_STAT & 3))
		{
		case 0:
			/* hblank -> */
			if (++R_LY >= 144)
			{
				/* FIXME: pick _one_ place to trigger vblank interrupt
				this better be done here or within stat_change(),
				otherwise CPU will have a chance to run	for some time
				before interrupt is triggered */
				if (cpu.halt)
				{
					hw_interrupt(IF_VBLANK, IF_VBLANK);
					CYCLES += 228;
				}
				else CYCLES += 10;
				stat_change(1); /* -> vblank */
				break;
			}

			// Hack for Worms Armageddon
			if (R_STAT == 0x48)
				hw_interrupt(0, IF_STAT);

			stat_change(2); /* -> search */
			CYCLES += 40;
			break;
		case 1:
			/* vblank -> */
			if (!(hw.ilines & IF_VBLANK))
			{
				CYCLES += 218;
				hw_interrupt(IF_VBLANK, IF_VBLANK);
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
