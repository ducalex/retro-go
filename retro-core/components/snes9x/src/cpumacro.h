/* This file is part of Snes9x. See LICENSE file. */

#ifndef _CPUMACRO_H_
#define _CPUMACRO_H_


extern int32_t OpAddress;

static INLINE void SetZN16(uint16_t Work)
{
   ICPU._Zero = Work != 0;
   ICPU._Negative = (uint8_t)(Work >> 8);
}

static INLINE void SetZN8(uint8_t Work)
{
   ICPU._Zero = Work;
   ICPU._Negative = Work;
}

static INLINE void ADC8(void)
{
   uint8_t Work8 = S9xGetByte(OpAddress);
   if (CheckDecimal())
   {
      int8_t Ans8;
      uint8_t A1 = (ICPU.Registers.A.W) & 0xf;
      uint8_t A2 = (ICPU.Registers.A.W >> 4) & 0xf;
      uint8_t W1 = Work8 & 0xf;
      uint8_t W2 = (Work8 >> 4) & 0xf;

      A1 += W1 + CheckCarry();
      if (A1 > 9)
      {
         A1 -= 10;
         A1 &= 0xf;
         A2++;
      }

      A2 += W2;
      if (A2 > 9)
      {
         A2 -= 10;
         A2 &= 0xf;
         SetCarry();
      }
      else
         ClearCarry();

      Ans8 = (A2 << 4) | A1;
      if (~(ICPU.Registers.AL ^ Work8) & (Work8 ^ Ans8) & 0x80)
         SetOverflow();
      else
         ClearOverflow();
      ICPU.Registers.AL = Ans8;
   }
   else
   {
      int16_t Ans16 = ICPU.Registers.AL + Work8 + CheckCarry();
      ICPU._Carry = Ans16 > 0xff;
      if (~(ICPU.Registers.AL ^ Work8) & (Work8 ^ (uint8_t) Ans16) & 0x80)
         SetOverflow();
      else
         ClearOverflow();
      ICPU.Registers.AL = (uint8_t) Ans16;
   }
   SetZN8(ICPU.Registers.AL);
}

static INLINE void ADC16(void)
{
   uint16_t Work16 = S9xGetWord(OpAddress);
   if (CheckDecimal())
   {
      uint16_t Ans16;
      uint8_t A1 = (ICPU.Registers.A.W) & 0xf;
      uint8_t A2 = (ICPU.Registers.A.W >> 4) & 0xf;
      uint8_t A3 = (ICPU.Registers.A.W >> 8) & 0xf;
      uint8_t A4 = (ICPU.Registers.A.W >> 12) & 0xf;
      uint8_t W1 = Work16 & 0xf;
      uint8_t W2 = (Work16 >> 4) & 0xf;
      uint8_t W3 = (Work16 >> 8) & 0xf;
      uint8_t W4 = (Work16 >> 12) & 0xf;

      A1 += W1 + CheckCarry();
      if (A1 > 9)
      {
         A1 -= 10;
         A1 &= 0xf;
         A2++;
      }

      A2 += W2;
      if (A2 > 9)
      {
         A2 -= 10;
         A2 &= 0xf;
         A3++;
      }

      A3 += W3;
      if (A3 > 9)
      {
         A3 -= 10;
         A3 &= 0xf;
         A4++;
      }

      A4 += W4;
      if (A4 > 9)
      {
         A4 -= 10;
         A4 &= 0xf;
         SetCarry();
      }
      else
         ClearCarry();

      Ans16 = (A4 << 12) | (A3 << 8) | (A2 << 4) | (A1);
      if (~(ICPU.Registers.A.W ^ Work16) & (Work16 ^ Ans16) & 0x8000)
         SetOverflow();
      else
         ClearOverflow();
      ICPU.Registers.A.W = Ans16;
   }
   else
   {
      uint32_t Ans32 = ICPU.Registers.A.W + Work16 + CheckCarry();
      ICPU._Carry = Ans32 > 0xffff;

      if (~(ICPU.Registers.A.W ^ Work16) & (Work16 ^ (uint16_t) Ans32) & 0x8000)
         SetOverflow();
      else
         ClearOverflow();
      ICPU.Registers.A.W = (uint16_t) Ans32;
   }
   SetZN16(ICPU.Registers.A.W);
}

