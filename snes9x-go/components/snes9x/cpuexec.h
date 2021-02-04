/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _CPUEXEC_H_
#define _CPUEXEC_H_

#include "ppu.h"
#ifdef DEBUGGER
#include "debug.h"
#endif

typedef void (*S9xOpcode) (void);

struct SICPU
{
	const S9xOpcode *S9xOpcodes;
	uint32	_Carry;
	uint32	_Zero;
	uint32	_Negative;
	uint32	_Overflow;
	uint32	ShiftedPB;
	uint32	ShiftedDB;
	uint32	Frame;
	uint32	FrameAdvanceCount;
};

extern struct SICPU		ICPU;

extern const S9xOpcode	S9xOpcodesM0X0[256];
extern const S9xOpcode	S9xOpcodesM0X1[256];
extern const S9xOpcode	S9xOpcodesM1X0[256];
extern const S9xOpcode	S9xOpcodesM1X1[256];
extern const S9xOpcode	S9xOpcodesSlow[256];

void S9xMainLoop (void);
void S9xReset (void);
void S9xSoftReset (void);
void S9xDoHEventProcessing (void);
void S9xOpcode_NMI (void);
void S9xOpcode_IRQ (void);

static inline void S9xUnpackStatus (void)
{
	ICPU._Zero = (Registers.PL & Zero) == 0;
	ICPU._Negative = (Registers.PL & Negative);
	ICPU._Carry = (Registers.PL & Carry);
	ICPU._Overflow = (Registers.PL & Overflow) >> 6;
}

static inline void S9xPackStatus (void)
{
	Registers.PL &= ~(Zero | Negative | Carry | Overflow);
	Registers.PL |= ICPU._Carry | ((ICPU._Zero == 0) << 1) | (ICPU._Negative & 0x80) | (ICPU._Overflow << 6);
}

#endif
