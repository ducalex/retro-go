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

#include <noftypes.h>
#include <nes_apu.h>
#include <nes_ppu.h>
#include <nes_mmc.h>
#include <nes_rom.h>
#include <bitmap.h>

#define NES_SCREEN_WIDTH     256
#define NES_SCREEN_HEIGHT    240

#define NES_RAMSIZE 0x800

#define MAX_MEM_HANDLERS     32
#define LAST_MEMORY_HANDLER  { -1, -1, NULL }

#define NES_CLOCK_DIVIDER    12
#define NES_MASTER_CLOCK     (236250000 / 11)
#define NES_SCANLINE_CYCLES  (1364.0 / NES_CLOCK_DIVIDER)
#define NES_FIQ_PERIOD       (NES_MASTER_CLOCK / NES_CLOCK_DIVIDER / 60)

#define nes_getbyte(x) nes6502_getbyte(x)
#define nes_putbyte(x,v) nes6502_putbyte(x,v)
#define nes_getpage(x) nes6502_getpage(x)
#define nes_setpage(x,v) nes6502_setpage(x,v)

typedef enum
{
   NES_AUTO,
   NES_NTSC,
   NES_PAL
} region_t;

enum
{
   SOFT_RESET,
   HARD_RESET,
   ZERO_RESET,
};

typedef struct
{
   uint32 min_range, max_range;
   uint8 (*read_func)(uint32 address);
} mem_read_handler_t;

typedef struct
{
   uint32 min_range, max_range;
   void (*write_func)(uint32 address, uint8 value);
} mem_write_handler_t;

typedef struct
{
   /* Plain memory (RAM/ROM) pages */
   uint8 *pages[16]; // 4KB memory pages

   /* Special memory handlers */
   mem_read_handler_t read_handlers[MAX_MEM_HANDLERS];
   mem_write_handler_t write_handlers[MAX_MEM_HANDLERS];

   /* Whether a page is affected by a special handler */
   /* If true we don't have to loop over the handlers */
   bool skip_handlers[16];

   void (*on_change_cb)();
} mem_map_t;

#include <nes6502.h>

typedef struct nes_s
{
   /* hardware things */
   nes6502_t *cpu;
   ppu_t *ppu;
   apu_t *apu;
   mmc_t *mmc;
   rominfo_t *rominfo;

   /* Memory */
   uint8 ram[NES_RAMSIZE];
   mem_map_t mem;

   /* video buffer */
   bitmap_t *vidbuf;

   /* Misc */
   region_t region;
   short overscan;

   /* Timing stuff */
   short refresh_rate;
   short scanlines;
   float cycles_per_line;

   short scanline;
   float cycles;

   /* control */
   bool autoframeskip;
   bool poweroff;
   bool pause;

} nes_t;

/* Function prototypes */
extern nes_t *nes_getptr(void);
extern nes_t *nes_create(region_t region);
extern int nes_insertcart(const char *filename);
extern void nes_destroy();
extern void nes_emulate(void);
extern void nes_reset(int reset_type);
extern void nes_poweroff(void);
extern void nes_togglepause(void);

#endif /* _NES_H_ */
