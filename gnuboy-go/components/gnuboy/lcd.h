#ifndef __LCD_H__
#define __LCD_H__

#include "emu.h"

#define GB_WIDTH (160)
#define GB_HEIGHT (144)

typedef struct
{
	byte y;
	byte x;
	byte pat;
	byte flags;
} obj_t;

typedef struct
{
	int pat;
	int x;
	int v;
	int pal;
	int pri;
} vissprite_t;

struct scan
{
	int bg[64];
	int wnd[64];
	byte buf[256];
	un16 pal2[64];
	byte pri[256];
	vissprite_t vs[16];
	int ns, l, x, y, s, t, u, v, wx, wy, wt, wv;
};

struct lcd
{
	byte vbank[2][8192];
	union
	{
		byte mem[256];
		obj_t obj[40];
	} oam;
	byte pal[128];

	int cycles;
};

enum {
	GB_PIXEL_PALETTED,
	GB_PIXEL_565_LE,
	GB_PIXEL_565_BE,
};

struct fb
{
	byte *ptr;
	int w, h;
	int pitch;
	int format;
	int enabled;
	void (*blit_func)();
};

extern struct lcd lcd;
extern struct fb fb;

extern int enable_window_offset_hack;

void lcd_reset();
void lcd_emulate();

void lcdc_change(byte b);
void stat_trigger();

void pal_write(byte i, byte b);
void pal_write_dmg(byte i, byte mapnum, byte d);
void pal_dirty();
void pal_set_dmg(int palette);
int  pal_get_dmg();
int  pal_count_dmg();

#endif
