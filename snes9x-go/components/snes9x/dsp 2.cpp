/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memory.h"
#ifdef DEBUGGER
#include "debug.h"
#endif

uint8	(*GetDSP) (uint16)        = NULL;
void	(*SetDSP) (uint8, uint16) = NULL;

struct SDSP0	DSP0;
struct SDSP1	DSP1;
struct SDSP2	DSP2;


void S9xResetDSP (void)
{
	memset(&DSP1, 0, sizeof(DSP1));
	DSP1.waiting4command = TRUE;
	DSP1.first_parameter = TRUE;

	memset(&DSP2, 0, sizeof(DSP2));
	DSP2.waiting4command = TRUE;
}

uint8 S9xGetDSP (uint16 address)
{
#ifdef DEBUGGER
	if (Settings.TraceDSP)
	{
		sprintf(String, "DSP read: 0x%04X", address);
		S9xMessage(S9X_TRACE, S9X_TRACE_DSP1, String);
	}
#endif

	return ((*GetDSP)(address));
}

void S9xSetDSP (uint8 byte, uint16 address)
{
#ifdef DEBUGGER
	if (Settings.TraceDSP)
	{
		sprintf(String, "DSP write: 0x%04X=0x%02X", address, byte);
		S9xMessage(S9X_TRACE, S9X_TRACE_DSP1, String);
	}
#endif

	(*SetDSP)(byte, address);
}
