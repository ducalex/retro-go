/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memory.h"
#include "dma.h"
#include "apu/apu.h"
#ifdef DEBUGGER
#include "debug.h"
#endif

struct SCPUState		CPU;
struct SICPU			ICPU;
struct SRegisters		Registers;
struct SPPU				PPU;
struct InternalPPU		IPPU;
struct SDMA				DMA[8];
struct SGFX				GFX;
struct SBG				BG;
struct SDSP0			DSP0;
struct SDSP1			DSP1;
struct SDSP2			DSP2;
struct SSettings		Settings;
CMemory					Memory;				// 33448 bytes

char	String[513];
uint32	OpenBus = 0;
