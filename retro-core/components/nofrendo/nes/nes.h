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
** nes.h: NES console emulation header
**
*/

#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef struct nes_s nes_t;

#include <nofrendo.h>
#include "apu.h"
#include "cpu.h"
#include "ppu.h"
#include "mmc.h"
#include "mem.h"
#include "rom.h"
#include "input.h"
#include "utils.h"

#define NES_SCREEN_WIDTH      (256)
#define NES_SCREEN_HEIGHT     (240)
#define NES_SCREEN_OVERDRAW   (8)
#define NES_SCREEN_PITCH      (8 + 256 + 8)

#define NES_SCREEN_GETPTR(buf, x, y)     ((buf) + ((y) * NES_SCREEN_PITCH) + (x) + NES_SCREEN_OVERDRAW)

#define NES_CPU_CLOCK_NTSC      (1789773)
#define NES_CPU_CLOCK_PAL       (1662607)
#define NES_REFRESH_RATE_NTSC   (60)
#define NES_REFRESH_RATE_PAL    (50)
#define NES_SCANLINES_NTSC      (262)
#define NES_SCANLINES_PAL       (312)

typedef enum
{
    SYS_NES_NTSC,
    SYS_NES_PAL,
    SYS_FAMICOM,
    SYS_UNKNOWN,
    SYS_DETECT = -1,
} nes_type_t;

typedef void (nes_timer_t)(int cycles);

typedef struct nes_s
{
    /* Hardware */
    nes6502_t *cpu;
    ppu_t *ppu;
    apu_t *apu;
    mem_t *mem;
    rom_t *cart;
    mapper_t *mapper;
    input_t *input;

    /* Video buffer */
    uint8 *vidbuf; // [NES_SCREEN_PITCH * NES_SCREEN_HEIGHT]

    /* Misc */
    nes_type_t system;
    int overscan;
    char *fds_bios;

    /* Timing constants */
    int refresh_rate;
    int scanlines_per_frame;
    int cycles_per_scanline;
    int cpu_clock;

    /* Timing counters */
    int scanline;
    int cycles;

    /* Periodic timer */
    nes_timer_t *timer_func;
    int timer_period;

    /* Port functions */
    void (*blit_func)(uint8 *);
} nes_t;

nes_t *nes_getptr(void);
nes_t *nes_init(nes_type_t system, int sample_rate, bool stereo, const char *fds_bios);
uint8 *nes_setvidbuf(uint8 *vidbuf);
void nes_shutdown(void);
int nes_insertcart(rom_t *cart);
int nes_loadfile(const char *filename);
void nes_settimer(nes_timer_t *func, int period);
void nes_emulate(bool draw);
void nes_reset(bool hard_reset);