static INLINE void AND16(void)
{
   ICPU.Registers.A.W &= S9xGetWord(OpAddress);
   SetZN16(ICPU.Registers.A.W);
}

static INLINE void AND8(void)
{
   ICPU.Registers.AL &= S9xGetByte(OpAddress);
   SetZN8(ICPU.Registers.AL);
}

static INLINE void A_ASL16(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU._Carry = (ICPU.Registers.AH & 0x80) != 0;
   ICPU.Registers.A.W <<= 1;
   SetZN16(ICPU.Registers.A.W);
}

static INLINE void A_ASL8(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU._Carry = (ICPU.Registers.AL & 0x80) != 0;
   ICPU.Registers.AL <<= 1;
   SetZN8(ICPU.Registers.AL);
}

static INLINE void ASL16(void)
{
   uint16_t Work16;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work16 = S9xGetWord(OpAddress);
   ICPU._Carry = (Work16 & 0x8000) != 0;
   Work16 <<= 1;
   S9xSetByte(Work16 >> 8, OpAddress + 1);
   S9xSetByte(Work16 & 0xFF, OpAddress);
   SetZN16(Work16);
}

static INLINE void ASL8(void)
{
   uint8_t Work8;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work8 = S9xGetByte(OpAddress);
   ICPU._Carry = (Work8 & 0x80) != 0;
   Work8 <<= 1;
   S9xSetByte(Work8, OpAddress);
   SetZN8(Work8);
}

static INLINE void BIT16(void)
{
   uint16_t Work16 = S9xGetWord(OpAddress);
   ICPU._Overflow = (Work16 & 0x4000) != 0;
   ICPU._Negative = (uint8_t)(Work16 >> 8);
   ICPU._Zero = (Work16 & ICPU.Registers.A.W) != 0;
}

static INLINE void BIT8(void)
{
   uint8_t Work8 = S9xGetByte(OpAddress);
   ICPU._Overflow = (Work8 & 0x40) != 0;
   ICPU._Negative = Work8;
   ICPU._Zero = Work8 & ICPU.Registers.AL;
}

static INLINE void CMP16(void)
{
   int32_t Int32 = (int32_t) ICPU.Registers.A.W - (int32_t) S9xGetWord(OpAddress);
   ICPU._Carry = Int32 >= 0;
   SetZN16((uint16_t) Int32);
}

static INLINE void CMP8(void)
{
   int16_t Int16 = (int16_t) ICPU.Registers.AL - (int16_t) S9xGetByte(OpAddress);
   ICPU._Carry = Int16 >= 0;
   SetZN8((uint8_t) Int16);
}

static INLINE void CMX16(void)
{
   int32_t Int32 = (int32_t) ICPU.Registers.X.W - (int32_t) S9xGetWord(OpAddress);
   ICPU._Carry = Int32 >= 0;
   SetZN16((uint16_t) Int32);
}

static INLINE void CMX8(void)
{
   int16_t Int16 = (int16_t) ICPU.Registers.XL - (int16_t) S9xGetByte(OpAddress);
   ICPU._Carry = Int16 >= 0;
   SetZN8((uint8_t) Int16);
}

static INLINE void CMY16(void)
{
   int32_t Int32 = (int32_t) ICPU.Registers.Y.W - (int32_t) S9xGetWord(OpAddress);
   ICPU._Carry = Int32 >= 0;
   SetZN16((uint16_t) Int32);
}

static INLINE void CMY8(void)
{
   int16_t Int16 = (int16_t) ICPU.Registers.YL - (int16_t) S9xGetByte(OpAddress);
   ICPU._Carry = Int16 >= 0;
   SetZN8((uint8_t) Int16);
}

static INLINE void A_DEC16(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   ICPU.Registers.A.W--;
   SetZN16(ICPU.Registers.A.W);
}

static INLINE void A_DEC8(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   ICPU.Registers.AL--;
   SetZN8(ICPU.Registers.AL);
}

static INLINE void DEC16(void)
{
   uint16_t Work16;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   Work16 = S9xGetWord(OpAddress) - 1;
   S9xSetByte(Work16 >> 8, OpAddress + 1);
   S9xSetByte(Work16 & 0xFF, OpAddress);
   SetZN16(Work16);
}

