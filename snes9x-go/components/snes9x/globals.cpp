/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "dma.h"
#include "apu/apu.h"
#ifdef DEBUGGER
#include "debug.h"
#include "missing.h"
#endif

struct SCPUState		CPU;
struct SICPU			ICPU;
struct SRegisters		Registers;
struct SPPU				PPU;
struct InternalPPU		IPPU;
struct SDMA				DMA[8];
struct STimings			Timings;
struct SGFX				GFX;
struct SBG				BG;
struct SDSP0			DSP0;
struct SDSP1			DSP1;
struct SDSP2			DSP2;
struct SSettings		Settings;
#ifdef DEBUGGER
struct Missing			missing;
#endif
CMemory					Memory;				// 33448 bytes

char	String[513];
uint8	OpenBus = 0;
uint8	*HDMAMemPointers[8];
