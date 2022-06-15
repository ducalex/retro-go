/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "dma.h"
#include "apu/apu.h"
#include "controls.h"
#include "movie.h"
#include "display.h"
#ifdef NETPLAY_SUPPORT
#include "netplay.h"
#endif
#ifdef DEBUGGER
#include "debug.h"
#include "missing.h"
#endif

struct SPPU				PPU;
struct InternalPPU		IPPU;


static inline void S9xLatchCounters (bool force)
{
	if (force || (Memory.CPU_IO[0x213] & 0x80))
	{
		// Latch h and v counters, like the gun
	#ifdef DEBUGGER
		missing.h_v_latch = 1;
	#endif

		PPU.HVBeamCounterLatched = 1;
		PPU.VBeamPosLatched = (uint16) CPU.V_Counter;

		// From byuu:
		// All dots are 4 cycles long, except dots 322 and 326. dots 322 and 326 are 6 cycles long.
		// This holds true for all scanlines except scanline 240 on non-interlace odd frames.
		// The reason for this is because this scanline is only 1360 cycles long,
		// instead of 1364 like all other scanlines.
		// This makes the effective range of hscan_pos 0-339 at all times.
		int32	hc = CPU.Cycles;

		if (hc >= 1292)
			hc -= (ONE_DOT_CYCLE / 2);
		if (hc >= 1308)
			hc -= (ONE_DOT_CYCLE / 2);

		PPU.HBeamPosLatched = (uint16) (hc / ONE_DOT_CYCLE);

		Memory.CPU_IO[0x13f] |= 0x40;
	}
}

static int CyclesUntilNext (int hc, int vc)
{
	int32 total = 0;
	int vpos = CPU.V_Counter;

	if (vc - vpos > 0)
	{
		// It's still in this frame */
		// Add number of lines
		total += (vc - vpos) * SNES_CYCLES_PER_SCANLINE;
	}
	else
	{
		if (vc == vpos && (hc > CPU.Cycles))
		{
			return hc;
		}

		total += (SNES_MAX_VCOUNTER - vpos) * SNES_CYCLES_PER_SCANLINE;

		total += (vc) * SNES_CYCLES_PER_SCANLINE;
		if (vc > 240)
			total -= ONE_DOT_CYCLE;
	}

	total += hc;

	return total;
}

void S9xUpdateIRQPositions (bool initial)
{
	PPU.HTimerPosition = PPU.IRQHBeamPos * ONE_DOT_CYCLE + SNES_IRQ_TRIGGER_CYCLES;
	PPU.HTimerPosition -= PPU.IRQHBeamPos ? 0 : ONE_DOT_CYCLE;
	PPU.HTimerPosition += PPU.IRQHBeamPos > 322 ? (ONE_DOT_CYCLE / 2) : 0;
	PPU.HTimerPosition += PPU.IRQHBeamPos > 326 ? (ONE_DOT_CYCLE / 2) : 0;
	PPU.VTimerPosition = PPU.IRQVBeamPos;

	if (PPU.VTimerEnabled && PPU.VTimerPosition >= SNES_MAX_VCOUNTER)
	{
		CPU.NextIRQTimer = 0x0fffffff;
	}
	else if (!PPU.HTimerEnabled && !PPU.VTimerEnabled)
	{
		CPU.NextIRQTimer = 0x0fffffff;
	}
	else if (PPU.HTimerEnabled && !PPU.VTimerEnabled)
	{
		int v_pos = CPU.V_Counter;

		CPU.NextIRQTimer = PPU.HTimerPosition;
		if (CPU.Cycles > CPU.NextIRQTimer - SNES_IRQ_TRIGGER_CYCLES)
		{
			CPU.NextIRQTimer += SNES_CYCLES_PER_SCANLINE;
			v_pos++;
		}
	}
	else if (!PPU.HTimerEnabled && PPU.VTimerEnabled)
	{
		if (CPU.V_Counter == PPU.VTimerPosition && initial)
			CPU.NextIRQTimer = CPU.Cycles + SNES_IRQ_TRIGGER_CYCLES - ONE_DOT_CYCLE;
		else
			CPU.NextIRQTimer = CyclesUntilNext (SNES_IRQ_TRIGGER_CYCLES - ONE_DOT_CYCLE, PPU.VTimerPosition);
	}
	else
	{
		CPU.NextIRQTimer = CyclesUntilNext (PPU.HTimerPosition, PPU.VTimerPosition);
	}

#ifdef DEBUGGER
	S9xTraceFormattedMessage("--- IRQ Timer HC:%d VC:%d set %d cycles HTimer:%d Pos:%04d->%04d  VTimer:%d Pos:%03d->%03d", CPU.Cycles, CPU.V_Counter,
		CPU.NextIRQTimer, PPU.HTimerEnabled, PPU.IRQHBeamPos, PPU.HTimerPosition, PPU.VTimerEnabled, PPU.IRQVBeamPos, PPU.VTimerPosition);
#endif
}

