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
    int elapsed_cycles = 0;

    if (nes.input_func)
    {
        nes.input_func();
    }

    while (nes.scanline < nes.scanlines_per_frame)
    {
        nes.cycles += nes.cycles_per_scanline;

        ppu_scanline(nes.vidbuf, nes.scanline, draw);

        if (nes.scanline == 241)
        {
            /* 7-9 cycle delay between when VINT flag goes up and NMI is taken */
            elapsed_cycles = nes6502_execute(7);
            nes.cycles -= elapsed_cycles;

            if (nes.ppu->ctrl0 & PPU_CTRL0F_NMI)
                nes6502_nmi();

            if (nes.mapper->vblank)
                nes.mapper->vblank();
        }

        if (nes.mapper->hblank)
            nes.mapper->hblank(nes.scanline);

        if (nes.timer_func == NULL)
        {
            elapsed_cycles = nes6502_execute(nes.cycles);
            apu_fc_advance(elapsed_cycles);
            nes.cycles -= elapsed_cycles;
        }
        else
        {
            while (nes.cycles >= 1)
            {
                elapsed_cycles = nes6502_execute(MIN(nes.timer_period, nes.cycles));
                apu_fc_advance(elapsed_cycles);
                nes.timer_func(elapsed_cycles);
                nes.cycles -= elapsed_cycles;
            }
        }

        ppu_endscanline();
        nes.scanline++;
    }

    nes.scanline = 0;

    if (draw && nes.blit_func)
    {
        nes.blit_func(nes.vidbuf);
        nes.vidbuf = nes.framebuffers[nes.vidbuf == nes.framebuffers[0]];
    }

    apu_emulate();

    if (nes.vsync_func)
    {
        nes.vsync_func();
    }
}

/* This sets a timer to be fired every `period` cpu cycles. It is NOT accurate. */
void nes_settimer(nes_timer_t *func, long period)
{
    nes.timer_func = func;
    nes.timer_period = period;
}

void nes_poweroff(void)
{
    nes.poweroff = true;
}

void nes_togglepause(void)
{
    nes.pause ^= true;
}

void nes_setcompathacks(void)
{
    // Hack to fix many MMC3 games with status bar vertical alignment issues
    // The issue is that the CPU and PPU aren't running in sync
    if (nes.cart->checksum == 0xD8578BFD || // Zen Intergalactic
        nes.cart->checksum == 0x2E6301ED || // Super Mario Bros 3
        nes.cart->checksum == 0x5ED6F221 || // Kirby's Adventure
        nes.cart->checksum == 0xD273B409)   // Power Blade 2
    {
        nes.cycles_per_scanline += 2.5;
        MESSAGE_INFO("NES: Enabled MMC3 Timing Hack\n");
    }
}

/* insert a cart into the NES */
int nes_insertcart(const char *filename, const char *biosfile)
{
    int status = 0;

    /* rom file */
    nes.cart = rom_loadfile(filename);
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
    if (nes.cart->flags & ROM_FLAG_FDS_DISK)
    {
        if (biosfile == NULL)
        {
            // TO DO: Try biosfile = dirname(filename) / disksys.rom
            status = -3;
            goto _fail;
        }
        FILE *fp = fopen(biosfile, "rb");
        if (!fp || !fread(nes.cart->prg_rom, ROM_PRG_BANK_SIZE, nes.cart->prg_rom_banks, fp))
        {
            MESSAGE_ERROR("NES: BIOS file load failed from '%s'.\n", biosfile);
            status = -3;
            fclose(fp);
            goto _fail;
        }
        fclose(fp);
        MESSAGE_INFO("NES: BIOS file loaded from '%s'.\n", biosfile);
    }

    nes_setcompathacks();

    nes_reset(true);

    return status;

_fail:
    nes_shutdown();
    return status;
}

/* insert a disk into the FDS */
int nes_insertdisk(const char *filename, const char *biosfile)
{
    return nes_insertcart(filename, biosfile);
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

    nes.vidbuf = nes.framebuffers[0];
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
    free(nes.framebuffers[0]);
    free(nes.framebuffers[1]);
}

/* Initialize NES CPU, hardware, etc. */
nes_t *nes_init(nes_type_t system, int sample_rate, bool stereo)
{
    memset(&nes, 0, sizeof(nes_t));

    nes.poweroff = false;
    nes.pause = false;
    nes.system = system;
    nes.refresh_rate = 60;

    /* Framebuffers */
    nes.framebuffers[0] = rg_alloc(NES_SCREEN_PITCH * NES_SCREEN_HEIGHT, MEM_FAST);
    nes.framebuffers[1] = rg_alloc(NES_SCREEN_PITCH * NES_SCREEN_HEIGHT, MEM_FAST);
    if (NULL == nes.framebuffers[0] || NULL == nes.framebuffers[1])
        goto _fail;

    /* memory */
    nes.mem = mem_init();
    if (NULL == nes.mem)
        goto _fail;

    /* cpu */
    nes.cpu = nes6502_init(nes.mem);
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

    MESSAGE_INFO("NES: System initialized!\n");
    return &nes;

_fail:
    MESSAGE_ERROR("NES: System initialized failed!\n");
    nes_shutdown();
    return NULL;
}
