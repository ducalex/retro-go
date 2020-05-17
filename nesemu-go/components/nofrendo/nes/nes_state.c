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
** nes_state.c
**
** state saving/loading
** $Id: nes_state.c,v 1.2 2001/04/27 14:37:11 neil Exp $
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <nofrendo.h>
#include <nes6502.h>
#include <libsnss.h>
#include <osd.h>
#include "nes_state.h"

extern nes6502_t cpu;

#define  FIRST_STATE_SLOT  0
#define  LAST_STATE_SLOT   9

static int state_slot = FIRST_STATE_SLOT;

/* Set the state-save slot to use (0 - 9) */
void state_setslot(int slot)
{
   /* Don't send a message if we're already at that slot */
   if (state_slot != slot && slot >= FIRST_STATE_SLOT
       && slot <= LAST_STATE_SLOT)
   {
      state_slot = slot;
      MESSAGE_INFO("State slot set to %d\n", slot);
   }
}

int save_baseblock(nes_t *state, SNSS_FILE *snssFile)
{
   int i;

   ASSERT(state);

   nes6502_getcontext(state->cpu);
   ppu_getcontext(state->ppu);

   snssFile->baseBlock.regA = state->cpu->a_reg;
   snssFile->baseBlock.regX = state->cpu->x_reg;
   snssFile->baseBlock.regY = state->cpu->y_reg;
   snssFile->baseBlock.regFlags = state->cpu->p_reg;
   snssFile->baseBlock.regStack = state->cpu->s_reg;
   snssFile->baseBlock.regPc = state->cpu->pc_reg;
   snssFile->baseBlock.reg2000 = state->ppu->ctrl0;
   snssFile->baseBlock.reg2001 = state->ppu->ctrl1;

   memcpy(snssFile->baseBlock.cpuRam, state->mem->ram, 0x800);
   memcpy(snssFile->baseBlock.spriteRam, state->ppu->oam, 0x100);
   memcpy(snssFile->baseBlock.ppuRam, state->ppu->nametab, 0x1000);


   /* Mask off priority color bits */
   for (i = 0; i < 32; i++)
   {
      snssFile->baseBlock.palette[i] = state->ppu->palette[i] & 0x3F;
   }

   snssFile->baseBlock.mirrorState[0] = (state->ppu->page[8] + 0x2000 - state->ppu->nametab) / 0x400;
   snssFile->baseBlock.mirrorState[1] = (state->ppu->page[9] + 0x2400 - state->ppu->nametab) / 0x400;
   snssFile->baseBlock.mirrorState[2] = (state->ppu->page[10] + 0x2800 - state->ppu->nametab) / 0x400;
   snssFile->baseBlock.mirrorState[3] = (state->ppu->page[11] + 0x2C00 - state->ppu->nametab) / 0x400;
   snssFile->baseBlock.vramAddress = state->ppu->vaddr;
   snssFile->baseBlock.spriteRamAddress = state->ppu->oam_addr;
   snssFile->baseBlock.tileXOffset = state->ppu->tile_xofs;

   return 0;
}

bool save_vramblock(nes_t *state, SNSS_FILE *snssFile)
{
   ASSERT(state);

   if (NULL == state->rominfo->vram)
   {
       printf("save_vramblock: NULL == state->rominfo->vram\n");
       return -1;
  }

   if (state->rominfo->vram_banks > 2)
   {
      printf("save_vramblock: too many VRAM banks: %d\n", state->rominfo->vram_banks);
      return -1;
   }

   snssFile->vramBlock.vramSize = VRAM_8K * state->rominfo->vram_banks;

   memcpy(snssFile->vramBlock.vram, state->rominfo->vram, snssFile->vramBlock.vramSize);
   return 0;
}

int save_sramblock(nes_t *state, SNSS_FILE *snssFile)
{
   int i;
   bool written = false;
   int sram_length;

   ASSERT(state);

   sram_length = state->rominfo->sram_banks * SRAM_1K;

   /* Check to see if any SRAM was written to */
   for (i = 0; i < sram_length; i++)
   {
      if (state->rominfo->sram[i])
      {
         written = true;
         break;
      }
   }

   if (false == written)
   {
      // The SRAM is empty, no need to save it to the file
      return -1;
   }

   if (state->rominfo->sram_banks > 8)
   {
      printf("save_sramblock: Unsupported number of SRAM banks: %d\n", state->rominfo->sram_banks);
      return -1;
   }

   snssFile->sramBlock.sramSize = SRAM_1K * state->rominfo->sram_banks;

   /* TODO: this should not always be true!! */
   snssFile->sramBlock.sramEnabled = true;

   memcpy(snssFile->sramBlock.sram, state->rominfo->sram, snssFile->sramBlock.sramSize);

   return 0;
}

