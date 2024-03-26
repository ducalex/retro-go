/* This file is part of Snes9x. See LICENSE file. */

#include "snes9x.h"
#include "memmap.h"
#include "ppu.h"
#include "cpuexec.h"
#include "dma.h"
#include "apu.h"

/*modified per anomie Mode 5 findings */
static const int32_t HDMA_ModeByteCounts [8] =
{
   1, 2, 2, 4, 4, 4, 2, 4
};
extern uint8_t* HDMAMemPointers [8];
extern uint8_t* HDMABasePointers [8];

/**********************************************************************************************/
/* S9xDoDMA()                                                                                 */
/* This function preforms the general dma transfer                                            */
/**********************************************************************************************/
void S9xDoDMA(uint8_t Channel)
{
   uint8_t Work;
   int32_t count;
   int32_t inc;
   SDMA* d;

   if (Channel > 7 || CPU.InDMA)
      return;

   CPU.InDMA = true;
   d = &DMA[Channel];
   count = d->TransferBytes;

   /* Prepare for custom chip DMA */
   if (count == 0)
      count = 0x10000;

   inc = d->AAddressFixed ? 0 : (!d->AAddressDecrement ? 1 : -1);

   if ((d->ABank == 0x7E || d->ABank == 0x7F) && d->BAddress == 0x80 && !d->TransferDirection)
   {
      d->AAddress += d->TransferBytes;
      /* Does an invalid DMA actually take time?
       * I'd say yes, since 'invalid' is probably just the WRAM chip
       * not being able to read and write itself at the same time */
      CPU.Cycles += (d->TransferBytes + 1) * SLOW_ONE_CYCLE;
      goto update_address;
   }
   switch (d->BAddress)
   {
      case 0x18:
      case 0x19:
         if (IPPU.RenderThisFrame)
            FLUSH_REDRAW();
         break;
   }

   if (!d->TransferDirection)
   {
      uint8_t* base;
      uint16_t p;
      /* XXX: DMA is potentially broken here for cases where we DMA across
       * XXX: memmap boundries. A possible solution would be to re-call
       * XXX: GetBasePointer whenever we cross a boundry, and when
       * XXX: GetBasePointer returns (0) to take the 'slow path' and use
       * XXX: S9xGetByte instead of *base. GetBasePointer() would want to
       * XXX: return 0 for MAP_PPU and whatever else is a register range
       * XXX: rather than a RAM/ROM block, and we'd want to detect MAP_PPU
       * XXX: (or specifically, Address Bus B addresses $2100-$21FF in
       * XXX: banks $00-$3F) specially and treat it as MAP_NONE (since
       * XXX: PPU->PPU transfers don't work).
       */

      /* reflects extra cycle used by DMA */
      CPU.Cycles += SLOW_ONE_CYCLE * (count + 1);

      base = GetBasePointer((d->ABank << 16) + d->AAddress);
      p    = d->AAddress;

      if (!base)
         base = Memory.ROM;

      if (inc > 0)
         d->AAddress += count;
      else if (inc < 0)
         d->AAddress -= count;

      if (d->TransferMode == 0 || d->TransferMode == 2 || d->TransferMode == 6)
      {
         switch (d->BAddress)
         {
            case 0x04:
               do
               {
                  Work = *(base + p);
                  REGISTER_2104(Work);
                  p += inc;
               } while (--count > 0);
               break;
            case 0x18:
               IPPU.FirstVRAMRead = true;
               if (!PPU.VMA.FullGraphicCount)
               {
                  do
                  {
                     Work = *(base + p);
                     REGISTER_2118_linear(Work);
                     p += inc;
                  } while (--count > 0);
               }
               else
               {
                  do
                  {
                     Work = *(base + p);
                     REGISTER_2118_tile(Work);
                     p += inc;
                  } while (--count > 0);
               }
               break;
            case 0x19:
               IPPU.FirstVRAMRead = true;
               if (!PPU.VMA.FullGraphicCount)
               {
                  do
                  {
                     Work = *(base + p);
                     REGISTER_2119_linear(Work);
                     p += inc;
                  } while (--count > 0);
               }
               else
               {
                  do
                  {
                     Work = *(base + p);
                     REGISTER_2119_tile(Work);
                     p += inc;
                  } while (--count > 0);
               }
               break;
            case 0x22:
               do
               {
                  Work = *(base + p);
                  REGISTER_2122(Work);
                  p += inc;
               } while (--count > 0);
               break;
            case 0x80:
               do
               {
                  Work = *(base + p);
                  REGISTER_2180(Work);
                  p += inc;
               } while (--count > 0);
               break;
            default:
               do
               {
                  Work = *(base + p);
                  S9xSetPPU(Work, 0x2100 + d->BAddress);
                  p += inc;
               } while (--count > 0);
               break;
         }
      }
      else if (d->TransferMode == 1 || d->TransferMode == 5)
      {
         if (d->BAddress == 0x18)
         {
            /* Write to V-RAM */
            IPPU.FirstVRAMRead = true;
            if (!PPU.VMA.FullGraphicCount)
            {
               while (count > 1)
               {
                  Work = *(base + p);
                  REGISTER_2118_linear(Work);
                  p += inc;

                  Work = *(base + p);
                  REGISTER_2119_linear(Work);
                  p += inc;
                  count -= 2;
               }
               if (count == 1)
               {
                  Work = *(base + p);
                  REGISTER_2118_linear(Work);
               }
            }
            else
            {
               while (count > 1)
               {
                  Work = *(base + p);
                  REGISTER_2118_tile(Work);
                  p += inc;

                  Work = *(base + p);
                  REGISTER_2119_tile(Work);
                  p += inc;
                  count -= 2;
               }
               if (count == 1)
               {
                  Work = *(base + p);
                  REGISTER_2118_tile(Work);
               }
            }
         }
         else
         {
            /* DMA mode 1 general case */
            while (count > 1)
            {
               Work = *(base + p);
               S9xSetPPU(Work, 0x2100 + d->BAddress);
               p += inc;

               Work = *(base + p);
               S9xSetPPU(Work, 0x2101 + d->BAddress);
               p += inc;
               count -= 2;
            }
            if (count == 1)
            {
               Work = *(base + p);
               S9xSetPPU(Work, 0x2100 + d->BAddress);
            }
         }
      }
      else if (d->TransferMode == 3 || d->TransferMode == 7)
      {
         do
         {
            Work = *(base + p);
            S9xSetPPU(Work, 0x2100 + d->BAddress);
            p += inc;
            if (count <= 1)
               break;

            Work = *(base + p);
            S9xSetPPU(Work, 0x2100 + d->BAddress);
            p += inc;
            if (count <= 2)
               break;

            Work = *(base + p);
            S9xSetPPU(Work, 0x2101 + d->BAddress);
            p += inc;
            if (count <= 3)
               break;

            Work = *(base + p);
            S9xSetPPU(Work, 0x2101 + d->BAddress);
            p += inc;
            count -= 4;
         } while (count > 0);
      }
      else if (d->TransferMode == 4)
      {
         do
         {
            Work = *(base + p);
            S9xSetPPU(Work, 0x2100 + d->BAddress);
            p += inc;
            if (count <= 1)
               break;

            Work = *(base + p);
            S9xSetPPU(Work, 0x2101 + d->BAddress);
            p += inc;
            if (count <= 2)
               break;

            Work = *(base + p);
            S9xSetPPU(Work, 0x2102 + d->BAddress);
            p += inc;
            if (count <= 3)
               break;

            Work = *(base + p);
            S9xSetPPU(Work, 0x2103 + d->BAddress);
            p += inc;
            count -= 4;
         } while (count > 0);
      }
   }
   else
   {
      /* XXX: DMA is potentially broken here for cases where the dest is
       * XXX: in the Address Bus B range. Note that this bad dest may not
       * XXX: cover the whole range of the DMA though, if we transfer
       * XXX: 65536 bytes only 256 of them may be Address Bus B.
       */
      do
      {
         switch (d->TransferMode)
         {
            case 0:
            case 2:
            case 6:
               Work = S9xGetPPU(0x2100 + d->BAddress);
               S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
               d->AAddress += inc;
               --count;
               break;
            case 1:
            case 5:
               Work = S9xGetPPU(0x2100 + d->BAddress);
               S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
               d->AAddress += inc;
               if (!--count)
                  break;

               Work = S9xGetPPU(0x2101 + d->BAddress);
               S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
               d->AAddress += inc;
               count--;
               break;
            case 3:
            case 7:
               Work = S9xGetPPU(0x2100 + d->BAddress);
               S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
               d->AAddress += inc;
               if (!--count)
                  break;

               Work = S9xGetPPU(0x2100 + d->BAddress);
               S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
               d->AAddress += inc;
               if (!--count)
                  break;

               Work = S9xGetPPU(0x2101 + d->BAddress);
               S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
               d->AAddress += inc;
               if (!--count)
                  break;

               Work = S9xGetPPU(0x2101 + d->BAddress);
               S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
               d->AAddress += inc;
               count--;
               break;
            case 4:
               Work = S9xGetPPU(0x2100 + d->BAddress);
               S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
               d->AAddress += inc;
               if (!--count)
                  break;

               Work = S9xGetPPU(0x2101 + d->BAddress);
               S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
               d->AAddress += inc;
               if (!--count)
                  break;

               Work = S9xGetPPU(0x2102 + d->BAddress);
               S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
               d->AAddress += inc;
               if (!--count)
                  break;

               Work = S9xGetPPU(0x2103 + d->BAddress);
               S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
               d->AAddress += inc;
               count--;
               break;
            default:
               count = 0;
               break;
         }
      } while (count);
   }
#ifndef USE_BLARGG_APU
   IAPU.APUExecuting = Settings.APUEnabled;
   APU_EXECUTE();
#endif
   while (CPU.Cycles > CPU.NextEvent)
      S9xDoHBlankProcessing();

update_address:
   /* Super Punch-Out requires that the A-BUS address be updated after the DMA transfer. */
   Memory.FillRAM[0x4302 + (Channel << 4)] = (uint8_t) d->AAddress;
   Memory.FillRAM[0x4303 + (Channel << 4)] = d->AAddress >> 8;

   /* Secret of Mana requires that the DMA bytes transfer count be set to zero when DMA has completed. */
   Memory.FillRAM [0x4305 + (Channel << 4)] = 0;
   Memory.FillRAM [0x4306 + (Channel << 4)] = 0;

   DMA[Channel].IndirectAddress = 0;
   d->TransferBytes = 0;

   CPU.InDMA = false;
}

