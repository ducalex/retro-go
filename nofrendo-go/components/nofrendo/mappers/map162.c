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
** map162.c: Mapper 162 and 163 interface (Nanjing games)
** by ducalex
**
*/

#include <nofrendo.h>
#include <mmc.h>
#include <nes.h>

static uint8 reg5000;
static uint8 reg5100;
static uint8 reg5101;
static uint8 reg5200;
static uint8 reg5300;
static uint8 trigger;


static void map_update()
{
    uint8 bank = (reg5200 & 0x3) << 4 | (reg5000 & 0xF);
    mmc_bankrom(32, 0x8000, bank);
    // mmc_bankvrom(8, 0x0000, 0);
}

static void map_hblank(int scanline)
{
    if ((reg5000 & 0x80))
    {
        if (scanline == 127)
        {
            mmc_bankvrom(4, 0x0000, 1);
            mmc_bankvrom(4, 0x1000, 1);
        }
        else if (scanline == 239)
        {
            mmc_bankvrom(4, 0x0000, 0);
            mmc_bankvrom(4, 0x1000, 0);
        }
    }
}

static uint8 map_reg_read(uint32 address)
{
    switch (address & 0x7300)
    {
    case 0x5100:
        return reg5300; // | reg5200 | reg5000 | (reg5100 ^ 0xFF);

    case 0x5500:
        return trigger ? reg5300 : 0;

    default:
        MESSAGE_ERROR("mapper 162: unhandled read to $%04X\n", (unsigned)address);
        return 0x04;
    }
}

static void map_reg_write(uint32 address, uint8 value)
{
    switch (address & 0x7300)
    {
    case 0x5000:
        reg5000 = value;
        if (!(reg5000 & 0x80) && NES_CURRENT_SCANLINE < 128)
        {
            mmc_bankvrom(8, 0x0000, 0);
        }
        map_update();
        break;

    case 0x5100:
        if (address == 0x5101) // Copy protection
        {
            if (reg5101 && !value)
                trigger = !trigger;
            reg5101 = value;
        }
        else if (value == 0x6)
        {
            mmc_bankrom(32, 0x8000, 3);
        }
        reg5100 = value;
        break;

    case 0x5200:
        reg5200 = value;
        map_update();
        break;

    case 0x5300:
        reg5300 = value;
        break;

    default:
        MESSAGE_ERROR("mapper 162: unhandled write $%02X to $%04X\n", value, (unsigned)address);
    }
}

static void map_init(rom_t *cart)
{
    reg5000 = 0;
    reg5100 = 1;
    reg5101 = 1;
    reg5200 = 0;
    reg5300 = 0;
    trigger = 0;
    mmc_bankvrom(4, 0x0000, 0);
    mmc_bankvrom(4, 0x1000, 0);
    map_update();
}


mapintf_t map162_intf =
{
    .number     = 162,
    .name       = "Nanjing 162",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = map_hblank,
    .get_state  = NULL,
    .set_state  = NULL,
    .mem_read   = {{0x5000, 0x5FFF, map_reg_read}},
    .mem_write  = {{0x5000, 0x5FFF, map_reg_write}},
};

mapintf_t map163_intf =
{
    .number     = 163,
    .name       = "Nanjing 163",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = map_hblank,
    .get_state  = NULL,
    .set_state  = NULL,
    .mem_read   = {{0x5000, 0x5FFF, map_reg_read}},
    .mem_write  = {{0x5000, 0x5FFF, map_reg_write}},
};
