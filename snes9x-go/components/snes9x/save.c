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

const char header[16] = "SNES9X_000000001";


bool S9xSaveState(const char *filename)
{
   uint8_t *buffer, *buffer_ptr;
   bool success = false;
   FILE *fp = NULL;

   if (!(buffer = malloc(400 * 1024)))
      return false;

   if (!(fp = fopen(filename, "wb")))
      goto done;

   buffer_ptr = buffer;

   memcpy(buffer_ptr, &header, sizeof(header));
   buffer_ptr += sizeof(header);
   memcpy(buffer_ptr, &CPU, sizeof(CPU));
   buffer_ptr += sizeof(CPU);
   memcpy(buffer_ptr, &ICPU, sizeof(ICPU));
   buffer_ptr += sizeof(ICPU);
   memcpy(buffer_ptr, &PPU, sizeof(PPU));
   buffer_ptr += sizeof(PPU);
   memcpy(buffer_ptr, &DMA, sizeof(DMA));
   buffer_ptr += sizeof(DMA);
   memcpy(buffer_ptr, Memory.VRAM, VRAM_SIZE);
   buffer_ptr += VRAM_SIZE;
   memcpy(buffer_ptr, Memory.RAM, RAM_SIZE);
   buffer_ptr += RAM_SIZE;
   memcpy(buffer_ptr, Memory.SRAM, SRAM_SIZE);
   buffer_ptr += SRAM_SIZE;
   memcpy(buffer_ptr, Memory.FillRAM, FILLRAM_SIZE);
   buffer_ptr += FILLRAM_SIZE;
   memcpy(buffer_ptr, &APU, sizeof(APU));
   buffer_ptr += sizeof(APU);
   memcpy(buffer_ptr, &IAPU, sizeof(IAPU));
   buffer_ptr += sizeof(IAPU);
   memcpy(buffer_ptr, IAPU.RAM, 0x10000);
   buffer_ptr += 0x10000;
   memcpy(buffer_ptr, &SoundData, sizeof(SoundData));
   buffer_ptr += sizeof(SoundData);

   printf("Save size = %d\n", buffer_ptr - buffer);

   if (fwrite(buffer, buffer_ptr - buffer, 1, fp) != 1)
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

   if (!(buffer = malloc(400 * 1024)))
      return false;

   if (!(fp = fopen(filename, "rb")))
      goto done;

   if (fread(buffer, 1, 400 * 1024, fp) < 16)
      goto done;

   if (memcmp(header, buffer, sizeof(header)) != 0)
      goto done;

   S9xReset();

   uint8_t *IAPU_RAM = IAPU.RAM;

   buffer_ptr = buffer + sizeof(header);

   memcpy(&CPU, buffer_ptr, sizeof(CPU));
   buffer_ptr += sizeof(CPU);
   memcpy(&ICPU, buffer_ptr, sizeof(ICPU));
   buffer_ptr += sizeof(ICPU);
   memcpy(&PPU, buffer_ptr, sizeof(PPU));
   buffer_ptr += sizeof(PPU);
   memcpy(&DMA, buffer_ptr, sizeof(DMA));
   buffer_ptr += sizeof(DMA);
   memcpy(Memory.VRAM, buffer_ptr, VRAM_SIZE);
   buffer_ptr += VRAM_SIZE;
   memcpy(Memory.RAM, buffer_ptr, RAM_SIZE);
   buffer_ptr += RAM_SIZE;
   memcpy(Memory.SRAM, buffer_ptr, SRAM_SIZE);
   buffer_ptr += SRAM_SIZE;
   memcpy(Memory.FillRAM, buffer_ptr, FILLRAM_SIZE);
   buffer_ptr += FILLRAM_SIZE;
   memcpy(&APU, buffer_ptr, sizeof(APU));
   buffer_ptr += sizeof(APU);
   memcpy(&IAPU, buffer_ptr, sizeof(IAPU));
   buffer_ptr += sizeof(IAPU);
   memcpy(IAPU.RAM, buffer_ptr, 0x10000);
   buffer_ptr += 0x10000;
   memcpy(&SoundData, buffer_ptr, sizeof(SoundData));
   buffer_ptr += sizeof(SoundData);

   printf("Load size = %d\n", buffer_ptr - buffer);

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

   success = true;
done:
   if (fp) fclose(fp);
   free(buffer);
   return success;
}