void S9xStartHDMA(void)
{
   uint8_t i;
   IPPU.HDMA = Memory.FillRAM [0x420c];

   if (IPPU.HDMA != 0)
      CPU.Cycles += ONE_CYCLE * 3;

   for (i = 0; i < 8; i++)
   {
      if (IPPU.HDMA & (1 << i))
      {
         CPU.Cycles += SLOW_ONE_CYCLE;
         DMA [i].LineCount = 0;
         DMA [i].FirstLine = true;
         DMA [i].Address = DMA [i].AAddress;
         if (DMA[i].HDMAIndirectAddressing)
            CPU.Cycles += (SLOW_ONE_CYCLE << 2);
      }
      HDMAMemPointers [i] = NULL;
   }
}

uint8_t S9xDoHDMA(uint8_t byte)
{
   uint8_t mask;
   SDMA* p = &DMA [0];
   int32_t d = 0;
   CPU.InDMA = true;
   CPU.Cycles += ONE_CYCLE * 3;

   for (mask = 1; mask; mask <<= 1, p++, d++)
   {
      if (byte & mask)
      {
         if (!p->LineCount)
         {
            uint8_t line;
            /* remember, InDMA is set.
             * Get/Set incur no charges! */
            CPU.Cycles += SLOW_ONE_CYCLE;
            line        = S9xGetByte((p->ABank << 16) + p->Address);

            if (line == 0x80)
            {
               p->Repeat = true;
               p->LineCount = 128;
            }
            else
            {
               p->Repeat = !(line & 0x80);
               p->LineCount = line & 0x7f;
            }

            /* Disable H-DMA'ing into V-RAM (register 2118) for Hook
             * XXX: instead of p->BAddress == 0x18, make S9xSetPPU fail
             * XXX: writes to $2118/9 when appropriate
             */
            if (!p->LineCount || p->BAddress == 0x18)
            {
               byte &= ~mask;
               p->IndirectAddress += HDMAMemPointers [d] - HDMABasePointers [d];
               Memory.FillRAM [0x4305 + (d << 4)] = (uint8_t) p->IndirectAddress;
               Memory.FillRAM [0x4306 + (d << 4)] = p->IndirectAddress >> 8;
               continue;
            }

            p->Address++;
            p->FirstLine = true;
            if (p->HDMAIndirectAddressing)
            {
               p->IndirectBank = Memory.FillRAM [0x4307 + (d << 4)];
               /* again, no cycle charges while InDMA is set! */
               CPU.Cycles += SLOW_ONE_CYCLE << 2;
               p->IndirectAddress = S9xGetWord((p->ABank << 16) + p->Address);
               p->Address += 2;
            }
            else
            {
               p->IndirectBank = p->ABank;
               p->IndirectAddress = p->Address;
            }
            HDMABasePointers [d] = HDMAMemPointers [d] = S9xGetMemPointer((p->IndirectBank << 16) + p->IndirectAddress);
         }
         else
            CPU.Cycles += SLOW_ONE_CYCLE;

         if (!HDMAMemPointers [d])
         {
            if (!p->HDMAIndirectAddressing)
            {
               p->IndirectBank = p->ABank;
               p->IndirectAddress = p->Address;
            }

            if (!(HDMABasePointers [d] = HDMAMemPointers [d] = S9xGetMemPointer((p->IndirectBank << 16) + p->IndirectAddress)))
            {
               /* XXX: Instead of this, goto a slow path that first
                * XXX: verifies src!=Address Bus B, then uses
                * XXX: S9xGetByte(). Or make S9xGetByte return OpenBus
                * XXX: (probably?) for Address Bus B while inDMA.
                */
               byte &= ~mask;
               continue;
            }
         }
         if (p->Repeat && !p->FirstLine)
         {
            p->LineCount--;
            continue;
         }

         switch (p->TransferMode)
         {
            case 0:
               CPU.Cycles += SLOW_ONE_CYCLE;
               S9xSetPPU(*HDMAMemPointers [d]++, 0x2100 + p->BAddress);
               break;
            case 5:
               CPU.Cycles += 2 * SLOW_ONE_CYCLE;
               S9xSetPPU(*(HDMAMemPointers [d] + 0), 0x2100 + p->BAddress);
               S9xSetPPU(*(HDMAMemPointers [d] + 1), 0x2101 + p->BAddress);
               HDMAMemPointers [d] += 2;
               /* fall through */
            case 1:
               CPU.Cycles += 2 * SLOW_ONE_CYCLE;
               S9xSetPPU(*(HDMAMemPointers [d] + 0), 0x2100 + p->BAddress);
               S9xSetPPU(*(HDMAMemPointers [d] + 1), 0x2101 + p->BAddress);
               HDMAMemPointers [d] += 2;
               break;
            case 2:
            case 6:
               CPU.Cycles += 2 * SLOW_ONE_CYCLE;
               S9xSetPPU(*(HDMAMemPointers [d] + 0), 0x2100 + p->BAddress);
               S9xSetPPU(*(HDMAMemPointers [d] + 1), 0x2100 + p->BAddress);
               HDMAMemPointers [d] += 2;
               break;
            case 3:
            case 7:
               CPU.Cycles += 4 * SLOW_ONE_CYCLE;
               S9xSetPPU(*(HDMAMemPointers [d] + 0), 0x2100 + p->BAddress);
               S9xSetPPU(*(HDMAMemPointers [d] + 1), 0x2100 + p->BAddress);
               S9xSetPPU(*(HDMAMemPointers [d] + 2), 0x2101 + p->BAddress);
               S9xSetPPU(*(HDMAMemPointers [d] + 3), 0x2101 + p->BAddress);
               HDMAMemPointers [d] += 4;
               break;
            case 4:
               CPU.Cycles += 4 * SLOW_ONE_CYCLE;
               S9xSetPPU(*(HDMAMemPointers [d] + 0), 0x2100 + p->BAddress);
               S9xSetPPU(*(HDMAMemPointers [d] + 1), 0x2101 + p->BAddress);
               S9xSetPPU(*(HDMAMemPointers [d] + 2), 0x2102 + p->BAddress);
               S9xSetPPU(*(HDMAMemPointers [d] + 3), 0x2103 + p->BAddress);
               HDMAMemPointers [d] += 4;
               break;
         }
         if (!p->HDMAIndirectAddressing)
            p->Address += HDMA_ModeByteCounts [p->TransferMode];
         p->IndirectAddress += HDMA_ModeByteCounts [p->TransferMode];
         /* XXX: Check for p->IndirectAddress crossing a mapping boundary,
          * XXX: and invalidate HDMAMemPointers[d]
          */
         p->FirstLine = false;
         p->LineCount--;
      }
   }
   CPU.InDMA = false;
   return byte;
}

void S9xResetDMA(void)
{
   int32_t c, d;
   for (d = 0; d < 8; d++)
   {
      DMA [d].TransferDirection = false;
      DMA [d].HDMAIndirectAddressing = false;
      DMA [d].AAddressFixed = true;
      DMA [d].AAddressDecrement = false;
      DMA [d].TransferMode = 7;
      DMA [d].ABank = 0xff;
      DMA [d].AAddress = 0xffff;
      DMA [d].Address = 0xffff;
      DMA [d].BAddress = 0xff;
      DMA [d].TransferBytes = 0xffff;
      DMA [d].IndirectAddress = 0xffff;
   }
}
