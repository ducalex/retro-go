#pragma once

#include "gnuboy.h"

typedef struct
{
	byte vbank[2][8192];
	byte oam[256];
	union {
		uint16_t palette[64];
		uint8_t pal[128];
	};
	int cycles;
} gb_lcd_t;

extern gb_lcd_t lcd;

gb_lcd_t *lcd_init(void);
void lcd_reset(bool hard);
void lcd_emulate(int cycles);
void lcd_stat_trigger(void);
void lcd_lcdc_change(byte b);
void lcd_pal_dirty(void);