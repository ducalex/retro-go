/* This file is part of Snes9x. See LICENSE file. */

#ifndef USE_BLARGG_APU
#pragma GCC optimize("O3")

#include "snes9x.h"
#include "spc700.h"
#include "memmap.h"
#include "display.h"
#include "cpuexec.h"
#include "apu.h"

static uint8_t S9xAPUGetByteZ(uint8_t Address)
{
   if (Address >= 0xf0 && IAPU.DirectPage == IAPU.RAM)
   {
      if (Address >= 0xf4 && Address <= 0xf7)
      {
         IAPU.WaitAddress2 = IAPU.WaitAddress1;
         IAPU.WaitAddress1 = IAPU.PC;
         return IAPU.RAM [Address];
      }
      if (Address >= 0xfd)
      {
         uint8_t t = IAPU.RAM [Address];
         IAPU.WaitAddress2 = IAPU.WaitAddress1;
         IAPU.WaitAddress1 = IAPU.PC;
         IAPU.RAM [Address] = 0;
         return t;
      }
      else if (Address == 0xf3)
         return S9xGetAPUDSP();

      return IAPU.RAM [Address];
   }
   return IAPU.DirectPage [Address];
}

static void S9xAPUSetByteZ(uint8_t byte, uint8_t Address)
{
   if (Address >= 0xf0 && IAPU.DirectPage == IAPU.RAM)
   {
      if (Address == 0xf3)
         S9xSetAPUDSP(byte);
      else if (Address >= 0xf4 && Address <= 0xf7)
         APU.OutPorts [Address - 0xf4] = byte;
      else if (Address == 0xf1)
         S9xSetAPUControl(byte);
      else if (Address < 0xfd)
      {
         IAPU.RAM [Address] = byte;
         if (Address >= 0xfa)
         {
            if (byte == 0)
               APU.TimerTarget [Address - 0xfa] = 0x100;
            else
               APU.TimerTarget [Address - 0xfa] = byte;
         }
      }
   }
   else
      IAPU.DirectPage [Address] = byte;
}

static uint8_t S9xAPUGetByte(uint32_t Address)
{
   bool zero;
   uint8_t t;

   Address &= 0xffff;

   if (Address == 0xf3)
      return S9xGetAPUDSP();

   zero = (Address >= 0xfd && Address <= 0xff);
   t    = IAPU.RAM [Address];

   if (zero || (Address >= 0xf4 && Address <= 0xf7))
   {
      IAPU.WaitAddress2 = IAPU.WaitAddress1;
      IAPU.WaitAddress1 = IAPU.PC;
   }

   if(zero)
      IAPU.RAM [Address] = 0;

   return t;
}

static void S9xAPUSetByte(uint8_t byte, uint32_t Address)
{
   Address &= 0xffff;

   if (Address <= 0xff && Address >= 0xf0)
   {
      if (Address == 0xf3)
         S9xSetAPUDSP(byte);
      else if (Address >= 0xf4 && Address <= 0xf7)
         APU.OutPorts [Address - 0xf4] = byte;
      else if (Address == 0xf1)
         S9xSetAPUControl(byte);
      else if (Address < 0xfd)
      {
         IAPU.RAM [Address] = byte;
         if (Address >= 0xfa)
         {
            if (byte == 0)
               APU.TimerTarget [Address - 0xfa] = 0x100;
            else
               APU.TimerTarget [Address - 0xfa] = byte;
         }
      }
   }
   else
   {
      if (Address < 0xffc0)
         IAPU.RAM [Address] = byte;
      else
      {
         APU.ExtraRAM [Address - 0xffc0] = byte;
         if (!APU.ShowROM)
            IAPU.RAM [Address] = byte;
      }
   }
}

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

#define SET(b) \
S9xAPUSetByteZ ((uint8_t) (S9xAPUGetByteZ (OP1 ) | (1 << (b))), OP1); \
IAPU.PC += 2

#define CLR(b) \
S9xAPUSetByteZ ((uint8_t) (S9xAPUGetByteZ (OP1) & ~(1 << (b))), OP1); \
IAPU.PC += 2;

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