static INLINE void DEC8(void)
{
   uint8_t Work8;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   Work8 = S9xGetByte(OpAddress) - 1;
   S9xSetByte(Work8, OpAddress);
   SetZN8(Work8);
}

static INLINE void EOR16(void)
{
   ICPU.Registers.A.W ^= S9xGetWord(OpAddress);
   SetZN16(ICPU.Registers.A.W);
}

static INLINE void EOR8(void)
{
   ICPU.Registers.AL ^= S9xGetByte(OpAddress);
   SetZN8(ICPU.Registers.AL);
}

static INLINE void A_INC16(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   ICPU.Registers.A.W++;
   SetZN16(ICPU.Registers.A.W);
}

static INLINE void A_INC8(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   ICPU.Registers.AL++;
   SetZN8(ICPU.Registers.AL);
}

static INLINE void INC16(void)
{
   uint16_t Work16;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   Work16 = S9xGetWord(OpAddress) + 1;
   S9xSetByte(Work16 >> 8, OpAddress + 1);
   S9xSetByte(Work16 & 0xFF, OpAddress);
   SetZN16(Work16);
}

static INLINE void INC8(void)
{
   uint8_t Work8;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   Work8 = S9xGetByte(OpAddress) + 1;
   S9xSetByte(Work8, OpAddress);
   SetZN8(Work8);
}

static INLINE void LDA16(void)
{
   ICPU.Registers.A.W = S9xGetWord(OpAddress);
   SetZN16(ICPU.Registers.A.W);
}

static INLINE void LDA8(void)
{
   ICPU.Registers.AL = S9xGetByte(OpAddress);
   SetZN8(ICPU.Registers.AL);
}

static INLINE void LDX16(void)
{
   ICPU.Registers.X.W = S9xGetWord(OpAddress);
   SetZN16(ICPU.Registers.X.W);
}

static INLINE void LDX8(void)
{
   ICPU.Registers.XL = S9xGetByte(OpAddress);
   SetZN8(ICPU.Registers.XL);
}

static INLINE void LDY16(void)
{
   ICPU.Registers.Y.W = S9xGetWord(OpAddress);
   SetZN16(ICPU.Registers.Y.W);
}

static INLINE void LDY8(void)
{
   ICPU.Registers.YL = S9xGetByte(OpAddress);
   SetZN8(ICPU.Registers.YL);
}

static INLINE void A_LSR16(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU._Carry = ICPU.Registers.AL & 1;
   ICPU.Registers.A.W >>= 1;
   SetZN16(ICPU.Registers.A.W);
}

static INLINE void A_LSR8(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU._Carry = ICPU.Registers.AL & 1;
   ICPU.Registers.AL >>= 1;
   SetZN8(ICPU.Registers.AL);
}

static INLINE void LSR16(void)
{
   uint16_t Work16;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work16 = S9xGetWord(OpAddress);
   ICPU._Carry = Work16 & 1;
   Work16 >>= 1;
   S9xSetByte(Work16 >> 8, OpAddress + 1);
   S9xSetByte(Work16 & 0xFF, OpAddress);
   SetZN16(Work16);
}

static INLINE void LSR8(void)
{
   uint8_t Work8;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work8 = S9xGetByte(OpAddress);
   ICPU._Carry = Work8 & 1;
   Work8 >>= 1;
   S9xSetByte(Work8, OpAddress);
   SetZN8(Work8);
}

static INLINE void ORA16(void)
{
   ICPU.Registers.A.W |= S9xGetWord(OpAddress);
   SetZN16(ICPU.Registers.A.W);
}

static INLINE void ORA8(void)
{
   ICPU.Registers.AL |= S9xGetByte(OpAddress);
   SetZN8(ICPU.Registers.AL);
}

static INLINE void A_ROL16(void)
{
   uint32_t Work32;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work32 = (ICPU.Registers.A.W << 1) | CheckCarry();
   ICPU._Carry = Work32 > 0xffff;
   ICPU.Registers.A.W = (uint16_t) Work32;
   SetZN16((uint16_t) Work32);
}

static INLINE void A_ROL8(void)
{
   uint16_t Work16;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work16 = ICPU.Registers.AL;
   Work16 <<= 1;
   Work16 |= CheckCarry();
   ICPU._Carry = Work16 > 0xff;
   ICPU.Registers.AL = (uint8_t) Work16;
   SetZN8((uint8_t) Work16);
}

