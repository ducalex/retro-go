/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "dma.h"
#include "apu/apu.h"
#include "snapshot.h"
#include "cpuops.h"
#ifdef DEBUGGER
#include "debug.h"
#endif

struct SCPUState		CPU;
struct SICPU			ICPU;
struct SRegisters		Registers;


IRAM_ATTR void S9xMainLoop (void)
{
	uint32 loops = 0;
	// uint32 slow = 0;
	uint32 Op = 0;

	CPU.Flags &= ~SCAN_KEYS_FLAG;

	#define CHECK_FOR_IRQ_CHANGE() \
	if (CPU.IRQFlagChanging) \
	{ \
		if (CPU.IRQFlagChanging & IRQ_TRIGGER_NMI) \
		{ \
			CPU.NMIPending = TRUE; \
			CPU.NMITriggerPos = CPU.Cycles + 6; \
		} \
		if (CPU.IRQFlagChanging & IRQ_CLEAR_FLAG) \
			ClearIRQ(); \
		else if (CPU.IRQFlagChanging & IRQ_SET_FLAG) \
			SetIRQ(); \
		CPU.IRQFlagChanging = IRQ_NONE; \
	}

	for (;;)
	{
		#if (RETRO_LESS_ACCURATE_CPU || RETRO_LESS_ACCURATE_MEM)
			// Only check for interrupts every 15 loops. More than that breaks too many games
			// This is temporary until we improve speed elsewhere...
			if (loops--)
				goto run_opcode;
			else
				loops = 20;

			if (CPU.Cycles >= CPU.NextEvent)
				S9xDoHEventProcessing();
		#endif

		if (CPU.NMIPending)
		{
			#ifdef DEBUGGER
			if (Settings.TraceHCEvent)
			    S9xTraceFormattedMessage ("Comparing %d to %d\n", CPU.NMITriggerPos, CPU.Cycles);
			#endif
			if (CPU.NMITriggerPos <= CPU.Cycles)
			{
				CPU.NMIPending = FALSE;
				CPU.NMITriggerPos = 0xffff;
				if (CPU.WaitingForInterrupt)
				{
					CPU.WaitingForInterrupt = FALSE;
					Registers.PCw++;
					CPU.Cycles += TWO_CYCLES + ONE_DOT_CYCLE / 2;
					if (CPU.Cycles >= CPU.NextEvent)
						S9xDoHEventProcessing();
				}

				CHECK_FOR_IRQ_CHANGE();
				S9xOpcode_NMI();
			}
		}

		if (CPU.Cycles >= CPU.NextIRQTimer)
		{
			#ifdef DEBUGGER
			S9xTraceMessage ("Timer triggered\n");
			#endif

			S9xUpdateIRQPositions(false);
			CPU.IRQLine = TRUE;
		}

		if (CPU.IRQLine)
		{
			if (CPU.WaitingForInterrupt)
			{
				CPU.WaitingForInterrupt = FALSE;
				Registers.PCw++;
				CPU.Cycles += TWO_CYCLES + ONE_DOT_CYCLE / 2;
				if (CPU.Cycles >= CPU.NextEvent)
					S9xDoHEventProcessing();
			}

			if (!CheckFlag(IRQ))
			{
				/* The flag pushed onto the stack is the new value */
				CHECK_FOR_IRQ_CHANGE();
				S9xOpcode_IRQ();
			}
		}

		/* Change IRQ flag for instructions that set it only on last cycle */
		CHECK_FOR_IRQ_CHANGE();

	#ifdef DEBUGGER
		if ((CPU.Flags & BREAK_FLAG) && !(CPU.Flags & SINGLE_STEP_FLAG))
		{
			for (int Break = 0; Break != 6; Break++)
			{
				if (S9xBreakpoint[Break].Enabled &&
					S9xBreakpoint[Break].Bank == Registers.PB &&
					S9xBreakpoint[Break].Address == Registers.PCw)
				{
					if (S9xBreakpoint[Break].Enabled == 2)
						S9xBreakpoint[Break].Enabled = TRUE;
					else
						CPU.Flags |= DEBUG_MODE_FLAG;
				}
			}
		}

		if (CPU.Flags & DEBUG_MODE_FLAG)
			break;

		if (CPU.Flags & TRACE_FLAG)
			S9xTrace();

		if (CPU.Flags & SINGLE_STEP_FLAG)
		{
			CPU.Flags &= ~SINGLE_STEP_FLAG;
			CPU.Flags |= DEBUG_MODE_FLAG;
		}
	#endif

		if (CPU.Flags & SCAN_KEYS_FLAG)
		{
			#ifdef DEBUGGER
			if (!(CPU.Flags & FRAME_ADVANCE_FLAG))
			#endif
			{
				// S9xSyncSpeed();
			}

			break;
		}

	run_opcode:

		// If we're crossing a page or PCBase isn't set then we must use the slow route!
		if (CPU.PCBase == NULL || (Registers.PCw & MEMMAP_MASK) + 4 >= MEMMAP_BLOCK_SIZE)
		{
			Op = S9xGetByte(Registers.PBPC);
			OpenBus = Op;

			Registers.PCw++;
			(S9xOpcodesSlow[Op])();
			// slow++;
		}
		else
		{
			Op = CPU.PCBase[Registers.PCw];
			CPU.Cycles += CPU.MemSpeed;

			Registers.PCw++;
			(ICPU.S9xOpcodes[Op])();
		}
	}

	// printf("fast: %d /  slow: %d\n", loops - slow, slow);
}

