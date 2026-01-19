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
** map165.c: Fire Emblem mapper
** Code adapted from VirtuaNES (GPLv2)
**
*/

#include "nes/nes.h"

static uint8_t reg[8];
static uint8_t prg0, prg1;
static uint8_t chr0, chr1, chr2, chr3;
static uint8_t we_sram;
static uint8_t latch;

static void SetPROM_8K_Bank(int page, int bank)
{
    // CPU_MEM_BANK[page] = PROM+0x2000*bank;
    // CPU_MEM_TYPE[page] = BANKTYPE_ROM;
    // CPU_MEM_PAGE[page] = bank;
    mmc_bankprg(8, 0x2000 * page, bank, PRG_ROM);
}

static void SetBank_CPU(void)
{
    SetPROM_8K_Bank(4, prg0);
    SetPROM_8K_Bank(5, prg1);
    SetPROM_8K_Bank(6, -2); // PROM_8K_SIZE - 2
    SetPROM_8K_Bank(7, -1); // PROM_8K_SIZE - 1
}

static void SetBank_PPUSUB(int bank, int page)
{
    if (page == 0)
    {
        // SetCRAM_4K_Bank(bank, page >> 2);
        mmc_bankchr(4, 0x0400 * bank, page >> 2, CHR_RAM);
    }
    else
    {
        // SetVROM_4K_Bank(bank, page >> 2);
        mmc_bankchr(4, 0x0400 * bank, page >> 2, CHR_ROM);
    }
}

static void SetBank_PPU(void)
{
    if (latch == 0xFD)
    {
        SetBank_PPUSUB(0, chr0);
        SetBank_PPUSUB(4, chr2);
    }
    else
    {
        SetBank_PPUSUB(0, chr1);
        SetBank_PPUSUB(4, chr3);
    }
}

static void PPU_ChrLatch(uint32 addr, uint8 data)
{
    uint32_t mask = addr & 0x1FF0;

    if (mask == 0x1FD0)
    {
        latch = 0xFD;
        SetBank_PPU();
    }
    else if (mask == 0x1FE0)
    {
        latch = 0xFE;
        SetBank_PPU();
    }
}

static void map_write(uint32 addr, uint8 data)
{
    switch (addr & 0xE001)
    {
    case 0x8000:
        reg[0] = data;
        SetBank_CPU();
        SetBank_PPU();
        break;
    case 0x8001:
        reg[1] = data;

        switch (reg[0] & 0x07)
        {
        case 0x00:
            chr0 = data & 0xFC;
            if (latch == 0xFD)
                SetBank_PPU();
            break;
        case 0x01:
            chr1 = data & 0xFC;
            if (latch == 0xFE)
                SetBank_PPU();
            break;

        case 0x02:
            chr2 = data & 0xFC;
            if (latch == 0xFD)
                SetBank_PPU();
            break;
        case 0x04:
            chr3 = data & 0xFC;
            if (latch == 0xFE)
                SetBank_PPU();
            break;

        case 0x06:
            prg0 = data;
            SetBank_CPU();
            break;
        case 0x07:
            prg1 = data;
            SetBank_CPU();
            break;
        }
        break;
    case 0xA000:
        reg[2] = data;
        if (data & 0x01)
            ppu_setmirroring(PPU_MIRROR_HORI);
        else
            ppu_setmirroring(PPU_MIRROR_VERT);
        break;
    case 0xA001:
        reg[3] = data;
        break;
    default:
        break;
    }
}

static void map_getstate(uint8 *state)
{
    for (int i = 0; i < 8; i++)
        state[i] = reg[i];
    state[8] = prg0;
    state[9] = prg1;
    state[10] = chr0;
    state[11] = chr1;
    state[12] = chr2;
    state[13] = chr3;
    state[14] = latch;
}

static void map_setstate(uint8 *state)
{
    for (int i = 0; i < 8; i++)
        reg[i] = state[i];
    prg0 = state[8];
    prg1 = state[9];
    chr0 = state[10];
    chr1 = state[11];
    chr2 = state[12];
    chr3 = state[13];
    latch = state[14];
}

static void map_init(rom_t *cart)
{
    for (int i = 0; i < 8; i++)
        reg[i] = 0x00;
    prg0 = 0;
    prg1 = 1;
    SetBank_CPU();

    chr0 = 0;
    chr1 = 0;
    chr2 = 4;
    chr3 = 4;
    latch = 0xFD;
    SetBank_PPU();

    we_sram = 0; // Disable

    ppu_setlatchfunc(PPU_ChrLatch);
}

mapintf_t map165_intf =
    {
        .number = 165,
        .name = "Mapper 165",
        .init = map_init,
        .vblank = NULL,
        .hblank = NULL,
        .get_state = map_getstate,
        .set_state = map_setstate,
        .mem_read = {},
        .mem_write = {
            {0x8000, 0xFFFF, map_write},
        },
};
