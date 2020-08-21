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
** map228.c
**
** Mapper 228 interface (Cheetahmen II)
** Implementation by ducalex
**
*/

#include <nofrendo.h>
#include <nes_mmc.h>
#include <nes.h>

static uint8 mram[4];

static void update(uint32 address, uint8 value)
{
	uint16 bank1 = ((address >> 6) & 0x7E) + (((address >> 6) & 1) & ((address >> 5) & 1));
    uint16 bank2 = bank1 + (((address >> 5) & 1) ^ 1);
    uint16 vbank = (((address & 0xF) << 2)) | (value & 0x3);

    // Chip 2 doesn't exist
	if ((bank1 & 0x60) == 0x60) {
		bank1 -= 0x20;
		bank2 -= 0x20;
    }

    mmc_bankrom(16, 0x8000, bank1);
    mmc_bankrom(16, 0xc000, bank2);
    mmc_bankvrom(8, 0x0000, vbank);

    if ((address & 0x2000) == 0) {
        ppu_setmirroring(PPU_MIRROR_VERT);
    } else {
        ppu_setmirroring(PPU_MIRROR_HORI);
    }
}

static void map228_init(void)
{
    update(0x8000, 0x00);
}

static uint8 map228_read(uint32 address)
{
    return mram[address & 3] & 0xF;
}

static void map228_write(uint32 address, uint8 value)
{
    if (address >= 0x4020 && address <= 0x5FFF)
    {
        mram[address & 3] = value;
    }
    else
    {
        update(address, value);
    }
}

static mem_write_handler_t map228_memwrite [] =
{
    { 0x8000, 0xFFFF, map228_write },
    { 0x4020, 0x5FFF, map228_write },
    LAST_MEMORY_HANDLER
};

static mem_read_handler_t map228_memread [] =
{
    { 0x4020, 0x5FFF, map228_read },
    LAST_MEMORY_HANDLER
};

mapintf_t map228_intf =
{
    228,                              /* Mapper number */
    "Mapper 228",                     /* Mapper name */
    map228_init,                      /* Initialization routine */
    NULL,                             /* VBlank callback */
    NULL,                             /* HBlank callback */
    NULL,                             /* Get state (SNSS) */
    NULL,                             /* Set state (SNSS) */
    map228_memread,                   /* Memory read structure */
    map228_memwrite,                  /* Memory write structure */
    NULL                              /* External sound device */
};
