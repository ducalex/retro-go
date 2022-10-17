/* This file is part of Snes9x. See LICENSE file. */

#ifndef USE_BLARGG_APU
#pragma GCC optimize("O3")

#include "snes9x.h"
#include "spc700.h"
#include "memmap.h"
#include "display.h"
#include "cpuexec.h"
#include "apu.h"
#include "apumem.h"

#define DECLARE_VARIABLES() \
   int8_t   Int8; \
   int16_t  Int16; \
   int32_t  Int32; \
   uint8_t  Work8, W1; \
   uint16_t Work16; \
   uint32_t Work32;

#define OP1 IAPU.PC[1]
#define OP2 IAPU.PC[2]

#define APUShutdown() \
    if (Settings.Shutdown && (IAPU.PC == IAPU.WaitAddress1 || IAPU.PC == IAPU.WaitAddress2)) \
    { \
       if (IAPU.WaitCounter == 0) \
       { \
          if (!ICPU.CPUExecuting) \
             APU.Cycles = CPU.Cycles = CPU.NextEvent; \
          else \
             IAPU.APUExecuting = false; \
       } \
       else if (IAPU.WaitCounter >= 2) \
          IAPU.WaitCounter = 1; \
       else \
          IAPU.WaitCounter--; \
    }

#define APUSetZN8(b) \
    IAPU._Zero = (b)

#define APUSetZN16(w) \
    IAPU._Zero = ((w) != 0) | ((w) >> 8)

#define TCALL(n) \
{\
    PushW (IAPU.PC - IAPU.RAM + 1); \
    IAPU.PC = IAPU.RAM + S9xAPUGetByte(0xffc0 + ((15 - n) << 1)) + \
       (S9xAPUGetByte(0xffc1 + ((15 - n) << 1)) << 8); \
}

#define SBC(a, b) \
    Int16 = (int16_t) (a) - (int16_t) (b) + (int16_t) (APUCheckCarry ()) - 1; \
    IAPU._Carry = Int16 >= 0; \
    if ((((a) ^ (b)) & 0x80) && (((a) ^ (uint8_t) Int16) & 0x80)) \
       APUSetOverflow (); \
    else \
       APUClearOverflow (); \
    APUSetHalfCarry (); \
    if(((a) ^ (b) ^ (uint8_t) Int16) & 0x10) \
       APUClearHalfCarry (); \
    (a) = (uint8_t) Int16; \
    APUSetZN8 ((uint8_t) Int16)

#define ADC(a,b) \
    Work16 = (a) + (b) + APUCheckCarry(); \
    IAPU._Carry = Work16 >= 0x100; \
    if (~((a) ^ (b)) & ((b) ^ (uint8_t) Work16) & 0x80) \
       APUSetOverflow (); \
    else \
       APUClearOverflow (); \
    APUClearHalfCarry (); \
    if(((a) ^ (b) ^ (uint8_t) Work16) & 0x10) \
       APUSetHalfCarry (); \
    (a) = (uint8_t) Work16; \
    APUSetZN8 ((uint8_t) Work16)

#define CMP(a,b) \
    Int16 = (int16_t) (a) - (int16_t) (b); \
    IAPU._Carry = Int16 >= 0; \
    APUSetZN8((uint8_t) Int16)

#define ASL(b) \
    IAPU._Carry = ((b) & 0x80) != 0; \
    (b) <<= 1; \
    APUSetZN8 (b)

#define LSR(b) \
    IAPU._Carry = (b) & 1; \
    (b) >>= 1; \
    APUSetZN8 (b)

#define ROL(b) \
    Work16 = ((b) << 1) | APUCheckCarry (); \
    IAPU._Carry = Work16 >= 0x100; \
    (b) = (uint8_t) Work16; \
    APUSetZN8 (b)

#define ROR(b) \
    Work16 = (b) | ((uint16_t) APUCheckCarry () << 8); \
    IAPU._Carry = (uint8_t) Work16 & 1; \
    Work16 >>= 1; \
    (b) = (uint8_t) Work16; \
    APUSetZN8 (b)

#define Push(b) \
    IAPU.RAM[0x100 + IAPU.Registers.S] = b; \
    IAPU.Registers.S--

#define Pop(b) \
    IAPU.Registers.S++; \
    (b) = IAPU.RAM[0x100 + IAPU.Registers.S]

#ifdef FAST_LSB_WORD_ACCESS
#define PushW(w) \
    if (IAPU.Registers.S == 0) \
    {\
       IAPU.RAM[0x1ff] = (w); \
       IAPU.RAM[0x100] = ((w) >> 8); \
    } \
    else \
       *(uint16_t *) (IAPU.RAM + 0xff + IAPU.Registers.S) = w; \
    IAPU.Registers.S -= 2

#define PopW(w) \
    IAPU.Registers.S += 2; \
    if (IAPU.Registers.S == 0) \
       (w) = IAPU.RAM[0x1ff] | (IAPU.RAM[0x100] << 8); \
    else \
       (w) = *(uint16_t *) (IAPU.RAM + 0xff + IAPU.Registers.S)
#else
#define PushW(w) \
    IAPU.RAM[0xff + IAPU.Registers.S] = w; \
    IAPU.RAM[0x100 + IAPU.Registers.S] = ((w) >> 8); \
    IAPU.Registers.S -= 2

#define PopW(w) \
    IAPU.Registers.S += 2; \
    if(IAPU.Registers.S == 0) \
       (w) = IAPU.RAM[0x1ff] | (IAPU.RAM[0x100] << 8); \
    else \
       (w) = IAPU.RAM[0xff + IAPU.Registers.S] + (IAPU.RAM[0x100 + IAPU.Registers.S] << 8)
#endif

#define Relative() \
    Int8 = OP1; \
    Int16 = (int16_t) (IAPU.PC + 2 - IAPU.RAM) + Int8;

#define Relative2() \
    Int8 = OP2; \
    Int16 = (int16_t) (IAPU.PC + 3 - IAPU.RAM) + Int8;

#ifdef FAST_LSB_WORD_ACCESS
#define IndexedXIndirect() \
    IAPU.Address = *(uint16_t *) (IAPU.DirectPage + ((OP1 + IAPU.Registers.X) & 0xff));

#define Absolute() \
    IAPU.Address = *(uint16_t *) (IAPU.PC + 1);

#define AbsoluteX() \
    IAPU.Address = *(uint16_t *) (IAPU.PC + 1) + IAPU.Registers.X;

#define AbsoluteY() \
    IAPU.Address = *(uint16_t *) (IAPU.PC + 1) + IAPU.Registers.YA.B.Y;

#define MemBit() \
    IAPU.Address = *(uint16_t *) (IAPU.PC + 1); \
    IAPU.Bit = (uint8_t)(IAPU.Address >> 13); \
    IAPU.Address &= 0x1fff;

#define IndirectIndexedY() \
    IAPU.Address = *(uint16_t *) (IAPU.DirectPage + OP1) + IAPU.Registers.YA.B.Y;
#else
#define IndexedXIndirect() \
    IAPU.Address = IAPU.DirectPage[(OP1 + IAPU.Registers.X) & 0xff] + \
       (IAPU.DirectPage[(OP1 + IAPU.Registers.X + 1) & 0xff] << 8);

#define Absolute() \
    IAPU.Address = OP1 + (OP2 << 8);

#define AbsoluteX() \
    IAPU.Address = OP1 + (OP2 << 8) + IAPU.Registers.X;

#define AbsoluteY() \
    IAPU.Address = OP1 + (OP2 << 8) + IAPU.Registers.YA.B.Y;

#define MemBit() \
    IAPU.Address = OP1 + (OP2 << 8); \
    IAPU.Bit = (int8_t) (IAPU.Address >> 13); \
    IAPU.Address &= 0x1fff;

