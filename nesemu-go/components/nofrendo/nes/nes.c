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
#include <nes6502.h>
#include <osd.h>
#include <nes.h>
#include <nes_apu.h>
#include <nes_ppu.h>
#include <nes_rom.h>
#include <nes_mmc.h>
#include <nofrendo.h>
#include <nes_input.h>
#include <nes_state.h>

#include <esp_attr.h>
#include <odroid_system.h>

static nes_t nes;

nes_t *nes_getptr(void)
{
   return &nes;
}

static void write_protect(uint32 address, uint8 value)
{
   /* don't allow write to go through */
   UNUSED(address);
   UNUSED(value);
}

static uint8 read_protect(uint32 address)
{
   /* don't allow read to go through */
   UNUSED(address);

   return 0xFF;
}

/* read/write handlers for standard NES */
static nes6502_memread default_readhandler[] =
{
   { 0x2000, 0x3FFF, ppu_read   },
   { 0x4000, 0x4015, apu_read   },
   { 0x4016, 0x4017, input_read },
   LAST_MEMORY_HANDLER
};

static nes6502_memwrite default_writehandler[] =
{
   { 0x2000, 0x3FFF, ppu_write   },
   { 0x4014, 0x4014, ppu_write   },
   { 0x4016, 0x4016, input_write },
   { 0x4000, 0x4017, apu_write   },
   LAST_MEMORY_HANDLER
};

static void build_address_handlers(nes_t *machine)
{
   ASSERT(machine);

   int num_read_handlers = 0, num_write_handlers = 0;
   mapintf_t *intf = machine->mmc->intf;

   memset(machine->read_handlers, 0, sizeof(machine->read_handlers));
   memset(machine->write_handlers, 0, sizeof(machine->write_handlers));

   for (int i = 0, rc = 0, wc = 0; i < MAX_MEM_HANDLERS; i++)
   {
      if (default_readhandler[rc].read_func)
         memcpy(&machine->read_handlers[num_read_handlers++], &default_readhandler[rc++],
               sizeof(nes6502_memread));

      if (default_writehandler[wc].write_func)
         memcpy(&machine->write_handlers[num_write_handlers++], &default_writehandler[wc++],
               sizeof(nes6502_memwrite));
   }

   for (int i = 0, rc = 0, wc = 0; i < MAX_MEM_HANDLERS; i++)
   {
      if (intf->mem_read && intf->mem_read[rc].read_func)
         memcpy(&machine->read_handlers[num_read_handlers++], &intf->mem_read[rc++],
               sizeof(nes6502_memread));

      if (intf->mem_write && intf->mem_write[wc].write_func)
         memcpy(&machine->write_handlers[num_write_handlers++], &intf->mem_write[wc++],
               sizeof(nes6502_memwrite));
   }

   /* catch-all for bad reads */
   machine->read_handlers[num_read_handlers].min_range = 0x4018;
   machine->read_handlers[num_read_handlers].max_range = 0x5FFF;
   machine->read_handlers[num_read_handlers].read_func = read_protect;
   num_read_handlers++;
   machine->read_handlers[num_read_handlers].min_range = -1;
   machine->read_handlers[num_read_handlers].max_range = -1;
   machine->read_handlers[num_read_handlers].read_func = NULL;
   num_read_handlers++;

   ASSERT(num_read_handlers <= MAX_MEM_HANDLERS);

   /* catch-all for bad writes */
   machine->write_handlers[num_write_handlers].min_range = 0x4018;
   machine->write_handlers[num_write_handlers].max_range = 0x5FFF;
   machine->write_handlers[num_write_handlers].write_func = write_protect;
   num_write_handlers++;
   machine->write_handlers[num_write_handlers].min_range = 0x8000;
   machine->write_handlers[num_write_handlers].max_range = 0xFFFF;
   machine->write_handlers[num_write_handlers].write_func = write_protect;
   num_write_handlers++;
   machine->write_handlers[num_write_handlers].min_range = -1;
   machine->write_handlers[num_write_handlers].max_range = -1;
   machine->write_handlers[num_write_handlers].write_func = NULL;
   num_write_handlers++;

   ASSERT(num_write_handlers <= MAX_MEM_HANDLERS);
}