static INLINE void ROL16(void)
{
   uint32_t Work32;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work32 = S9xGetWord(OpAddress);
   Work32 <<= 1;
   Work32 |= CheckCarry();
   ICPU._Carry = Work32 > 0xffff;
   S9xSetByte((Work32 >> 8) & 0xFF, OpAddress + 1);
   S9xSetByte(Work32 & 0xFF, OpAddress);
   SetZN16((uint16_t) Work32);
}

static INLINE void ROL8(void)
{
   uint16_t Work16;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work16 = S9xGetByte(OpAddress);
   Work16 <<= 1;
   Work16 |= CheckCarry();
   ICPU._Carry = Work16 > 0xff;
   S9xSetByte((uint8_t) Work16, OpAddress);
   SetZN8((uint8_t) Work16);
}

static INLINE void A_ROR16(void)
{
   uint32_t Work32;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work32 = ICPU.Registers.A.W;
   Work32 |= (int32_t) CheckCarry() << 16;
   ICPU._Carry = (uint8_t)(Work32 & 1);
   Work32 >>= 1;
   ICPU.Registers.A.W = (uint16_t) Work32;
   SetZN16((uint16_t) Work32);
}

static INLINE void A_ROR8(void)
{
   uint16_t Work16;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work16 = ICPU.Registers.AL | ((uint16_t) CheckCarry() << 8);
   ICPU._Carry = (uint8_t) Work16 & 1;
   Work16 >>= 1;
   ICPU.Registers.AL = (uint8_t) Work16;
   SetZN8((uint8_t) Work16);
}

static INLINE void ROR16(void)
{
   uint32_t Work32;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work32 = S9xGetWord(OpAddress);
   Work32 |= (int32_t) CheckCarry() << 16;
   ICPU._Carry = (uint8_t)(Work32 & 1);
   Work32 >>= 1;
   S9xSetByte((Work32 >> 8) & 0x00FF, OpAddress + 1);
   S9xSetByte(Work32 & 0x00FF, OpAddress);
   SetZN16((uint16_t) Work32);
}

static INLINE void ROR8(void)
{
   uint16_t Work16;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work16 = S9xGetByte(OpAddress);
   Work16 |= (int32_t) CheckCarry() << 8;
   ICPU._Carry = (uint8_t)(Work16 & 1);
   Work16 >>= 1;
   S9xSetByte((uint8_t) Work16, OpAddress);
   SetZN8((uint8_t) Work16);
}

static INLINE void SBC16(void)
{
   uint16_t Work16 = S9xGetWord(OpAddress);
   if (CheckDecimal())
   {
      uint16_t Ans16;
      uint8_t A1 = (ICPU.Registers.A.W) & 0xf;
      uint8_t A2 = (ICPU.Registers.A.W >> 4) & 0xf;
      uint8_t A3 = (ICPU.Registers.A.W >> 8) & 0xf;
      uint8_t A4 = (ICPU.Registers.A.W >> 12) & 0xf;
      uint8_t W1 = Work16 & 0xf;
      uint8_t W2 = (Work16 >> 4) & 0xf;
      uint8_t W3 = (Work16 >> 8) & 0xf;
      uint8_t W4 = (Work16 >> 12) & 0xf;

      A1 -= W1 + !CheckCarry();
      A2 -= W2;
      A3 -= W3;
      A4 -= W4;
      if (A1 > 9)
      {
         A1 += 10;
         A2--;
      }
      if (A2 > 9)
      {
         A2 += 10;
         A3--;
      }
      if (A3 > 9)
      {
         A3 += 10;
         A4--;
      }
      if (A4 > 9)
      {
         A4 += 10;
         ClearCarry();
      }
      else
         SetCarry();

      Ans16 = (A4 << 12) | (A3 << 8) | (A2 << 4) | (A1);
      if ((ICPU.Registers.A.W ^ Work16) & (ICPU.Registers.A.W ^ Ans16) & 0x8000)
         SetOverflow();
      else
         ClearOverflow();
      ICPU.Registers.A.W = Ans16;
   }
   else
   {
      int32_t Int32 = (int32_t) ICPU.Registers.A.W - (int32_t) Work16 + (int32_t) CheckCarry() - 1;
      ICPU._Carry = Int32 >= 0;
      if ((ICPU.Registers.A.W ^ Work16) & (ICPU.Registers.A.W ^ (uint16_t) Int32) & 0x8000)
         SetOverflow();
      else
         ClearOverflow();
      ICPU.Registers.A.W = (uint16_t) Int32;
   }
   SetZN16(ICPU.Registers.A.W);
}