void S9xSetPPU (uint8 Byte, uint32 Address)
{
	// MAP_PPU: $2000-$3FFF

	if (CPU.InDMAorHDMA)
	{
		if (CPU.CurrentDMAorHDMAChannel >= 0 && DMA[CPU.CurrentDMAorHDMAChannel].ReverseTransfer)
		{
			// S9xSetPPU() is called to write to DMA[].AAddress
			if ((Address & 0xff00) == 0x2100)
			{
				// Cannot access to Address Bus B ($2100-$21ff) via (H)DMA
				return;
			}
			else
			{
				// 0x2000-0x3FFF is connected to Address Bus A
				// SA1, SuperFX and SRTC are mapped here
				// I don't bother for now...
				return;
			}
		}
		else
		{
			// S9xSetPPU() is called to read from $21xx
			// Take care of DMA wrapping
			if (Address > 0x21ff)
				Address = 0x2100 + (Address & 0xff);
		}
	}

#ifdef DEBUGGER
	if (CPU.InHDMA)
		S9xTraceFormattedMessage("--- HDMA PPU %04X -> %02X", Address, Byte);
#endif

	if ((Address & 0xffc0) == 0x2140) // APUIO0, APUIO1, APUIO2, APUIO3
		// write_port will run the APU until given clock before writing value
		S9xAPUWritePort(Address & 3, Byte);
	else
	if (Address <= 0x2183)
	{
		switch (Address)
		{
			case 0x2100: // INIDISP
				if (Byte != Memory.PPU_IO[0x100])
				{
					FLUSH_REDRAW();

					if (PPU.Brightness != (Byte & 0xf))
					{
						IPPU.ColorsChanged = TRUE;
						PPU.Brightness = Byte & 0xf;
						S9xFixColourBrightness();
						S9xBuildDirectColourMaps();
						if (PPU.Brightness > IPPU.MaxBrightness)
							IPPU.MaxBrightness = PPU.Brightness;
					}

					if ((Memory.PPU_IO[0x100] & 0x80) != (Byte & 0x80))
					{
						IPPU.ColorsChanged = TRUE;
						PPU.ForcedBlanking = (Byte >> 7) & 1;
					}
				}

				if ((Memory.PPU_IO[0x100] & 0x80) && CPU.V_Counter == PPU.ScreenHeight + FIRST_VISIBLE_LINE)
				{
					PPU.OAMAddr = PPU.SavedOAMAddr;

					uint8 tmp = 0;
					if (PPU.OAMPriorityRotation)
						tmp = (PPU.OAMAddr & 0xfe) >> 1;
					if ((PPU.OAMFlip & 1) || PPU.FirstSprite != tmp)
					{
						PPU.FirstSprite = tmp;
						IPPU.OBJChanged = TRUE;
					}

					PPU.OAMFlip = 0;
				}

				break;

			case 0x2101: // OBSEL
				if (Byte != Memory.PPU_IO[0x101])
				{
					FLUSH_REDRAW();
					PPU.OBJNameBase = (Byte & 3) << 14;
					PPU.OBJNameSelect = ((Byte >> 3) & 3) << 13;
					PPU.OBJSizeSelect = (Byte >> 5) & 7;
					IPPU.OBJChanged = TRUE;
				}

				break;

			case 0x2102: // OAMADDL
				PPU.OAMAddr = ((Memory.PPU_IO[0x103] & 1) << 8) | Byte;
				PPU.OAMFlip = 0;
				PPU.OAMReadFlip = 0;
				PPU.SavedOAMAddr = PPU.OAMAddr;
				if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
				{
					PPU.FirstSprite = (PPU.OAMAddr & 0xfe) >> 1;
					IPPU.OBJChanged = TRUE;
				}

				break;

			case 0x2103: // OAMADDH
				PPU.OAMAddr = ((Byte & 1) << 8) | Memory.PPU_IO[0x102];
				PPU.OAMPriorityRotation = (Byte & 0x80) ? 1 : 0;
				if (PPU.OAMPriorityRotation)
				{
					if (PPU.FirstSprite != (PPU.OAMAddr >> 1))
					{
						PPU.FirstSprite = (PPU.OAMAddr & 0xfe) >> 1;
						IPPU.OBJChanged = TRUE;
					}
				}
				else
				{
					if (PPU.FirstSprite != 0)
					{
						PPU.FirstSprite = 0;
						IPPU.OBJChanged = TRUE;
					}
				}

				PPU.OAMFlip = 0;
				PPU.OAMReadFlip = 0;
				PPU.SavedOAMAddr = PPU.OAMAddr;

				break;

			case 0x2104: // OAMDATA
				REGISTER_2104(Byte);
				break;

			case 0x2105: // BGMODE
				if (Byte != Memory.PPU_IO[0x105])
				{
					FLUSH_REDRAW();
					PPU.BG[0].BGSize = (Byte >> 4) & 1;
					PPU.BG[1].BGSize = (Byte >> 5) & 1;
					PPU.BG[2].BGSize = (Byte >> 6) & 1;
					PPU.BG[3].BGSize = (Byte >> 7) & 1;
					PPU.BGMode = Byte & 7;
					// BJ: BG3Priority only takes effect if BGMode == 1 and the bit is set
					PPU.BG3Priority = ((Byte & 0x0f) == 0x09);
				}

				break;

			case 0x2106: // MOSAIC
				if (Byte != Memory.PPU_IO[0x106])
				{
					FLUSH_REDRAW();
					PPU.MosaicStart = CPU.V_Counter;
					if (PPU.MosaicStart > PPU.ScreenHeight)
						PPU.MosaicStart = 0;
					PPU.Mosaic = (Byte >> 4) + 1;
					PPU.BGMosaic[0] = (Byte & 1);
					PPU.BGMosaic[1] = (Byte & 2);
					PPU.BGMosaic[2] = (Byte & 4);
					PPU.BGMosaic[3] = (Byte & 8);
				}

				break;

			case 0x2107: // BG1SC
				if (Byte != Memory.PPU_IO[0x107])
				{
					FLUSH_REDRAW();
					PPU.BG[0].SCSize = Byte & 3;
					PPU.BG[0].SCBase = (Byte & 0x7c) << 8;
				}

				break;

			case 0x2108: // BG2SC
				if (Byte != Memory.PPU_IO[0x108])
				{
					FLUSH_REDRAW();
					PPU.BG[1].SCSize = Byte & 3;
					PPU.BG[1].SCBase = (Byte & 0x7c) << 8;
				}

				break;

			case 0x2109: // BG3SC
				if (Byte != Memory.PPU_IO[0x109])
				{
					FLUSH_REDRAW();
					PPU.BG[2].SCSize = Byte & 3;
					PPU.BG[2].SCBase = (Byte & 0x7c) << 8;
				}

				break;

			case 0x210a: // BG4SC
				if (Byte != Memory.PPU_IO[0x10a])
				{
					FLUSH_REDRAW();
					PPU.BG[3].SCSize = Byte & 3;
					PPU.BG[3].SCBase = (Byte & 0x7c) << 8;
				}

				break;

			case 0x210b: // BG12NBA
				if (Byte != Memory.PPU_IO[0x10b])
				{
					FLUSH_REDRAW();
					PPU.BG[0].NameBase = (Byte & 7) << 12;
					PPU.BG[1].NameBase = ((Byte >> 4) & 7) << 12;
				}

				break;

			case 0x210c: // BG34NBA
				if (Byte != Memory.PPU_IO[0x10c])
				{
					FLUSH_REDRAW();
					PPU.BG[2].NameBase = (Byte & 7) << 12;
					PPU.BG[3].NameBase = ((Byte >> 4) & 7) << 12;
				}

				break;

			case 0x210d: // BG1HOFS, M7HOFS
				PPU.BG[0].HOffset = (Byte << 8) | (PPU.BGnxOFSbyte & ~7) | ((PPU.BG[0].HOffset >> 8) & 7);
				PPU.M7HOFS = (Byte << 8) | PPU.M7byte;
				PPU.BGnxOFSbyte = Byte;
				PPU.M7byte = Byte;
				break;

			case 0x210e: // BG1VOFS, M7VOFS
				PPU.BG[0].VOffset = (Byte << 8) | PPU.BGnxOFSbyte;
				PPU.M7VOFS = (Byte << 8) | PPU.M7byte;
				PPU.BGnxOFSbyte = Byte;
				PPU.M7byte = Byte;
				break;

			case 0x210f: // BG2HOFS
				PPU.BG[1].HOffset = (Byte << 8) | (PPU.BGnxOFSbyte & ~7) | ((PPU.BG[1].HOffset >> 8) & 7);
				PPU.BGnxOFSbyte = Byte;
				break;

			case 0x2110: // BG2VOFS
				PPU.BG[1].VOffset = (Byte << 8) | PPU.BGnxOFSbyte;
				PPU.BGnxOFSbyte = Byte;
				break;

			case 0x2111: // BG3HOFS
				PPU.BG[2].HOffset = (Byte << 8) | (PPU.BGnxOFSbyte & ~7) | ((PPU.BG[2].HOffset >> 8) & 7);
				PPU.BGnxOFSbyte = Byte;
				break;

			case 0x2112: // BG3VOFS
				PPU.BG[2].VOffset = (Byte << 8) | PPU.BGnxOFSbyte;
				PPU.BGnxOFSbyte = Byte;
				break;

			case 0x2113: // BG4HOFS
				PPU.BG[3].HOffset = (Byte << 8) | (PPU.BGnxOFSbyte & ~7) | ((PPU.BG[3].HOffset >> 8) & 7);
				PPU.BGnxOFSbyte = Byte;
				break;

			case 0x2114: // BG4VOFS
				PPU.BG[3].VOffset = (Byte << 8) | PPU.BGnxOFSbyte;
				PPU.BGnxOFSbyte = Byte;
				break;

			case 0x2115: // VMAIN
				PPU.VMA.High = (Byte & 0x80) == 0 ? FALSE : TRUE;
				switch (Byte & 3)
				{
					case 0: PPU.VMA.Increment = 1;   break;
					case 1: PPU.VMA.Increment = 32;  break;
					case 2: PPU.VMA.Increment = 128; break;
					case 3: PPU.VMA.Increment = 128; break;
				}

				if (Byte & 0x0c)
				{
					const uint16 Shift[4]    = { 0, 5, 6, 7 };
					const uint16 IncCount[4] = { 0, 32, 64, 128 };

					uint8 i = (Byte & 0x0c) >> 2;
					PPU.VMA.FullGraphicCount = IncCount[i];
					PPU.VMA.Mask1 = IncCount[i] * 8 - 1;
					PPU.VMA.Shift = Shift[i];
				}
				else
					PPU.VMA.FullGraphicCount = 0;
				break;

			case 0x2116: // VMADDL
				PPU.VMA.Address &= 0xff00;
				PPU.VMA.Address |= Byte;

				S9xUpdateVRAMReadBuffer();

				break;

			case 0x2117: // VMADDH
				PPU.VMA.Address &= 0x00ff;
				PPU.VMA.Address |= Byte << 8;

				S9xUpdateVRAMReadBuffer();

				break;

			case 0x2118: // VMDATAL
				REGISTER_2118(Byte);
				break;

			case 0x2119: // VMDATAH
				REGISTER_2119(Byte);
				break;

			case 0x211a: // M7SEL
				if (Byte != Memory.PPU_IO[0x11a])
				{
					FLUSH_REDRAW();
					PPU.Mode7Repeat = Byte >> 6;
					if (PPU.Mode7Repeat == 1)
						PPU.Mode7Repeat = 0;
					PPU.Mode7VFlip = (Byte & 2) >> 1;
					PPU.Mode7HFlip = Byte & 1;
				}

				break;

			case 0x211b: // M7A
				PPU.MatrixA = PPU.M7byte | (Byte << 8);
				PPU.Need16x8Mulitply = TRUE;
				PPU.M7byte = Byte;
				break;

			case 0x211c: // M7B
				PPU.MatrixB = PPU.M7byte | (Byte << 8);
				PPU.Need16x8Mulitply = TRUE;
				PPU.M7byte = Byte;
				break;

			case 0x211d: // M7C
				PPU.MatrixC = PPU.M7byte | (Byte << 8);
				PPU.M7byte = Byte;
				break;

			case 0x211e: // M7D
				PPU.MatrixD = PPU.M7byte | (Byte << 8);
				PPU.M7byte = Byte;
				break;

			case 0x211f: // M7X
				PPU.CentreX = PPU.M7byte | (Byte << 8);
				PPU.M7byte = Byte;
				break;

			case 0x2120: // M7Y
				PPU.CentreY = PPU.M7byte | (Byte << 8);
				PPU.M7byte = Byte;
				break;

			case 0x2121: // CGADD
				PPU.CGFLIP = 0;
				PPU.CGFLIPRead = 0;
				PPU.CGADD = Byte;
				break;

			case 0x2122: // CGDATA
				REGISTER_2122(Byte);
				break;

			case 0x2123: // W12SEL
				if (Byte != Memory.PPU_IO[0x123])
				{
					FLUSH_REDRAW();
					PPU.ClipWindow1Enable[0] = !!(Byte & 0x02);
					PPU.ClipWindow1Enable[1] = !!(Byte & 0x20);
					PPU.ClipWindow2Enable[0] = !!(Byte & 0x08);
					PPU.ClipWindow2Enable[1] = !!(Byte & 0x80);
					PPU.ClipWindow1Inside[0] = !(Byte & 0x01);
					PPU.ClipWindow1Inside[1] = !(Byte & 0x10);
					PPU.ClipWindow2Inside[0] = !(Byte & 0x04);
					PPU.ClipWindow2Inside[1] = !(Byte & 0x40);
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x2124: // W34SEL
				if (Byte != Memory.PPU_IO[0x124])
				{
					FLUSH_REDRAW();
					PPU.ClipWindow1Enable[2] = !!(Byte & 0x02);
					PPU.ClipWindow1Enable[3] = !!(Byte & 0x20);
					PPU.ClipWindow2Enable[2] = !!(Byte & 0x08);
					PPU.ClipWindow2Enable[3] = !!(Byte & 0x80);
					PPU.ClipWindow1Inside[2] = !(Byte & 0x01);
					PPU.ClipWindow1Inside[3] = !(Byte & 0x10);
					PPU.ClipWindow2Inside[2] = !(Byte & 0x04);
					PPU.ClipWindow2Inside[3] = !(Byte & 0x40);
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x2125: // WOBJSEL
				if (Byte != Memory.PPU_IO[0x125])
				{
					FLUSH_REDRAW();
					PPU.ClipWindow1Enable[4] = !!(Byte & 0x02);
					PPU.ClipWindow1Enable[5] = !!(Byte & 0x20);
					PPU.ClipWindow2Enable[4] = !!(Byte & 0x08);
					PPU.ClipWindow2Enable[5] = !!(Byte & 0x80);
					PPU.ClipWindow1Inside[4] = !(Byte & 0x01);
					PPU.ClipWindow1Inside[5] = !(Byte & 0x10);
					PPU.ClipWindow2Inside[4] = !(Byte & 0x04);
					PPU.ClipWindow2Inside[5] = !(Byte & 0x40);
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x2126: // WH0
				if (Byte != Memory.PPU_IO[0x126])
				{
					FLUSH_REDRAW();
					PPU.Window1Left = Byte;
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x2127: // WH1
				if (Byte != Memory.PPU_IO[0x127])
				{
					FLUSH_REDRAW();
					PPU.Window1Right = Byte;
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x2128: // WH2
				if (Byte != Memory.PPU_IO[0x128])
				{
					FLUSH_REDRAW();
					PPU.Window2Left = Byte;
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x2129: // WH3
				if (Byte != Memory.PPU_IO[0x129])
				{
					FLUSH_REDRAW();
					PPU.Window2Right = Byte;
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x212a: // WBGLOG
				if (Byte != Memory.PPU_IO[0x12a])
				{
					FLUSH_REDRAW();
					PPU.ClipWindowOverlapLogic[0] = (Byte & 0x03);
					PPU.ClipWindowOverlapLogic[1] = (Byte & 0x0c) >> 2;
					PPU.ClipWindowOverlapLogic[2] = (Byte & 0x30) >> 4;
					PPU.ClipWindowOverlapLogic[3] = (Byte & 0xc0) >> 6;
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x212b: // WOBJLOG
				if (Byte != Memory.PPU_IO[0x12b])
				{
					FLUSH_REDRAW();
					PPU.ClipWindowOverlapLogic[4] = (Byte & 0x03);
					PPU.ClipWindowOverlapLogic[5] = (Byte & 0x0c) >> 2;
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x212c: // TM
				if (Byte != Memory.PPU_IO[0x12c])
				{
					FLUSH_REDRAW();
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x212d: // TS
				if (Byte != Memory.PPU_IO[0x12d])
				{
					FLUSH_REDRAW();
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x212e: // TMW
				if (Byte != Memory.PPU_IO[0x12e])
				{
					FLUSH_REDRAW();
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x212f: // TSW
				if (Byte != Memory.PPU_IO[0x12f])
				{
					FLUSH_REDRAW();
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x2130: // CGWSEL
				if (Byte != Memory.PPU_IO[0x130])
				{
					FLUSH_REDRAW();
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x2131: // CGADSUB
				if (Byte != Memory.PPU_IO[0x131])
				{
					FLUSH_REDRAW();
				}

				break;

			case 0x2132: // COLDATA
				if (Byte != Memory.PPU_IO[0x132])
				{
					FLUSH_REDRAW();
					if (Byte & 0x80)
						PPU.FixedColourBlue  = Byte & 0x1f;
					if (Byte & 0x40)
						PPU.FixedColourGreen = Byte & 0x1f;
					if (Byte & 0x20)
						PPU.FixedColourRed   = Byte & 0x1f;
				}

				break;

			case 0x2133: // SETINI
				if (Byte != Memory.PPU_IO[0x133])
				{
					if ((Memory.PPU_IO[0x133] ^ Byte) & 8)
					{
						FLUSH_REDRAW();
						IPPU.PseudoHires = Byte & 8;
					}

					if (Byte & 0x04)
					{
						PPU.ScreenHeight = SNES_HEIGHT_EXTENDED;
						IPPU.RenderedScreenHeight = PPU.ScreenHeight;
					}
					else
					{
						PPU.ScreenHeight = SNES_HEIGHT;
						IPPU.RenderedScreenHeight = PPU.ScreenHeight;
					}

					if ((Memory.PPU_IO[0x133] ^ Byte) & 3)
					{
						FLUSH_REDRAW();
						if ((Memory.PPU_IO[0x133] ^ Byte) & 2)
							IPPU.OBJChanged = TRUE;

						IPPU.Interlace = Byte & 1;
						IPPU.InterlaceOBJ = Byte & 2;
					}
				}

				break;

			case 0x2134: // MPYL
			case 0x2135: // MPYM
			case 0x2136: // MPYH
			case 0x2137: // SLHV
			case 0x2138: // OAMDATAREAD
			case 0x2139: // VMDATALREAD
			case 0x213a: // VMDATAHREAD
			case 0x213b: // CGDATAREAD
			case 0x213c: // OPHCT
			case 0x213d: // OPVCT
			case 0x213e: // STAT77
			case 0x213f: // STAT78
				return;

			case 0x2180: // WMDATA
				if (!CPU.InWRAMDMAorHDMA)
					REGISTER_2180(Byte);
				break;

			case 0x2181: // WMADDL
				if (!CPU.InWRAMDMAorHDMA)
				{
					PPU.WRAM &= 0x1ff00;
					PPU.WRAM |= Byte;
				}

				break;

			case 0x2182: // WMADDM
				if (!CPU.InWRAMDMAorHDMA)
				{
					PPU.WRAM &= 0x100ff;
					PPU.WRAM |= Byte << 8;
				}

				break;

			case 0x2183: // WMADDH
				if (!CPU.InWRAMDMAorHDMA)
				{
					PPU.WRAM &= 0x0ffff;
					PPU.WRAM |= Byte << 16;
					PPU.WRAM &= 0x1ffff;
				}

				break;
		}
	}
	else
	{
	#ifdef DEBUGGER
		if (Settings.TraceUnknownRegisters)
		{
			sprintf(String, "Unknown register write: $%02X->$%04X\n", Byte, Address);
			S9xMessage(S9X_TRACE, S9X_PPU_TRACE, String);
		}
	#endif
	}

	if (Address >= 0x2000 && Address < 0x2200)
		Memory.PPU_IO[Address - 0x2000] = Byte;
	else if (Address >= 0x4000 && Address < 0x4400)
		Memory.CPU_IO[Address - 0x4000] = Byte;
}

uint8 S9xGetPPU (uint32 Address)
{
	// MAP_PPU: $2000-$3FFF
	if (Address < 0x2100)
		return (OpenBus);

	if (CPU.InDMAorHDMA)
	{
		if (CPU.CurrentDMAorHDMAChannel >= 0 && !DMA[CPU.CurrentDMAorHDMAChannel].ReverseTransfer)
		{
			// S9xGetPPU() is called to read from DMA[].AAddress
			if ((Address & 0xff00) == 0x2100)
				// Cannot access to Address Bus B ($2100-$21FF) via (H)DMA
				return (OpenBus);
			else
				// $2200-$3FFF are connected to Address Bus A
				// SA1, SuperFX and SRTC are mapped here
				// I don't bother for now...
				return (OpenBus);
		}
		else
		{
			// S9xGetPPU() is called to write to $21xx
			// Take care of DMA wrapping
			if (Address > 0x21ff)
				Address = 0x2100 + (Address & 0xff);
		}
	}

	if ((Address & 0xffc0) == 0x2140) // APUIO0, APUIO1, APUIO2, APUIO3
		// read_port will run the APU until given APU time before reading value
		return (S9xAPUReadPort(Address & 3));
	else
	if (Address <= 0x2183)
    {
		uint8	byte;

		switch (Address)
		{
			case 0x2104: // OAMDATA
			case 0x2105: // BGMODE
			case 0x2106: // MOSAIC
			case 0x2108: // BG2SC
			case 0x2109: // BG3SC
			case 0x210a: // BG4SC
			case 0x2114: // BG4VOFS
			case 0x2115: // VMAIN
			case 0x2116: // VMADDL
			case 0x2118: // VMDATAL
			case 0x2119: // VMDATAH
			case 0x211a: // M7SEL
			case 0x2124: // W34SEL
			case 0x2125: // WOBJSEL
			case 0x2126: // WH0
			case 0x2128: // WH2
			case 0x2129: // WH3
			case 0x212a: // WBGLOG
				return (PPU.OpenBus1);

			case 0x2134: // MPYL
			case 0x2135: // MPYM
			case 0x2136: // MPYH
				if (PPU.Need16x8Mulitply)
				{
					int32 r = (int32) PPU.MatrixA * (int32) (PPU.MatrixB >> 8);
					Memory.PPU_IO[0x134] = (uint8) r;
					Memory.PPU_IO[0x135] = (uint8) (r >> 8);
					Memory.PPU_IO[0x136] = (uint8) (r >> 16);
					PPU.Need16x8Mulitply = FALSE;
				}
				return (PPU.OpenBus1 = Memory.PPU_IO[Address - 0x2000]);

			case 0x2137: // SLHV
				S9xLatchCounters(0);
				return (PPU.OpenBus1);

			case 0x2138: // OAMDATAREAD
				if (PPU.OAMAddr & 0x100)
				{
					if (!(PPU.OAMFlip & 1))
						byte = PPU.OAMData[(PPU.OAMAddr & 0x10f) << 1];
					else
					{
						byte = PPU.OAMData[((PPU.OAMAddr & 0x10f) << 1) + 1];
						PPU.OAMAddr = (PPU.OAMAddr + 1) & 0x1ff;
						if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
						{
							PPU.FirstSprite = (PPU.OAMAddr & 0xfe) >> 1;
							IPPU.OBJChanged = TRUE;
						}
					}
				}
				else
				{
					if (!(PPU.OAMFlip & 1))
						byte = PPU.OAMData[PPU.OAMAddr << 1];
					else
					{
						byte = PPU.OAMData[(PPU.OAMAddr << 1) + 1];
						++PPU.OAMAddr;
						if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
						{
							PPU.FirstSprite = (PPU.OAMAddr & 0xfe) >> 1;
							IPPU.OBJChanged = TRUE;
						}
					}
				}

				PPU.OAMFlip ^= 1;
				return (PPU.OpenBus1 = byte);

			case 0x2139: // VMDATALREAD
				byte = PPU.VRAMReadBuffer & 0xff;
				if (!PPU.VMA.High)
				{
					S9xUpdateVRAMReadBuffer();
					PPU.VMA.Address += PPU.VMA.Increment;
				}

				return (PPU.OpenBus1 = byte);

			case 0x213a: // VMDATAHREAD
				byte = (PPU.VRAMReadBuffer >> 8) & 0xff;
				if (PPU.VMA.High)
				{
					S9xUpdateVRAMReadBuffer();
					PPU.VMA.Address += PPU.VMA.Increment;
				}
				return (PPU.OpenBus1 = byte);

			case 0x213b: // CGDATAREAD
				if (PPU.CGFLIPRead)
					byte = (PPU.OpenBus2 & 0x80) | ((PPU.CGDATA[PPU.CGADD++] >> 8) & 0x7f);
				else
					byte = PPU.CGDATA[PPU.CGADD] & 0xff;
				PPU.CGFLIPRead ^= 1;
				return (PPU.OpenBus2 = byte);

			case 0x213c: // OPHCT
				if (PPU.HBeamFlip)
					byte = (PPU.OpenBus2 & 0xfe) | ((PPU.HBeamPosLatched >> 8) & 0x01);
				else
					byte = (uint8) PPU.HBeamPosLatched;
				PPU.HBeamFlip ^= 1;
				return (PPU.OpenBus2 = byte);

			case 0x213d: // OPVCT
				if (PPU.VBeamFlip)
					byte = (PPU.OpenBus2 & 0xfe) | ((PPU.VBeamPosLatched >> 8) & 0x01);
				else
					byte = (uint8) PPU.VBeamPosLatched;
				PPU.VBeamFlip ^= 1;
				return (PPU.OpenBus2 = byte);

			case 0x213e: // STAT77
				FLUSH_REDRAW();
				byte = (PPU.OpenBus1 & 0x10) | PPU.RangeTimeOver | 1;
				return (PPU.OpenBus1 = byte);

			case 0x213f: // STAT78
				PPU.VBeamFlip = PPU.HBeamFlip = 0;
				byte = (PPU.OpenBus2 & 0x20) | (Memory.PPU_IO[0x13f] & 0xc0) | 3;
				if (Settings.Region == S9X_PAL) byte |= 0x10;
				Memory.PPU_IO[0x13f] &= ~0x40;
				return (PPU.OpenBus2 = byte);

			case 0x2180: // WMDATA
				if (!CPU.InWRAMDMAorHDMA)
				{
					byte = Memory.RAM[PPU.WRAM++];
					PPU.WRAM &= 0x1ffff;
				}
				else
					byte = OpenBus;
				return (byte);

			default:
				return (OpenBus);
		}
	}
	else
    {
		return (OpenBus);
	}
}

void S9xSetCPU (uint8 Byte, uint32 Address)
{
	if (Address < 0x4200)
	{
		switch (Address)
		{
			case 0x4016: // JOYSER0
				S9xSetJoypadLatch(Byte & 1);
				break;

			case 0x4017: // JOYSER1
				return;

			default:
				break;
		}
	}
	else
	if ((Address & 0xff80) == 0x4300)
	{
		if (CPU.InDMAorHDMA)
			return;

		int	d = (Address >> 4) & 0x7;

		switch (Address & 0xf)
		{
			case 0x0: // 0x43x0: DMAPx
				DMA[d].ReverseTransfer        = (Byte & 0x80) ? TRUE : FALSE;
				DMA[d].HDMAIndirectAddressing = (Byte & 0x40) ? TRUE : FALSE;
				DMA[d].UnusedBit43x0          = (Byte & 0x20) ? TRUE : FALSE;
				DMA[d].AAddressDecrement      = (Byte & 0x10) ? TRUE : FALSE;
				DMA[d].AAddressFixed          = (Byte & 0x08) ? TRUE : FALSE;
				DMA[d].TransferMode           = (Byte & 7);
				return;

			case 0x1: // 0x43x1: BBADx
				DMA[d].BAddress = Byte;
				return;

			case 0x2: // 0x43x2: A1TxL
				DMA[d].AAddress &= 0xff00;
				DMA[d].AAddress |= Byte;
				return;

			case 0x3: // 0x43x3: A1TxH
				DMA[d].AAddress &= 0xff;
				DMA[d].AAddress |= Byte << 8;
				return;

			case 0x4: // 0x43x4: A1Bx
				DMA[d].ABank = Byte;
				DMA[d].MemPointer = NULL;
				return;

			case 0x5: // 0x43x5: DASxL
				DMA[d].DMACount_Or_HDMAIndirectAddress &= 0xff00;
				DMA[d].DMACount_Or_HDMAIndirectAddress |= Byte;
				DMA[d].MemPointer = NULL;
				return;

			case 0x6: // 0x43x6: DASxH
				DMA[d].DMACount_Or_HDMAIndirectAddress &= 0xff;
				DMA[d].DMACount_Or_HDMAIndirectAddress |= Byte << 8;
				DMA[d].MemPointer = NULL;
				return;

			case 0x7: // 0x43x7: DASBx
				DMA[d].IndirectBank = Byte;
				DMA[d].MemPointer = NULL;
				return;

			case 0x8: // 0x43x8: A2AxL
				DMA[d].Address &= 0xff00;
				DMA[d].Address |= Byte;
				DMA[d].MemPointer = NULL;
				return;

			case 0x9: // 0x43x9: A2AxH
				DMA[d].Address &= 0xff;
				DMA[d].Address |= Byte << 8;
				DMA[d].MemPointer = NULL;
				return;

			case 0xa: // 0x43xa: NLTRx
				if (Byte & 0x7f)
				{
					DMA[d].LineCount = Byte & 0x7f;
					DMA[d].Repeat = !(Byte & 0x80);
				}
				else
				{
					DMA[d].LineCount = 128;
					DMA[d].Repeat = !!(Byte & 0x80);
				}

				return;

			case 0xb: // 0x43xb: ????x
			case 0xf: // 0x43xf: mirror of 0x43xb
				DMA[d].UnknownByte = Byte;
				return;

			default:
				break;
		}
	}
	else
	{
		uint16	pos;

		switch (Address)
		{
			case 0x4200: // NMITIMEN
				#ifdef DEBUGGER
				if (Settings.TraceHCEvent)
					S9xTraceFormattedMessage("Write to 0x4200. Byte is %2x was %2x\n", Byte, Memory.CPU_IO[0x200]);
				#endif

				if (Byte == Memory.CPU_IO[0x200])
					break;

				if (Byte & 0x20)
				{
					PPU.VTimerEnabled = TRUE;

					#ifdef DEBUGGER
					missing.virq = 1;
					missing.virq_pos = PPU.IRQVBeamPos;
					#endif
				}
				else
					PPU.VTimerEnabled = FALSE;

				if (Byte & 0x10)
				{
					PPU.HTimerEnabled = TRUE;

					#ifdef DEBUGGER
					missing.hirq = 1;
					missing.hirq_pos = PPU.IRQHBeamPos;
					#endif
				}
				else
					PPU.HTimerEnabled = FALSE;

				if (!(Byte & 0x10) && !(Byte & 0x20))
				{
					CPU.IRQLine = FALSE;
					CPU.IRQTransition = FALSE;
				}

				if ((Byte & 0x30) != (Memory.CPU_IO[0x200] & 0x30))
				{
					// Only allow instantaneous IRQ if turning it completely on or off
					if ((Byte & 0x30) == 0 || (Memory.CPU_IO[0x200] & 0x30) == 0)
						S9xUpdateIRQPositions(true);
					else
						S9xUpdateIRQPositions(false);
				}

				// NMI can trigger immediately during VBlank as long as NMI_read ($4210) wasn't cleard.
				if ((Byte & 0x80) && !(Memory.CPU_IO[0x200] & 0x80) &&
					(CPU.V_Counter >= PPU.ScreenHeight + FIRST_VISIBLE_LINE) && (Memory.CPU_IO[0x210] & 0x80))
				{
					// FIXME: triggered at HC+=6, checked just before the final CPU cycle,
					// then, when to call S9xOpcode_NMI()?
					CPU.IRQFlagChanging |= IRQ_TRIGGER_NMI;

					#ifdef DEBUGGER
					if (Settings.TraceHCEvent)
						S9xTraceFormattedMessage("NMI Triggered on low-to-high occurring at next HC=%d\n", CPU.NMITriggerPos);
					#endif
				}

				#ifdef DEBUGGER
				S9xTraceFormattedMessage("--- IRQ Timer Enable HTimer:%d Pos:%04d  VTimer:%d Pos:%03d",
				PPU.HTimerEnabled, PPU.HTimerPosition, PPU.VTimerEnabled, PPU.VTimerPosition);
				#endif

				break;

			case 0x4201: // WRIO
				if ((Byte & 0x80) == 0 && (Memory.CPU_IO[0x213] & 0x80) == 0x80)
					S9xLatchCounters(1);
				Memory.CPU_IO[0x201] = Memory.CPU_IO[0x213] = Byte;
				break;

			case 0x4202: // WRMPYA
				break;

			case 0x4203: // WRMPYB
			{
				uint32 res = Memory.CPU_IO[0x202] * Byte;
				// FIXME: The update occurs 8 machine cycles after $4203 is set.
				Memory.CPU_IO[0x216] = (uint8) res;
				Memory.CPU_IO[0x217] = (uint8) (res >> 8);
				break;
			}

			case 0x4204: // WRDIVL
			case 0x4205: // WRDIVH
				break;

			case 0x4206: // WRDIVB
			{
				uint16 a = Memory.CPU_IO[0x204] + (Memory.CPU_IO[0x205] << 8);
				uint16 div = Byte ? a / Byte : 0xffff;
				uint16 rem = Byte ? a % Byte : a;
				// FIXME: The update occurs 16 machine cycles after $4206 is set.
				Memory.CPU_IO[0x214] = (uint8) div;
				Memory.CPU_IO[0x215] = div >> 8;
				Memory.CPU_IO[0x216] = (uint8) rem;
				Memory.CPU_IO[0x217] = rem >> 8;
				break;
			}

			case 0x4207: // HTIMEL
				pos = PPU.IRQHBeamPos;
				PPU.IRQHBeamPos = (PPU.IRQHBeamPos & 0xff00) | Byte;
				if (PPU.IRQHBeamPos != pos)
					S9xUpdateIRQPositions(false);
				break;

			case 0x4208: // HTIMEH
				pos = PPU.IRQHBeamPos;
				PPU.IRQHBeamPos = (PPU.IRQHBeamPos & 0xff) | ((Byte & 1) << 8);
				if (PPU.IRQHBeamPos != pos)
					S9xUpdateIRQPositions(false);
				break;

			case 0x4209: // VTIMEL
				pos = PPU.IRQVBeamPos;
				PPU.IRQVBeamPos = (PPU.IRQVBeamPos & 0xff00) | Byte;
				if (PPU.IRQVBeamPos != pos)
					S9xUpdateIRQPositions(true);
				break;

			case 0x420a: // VTIMEH
				pos = PPU.IRQVBeamPos;
				PPU.IRQVBeamPos = (PPU.IRQVBeamPos & 0xff) | ((Byte & 1) << 8);
				if (PPU.IRQVBeamPos != pos)
					S9xUpdateIRQPositions(true);
				break;

			case 0x420b: // MDMAEN
				if (CPU.InDMAorHDMA)
					return;
				// XXX: Not quite right...
				if (Byte) {
					CPU.Cycles += SNES_DMA_CPU_SYNC_CYCLES;
				}
				if (Byte & 0x01)
					S9xDoDMA(0);
				if (Byte & 0x02)
					S9xDoDMA(1);
				if (Byte & 0x04)
					S9xDoDMA(2);
				if (Byte & 0x08)
					S9xDoDMA(3);
				if (Byte & 0x10)
					S9xDoDMA(4);
				if (Byte & 0x20)
					S9xDoDMA(5);
				if (Byte & 0x40)
					S9xDoDMA(6);
				if (Byte & 0x80)
					S9xDoDMA(7);
			#ifdef DEBUGGER
				missing.dma_this_frame = Byte;
				missing.dma_channels = Byte;
			#endif
				break;

			case 0x420c: // HDMAEN
				if (CPU.InDMAorHDMA)
					return;
				Memory.CPU_IO[0x20c] = Byte;
				// Yoshi's Island, Genjyu Ryodan, Mortal Kombat, Tales of Phantasia
				PPU.HDMA = Byte & ~PPU.HDMAEnded;
			#ifdef DEBUGGER
				missing.hdma_this_frame |= Byte;
				missing.hdma_channels |= Byte;
			#endif
				break;

			case 0x420d: // MEMSEL
				if ((Byte & 1) != (Memory.CPU_IO[0x20d] & 1))
				{
					if (Byte & 1)
					{
						CPU.FastROMSpeed = ONE_CYCLE;
					#ifdef DEBUGGER
						missing.fast_rom = 1;
					#endif
					}
					else
						CPU.FastROMSpeed = SLOW_ONE_CYCLE;
					// we might currently be in FastROMSpeed region, S9xSetPCBase will update CPU.MemSpeed
					S9xSetPCBase(Registers.PBPC);
				}

				break;

			case 0x4210: // RDNMI
			case 0x4211: // TIMEUP
			case 0x4212: // HVBJOY
			case 0x4213: // RDIO
			case 0x4214: // RDDIVL
			case 0x4215: // RDDIVH
			case 0x4216: // RDMPYL
			case 0x4217: // RDMPYH
			case 0x4218: // JOY1L
			case 0x4219: // JOY1H
			case 0x421a: // JOY2L
			case 0x421b: // JOY2H
			case 0x421c: // JOY3L
			case 0x421d: // JOY3H
			case 0x421e: // JOY4L
			case 0x421f: // JOY4H
				return;

			default:
				break;
		}
	}

	if (Address >= 0x2000 && Address < 0x2200)
		Memory.PPU_IO[Address - 0x2000] = Byte;
	else if (Address >= 0x4000 && Address < 0x4400)
		Memory.CPU_IO[Address - 0x4000] = Byte;
}

uint8 S9xGetCPU (uint32 Address)
{
	if (Address < 0x4200)
	{
		switch (Address)
		{
			case 0x4016: // JOYSER0
			case 0x4017: // JOYSER1
				return (S9xReadJOYSERn(Address));

			default:
				return (OpenBus);
		}
	}
	else
	if ((Address & 0xff80) == 0x4300)
	{
		if (CPU.InDMAorHDMA)
			return (OpenBus);

		int	d = (Address >> 4) & 0x7;

		switch (Address & 0xf)
		{
			case 0x0: // 0x43x0: DMAPx
				return ((DMA[d].ReverseTransfer        ? 0x80 : 0) |
						(DMA[d].HDMAIndirectAddressing ? 0x40 : 0) |
						(DMA[d].UnusedBit43x0          ? 0x20 : 0) |
						(DMA[d].AAddressDecrement      ? 0x10 : 0) |
						(DMA[d].AAddressFixed          ? 0x08 : 0) |
						(DMA[d].TransferMode & 7));

			case 0x1: // 0x43x1: BBADx
				return (DMA[d].BAddress);

			case 0x2: // 0x43x2: A1TxL
				return (DMA[d].AAddress & 0xff);

			case 0x3: // 0x43x3: A1TxH
				return (DMA[d].AAddress >> 8);

			case 0x4: // 0x43x4: A1Bx
				return (DMA[d].ABank);

			case 0x5: // 0x43x5: DASxL
				return (DMA[d].DMACount_Or_HDMAIndirectAddress & 0xff);

			case 0x6: // 0x43x6: DASxH
				return (DMA[d].DMACount_Or_HDMAIndirectAddress >> 8);

			case 0x7: // 0x43x7: DASBx
				return (DMA[d].IndirectBank);

			case 0x8: // 0x43x8: A2AxL
				return (DMA[d].Address & 0xff);

			case 0x9: // 0x43x9: A2AxH
				return (DMA[d].Address >> 8);

			case 0xa: // 0x43xa: NLTRx
				return (DMA[d].LineCount ^ (DMA[d].Repeat ? 0x00 : 0x80));

			case 0xb: // 0x43xb: ????x
			case 0xf: // 0x43xf: mirror of 0x43xb
				return (DMA[d].UnknownByte);

			default:
				return (OpenBus);
		}
	}
	else
	{
		uint8	byte;

		switch (Address)
		{
			case 0x4210: // RDNMI
				byte = Memory.CPU_IO[0x210];
				Memory.CPU_IO[0x210] = 2;
				return ((byte & 0x80) | (OpenBus & 0x70) | 2);

			case 0x4211: // TIMEUP
				byte = 0;
				if (CPU.IRQLine)
				{
					byte = 0x80;
					CPU.IRQLine = FALSE;
					CPU.IRQTransition = FALSE;
				}

				return (byte | (OpenBus & 0x7f));

			case 0x4212: // HVBJOY
				return (REGISTER_4212() | (OpenBus & 0x3e));

			case 0x4213: // RDIO
			case 0x4214: // RDDIVL
			case 0x4215: // RDDIVH
			case 0x4216: // RDMPYL
			case 0x4217: // RDMPYH
			case 0x4218: // JOY1L
			case 0x4219: // JOY1H
			case 0x421a: // JOY2L
			case 0x421b: // JOY2H
			case 0x421c: // JOY3L
			case 0x421d: // JOY3H
			case 0x421e: // JOY4L
			case 0x421f: // JOY4H
				return (Memory.CPU_IO[Address - 0x4000]);

			default:
				return (OpenBus);
		}
	}
}

void S9xResetPPU (void)
{
	S9xSoftResetPPU();
	S9xControlsReset();
	PPU.M7HOFS = 0;
	PPU.M7VOFS = 0;
	PPU.M7byte = 0;
}

void S9xResetPPUFast (void)
{
	PPU.RecomputeClipWindows = TRUE;
	IPPU.ColorsChanged = TRUE;
	IPPU.OBJChanged = TRUE;
	memset(IPPU.TileCache, 0, sizeof(IPPU.TileCache));

	S9xFixColourBrightness();
	S9xBuildDirectColourMaps();
}

void S9xSoftResetPPU (void)
{
	S9xControlsReset();

	PPU.VMA.High = 0;
	PPU.VMA.Increment = 1;
	PPU.VMA.Address = 0;
	PPU.VMA.FullGraphicCount = 0;
	PPU.VMA.Shift = 0;

	PPU.WRAM = 0;

	for (int c = 0; c < 4; c++)
	{
		PPU.BG[c].SCBase = 0;
		PPU.BG[c].HOffset = 0;
		PPU.BG[c].VOffset = 0;
		PPU.BG[c].BGSize = 0;
		PPU.BG[c].NameBase = 0;
		PPU.BG[c].SCSize = 0;
	}

	PPU.BGMode = 0;
	PPU.BG3Priority = 0;

	PPU.CGFLIP = 0;
	PPU.CGFLIPRead = 0;
	PPU.CGADD = 0;

	for (int c = 0; c < 256; c++)
	{
		PPU.CGDATA[c] = ((c & 7) << 2) | (((c >> 3) & 7) << 2) | (((c >> 6) & 2) << 3);
	}

	for (int c = 0; c < 128; c++)
	{
		PPU.OBJ[c].HPos = 0;
		PPU.OBJ[c].VPos = 0;
		PPU.OBJ[c].HFlip = 0;
		PPU.OBJ[c].VFlip = 0;
		PPU.OBJ[c].Name = 0;
		PPU.OBJ[c].Priority = 0;
		PPU.OBJ[c].Palette = 0;
		PPU.OBJ[c].Size = 0;
	}

	PPU.OBJNameBase = 0;
	PPU.OBJNameSelect = 0;
	PPU.OBJSizeSelect = 0;

	PPU.OAMAddr = 0;
	PPU.SavedOAMAddr = 0;
	PPU.OAMPriorityRotation = 0;
	PPU.OAMFlip = 0;
	PPU.OAMReadFlip = 0;
	PPU.OAMTileAddress = 0;
	PPU.OAMWriteRegister = 0;
	memset(PPU.OAMData, 0, 512 + 32);

	PPU.FirstSprite = 0;
	PPU.LastSprite = 127;
	PPU.RangeTimeOver = 0;

	PPU.HTimerEnabled = FALSE;
	PPU.VTimerEnabled = FALSE;
	PPU.HTimerPosition = SNES_CYCLES_PER_SCANLINE + 1;
	PPU.VTimerPosition = SNES_MAX_VCOUNTER + 1;
	PPU.IRQHBeamPos = 0x1ff;
	PPU.IRQVBeamPos = 0x1ff;

	PPU.HBeamFlip = 0;
	PPU.VBeamFlip = 0;
	PPU.HBeamPosLatched = 0;
	PPU.VBeamPosLatched = 0;
	PPU.GunHLatch = 0;
	PPU.GunVLatch = 1000;
	PPU.HVBeamCounterLatched = 0;

	PPU.Mode7HFlip = FALSE;
	PPU.Mode7VFlip = FALSE;
	PPU.Mode7Repeat = 0;
	PPU.MatrixA = 0;
	PPU.MatrixB = 0;
	PPU.MatrixC = 0;
	PPU.MatrixD = 0;
	PPU.CentreX = 0;
	PPU.CentreY = 0;

	PPU.Mosaic = 0;
	PPU.BGMosaic[0] = FALSE;
	PPU.BGMosaic[1] = FALSE;
	PPU.BGMosaic[2] = FALSE;
	PPU.BGMosaic[3] = FALSE;

	PPU.Window1Left = 1;
	PPU.Window1Right = 0;
	PPU.Window2Left = 1;
	PPU.Window2Right = 0;
	PPU.RecomputeClipWindows = TRUE;

	for (int c = 0; c < 6; c++)
	{
		PPU.ClipCounts[c] = 0;
		PPU.ClipWindowOverlapLogic[c] = CLIP_OR;
		PPU.ClipWindow1Enable[c] = FALSE;
		PPU.ClipWindow2Enable[c] = FALSE;
		PPU.ClipWindow1Inside[c] = TRUE;
		PPU.ClipWindow2Inside[c] = TRUE;
	}

	PPU.ForcedBlanking = TRUE;

	PPU.FixedColourRed = 0;
	PPU.FixedColourGreen = 0;
	PPU.FixedColourBlue = 0;
	PPU.Brightness = 0;
	PPU.ScreenHeight = SNES_HEIGHT;

	PPU.Need16x8Mulitply = FALSE;
	PPU.BGnxOFSbyte = 0;

	PPU.HDMA = 0;
	PPU.HDMAEnded = 0;

	PPU.OpenBus1 = 0;
	PPU.OpenBus2 = 0;

	for (int c = 0; c < 2; c++)
		memset(&IPPU.Clip[c], 0, sizeof(struct ClipData));
	IPPU.ColorsChanged = TRUE;
	IPPU.OBJChanged = TRUE;
	memset(IPPU.TileCache, 0, sizeof(IPPU.TileCache));
	PPU.VRAMReadBuffer = 0; // XXX: FIXME: anything better?
	IPPU.Interlace = FALSE;
	IPPU.InterlaceOBJ = FALSE;
	IPPU.CurrentLine = 0;
	IPPU.PreviousLine = 0;
	IPPU.XB = NULL;
	for (int c = 0; c < 256; c++)
		IPPU.ScreenColors[c] = c;
	IPPU.MaxBrightness = 0;
	IPPU.RenderThisFrame = TRUE;
	IPPU.RenderedScreenWidth = SNES_WIDTH;
	IPPU.RenderedScreenHeight = SNES_HEIGHT;
	IPPU.FrameCount = 0;
	IPPU.RenderedFramesCount = 0;
	IPPU.TotalEmulatedFrames = 0;
	IPPU.DisplayedRenderedFrameCount = 0;
	IPPU.SkippedFrames = 0;
	IPPU.FrameSkip = 0;

	S9xFixColourBrightness();
	S9xBuildDirectColourMaps();

	memset(&Memory.PPU_IO, 0, 0x200);
	memset(&Memory.CPU_IO, 0, 0x400);

	// for (int c = 0x0000; c < 0x2800; c += 0x100)
	// 	memset(&Memory.FillRAM[c], c >> 8, 0x100);

	Memory.PPU_IO[0x126] = Memory.PPU_IO[0x128] = 0x01;
	Memory.CPU_IO[0x201] = Memory.CPU_IO[0x213] = 0xFF;
}
