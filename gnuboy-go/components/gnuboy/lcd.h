#pragma once

#include "gnuboy.h"

#define GB_WIDTH (160)
#define GB_HEIGHT (144)

typedef struct
{
	byte y;
	byte x;
	byte pat;
	byte flags;
} gb_obj_t;

typedef struct
{
	int pat, x, v, pal, pri;
} gb_vs_t;

typedef struct
{
	byte vbank[2][8192];
	union {
		byte mem[256];
		gb_obj_t obj[40];
	} oam;
	byte pal[128];

	int BG[64];
	int WND[64];
	byte BUF[0x100];
	byte PRI[0x100];
	gb_vs_t VS[16];

	int S, T, U, V;
	int WX, WY, WT, WV;

	int cycles;

	// Fix for Fushigi no Dungeon - Fuurai no Shiren GB2 and Donkey Kong
	int enable_window_offset_hack;

	struct {
		byte *buffer;
		un16 cgb_pal[64];
		un16 dmg_pal[4][4];
		int colorize;
		int format;
		int enabled;
		void (*blit_func)();
	} out;
} gb_lcd_t;

enum {
	GB_PIXEL_PALETTED,
	GB_PIXEL_565_LE,
	GB_PIXEL_565_BE,
};

enum {
    GB_PALETTE_GBCBIOS,
    GB_PALETTE_DEFAULT,
    GB_PALETTE_2BGRAYS,
    GB_PALETTE_LINKSAW,
    GB_PALETTE_NSUPRGB,
    GB_PALETTE_NGBARNE,
    GB_PALETTE_GRAPEFR,
    GB_PALETTE_MEGAMAN,
    GB_PALETTE_POKEMON,
    GB_PALETTE_DMGREEN,
    GB_PALETTE_COUNT,
};


extern gb_lcd_t lcd;

void lcd_reset(bool hard);
void lcd_emulate(void);
void lcd_rebuildpal(void);
void lcd_stat_trigger(void);
void lcd_lcdc_change(byte b);

void pal_write_cgb(byte i, byte b);
void pal_write_dmg(byte i, byte mapnum, byte d);
