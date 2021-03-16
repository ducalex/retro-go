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
** map087.c: Mapper 87 interface
** Implemented by Firebug
**
*/

#include <nofrendo.h>
#include <mmc.h>
#include <nes.h>


static void map87_write(uint32 address, uint8 value)
{
   UNUSED(address);
/***
   value:  [.... ..LH]
   This reg selects 8k CHR @ $0000.  Note the reversed bit orders.  Most games using this
   mapper only have 16k CHR, so the 'H' bit is usually unused.
***/
   mmc_bankvrom(8, 0x0000, (value & 3) >> 1);
}

static const mem_write_handler_t map87_memwrite[] =
{
   { 0x6000, 0x7FFF, map87_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map87_intf =
{
   87,               /* Mapper number */
   "Mapper 087",     /* Mapper name */
   NULL,             /* Initialization routine */
   NULL,             /* VBlank callback */
   NULL,             /* HBlank callback */
   NULL,             /* Get state (SNSS) */
   NULL,             /* Set state (SNSS) */
   NULL,             /* Memory read structure */
   map87_memwrite,   /* Memory write structure */
   NULL              /* External sound device */
};
