/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _CPU_H_
#define _CPU_H_

#include "ppu.h"
#ifdef DEBUGGER
#include "debug.h"
#endif

#define Carry		1
#define Zero		2
#define IRQ			4
#define Decimal		8
#define IndexFlag	16
#define MemoryFlag	32
#define Overflow	64
#define Negative	128
#define Emulation	256

#define SetCarry()			(ICPU._Carry = 1)
#define ClearCarry()		(ICPU._Carry = 0)
#define SetZero()			(ICPU._Zero = 0)
#define ClearZero()			(ICPU._Zero = 1)
#define SetIRQ()			(Registers.PL |= IRQ)
#define ClearIRQ()			(Registers.PL &= ~IRQ)
#define SetDecimal()		(Registers.PL |= Decimal)
#define ClearDecimal()		(Registers.PL &= ~Decimal)
#define SetIndex()			(Registers.PL |= IndexFlag)
#define ClearIndex()		(Registers.PL &= ~IndexFlag)
#define SetMemory()			(Registers.PL |= MemoryFlag)
#define ClearMemory()		(Registers.PL &= ~MemoryFlag)
#define SetOverflow()		(ICPU._Overflow = 1)
#define ClearOverflow()		(ICPU._Overflow = 0)
#define SetNegative()		(ICPU._Negative = 0x80)
#define ClearNegative()		(ICPU._Negative = 0)

#define CheckCarry()		(ICPU._Carry)
#define CheckZero()			(ICPU._Zero == 0)
#define CheckIRQ()			(Registers.PL & IRQ)
#define CheckDecimal()		(Registers.PL & Decimal)
#define CheckIndex()		(Registers.PL & IndexFlag)
#define CheckMemory()		(Registers.PL & MemoryFlag)
#define CheckOverflow()		(ICPU._Overflow)
#define CheckNegative()		(ICPU._Negative & 0x80)
#define CheckEmulation()	(Registers.P.W & Emulation)

#define SetFlags(f)			(Registers.P.W |= (f))
#define ClearFlags(f)		(Registers.P.W &= ~(f))
#define CheckFlag(f)		(Registers.PL & (f))

typedef union
{
#ifdef LSB_FIRST
	struct { uint8	l, h; } B;
#else
	struct { uint8	h, l; } B;
#endif
	uint16	W;
}	pair;

typedef union
{
#ifdef LSB_FIRST
	struct { uint8	xPCl, xPCh, xPB, z; } B;
	struct { uint16	xPC, d; } W;
#else
	struct { uint8	z, xPB, xPCh, xPCl; } B;
	struct { uint16	d, xPC; } W;
#endif
    uint32	xPBPC;
}	PC_t;

struct SRegisters
{
	uint8	DB;
	pair	P;
	pair	A;
	pair	D;
	pair	S;
	pair	X;
	pair	Y;
	PC_t	PC;
};

#define AL		A.B.l
#define AH		A.B.h
#define XL		X.B.l
#define XH		X.B.h
#define YL		Y.B.l
#define YH		Y.B.h
#define SL		S.B.l
#define SH		S.B.h
#define DL		D.B.l
#define DH		D.B.h
#define PL		P.B.l
#define PH		P.B.h
#define PBPC	PC.xPBPC
#define PCw		PC.W.xPC
#define PCh		PC.B.xPCh
#define PCl		PC.B.xPCl
#define PB		PC.B.xPB

extern struct SRegisters	Registers;

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