/* raise an IRQ */
void IRAM_ATTR nes_irq(void)
{
#ifdef NOFRENDO_DEBUG
   if (nes.scanline <= NES_SCREEN_HEIGHT)
      memset(nes.vidbuf->line[nes.scanline - 1], GUI_RED, NES_SCREEN_WIDTH);
#endif /* NOFRENDO_DEBUG */

   nes6502_irq();
}

void IRAM_ATTR nes_nmi(void)
{
   nes6502_nmi();
}

static void inline nes_renderframe(bool draw_flag)
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

static void inline system_video(bool draw)
{
   if (!draw) {
      return;
   }

   /* Swap buffer to primary */
   bitmap_t *temp = primary_buffer;
   primary_buffer = nes.vidbuf;
   nes.vidbuf = temp;

   /* overlay our GUI on top of it */
   //gui_frame(true);

   /* Flush buffer to screen */
   osd_blitscreen(primary_buffer);
}

extern bool fullFrame;

/* main emulation loop */
void nes_emulate(void)
{
   const int audioSamples = nes.apu->sample_rate / nes.refresh_rate;
   const int frameTime = get_frame_time(nes.refresh_rate);
   uint skipFrames = 0;

   primary_buffer = bmp_create(NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, 8);

   // Discard the garbage frames
   nes_renderframe(1);
   nes_renderframe(1);

   osd_setsound(nes.apu->process);
   osd_loadstate();

   while (false == nes.poweroff)
   {
      uint startTime = get_elapsed_time();
      bool drawFrame = !skipFrames;

      osd_getinput();
      nes_renderframe(drawFrame);
      system_video(drawFrame);

      if (skipFrames == 0)
      {
         if (get_elapsed_time_since(startTime) > frameTime) skipFrames = 1;
         if (speedupEnabled) skipFrames += speedupEnabled * 2;
      }
      else if (skipFrames > 0)
      {
         skipFrames--;
      }

      if (!speedupEnabled) {
         osd_audioframe(audioSamples);
      }

      odroid_system_stats_tick(!drawFrame, fullFrame);
   }
}

static void mem_reset(uint8 *buffer, int length, int reset_type)
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
   mem_reset(nes.cpu->mem_page[0], NES_RAMSIZE, reset_type);
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
   rom_free(nes.rominfo);
   mmc_destroy(nes.mmc);
   ppu_destroy(nes.ppu);
   apu_destroy(nes.apu);
   bmp_destroy(nes.vidbuf);
   free(nes.cpu);
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
int nes_insertcart(const char *filename, nes_t *machine)
{
   nes6502_setcontext(machine->cpu);

   /* rom file */
   machine->rominfo = rom_load(filename);
   if (NULL == machine->rominfo)
      goto _fail;

   /* map cart's SRAM to CPU $6000-$7FFF */
   if (machine->rominfo->sram)
   {
      machine->cpu->mem_page[6] = machine->rominfo->sram;
      machine->cpu->mem_page[7] = machine->rominfo->sram + 0x1000;
   }

   /* mapper */
   machine->mmc = mmc_create(machine->rominfo);
   if (NULL == machine->mmc)
      goto _fail;

   /* if there's VRAM, let the PPU know */
   if (NULL != machine->rominfo->vram)
      machine->ppu->vram_present = true;

   apu_setext(machine->apu, machine->mmc->intf->sound_ext);

   build_address_handlers(machine);

   // nes_setregion();

   apu_setcontext(machine->apu);
   ppu_setcontext(machine->ppu);
   nes6502_setcontext(machine->cpu);
   mmc_setcontext(machine->mmc);

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

   /* bitmap */
   /* 8 pixel overdraw */
   machine->vidbuf = bmp_create(NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, 8);
   if (NULL == machine->vidbuf)
      goto _fail;

   /* cpu */
   machine->cpu = calloc(sizeof(nes6502_t), 1);
   if (NULL == machine->cpu)
      goto _fail;

   /* Memory */
   machine->cpu->mem_page[0] = &machine->ram;
   machine->cpu->read_handlers = machine->read_handlers;
   machine->cpu->write_handlers = machine->write_handlers;

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
