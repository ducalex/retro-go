#pragma GCC optimize ("O3")

#include <string.h>

#include "gnuboy.h"
#include "defs.h"
#include "regs.h"
#include "hw.h"
#include "mem.h"
#include "lcd.h"
#include "rc.h"
#include "fb.h"
#ifdef USE_ASM
#include "asm.h"
#endif

#include <stdlib.h>
#include <esp_attr.h>
#include <stdint.h>

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





static int palettes[8][4] = {GB_DEFAULT_PALETTE,
							GB_2BGRAYS_PALETTE,
							GB_LINKSAW_PALETTE,
							GB_NSUPRGB_PALETTE,
							GB_NGBARNE_PALETTE,
							GB_GRAPEFR_PALETTE,
							GB_MEGAMAN_PALETTE,
							GB_POKEMON_PALETTE	};

static int current_palette = 1;
static int nr_of_palettes = 8;

					
static int sprsort = 1;
static int sprdebug = 0;

// BGR
#if 0
// Testing/Debug palette
static int dmg_pal[4][4] = {{0xffffff, 0x808080, 0x404040, 0x000000},
							{0xff0000, 0x800000, 0x400000, 0x000000},
							{0x00ff00, 0x008000, 0x004000, 0x000000},
							{0x0000ff, 0x000080, 0x000040, 0x000000} };
#else
 static int dmg_pal[4][4] = {GB_NGBARNE_PALETTE,
	 						GB_NGBARNE_PALETTE,
							GB_NGBARNE_PALETTE,
							GB_NGBARNE_PALETTE };

#endif

static byte *vdest;

//#ifdef ALLOW_UNALIGNED_IO /* long long is ok since this is i386-only anyway? */
#define MEMCPY8(d, s) ((*(long long *)(d)) = (*(long long *)(s)))
//#else
//#define MEMCPY8(d, s) memcpy((d), (s), 8)
//#endif

static byte pix[8];

__attribute__((optimize("unroll-loops")))
static const byte* IRAM_ATTR get_patpix(int i, int x)
{
	const int index = i & 0x3ff; // 1024 entries
	const int rotation = i >> 10; // / 1024;

	int j;
	int a, c;
	const byte* const vram = lcd.vbank[0];

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


#ifndef ASM_UPDATEPATPIX
inline void updatepatpix()
{
}
#endif /* ASM_UPDATEPATPIX */


static const short DRAM_ATTR wraptable[64] =
{
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,-32
};

static void IRAM_ATTR tilebuf()
{
	int i, cnt;
	int base;
	byte *tilemap, *attrmap;
	int *tilebuf;
	short *wrap;


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


static void IRAM_ATTR bg_scan()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX <= 0) return;
	cnt = WX;
	tile = BG;
	dest = BUF;

	src = get_patpix(*(tile++), V) + U;

#if 0
	memcpy(dest, src, 8-U);
#else
	byte tmp = 8-U;
	switch ((tmp))
	{
		case 8:
			dest[7] = src[7];
		case 7:
			dest[6] = src[6];
		case 6:
			dest[5] = src[5];
		case 5:
			dest[4] = src[4];
		case 4:
			dest[3] = src[2];
		case 3:
			dest[2] = src[2];
		case 2:
			dest[1] = src[1];
		case 1:
			dest[0] = src[0];
		default:
			break;
	}
#endif

	dest += 8-U;
	cnt -= 8-U;
	if (cnt <= 0) return;
	while (cnt >= 8)
	{
		src = get_patpix(*(tile++), V);

#if 0
		MEMCPY8(dest, src);
#else
		int* tmpDest =(int*)dest;
		int* tmpSrc = (int*)src;
		tmpDest[0] = tmpSrc[0];
		tmpDest[1] = tmpSrc[1];
#endif

		dest += 8;
		cnt -= 8;
	}
	src = get_patpix(*tile, V);
	while (cnt--)
		*(dest++) = *(src++);
}

static void IRAM_ATTR wnd_scan()
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

#if 0
		MEMCPY8(dest, src);
#else
		int* tmpDest =(int*)dest;
		int* tmpSrc = (int*)src;
		tmpDest[0] = tmpSrc[0];
		tmpDest[1] = tmpSrc[1];
