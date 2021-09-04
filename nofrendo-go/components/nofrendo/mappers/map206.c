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
** map206.c: NAMCOT-118 mapper interface
** Implemented by ducalex with the help of nesdevwiki
**
*/

#include <nofrendo.h>
#include <mmc.h>

static uint8 reg8000;

typedef struct
{
   uint8 reg8000;
} mapperState;


static void map_write(uint32 address, uint8 value)
{
   switch (address & 0xE001)
   {
   case 0x8000: // Bank select
      reg8000 = value;
      break;

   case 0x8001: // Bank data
      switch (reg8000 & 0x07)
      {
      case 0: // Select 2 KB CHR bank at PPU $0000-$07FF
         mmc_bankchr(2, 0x0000, value & 0x1F, CHR_ROM);
         break;

      case 1: // Select 2 KB CHR bank at PPU $0800-$0FFF
         mmc_bankchr(2, 0x0000, value & 0x1F, CHR_ROM);
         break;

      case 2: // Select 1 KB CHR bank at PPU $1000-$13FF
         mmc_bankchr(1, 0x1000, value & 0x3F, CHR_ROM);
         break;

      case 3: // Select 1 KB CHR bank at PPU $1400-$17FF
         mmc_bankchr(1, 0x1400, value & 0x3F, CHR_ROM);
         break;

      case 4: // Select 1 KB CHR bank at PPU $1800-$1BFF
         mmc_bankchr(1, 0x1800, value & 0x3F, CHR_ROM);
         break;

      case 5: // Select 1 KB CHR bank at PPU $1C00-$1FFF
         mmc_bankchr(1, 0x1C00, value & 0x3F, CHR_ROM);
         break;

      case 6: // Select 8 KB PRG ROM bank at $8000-$9FFF
         mmc_bankprg(8, 0x8000, value & 0xF, PRG_ROM);
         break;

      case 7: // Select 8 KB PRG ROM bank at $A000-$BFFF
         mmc_bankprg(8, 0xA000, value & 0xF, PRG_ROM);
         break;
      }
      break;

   default:
      MESSAGE_DEBUG("map206: unhandled write: address=%p, value=0x%x\n", (void*)address, value);
      break;
   }
}

static void map_getstate(void *state)
{
   ((mapperState*)state)->reg8000 = reg8000;
}

static void map_setstate(void *state)
{
   reg8000 = ((mapperState*)state)->reg8000;
}

static void map_init(rom_t *cart)
{
   // The MMC already mapped the fixed PRG correctly by now, do nothing.
   // mmc_bankprg(8, 0xC000, -2, PRG_ROM);
   // mmc_bankprg(8, 0xE000, -1, PRG_ROM);
   reg8000 = 0;
}

static const mem_write_handler_t map_memwrite[] =
{
   { 0x8000, 0xFFFF, map_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map206_intf =
{
   206,              /* mapper number */
   "NAMCOT-118",     /* mapper name */
   map_init,         /* init routine */
   NULL,             /* vblank callback */
   NULL,             /* hblank callback */
   map_getstate,     /* get state (snss) */
   map_setstate,     /* set state (snss) */
   NULL,             /* memory read structure */
   map_memwrite,     /* memory write structure */
   NULL              /* external sound device */
};