#define IndirectIndexedY() \
    IAPU.Address = IAPU.DirectPage[OP1] + \
       (IAPU.DirectPage[OP1 + 1] << 8) + \
       IAPU.Registers.YA.B.Y;
#endif

void Apu00(void) /* NOP */
{
   IAPU.PC++;
}

void Apu01(void)
{
   DECLARE_VARIABLES();
   TCALL(0);
}

void Apu11(void)
{
   DECLARE_VARIABLES();
   TCALL(1);
}

void Apu21(void)
{
   DECLARE_VARIABLES();
   TCALL(2);
}

void Apu31(void)
{
   DECLARE_VARIABLES();
   TCALL(3);
}

void Apu41(void)
{
   DECLARE_VARIABLES();
   TCALL(4);
}

void Apu51(void)
{
   DECLARE_VARIABLES();
   TCALL(5);
}

void Apu61(void)
{
   DECLARE_VARIABLES();
   TCALL(6);
}

void Apu71(void)
{
   DECLARE_VARIABLES();
   TCALL(7);
}

void Apu81(void)
{
   DECLARE_VARIABLES();
   TCALL(8);
}

void Apu91(void)
{
   DECLARE_VARIABLES();
   TCALL(9);
}

void ApuA1(void)
{
   DECLARE_VARIABLES();
   TCALL(10);
}

void ApuB1(void)
{
   DECLARE_VARIABLES();
   TCALL(11);
}

void ApuC1(void)
{
   DECLARE_VARIABLES();
   TCALL(12);
}

void ApuD1(void)
{
   DECLARE_VARIABLES();
   TCALL(13);
}

void ApuE1(void)
{
   DECLARE_VARIABLES();
   TCALL(14);
}

void ApuF1(void)
{
   DECLARE_VARIABLES();
   TCALL(15);
}

void Apu3F(void) /* CALL absolute */
{
   Absolute();
   /* 0xB6f for Star Fox 2 */
   PushW(IAPU.PC + 3 - IAPU.RAM);
   IAPU.PC = IAPU.RAM + IAPU.Address;
}

void Apu4F(void) /* PCALL $XX */
{
   uint8_t Work8 = OP1;
   PushW(IAPU.PC + 2 - IAPU.RAM);
   IAPU.PC = IAPU.RAM + 0xff00 + Work8;
}

#define SET(b) \
S9xAPUSetByteZ ((uint8_t) (S9xAPUGetByteZ (OP1 ) | (1 << (b))), OP1); \
IAPU.PC += 2

void Apu02(void)
{
   DECLARE_VARIABLES();
   SET(0);
}

void Apu22(void)
{
   DECLARE_VARIABLES();
   SET(1);
}

void Apu42(void)
{
   DECLARE_VARIABLES();
   SET(2);
}

void Apu62(void)
{
   DECLARE_VARIABLES();
   SET(3);
}

void Apu82(void)
{
   DECLARE_VARIABLES();
   SET(4);
}

void ApuA2(void)
{
   DECLARE_VARIABLES();
   SET(5);
}

void ApuC2(void)
{
   DECLARE_VARIABLES();
   SET(6);
}

void ApuE2(void)
{
   DECLARE_VARIABLES();
   SET(7);
}

#define CLR(b) \
S9xAPUSetByteZ ((uint8_t) (S9xAPUGetByteZ (OP1) & ~(1 << (b))), OP1); \
IAPU.PC += 2;

void Apu12(void)
{
   DECLARE_VARIABLES();
   CLR(0);
}

void Apu32(void)
{
   DECLARE_VARIABLES();
   CLR(1);
}

void Apu52(void)
{
   DECLARE_VARIABLES();
   CLR(2);
}

void Apu72(void)
{
   DECLARE_VARIABLES();
   CLR(3);
}

void Apu92(void)
{
   DECLARE_VARIABLES();
   CLR(4);
}

void ApuB2(void)
{
   DECLARE_VARIABLES();
   CLR(5);
}

void ApuD2(void)
{
   DECLARE_VARIABLES();
   CLR(6);
}

void ApuF2(void)
{
   DECLARE_VARIABLES();
   CLR(7);
}

#define BBS(b) \
Work8 = OP1; \
Relative2 (); \
if (S9xAPUGetByteZ (Work8) & (1 << (b))) \
{ \
    IAPU.PC = IAPU.RAM + (uint16_t) Int16; \
    APU.Cycles += IAPU.TwoCycles; \
} \
else \
    IAPU.PC += 3

void Apu03(void)
{
   DECLARE_VARIABLES();
   BBS(0);
}

void Apu23(void)
{
   DECLARE_VARIABLES();
   BBS(1);
}

void Apu43(void)
{
   DECLARE_VARIABLES();
   BBS(2);
}

void Apu63(void)
{
   DECLARE_VARIABLES();
   BBS(3);
}

void Apu83(void)
{
   DECLARE_VARIABLES();
   BBS(4);
}

void ApuA3(void)
{
   DECLARE_VARIABLES();
   BBS(5);
}

void ApuC3(void)
{
   DECLARE_VARIABLES();
   BBS(6);
}

void ApuE3(void)
{
   DECLARE_VARIABLES();
   BBS(7);
}

#define BBC(b) \
Work8 = OP1; \
Relative2 (); \
if (!(S9xAPUGetByteZ (Work8) & (1 << (b)))) \
{ \
    IAPU.PC = IAPU.RAM + (uint16_t) Int16; \
    APU.Cycles += IAPU.TwoCycles; \
} \
else \
    IAPU.PC += 3

void Apu13(void)
{
   DECLARE_VARIABLES();
   BBC(0);
}

void Apu33(void)
{
   DECLARE_VARIABLES();
   BBC(1);
}

void Apu53(void)
{
   DECLARE_VARIABLES();
   BBC(2);
}

void Apu73(void)
{
   DECLARE_VARIABLES();
   BBC(3);
}

void Apu93(void)
{
   DECLARE_VARIABLES();
   BBC(4);
}

void ApuB3(void)
{
   DECLARE_VARIABLES();
   BBC(5);
}

void ApuD3(void)
{
   DECLARE_VARIABLES();
   BBC(6);
}

void ApuF3(void)
{
   DECLARE_VARIABLES();
   BBC(7);
}