int save_soundblock(nes_t *state, SNSS_FILE *snssFile)
{
   ASSERT(state);

   apu_getcontext(state->apu);

   /* rect 0 */
   snssFile->soundBlock.soundRegisters[0x00] = state->apu->rectangle[0].regs[0];
   snssFile->soundBlock.soundRegisters[0x01] = state->apu->rectangle[0].regs[1];
   snssFile->soundBlock.soundRegisters[0x02] = state->apu->rectangle[0].regs[2];
   snssFile->soundBlock.soundRegisters[0x03] = state->apu->rectangle[0].regs[3];
   /* rect 1 */
   snssFile->soundBlock.soundRegisters[0x04] = state->apu->rectangle[1].regs[0];
   snssFile->soundBlock.soundRegisters[0x05] = state->apu->rectangle[1].regs[1];
   snssFile->soundBlock.soundRegisters[0x06] = state->apu->rectangle[1].regs[2];
   snssFile->soundBlock.soundRegisters[0x07] = state->apu->rectangle[1].regs[3];
   /* triangle */
   snssFile->soundBlock.soundRegisters[0x08] = state->apu->triangle.regs[0];
   snssFile->soundBlock.soundRegisters[0x0A] = state->apu->triangle.regs[1];
   snssFile->soundBlock.soundRegisters[0x0B] = state->apu->triangle.regs[2];
   /* noise */
   snssFile->soundBlock.soundRegisters[0X0C] = state->apu->noise.regs[0];
   snssFile->soundBlock.soundRegisters[0X0E] = state->apu->noise.regs[1];
   snssFile->soundBlock.soundRegisters[0x0F] = state->apu->noise.regs[2];
   /* dmc */
   snssFile->soundBlock.soundRegisters[0x10] = state->apu->dmc.regs[0];
   snssFile->soundBlock.soundRegisters[0x11] = state->apu->dmc.regs[1];
   snssFile->soundBlock.soundRegisters[0x12] = state->apu->dmc.regs[2];
   snssFile->soundBlock.soundRegisters[0x13] = state->apu->dmc.regs[3];
   /* control */
   snssFile->soundBlock.soundRegisters[0x15] = state->apu->control_reg;

   return 0;
}

int save_mapperblock(nes_t *state, SNSS_FILE *snssFile)
{
   int i;
   ASSERT(state);

   mmc_getcontext(state->mmc);

   /* TODO: filthy hack in snss standard */
   /* We don't need to write mapper state for mapper 0 */
   if (0 == state->mmc->intf->number)
   {
       printf("save_mapperblock: 0 == state->mmc->intf->number\n");
       return -1;
    }

   /* TODO: snss spec should be updated, using 4kB ROM pages.. */
   for (i = 0; i < 4; i++)
      snssFile->mapperBlock.prgPages[i] = (mem_getpage((i + 4) * 2) - state->rominfo->rom) >> 13;

   if (state->rominfo->vrom_banks)
   {
      for (i = 0; i < 8; i++)
         snssFile->mapperBlock.chrPages[i] = (ppu_getpage(i) - state->rominfo->vrom + (i * 0x400)) >> 10;
   }
   else
   {
      /* bleh! slight hack */
      for (i = 0; i < 8; i++)
         snssFile->mapperBlock.chrPages[i] = i;
   }

   if (state->mmc->intf->get_state)
      state->mmc->intf->get_state(&snssFile->mapperBlock);

   return 0;
}

