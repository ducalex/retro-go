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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <noftypes.h>
#include <nofrendo.h>
#include <nes_input.h>
#include <osd.h>
#include <nes.h>

#include <esp_attr.h>
#include <odroid_system.h>

static bitmap_t *framebuffers[2];
static nes_t nes;

extern bool fullFrame;


nes_t *nes_getptr(void)
{
   return &nes;
}

IRAM_ATTR void write_protect(uint32 address, uint8 value)
{
   /* don't allow write to go through */
   UNUSED(address);
   UNUSED(value);
}

/* read/write handlers for standard NES */
static mem_read_handler_t read_handlers[] =
{
   { 0x2000, 0x3FFF, ppu_read   },
   { 0x4000, 0x4015, apu_read   },
   { 0x4016, 0x4017, input_read },
   LAST_MEMORY_HANDLER
};

static mem_write_handler_t write_handlers[] =
{
   { 0x2000, 0x3FFF, ppu_write   },
   { 0x4014, 0x4014, ppu_write   },
   { 0x4016, 0x4016, input_write },
   { 0x4000, 0x4017, apu_write   },
   { 0x8000, 0xFFFF, write_protect},
   LAST_MEMORY_HANDLER
};

INLINE void build_memory_map()
{
   int num_read_handlers = 0, num_write_handlers = 0;
   mapintf_t *intf = nes.mmc->intf;

   memset(&nes.mem, 0, sizeof(nes.mem));

   // Memory pages
   nes6502_setpage(0, nes.ram);
   nes6502_setpage(1, nes.ram);
   nes6502_setpage(6, nes.rominfo->sram);
   nes6502_setpage(7, nes.rominfo->sram + 0x1000);

   // Special MMC handlers
   for (int i = 0, rc = 0, wc = 0; i < MAX_MEM_HANDLERS; i++)
   {
      if (intf->mem_read && intf->mem_read[rc].read_func)
         memcpy(&nes.mem.read_handlers[num_read_handlers++], &intf->mem_read[rc++],
               sizeof(mem_read_handler_t));

      if (intf->mem_write && intf->mem_write[wc].write_func)
         memcpy(&nes.mem.write_handlers[num_write_handlers++], &intf->mem_write[wc++],
               sizeof(mem_write_handler_t));
   }

   // Special IO handlers
   for (int i = 0, rc = 0, wc = 0; i < MAX_MEM_HANDLERS; i++)
   {
      if (read_handlers[rc].read_func)
         memcpy(&nes.mem.read_handlers[num_read_handlers++], &read_handlers[rc++],
               sizeof(mem_read_handler_t));

      if (write_handlers[wc].write_func)
         memcpy(&nes.mem.write_handlers[num_write_handlers++], &write_handlers[wc++],
               sizeof(mem_write_handler_t));
   }

   ASSERT(num_read_handlers <= MAX_MEM_HANDLERS);
   ASSERT(num_write_handlers <= MAX_MEM_HANDLERS);
}

/* Emulate one frame and render if draw_flag is true */
INLINE void renderframe(bool draw_flag)
{
   int elapsed_cycles;
   mapintf_t *mapintf = nes.mmc->intf;

   // Swap buffers (We may not be done blitting the current one)
   nes.vidbuf = (nes.vidbuf == framebuffers[1])
      ? framebuffers[0] : framebuffers[1];

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

   osd_setsound(nes.apu->process);
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

      odroid_system_stats_tick(!drawFrame, fullFrame);
   }
}

INLINE void mem_reset(uint8 *buffer, int length, int reset_type)
{
   if (!buffer) return;

   if (HARD_RESET == reset_type)
   {
      for (int i = 0; i < length; i++)
         buffer[i] = (uint8) rand();
   }
   else if (ZERO_RESET == reset_type)
   {
      memset(buffer, 0, length);
   }
}

/* Reset NES hardware */
void nes_reset(int reset_type)
{
   mem_reset(nes.ram, NES_RAMSIZE, reset_type);
   mem_reset(nes.rominfo->vram, 0x2000 * nes.rominfo->vram_banks, reset_type);

   apu_reset();
   ppu_reset();
   mmc_reset();
   nes6502_reset();

   nes.scanline = 241;
   nes.cycles = 0;

   MESSAGE_INFO("NES %s\n", (SOFT_RESET == reset_type) ? "reset" : "powered on");
}

void nes_destroy()
{
   rom_free(&nes.rominfo);
   mmc_destroy(&nes.mmc);
   ppu_destroy(&nes.ppu);
   apu_destroy(&nes.apu);
   bmp_destroy(&framebuffers[0]);
   bmp_destroy(&framebuffers[1]);
   nes6502_destroy(&nes.cpu);
   memset(&nes, 0, sizeof(nes));
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

   build_memory_map();

   nes6502_setcontext(nes.cpu);
   apu_setcontext(nes.apu);
   ppu_setcontext(nes.ppu);
   mmc_setcontext(nes.mmc);

   nes_reset(HARD_RESET);

   return 0;

_fail:
   nes_destroy();
   return -1;
}

/* Initialize NES CPU, hardware, etc. */
nes_t *nes_create(region_t region)
{
   sndinfo_t osd_sound;
   nes_t *machine;

   machine = &nes;
   memset(machine, 0, sizeof(nes_t));

   // https://wiki.nesdev.com/w/index.php/Cycle_reference_chart
   if (region == NES_PAL)
   {
      machine->refresh_rate = 50;
      machine->scanlines = 312;
      machine->overscan = 0;
      machine->cycles_per_line = 341.f * 5 / 16;
      MESSAGE_INFO("System region: PAL\n");
   }
   else
   {
      machine->refresh_rate = 60;
      machine->scanlines = 262;
      machine->overscan = 8;
      machine->cycles_per_line = 341.f * 4 / 12;
      MESSAGE_INFO("System region: NTSC\n");
   }

   machine->region = region;
   machine->autoframeskip = true;
   machine->poweroff = false;
   machine->pause = false;

   /* Framebuffers */
   framebuffers[0] = bmp_create(NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, 8);
   framebuffers[1] = bmp_create(NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, 8);
   if (NULL == framebuffers[0] || NULL == framebuffers[1])
      goto _fail;

   /* cpu */
   machine->cpu = nes6502_create(&machine->mem);
   if (NULL == machine->cpu)
      goto _fail;

   /* apu */
   osd_getsoundinfo(&osd_sound);
   machine->apu = apu_create(0, osd_sound.sample_rate, machine->refresh_rate, osd_sound.bps);
   if (NULL == machine->apu)
      goto _fail;

   /* ppu */
   machine->ppu = ppu_create();
   if (NULL == machine->ppu)
      goto _fail;

   return machine;

_fail:
   nes_destroy();
   return NULL;
}
