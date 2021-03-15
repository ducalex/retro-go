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
** map020.c
**
** Famicom Disk System
** Implementation by ducalex
**
** Mapper 20 is reserved by iNES for internal usage when emulating an FDS game.
** The FDS doesn't actually use a mapper, but our mapper infrastructure is a
** convenient place to hook into the memory map, sound, and save state systems.
*/

#include <nofrendo.h>
#include <string.h>
#include <mmc.h>
#include <nes.h>

#define FDS_CLOCK (NES_CPU_CLOCK_NTSC / 2)

typedef struct
{
    uint8 prog[0x8000];
    uint8 bios[0x2000];
    uint8 *disk[8];
    uint8 sides;
    uint8 regs[8];
} fds_t;

static fds_t *fds;


static void fds_hblank(int scanline)
{
    // int a = NES_CYCLES_PER_SCANLINE;
}

static uint8 fds_read(uint32 address)
{
    MESSAGE_INFO("FDS read at %04X\n", address);

    switch (address)
    {
        case 0x4030:            // Disk Status Register 0
            return 0x00;

        case 0x4031:            // Read data register
            return 0x00;

        case 0x4032:            // Disk drive status register
            return 0x00;

        case 0x4033:            // External connector read
            return 0x80;        // bit 7 = battery status

        default:
            return 0x00;
    }
}

static void fds_write(uint32 address, uint8 value)
{
    MESSAGE_INFO("FDS write at %04X: %02X\n", address, value);

	switch (address)
    {
        case 0x4020:
            break;

        case 0x4021:
            break;

        case 0x4022:
            break;

        case 0x4023:
            break;

        case 0x4024:
            break;

        case 0x4025:
            break;
	}

    fds->regs[address & 7] = value;
}

static uint8 fds_sound_read(uint32 address)
{
    return 0x00;
}

static void fds_sound_write(uint32 address, uint8 value)
{
    //
}

static uint8 fds_wave_read(uint32 address)
{
    return 0x00;
}

static void fds_wave_write(uint32 address, uint8 value)
{
    //
}

static void fds_getstate(void *state)
{
    //
}

static void fds_setstate(void *state)
{
    //
}

void fds_init(rom_t *cart)
{
    if (!fds)
    {
        fds = calloc(1, sizeof(fds_t));

        // I'm not sure yet where we should load the bios
        // so it shall be hardcoded here while I work on
        // the actual hardware emulation...
        FILE *fp = fopen("/sd/roms/fds/disksys.rom", "rb");
        fread(fds->bios, 0x2000, 1, fp);
        fclose(fp);
    }

    uint8 *disk_ptr = cart->data_ptr;

    if (memcmp(disk_ptr, FDS_HEAD_MAGIC, 4) == 0)
    {
        fds->sides = ((fdsheader_t *)disk_ptr)->sides;
        disk_ptr += 16;
    }
    else
    {
        fds->sides = cart->data_len / 65500;
    }

    fds->sides = MIN(MAX(fds->sides, 1), 8);

    for (int i = 0; i < fds->sides; i++)
    {
        fds->disk[i] = disk_ptr + (i * 65500);
    }

    // cart->prg_rom = fds->prog;
    // cart->prg_rom_banks = 4;

    mmc_bankprg(32, 0x6000, 0, fds->prog); // PRG-RAM 0x6000-0xDFFF
    mmc_bankprg(8,  0xE000, 0, fds->bios); // BIOS 0xE000-0xFFFF
}

static const mem_write_handler_t fds_memwrite[] =
{
    {0x4020, 0x4027, fds_write},
    {0x4040, 0x407F, fds_wave_write},
    {0x4080, 0x408A, fds_sound_write},
    LAST_MEMORY_HANDLER
};

static const mem_read_handler_t fds_memread[] =
{
    {0x4030, 0x4037, fds_read},
    {0x4040, 0x407F, fds_wave_read},
    {0x4090, 0x4092, fds_sound_read},
    LAST_MEMORY_HANDLER
};

mapintf_t map20_intf =
{
    20,                     /* mapper number */
    "Famicom Disk System",  /* mapper name */
    fds_init,               /* init routine */
    NULL,                   /* vblank callback */
    fds_hblank,             /* hblank callback */
    fds_getstate,           /* get state (snss) */
    fds_setstate,           /* set state (snss) */
    fds_memread,            /* memory read structure */
    fds_memwrite,           /* memory write structure */
    NULL                    /* external sound device */
};