void APUExecute(void/*int32_t target_cycles*/)
{
   int8_t   Int8;
   int16_t  Int16;
   int32_t  Int32;
   uint8_t  Work8, W1;
   uint16_t Work16;
   uint32_t Work32;

   // while (APU.Cycles <= target_cycles)
   // {
      uint8_t opcode = *IAPU.PC;

      APU.Cycles += S9xAPUCycles[opcode];

      switch (opcode)
      {
      case 0x00: /* NOP */
         IAPU.PC++;
         break;
      case 0x01:
         TCALL(0);
         break;
      case 0x02:
         SET(0);
         break;
      case 0x03:
         BBS(0);
         break;
      case 0x04: /* OR A,dp */
         IAPU.Registers.YA.B.A |= S9xAPUGetByteZ(OP1);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x05: /* OR A,abs */
         Absolute();
         IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 3;
         break;
      case 0x06: /* OR A,(X) */
         IAPU.Registers.YA.B.A |= S9xAPUGetByteZ(IAPU.Registers.X);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;
      case 0x07: /* OR A,(dp+X) */
         IndexedXIndirect();
         IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x08: /* OR A,#00 */
         IAPU.Registers.YA.B.A |= OP1;
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x09: /* OR dp(dest),dp(src) */
         Work8 = S9xAPUGetByteZ(OP1);
         Work8 |= S9xAPUGetByteZ(OP2);
         S9xAPUSetByteZ(Work8, OP2);
         APUSetZN8(Work8);
         IAPU.PC += 3;
         break;
      case 0x0A: /* OR1 C,membit */
         MemBit();
         if (!APUCheckCarry())
            if (S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit))
               APUSetCarry();
         IAPU.PC += 3;
         break;
      case 0x0B: /* ASL dp */
         Work8 = S9xAPUGetByteZ(OP1);
         ASL(Work8);
         S9xAPUSetByteZ(Work8, OP1);
         IAPU.PC += 2;
         break;
      case 0x0C: /* ASL abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address);
         ASL(Work8);
         S9xAPUSetByte(Work8, IAPU.Address);
         IAPU.PC += 3;
         break;
      case 0x0D: /* PUSH PSW */
         S9xAPUPackStatus();
         Push(IAPU.Registers.P);
         IAPU.PC++;
         break;
      case 0x0E: /* TSET1 abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address);
         S9xAPUSetByte(Work8 | IAPU.Registers.YA.B.A, IAPU.Address);
         Work8 = IAPU.Registers.YA.B.A - Work8;
         APUSetZN8(Work8);
         IAPU.PC += 3;
         break;
      case 0x0F: /* BRK */
         PushW(IAPU.PC + 1 - IAPU.RAM);
         S9xAPUPackStatus();
         Push(IAPU.Registers.P);
         APUSetBreak();
         APUClearInterrupt();
         IAPU.PC = IAPU.RAM + S9xAPUGetByte(0xffde) + (S9xAPUGetByte(0xffdf) << 8);
         break;

      case 0x10: /* BPL */
         Relative();
         if (!APUCheckNegative())
         {
            IAPU.PC = IAPU.RAM + (uint16_t) Int16;
            APU.Cycles += IAPU.TwoCycles;
            APUShutdown();
         }
         else
            IAPU.PC += 2;
         break;
      case 0x11:
         TCALL(1);
         break;
      case 0x12:
         CLR(0);
         break;
      case 0x13:
         BBC(0);
         break;
      case 0x14: /* OR A,dp+X */
         IAPU.Registers.YA.B.A |= S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x15: /* OR A,abs+X */
         AbsoluteX();
         IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 3;
         break;
      case 0x16: /* OR A,abs+Y */
         AbsoluteY();
         IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 3;
         break;
      case 0x17: /* OR A,(dp)+Y */
         IndirectIndexedY();
         IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x18: /* OR dp,#00 */
         Work8 = OP1;
         Work8 |= S9xAPUGetByteZ(OP2);
         S9xAPUSetByteZ(Work8, OP2);
         APUSetZN8(Work8);
         IAPU.PC += 3;
         break;
      case 0x19: /* OR (X),(Y) */
         Work8 = S9xAPUGetByteZ(IAPU.Registers.X) | S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
         APUSetZN8(Work8);
         S9xAPUSetByteZ(Work8, IAPU.Registers.X);
         IAPU.PC++;
         break;
      case 0x1A: /* DECW dp */
         Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8) - 1;
         S9xAPUSetByteZ((uint8_t) Work16, OP1);
         S9xAPUSetByteZ(Work16 >> 8, OP1 + 1);
         APUSetZN16(Work16);
         IAPU.PC += 2;
         break;
      case 0x1B: /* ASL dp+X */
         Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
         ASL(Work8);
         S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
         IAPU.PC += 2;
         break;
      case 0x1C: /* ASL A */
         ASL(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;
      case 0x1D: /* DEC X */
         IAPU.Registers.X--;
         APUSetZN8(IAPU.Registers.X);
         IAPU.WaitCounter++;
         IAPU.PC++;
         break;
      case 0x1E: /* CMP X,abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address);
         CMP(IAPU.Registers.X, Work8);
         IAPU.PC += 3;
         break;
      case 0x1F: /* JMP (abs+X) */
         Absolute();
         IAPU.PC = IAPU.RAM + S9xAPUGetByte(IAPU.Address + IAPU.Registers.X) + (S9xAPUGetByte(IAPU.Address + IAPU.Registers.X + 1) << 8);
         break;

      case 0x20: /* CLRP */
         APUClearDirectPage();
         IAPU.DirectPage = IAPU.RAM;
         IAPU.PC++;
         break;
      case 0x21:
         TCALL(2);
         break;
      case 0x22:
         SET(1);
         break;
      case 0x23:
         BBS(1);
         break;
      case 0x24: /* AND A,dp */
         IAPU.Registers.YA.B.A &= S9xAPUGetByteZ(OP1);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x25: /* AND A,abs */
         Absolute();
         IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 3;
         break;
      case 0x26: /* AND A,(X) */
         IAPU.Registers.YA.B.A &= S9xAPUGetByteZ(IAPU.Registers.X);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;
      case 0x27: /* AND A,(dp+X) */
         IndexedXIndirect();
         IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x28: /* AND A,#00 */
         IAPU.Registers.YA.B.A &= OP1;
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x29: /* AND dp(dest),dp(src) */
         Work8 = S9xAPUGetByteZ(OP1);
         Work8 &= S9xAPUGetByteZ(OP2);
         S9xAPUSetByteZ(Work8, OP2);
         APUSetZN8(Work8);
         IAPU.PC += 3;
         break;
      case 0x2A: /* OR1 C,not membit */
         MemBit();
         if (!APUCheckCarry())
            if (!(S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit)))
               APUSetCarry();
         IAPU.PC += 3;
         break;
      case 0x2B: /* ROL dp */
         Work8 = S9xAPUGetByteZ(OP1);
         ROL(Work8);
         S9xAPUSetByteZ(Work8, OP1);
         IAPU.PC += 2;
         break;
      case 0x2C: /* ROL abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address);
         ROL(Work8);
         S9xAPUSetByte(Work8, IAPU.Address);
         IAPU.PC += 3;
         break;
      case 0x2D: /* PUSH A */
         Push(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;
      case 0x2E: /* CBNE dp,rel */
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
         break;
      case 0x2F: /* BRA */
         Relative();
         IAPU.PC = IAPU.RAM + (uint16_t) Int16;
         break;

      case 0x30: /* BMI */
         Relative();
         if (APUCheckNegative())
         {
            IAPU.PC = IAPU.RAM + (uint16_t) Int16;
            APU.Cycles += IAPU.TwoCycles;
            APUShutdown();
         }
         else
            IAPU.PC += 2;
         break;
      case 0x31:
         TCALL(3);
         break;
      case 0x32:
         CLR(1);
         break;
      case 0x33:
         BBC(1);
         break;
      case 0x34: /* AND A,dp+X */
         IAPU.Registers.YA.B.A &= S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x35: /* AND A,abs+X */
         AbsoluteX();
         IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 3;
         break;
      case 0x36: /* AND A,abs+Y */
         AbsoluteY();
         IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 3;
         break;
      case 0x37: /* AND A,(dp)+Y */
         IndirectIndexedY();
         IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x38: /* AND dp,#00 */
         Work8 = OP1;
         Work8 &= S9xAPUGetByteZ(OP2);
         S9xAPUSetByteZ(Work8, OP2);
         APUSetZN8(Work8);
         IAPU.PC += 3;
         break;
      case 0x39: /* AND (X),(Y) */
         Work8 = S9xAPUGetByteZ(IAPU.Registers.X) & S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
         APUSetZN8(Work8);
         S9xAPUSetByteZ(Work8, IAPU.Registers.X);
         IAPU.PC++;
         break;
      case 0x3A: /* INCW dp */
         Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8) + 1;
         S9xAPUSetByteZ((uint8_t) Work16, OP1);
         S9xAPUSetByteZ(Work16 >> 8, OP1 + 1);
         APUSetZN16(Work16);
         IAPU.PC += 2;
         break;
      case 0x3B: /* ROL dp+X */
         Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
         ROL(Work8);
         S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
         IAPU.PC += 2;
         break;
      case 0x3C: /* ROL A */
         ROL(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;
      case 0x3D: /* INC X */
         IAPU.Registers.X++;
         APUSetZN8(IAPU.Registers.X);
         IAPU.WaitCounter++;
         IAPU.PC++;
         break;
      case 0x3E: /* CMP X,dp */
         Work8 = S9xAPUGetByteZ(OP1);
         CMP(IAPU.Registers.X, Work8);
         IAPU.PC += 2;
         break;
      case 0x3F:  /* CALL absolute */
         Absolute();
         /* 0xB6f for Star Fox 2 */
         PushW(IAPU.PC + 3 - IAPU.RAM);
         IAPU.PC = IAPU.RAM + IAPU.Address;
         break;

      case 0x40: /* SETP */
         APUSetDirectPage();
         IAPU.DirectPage = IAPU.RAM + 0x100;
         IAPU.PC++;
         break;
      case 0x41:
         TCALL(4);
         break;
      case 0x42:
         SET(2);
         break;
      case 0x43:
         BBS(2);
         break;
      case 0x44: /* EOR A,dp */
         IAPU.Registers.YA.B.A ^= S9xAPUGetByteZ(OP1);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x45: /* EOR A,abs */
         Absolute();
         IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 3;
         break;
      case 0x46: /* EOR A,(X) */
         IAPU.Registers.YA.B.A ^= S9xAPUGetByteZ(IAPU.Registers.X);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;
      case 0x47: /* EOR A,(dp+X) */
         IndexedXIndirect();
         IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x48: /* EOR A,#00 */
         IAPU.Registers.YA.B.A ^= OP1;
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x49: /* EOR dp(dest),dp(src) */
         Work8 = S9xAPUGetByteZ(OP1);
         Work8 ^= S9xAPUGetByteZ(OP2);
         S9xAPUSetByteZ(Work8, OP2);
         APUSetZN8(Work8);
         IAPU.PC += 3;
         break;
      case 0x4A: /* AND1 C,membit */
         MemBit();
         if (APUCheckCarry())
            if (!(S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit)))
               APUClearCarry();
         IAPU.PC += 3;
         break;
      case 0x4B: /* LSR dp */
         Work8 = S9xAPUGetByteZ(OP1);
         LSR(Work8);
         S9xAPUSetByteZ(Work8, OP1);
         IAPU.PC += 2;
         break;
      case 0x4C: /* LSR abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address);
         LSR(Work8);
         S9xAPUSetByte(Work8, IAPU.Address);
         IAPU.PC += 3;
         break;
      case 0x4D: /* PUSH X */
         Push(IAPU.Registers.X);
         IAPU.PC++;
         break;
      case 0x4E: /* TCLR1 abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address);
         S9xAPUSetByte(Work8 & ~IAPU.Registers.YA.B.A, IAPU.Address);
         Work8 = IAPU.Registers.YA.B.A - Work8;
         APUSetZN8(Work8);
         IAPU.PC += 3;
         break;
      case 0x4F: /* PCALL $XX */
         Work8 = OP1;
         PushW(IAPU.PC + 2 - IAPU.RAM);
         IAPU.PC = IAPU.RAM + 0xff00 + Work8;
         break;

      case 0x50: /* BVC */
         Relative();
         if (!APUCheckOverflow())
         {
            IAPU.PC = IAPU.RAM + (uint16_t) Int16;
            APU.Cycles += IAPU.TwoCycles;
         }
         else
            IAPU.PC += 2;
         break;
      case 0x51:
         TCALL(5);
         break;
      case 0x52:
         CLR(2);
         break;
      case 0x53:
         BBC(2);
         break;
      case 0x54: /* EOR A,dp+X */
         IAPU.Registers.YA.B.A ^= S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x55: /* EOR A,abs+X */
         AbsoluteX();
         IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 3;
         break;
      case 0x56: /* EOR A,abs+Y */
         AbsoluteY();
         IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 3;
         break;
      case 0x57: /* EOR A,(dp)+Y */
         IndirectIndexedY();
         IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0x58: /* EOR dp,#00 */
         Work8 = OP1;
         Work8 ^= S9xAPUGetByteZ(OP2);
         S9xAPUSetByteZ(Work8, OP2);
         APUSetZN8(Work8);
         IAPU.PC += 3;
         break;
      case 0x59: /* EOR (X),(Y) */
         Work8 = S9xAPUGetByteZ(IAPU.Registers.X) ^ S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
         APUSetZN8(Work8);
         S9xAPUSetByteZ(Work8, IAPU.Registers.X);
         IAPU.PC++;
         break;
      case 0x5A: /* CMPW YA,dp */
         Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8);
         Int32 = (int32_t) IAPU.Registers.YA.W - (int32_t) Work16;
         IAPU._Carry = Int32 >= 0;
         APUSetZN16((uint16_t) Int32);
         IAPU.PC += 2;
         break;
      case 0x5B: /* LSR dp+X */
         Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
         LSR(Work8);
         S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
         IAPU.PC += 2;
         break;
      case 0x5C: /* LSR A */
         LSR(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;
      case 0x5D: /* MOV X,A */
         IAPU.Registers.X = IAPU.Registers.YA.B.A;
         APUSetZN8(IAPU.Registers.X);
         IAPU.PC++;
         break;
      case 0x5E: /* CMP Y,abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address);
         CMP(IAPU.Registers.YA.B.Y, Work8);
         IAPU.PC += 3;
         break;
      case 0x5F: /* JMP abs */
         Absolute();
         IAPU.PC = IAPU.RAM + IAPU.Address;
         break;

      case 0x60: /* CLRC */
         APUClearCarry();
         IAPU.PC++;
         break;
      case 0x61:
         TCALL(6);
         break;
      case 0x62:
         SET(3);
         break;
      case 0x63:
         BBS(3);
         break;
      case 0x64: /* CMP A,dp */
         Work8 = S9xAPUGetByteZ(OP1);
         CMP(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0x65: /* CMP A,abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address);
         CMP(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 3;
         break;
      case 0x66: /* CMP A,(X) */
         Work8 = S9xAPUGetByteZ(IAPU.Registers.X);
         CMP(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC++;
         break;
      case 0x67: /* CMP A,(dp+X) */
         IndexedXIndirect();
         Work8 = S9xAPUGetByte(IAPU.Address);
         CMP(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0x68: /* CMP A,#00 */
         Work8 = OP1;
         CMP(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0x69: /* CMP dp(dest), dp(src) */
         W1 = S9xAPUGetByteZ(OP1);
         Work8 = S9xAPUGetByteZ(OP2);
         CMP(Work8, W1);
         IAPU.PC += 3;
         break;
      case 0x6A: /* AND1 C, not membit */
         MemBit();
         if (APUCheckCarry())
            if ((S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit)))
               APUClearCarry();
         IAPU.PC += 3;
         break;
      case 0x6B: /* ROR dp */
         Work8 = S9xAPUGetByteZ(OP1);
         ROR(Work8);
         S9xAPUSetByteZ(Work8, OP1);
         IAPU.PC += 2;
         break;
      case 0x6C: /* ROR abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address);
         ROR(Work8);
         S9xAPUSetByte(Work8, IAPU.Address);
         IAPU.PC += 3;
         break;
      case 0x6D: /* PUSH Y */
         Push(IAPU.Registers.YA.B.Y);
         IAPU.PC++;
         break;
      case 0x6E: /* DBNZ dp,rel */
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
         break;
      case 0x6F: /* RET */
         PopW(IAPU.Registers.PC);
         IAPU.PC = IAPU.RAM + IAPU.Registers.PC;
         break;

      case 0x70: /* BVS */
         Relative();
         if (APUCheckOverflow())
         {
            IAPU.PC = IAPU.RAM + (uint16_t) Int16;
            APU.Cycles += IAPU.TwoCycles;
         }
         else
            IAPU.PC += 2;
         break;
      case 0x71:
         TCALL(7);
         break;
      case 0x72:
         CLR(3);
         break;
      case 0x73:
         BBC(3);
         break;
      case 0x74: /* CMP A, dp+X */
         Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
         CMP(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0x75: /* CMP A,abs+X */
         AbsoluteX();
         Work8 = S9xAPUGetByte(IAPU.Address);
         CMP(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 3;
         break;
      case 0x76: /* CMP A, abs+Y */
         AbsoluteY();
         Work8 = S9xAPUGetByte(IAPU.Address);
         CMP(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 3;
         break;
      case 0x77: /* CMP A,(dp)+Y */
         IndirectIndexedY();
         Work8 = S9xAPUGetByte(IAPU.Address);
         CMP(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0x78: /* CMP dp,#00 */
         Work8 = OP1;
         W1 = S9xAPUGetByteZ(OP2);
         CMP(W1, Work8);
         IAPU.PC += 3;
         break;
      case 0x79: /* CMP (X),(Y) */
         W1 = S9xAPUGetByteZ(IAPU.Registers.X);
         Work8 = S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
         CMP(W1, Work8);
         IAPU.PC++;
         break;
      case 0x7A: /* ADDW YA,dp */
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
         break;
      case 0x7B: /* ROR dp+X */
         Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
         ROR(Work8);
         S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
         IAPU.PC += 2;
         break;
      case 0x7C: /* ROR A */
         ROR(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;
      case 0x7D: /* MOV A,X */
         IAPU.Registers.YA.B.A = IAPU.Registers.X;
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;
      case 0x7E: /* CMP Y,dp */
         Work8 = S9xAPUGetByteZ(OP1);
         CMP(IAPU.Registers.YA.B.Y, Work8);
         IAPU.PC += 2;
         break;
      case 0x7F: /* RETI */
         Pop(IAPU.Registers.P);
         S9xAPUUnpackStatus();
         PopW(IAPU.Registers.PC);
         IAPU.PC = IAPU.RAM + IAPU.Registers.PC;
         break;

      case 0x80: /* SETC */
         APUSetCarry();
         IAPU.PC++;
         break;
      case 0x81:
         TCALL(8);
         break;
      case 0x82:
         SET(4);
         break;
      case 0x83:
         BBS(4);
         break;
      case 0x84: /* ADC A,dp */
         Work8 = S9xAPUGetByteZ(OP1);
         ADC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0x85: /* ADC A, abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address);
         ADC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 3;
         break;
      case 0x86: /* ADC A,(X) */
         Work8 = S9xAPUGetByteZ(IAPU.Registers.X);
         ADC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC++;
         break;
      case 0x87: /* ADC A,(dp+X) */
         IndexedXIndirect();
         Work8 = S9xAPUGetByte(IAPU.Address);
         ADC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0x88: /* ADC A,#00 */
         Work8 = OP1;
         ADC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0x89: /* ADC dp(dest),dp(src) */
         Work8 = S9xAPUGetByteZ(OP1);
         W1 = S9xAPUGetByteZ(OP2);
         ADC(W1, Work8);
         S9xAPUSetByteZ(W1, OP2);
         IAPU.PC += 3;
         break;
      case 0x8A: /* EOR1 C, membit */
         MemBit();
         if (S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit))
         {
            if (APUCheckCarry())
               APUClearCarry();
            else
               APUSetCarry();
         }
         IAPU.PC += 3;
         break;
      case 0x8B: /* DEC dp */
         Work8 = S9xAPUGetByteZ(OP1) - 1;
         S9xAPUSetByteZ(Work8, OP1);
         APUSetZN8(Work8);
         IAPU.WaitCounter++;
         IAPU.PC += 2;
         break;
      case 0x8C: /* DEC abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address) - 1;
         S9xAPUSetByte(Work8, IAPU.Address);
         APUSetZN8(Work8);
         IAPU.WaitCounter++;
         IAPU.PC += 3;
         break;
      case 0x8D: /* MOV Y,#00 */
         IAPU.Registers.YA.B.Y = OP1;
         APUSetZN8(IAPU.Registers.YA.B.Y);
         IAPU.PC += 2;
         break;
      case 0x8E: /* POP PSW */
         Pop(IAPU.Registers.P);
         S9xAPUUnpackStatus();
         if (APUCheckDirectPage())
            IAPU.DirectPage = IAPU.RAM + 0x100;
         else
            IAPU.DirectPage = IAPU.RAM;
         IAPU.PC++;
         break;
      case 0x8F: /* MOV dp,#00 */
         Work8 = OP1;
         S9xAPUSetByteZ(Work8, OP2);
         IAPU.PC += 3;
         break;

      case 0x90: /* BCC */
         Relative();
         if (!APUCheckCarry())
         {
            IAPU.PC = IAPU.RAM + (uint16_t) Int16;
            APU.Cycles += IAPU.TwoCycles;
            APUShutdown();
         }
         else
            IAPU.PC += 2;
         break;
      case 0x91:
         TCALL(9);
         break;
      case 0x92:
         CLR(4);
         break;
      case 0x93:
         BBC(4);
         break;
      case 0x94: /* ADC A,dp+X */
         Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
         ADC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0x95: /* ADC A, abs+X */
         AbsoluteX();
         Work8 = S9xAPUGetByte(IAPU.Address);
         ADC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 3;
         break;
      case 0x96: /* ADC A, abs+Y */
         AbsoluteY();
         Work8 = S9xAPUGetByte(IAPU.Address);
         ADC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 3;
         break;
      case 0x97: /* ADC A, (dp)+Y */
         IndirectIndexedY();
         Work8 = S9xAPUGetByte(IAPU.Address);
         ADC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0x98: /* ADC dp,#00 */
         Work8 = OP1;
         W1 = S9xAPUGetByteZ(OP2);
         ADC(W1, Work8);
         S9xAPUSetByteZ(W1, OP2);
         IAPU.PC += 3;
         break;
      case 0x99: /* ADC (X),(Y) */
         W1 = S9xAPUGetByteZ(IAPU.Registers.X);
         Work8 = S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
         ADC(W1, Work8);
         S9xAPUSetByteZ(W1, IAPU.Registers.X);
         IAPU.PC++;
         break;
      case 0x9A: /* SUBW YA,dp */
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
         break;
      case 0x9B: /* DEC dp+X */
         Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X) - 1;
         S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
         APUSetZN8(Work8);
         IAPU.WaitCounter++;
         IAPU.PC += 2;
         break;
      case 0x9C: /* DEC A */
         IAPU.Registers.YA.B.A--;
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.WaitCounter++;
         IAPU.PC++;
         break;
      case 0x9D: /* MOV X,SP */
         IAPU.Registers.X = IAPU.Registers.S;
         APUSetZN8(IAPU.Registers.X);
         IAPU.PC++;
         break;
      case 0x9E: {
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
         break;
      case 0x9F: /* XCN A */
         IAPU.Registers.YA.B.A = (IAPU.Registers.YA.B.A >> 4) | (IAPU.Registers.YA.B.A << 4);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;

      case 0xA0: /* EI */
         APUSetInterrupt();
         IAPU.PC++;
         break;
      case 0xA1:
         TCALL(10);
         break;
      case 0xA2:
         SET(5);
         break;
      case 0xA3:
         BBS(5);
         break;
      case 0xA4: /* SBC A, dp */
         Work8 = S9xAPUGetByteZ(OP1);
         SBC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0xA5: /* SBC A, abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address);
         SBC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 3;
         break;
      case 0xA6: /* SBC A, (X) */
         Work8 = S9xAPUGetByteZ(IAPU.Registers.X);
         SBC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC++;
         break;
      case 0xA7: /* SBC A,(dp+X) */
         IndexedXIndirect();
         Work8 = S9xAPUGetByte(IAPU.Address);
         SBC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0xA8: /* SBC A,#00 */
         Work8 = OP1;
         SBC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0xA9: /* SBC dp(dest), dp(src) */
         Work8 = S9xAPUGetByteZ(OP1);
         W1 = S9xAPUGetByteZ(OP2);
         SBC(W1, Work8);
         S9xAPUSetByteZ(W1, OP2);
         IAPU.PC += 3;
         break;
      case 0xAA: /* MOV1 C,membit */
         MemBit();
         if (S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit))
            APUSetCarry();
         else
            APUClearCarry();
         IAPU.PC += 3;
         break;
      case 0xAB: /* INC dp */
         Work8 = S9xAPUGetByteZ(OP1) + 1;
         S9xAPUSetByteZ(Work8, OP1);
         APUSetZN8(Work8);
         IAPU.WaitCounter++;
         IAPU.PC += 2;
         break;
      case 0xAC: /* INC abs */
         Absolute();
         Work8 = S9xAPUGetByte(IAPU.Address) + 1;
         S9xAPUSetByte(Work8, IAPU.Address);
         APUSetZN8(Work8);
         IAPU.WaitCounter++;
         IAPU.PC += 3;
         break;
      case 0xAD: /* CMP Y,#00 */
         Work8 = OP1;
         CMP(IAPU.Registers.YA.B.Y, Work8);
         IAPU.PC += 2;
         break;
      case 0xAE: /* POP A */
         Pop(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;
      case 0xAF: /* MOV (X)+, A */
         S9xAPUSetByteZ(IAPU.Registers.YA.B.A, IAPU.Registers.X++);
         IAPU.PC++;
         break;

      case 0xB0: /* BCS */
         Relative();
         if (APUCheckCarry())
         {
            IAPU.PC = IAPU.RAM + (uint16_t) Int16;
            APU.Cycles += IAPU.TwoCycles;
            APUShutdown();
         }
         else
            IAPU.PC += 2;
         break;
      case 0xB1:
         TCALL(11);
         break;
      case 0xB2:
         CLR(5);
         break;
      case 0xB3:
         BBC(5);
         break;
      case 0xB4: /* SBC A, dp+X */
         Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
         SBC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0xB5: /* SBC A,abs+X */
         AbsoluteX();
         Work8 = S9xAPUGetByte(IAPU.Address);
         SBC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 3;
         break;
      case 0xB6: /* SBC A,abs+Y */
         AbsoluteY();
         Work8 = S9xAPUGetByte(IAPU.Address);
         SBC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 3;
         break;
      case 0xB7: /* SBC A,(dp)+Y */
         IndirectIndexedY();
         Work8 = S9xAPUGetByte(IAPU.Address);
         SBC(IAPU.Registers.YA.B.A, Work8);
         IAPU.PC += 2;
         break;
      case 0xB8: /* SBC dp,#00 */
         Work8 = OP1;
         W1 = S9xAPUGetByteZ(OP2);
         SBC(W1, Work8);
         S9xAPUSetByteZ(W1, OP2);
         IAPU.PC += 3;
         break;
      case 0xB9: /* SBC (X),(Y) */
         W1 = S9xAPUGetByteZ(IAPU.Registers.X);
         Work8 = S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
         SBC(W1, Work8);
         S9xAPUSetByteZ(W1, IAPU.Registers.X);
         IAPU.PC++;
         break;
      case 0xBA: /* MOVW YA,dp */
         IAPU.Registers.YA.B.A = S9xAPUGetByteZ(OP1);
         IAPU.Registers.YA.B.Y = S9xAPUGetByteZ(OP1 + 1);
         APUSetZN16(IAPU.Registers.YA.W);
         IAPU.PC += 2;
         break;
      case 0xBB: /* INC dp+X */
         Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X) + 1;
         S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
         APUSetZN8(Work8);
         IAPU.WaitCounter++;
         IAPU.PC += 2;
         break;
      case 0xBC: /* INC A */
         IAPU.Registers.YA.B.A++;
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.WaitCounter++;
         IAPU.PC++;
         break;
      case 0xBD: /* MOV SP,X */
         IAPU.Registers.S = IAPU.Registers.X;
         IAPU.PC++;
         break;
      case 0xBE: /* DAS */
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
         break;
      case 0xBF: /* MOV A,(X)+ */
         IAPU.Registers.YA.B.A = S9xAPUGetByteZ(IAPU.Registers.X++);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;

      case 0xC0: /* DI */
         APUClearInterrupt();
         IAPU.PC++;
         break;
      case 0xC1:
         TCALL(12);
         break;
      case 0xC2:
         SET(6);
         break;
      case 0xC3:
         BBS(6);
         break;
      case 0xC4: /* MOV dp,A */
         S9xAPUSetByteZ(IAPU.Registers.YA.B.A, OP1);
         IAPU.PC += 2;
         break;
      case 0xC5: /* MOV abs,A */
         Absolute();
         S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
         IAPU.PC += 3;
         break;
      case 0xC6: /* MOV (X), A */
         S9xAPUSetByteZ(IAPU.Registers.YA.B.A, IAPU.Registers.X);
         IAPU.PC++;
         break;
      case 0xC7: /* MOV (dp+X),A */
         IndexedXIndirect();
         S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
         IAPU.PC += 2;
         break;
      case 0xC8: /* CMP X,#00 */
         CMP(IAPU.Registers.X, OP1);
         IAPU.PC += 2;
         break;
      case 0xC9: /* MOV abs,X */
         Absolute();
         S9xAPUSetByte(IAPU.Registers.X, IAPU.Address);
         IAPU.PC += 3;
         break;
      case 0xCA: /* MOV1 membit,C */
         MemBit();
         if (APUCheckCarry())
            S9xAPUSetByte(S9xAPUGetByte(IAPU.Address) | (1 << IAPU.Bit), IAPU.Address);
         else
            S9xAPUSetByte(S9xAPUGetByte(IAPU.Address) & ~(1 << IAPU.Bit), IAPU.Address);
         IAPU.PC += 3;
         break;
      case 0xCB: /* MOV dp,Y */
         S9xAPUSetByteZ(IAPU.Registers.YA.B.Y, OP1);
         IAPU.PC += 2;
         break;
      case 0xCC: /* MOV abs,Y */
         Absolute();
         S9xAPUSetByte(IAPU.Registers.YA.B.Y, IAPU.Address);
         IAPU.PC += 3;
         break;
      case 0xCD: /* MOV X,#00 */
         IAPU.Registers.X = OP1;
         APUSetZN8(IAPU.Registers.X);
         IAPU.PC += 2;
         break;
      case 0xCE: /* POP X */
         Pop(IAPU.Registers.X);
         IAPU.PC++;
         break;
      case 0xCF: /* MUL YA */
         IAPU.Registers.YA.W = (uint16_t) IAPU.Registers.YA.B.A * IAPU.Registers.YA.B.Y;
         APUSetZN8(IAPU.Registers.YA.B.Y);
         IAPU.PC++;
         break;

      case 0xD0: /* BNE */
         Relative();
         if (!APUCheckZero())
         {
            IAPU.PC = IAPU.RAM + (uint16_t) Int16;
            APU.Cycles += IAPU.TwoCycles;
            APUShutdown();
         }
         else
            IAPU.PC += 2;
         break;
      case 0xD1:
         TCALL(13);
         break;
      case 0xD2:
         CLR(6);
         break;
      case 0xD3:
         BBC(6);
         break;
      case 0xD4: /* MOV dp+X, A */
         S9xAPUSetByteZ(IAPU.Registers.YA.B.A, OP1 + IAPU.Registers.X);
         IAPU.PC += 2;
         break;
      case 0xD5: /* MOV abs+X,A */
         AbsoluteX();
         S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
         IAPU.PC += 3;
         break;
      case 0xD6: /* MOV abs+Y,A */
         AbsoluteY();
         S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
         IAPU.PC += 3;
         break;
      case 0xD7: /* MOV (dp)+Y,A */
         IndirectIndexedY();
         S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
         IAPU.PC += 2;
         break;
      case 0xD8: /* MOV dp,X */
         S9xAPUSetByteZ(IAPU.Registers.X, OP1);
         IAPU.PC += 2;
         break;
      case 0xD9: /* MOV dp+Y,X */
         S9xAPUSetByteZ(IAPU.Registers.X, OP1 + IAPU.Registers.YA.B.Y);
         IAPU.PC += 2;
         break;
      case 0xDA: /* MOVW dp,YA */
         S9xAPUSetByteZ(IAPU.Registers.YA.B.A, OP1);
         S9xAPUSetByteZ(IAPU.Registers.YA.B.Y, OP1 + 1);
         IAPU.PC += 2;
         break;
      case 0xDB: /* MOV dp+X,Y */
         S9xAPUSetByteZ(IAPU.Registers.YA.B.Y, OP1 + IAPU.Registers.X);
         IAPU.PC += 2;
         break;
      case 0xDC: /* DEC Y */
         IAPU.Registers.YA.B.Y--;
         APUSetZN8(IAPU.Registers.YA.B.Y);
         IAPU.WaitCounter++;
         IAPU.PC++;
         break;
      case 0xDD: /* MOV A,Y */
         IAPU.Registers.YA.B.A = IAPU.Registers.YA.B.Y;
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;
      case 0xDE: /* CBNE dp+X,rel */
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
         break;
      case 0xDF: /* DAA */
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
         break;

      case 0xE0: /* CLRV */
         APUClearHalfCarry();
         APUClearOverflow();
         IAPU.PC++;
         break;
      case 0xE1:
         TCALL(14);
         break;
      case 0xE2:
         SET(7);
         break;
      case 0xE3:
         BBS(7);
         break;
      case 0xE4: /* MOV A, dp */
         IAPU.Registers.YA.B.A = S9xAPUGetByteZ(OP1);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0xE5: /* MOV A,abs */
         Absolute();
         IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 3;
         break;
      case 0xE6: /* MOV A,(X) */
         IAPU.Registers.YA.B.A = S9xAPUGetByteZ(IAPU.Registers.X);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC++;
         break;
      case 0xE7: /* MOV A,(dp+X) */
         IndexedXIndirect();
         IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0xE8: /* MOV A,#00 */
         IAPU.Registers.YA.B.A = OP1;
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0xE9: /* MOV X, abs */
         Absolute();
         IAPU.Registers.X = S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.X);
         IAPU.PC += 3;
         break;
      case 0xEA: /* NOT1 membit */
         MemBit();
         S9xAPUSetByte(S9xAPUGetByte(IAPU.Address) ^ (1 << IAPU.Bit), IAPU.Address);
         IAPU.PC += 3;
         break;
      case 0xEB: /* MOV Y,dp */
         IAPU.Registers.YA.B.Y = S9xAPUGetByteZ(OP1);
         APUSetZN8(IAPU.Registers.YA.B.Y);
         IAPU.PC += 2;
         break;
      case 0xEC: /* MOV Y,abs */
         Absolute();
         IAPU.Registers.YA.B.Y = S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.Y);
         IAPU.PC += 3;
         break;
      case 0xED: /* NOTC */
         IAPU._Carry ^= 1;
         IAPU.PC++;
         break;
      case 0xEE: /* POP Y */
         Pop(IAPU.Registers.YA.B.Y);
         IAPU.PC++;
         break;
      case 0xEF: /* SLEEP */
         APU.TimerEnabled[0] = APU.TimerEnabled[1] = APU.TimerEnabled[2] = false;
         IAPU.APUExecuting = false;
         break;

      case 0xF0: /* BEQ */
         Relative();
         if (APUCheckZero())
         {
            IAPU.PC = IAPU.RAM + (uint16_t) Int16;
            APU.Cycles += IAPU.TwoCycles;
            APUShutdown();
         }
         else
            IAPU.PC += 2;
         break;
      case 0xF1:
         TCALL(15);
         break;
      case 0xF2:
         CLR(7);
         break;
      case 0xF3:
         BBC(7);
         break;
      case 0xF4: /* MOV A, dp+X */
         IAPU.Registers.YA.B.A = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0xF5: /* MOV A, abs+X */
         AbsoluteX();
         IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 3;
         break;
      case 0xF6: /* MOV A, abs+Y */
         AbsoluteY();
         IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 3;
         break;
      case 0xF7: /* MOV A, (dp)+Y */
         IndirectIndexedY();
         IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
         APUSetZN8(IAPU.Registers.YA.B.A);
         IAPU.PC += 2;
         break;
      case 0xF8: /* MOV X,dp */
         IAPU.Registers.X = S9xAPUGetByteZ(OP1);
         APUSetZN8(IAPU.Registers.X);
         IAPU.PC += 2;
         break;
      case 0xF9: /* MOV X,dp+Y */
         IAPU.Registers.X = S9xAPUGetByteZ(OP1 + IAPU.Registers.YA.B.Y);
         APUSetZN8(IAPU.Registers.X);
         IAPU.PC += 2;
         break;
      case 0xFA: /* MOV dp(dest),dp(src) */
         S9xAPUSetByteZ(S9xAPUGetByteZ(OP1), OP2);
         IAPU.PC += 3;
         break;
      case 0xFB: /* MOV Y,dp+X */
         IAPU.Registers.YA.B.Y = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
         APUSetZN8(IAPU.Registers.YA.B.Y);
         IAPU.PC += 2;
         break;
      case 0xFC: /* INC Y */
         IAPU.Registers.YA.B.Y++;
         APUSetZN8(IAPU.Registers.YA.B.Y);
         IAPU.WaitCounter++;
         IAPU.PC++;
         break;
      case 0xFD: /* MOV Y,A */
         IAPU.Registers.YA.B.Y = IAPU.Registers.YA.B.A;
         APUSetZN8(IAPU.Registers.YA.B.Y);
         IAPU.PC++;
         break;
      case 0xFE: /* DBNZ Y,rel */
         Relative();
         IAPU.Registers.YA.B.Y--;
         if (IAPU.Registers.YA.B.Y != 0)
         {
            IAPU.PC = IAPU.RAM + (uint16_t) Int16;
            APU.Cycles += IAPU.TwoCycles;
         }
         else
            IAPU.PC += 2;
         break;
      case 0xFF: /* STOP */
         APU.TimerEnabled[0] = APU.TimerEnabled[1] = APU.TimerEnabled[2] = false;
         IAPU.APUExecuting = false;
         Settings.APUEnabled = false; /* re-enabled on next APU reset */
         break;
      }
   // }
}
#endif
