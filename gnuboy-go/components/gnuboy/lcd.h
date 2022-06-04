#pragma once

#include "gnuboy.h"

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
	bool pal_dirty;

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

} gb_lcd_t;

extern gb_lcd_t lcd;

void lcd_reset(bool hard);
void lcd_emulate(int cycles);
void lcd_stat_trigger(void);
void lcd_lcdc_change(byte b);
