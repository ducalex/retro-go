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
** nes.c: NES console emulation
**
*/

#include "nes.h"

static nes_t nes;

nes_t *nes_getptr(void)
{
    return &nes;
}

/* run emulation for one frame */
void nes_emulate(bool draw)
{
    draw = draw && nes.vidbuf != NULL;

    while (nes.scanline < nes.scanlines_per_frame)
    {
        // Running a little bit ahead seems to fix both Battletoads games...
        int elapsed_cycles = nes6502_execute(86 - 12);

        ppu_renderline(nes.vidbuf, nes.scanline, draw);

        if (nes.scanline == 241)
        {
            elapsed_cycles += nes6502_execute(6);
            if (nes.ppu->ctrl0 & PPU_CTRL0F_NMI)
                nes6502_nmi();

            if (nes.mapper->vblank)
                nes.mapper->vblank(&nes);
        }

        if (nes.mapper->hblank)
        {
            // Mappers use various techniques to detect horizontal blank and we can't accommodate
            // all of them unfortunately. But ~86 cycles seems to work fine for everything tested.
            elapsed_cycles += nes6502_execute(86 - elapsed_cycles);
            nes.mapper->hblank(&nes);
        }

        nes.cycles += nes.cycles_per_scanline;

        elapsed_cycles += nes6502_execute(nes.cycles - elapsed_cycles);
        apu_fc_advance(elapsed_cycles);
        nes.cycles -= elapsed_cycles;

        ppu_endline();
        nes.scanline++;
    }

    nes.scanline = 0;

    if (draw && nes.blit_func)
        nes.blit_func(nes.vidbuf);

    apu_emulate();
}

uint8 *nes_setvidbuf(uint8 *vidbuf)
{
    uint8 *prevbuf = nes.vidbuf;
    nes.vidbuf = vidbuf;
    return prevbuf;
}

/* This sets a timer to be fired every `period` cpu cycles. It is NOT accurate. */
void nes_settimer(nes_timer_t *func, int period)
{
    nes.timer_func = func;
    nes.timer_period = period;
}

/* insert a cart into the NES */
int nes_insertcart(rom_t *cart)
{
    int status = 0;

    /* rom file */
    nes.cart = cart;
    if (NULL == nes.cart)
    {
        status = -1;
        goto _fail;
    }

    /* mapper */
    nes.mapper = mmc_init(nes.cart);
    if (NULL == nes.mapper)
    {
        status = -2;
        goto _fail;
    }

    /* Detect system type */
    if (nes.system == SYS_DETECT && nes.cart->system != SYS_UNKNOWN)
    {
        nes.system = nes.cart->system;
    }

    if (nes.system == SYS_UNKNOWN)
        MESSAGE_INFO("NES: System type: UNKNOWN  (NTSC-U)\n");
    else if (nes.system == SYS_NES_NTSC)
        MESSAGE_INFO("NES: System type: NES-NTSC (NTSC-U)\n");
    else if (nes.system == SYS_NES_PAL)
        MESSAGE_INFO("NES: System type: NES-PAL  (PAL)\n");
    else if (nes.system == SYS_FAMICOM)
        MESSAGE_INFO("NES: System type: FAMICOM  (NTSC-J)\n");

    /* Apply proper system timings (See https://wiki.nesdev.com/w/index.php/Cycle_reference_chart) */
    if (nes.system == SYS_NES_PAL)
    {
        nes.cpu_clock = NES_CPU_CLOCK_PAL;
        nes.refresh_rate = NES_REFRESH_RATE_PAL;
        nes.scanlines_per_frame = NES_SCANLINES_PAL;
        nes.cycles_per_scanline = nes.cpu_clock / nes.refresh_rate / nes.scanlines_per_frame;
        nes.overscan = 0;
    }
    else
    {
        nes.cpu_clock = NES_CPU_CLOCK_NTSC;
        nes.refresh_rate = NES_REFRESH_RATE_NTSC;
        nes.scanlines_per_frame = NES_SCANLINES_NTSC;
        nes.cycles_per_scanline = nes.cpu_clock / nes.refresh_rate / nes.scanlines_per_frame;
        nes.overscan = 8;
    }

    /* Load BIOS file if required (currently only for Famicom Disk System) */
    if (nes.cart->type == ROM_TYPE_FDS)
    {
        if (nes.fds_bios == NULL)
        {
            // TO DO: Try biosfile = dirname(filename) / disksys.rom
            status = -3;
            goto _fail;
        }
        FILE *fp = fopen(nes.fds_bios, "rb");
        if (!fp || !fread(nes.cart->prg_rom, ROM_PRG_BANK_SIZE, nes.cart->prg_rom_banks, fp))
        {
            MESSAGE_ERROR("NES: BIOS file load failed from '%s'.\n", nes.fds_bios);
            status = -3;
            fclose(fp);
            goto _fail;
        }
        fclose(fp);
        MESSAGE_INFO("NES: BIOS file loaded from '%s'.\n", nes.fds_bios);
    }

    nes_reset(true);

    return status;

_fail:
    nes_shutdown();
    return status;
}

/* Load a ROM file */
int nes_loadfile(const char *filename)
{
    return nes_insertcart(rom_loadfile(filename));
}

/* Reset NES hardware */
void nes_reset(bool hard_reset)
{
    if (hard_reset)
    {
        if (nes.cart->chr_ram_banks > 0)
            memset(nes.cart->chr_ram, 0, nes.cart->chr_ram_banks * ROM_CHR_BANK_SIZE);
        if (nes.cart->prg_ram_banks > 0)
            memset(nes.cart->prg_ram, 0, nes.cart->prg_ram_banks * ROM_PRG_BANK_SIZE);
    }

    apu_reset();
    ppu_reset();
    mem_reset();
    mmc_reset();
    input_reset();
    nes6502_reset();

    nes.vidbuf = NULL;
    nes.scanline = 241;
    nes.cycles = 0;

    /* if we're using VRAM, let the PPU know */
    nes.ppu->vram_present = (nes.cart->chr_rom_banks == 0);

    MESSAGE_INFO("NES: System reset (%s)\n", hard_reset ? "hard" : "soft");
}

/* Shutdown NES */
void nes_shutdown(void)
{
    mmc_shutdown();
    mem_shutdown();
    ppu_shutdown();
    apu_shutdown();
    nes6502_shutdown();
    rom_free();
}

/* Initialize NES CPU, hardware, etc. */
nes_t *nes_init(nes_type_t system, int sample_rate, bool stereo, const char *fds_bios)
{
    memset(&nes, 0, sizeof(nes_t));

    nes.system = system;
    nes.refresh_rate = 60;

    /* memory */
    nes.mem = mem_init_();
    if (NULL == nes.mem)
        goto _fail;

    /* cpu */
    nes.cpu = nes6502_init(nes.mem->pages);
    if (NULL == nes.cpu)
        goto _fail;

    /* ppu */
    nes.ppu = ppu_init();
    if (NULL == nes.ppu)
        goto _fail;

    /* apu */
    nes.apu = apu_init(sample_rate, stereo);
    if (NULL == nes.apu)
        goto _fail;

    /* input */
    nes.input = input_init();
    if (NULL == nes.input)
        goto _fail;

    /* fds */
    if (fds_bios)
        nes.fds_bios = strdup(fds_bios);

    MESSAGE_INFO("NES: System initialized!\n");
    return &nes;

_fail:
    MESSAGE_ERROR("NES: System initialized failed!\n");
    nes_shutdown();
    return NULL;
}
