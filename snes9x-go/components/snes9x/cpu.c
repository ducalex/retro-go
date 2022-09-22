/* This file is part of Snes9x. See LICENSE file. */

#include "snes9x.h"
#include "memmap.h"
#include "ppu.h"
#include "dsp1.h"
#include "cpuexec.h"
#include "apu.h"
#include "dma.h"
#include "srtc.h"
#include "obc1.h"

void S9xResetCPU()
{
   ICPU.Registers.PB = 0;
   ICPU.Registers.PC = S9xGetWord(0xfffc);
   ICPU.Registers.D.W = 0;
   ICPU.Registers.DB = 0;
   ICPU.Registers.SH = 1;
   ICPU.Registers.SL = 0xff;
   ICPU.Registers.XH = 0;
   ICPU.Registers.YH = 0;
   ICPU.Registers.P.W = 0;

   ICPU.ShiftedPB = 0;
   ICPU.ShiftedDB = 0;
   SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
   ClearFlags(Decimal);

   CPU.Flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
   CPU.BranchSkip = false;
   CPU.NMIActive = false;
   CPU.IRQActive = false;
   CPU.WaitingForInterrupt = false;
   CPU.InDMA = false;
   CPU.WhichEvent = HBLANK_START_EVENT;
   CPU.PC = NULL;
   CPU.PCBase = NULL;
   CPU.PCAtOpcodeStart = NULL;
   CPU.WaitAddress = NULL;
   CPU.WaitCounter = 1;
   CPU.Cycles = 188; /* This is the cycle count just after the jump to the Reset Vector. */
   CPU.NextEvent = Settings.HBlankStart;
   CPU.V_Counter = 0;
   CPU.MemSpeed = SLOW_ONE_CYCLE;
   CPU.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
   CPU.SRAMModified = false;
   CPU.NMICycleCount = 0;
   CPU.IRQCycleCount = 0;
   S9xSetPCBase(ICPU.Registers.PC);

   ICPU.S9xOpcodes = S9xOpcodesE1;
   ICPU.CPUExecuting = true;

   S9xUnpackStatus();
}

static void CommonS9xReset()
{
   memset(Memory.FillRAM, 0, FILLRAM_SIZE);
   memset(Memory.VRAM, 0x00, VRAM_SIZE);

   S9xResetCPU();
   S9xResetSRTC();

   S9xResetDMA();
   S9xResetAPU();
   S9xResetDSP1();
   if (Settings.OBC1)
      ResetOBC1();
   if (Settings.C4)
      S9xInitC4();
}

void S9xReset()
{
   CommonS9xReset();
   S9xResetPPU();
   memset(Memory.RAM, 0x55, RAM_SIZE);
}

void S9xSoftReset()
{
   CommonS9xReset();
   S9xSoftResetPPU();
}
