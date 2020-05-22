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
** nes_mmc.h
**
** NES Memory Management Controller (mapper) defines / prototypes
** $Id: nes_mmc.h,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#ifndef _NES_MMC_H_
#define _NES_MMC_H_

typedef struct mapintf_s mapintf_t;
typedef struct mmc_s mmc_t;

#include <nofrendo.h>
#include "nes_state.h"
#include "nes_apu.h"
#include "nes_rom.h"
#include "nes_mem.h"

struct mapintf_s
{
   int number;
   const char *name;
   void (*init)(void);
   void (*vblank)(void);
   void (*hblank)(int scanline);
   void (*get_state)(void *state); // State is a 128 bytes buffer
   void (*set_state)(void *state); // State is a 128 bytes buffer
   mem_read_handler_t *mem_read;
   mem_write_handler_t *mem_write;
   apuext_t *sound_ext;
};

struct mmc_s
{
   mapintf_t *intf;
   rominfo_t *cart;  /* link it back to the cart */
   uint8 *prg, *chr;
   uint8 prg_banks, chr_banks;
};

#define MMC_LASTBANK      -1

extern void mmc_bankwram(int size, uint32 address, int bank);
extern void mmc_bankvrom(int size, uint32 address, int bank);
extern void mmc_bankrom(int size, uint32 address, int bank);

extern mmc_t *mmc_init(rominfo_t *rominfo);
extern void mmc_refresh(void);
extern void mmc_reset(void);
extern void mmc_shutdown(void);

extern mapintf_t *mmc_peek(int map_num);
extern rominfo_t *mmc_getinfo(void);

extern void mmc_getcontext(mmc_t *dest_mmc);
extern void mmc_setcontext(mmc_t *src_mmc);

#endif /* _NES_MMC_H_ */
