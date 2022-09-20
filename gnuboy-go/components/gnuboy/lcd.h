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
	byte vbank[2][8192];
	union {
		byte mem[256];
		gb_obj_t obj[40];
	} oam;
	union {
		uint16_t palette[64];
		uint8_t pal[128];
	};
	bool pal_dirty;

	int BG[64];
	int WND[64];
	byte BUF[0x100];
	byte PRI[0x100];

	int WX, WY;

	int cycles;

} gb_lcd_t;

extern gb_lcd_t lcd;

gb_lcd_t *lcd_init(void);
void lcd_reset(bool hard);
void lcd_emulate(int cycles);
void lcd_stat_trigger(void);
void lcd_lcdc_change(byte b);
void lcd_pal_dirty(void);