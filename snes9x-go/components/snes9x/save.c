/* This file is part of Snes9x. See LICENSE file. */

#include "snes9x.h"
#include "memmap.h"
#include "ppu.h"
#include "cpuexec.h"
#include "apu.h"
#include "dma.h"
#include "display.h"
#include "srtc.h"
#include "soundux.h"

static const size_t SAVESTATE_SIZE =
    sizeof(CPU) + sizeof(ICPU) + sizeof(PPU) + sizeof(DMA) + VRAM_SIZE +
    RAM_SIZE + SRAM_SIZE + FILLRAM_SIZE + SPC_SAVE_STATE_BLOCK_SIZE;


bool S9xSaveState(const char *filename)
{
   uint8_t *buffer, *buffer_ptr;
   bool success = false;
   FILE *fp = NULL;

   if (!(buffer = malloc(SAVESTATE_SIZE)))
      return false;

   if (!(fp = fopen(filename, "wb")))
      goto done;

   buffer_ptr = buffer;

   // ...

   if (!fwrite(buffer, SAVESTATE_SIZE, 1, fp))
      goto done;

   // We've made it, yay!
   success = true;

done:
   if (fp) fclose(fp);
   free(buffer);
   return success;
}

bool S9xLoadState(const char *filename)
{
   uint8_t *buffer, *buffer_ptr;
   bool success = false;
   FILE *fp = NULL;

   if (!(buffer = malloc(SAVESTATE_SIZE)))
      return false;

   if (!(fp = fopen(filename, "rb")))
      goto done;

   if (!fread(buffer, SAVESTATE_SIZE, 1, fp))
      goto done;

done:
   if (fp) fclose(fp);
   free(buffer);
   return success;
}
