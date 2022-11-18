#pragma once

#include "gnuboy.h"

void lcd_init(void);
void lcd_reset(bool hard);
void lcd_emulate(int cycles);
void lcd_stat_trigger(void);
void lcd_lcdc_change(byte b);
void lcd_pal_dirty(void);