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

static const char header[16] = "SNES9X_000000002";


bool S9xSaveState(const char *filename)
{
   int chunks = 0;
   FILE *fp = NULL;

   if (!(fp = fopen(filename, "wb")))
      return false;

   chunks += fwrite(&header, sizeof(header), 1, fp);
   chunks += fwrite(&CPU, sizeof(CPU), 1, fp);
   chunks += fwrite(&ICPU, sizeof(ICPU), 1, fp);
   chunks += fwrite(&PPU, sizeof(PPU), 1, fp);
   chunks += fwrite(&DMA, sizeof(DMA), 1, fp);
   chunks += fwrite(Memory.VRAM, VRAM_SIZE, 1, fp);
   chunks += fwrite(Memory.RAM, RAM_SIZE, 1, fp);
   chunks += fwrite(Memory.SRAM, SRAM_SIZE, 1, fp);
   chunks += fwrite(Memory.FillRAM, FILLRAM_SIZE, 1, fp);
   chunks += fwrite(&APU, sizeof(APU), 1, fp);
   chunks += fwrite(&IAPU, sizeof(IAPU), 1, fp);
   chunks += fwrite(IAPU.RAM, 0x10000, 1, fp);
   chunks += fwrite(&SoundData, sizeof(SoundData), 1, fp);

   printf("Saved chunks = %d\n", chunks);

   fclose(fp);

   return chunks == 13;
}

bool S9xLoadState(const char *filename)
{
   uint8_t buffer[512];
   int chunks = 0;
   FILE *fp = NULL;

   if (!(fp = fopen(filename, "rb")))
      return false;

   if (!fread(buffer, 16, 1, fp) || memcmp(header, buffer, sizeof(header)) != 0)
   {
      printf("Wrong header found\n");
      goto fail;
   }

   // At this point we can't go back and a failure will corrupt the state anyway
   S9xReset();

   uint8_t *IAPU_RAM = IAPU.RAM;

   chunks += fread(&CPU, sizeof(CPU), 1, fp);
   chunks += fread(&ICPU, sizeof(ICPU), 1, fp);
   chunks += fread(&PPU, sizeof(PPU), 1, fp);
   chunks += fread(&DMA, sizeof(DMA), 1, fp);
   chunks += fread(Memory.VRAM, VRAM_SIZE, 1, fp);
   chunks += fread(Memory.RAM, RAM_SIZE, 1, fp);
   chunks += fread(Memory.SRAM, SRAM_SIZE, 1, fp);
   chunks += fread(Memory.FillRAM, FILLRAM_SIZE, 1, fp);
   chunks += fread(&APU, sizeof(APU), 1, fp);
   chunks += fread(&IAPU, sizeof(IAPU), 1, fp);
   chunks += fread(IAPU.RAM, 0x10000, 1, fp);
   chunks += fread(&SoundData, sizeof(SoundData), 1, fp);

   printf("Loaded chunks = %d\n", chunks);

   // Fixing up registers and pointers:

   IAPU.PC = IAPU.PC - IAPU.RAM + IAPU_RAM;
   IAPU.DirectPage = IAPU.DirectPage - IAPU.RAM + IAPU_RAM;
   IAPU.WaitAddress1 = IAPU.WaitAddress1 - IAPU.RAM + IAPU_RAM;
   IAPU.WaitAddress2 = IAPU.WaitAddress2 - IAPU.RAM + IAPU_RAM;
   IAPU.RAM = IAPU_RAM;

   FixROMSpeed();
   IPPU.ColorsChanged = true;
   IPPU.OBJChanged = true;
   CPU.InDMA = false;
   S9xFixColourBrightness();
   S9xAPUUnpackStatus();
   S9xFixSoundAfterSnapshotLoad();
   ICPU.ShiftedPB = ICPU.Registers.PB << 16;
   ICPU.ShiftedDB = ICPU.Registers.DB << 16;
   S9xSetPCBase(ICPU.ShiftedPB + ICPU.Registers.PC);
   S9xUnpackStatus();
   S9xFixCycles();
   S9xReschedule();

   fclose(fp);
   return true;

fail:
   fclose(fp);
   return false;
}
