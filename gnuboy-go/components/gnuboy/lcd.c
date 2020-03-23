#pragma GCC optimize ("O3")

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <esp_attr.h>

#include "defs.h"
#include "regs.h"
#include "hw.h"
#include "mem.h"
#include "lcd.h"
#include "rc.h"
#include "fb.h"

struct lcd lcd;

struct scan scan;

#define BG (scan.bg)
#define WND (scan.wnd)
#define BUF (scan.buf)
#define PRI (scan.pri)

#define PAL2 (scan.pal2)

#define VS (scan.vs) /* vissprites */
#define NS (scan.ns)

#define L (scan.l) /* line */
#define X (scan.x) /* screen position */
#define Y (scan.y)
#define S (scan.s) /* tilemap position */
#define T (scan.t)
#define U (scan.u) /* position within tile */
#define V (scan.v)
#define WX (scan.wx)
#define WY (scan.wy)
#define WT (scan.wt)
#define WV (scan.wv)


// Important! Colors are defined in reversed order: 0xBBGGRR !!!
#define GB_DEFAULT_PALETTE { 0xd5f3ef, 0x7ab6a3, 0x3b6137, 0x161c04 }
#define GB_2BGRAYS_PALETTE { 0xffffff, 0xb6b6b6, 0x676767, 0x000000 }
#define GB_LINKSAW_PALETTE { 0xb5ffff, 0x7bc67b, 0x428c6b, 0x21395a }
#define GB_NSUPRGB_PALETTE { 0xc6e7f7, 0x498ed6, 0x2537a6, 0x501e33 }
#define GB_NGBARNE_PALETTE { 0x6bb5ac, 0x488476, 0x3f503f, 0x373124 }
#define GB_GRAPEFR_PALETTE { 0xddf5ff, 0x6bb2f4, 0x9165b7, 0x6c2965 }
#define GB_MEGAMAN_PALETTE { 0xcecece, 0xdf9e6f, 0x8e6742, 0x332510 }
#define GB_POKEMON_PALETTE { 0xffefff, 0x8cb5f7, 0x9c7384, 0x101018 }
#define GB_DMGREEN_PALETTE { 0x079e92, 0x1b7153, 0x185529, 0x024712 }

static const int palettes[][4] = {GB_DEFAULT_PALETTE,
								  GB_2BGRAYS_PALETTE,
								  GB_LINKSAW_PALETTE,
								  GB_NSUPRGB_PALETTE,
								  GB_NGBARNE_PALETTE,
								  GB_GRAPEFR_PALETTE,
								  GB_MEGAMAN_PALETTE,
								  GB_POKEMON_PALETTE,
								  GB_DMGREEN_PALETTE	};

static int current_palette = 1;

static int sprsort = 1;
static int sprdebug = 0;

static byte *vdest;
static byte pix[8];


#define MEMCPY8(d, s) ((*(uint64_t *)(d)) = (*(uint64_t *)(s)))
//#define MEMCPY8(d, s) memcpy((d), (s), 8)


__attribute__((optimize("unroll-loops")))
static const byte* IRAM_ATTR get_patpix(int i, int x)
{
	const int index = i & 0x3ff; // 1024 entries
	const int rotation = i >> 10; // / 1024;

	int j;
	int a, c;
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
	int i, cnt;
	int base;
	byte *tilemap, *attrmap;
	int *tilebuf;
	short *wrap;
	static const short wraptable[64] =
	{
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,-32
	};

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
	if (cnt <= 0) return;
	while (cnt >= 8)
	{
		src = get_patpix(*(tile++), V);
		MEMCPY8(dest, src);
		dest += 8;
		cnt -= 8;
	}
	src = get_patpix(*tile, V);
	while (cnt--)
		*(dest++) = *(src++);
}

static inline void wnd_scan()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX >= 160) return;
	cnt = 160 - WX;
	tile = WND;
	dest = BUF + WX;

	while (cnt >= 8)
	{
		src = get_patpix(*(tile++), WV);

		MEMCPY8(dest, src);

		dest += 8;
		cnt -= 8;
	}
	src = get_patpix(*tile, WV);
	while (cnt--)
		*(dest++) = *(src++);
}

static inline void blendcpy(byte *dest, byte *src, byte b, int cnt)
{
	while (cnt--) *(dest++) = *(src++) | b;
}

static inline int priused(void *attr)
{
	un32 *a = attr;
	return (int)((a[0]|a[1]|a[2]|a[3]|a[4]|a[5]|a[6]|a[7])&0x80808080);
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
	if (cnt <= 0) return;
	while (cnt >= 8)
	{
		src = get_patpix(*(tile++), V);
		blendcpy(dest, src, *(tile++), 8);
		dest += 8;
		cnt -= 8;
	}
	src = get_patpix(*(tile++), V);
	blendcpy(dest, src, *(tile++), cnt);
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

	while (cnt >= 8)
	{
		src = get_patpix(*(tile++), WV);
		blendcpy(dest, src, *(tile++), 8);
		dest += 8;
		cnt -= 8;
	}
	src = get_patpix(*(tile++), WV);
	blendcpy(dest, src, *(tile++), cnt);
}

static inline void recolor(byte *buf, byte fill, int cnt)
{
	while (cnt--) *(buf++) |= fill;
}

static inline void spr_count()
{
	int i;
	struct obj *o;

	NS = 0;
	if (!(R_LCDC & 0x02)) return;

	o = lcd.oam.obj;

	for (i = 40; i; i--, o++)
	{
		if (L >= o->y || L + 16 < o->y)
			continue;
		if (L + 8 >= o->y && !(R_LCDC & 0x04))
			continue;
		if (++NS == 10) break;
	}
}


