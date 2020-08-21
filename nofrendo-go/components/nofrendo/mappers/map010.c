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
** map010.c
**
** mapper 10 interface
** $Id: map010.c,v 1.0 2018/07/07
*/

#include <nofrendo.h>
#include <nes_mmc.h>
#include <nes_ppu.h>
#include <string.h>

static uint8 latch[2];
static uint8 regs[4];

// Shouldn't that be packed? (It wasn't packed in SNSS...)
typedef struct
{
   unsigned char latch[2];
   unsigned char lastB000Write;
   unsigned char lastC000Write;
   unsigned char lastD000Write;
   unsigned char lastE000Write;
} mapper10Data;

/* Used when tile $FD/$FE is accessed */
static void mmc10_latchfunc(uint32 address, uint8 value)
{
   if (0xFD == value || 0xFE == value)
   {
      int reg;

      if (address)
      {
         latch[1] = value;
         reg = 2 + (value - 0xFD);
      }
      else
      {
         latch[0] = value;
         reg = value - 0xFD;
      }

      mmc_bankvrom(4, address, regs[reg]);
   }
}

/* mapper 10: MMC4 */
/* MMC4: Fire Emblem */
static void map10_write(uint32 address, uint8 value)
{
   switch ((address & 0xF000) >> 12)
   {
   case 0xA:
      mmc_bankrom(16, 0x8000, value);
      break;

   case 0xB:
      regs[0] = value;
      if (0xFD == latch[0])
         mmc_bankvrom(4, 0x0000, value);
      break;

   case 0xC:
      regs[1] = value;
      if (0xFE == latch[0])
         mmc_bankvrom(4, 0x0000, value);
      break;

   case 0xD:
      regs[2] = value;
      if (0xFD == latch[1])
         mmc_bankvrom(4, 0x1000, value);
      break;

   case 0xE:
      regs[3] = value;
      if (0xFE == latch[1])
         mmc_bankvrom(4, 0x1000, value);
      break;

   case 0xF:
      if (value & 1)
         ppu_setmirroring(PPU_MIRROR_HORI);
      else
         ppu_setmirroring(PPU_MIRROR_VERT);
      break;

   default:
      break;
   }
}

static void map10_init(void)
{
   memset(regs, 0, sizeof(regs));

   mmc_bankrom(16, 0x8000, 0);
   mmc_bankrom(16, 0xC000, MMC_LASTBANK);

   latch[0] = 0xFE;
   latch[1] = 0xFE;

   ppu_setlatchfunc(mmc10_latchfunc);
}

static void map10_getstate(void *state)
{
   ((mapper10Data*)state)->latch[0] = latch[0];
   ((mapper10Data*)state)->latch[1] = latch[1];
   ((mapper10Data*)state)->lastB000Write = regs[0];
   ((mapper10Data*)state)->lastC000Write = regs[1];
   ((mapper10Data*)state)->lastD000Write = regs[2];
   ((mapper10Data*)state)->lastE000Write = regs[3];
}

static void map10_setstate(void *state)
{
   latch[0] = ((mapper10Data*)state)->latch[0];
   latch[1] = ((mapper10Data*)state)->latch[1];
   regs[0]  = ((mapper10Data*)state)->lastB000Write;
   regs[1]  = ((mapper10Data*)state)->lastC000Write;
   regs[2]  = ((mapper10Data*)state)->lastD000Write;
   regs[3]  = ((mapper10Data*)state)->lastE000Write;
}

static mem_write_handler_t map10_memwrite[] =
{
   { 0x8000, 0xFFFF, map10_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map10_intf =
{
   10, /* mapper number */
   "MMC4", /* mapper name */
   map10_init, /* init routine */
   NULL, /* vblank callback */
   NULL, /* hblank callback */
   map10_getstate, /* get state (snss) */
   map10_setstate, /* set state (snss) */
   NULL, /* memory read structure */
   map10_memwrite, /* memory write structure */
   NULL /* external sound device */
};

/*
** $Log: map010.c,v $
** Revision 1.0  2018/07/07
** initial revision based on map009.c
**
*/