IRAM_ATTR void S9xDoHEventProcessing (void)
{
#ifdef DEBUGGER
	const char	eventname[7][32] =
	{
		"",
		"HC_HBLANK_START_EVENT",
		"HC_HDMA_START_EVENT  ",
		"HC_HCOUNTER_MAX_EVENT",
		"HC_HDMA_INIT_EVENT   ",
		"HC_RENDER_EVENT      ",
		"HC_WRAM_REFRESH_EVENT"
	};
#endif

	do
	{
	#ifdef DEBUGGER
		if (Settings.TraceHCEvent)
			S9xTraceFormattedMessage("--- HC event processing  (%s)  expected HC:%04d  executed HC:%04d VC:%04d",
				eventname[CPU.WhichEvent], CPU.NextEvent, CPU.Cycles, CPU.V_Counter);
	#endif

		switch (CPU.WhichEvent)
		{
			case HC_HBLANK_START_EVENT:
				CPU.WhichEvent = HC_HDMA_START_EVENT;
				CPU.NextEvent  = SNES_HDMA_START_HC;
				break;

			case HC_HDMA_START_EVENT:
				CPU.WhichEvent = HC_HCOUNTER_MAX_EVENT;
				CPU.NextEvent  = SNES_CYCLES_PER_SCANLINE;

				if (PPU.HDMA && CPU.V_Counter <= PPU.ScreenHeight)
				{
				#ifdef DEBUGGER
					S9xTraceFormattedMessage("*** HDMA Transfer HC:%04d, Channel:%02x", CPU.Cycles, PPU.HDMA);
				#endif
					PPU.HDMA = S9xDoHDMA(PPU.HDMA);
				}

				break;

			case HC_HCOUNTER_MAX_EVENT:
				S9xAPUEndScanline();
				CPU.Cycles -= SNES_CYCLES_PER_SCANLINE;
				if (CPU.NMITriggerPos != 0xffff)
					CPU.NMITriggerPos -= SNES_CYCLES_PER_SCANLINE;
				if (CPU.NextIRQTimer != 0x0fffffff)
					CPU.NextIRQTimer -= SNES_CYCLES_PER_SCANLINE;
				S9xAPUSetReferenceTime(CPU.Cycles);

				CPU.V_Counter++;
				if (CPU.V_Counter >= SNES_MAX_VCOUNTER)	// V ranges from 0 to MAX_VCOUNTER - 1
				{
					CPU.V_Counter = 0;

					Memory.PPU_IO[0x13F] ^= 0x80;
					PPU.RangeTimeOver = 0;

					// FIXME: reading $4210 will wait 2 cycles, then perform reading, then wait 4 more cycles.
					Memory.CPU_IO[0x210] = 2;

					ICPU.Frame++;
					PPU.HVBeamCounterLatched = 0;
				}

				if (CPU.V_Counter == PPU.ScreenHeight + FIRST_VISIBLE_LINE)	// VBlank starts from V=225(240).
				{
					S9xEndScreenRefresh();

					CPU.Flags |= SCAN_KEYS_FLAG;

					PPU.HDMA = 0;
					// Bits 7 and 6 of $4212 are computed when read in S9xGetPPU.
					IPPU.MaxBrightness = PPU.Brightness;
					PPU.ForcedBlanking = (Memory.PPU_IO[0x100] >> 7) & 1;

					if (!PPU.ForcedBlanking)
					{
						PPU.OAMAddr = PPU.SavedOAMAddr;

						uint8	tmp = 0;

						if (PPU.OAMPriorityRotation)
							tmp = (PPU.OAMAddr & 0xFE) >> 1;
						if ((PPU.OAMFlip & 1) || PPU.FirstSprite != tmp)
						{
							PPU.FirstSprite = tmp;
							IPPU.OBJChanged = TRUE;
						}

						PPU.OAMFlip = 0;
					}

					// FIXME: writing to $4210 will wait 6 cycles.
					Memory.CPU_IO[0x210] = 0x80 | 2;
					if (Memory.CPU_IO[0x200] & 0x80)
					{
					#ifdef DEBUGGER
						if (Settings.TraceHCEvent)
							S9xTraceFormattedMessage ("NMI Scheduled for next scanline.");
					#endif
						// FIXME: triggered at HC=6, checked just before the final CPU cycle,
						// then, when to call S9xOpcode_NMI()?
						CPU.NMIPending = TRUE;
						CPU.NMITriggerPos = 6 + 6;
					}

				}

				if (CPU.V_Counter == PPU.ScreenHeight + 3)	// FIXME: not true
				{
					if (Memory.CPU_IO[0x200] & 1)
						S9xDoAutoJoypad();
				}

				if (CPU.V_Counter == FIRST_VISIBLE_LINE)	// V=1
					S9xStartScreenRefresh();

				CPU.WhichEvent = HC_HDMA_INIT_EVENT;
				CPU.NextEvent  = SNES_HDMA_INIT_HC;
				break;

			case HC_HDMA_INIT_EVENT:
				CPU.WhichEvent = HC_RENDER_EVENT;
				CPU.NextEvent  = SNES_RENDER_START_HC;

				if (CPU.V_Counter == 0)
				{
				#ifdef DEBUGGER
					S9xTraceFormattedMessage("*** HDMA Init     HC:%04d, Channel:%02x", CPU.Cycles, PPU.HDMA);
				#endif
					S9xStartHDMA();
				}
				break;

			case HC_RENDER_EVENT:
				if (CPU.V_Counter >= FIRST_VISIBLE_LINE && CPU.V_Counter <= PPU.ScreenHeight)
					S9xRenderLine((uint8) (CPU.V_Counter - FIRST_VISIBLE_LINE));

				CPU.WhichEvent = HC_WRAM_REFRESH_EVENT;
				CPU.NextEvent  = SNES_WRAM_REFRESH_HC;
				break;

			case HC_WRAM_REFRESH_EVENT:
			#ifdef DEBUGGER
				S9xTraceFormattedMessage("*** WRAM Refresh  HC:%04d", CPU.Cycles);
			#endif

				CPU.Cycles += SNES_WRAM_REFRESH_CYCLES;
				CPU.WhichEvent = HC_HBLANK_START_EVENT;
				CPU.NextEvent  = SNES_HBLANK_START_HC;
				break;
		}

	#ifdef DEBUGGER
		if (Settings.TraceHCEvent)
			S9xTraceFormattedMessage("--- HC event rescheduled (%s)  expected HC:%04d  current  HC:%04d",
				eventname[CPU.WhichEvent], CPU.NextEvent, CPU.Cycles);
	#endif
	} while (CPU.Cycles >= CPU.NextEvent);
}

