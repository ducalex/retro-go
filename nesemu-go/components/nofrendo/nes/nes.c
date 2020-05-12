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
** nes.c
**
** NES hardware related routines
** $Id: nes.c,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#include <string.h>
#include <noftypes.h>
#include <nofrendo.h>
#include <nes_input.h>
#include <osd.h>
#include <nes.h>

#include <odroid_system.h>

static bitmap_t *framebuffers[2];
static nes_t nes;


nes_t *nes_getptr(void)
{
   return &nes;
}

/* Emulate one frame and render if draw_flag is true */
INLINE void renderframe(bool draw_flag)
{
   int elapsed_cycles;
   mapintf_t *mapintf = nes.mmc->intf;

   while (nes.scanline != nes.scanlines)
   {
      nes.cycles += nes.cycles_per_line;

      ppu_scanline(nes.vidbuf, nes.scanline, draw_flag);

      if (nes.scanline == 241)
      {
         /* 7-9 cycle delay between when VINT flag goes up and NMI is taken */
         elapsed_cycles = nes6502_execute(7);
         nes.cycles -= elapsed_cycles;

         ppu_checknmi();

         if (mapintf->vblank)
            mapintf->vblank();
      }

      if (mapintf->hblank)
         mapintf->hblank(nes.scanline);

      elapsed_cycles = nes6502_execute(nes.cycles);
      apu_fc_advance(elapsed_cycles);
      nes.cycles -= elapsed_cycles;

      ppu_endscanline(nes.scanline);
      nes.scanline++;
   }

   nes.scanline = 0;
}

/* main emulation loop */
void nes_emulate(void)
{
   const int audioSamples = nes.apu->sample_rate / nes.refresh_rate;
   const int frameTime = get_frame_time(nes.refresh_rate);
   uint skipFrames = 0;

   // Discard the garbage frames
   renderframe(1);
   renderframe(1);

   osd_loadstate();

   while (false == nes.poweroff)
   {
      uint startTime = get_elapsed_time();
      bool drawFrame = !skipFrames;

      osd_getinput();
      renderframe(drawFrame);

      if (drawFrame)
      {
         osd_blitscreen(nes.vidbuf);
         nes.vidbuf = (nes.vidbuf == framebuffers[1]) ? framebuffers[0] : framebuffers[1];
      }

      if (skipFrames == 0)
      {
         if (get_elapsed_time_since(startTime) > frameTime) skipFrames = 1;
         if (speedupEnabled) skipFrames += speedupEnabled * 2;
      }
      else if (skipFrames > 0)
      {
         skipFrames--;
      }

      if (!speedupEnabled)
      {
         osd_audioframe(audioSamples);
      }

      osd_wait_for_vsync();
   }
}

void nes_poweroff(void)
{
   nes.poweroff = true;
}

void nes_togglepause(void)
{
   nes.pause ^= true;
}

/* insert a cart into the NES */
int nes_insertcart(const char *filename)
{
   /* rom file */
   nes.rominfo = rom_load(filename);
   if (NULL == nes.rominfo)
      goto _fail;

   /* mapper */
   nes.mmc = mmc_create(nes.rominfo);
   if (NULL == nes.mmc)
      goto _fail;

   /* if there's VRAM, let the PPU know */
   nes.ppu->vram_present = (NULL != nes.rominfo->vram);

   nes6502_setcontext(nes.cpu);
   apu_setcontext(nes.apu);
   ppu_setcontext(nes.ppu);
   mmc_setcontext(nes.mmc);
   // mem_setcontext(nes.mmc);

   mem_setpage(6, nes.rominfo->sram);
   mem_setpage(7, nes.rominfo->sram + 0x1000);
   mem_setmapper(nes.mmc->intf);

   nes_reset(HARD_RESET);

   return true;

_fail:
   nes_shutdown();
   return false;
}

/* Reset NES hardware */
void nes_reset(reset_type_t reset_type)
{
   if (nes.rominfo->vram)
   {
      memset(nes.rominfo->vram, 0, 0x2000 * nes.rominfo->vram_banks);
   }

   apu_reset();
   ppu_reset();
   mem_reset();
   mmc_reset();
   nes6502_reset();

   nes.vidbuf = framebuffers[0];
   nes.scanline = 241;
   nes.cycles = 0;

   MESSAGE_INFO("NES %s\n", (SOFT_RESET == reset_type) ? "reset" : "powered on");
}

/* Shutdown NES */
void nes_shutdown(void)
{
   mmc_destroy(nes.mmc);
   mem_destroy(nes.mem);
   ppu_destroy(nes.ppu);
   apu_destroy(nes.apu);
   nes6502_destroy(nes.cpu);
   rom_free(nes.rominfo);
   bmp_free(framebuffers[0]);
   bmp_free(framebuffers[1]);
}

/* Initialize NES CPU, hardware, etc. */
int nes_init(region_t region, int sample_rate)
{
   memset(&nes, 0, sizeof(nes_t));

   // https://wiki.nesdev.com/w/index.php/Cycle_reference_chart
   if (region == NES_PAL)
   {
      nes.refresh_rate = NES_REFRESH_RATE_PAL;
      nes.scanlines = NES_SCANLINES_PAL;
      nes.overscan = 0;
      nes.cycles_per_line = 341.f * 5 / 16;
      MESSAGE_INFO("System region: PAL\n");
   }
   else
   {
      nes.refresh_rate = NES_REFRESH_RATE_NTSC;
      nes.scanlines = NES_SCANLINES_NTSC;
      nes.overscan = 8;
      nes.cycles_per_line = 341.f * 4 / 12;
      MESSAGE_INFO("System region: NTSC\n");
   }

   nes.region = region;
   nes.autoframeskip = true;
   nes.poweroff = false;
   nes.pause = false;

   /* Framebuffers */
   framebuffers[0] = bmp_create(NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, 8);
   framebuffers[1] = bmp_create(NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, 8);
   if (NULL == framebuffers[0] || NULL == framebuffers[1])
      goto _fail;

   /* memory */
   nes.mem = mem_create();
   if (NULL == nes.mem)
      goto _fail;

   /* cpu */
   nes.cpu = nes6502_create(nes.mem);
   if (NULL == nes.cpu)
      goto _fail;

   /* apu */
   nes.apu = apu_create(region, sample_rate);
   if (NULL == nes.apu)
      goto _fail;

   /* ppu */
   nes.ppu = ppu_create();
   if (NULL == nes.ppu)
      goto _fail;

   return true;

_fail:
   nes_shutdown();
   return false;
}
