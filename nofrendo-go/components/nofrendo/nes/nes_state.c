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
** State saving/loading
** $Id: nes_state.c,v 1.2 2001/04/27 14:37:11 neil Exp $
*/
#include <stdio.h>
#include <string.h>
#include <nofrendo.h>
#include <nes6502.h>
#include "nes_state.h"

#define _fread(buffer, size) {                       \
   if (fread(buffer, size, 1, file) != 1)            \
   {                                                 \
      MESSAGE_ERROR("state_load: fread failed.\n");  \
      goto _error;                                   \
   }                                                 \
}

#define _fwrite(buffer, size) {                      \
   if (fwrite(buffer, size, 1, file) != 1)           \
   {                                                 \
      MESSAGE_ERROR("state_save: fwrite failed.\n"); \
      goto _error;                                   \
   }                                                 \
}

#ifdef IS_LITTLE_ENDIAN
   static inline uint16_t swap16(uint16_t x)
   {
      return (x << 8) | (x >> 8);
   }

   static inline uint32_t swap32(uint32_t x)
   {
      return ((x>>24)&0xff) | ((x<<8)&0xff0000) | ((x>>8)&0xff00) | ((x<<24)&0xff000000);
   }
#else
   #define swap16(x) (x)
   #define swap32(x) (x)
#endif

static int save_slot = 0;


/* Set the state-save slot to use (0 - 9) */
void state_setslot(int slot)
{
   if (save_slot != slot && slot >= 0 && slot <= 9)
   {
      save_slot = slot;
      MESSAGE_INFO("Save slot changed to %d\n", slot);
   }
}

