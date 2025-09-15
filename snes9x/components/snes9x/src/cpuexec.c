/* This file is part of Snes9x. See LICENSE file. */

#include "snes9x.h"
#include "memmap.h"
#include "cpuops.h"
#include "ppu.h"
#include "cpuexec.h"
#include "gfx.h"
#include "apu.h"
#include "dma.h"


void S9xMainLoop()
{
   do
   {
      APU_EXECUTE();
      if (CPU.Flags)
      {
         if (CPU.Flags & NMI_FLAG)
         {
            if (--CPU.NMICycleCount == 0)
            {
               CPU.Flags &= ~NMI_FLAG;
               if (CPU.WaitingForInterrupt)
               {
                  CPU.WaitingForInterrupt = false;
                  CPU.PC++;
               }
               S9xOpcode_NMI();
            }
         }

         if (CPU.Flags & IRQ_PENDING_FLAG)
         {
            if (CPU.IRQCycleCount == 0)
            {
               if (CPU.WaitingForInterrupt)
               {
                  CPU.WaitingForInterrupt = false;
                  CPU.PC++;
               }
               if (CPU.IRQActive && !Settings.DisableIRQ)
               {
                  if (!CheckFlag(IRQ))
                     S9xOpcode_IRQ();
               }
               else
                  CPU.Flags &= ~IRQ_PENDING_FLAG;
            }
            else if (--CPU.IRQCycleCount == 0 && CheckFlag(IRQ))
               CPU.IRQCycleCount = 1;
         }
         if (CPU.Flags & SCAN_KEYS_FLAG)
            break;
      }

      CPU.PCAtOpcodeStart = CPU.PC;
      CPU.Cycles += CPU.MemSpeed;
      (*ICPU.S9xOpcodes [*CPU.PC++].S9xOpcode)();
      if (CPU.Cycles >= CPU.NextEvent)
         S9xDoHBlankProcessing();
   } while(true);

   ICPU.Registers.PC = CPU.PC - CPU.PCBase;
#ifndef USE_BLARGG_APU
   IAPU.Registers.PC = IAPU.PC - IAPU.RAM;
#endif

   S9xPackStatus();
#ifndef USE_BLARGG_APU
   S9xAPUPackStatus();
#endif
   CPU.Flags &= ~SCAN_KEYS_FLAG;
}

void S9xSetIRQ(uint32_t source)
{
   CPU.IRQActive |= source;
   CPU.Flags |= IRQ_PENDING_FLAG;
   CPU.IRQCycleCount = 3;
   if (CPU.WaitingForInterrupt)
   {
      /* Force IRQ to trigger immediately after WAI -
       * Final Fantasy Mystic Quest crashes without this. */
      CPU.IRQCycleCount = 0;
      CPU.WaitingForInterrupt = false;
      CPU.PC++;
   }
}

void S9xClearIRQ(uint32_t source)
{
   CLEAR_IRQ_SOURCE(source);
}