#endif

		dest += 8;
		cnt -= 8;
	}
	src = get_patpix(*tile, WV);
	while (cnt--)
		*(dest++) = *(src++);
}

inline static void blendcpy(byte *dest, byte *src, byte b, int cnt)
{
	while (cnt--) *(dest++) = *(src++) | b;
}

inline static int priused(void *attr)
{
	un32 *a = attr;
	return (int)((a[0]|a[1]|a[2]|a[3]|a[4]|a[5]|a[6]|a[7])&0x80808080);
}

static void IRAM_ATTR bg_scan_pri()
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

static void IRAM_ATTR wnd_scan_pri()
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

#ifndef ASM_BG_SCAN_COLOR
static void IRAM_ATTR bg_scan_color()
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
#endif

static void IRAM_ATTR wnd_scan_color()
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

inline static void recolor(byte *buf, byte fill, int cnt)
{
	while (cnt--) *(buf++) |= fill;
}

static void IRAM_ATTR spr_count()
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


static struct vissprite ts[10];

static void IRAM_ATTR spr_enum()
{
	int i, j;
	struct obj *o;
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

#if 1
	memcpy(VS, ts, sizeof VS);
#else
	int* vsPtr = (int*)VS;
	int* tsPtr = (int*)ts;
	int count = 16;
	while(count--)
	{
		vsPtr[0] = tsPtr[0];
		vsPtr++[1] = tsPtr++[1];
	}
#endif
}


static byte bgdup[256];

static void IRAM_ATTR spr_scan()
{
	int i, x;
	byte pal, b, ns = NS;
	byte *src, *dest, *bg, *pri;
	struct vissprite *vs;

	if (!ns) return;

#if 1
	memcpy(bgdup, BUF, 256);
#else
	for (i = 0; i < 64; ++i)
	{
		((int*)bgdup)[i] = ((int*)BUF)[i];
	}
#endif

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


inline void lcd_begin()
{
	vdest = fb.ptr;
	WY = R_WY;
}


extern int frame;
extern uint16_t* displayBuffer[2];
int lastLcdDisabled = 0;

void IRAM_ATTR lcd_refreshline()
{
	byte *dest;

	if ((frame % 7) == 0) ++frame;


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

	if ((frame % 2) == 0)
	{
		if (!(R_LCDC & 0x80))
		{
			if (!lastLcdDisabled)
			{
				memset(displayBuffer[0], 0xff, 144 * 160 * 2);
				memset(displayBuffer[1], 0xff, 144 * 160 * 2);

				lastLcdDisabled = 1;
			}

			return;
		}

		lastLcdDisabled = 0;


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

		dest = vdest;

		int cnt = 160;
		un16* dst = (un16*)dest;
		byte* src = BUF;

		while (cnt--) *(dst++) = PAL2[*(src++)];
	}

	vdest += fb.pitch;
}

//void change_palette(int i)
//{
	
//}

inline static void updatepalette(int i)
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
}

inline void pal_write(int i, byte b)
{
	if (lcd.pal[i] != b)
	{
		lcd.pal[i] = b;
		updatepalette(i>>1);
	}
}

void IRAM_ATTR pal_write_dmg(int i, int mapnum, byte d)
{
	int j;
	int * const cmap = dmg_pal[mapnum & 0x3];
	int c;
	int r, g, b;

	if (hw.cgb) return;

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

	//printf("pal_write_dmg: i=%d, d=0x%x\n", i , d);
}

inline void vram_write(int a, byte b)
{
	//if (lcd.vbank[R_VBK&1][a] != b)
	{
		lcd.vbank[R_VBK&1][a] = b;
		if (a >= 0x1800) return;
	}
}

void vram_dirty()
{
}

void pal_set(int palette)
{
	int i,j;
	current_palette = palette % nr_of_palettes;
	
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4;j++)
		{
			 dmg_pal[i][j] = palettes[current_palette][j];
		}
	}
	
	pal_dirty();	
}

void pal_next()
{
	pal_set(current_palette + 1);
}

int pal_get()
{
	return current_palette;
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
	//else
	{
		for (i = 0; i < 64; i++)
		{
			updatepalette(i);
		}
	}
}

void lcd_reset()
{
	memset(&lcd, 0, sizeof lcd);

	lcd_begin();
	vram_dirty();
	pal_dirty();
}
