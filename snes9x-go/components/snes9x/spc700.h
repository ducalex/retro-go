/* This file is part of Snes9x. See LICENSE file. */

#ifndef USE_BLARGG_APU

#ifndef _SPC700_H_
#define _SPC700_H_

#define Carry       1
#define Zero        2
#define Interrupt   4
#define HalfCarry   8
#define BreakFlag  16
#define DirectPageFlag 32
#define Overflow   64
#define Negative  128

#define APUClearCarry() (IAPU._Carry = 0)
#define APUSetCarry() (IAPU._Carry = 1)
#define APUSetInterrupt() (IAPU.Registers.P |= Interrupt)
#define APUClearInterrupt() (IAPU.Registers.P &= ~Interrupt)
#define APUSetHalfCarry() (IAPU.Registers.P |= HalfCarry)
#define APUClearHalfCarry() (IAPU.Registers.P &= ~HalfCarry)
#define APUSetBreak() (IAPU.Registers.P |= BreakFlag)
#define APUClearBreak() (IAPU.Registers.P &= ~BreakFlag)
#define APUSetDirectPage() (IAPU.Registers.P |= DirectPageFlag)
#define APUClearDirectPage() (IAPU.Registers.P &= ~DirectPageFlag)
#define APUSetOverflow() (IAPU._Overflow = 1)
#define APUClearOverflow() (IAPU._Overflow = 0)

#define APUCheckZero() (IAPU._Zero == 0)
#define APUCheckCarry() (IAPU._Carry)
#define APUCheckInterrupt() (IAPU.Registers.P & Interrupt)
#define APUCheckHalfCarry() (IAPU.Registers.P & HalfCarry)
#define APUCheckBreak() (IAPU.Registers.P & BreakFlag)
#define APUCheckDirectPage() (IAPU.Registers.P & DirectPageFlag)
#define APUCheckOverflow() (IAPU._Overflow)
#define APUCheckNegative() (IAPU._Zero & 0x80)

typedef union
{
   struct
   {
#ifdef MSB_FIRST
      uint8_t Y, A;
#else
      uint8_t A, Y;
#endif
   } B;

   uint16_t W;
} YAndA;

typedef struct
{
   uint8_t   P;
   YAndA     YA;
   uint8_t   X;
   uint8_t   S;
   uint16_t  PC;
} SAPURegisters;

/* Needed by ILLUSION OF GAIA */
#define ONE_APU_CYCLE 21

#define APU_EXECUTE1() \
{ \
    APU.Cycles += S9xAPUCycles [*IAPU.PC]; \
    (*S9xApuOpcodes[*IAPU.PC]) (); \
}

#define APU_EXECUTE() \
if (IAPU.APUExecuting) \
    while (APU.Cycles <= CPU.Cycles) \
       APU_EXECUTE1();

#endif
#endif