int state_save(char* fn)
{
   uint8 buffer[512];
   uint8 numberOfBlocks = 0;
   nes_t *machine;
   FILE *file;
   uint16 i, temp;

   /* get the pointer to our NES machine context */
   machine = nes_getptr();

   if (!(file = fopen(fn, "wb")))
   {
       MESSAGE_ERROR("state_save: file '%s' could not be opened.\n", fn);
       return -1; //goto _error;
   }

   MESSAGE_INFO("state_save: file '%s' opened.\n", fn);

   _fwrite("SNSS\x00\x00\x00\x05", 8);


   /****************************************************/

   MESSAGE_INFO("  - Saving base block\n");

   // SnssBlockHeader
   _fwrite("BASR\x00\x00\x00\x01\x00\x00\x19\x31", 12);
   numberOfBlocks++;

   buffer[0] = machine->cpu->a_reg;
   buffer[1] = machine->cpu->x_reg;
   buffer[2] = machine->cpu->y_reg;
   buffer[3] = machine->cpu->p_reg;
   buffer[4] = machine->cpu->s_reg;
   temp = swap16(machine->cpu->pc_reg);
   buffer[5] = ((uint8*)&temp)[0];
   buffer[6] = ((uint8*)&temp)[1];
   buffer[7] = machine->ppu->ctrl0;
   buffer[8] = machine->ppu->ctrl1;

   _fwrite(&buffer, 9);

   _fwrite(machine->mem->ram, 0x800);
   _fwrite(machine->ppu->oam, 0x100);
   _fwrite(machine->ppu->nametab, 0x1000);

   /* Mask off priority color bits */
   for (i = 0; i < 32; i++)
   {
      buffer[i] = machine->ppu->palette[i] & 0x3F;
   }

   buffer[32] = machine->ppu->nt1;
   buffer[33] = machine->ppu->nt2;
   buffer[34] = machine->ppu->nt3;
   buffer[35] = machine->ppu->nt4;
   temp = swap16(machine->ppu->vaddr);
   buffer[36] = ((uint8*)&temp)[0];
   buffer[37] = ((uint8*)&temp)[1];
   buffer[38] = machine->ppu->oam_addr;
   buffer[39] = machine->ppu->tile_xofs;

   _fwrite(&buffer, 40);


   /****************************************************/

   if (machine->rominfo->vram)
   {
      MESSAGE_INFO("  - Saving VRAM block\n");

      // SnssBlockHeader
      _fwrite("VRAM\x00\x00\x00\x01\x00\x00\x20\x00", 12);
      numberOfBlocks++;

      _fwrite(machine->rominfo->vram, 0x2000);
   }


   /****************************************************/

   if (machine->rominfo->sram)
   {
      MESSAGE_INFO("  - Saving SRAM block\n");

      // Byte 13 = SRAM enabled (unused)
      // Length is always $2001
      // SnssBlockHeader
      _fwrite("SRAM\x00\x00\x00\x01\x00\x00\x20\x01\x01", 13);
      numberOfBlocks++;

      _fwrite(machine->rominfo->sram, 0x2000);
   }


   /****************************************************/

   MESSAGE_INFO("  - Saving sound block\n");

   // SnssBlockHeader
   _fwrite("SOUN\x00\x00\x00\x01\x00\x00\x00\x16", 12);
   numberOfBlocks++;

   buffer[0x00] = machine->apu->rectangle[0].regs[0];
   buffer[0x01] = machine->apu->rectangle[0].regs[1];
   buffer[0x02] = machine->apu->rectangle[0].regs[2];
   buffer[0x03] = machine->apu->rectangle[0].regs[3];
   buffer[0x04] = machine->apu->rectangle[1].regs[0];
   buffer[0x05] = machine->apu->rectangle[1].regs[1];
   buffer[0x06] = machine->apu->rectangle[1].regs[2];
   buffer[0x07] = machine->apu->rectangle[1].regs[3];
   buffer[0x08] = machine->apu->triangle.regs[0];
   buffer[0x0A] = machine->apu->triangle.regs[1];
   buffer[0x0B] = machine->apu->triangle.regs[2];
   buffer[0X0C] = machine->apu->noise.regs[0];
   buffer[0X0E] = machine->apu->noise.regs[1];
   buffer[0x0F] = machine->apu->noise.regs[2];
   buffer[0x10] = machine->apu->dmc.regs[0];
   buffer[0x11] = machine->apu->dmc.regs[1];
   buffer[0x12] = machine->apu->dmc.regs[2];
   buffer[0x13] = machine->apu->dmc.regs[3];
   buffer[0x15] = machine->apu->control_reg;

   _fwrite(&buffer, 0x16);


   /****************************************************/

   if (machine->mmc->intf->number > 0)
   {
      MESSAGE_INFO("  - Saving mapper block\n");

      // SnssBlockHeader
      _fwrite("MPRD\x00\x00\x00\x01\x00\x00\x00\x98", 12);
      numberOfBlocks++;

      uint8 state[0x80];
      uint16 temp;

      /* TODO: snss spec should be updated, using 4kB ROM pages.. */
      for (i = 0; i < 4; i++)
      {
         temp = swap16((mem_getpage((i + 4) * 4) - machine->rominfo->rom) >> 13);
         buffer[(i * 2) + 0] = ((uint8 *) &temp)[0];
         buffer[(i * 2) + 1] = ((uint8 *) &temp)[1];
      }

      for (i = 0; i < 8; i++)
      {
         temp = (machine->rominfo->vrom_banks) ?
            ((ppu_getpage(i) - machine->rominfo->vrom + (i * 0x400)) >> 10) : (i);
         temp = swap16(temp);
         buffer[8 + (i * 2) + 0] = ((uint8 *) &temp)[0];
         buffer[8 + (i * 2) + 1] = ((uint8 *) &temp)[1];
      }

      if (machine->mmc->intf->get_state)
      {
         machine->mmc->intf->get_state(&state);
         memcpy(buffer + 0x18, state, 0x80);
      }

      _fwrite(&buffer, 0x98);
   }


   /****************************************************/

   // Update number of blocks
   fseek(file, 7, SEEK_SET);
   fwrite(&numberOfBlocks, 1, 1, file);

   fclose(file);

   MESSAGE_INFO("state_save: Game %d saved!\n", save_slot);

   return 0;

_error:
   MESSAGE_ERROR("state_save: Save failed!\n");
   fclose(file);
   return -1;
}

