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

#ifndef _NES_MMC_H_
#define _NES_MMC_H_

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
   int number;
   const char *name;
   void (*init)(rom_t *cart);
   void (*vblank)(void);
   void (*hblank)(int scanline);
   void (*get_state)(void *state); // State is a 128 bytes buffer
   void (*set_state)(void *state); // State is a 128 bytes buffer
   const mem_read_handler_t *mem_read;
   const mem_write_handler_t *mem_write;
   apuext_t *sound_ext;
};

#define MMC_LASTBANK      -1

#define mmc_bankvrom(a, b, c) mmc_bankchr(a, b, c, CHR_ANY)
#define mmc_bankrom(a, b, c) mmc_bankprg(a, b, c, PRG_ROM)

extern mapper_t *mmc_init(rom_t *cart);
extern void mmc_shutdown(void);
extern void mmc_reset(void);
extern void mmc_bankprg(int size, uint32 address, int bank, uint8 *base);
extern void mmc_bankchr(int size, uint32 address, int bank, uint8 *base);

#endif /* _NES_MMC_H_ */