static INLINE void SBC8(void)
{
   uint8_t Work8 = S9xGetByte(OpAddress);
   if (CheckDecimal())
   {
      uint8_t Ans8;
      uint8_t A1 = (ICPU.Registers.A.W) & 0xf;
      uint8_t A2 = (ICPU.Registers.A.W >> 4) & 0xf;
      uint8_t W1 = Work8 & 0xf;
      uint8_t W2 = (Work8 >> 4) & 0xf;

      A1 -= W1 + !CheckCarry();
      A2 -= W2;
      if (A1 > 9)
      {
         A1 += 10;
         A2--;
      }
      if (A2 > 9)
      {
         A2 += 10;
         ClearCarry();
      }
      else
         SetCarry();

      Ans8 = (A2 << 4) | A1;
      if ((ICPU.Registers.AL ^ Work8) & (ICPU.Registers.AL ^ Ans8) & 0x80)
         SetOverflow();
      else
         ClearOverflow();
      ICPU.Registers.AL = Ans8;
   }
   else
   {
      int16_t Int16 = (int16_t) ICPU.Registers.AL - (int16_t) Work8 + (int16_t) CheckCarry() - 1;
      ICPU._Carry = Int16 >= 0;
      if ((ICPU.Registers.AL ^ Work8) & (ICPU.Registers.AL ^ (uint8_t) Int16) & 0x80)
         SetOverflow();
      else
         ClearOverflow();
      ICPU.Registers.AL = (uint8_t) Int16;
   }
   SetZN8(ICPU.Registers.AL);
}

static INLINE void STA16(void)
{
   S9xSetWord(ICPU.Registers.A.W, OpAddress);
}

static INLINE void STA8(void)
{
   S9xSetByte(ICPU.Registers.AL, OpAddress);
}

static INLINE void STX16(void)
{
   S9xSetWord(ICPU.Registers.X.W, OpAddress);
}

static INLINE void STX8(void)
{
   S9xSetByte(ICPU.Registers.XL, OpAddress);
}

static INLINE void STY16(void)
{
   S9xSetWord(ICPU.Registers.Y.W, OpAddress);
}

static INLINE void STY8(void)
{
   S9xSetByte(ICPU.Registers.YL, OpAddress);
}

static INLINE void STZ16(void)
{
   S9xSetWord(0, OpAddress);
}

static INLINE void STZ8(void)
{
   S9xSetByte(0, OpAddress);
}

static INLINE void TSB16(void)
{
   uint16_t Work16;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work16 = S9xGetWord(OpAddress);
   ICPU._Zero = (Work16 & ICPU.Registers.A.W) != 0;
   Work16 |= ICPU.Registers.A.W;
   S9xSetByte(Work16 >> 8, OpAddress + 1);
   S9xSetByte(Work16 & 0xFF, OpAddress);
}

static INLINE void TSB8(void)
{
   uint8_t Work8;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work8 = S9xGetByte(OpAddress);
   ICPU._Zero = Work8 & ICPU.Registers.AL;
   Work8 |= ICPU.Registers.AL;
   S9xSetByte(Work8, OpAddress);
}

static INLINE void TRB16(void)
{
   uint16_t Work16;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work16 = S9xGetWord(OpAddress);
   ICPU._Zero = (Work16 & ICPU.Registers.A.W) != 0;
   Work16 &= ~ICPU.Registers.A.W;
   S9xSetByte(Work16 >> 8, OpAddress + 1);
   S9xSetByte(Work16 & 0xFF, OpAddress);
}

static INLINE void TRB8(void)
{
   uint8_t Work8;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   Work8 = S9xGetByte(OpAddress);
   ICPU._Zero = Work8 & ICPU.Registers.AL;
   Work8 &= ~ICPU.Registers.AL;
   S9xSetByte(Work8, OpAddress);
}
#endif
