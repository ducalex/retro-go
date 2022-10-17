/* This file is part of Snes9x. See LICENSE file. */

#ifndef _65c816_h_
#define _65c816_h_

#define AL A.B.l
#define AH A.B.h
#define XL X.B.l
#define XH X.B.h
#define YL Y.B.l
#define YH Y.B.h
#define SL S.B.l
#define SH S.B.h
#define DL D.B.l
#define DH D.B.h
#define PL P.B.l
#define PH P.B.h

#define Carry       1
#define Zero        2
#define IRQ         4
#define Decimal     8
#define IndexFlag   16
#define MemoryFlag  32
#define Overflow    64
#define Negative    128
#define Emulation   256

#define SetCarry()       (ICPU._Carry = 1)
#define ClearCarry()     (ICPU._Carry = 0)
#define SetZero()        (ICPU._Zero = 0)
#define ClearZero()      (ICPU._Zero = 1)
#define SetIRQ()         (ICPU.Registers.PL |= IRQ)
#define ClearIRQ()       (ICPU.Registers.PL &= ~IRQ)
#define SetDecimal()     (ICPU.Registers.PL |= Decimal)
#define ClearDecimal()   (ICPU.Registers.PL &= ~Decimal)
#define SetIndex()       (ICPU.Registers.PL |= IndexFlag)
#define ClearIndex()     (ICPU.Registers.PL &= ~IndexFlag)
#define SetMemory()      (ICPU.Registers.PL |= MemoryFlag)
#define ClearMemory()    (ICPU.Registers.PL &= ~MemoryFlag)
#define SetOverflow()    (ICPU._Overflow = 1)
#define ClearOverflow()  (ICPU._Overflow = 0)
#define SetNegative()    (ICPU._Negative = 0x80)
#define ClearNegative()  (ICPU._Negative = 0)

#define CheckCarry()     (ICPU._Carry)
#define CheckZero()      (ICPU._Zero == 0)
#define CheckIRQ()       (ICPU.Registers.PL & IRQ)
#define CheckDecimal()   (ICPU.Registers.PL & Decimal)
#define CheckIndex()     (ICPU.Registers.PL & IndexFlag)
#define CheckMemory()    (ICPU.Registers.PL & MemoryFlag)
#define CheckOverflow()  (ICPU._Overflow)
#define CheckNegative()  (ICPU._Negative & 0x80)
#define CheckEmulation() (ICPU.Registers.P.W & Emulation)

#define SetFlags(f)      (ICPU.Registers.P.W |=  (f))
#define ClearFlags(f)    (ICPU.Registers.P.W &= ~(f))
#define CheckFlag(f)     (ICPU.Registers.PL & (f))

typedef union
{
   struct
   {
#ifdef MSB_FIRST
      uint8_t h, l;
#else
      uint8_t l, h;
#endif
   } B;

   uint16_t W;
} pair;

typedef struct
{
   uint8_t  PB;
   uint8_t  DB;
   pair     P;
   pair     A;
   pair     D;
   pair     S;
   pair     X;
   pair     Y;
   uint16_t PC;
} SRegisters;

#endif
