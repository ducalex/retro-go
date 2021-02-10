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
** Note: all architectures should call these functions
**
** $Id: nofrendo.h,v 1.2 2001/04/27 11:10:08 neil Exp $
*/

#ifndef _NOFRENDO_H_
#define _NOFRENDO_H_

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

#undef PATH_MAX
#define PATH_MAX 512
#undef PATH_SEP
#define PATH_SEP '/'

#ifndef __PACKED__
#define __PACKED__ __attribute__((packed))
#endif

#define INLINE static inline __attribute__((__always_inline__))

#if !defined(MIN)
#define MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#define MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })
#endif

#ifdef NOFRENDO_DEBUG
#define UNUSED(x)
#define ASSERT(expr) while (!(expr)) { osd_log(1, "ASSERTION FAILED IN %s: " #expr "\n", __func__); abort(); }
#define MESSAGE_TRACE(x...)   osd_log(5, "~ %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_DEBUG(x...)   osd_log(4, "> %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_INFO(x, ...)  osd_log(3, "* %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_WARN(x, ...)  osd_log(2, "* %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_ERROR(x, ...) osd_log(1, "!! %s: " x, __func__, ## __VA_ARGS__)
#else
#define UNUSED(x) (void)x
#define ASSERT(expr)
#define MESSAGE_DEBUG(x, ...)
#define MESSAGE_INFO(x...)  osd_log(3, x)
#define MESSAGE_WARN(x...)  osd_log(2, " ! " x)
#define MESSAGE_ERROR(x...) osd_log(1, "!! " x)
#endif

/* End macros */

/* Basic types */

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef struct
{
    uint8 r, g, b;
} rgb_t;

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <rg_system.h>
#include <osd.h>
#include <nes.h>

/* End basic types */

extern int nofrendo_start(const char *filename, int region, int sample_rate, bool stereo);
extern void nofrendo_stop(void);

#endif /* !_NOFRENDO_H_ */
