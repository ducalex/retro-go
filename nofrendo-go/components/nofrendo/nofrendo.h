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
/* uncomment option to enable */

/* Enable debugging messages */
// #define NOFRENDO_DEBUG

/* Enable live dissassembler */
// #define NES6502_DISASM

/* Save/load a game's SRAM from disk */
// #define USE_SRAM_FILE

/* Undef this if running on big-endian (68k) systems */
// #define IS_LITTLE_ENDIAN

/* End configuration */

/* Macros */

#define INLINE static inline __attribute__((__always_inline__))

#if !defined(MIN)
#define MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#define MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })
#endif

#ifdef NOFRENDO_DEBUG
#define UNUSED(x)
#define ASSERT(expr) while (!(expr)) { nofrendo_log(1, "ASSERTION FAILED IN %s: " #expr "\n", __func__); abort(); }
#define MESSAGE_TRACE(x...)   nofrendo_log(5, "~ %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_DEBUG(x...)   nofrendo_log(4, "> %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_INFO(x, ...)  nofrendo_log(3, "* %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_WARN(x, ...)  nofrendo_log(2, "* %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_ERROR(x, ...) nofrendo_log(1, "!! %s: " x, __func__, ## __VA_ARGS__)
#else
#define UNUSED(x) (void)x
#define ASSERT(expr)
#define MESSAGE_DEBUG(x, ...)
#define MESSAGE_INFO(x...)  nofrendo_log(3, x)
#define MESSAGE_WARN(x...)  nofrendo_log(2, " ! " x)
#define MESSAGE_ERROR(x...) nofrendo_log(1, "!! " x)
#endif

/* End macros */

/* Basic types */

#include <stdbool.h>
#include <stdint.h>

typedef int8_t int8;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

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

#include <rg_system.h>
#include <nes.h>

/* End basic types */

int nofrendo_init(int system, int sample_rate, bool stereo, void *blit, void *vsync, void *input);
int nofrendo_start(const char *filename, const char *savefile);
void nofrendo_stop(void);
void nofrendo_log(int type, const char *format, ...);
void *nofrendo_buildpalette(nespal_t n, int bitdepth);