static inline void spr_enum()
{
	int i, j;
	struct obj *o;
	struct vissprite ts[10];
	int v, pat;
	int l, x;

	NS = 0;
	if (!(R_LCDC & 0x02)) return;

	o = lcd.oam.obj;

	for (i = 40; i; i--, o++)
	{
		if (L >= o->y || L + 16 < o->y)
			continue;
		if (L + 8 >= o->y && !(R_LCDC & 0x04))
			continue;
		VS[NS].x = (int)o->x - 8;
		v = L - (int)o->y + 16;
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
	if (!sprsort || hw.cgb) return;
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

	memcpy(VS, ts, sizeof VS);
}


static inline void spr_scan()
{
	int i, x;
	byte pal, b, ns = NS;
	byte *src, *dest, *bg, *pri;
	struct vissprite *vs;
	static byte bgdup[256];

	if (!ns) return;

	memcpy(bgdup, BUF, 256);

	vs = &VS[ns-1];

	for (; ns; ns--, vs--)
	{
		byte* sbuf = get_patpix(vs->pat, vs->v);

		x = vs->x;
		if (x >= 160) continue;
		if (x <= -8) continue;
		if (x < 0)
		{
			src = sbuf - x;
			dest = BUF;
			i = 8 + x;
		}
		else
		{
			src = sbuf;
			dest = BUF + x;
			if (x > 152) i = 160 - x;
			else i = 8;
		}
		pal = vs->pal;
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
		else while (i--) if (src[i]) dest[i] = pal|src[i];
	}
	if (sprdebug) for (i = 0; i < NS; i++) BUF[i<<1] = 36;
}


void lcd_begin()
{
	vdest = fb.ptr;
	WY = R_WY;
}

void lcd_reset()
{
	memset(&lcd, 0, sizeof lcd);
	lcd_begin();
	vram_dirty();
	pal_dirty();
}

void IRAM_ATTR lcd_refreshline()
{
	if (!fb.enabled) return;

	if (!(R_LCDC & 0x80))
		return; /* should not happen... */

	L = R_LY;
	X = R_SCX;
	Y = (R_SCY + L) & 0xff;
	S = X >> 3;
	T = Y >> 3;
	U = X & 7;
	V = Y & 7;

	WX = R_WX - 7;
	if (WY>L || WY<0 || WY>143 || WX<-7 || WX>159 || !(R_LCDC&0x20))
		WX = 160;
	WT = (L - WY) >> 3;
	WV = (L - WY) & 7;

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
		recolor(BUF+WX, 0x04, 160-WX);
	}
	spr_scan();

	if (fb.dirty)
	{
		memset(fb.ptr, 0, fb.pitch * fb.h);
		fb.dirty = 0;
	}

	int cnt = 160;
	un16* dst = (un16*)vdest;
	byte* src = BUF;

	while (cnt--) *(dst++) = PAL2[*(src++)];

	vdest += fb.pitch;
}


static inline void updatepalette(int i)
{
	short c;
	short r, g, b; //, y, u, v, rr, gg;

	short low = lcd.pal[i << 1];
	short high = lcd.pal[(i << 1) | 1];

	c = (low | (high << 8)) & 0x7fff;

	//bit 0-4 red
	r = c & 0x1f;

	// bit 5-9 green
	g = (c >> 5) & 0x1f;

	// bit 10-14 blue
	b = (c >> 10) & 0x1f;

	PAL2[i] = (r << 11) | (g << (5 + 1)) | (b);

	if (fb.byteorder == 1) {
		PAL2[i] = (PAL2[i] << 8) | (PAL2[i] >> 8);
	}
}

inline void pal_write(int i, byte b)
{
	if (lcd.pal[i] == b) return;
	lcd.pal[i] = b;
	updatepalette(i>>1);
}

void IRAM_ATTR pal_write_dmg(int i, int mapnum, byte d)
{
	int j;
	// int *cmap = dmg_pal[mapnum];
	int *cmap = palettes[current_palette];
	int c, r, g, b;

	if (hw.cgb) return;

	/* if (mapnum >= 2) d = 0xe4; */
	for (j = 0; j < 8; j += 2)
	{
		c = cmap[(d >> j) & 3];
		r = (c & 0xf8) >> 3;
		g = (c & 0xf800) >> 6;
		b = (c & 0xf80000) >> 9;
		c = r | g | b;

		/* FIXME - handle directly without faking cgb */
		pal_write(i+j, c & 0xff);
		pal_write(i+j+1, c >> 8);
	}
}

void pal_set_dmg(int palette)
{
	current_palette = palette % pal_count_dmg();
	pal_dirty();
}

int pal_get_dmg()
{
	return current_palette;
}

int pal_count_dmg()
{
	return sizeof(palettes) / sizeof(palettes[0]);
}

void pal_dirty()
{
	int i;
	if (!hw.cgb)
	{
		pal_write_dmg(0, 0, R_BGP);
		pal_write_dmg(8, 1, R_BGP);
		pal_write_dmg(64, 2, R_OBP0);
		pal_write_dmg(72, 3, R_OBP1);
	}
	for (i = 0; i < 64; i++)
		updatepalette(i);
}

inline void vram_write(int a, byte b)
{
	lcd.vbank[R_VBK & 1][a] = b;
	if (a >= 0x1800) return;
	// patdirty[((R_VBK&1)<<9)+(a>>4)] = 1;
	// anydirty = 1;
}

void vram_dirty()
{
	// anydirty = 1;
	// memset(patdirty, 1, sizeof patdirty);
}
