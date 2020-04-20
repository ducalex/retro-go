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

#pragma GCC optimize ("O3")

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <noftypes.h>
#include <nes6502.h>
#include <log.h>
#include <osd.h>
#include <nes.h>
#include <nes_apu.h>
#include <nes_ppu.h>
#include <nes_rom.h>
#include <nes_mmc.h>
#include <nofrendo.h>
#include <nesstate.h>

#include <esp_attr.h>
#include <esp_system.h>
#include <odroid_system.h>


// #define  NES_CLOCK_DIVIDER    12
// #define  NES_MASTER_CLOCK     (236250000 / 11)
// #define  NES_SCANLINE_CYCLES  (1364.0 / NES_CLOCK_DIVIDER)
// #define  NES_FIQ_PERIOD       (NES_MASTER_CLOCK / NES_CLOCK_DIVIDER / 60)

static const int NES_RAMSIZE = (0x800);

static nes_t nes;

/* TODO: just asking for problems -- please remove */
nes_t *nes_getcontextptr(void)
{
   return &nes;
}

void nes_getcontext(nes_t *machine)
{
   apu_getcontext(nes.apu);
   ppu_getcontext(nes.ppu);
   nes6502_getcontext(nes.cpu);
   mmc_getcontext(nes.mmc);

   *machine = nes;
}

void nes_setcontext(nes_t *machine)
{
   ASSERT(machine);

   apu_setcontext(machine->apu);
   ppu_setcontext(machine->ppu);
   nes6502_setcontext(machine->cpu);
   mmc_setcontext(machine->mmc);

   nes = *machine;
}

static uint8 IRAM_ATTR ram_read(uint32 address)
{
   return nes.cpu->mem_page[0][address & (NES_RAMSIZE - 1)];
}