void load_baseblock(nes_t *state, SNSS_FILE *snssFile)
{
   int i;

   ASSERT(state);

   nes6502_getcontext(state->cpu);
   ppu_getcontext(state->ppu);

   state->cpu->a_reg = snssFile->baseBlock.regA;
   state->cpu->x_reg = snssFile->baseBlock.regX;
   state->cpu->y_reg = snssFile->baseBlock.regY;
   state->cpu->p_reg = snssFile->baseBlock.regFlags;
   state->cpu->s_reg = snssFile->baseBlock.regStack;
   state->cpu->pc_reg = snssFile->baseBlock.regPc;
   state->ppu->ctrl0 = snssFile->baseBlock.reg2000;
   state->ppu->ctrl1 = snssFile->baseBlock.reg2001;

   memcpy(state->mem->ram, snssFile->baseBlock.cpuRam, 0x800);
   memcpy(state->ppu->oam, snssFile->baseBlock.spriteRam, 0x100);
   memcpy(state->ppu->nametab, snssFile->baseBlock.ppuRam, 0x1000);
   memcpy(state->ppu->palette, snssFile->baseBlock.palette, 0x20);

   /* TODO: argh, this is to handle nofrendo's filthy sprite priority method */
   for (i = 0; i < 8; i++)
      state->ppu->palette[i << 2] = state->ppu->palette[0] | 0x80;//BG_TRANS_MASK;

   for (i = 0; i < 4; i++)
   {
      state->ppu->page[i + 8] = state->ppu->page[i + 12] =
         state->ppu->nametab + (snssFile->baseBlock.mirrorState[i] * 0x400) - (0x2000 + (i * 0x400));
   }

   state->ppu->vaddr = snssFile->baseBlock.vramAddress;
   state->ppu->oam_addr = snssFile->baseBlock.spriteRamAddress;
   state->ppu->tile_xofs = snssFile->baseBlock.tileXOffset;

   /* do some extra handling */
   state->ppu->flipflop = 0;
   state->ppu->strikeflag = false;

   nes6502_setcontext(state->cpu);
   ppu_setcontext(state->ppu);

   ppu_write(PPU_CTRL0, state->ppu->ctrl0);
   ppu_write(PPU_CTRL1, state->ppu->ctrl1);
   ppu_write(PPU_VADDR, (uint8) (state->ppu->vaddr >> 8));
   ppu_write(PPU_VADDR, (uint8) (state->ppu->vaddr & 0xFF));
}

void load_vramblock(nes_t *state, SNSS_FILE *snssFile)
{
   ASSERT(state);

   ASSERT(snssFile->vramBlock.vramSize <= VRAM_8K); /* can't handle more than this! */
   memcpy(state->rominfo->vram, snssFile->vramBlock.vram, snssFile->vramBlock.vramSize);
}

void load_sramblock(nes_t *state, SNSS_FILE *snssFile)
{
   ASSERT(state);

   ASSERT(snssFile->sramBlock.sramSize <= SRAM_8K); /* can't handle more than this! */
   memcpy(state->rominfo->sram, snssFile->sramBlock.sram, snssFile->sramBlock.sramSize);
}

void load_controllerblock(nes_t *state, SNSS_FILE *snssFile)
{
   UNUSED(state);
   UNUSED(snssFile);
}

void load_soundblock(nes_t *state, SNSS_FILE *snssFile)
{
   int i;

   ASSERT(state);

    apu_reset();

    for (i = 0; i < SOUND_BLOCK_LENGTH; i++)
    {
          apu_write(0x4000 + i, snssFile->soundBlock.soundRegisters[i]);
    }
}

/* TODO: magic numbers galore */
void load_mapperblock(nes_t *state, SNSS_FILE *snssFile)
{
   int i;

   ASSERT(state);

   mmc_getcontext(state->mmc);

   for (i = 0; i < 4; i++)
      mmc_bankrom(8, 0x8000 + (i * 0x2000), snssFile->mapperBlock.prgPages[i]);

   if (state->rominfo->vrom_banks)
   {
      for (i = 0; i < 8; i++)
         mmc_bankvrom(1, i * 0x400, snssFile->mapperBlock.chrPages[i]);
   }
   else
   {
      ASSERT(state->rominfo->vram);

      for (i = 0; i < 8; i++)
         ppu_setpage(1, i, state->rominfo->vram);
   }

   if (state->mmc->intf->set_state)
      state->mmc->intf->set_state(&snssFile->mapperBlock);

   mmc_setcontext(state->mmc);
}


