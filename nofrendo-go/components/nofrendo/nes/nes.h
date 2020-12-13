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
** nes.h
**
** NES hardware related definitions / prototypes
** $Id: nes.h,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#ifndef _NES_H_
#define _NES_H_

#include <nofrendo.h>
#include <nes_apu.h>
#include <nes_ppu.h>
#include <nes_mmc.h>
#include <nes_mem.h>
#include <nes_rom.h>
#include <nes6502.h>
#include <bitmap.h>

#define NES_SCREEN_WIDTH     256
#define NES_SCREEN_HEIGHT    240

#define NES_CPU_CLOCK_NTSC    1789772.72727
#define NES_CPU_CLOCK_PAL     1662607.03125
#define NES_REFRESH_RATE_NTSC 60
#define NES_REFRESH_RATE_PAL  50
#define NES_SCANLINES_NTSC    262
#define NES_SCANLINES_PAL     312

typedef enum
{
   NES_AUTO,
   NES_NTSC,
   NES_PAL
} region_t;

typedef enum
{
   SOFT_RESET,
   HARD_RESET,
   ZERO_RESET,
} reset_type_t;

typedef struct nes_s
{
   /* Hardware */
   nes6502_t *cpu;
   ppu_t *ppu;
   apu_t *apu;
   mmc_t *mmc;
   rominfo_t *rominfo;

   /* Memory */
   mem_t *mem;

   /* Video buffer */
   bitmap_t *vidbuf;

   /* Misc */
   region_t region;
   short overscan;

   /* Timing stuff */
   short refresh_rate;
   short scanlines_per_frame;
   float cycles_per_line;

   short scanline;
   float cycles;

   /* Control */
   bool autoframeskip;
   bool poweroff;
   bool pause;
   bool drawframe;

} nes_t;

/* Function prototypes */
extern nes_t *nes_getptr(void);
extern int  nes_init(region_t region, int sample_rate, bool stereo);
extern void nes_shutdown(void);
extern void nes_setregion(region_t region);
extern int  nes_insertcart(const char *filename);
extern void nes_emulate(void);
extern void nes_reset(reset_type_t reset_type);
extern void nes_poweroff(void);
extern void nes_togglepause(void);

#endif /* _NES_H_ */