void Apu04(void)
{
   DECLARE_VARIABLES();
   /* OR A,dp */
   IAPU.Registers.YA.B.A |= S9xAPUGetByteZ(OP1);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu05(void)
{
   DECLARE_VARIABLES();
   /* OR A,abs */
   Absolute();
   IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu06(void)
{
   DECLARE_VARIABLES();
   /* OR A,(X) */
   IAPU.Registers.YA.B.A |= S9xAPUGetByteZ(IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu07(void)
{
   DECLARE_VARIABLES();
   /* OR A,(dp+X) */
   IndexedXIndirect();
   IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu08(void)
{
   DECLARE_VARIABLES();
   /* OR A,#00 */
   IAPU.Registers.YA.B.A |= OP1;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu09(void)
{
   DECLARE_VARIABLES();
   /* OR dp(dest),dp(src) */
   Work8 = S9xAPUGetByteZ(OP1);
   Work8 |= S9xAPUGetByteZ(OP2);
   S9xAPUSetByteZ(Work8, OP2);
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu14(void)
{
   DECLARE_VARIABLES();
   /* OR A,dp+X */
   IAPU.Registers.YA.B.A |= S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu15(void)
{
   DECLARE_VARIABLES();
   /* OR A,abs+X */
   AbsoluteX();
   IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu16(void)
{
   DECLARE_VARIABLES();
   /* OR A,abs+Y */
   AbsoluteY();
   IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu17(void)
{
   DECLARE_VARIABLES();
   /* OR A,(dp)+Y */
   IndirectIndexedY();
   IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu18(void)
{
   DECLARE_VARIABLES();
   /* OR dp,#00 */
   Work8 = OP1;
   Work8 |= S9xAPUGetByteZ(OP2);
   S9xAPUSetByteZ(Work8, OP2);
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu19(void)
{
   DECLARE_VARIABLES();
   /* OR (X),(Y) */
   Work8 = S9xAPUGetByteZ(IAPU.Registers.X) | S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
   APUSetZN8(Work8);
   S9xAPUSetByteZ(Work8, IAPU.Registers.X);
   IAPU.PC++;
}

void Apu0A(void)
{
   DECLARE_VARIABLES();
   /* OR1 C,membit */
   MemBit();
   if (!APUCheckCarry())
      if (S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit))
         APUSetCarry();
   IAPU.PC += 3;
}

void Apu2A(void)
{
   DECLARE_VARIABLES();
   /* OR1 C,not membit */
   MemBit();
   if (!APUCheckCarry())
      if (!(S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit)))
         APUSetCarry();
   IAPU.PC += 3;
}

void Apu4A(void)
{
   DECLARE_VARIABLES();
   /* AND1 C,membit */
   MemBit();
   if (APUCheckCarry())
      if (!(S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit)))
         APUClearCarry();
   IAPU.PC += 3;
}

void Apu6A(void)
{
   DECLARE_VARIABLES();
   /* AND1 C, not membit */
   MemBit();
   if (APUCheckCarry())
      if ((S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit)))
         APUClearCarry();
   IAPU.PC += 3;
}

void Apu8A(void)
{
   DECLARE_VARIABLES();
   /* EOR1 C, membit */
   MemBit();
   if (S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit))
   {
      if (APUCheckCarry())
         APUClearCarry();
      else
         APUSetCarry();
   }
   IAPU.PC += 3;
}

void ApuAA(void)
{
   DECLARE_VARIABLES();
   /* MOV1 C,membit */
   MemBit();
   if (S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit))
      APUSetCarry();
   else
      APUClearCarry();
   IAPU.PC += 3;
}

void ApuCA(void)
{
   DECLARE_VARIABLES();
   /* MOV1 membit,C */
   MemBit();
   if (APUCheckCarry())
      S9xAPUSetByte(S9xAPUGetByte(IAPU.Address) | (1 << IAPU.Bit), IAPU.Address);
   else
      S9xAPUSetByte(S9xAPUGetByte(IAPU.Address) & ~(1 << IAPU.Bit), IAPU.Address);
   IAPU.PC += 3;
}

void ApuEA(void)
{
   DECLARE_VARIABLES();
   /* NOT1 membit */
   MemBit();
   S9xAPUSetByte(S9xAPUGetByte(IAPU.Address) ^ (1 << IAPU.Bit), IAPU.Address);
   IAPU.PC += 3;
}

void Apu0B(void)
{
   DECLARE_VARIABLES();
   /* ASL dp */
   Work8 = S9xAPUGetByteZ(OP1);
   ASL(Work8);
   S9xAPUSetByteZ(Work8, OP1);
   IAPU.PC += 2;
}

void Apu0C(void)
{
   DECLARE_VARIABLES();
   /* ASL abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ASL(Work8);
   S9xAPUSetByte(Work8, IAPU.Address);
   IAPU.PC += 3;
}

void Apu1B(void)
{
   DECLARE_VARIABLES();
   /* ASL dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   ASL(Work8);
   S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
   IAPU.PC += 2;
}

void Apu1C(void)
{
   DECLARE_VARIABLES();
   /* ASL A */
   ASL(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu0D(void)
{
   DECLARE_VARIABLES();
   /* PUSH PSW */
   S9xAPUPackStatus();
   Push(IAPU.Registers.P);
   IAPU.PC++;
}

void Apu2D(void)
{
   DECLARE_VARIABLES();
   /* PUSH A */
   Push(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu4D(void)
{
   DECLARE_VARIABLES();
   /* PUSH X */
   Push(IAPU.Registers.X);
   IAPU.PC++;
}

void Apu6D(void)
{
   DECLARE_VARIABLES();
   /* PUSH Y */
   Push(IAPU.Registers.YA.B.Y);
   IAPU.PC++;
}

void Apu8E(void)
{
   DECLARE_VARIABLES();
   /* POP PSW */
   Pop(IAPU.Registers.P);
   S9xAPUUnpackStatus();
   if (APUCheckDirectPage())
      IAPU.DirectPage = IAPU.RAM + 0x100;
   else
      IAPU.DirectPage = IAPU.RAM;
   IAPU.PC++;
}

void ApuAE(void)
{
   DECLARE_VARIABLES();
   /* POP A */
   Pop(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuCE(void)
{
   DECLARE_VARIABLES();
   /* POP X */
   Pop(IAPU.Registers.X);
   IAPU.PC++;
}

void ApuEE(void)
{
   DECLARE_VARIABLES();
   /* POP Y */
   Pop(IAPU.Registers.YA.B.Y);
   IAPU.PC++;
}

void Apu0E(void)
{
   DECLARE_VARIABLES();
   /* TSET1 abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   S9xAPUSetByte(Work8 | IAPU.Registers.YA.B.A, IAPU.Address);
   Work8 = IAPU.Registers.YA.B.A - Work8;
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu4E(void)
{
   DECLARE_VARIABLES();
   /* TCLR1 abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   S9xAPUSetByte(Work8 & ~IAPU.Registers.YA.B.A, IAPU.Address);
   Work8 = IAPU.Registers.YA.B.A - Work8;
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu0F(void)
{
   DECLARE_VARIABLES();
   /* BRK */
   PushW(IAPU.PC + 1 - IAPU.RAM);
   S9xAPUPackStatus();
   Push(IAPU.Registers.P);
   APUSetBreak();
   APUClearInterrupt();
   IAPU.PC = IAPU.RAM + S9xAPUGetByte(0xffde) + (S9xAPUGetByte(0xffdf) << 8);
}

void ApuEF(void)
{
   DECLARE_VARIABLES();
   /* SLEEP */
   APU.TimerEnabled[0] = APU.TimerEnabled[1] = APU.TimerEnabled[2] = false;
   IAPU.APUExecuting = false;
}

void ApuFF(void)
{
   DECLARE_VARIABLES();
   /* STOP */
   APU.TimerEnabled[0] = APU.TimerEnabled[1] = APU.TimerEnabled[2] = false;
   IAPU.APUExecuting = false;
   Settings.APUEnabled = false; /* re-enabled on next APU reset */
}

void Apu10(void)
{
   DECLARE_VARIABLES();
   /* BPL */
   Relative();
   if (!APUCheckNegative())
   {
      IAPU.PC = IAPU.RAM + (uint16_t) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 2;
}

void Apu30(void)
{
   DECLARE_VARIABLES();
   /* BMI */
   Relative();
   if (APUCheckNegative())
   {
      IAPU.PC = IAPU.RAM + (uint16_t) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 2;
}

void Apu90(void)
{
   DECLARE_VARIABLES();
   /* BCC */
   Relative();
   if (!APUCheckCarry())
   {
      IAPU.PC = IAPU.RAM + (uint16_t) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 2;
}

void ApuB0(void)
{
   DECLARE_VARIABLES();
   /* BCS */
   Relative();
   if (APUCheckCarry())
   {
      IAPU.PC = IAPU.RAM + (uint16_t) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 2;
}

void ApuD0(void)
{
   DECLARE_VARIABLES();
   /* BNE */
   Relative();
   if (!APUCheckZero())
   {
      IAPU.PC = IAPU.RAM + (uint16_t) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 2;
}

void ApuF0(void)
{
   DECLARE_VARIABLES();
   /* BEQ */
   Relative();
   if (APUCheckZero())
   {
      IAPU.PC = IAPU.RAM + (uint16_t) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 2;
}

void Apu50(void)
{
   DECLARE_VARIABLES();
   /* BVC */
   Relative();
   if (!APUCheckOverflow())
   {
      IAPU.PC = IAPU.RAM + (uint16_t) Int16;
      APU.Cycles += IAPU.TwoCycles;
   }
   else
      IAPU.PC += 2;
}

void Apu70(void)
{
   DECLARE_VARIABLES();
   /* BVS */
   Relative();
   if (APUCheckOverflow())
   {
      IAPU.PC = IAPU.RAM + (uint16_t) Int16;
      APU.Cycles += IAPU.TwoCycles;
   }
   else
      IAPU.PC += 2;
}

void Apu2F(void)
{
   DECLARE_VARIABLES();
   /* BRA */
   Relative();
   IAPU.PC = IAPU.RAM + (uint16_t) Int16;
}

void Apu80(void)
{
   DECLARE_VARIABLES();
   /* SETC */
   APUSetCarry();
   IAPU.PC++;
}

void ApuED(void)
{
   DECLARE_VARIABLES();
   /* NOTC */
   IAPU._Carry ^= 1;
   IAPU.PC++;
}

void Apu40(void)
{
   DECLARE_VARIABLES();
   /* SETP */
   APUSetDirectPage();
   IAPU.DirectPage = IAPU.RAM + 0x100;
   IAPU.PC++;
}

void Apu1A(void)
{
   DECLARE_VARIABLES();
   /* DECW dp */
   Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8) - 1;
   S9xAPUSetByteZ((uint8_t) Work16, OP1);
   S9xAPUSetByteZ(Work16 >> 8, OP1 + 1);
   APUSetZN16(Work16);
   IAPU.PC += 2;
}

void Apu5A(void)
{
   DECLARE_VARIABLES();
   /* CMPW YA,dp */
   Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8);
   Int32 = (int32_t) IAPU.Registers.YA.W - (int32_t) Work16;
   IAPU._Carry = Int32 >= 0;
   APUSetZN16((uint16_t) Int32);
   IAPU.PC += 2;
}

void Apu3A(void)
{
   DECLARE_VARIABLES();
   /* INCW dp */
   Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8) + 1;
   S9xAPUSetByteZ((uint8_t) Work16, OP1);
   S9xAPUSetByteZ(Work16 >> 8, OP1 + 1);
   APUSetZN16(Work16);
   IAPU.PC += 2;
}

void Apu7A(void)
{
   DECLARE_VARIABLES();
   /* ADDW YA,dp */
   Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8);
   Work32 = (uint32_t) IAPU.Registers.YA.W + Work16;
   IAPU._Carry = Work32 >= 0x10000;
   if (~(IAPU.Registers.YA.W ^ Work16) & (Work16 ^ (uint16_t) Work32) & 0x8000)
      APUSetOverflow();
   else
      APUClearOverflow();
   APUClearHalfCarry();
   if ((IAPU.Registers.YA.W ^ Work16 ^ (uint16_t) Work32) & 0x1000)
      APUSetHalfCarry();
   IAPU.Registers.YA.W = (uint16_t) Work32;
   APUSetZN16(IAPU.Registers.YA.W);
   IAPU.PC += 2;
}

void Apu9A(void)
{
   DECLARE_VARIABLES();
   /* SUBW YA,dp */
   Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8);
   Int32 = (int32_t) IAPU.Registers.YA.W - (int32_t) Work16;
   APUClearHalfCarry();
   IAPU._Carry = Int32 >= 0;
   if (((IAPU.Registers.YA.W ^ Work16) & 0x8000) && ((IAPU.Registers.YA.W ^ (uint16_t) Int32) & 0x8000))
      APUSetOverflow();
   else
      APUClearOverflow();
   APUSetHalfCarry();
   if ((IAPU.Registers.YA.W ^ Work16 ^ (uint16_t) Int32) & 0x1000)
      APUClearHalfCarry();
   IAPU.Registers.YA.W = (uint16_t) Int32;
   APUSetZN16(IAPU.Registers.YA.W);
   IAPU.PC += 2;
}

void ApuBA(void)
{
   DECLARE_VARIABLES();
   /* MOVW YA,dp */
   IAPU.Registers.YA.B.A = S9xAPUGetByteZ(OP1);
   IAPU.Registers.YA.B.Y = S9xAPUGetByteZ(OP1 + 1);
   APUSetZN16(IAPU.Registers.YA.W);
   IAPU.PC += 2;
}

void ApuDA(void)
{
   DECLARE_VARIABLES();
   /* MOVW dp,YA */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.A, OP1);
   S9xAPUSetByteZ(IAPU.Registers.YA.B.Y, OP1 + 1);
   IAPU.PC += 2;
}

void Apu64(void)
{
   DECLARE_VARIABLES();
   /* CMP A,dp */
   Work8 = S9xAPUGetByteZ(OP1);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu65(void)
{
   DECLARE_VARIABLES();
   /* CMP A,abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void Apu66(void)
{
   DECLARE_VARIABLES();
   /* CMP A,(X) */
   Work8 = S9xAPUGetByteZ(IAPU.Registers.X);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC++;
}

void Apu67(void)
{
   DECLARE_VARIABLES();
   /* CMP A,(dp+X) */
   IndexedXIndirect();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu68(void)
{
   DECLARE_VARIABLES();
   /* CMP A,#00 */
   Work8 = OP1;
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu69(void)
{
   DECLARE_VARIABLES();
   /* CMP dp(dest), dp(src) */
   W1 = S9xAPUGetByteZ(OP1);
   Work8 = S9xAPUGetByteZ(OP2);
   CMP(Work8, W1);
   IAPU.PC += 3;
}

void Apu74(void)
{
   DECLARE_VARIABLES();
   /* CMP A, dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu75(void)
{
   DECLARE_VARIABLES();
   /* CMP A,abs+X */
   AbsoluteX();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void Apu76(void)
{
   DECLARE_VARIABLES();
   /* CMP A, abs+Y */
   AbsoluteY();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void Apu77(void)
{
   DECLARE_VARIABLES();
   /* CMP A,(dp)+Y */
   IndirectIndexedY();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu78(void)
{
   DECLARE_VARIABLES();
   /* CMP dp,#00 */
   Work8 = OP1;
   W1 = S9xAPUGetByteZ(OP2);
   CMP(W1, Work8);
   IAPU.PC += 3;
}

void Apu79(void)
{
   DECLARE_VARIABLES();
   /* CMP (X),(Y) */
   W1 = S9xAPUGetByteZ(IAPU.Registers.X);
   Work8 = S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
   CMP(W1, Work8);
   IAPU.PC++;
}

void Apu1E(void)
{
   DECLARE_VARIABLES();
   /* CMP X,abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.X, Work8);
   IAPU.PC += 3;
}

void Apu3E(void)
{
   DECLARE_VARIABLES();
   /* CMP X,dp */
   Work8 = S9xAPUGetByteZ(OP1);
   CMP(IAPU.Registers.X, Work8);
   IAPU.PC += 2;
}

void ApuC8(void)
{
   DECLARE_VARIABLES();
   /* CMP X,#00 */
   CMP(IAPU.Registers.X, OP1);
   IAPU.PC += 2;
}

void Apu5E(void)
{
   DECLARE_VARIABLES();
   /* CMP Y,abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.YA.B.Y, Work8);
   IAPU.PC += 3;
}

void Apu7E(void)
{
   DECLARE_VARIABLES();
   /* CMP Y,dp */
   Work8 = S9xAPUGetByteZ(OP1);
   CMP(IAPU.Registers.YA.B.Y, Work8);
   IAPU.PC += 2;
}

void ApuAD(void)
{
   DECLARE_VARIABLES();
   /* CMP Y,#00 */
   Work8 = OP1;
   CMP(IAPU.Registers.YA.B.Y, Work8);
   IAPU.PC += 2;
}

void Apu1F(void)
{
   DECLARE_VARIABLES();
   /* JMP (abs+X) */
   Absolute();
   IAPU.PC = IAPU.RAM + S9xAPUGetByte(IAPU.Address + IAPU.Registers.X) + (S9xAPUGetByte(IAPU.Address + IAPU.Registers.X + 1) << 8);
}

void Apu5F(void)
{
   DECLARE_VARIABLES();
   /* JMP abs */
   Absolute();
   IAPU.PC = IAPU.RAM + IAPU.Address;
}

void Apu20(void)
{
   DECLARE_VARIABLES();
   /* CLRP */
   APUClearDirectPage();
   IAPU.DirectPage = IAPU.RAM;
   IAPU.PC++;
}

void Apu60(void)
{
   DECLARE_VARIABLES();
   /* CLRC */
   APUClearCarry();
   IAPU.PC++;
}

void ApuE0(void)
{
   DECLARE_VARIABLES();
   /* CLRV */
   APUClearHalfCarry();
   APUClearOverflow();
   IAPU.PC++;
}

void Apu24(void)
{
   DECLARE_VARIABLES();
   /* AND A,dp */
   IAPU.Registers.YA.B.A &= S9xAPUGetByteZ(OP1);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu25(void)
{
   DECLARE_VARIABLES();
   /* AND A,abs */
   Absolute();
   IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu26(void)
{
   DECLARE_VARIABLES();
   /* AND A,(X) */
   IAPU.Registers.YA.B.A &= S9xAPUGetByteZ(IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu27(void)
{
   DECLARE_VARIABLES();
   /* AND A,(dp+X) */
   IndexedXIndirect();
   IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu28(void)
{
   DECLARE_VARIABLES();
   /* AND A,#00 */
   IAPU.Registers.YA.B.A &= OP1;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu29(void)
{
   DECLARE_VARIABLES();
   /* AND dp(dest),dp(src) */
   Work8 = S9xAPUGetByteZ(OP1);
   Work8 &= S9xAPUGetByteZ(OP2);
   S9xAPUSetByteZ(Work8, OP2);
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu34(void)
{
   DECLARE_VARIABLES();
   /* AND A,dp+X */
   IAPU.Registers.YA.B.A &= S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu35(void)
{
   DECLARE_VARIABLES();
   /* AND A,abs+X */
   AbsoluteX();
   IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu36(void)
{
   DECLARE_VARIABLES();
   /* AND A,abs+Y */
   AbsoluteY();
   IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu37(void)
{
   DECLARE_VARIABLES();
   /* AND A,(dp)+Y */
   IndirectIndexedY();
   IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu38(void)
{
   DECLARE_VARIABLES();
   /* AND dp,#00 */
   Work8 = OP1;
   Work8 &= S9xAPUGetByteZ(OP2);
   S9xAPUSetByteZ(Work8, OP2);
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu39(void)
{
   DECLARE_VARIABLES();
   /* AND (X),(Y) */
   Work8 = S9xAPUGetByteZ(IAPU.Registers.X) & S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
   APUSetZN8(Work8);
   S9xAPUSetByteZ(Work8, IAPU.Registers.X);
   IAPU.PC++;
}

void Apu2B(void)
{
   DECLARE_VARIABLES();
   /* ROL dp */
   Work8 = S9xAPUGetByteZ(OP1);
   ROL(Work8);
   S9xAPUSetByteZ(Work8, OP1);
   IAPU.PC += 2;
}

void Apu2C(void)
{
   DECLARE_VARIABLES();
   /* ROL abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ROL(Work8);
   S9xAPUSetByte(Work8, IAPU.Address);
   IAPU.PC += 3;
}

void Apu3B(void)
{
   DECLARE_VARIABLES();
   /* ROL dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   ROL(Work8);
   S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
   IAPU.PC += 2;
}

void Apu3C(void)
{
   DECLARE_VARIABLES();
   /* ROL A */
   ROL(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu2E(void)
{
   DECLARE_VARIABLES();
   /* CBNE dp,rel */
   Work8 = OP1;
   Relative2();

   if (S9xAPUGetByteZ(Work8) != IAPU.Registers.YA.B.A)
   {
      IAPU.PC = IAPU.RAM + (uint16_t) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 3;
}

void ApuDE(void)
{
   DECLARE_VARIABLES();
   /* CBNE dp+X,rel */
   Work8 = OP1 + IAPU.Registers.X;
   Relative2();

   if (S9xAPUGetByteZ(Work8) != IAPU.Registers.YA.B.A)
   {
      IAPU.PC = IAPU.RAM + (uint16_t) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 3;
}

void Apu3D(void)
{
   DECLARE_VARIABLES();
   /* INC X */
   IAPU.Registers.X++;
   APUSetZN8(IAPU.Registers.X);
   IAPU.WaitCounter++;
   IAPU.PC++;
}

void ApuFC(void)
{
   DECLARE_VARIABLES();
   /* INC Y */
   IAPU.Registers.YA.B.Y++;
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.WaitCounter++;
   IAPU.PC++;
}

void Apu1D(void)
{
   DECLARE_VARIABLES();
   /* DEC X */
   IAPU.Registers.X--;
   APUSetZN8(IAPU.Registers.X);
   IAPU.WaitCounter++;
   IAPU.PC++;
}

void ApuDC(void)
{
   DECLARE_VARIABLES();
   /* DEC Y */
   IAPU.Registers.YA.B.Y--;
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.WaitCounter++;
   IAPU.PC++;
}

void ApuAB(void)
{
   DECLARE_VARIABLES();
   /* INC dp */
   Work8 = S9xAPUGetByteZ(OP1) + 1;
   S9xAPUSetByteZ(Work8, OP1);
   APUSetZN8(Work8);
   IAPU.WaitCounter++;
   IAPU.PC += 2;
}

void ApuAC(void)
{
   DECLARE_VARIABLES();
   /* INC abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address) + 1;
   S9xAPUSetByte(Work8, IAPU.Address);
   APUSetZN8(Work8);
   IAPU.WaitCounter++;
   IAPU.PC += 3;
}

void ApuBB(void)
{
   DECLARE_VARIABLES();
   /* INC dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X) + 1;
   S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
   APUSetZN8(Work8);
   IAPU.WaitCounter++;
   IAPU.PC += 2;
}

void ApuBC(void)
{
   DECLARE_VARIABLES();
   /* INC A */
   IAPU.Registers.YA.B.A++;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.WaitCounter++;
   IAPU.PC++;
}

void Apu8B(void)
{
   DECLARE_VARIABLES();
   /* DEC dp */
   Work8 = S9xAPUGetByteZ(OP1) - 1;
   S9xAPUSetByteZ(Work8, OP1);
   APUSetZN8(Work8);
   IAPU.WaitCounter++;
   IAPU.PC += 2;
}

void Apu8C(void)
{
   DECLARE_VARIABLES();
   /* DEC abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address) - 1;
   S9xAPUSetByte(Work8, IAPU.Address);
   APUSetZN8(Work8);
   IAPU.WaitCounter++;
   IAPU.PC += 3;
}

void Apu9B(void)
{
   DECLARE_VARIABLES();
   /* DEC dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X) - 1;
   S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
   APUSetZN8(Work8);
   IAPU.WaitCounter++;
   IAPU.PC += 2;
}

void Apu9C(void)
{
   DECLARE_VARIABLES();
   /* DEC A */
   IAPU.Registers.YA.B.A--;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.WaitCounter++;
   IAPU.PC++;
}

void Apu44(void)
{
   DECLARE_VARIABLES();
   /* EOR A,dp */
   IAPU.Registers.YA.B.A ^= S9xAPUGetByteZ(OP1);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu45(void)
{
   DECLARE_VARIABLES();
   /* EOR A,abs */
   Absolute();
   IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu46(void)
{
   DECLARE_VARIABLES();
   /* EOR A,(X) */
   IAPU.Registers.YA.B.A ^= S9xAPUGetByteZ(IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu47(void)
{
   DECLARE_VARIABLES();
   /* EOR A,(dp+X) */
   IndexedXIndirect();
   IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu48(void)
{
   DECLARE_VARIABLES();
   /* EOR A,#00 */
   IAPU.Registers.YA.B.A ^= OP1;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu49(void)
{
   DECLARE_VARIABLES();
   /* EOR dp(dest),dp(src) */
   Work8 = S9xAPUGetByteZ(OP1);
   Work8 ^= S9xAPUGetByteZ(OP2);
   S9xAPUSetByteZ(Work8, OP2);
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu54(void)
{
   DECLARE_VARIABLES();
   /* EOR A,dp+X */
   IAPU.Registers.YA.B.A ^= S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu55(void)
{
   DECLARE_VARIABLES();
   /* EOR A,abs+X */
   AbsoluteX();
   IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu56(void)
{
   DECLARE_VARIABLES();
   /* EOR A,abs+Y */
   AbsoluteY();
   IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu57(void)
{
   DECLARE_VARIABLES();
   /* EOR A,(dp)+Y */
   IndirectIndexedY();
   IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu58(void)
{
   DECLARE_VARIABLES();
   /* EOR dp,#00 */
   Work8 = OP1;
   Work8 ^= S9xAPUGetByteZ(OP2);
   S9xAPUSetByteZ(Work8, OP2);
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu59(void)
{
   DECLARE_VARIABLES();
   /* EOR (X),(Y) */
   Work8 = S9xAPUGetByteZ(IAPU.Registers.X) ^ S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
   APUSetZN8(Work8);
   S9xAPUSetByteZ(Work8, IAPU.Registers.X);
   IAPU.PC++;
}

void Apu4B(void)
{
   DECLARE_VARIABLES();
   /* LSR dp */
   Work8 = S9xAPUGetByteZ(OP1);
   LSR(Work8);
   S9xAPUSetByteZ(Work8, OP1);
   IAPU.PC += 2;
}

void Apu4C(void)
{
   DECLARE_VARIABLES();
   /* LSR abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   LSR(Work8);
   S9xAPUSetByte(Work8, IAPU.Address);
   IAPU.PC += 3;
}

void Apu5B(void)
{
   DECLARE_VARIABLES();
   /* LSR dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   LSR(Work8);
   S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
   IAPU.PC += 2;
}

void Apu5C(void)
{
   DECLARE_VARIABLES();
   /* LSR A */
   LSR(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu7D(void)
{
   DECLARE_VARIABLES();
   /* MOV A,X */
   IAPU.Registers.YA.B.A = IAPU.Registers.X;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuDD(void)
{
   DECLARE_VARIABLES();
   /* MOV A,Y */
   IAPU.Registers.YA.B.A = IAPU.Registers.YA.B.Y;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu5D(void)
{
   DECLARE_VARIABLES();
   /* MOV X,A */
   IAPU.Registers.X = IAPU.Registers.YA.B.A;
   APUSetZN8(IAPU.Registers.X);
   IAPU.PC++;
}

void ApuFD(void)
{
   DECLARE_VARIABLES();
   /* MOV Y,A */
   IAPU.Registers.YA.B.Y = IAPU.Registers.YA.B.A;
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.PC++;
}

void Apu9D(void)
{
   DECLARE_VARIABLES();
   /* MOV X,SP */
   IAPU.Registers.X = IAPU.Registers.S;
   APUSetZN8(IAPU.Registers.X);
   IAPU.PC++;
}

void ApuBD(void)
{
   DECLARE_VARIABLES();
   /* MOV SP,X */
   IAPU.Registers.S = IAPU.Registers.X;
   IAPU.PC++;
}

void Apu6B(void)
{
   DECLARE_VARIABLES();
   /* ROR dp */
   Work8 = S9xAPUGetByteZ(OP1);
   ROR(Work8);
   S9xAPUSetByteZ(Work8, OP1);
   IAPU.PC += 2;
}

void Apu6C(void)
{
   DECLARE_VARIABLES();
   /* ROR abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ROR(Work8);
   S9xAPUSetByte(Work8, IAPU.Address);
   IAPU.PC += 3;
}

void Apu7B(void)
{
   DECLARE_VARIABLES();
   /* ROR dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   ROR(Work8);
   S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
   IAPU.PC += 2;
}

void Apu7C(void)
{
   DECLARE_VARIABLES();
   /* ROR A */
   ROR(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu6E(void)
{
   DECLARE_VARIABLES();
   /* DBNZ dp,rel */
   Work8 = OP1;
   Relative2();
   W1 = S9xAPUGetByteZ(Work8) - 1;
   S9xAPUSetByteZ(W1, Work8);
   if (W1 != 0)
   {
      IAPU.PC = IAPU.RAM + (uint16_t) Int16;
      APU.Cycles += IAPU.TwoCycles;
   }
   else
      IAPU.PC += 3;
}

void ApuFE(void)
{
   DECLARE_VARIABLES();
   /* DBNZ Y,rel */
   Relative();
   IAPU.Registers.YA.B.Y--;
   if (IAPU.Registers.YA.B.Y != 0)
   {
      IAPU.PC = IAPU.RAM + (uint16_t) Int16;
      APU.Cycles += IAPU.TwoCycles;
   }
   else
      IAPU.PC += 2;
}

void Apu6F(void)
{
   DECLARE_VARIABLES();
   /* RET */
   PopW(IAPU.Registers.PC);
   IAPU.PC = IAPU.RAM + IAPU.Registers.PC;
}

void Apu7F(void)
{
   DECLARE_VARIABLES();
   /* RETI */
   Pop(IAPU.Registers.P);
   S9xAPUUnpackStatus();
   PopW(IAPU.Registers.PC);
   IAPU.PC = IAPU.RAM + IAPU.Registers.PC;
}

void Apu84(void)
{
   DECLARE_VARIABLES();
   /* ADC A,dp */
   Work8 = S9xAPUGetByteZ(OP1);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu85(void)
{
   DECLARE_VARIABLES();
   /* ADC A, abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void Apu86(void)
{
   DECLARE_VARIABLES();
   /* ADC A,(X) */
   Work8 = S9xAPUGetByteZ(IAPU.Registers.X);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC++;
}

void Apu87(void)
{
   DECLARE_VARIABLES();
   /* ADC A,(dp+X) */
   IndexedXIndirect();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu88(void)
{
   DECLARE_VARIABLES();
   /* ADC A,#00 */
   Work8 = OP1;
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu89(void)
{
   DECLARE_VARIABLES();
   /* ADC dp(dest),dp(src) */
   Work8 = S9xAPUGetByteZ(OP1);
   W1 = S9xAPUGetByteZ(OP2);
   ADC(W1, Work8);
   S9xAPUSetByteZ(W1, OP2);
   IAPU.PC += 3;
}

void Apu94(void)
{
   DECLARE_VARIABLES();
   /* ADC A,dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu95(void)
{
   DECLARE_VARIABLES();
   /* ADC A, abs+X */
   AbsoluteX();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void Apu96(void)
{
   DECLARE_VARIABLES();
   /* ADC A, abs+Y */
   AbsoluteY();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void Apu97(void)
{
   DECLARE_VARIABLES();
   /* ADC A, (dp)+Y */
   IndirectIndexedY();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu98(void)
{
   DECLARE_VARIABLES();
   /* ADC dp,#00 */
   Work8 = OP1;
   W1 = S9xAPUGetByteZ(OP2);
   ADC(W1, Work8);
   S9xAPUSetByteZ(W1, OP2);
   IAPU.PC += 3;
}

void Apu99(void)
{
   DECLARE_VARIABLES();
   /* ADC (X),(Y) */
   W1 = S9xAPUGetByteZ(IAPU.Registers.X);
   Work8 = S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
   ADC(W1, Work8);
   S9xAPUSetByteZ(W1, IAPU.Registers.X);
   IAPU.PC++;
}

void Apu8D(void)
{
   DECLARE_VARIABLES();
   /* MOV Y,#00 */
   IAPU.Registers.YA.B.Y = OP1;
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.PC += 2;
}

void Apu8F(void)
{
   DECLARE_VARIABLES();
   /* MOV dp,#00 */
   Work8 = OP1;
   S9xAPUSetByteZ(Work8, OP2);
   IAPU.PC += 3;
}

void Apu9E(void)
{
   DECLARE_VARIABLES();
   uint32_t i;
   uint32_t yva;
   uint32_t x;

   /* DIV YA,X */
   if ((IAPU.Registers.X & 0x0f) <= (IAPU.Registers.YA.B.Y & 0x0f))
      APUSetHalfCarry();
   else
      APUClearHalfCarry();

   yva = IAPU.Registers.YA.W;
   x   = IAPU.Registers.X << 9;

   for (i = 0 ; i < 9 ; ++i)
   {
      yva <<= 1;
      if (yva & 0x20000)
         yva = (yva & 0x1ffff) | 1;
      if (yva >= x)
         yva ^= 1;
      if (yva & 1)
         yva = (yva - x) & 0x1ffff;
   }

   if (yva & 0x100)
       APUSetOverflow();
   else
       APUClearOverflow();

   IAPU.Registers.YA.B.Y = (yva >> 9) & 0xff;
   IAPU.Registers.YA.B.A = yva & 0xff;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu9F(void)
{
   DECLARE_VARIABLES();
   /* XCN A */
   IAPU.Registers.YA.B.A = (IAPU.Registers.YA.B.A >> 4) | (IAPU.Registers.YA.B.A << 4);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuA4(void)
{
   DECLARE_VARIABLES();
   /* SBC A, dp */
   Work8 = S9xAPUGetByteZ(OP1);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void ApuA5(void)
{
   DECLARE_VARIABLES();
   /* SBC A, abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void ApuA6(void)
{
   DECLARE_VARIABLES();
   /* SBC A, (X) */
   Work8 = S9xAPUGetByteZ(IAPU.Registers.X);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC++;
}

void ApuA7(void)
{
   DECLARE_VARIABLES();
   /* SBC A,(dp+X) */
   IndexedXIndirect();
   Work8 = S9xAPUGetByte(IAPU.Address);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void ApuA8(void)
{
   DECLARE_VARIABLES();
   /* SBC A,#00 */
   Work8 = OP1;
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void ApuA9(void)
{
   DECLARE_VARIABLES();
   /* SBC dp(dest), dp(src) */
   Work8 = S9xAPUGetByteZ(OP1);
   W1 = S9xAPUGetByteZ(OP2);
   SBC(W1, Work8);
   S9xAPUSetByteZ(W1, OP2);
   IAPU.PC += 3;
}

void ApuB4(void)
{
   DECLARE_VARIABLES();
   /* SBC A, dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void ApuB5(void)
{
   DECLARE_VARIABLES();
   /* SBC A,abs+X */
   AbsoluteX();
   Work8 = S9xAPUGetByte(IAPU.Address);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void ApuB6(void)
{
   DECLARE_VARIABLES();
   /* SBC A,abs+Y */
   AbsoluteY();
   Work8 = S9xAPUGetByte(IAPU.Address);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void ApuB7(void)
{
   DECLARE_VARIABLES();
   /* SBC A,(dp)+Y */
   IndirectIndexedY();
   Work8 = S9xAPUGetByte(IAPU.Address);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void ApuB8(void)
{
   DECLARE_VARIABLES();
   /* SBC dp,#00 */
   Work8 = OP1;
   W1 = S9xAPUGetByteZ(OP2);
   SBC(W1, Work8);
   S9xAPUSetByteZ(W1, OP2);
   IAPU.PC += 3;
}

void ApuB9(void)
{
   DECLARE_VARIABLES();
   /* SBC (X),(Y) */
   W1 = S9xAPUGetByteZ(IAPU.Registers.X);
   Work8 = S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
   SBC(W1, Work8);
   S9xAPUSetByteZ(W1, IAPU.Registers.X);
   IAPU.PC++;
}

void ApuAF(void)
{
   DECLARE_VARIABLES();
   /* MOV (X)+, A */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.A, IAPU.Registers.X++);
   IAPU.PC++;
}

void ApuBE(void)
{
   DECLARE_VARIABLES();
   /* DAS */
   if (IAPU.Registers.YA.B.A > 0x99 || !IAPU._Carry)
   {
      IAPU.Registers.YA.B.A -= 0x60;
      APUClearCarry();
   }
   else
      APUSetCarry();

   if ((IAPU.Registers.YA.B.A & 0x0f) > 9 || !APUCheckHalfCarry())
      IAPU.Registers.YA.B.A -= 6;

   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuBF(void)
{
   DECLARE_VARIABLES();
   /* MOV A,(X)+ */
   IAPU.Registers.YA.B.A = S9xAPUGetByteZ(IAPU.Registers.X++);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuC0(void)
{
   DECLARE_VARIABLES();
   /* DI */
   APUClearInterrupt();
   IAPU.PC++;
}

void ApuA0(void)
{
   DECLARE_VARIABLES();
   /* EI */
   APUSetInterrupt();
   IAPU.PC++;
}

void ApuC4(void)
{
   DECLARE_VARIABLES();
   /* MOV dp,A */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.A, OP1);
   IAPU.PC += 2;
}

void ApuC5(void)
{
   DECLARE_VARIABLES();
   /* MOV abs,A */
   Absolute();
   S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
   IAPU.PC += 3;
}

void ApuC6(void)
{
   DECLARE_VARIABLES();
   /* MOV (X), A */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.A, IAPU.Registers.X);
   IAPU.PC++;
}

void ApuC7(void)
{
   DECLARE_VARIABLES();
   /* MOV (dp+X),A */
   IndexedXIndirect();
   S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
   IAPU.PC += 2;
}

void ApuC9(void)
{
   DECLARE_VARIABLES();
   /* MOV abs,X */
   Absolute();
   S9xAPUSetByte(IAPU.Registers.X, IAPU.Address);
   IAPU.PC += 3;
}

void ApuCB(void)
{
   DECLARE_VARIABLES();
   /* MOV dp,Y */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.Y, OP1);
   IAPU.PC += 2;
}

void ApuCC(void)
{
   DECLARE_VARIABLES();
   /* MOV abs,Y */
   Absolute();
   S9xAPUSetByte(IAPU.Registers.YA.B.Y, IAPU.Address);
   IAPU.PC += 3;
}

void ApuCD(void)
{
   DECLARE_VARIABLES();
   /* MOV X,#00 */
   IAPU.Registers.X = OP1;
   APUSetZN8(IAPU.Registers.X);
   IAPU.PC += 2;
}

void ApuCF(void)
{
   DECLARE_VARIABLES();
   /* MUL YA */
   IAPU.Registers.YA.W = (uint16_t) IAPU.Registers.YA.B.A * IAPU.Registers.YA.B.Y;
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.PC++;
}

void ApuD4(void)
{
   DECLARE_VARIABLES();
   /* MOV dp+X, A */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.A, OP1 + IAPU.Registers.X);
   IAPU.PC += 2;
}

void ApuD5(void)
{
   DECLARE_VARIABLES();
   /* MOV abs+X,A */
   AbsoluteX();
   S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
   IAPU.PC += 3;
}

void ApuD6(void)
{
   DECLARE_VARIABLES();
   /* MOV abs+Y,A */
   AbsoluteY();
   S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
   IAPU.PC += 3;
}

void ApuD7(void)
{
   DECLARE_VARIABLES();
   /* MOV (dp)+Y,A */
   IndirectIndexedY();
   S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
   IAPU.PC += 2;
}

void ApuD8(void)
{
   DECLARE_VARIABLES();
   /* MOV dp,X */
   S9xAPUSetByteZ(IAPU.Registers.X, OP1);
   IAPU.PC += 2;
}

void ApuD9(void)
{
   DECLARE_VARIABLES();
   /* MOV dp+Y,X */
   S9xAPUSetByteZ(IAPU.Registers.X, OP1 + IAPU.Registers.YA.B.Y);
   IAPU.PC += 2;
}

void ApuDB(void)
{
   DECLARE_VARIABLES();
   /* MOV dp+X,Y */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.Y, OP1 + IAPU.Registers.X);
   IAPU.PC += 2;
}

void ApuDF(void)
{
   DECLARE_VARIABLES();
   /* DAA */
   if (IAPU.Registers.YA.B.A > 0x99 || IAPU._Carry)
   {
      IAPU.Registers.YA.B.A += 0x60;
      APUSetCarry();
   }
   else
      APUClearCarry();

   if ((IAPU.Registers.YA.B.A & 0x0f) > 9 || APUCheckHalfCarry())
      IAPU.Registers.YA.B.A += 6;

   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuE4(void)
{
   DECLARE_VARIABLES();
   /* MOV A, dp */
   IAPU.Registers.YA.B.A = S9xAPUGetByteZ(OP1);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void ApuE5(void)
{
   DECLARE_VARIABLES();
   /* MOV A,abs */
   Absolute();
   IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void ApuE6(void)
{
   DECLARE_VARIABLES();
   /* MOV A,(X) */
   IAPU.Registers.YA.B.A = S9xAPUGetByteZ(IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuE7(void)
{
   DECLARE_VARIABLES();
   /* MOV A,(dp+X) */
   IndexedXIndirect();
   IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void ApuE8(void)
{
   DECLARE_VARIABLES();
   /* MOV A,#00 */
   IAPU.Registers.YA.B.A = OP1;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void ApuE9(void)
{
   DECLARE_VARIABLES();
   /* MOV X, abs */
   Absolute();
   IAPU.Registers.X = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.X);
   IAPU.PC += 3;
}

void ApuEB(void)
{
   DECLARE_VARIABLES();
   /* MOV Y,dp */
   IAPU.Registers.YA.B.Y = S9xAPUGetByteZ(OP1);
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.PC += 2;
}

void ApuEC(void)
{
   DECLARE_VARIABLES();
   /* MOV Y,abs */
   Absolute();
   IAPU.Registers.YA.B.Y = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.PC += 3;
}

void ApuF4(void)
{
   DECLARE_VARIABLES();
   /* MOV A, dp+X */
   IAPU.Registers.YA.B.A = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void ApuF5(void)
{
   DECLARE_VARIABLES();
   /* MOV A, abs+X */
   AbsoluteX();
   IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void ApuF6(void)
{
   DECLARE_VARIABLES();
   /* MOV A, abs+Y */
   AbsoluteY();
   IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void ApuF7(void)
{
   DECLARE_VARIABLES();
   /* MOV A, (dp)+Y */
   IndirectIndexedY();
   IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void ApuF8(void)
{
   DECLARE_VARIABLES();
   /* MOV X,dp */
   IAPU.Registers.X = S9xAPUGetByteZ(OP1);
   APUSetZN8(IAPU.Registers.X);
   IAPU.PC += 2;
}

void ApuF9(void)
{
   DECLARE_VARIABLES();
   /* MOV X,dp+Y */
   IAPU.Registers.X = S9xAPUGetByteZ(OP1 + IAPU.Registers.YA.B.Y);
   APUSetZN8(IAPU.Registers.X);
   IAPU.PC += 2;
}

void ApuFA(void)
{
   DECLARE_VARIABLES();
   /* MOV dp(dest),dp(src) */
   S9xAPUSetByteZ(S9xAPUGetByteZ(OP1), OP2);
   IAPU.PC += 3;
}

void ApuFB(void)
{
   DECLARE_VARIABLES();
   /* MOV Y,dp+X */
   IAPU.Registers.YA.B.Y = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.PC += 2;
}

void (*const S9xApuOpcodes[256])(void) =
{
   Apu00, Apu01, Apu02, Apu03, Apu04, Apu05, Apu06, Apu07,
   Apu08, Apu09, Apu0A, Apu0B, Apu0C, Apu0D, Apu0E, Apu0F,
   Apu10, Apu11, Apu12, Apu13, Apu14, Apu15, Apu16, Apu17,
   Apu18, Apu19, Apu1A, Apu1B, Apu1C, Apu1D, Apu1E, Apu1F,
   Apu20, Apu21, Apu22, Apu23, Apu24, Apu25, Apu26, Apu27,
   Apu28, Apu29, Apu2A, Apu2B, Apu2C, Apu2D, Apu2E, Apu2F,
   Apu30, Apu31, Apu32, Apu33, Apu34, Apu35, Apu36, Apu37,
   Apu38, Apu39, Apu3A, Apu3B, Apu3C, Apu3D, Apu3E, Apu3F,
   Apu40, Apu41, Apu42, Apu43, Apu44, Apu45, Apu46, Apu47,
   Apu48, Apu49, Apu4A, Apu4B, Apu4C, Apu4D, Apu4E, Apu4F,
   Apu50, Apu51, Apu52, Apu53, Apu54, Apu55, Apu56, Apu57,
   Apu58, Apu59, Apu5A, Apu5B, Apu5C, Apu5D, Apu5E, Apu5F,
   Apu60, Apu61, Apu62, Apu63, Apu64, Apu65, Apu66, Apu67,
   Apu68, Apu69, Apu6A, Apu6B, Apu6C, Apu6D, Apu6E, Apu6F,
   Apu70, Apu71, Apu72, Apu73, Apu74, Apu75, Apu76, Apu77,
   Apu78, Apu79, Apu7A, Apu7B, Apu7C, Apu7D, Apu7E, Apu7F,
   Apu80, Apu81, Apu82, Apu83, Apu84, Apu85, Apu86, Apu87,
   Apu88, Apu89, Apu8A, Apu8B, Apu8C, Apu8D, Apu8E, Apu8F,
   Apu90, Apu91, Apu92, Apu93, Apu94, Apu95, Apu96, Apu97,
   Apu98, Apu99, Apu9A, Apu9B, Apu9C, Apu9D, Apu9E, Apu9F,
   ApuA0, ApuA1, ApuA2, ApuA3, ApuA4, ApuA5, ApuA6, ApuA7,
   ApuA8, ApuA9, ApuAA, ApuAB, ApuAC, ApuAD, ApuAE, ApuAF,
   ApuB0, ApuB1, ApuB2, ApuB3, ApuB4, ApuB5, ApuB6, ApuB7,
   ApuB8, ApuB9, ApuBA, ApuBB, ApuBC, ApuBD, ApuBE, ApuBF,
   ApuC0, ApuC1, ApuC2, ApuC3, ApuC4, ApuC5, ApuC6, ApuC7,
   ApuC8, ApuC9, ApuCA, ApuCB, ApuCC, ApuCD, ApuCE, ApuCF,
   ApuD0, ApuD1, ApuD2, ApuD3, ApuD4, ApuD5, ApuD6, ApuD7,
   ApuD8, ApuD9, ApuDA, ApuDB, ApuDC, ApuDD, ApuDE, ApuDF,
   ApuE0, ApuE1, ApuE2, ApuE3, ApuE4, ApuE5, ApuE6, ApuE7,
   ApuE8, ApuE9, ApuEA, ApuEB, ApuEC, ApuED, ApuEE, ApuEF,
   ApuF0, ApuF1, ApuF2, ApuF3, ApuF4, ApuF5, ApuF6, ApuF7,
   ApuF8, ApuF9, ApuFA, ApuFB, ApuFC, ApuFD, ApuFE, ApuFF
};
#endif
