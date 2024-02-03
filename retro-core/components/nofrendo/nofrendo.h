/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** nofrendo.h (c) 1998-2000 Matthew Conte (matt@conte.com)
**            (c) 2000 Neil Stevens (multivac@fcmail.com)
**
*/

#pragma once

#define APP_STRING  "Nofrendo"
#define APP_VERSION "3.0"

/* Configuration */

/* Uncomment to enable debugging messages */
// #define NOFRENDO_DEBUG

/* Uncomment to enable live dissassembler */
// #define NES6502_DISASM

/* Uncomment to save/load a game's SRAM to disk */
// #define USE_SRAM_FILE

/* Uncomment on big-endian machines */
// #define IS_BIG_ENDIAN

/* End configuration */

#include <stdbool.h>

typedef enum
{
    NES_PALETTE_NOFRENDO = 0,
    NES_PALETTE_COMPOSITE,
    NES_PALETTE_NESCLASSIC,
    NES_PALETTE_NTSC,
    NES_PALETTE_PVM,
    NES_PALETTE_SMOOTH,
    NES_PALETTE_COUNT,
} nespal_t;

enum
{
    // Start at 192 because it's unlikely that a pixel will have both BG_TRANS|SP_PIXEL
    NES_GUI_BLACK = 192,
    NES_GUI_DKGRAY,
    NES_GUI_GRAY,
    NES_GUI_LTGRAY,
    NES_GUI_WHITE,
    NES_GUI_RED,
    NES_GUI_GREEN,
    NES_GUI_BLUE,
};

int nofrendo_init(int system, int sample_rate, bool stereo, void *blit, void *vsync, void *input);
int nofrendo_start(const char *filename, const char *savefile);
void nofrendo_stop(void);
void nofrendo_pause(bool pause);
void *nofrendo_buildpalette(nespal_t n, int bitdepth);
