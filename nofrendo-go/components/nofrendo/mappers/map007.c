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
** map7.c
**
** mapper 7 interface
** $Id: map007.c,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#include <nofrendo.h>
#include <nes_mmc.h>
#include <nes_ppu.h>

static int is_battletoads = 0;

/* mapper 7: AOROM */
static void map7_write(uint32 address, uint8 value)
{
   mmc_bankrom(32, 0x8000, value & 0xF);

   if (value & 0x10)
      // ppu_setmirroring(PPU_MIRROR_SCR1);
      ppu_setnametables(1, is_battletoads ? 0 : 1, 1, 1);
   else
      ppu_setmirroring(PPU_MIRROR_SCR0);
}

static void map7_init(void)
{
   map7_write(0x8000, 0);

   if (nes_getptr()->rominfo->checksum == 0x279710DC)
   {
      is_battletoads = 1;
      MESSAGE_INFO("Enabled Battletoads mirroring hack\n");
   }
}

static mem_write_handler_t map7_memwrite[] =
{
   { 0x8000, 0xFFFF, map7_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map7_intf =
{
   7, /* mapper number */
   "AOROM", /* mapper name */
   map7_init, /* init routine */
   NULL, /* vblank callback */
   NULL, /* hblank callback */
   NULL, /* get state (snss) */
   NULL, /* set state (snss) */
   NULL, /* memory read structure */
   map7_memwrite, /* memory write structure */
   NULL /* external sound device */
};
