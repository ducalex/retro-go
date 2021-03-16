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
** map003.c: CNROM mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>


static void map3_write(uint32 address, uint8 value)
{
   UNUSED(address);
   mmc_bankvrom(8, 0x0000, value);
}

static const mem_write_handler_t map3_memwrite[] =
{
   { 0x8000, 0xFFFF, map3_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map3_intf =
{
   3,                /* mapper number */
   "CNROM",          /* mapper name */
   NULL,             /* init routine */
   NULL,             /* vblank callback */
   NULL,             /* hblank callback */
   NULL,             /* get state (snss) */
   NULL,             /* set state (snss) */
   NULL,             /* memory read structure */
   map3_memwrite,    /* memory write structure */
   NULL              /* external sound device */
};