void S9xDoHBlankProcessing()
{
   CPU.WaitCounter++;
   switch (CPU.WhichEvent)
   {
   case HBLANK_START_EVENT:
      if (IPPU.HDMA && CPU.V_Counter <= PPU.ScreenHeight)
         IPPU.HDMA = S9xDoHDMA(IPPU.HDMA);
      break;
   case HBLANK_END_EVENT:
#ifndef USE_BLARGG_APU
      CPU.Cycles -= Settings.H_Max;
      if (IAPU.APUExecuting)
         APU.Cycles -= Settings.H_Max;
      else
         APU.Cycles = 0;
#else
      S9xAPUExecute();
      CPU.Cycles -= Settings.H_Max;
      S9xAPUSetReferenceTime(CPU.Cycles);
#endif
      CPU.NextEvent = -1;

      if (++CPU.V_Counter >= (Settings.PAL ? SNES_MAX_PAL_VCOUNTER : SNES_MAX_NTSC_VCOUNTER))
      {
         CPU.V_Counter = 0;
         Memory.FillRAM[0x213F] ^= 0x80;
         PPU.RangeTimeOver = 0;
         CPU.NMIActive = false;
         ICPU.Frame++;
         CPU.Flags |= SCAN_KEYS_FLAG;
         S9xStartHDMA();
      }

      if (PPU.VTimerEnabled && !PPU.HTimerEnabled && CPU.V_Counter == PPU.IRQVBeamPos)
         S9xSetIRQ(PPU_V_BEAM_IRQ_SOURCE);

      if (CPU.V_Counter == PPU.ScreenHeight + FIRST_VISIBLE_LINE)
      {
         /* Start of V-blank */
         S9xEndScreenRefresh();
         IPPU.HDMA = 0;
         /* Bits 7 and 6 of $4212 are computed when read in S9xGetPPU. */
         PPU.ForcedBlanking = (Memory.FillRAM [0x2100] >> 7) & 1;

         if (!PPU.ForcedBlanking)
         {
            uint8_t tmp = 0;
            PPU.OAMAddr = PPU.SavedOAMAddr;

            if (PPU.OAMPriorityRotation)
               tmp = (PPU.OAMAddr & 0xFE) >> 1;
            if ((PPU.OAMFlip & 1) || PPU.FirstSprite != tmp)
            {
               PPU.FirstSprite = tmp;
               IPPU.OBJChanged = true;
            }

            PPU.OAMFlip = 0;
         }

         Memory.FillRAM[0x4210] = 0x80 | SNES_5A22;
         if (Memory.FillRAM[0x4200] & 0x80)
         {
            CPU.NMIActive = true;
            CPU.Flags |= NMI_FLAG;
            CPU.NMICycleCount = CPU.NMITriggerPoint;
         }
      }

      if (CPU.V_Counter == PPU.ScreenHeight + 3)
         S9xUpdateJoypads();

      if (CPU.V_Counter == FIRST_VISIBLE_LINE)
      {
         Memory.FillRAM[0x4210] = SNES_5A22;
         CPU.Flags &= ~NMI_FLAG;
         S9xStartScreenRefresh();
      }
      if (CPU.V_Counter >= FIRST_VISIBLE_LINE && CPU.V_Counter < PPU.ScreenHeight + FIRST_VISIBLE_LINE)
         RenderLine(CPU.V_Counter - FIRST_VISIBLE_LINE);
#ifndef USE_BLARGG_APU
      if (APU.TimerEnabled [2])
      {
         APU.Timer [2] += 4;
         while (APU.Timer [2] >= APU.TimerTarget [2])
         {
            IAPU.RAM [0xff] = (IAPU.RAM [0xff] + 1) & 0xf;
            APU.Timer [2] -= APU.TimerTarget [2];
            IAPU.WaitCounter++;
            IAPU.APUExecuting = true;
         }
      }
      if (CPU.V_Counter & 1)
      {
         if (APU.TimerEnabled [0])
         {
            APU.Timer [0]++;
            if (APU.Timer [0] >= APU.TimerTarget [0])
            {
               IAPU.RAM [0xfd] = (IAPU.RAM [0xfd] + 1) & 0xf;
               APU.Timer [0] = 0;
               IAPU.WaitCounter++;
               IAPU.APUExecuting = true;
            }
         }
         if (APU.TimerEnabled [1])
         {
            APU.Timer [1]++;
            if (APU.Timer [1] >= APU.TimerTarget [1])
            {
               IAPU.RAM [0xfe] = (IAPU.RAM [0xfe] + 1) & 0xf;
               APU.Timer [1] = 0;
               IAPU.WaitCounter++;
               IAPU.APUExecuting = true;
            }
         }
      }
#endif
      break;
   case HTIMER_BEFORE_EVENT:
   case HTIMER_AFTER_EVENT:
      if (PPU.HTimerEnabled && (!PPU.VTimerEnabled || CPU.V_Counter == PPU.IRQVBeamPos))
         S9xSetIRQ(PPU_H_BEAM_IRQ_SOURCE);
      break;
   }
   S9xReschedule();
}
