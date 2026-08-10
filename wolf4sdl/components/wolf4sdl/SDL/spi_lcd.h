#ifndef TFT_LCD_H
#define TFT_LCD_H
#include "stdint.h"
#include "driver/ledc.h"

void spi_lcd_wait_finish();
void spi_lcd_send(uint16_t *scr);
void spi_lcd_send_boarder(uint16_t *scr, int boarder);
void spi_lcd_clear();
void spi_lcd_init();
extern int16_t lcdpal[256];

#endif
