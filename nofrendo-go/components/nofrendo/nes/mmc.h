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
** nes/mmc.h: Mapper emulation header
**
*/

#pragma once

typedef struct mapper_s mapper_t;
typedef const mapper_t mapintf_t;

#include <nofrendo.h>
#include "state.h"
#include "apu.h"
#include "rom.h"
#include "mem.h"

#define PRG_RAM ((void*)1)
#define PRG_ROM ((void*)2)
#define PRG_ANY ((void*)3)
#define CHR_RAM ((void*)4)
#define CHR_ROM ((void*)5)
#define CHR_ANY ((void*)6)

struct mapper_s
{
   int number;                         /* mapper number */
   const char *name;                   /* mapper name */
   void (*init)(rom_t *cart);          /* init routine */
   void (*vblank)(void);               /* vblank callback */
   void (*hblank)(int scanline);       /* hblank callback */
   void (*get_state)(uint8 *state);    /* get state (uint8 *(state[128])) */
   void (*set_state)(uint8 *state);    /* set state (uint8 *(state[128])) */
   mem_read_handler_t mem_read[4];     /* memory read structure */
   mem_write_handler_t mem_write[4];   /* memory write structure */
   // apuext_t *sound_ext;                /* external sound device */
   // ppu_latchfunc_t ppu_latch;
   // ppu_vreadfunc_t ppu_vread;
};

#define MMC_LASTBANK      -1

#define mmc_bankvrom(a, b, c) mmc_bankchr(a, b, c, CHR_ANY)
#define mmc_bankrom(a, b, c) mmc_bankprg(a, b, c, PRG_ROM)

mapper_t *mmc_init(rom_t *cart);
void mmc_shutdown(void);
void mmc_reset(void);
void mmc_bankprg(unsigned size, unsigned address, int bank, uint8 *base);
void mmc_bankchr(unsigned size, unsigned address, int bank, uint8 *base);