int state_save(char* fn)
{
   SNSS_FILE *snssFile;
   SNSS_RETURN_CODE status;

   nes_t *machine;

   /* get the pointer to our NES machine context */
   machine = nes_getptr();
   ASSERT(machine);

   printf("state_save: fn='%s'\n", fn);

   /* open our state file for writing */
   status = SNSS_OpenFile(&snssFile, fn, SNSS_OPEN_WRITE);
   if (SNSS_OK != status)
      return -1;

   printf("state_save: SNSS_OpenFile OK\n");

   /* now get all of our blocks */
   if (0 == save_baseblock(machine, snssFile))
   {
      status = SNSS_WriteBlock(snssFile, SNSS_BASR);
      if (SNSS_OK != status)
         goto _error;
      printf("  - save_baseblock OK\n");
   }

   if (0 == save_vramblock(machine, snssFile))
   {
      status = SNSS_WriteBlock(snssFile, SNSS_VRAM);
      if (SNSS_OK != status)
         goto _error;
      printf("  - save_vramblock OK\n");
   }

   if (0 == save_sramblock(machine, snssFile))
   {
      status = SNSS_WriteBlock(snssFile, SNSS_SRAM);
      if (SNSS_OK != status)
         goto _error;
      printf("  - save_sramblock OK\n");
   }

   if (0 == save_soundblock(machine, snssFile))
   {
      status = SNSS_WriteBlock(snssFile, SNSS_SOUN);
      if (SNSS_OK != status)
         goto _error;
      printf("  - save_soundblock OK\n");
   }

   if (0 == save_mapperblock(machine, snssFile))
   {
      status = SNSS_WriteBlock(snssFile, SNSS_MPRD);
      if (SNSS_OK != status)
         goto _error;
      printf("  - save_mapperblock OK\n");
   }

   /* close the file, we're done */
   status = SNSS_CloseFile(&snssFile);
   if (SNSS_OK != status)
      goto _error;

   printf("state_save: Game %d saved\n", state_slot);
   return 0;

_error:
   printf("state_save: Failed: %s\n", SNSS_GetErrorString(status));
   SNSS_CloseFile(&snssFile);
   return -1;
}


int state_load(char* fn)
{
   SNSS_FILE *snssFile;
   SNSS_RETURN_CODE status;
   SNSS_BLOCK_TYPE block_type;

   unsigned int i;
   nes_t *machine;

   /* get our machine's context pointer */
   machine = nes_getptr();

   ASSERT(machine);

   /* open our file for reading */
   status = SNSS_OpenFile(&snssFile, fn, SNSS_OPEN_READ);
   if (SNSS_OK != status)
   {
       printf("state_load: file '%s' could not be opened.\n", fn);
       return -1; //goto _error;
  }

   printf("state_load: file '%s' opened.\n", fn);

   /* iterate through all present blocks */
   printf("state_load: snssFile->headerBlock.numberOfBlocks=%d\n", snssFile->headerBlock.numberOfBlocks);

   for (i = 0; i < snssFile->headerBlock.numberOfBlocks; i++)
   {
      status = SNSS_GetNextBlockType(&block_type, snssFile);
      if (SNSS_OK != status)
         goto _error;

      status = SNSS_ReadBlock(snssFile, block_type);
      if (SNSS_OK != status)
         goto _error;

      switch (block_type)
      {
      case SNSS_BASR:
         printf("  - load_baseblock\n");
         load_baseblock(machine, snssFile);
         break;

      case SNSS_VRAM:
         printf("  - load_vramblock\n");
         load_vramblock(machine, snssFile);
         break;

      case SNSS_SRAM:
         printf("  - load_sramblock\n");
         load_sramblock(machine, snssFile);
         break;

      case SNSS_MPRD:
         printf("  - load_mapperblock\n");
         load_mapperblock(machine, snssFile);
         break;

      case SNSS_CNTR:
         printf("  - load_controllerblock\n");
         load_controllerblock(machine, snssFile);
         break;

      case SNSS_SOUN:
         printf("  - load_soundblock\n");
         load_soundblock(machine, snssFile);
         break;

      case SNSS_UNKNOWN_BLOCK:
      default:
         printf("  - unknown SNSS block type\n");
         break;
      }
   }

   /* close file, we're done */
   status = SNSS_CloseFile(&snssFile);

   printf("state_load: Game %d restored\n", state_slot);

   return 0;

_error:
   printf("state_load: Failed: %s\n", SNSS_GetErrorString(status));
   SNSS_CloseFile(&snssFile);
   abort();
}