int state_load(char* fn)
{
   uint8 buffer[512];
   FILE *file;
   nes_t *machine;
   int blk, i;

   uint32 numberOfBlocks;
   uint32 blockVersion;
   uint32 blockLength;
   uint32 nextBlock = 8;

   machine = nes_getptr();

   if (!(file = fopen(fn, "rb")))
   {
       MESSAGE_ERROR("state_load: file '%s' could not be opened.\n", fn);
       return -1; //goto _error;
  }

   _fread(buffer, 8);

   if (memcmp(buffer, "SNSS", 4) != 0)
   {
      MESSAGE_ERROR("state_load: file '%s' is not a save file.\n", fn);
      goto _error;
   }

   numberOfBlocks = swap32(*((uint32*)&buffer[4]));

   MESSAGE_INFO("state_load: file '%s' opened, blocks=%d.\n", fn, numberOfBlocks);

   for (blk = 0; blk < numberOfBlocks; blk++)
   {
      fseek(file, nextBlock, SEEK_SET);
      _fread(buffer, 12);

      blockVersion = swap32(*((uint32*)&buffer[4]));
      blockLength = swap32(*((uint32*)&buffer[8]));

      UNUSED(blockVersion);

      nextBlock += 12 + blockLength;

      /****************************************************/

      if (memcmp(buffer, "BASR", 4) == 0)
      {
         MESSAGE_INFO("  - Found base block\n");

         _fread(buffer, 9);

         machine->cpu->a_reg = buffer[0x0];
         machine->cpu->x_reg = buffer[0x1];
         machine->cpu->y_reg = buffer[0x2];
         machine->cpu->p_reg = buffer[0x3];
         machine->cpu->s_reg = buffer[0x4];
         machine->cpu->pc_reg = swap16(*((uint16*)&buffer[0x5]));
         machine->ppu->ctrl0 = buffer[0x7];
         machine->ppu->ctrl1 = buffer[0x8];

         _fread(machine->mem->ram, 0x800);
         _fread(machine->ppu->oam, 0x100);
         _fread(machine->ppu->nametab, 0x1000);
         _fread(machine->ppu->palette, 0x20);

         /* TODO: argh, this is to handle nofrendo's filthy sprite priority method */
         for (i = 0; i < 8; i++)
            machine->ppu->palette[i << 2] = machine->ppu->palette[0] | 0x80; // BG_TRANS;

         _fread(buffer, 8);

         machine->ppu->vaddr = swap16(*((uint16*)&buffer[0x4]));
         machine->ppu->oam_addr = buffer[0x6];
         machine->ppu->tile_xofs = buffer[0x7];

         /* do some extra handling */
         machine->ppu->flipflop = 0;
         machine->ppu->strikeflag = false;

         ppu_setnametables(buffer[0], buffer[1], buffer[2], buffer[3]);
         ppu_write(PPU_CTRL0, machine->ppu->ctrl0);
         ppu_write(PPU_CTRL1, machine->ppu->ctrl1);
         ppu_write(PPU_VADDR, machine->ppu->vaddr >> 8);
         ppu_write(PPU_VADDR, machine->ppu->vaddr & 0xFF);
      }


      /****************************************************/

      else if (memcmp(buffer, "VRAM", 4) == 0)
      {
         MESSAGE_INFO("  - Found VRAM block\n");

         if (machine->rominfo->vram == NULL)
         {
            MESSAGE_ERROR("rominfo says no vram!\n");
            continue;
         }

         _fread(machine->rominfo->vram, MIN(blockLength, 0x4000)); // Max 16K
      }


      /****************************************************/

      else if (memcmp(buffer, "SRAM", 4) == 0)
      {
         MESSAGE_INFO("  - Found SRAM block\n");

         if (machine->rominfo->sram == NULL)
         {
            MESSAGE_ERROR("rominfo says no sram!\n");
            continue;
         }

         _fread(buffer, 1); // SRAM enabled (always true)
         _fread(machine->rominfo->sram, MIN(blockLength - 1, 0x2000)); // Max 8K
      }


      /****************************************************/

      else if (memcmp(buffer, "MPRD", 4) == 0)
      {
         MESSAGE_INFO("  - Found mapper block\n");

         _fread(buffer, blockLength);

         for (i = 0; i < 4; i++)
            mmc_bankrom(8, 0x8000 + (i * 0x2000), swap16(((uint16*)buffer)[i]));

         if (machine->rominfo->vrom_banks)
         {
            for (i = 0; i < 8; i++)
               mmc_bankvrom(1, i * 0x400, swap16(((uint16*)buffer)[4 + i]));
         }
         else if (machine->rominfo->vram)
         {
            for (i = 0; i < 8; i++)
               ppu_setpage(1, i, machine->rominfo->vram);
         }

         if (machine->mmc->intf->set_state)
            machine->mmc->intf->set_state(buffer + 0x18);
      }


      /****************************************************/

      else if (memcmp(buffer, "SOUN", 4) == 0)
      {
         MESSAGE_INFO("  - Found sound block\n");

         _fread(buffer, 0x16);

         apu_reset();

         for (i = 0; i < 0x16; i++)
            apu_write(0x4000 + i, buffer[i]);
      }


      /****************************************************/

      else
      {
         MESSAGE_ERROR("Found unknown block type!\n");
      }
   }

   /* close file, we're done */
   fclose(file);

   MESSAGE_INFO("state_load: Game %d restored\n", save_slot);

   return 0;

_error:
   MESSAGE_ERROR("state_load: Load failed!\n");
   fclose(file);
   // abort();
   return -1;
}