static void S9xSoftResetCPU (void)
{
	CPU.Cycles = 182; // Or 188. This is the cycle count just after the jump to the Reset Vector.
	CPU.PrevCycles = CPU.Cycles;
	CPU.V_Counter = 0;
	CPU.Flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
	CPU.PCBase = NULL;
	CPU.NMIPending = FALSE;
	CPU.IRQLine = FALSE;
	CPU.MemSpeed = SLOW_ONE_CYCLE;
	CPU.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
	CPU.FastROMSpeed = SLOW_ONE_CYCLE;
	CPU.InDMA = FALSE;
	CPU.InHDMA = FALSE;
	CPU.InDMAorHDMA = FALSE;
	CPU.InWRAMDMAorHDMA = FALSE;
	CPU.HDMARanInDMA = 0;
	CPU.CurrentDMAorHDMAChannel = -1;
	CPU.WhichEvent = HC_RENDER_EVENT;
	CPU.NextEvent  = SNES_RENDER_START_HC;
	CPU.WaitingForInterrupt = FALSE;
	CPU.AutoSaveTimer = 0;
	CPU.SRAMModified = FALSE;

	Registers.PBPC = 0;
	Registers.PB = 0;
	Registers.PCw = S9xGetWord(0xfffc);
	OpenBus = Registers.PCh;
	Registers.D.W = 0;
	Registers.DB = 0;
	Registers.SH = 1;
	Registers.SL -= 3;
	Registers.XH = 0;
	Registers.YH = 0;

	ICPU.ShiftedPB = 0;
	ICPU.ShiftedDB = 0;
	SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
	ClearFlags(Decimal);

	CPU.NMITriggerPos = 0xffff;
	CPU.NextIRQTimer = 0x0fffffff;
	CPU.IRQFlagChanging = IRQ_NONE;

	S9xSetPCBase(Registers.PBPC);
	S9xFixCycles();
}

static void S9xResetCPU (void)
{
	S9xSoftResetCPU();
	Registers.SL = 0xff;
	Registers.P.W = 0;
	Registers.A.W = 0;
	Registers.X.W = 0;
	Registers.Y.W = 0;
	SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
	ClearFlags(Decimal);
}

void S9xReset (void)
{
	memset(Memory.RAM, 0x55, 0x20000);
	memset(Memory.VRAM, 0x00, 0x10000);
	memset(Memory.CPU_IO, 0, 0x400);

	S9xResetCPU();
	S9xResetPPU();
	S9xResetDMA();
	S9xResetAPU();

	if (Settings.DSP)
		S9xResetDSP();
}

void S9xSoftReset (void)
{
	memset(Memory.CPU_IO, 0, 0x400);

	S9xSoftResetCPU();
	S9xSoftResetPPU();
	S9xResetDMA();
	S9xSoftResetAPU();

	if (Settings.DSP)
		S9xResetDSP();
}