static void IRAM_ATTR ram_write(uint32 address, uint8 value)
{
   nes.cpu->mem_page[0][address & (NES_RAMSIZE - 1)] = value;
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

#define  LAST_MEMORY_HANDLER  { -1, -1, NULL }
/* read/write handlers for standard NES */
static nes6502_memread default_readhandler[] =
{
   { 0x0800, 0x1FFF, ram_read },
   { 0x2000, 0x3FFF, ppu_read },
   { 0x4000, 0x4015, apu_read },
   { 0x4016, 0x4017, ppu_readhigh },
   LAST_MEMORY_HANDLER
};

static nes6502_memwrite default_writehandler[] =
{
   { 0x0800, 0x1FFF, ram_write },
   { 0x2000, 0x3FFF, ppu_write },
   { 0x4000, 0x4013, apu_write },
   { 0x4015, 0x4015, apu_write },
   { 0x4017, 0x4017, apu_write },
   { 0x4014, 0x4016, ppu_writehigh },
   LAST_MEMORY_HANDLER
};

/* this big nasty boy sets up the address handlers that the CPU uses */
static void build_address_handlers(nes_t *machine)
{
   int count, num_handlers = 0;
   mapintf_t *intf;

   ASSERT(machine);
   intf = machine->mmc->intf;

   memset(machine->readhandler, 0, sizeof(machine->readhandler));
   memset(machine->writehandler, 0, sizeof(machine->writehandler));

   for (count = 0; num_handlers < MAX_MEM_HANDLERS; count++, num_handlers++)
   {
      if (NULL == default_readhandler[count].read_func)
         break;

      memcpy(&machine->readhandler[num_handlers], &default_readhandler[count],
             sizeof(nes6502_memread));
   }

   if (intf->sound_ext)
   {
      if (NULL != intf->sound_ext->mem_read)
      {
         for (count = 0; num_handlers < MAX_MEM_HANDLERS; count++, num_handlers++)
         {
            if (NULL == intf->sound_ext->mem_read[count].read_func)
               break;

            memcpy(&machine->readhandler[num_handlers], &intf->sound_ext->mem_read[count],
                   sizeof(nes6502_memread));
         }
      }
   }

   if (NULL != intf->mem_read)
   {
      for (count = 0; num_handlers < MAX_MEM_HANDLERS; count++, num_handlers++)
      {
         if (NULL == intf->mem_read[count].read_func)
            break;

         memcpy(&machine->readhandler[num_handlers], &intf->mem_read[count],
                sizeof(nes6502_memread));
      }
   }

   /* TODO: poof! numbers */
   machine->readhandler[num_handlers].min_range = 0x4018;
   machine->readhandler[num_handlers].max_range = 0x5FFF;
   machine->readhandler[num_handlers].read_func = read_protect;
   num_handlers++;
   machine->readhandler[num_handlers].min_range = -1;
   machine->readhandler[num_handlers].max_range = -1;
   machine->readhandler[num_handlers].read_func = NULL;
   num_handlers++;
   ASSERT(num_handlers <= MAX_MEM_HANDLERS);

   num_handlers = 0;

   for (count = 0; num_handlers < MAX_MEM_HANDLERS; count++, num_handlers++)
   {
      if (NULL == default_writehandler[count].write_func)
         break;

      memcpy(&machine->writehandler[num_handlers], &default_writehandler[count],
             sizeof(nes6502_memwrite));
   }

   if (intf->sound_ext)
   {
      if (NULL != intf->sound_ext->mem_write)
      {
         for (count = 0; num_handlers < MAX_MEM_HANDLERS; count++, num_handlers++)
         {
            if (NULL == intf->sound_ext->mem_write[count].write_func)
               break;

            memcpy(&machine->writehandler[num_handlers], &intf->sound_ext->mem_write[count],
                   sizeof(nes6502_memwrite));
         }
      }
   }

   if (NULL != intf->mem_write)
   {
      for (count = 0; num_handlers < MAX_MEM_HANDLERS; count++, num_handlers++)
      {
         if (NULL == intf->mem_write[count].write_func)
            break;

         memcpy(&machine->writehandler[num_handlers], &intf->mem_write[count],
                sizeof(nes6502_memwrite));
      }
   }

   /* catch-all for bad writes */
   /* TODO: poof! numbers */
   machine->writehandler[num_handlers].min_range = 0x4018;
   machine->writehandler[num_handlers].max_range = 0x5FFF;
   machine->writehandler[num_handlers].write_func = write_protect;
   num_handlers++;
   machine->writehandler[num_handlers].min_range = 0x8000;
   machine->writehandler[num_handlers].max_range = 0xFFFF;
   machine->writehandler[num_handlers].write_func = write_protect;
   num_handlers++;
   machine->writehandler[num_handlers].min_range = -1;
   machine->writehandler[num_handlers].max_range = -1;
   machine->writehandler[num_handlers].write_func = NULL;
   num_handlers++;
   ASSERT(num_handlers <= MAX_MEM_HANDLERS);
}

void nes_compatibility_hacks(void)
{
   // Disable the Frame Counter interrupt
   // apu_getcontextptr()->fc.disable_irq =
   //       nes.rominfo->checksum == 0xDBB3BC30 || // Teenage Mutant Ninja Turtles 3 (USA)
   //       nes.rominfo->checksum == 0x0639E88E || // Felix the cat (USA)
   //       nes.rominfo->checksum == 0x150908E5;   // Tetris 2 + Bombliss (J)
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
         mapintf->hblank(nes.scanline >= 241);

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

   nofrendo_notify("NES %s", (SOFT_RESET == reset_type) ? "reset" : "powered on");
}

void nes_destroy(nes_t **machine)
{
   if (*machine)
   {
      rom_free(&(*machine)->rominfo);
      mmc_destroy(&(*machine)->mmc);
      ppu_destroy(&(*machine)->ppu);
      apu_destroy(&(*machine)->apu);
      bmp_destroy(&(*machine)->vidbuf);
      if ((*machine)->cpu)
      {
         if ((*machine)->cpu->mem_page[0])
            free((*machine)->cpu->mem_page[0]);
         free((*machine)->cpu);
      }

      free(*machine);
      *machine = NULL;
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

   nes_setcontext(machine);

   nes_reset(HARD_RESET);

   return 0;

_fail:
   nes_destroy(&machine);
   return -1;
}


/* Initialize NES CPU, hardware, etc. */
nes_t *nes_create(region_t region)
{
   nes_t *machine;
   sndinfo_t osd_sound;

   machine = calloc(sizeof(nes_t), 1);
   if (NULL == machine)
      return NULL;

   // https://wiki.nesdev.com/w/index.php/Cycle_reference_chart
   if (region == NES_PAL)
   {
      machine->refresh_rate = 50;
      machine->scanlines = 312;
      machine->overscan = 0;
      machine->cycles_per_line = 341.f * 5 / 16;
      printf("nes_create: System region: PAL\n");
   }
   else
   {
      machine->refresh_rate = 60;
      machine->scanlines = 262;
      machine->overscan = 8;
      machine->cycles_per_line = 341.f * 4 / 12;
      printf("nes_create: System region: NTSC\n");
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
   machine->cpu = calloc(sizeof(nes6502_context), 1);
   if (NULL == machine->cpu)
      goto _fail;

   /* allocate 2kB RAM */
   machine->cpu->mem_page[0] = malloc(NES_RAMSIZE);
   if (NULL == machine->cpu->mem_page[0])
      goto _fail;

   machine->cpu->read_handler = machine->readhandler;
   machine->cpu->write_handler = machine->writehandler;

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
   nes_destroy(&machine);
   return NULL;
}
