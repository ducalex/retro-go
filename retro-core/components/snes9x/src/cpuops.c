/* This file is part of Snes9x. See LICENSE file. */

/*****************************************************************************/
/* CPU-S9xOpcodes.CPP                                                        */
/* This file contains all the opcodes                                        */
/*****************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "apu.h"
#include "cpuexec.h"
#include "cpuaddr.h"
#include "cpuops.h"
#include "cpumacro.h"
#include "apu.h"


/* ADC */
static void Op69M1(void)
{
   Immediate8();
   ADC8();
}

static void Op69M0(void)
{
   Immediate16();
   ADC16();
}

static void Op65M1(void)
{
   Direct(true);
   ADC8();
}

static void Op65M0(void)
{
   Direct(true);
   ADC16();
}

static void Op75M1(void)
{
   DirectIndexedX(true);
   ADC8();
}

static void Op75M0(void)
{
   DirectIndexedX(true);
   ADC16();
}

static void Op72M1(void)
{
   DirectIndirect(true);
   ADC8();
}

static void Op72M0(void)
{
   DirectIndirect(true);
   ADC16();
}

static void Op61M1(void)
{
   DirectIndexedIndirect(true);
   ADC8();
}

static void Op61M0(void)
{
   DirectIndexedIndirect(true);
   ADC16();
}

static void Op71M1(void)
{
   DirectIndirectIndexed(true);
   ADC8();
}

static void Op71M0(void)
{
   DirectIndirectIndexed(true);
   ADC16();
}

static void Op67M1(void)
{
   DirectIndirectLong(true);
   ADC8();
}

static void Op67M0(void)
{
   DirectIndirectLong(true);
   ADC16();
}

static void Op77M1(void)
{
   DirectIndirectIndexedLong(true);
   ADC8();
}

static void Op77M0(void)
{
   DirectIndirectIndexedLong(true);
   ADC16();
}

static void Op6DM1(void)
{
   Absolute(true);
   ADC8();
}

static void Op6DM0(void)
{
   Absolute(true);
   ADC16();
}

static void Op7DM1(void)
{
   AbsoluteIndexedX(true);
   ADC8();
}

static void Op7DM0(void)
{
   AbsoluteIndexedX(true);
   ADC16();
}

static void Op79M1(void)
{
   AbsoluteIndexedY(true);
   ADC8();
}

static void Op79M0(void)
{
   AbsoluteIndexedY(true);
   ADC16();
}

static void Op6FM1(void)
{
   AbsoluteLong(true);
   ADC8();
}

static void Op6FM0(void)
{
   AbsoluteLong(true);
   ADC16();
}

static void Op7FM1(void)
{
   AbsoluteLongIndexedX(true);
   ADC8();
}

static void Op7FM0(void)
{
   AbsoluteLongIndexedX(true);
   ADC16();
}

static void Op63M1(void)
{
   StackRelative(true);
   ADC8();
}

static void Op63M0(void)
{
   StackRelative(true);
   ADC16();
}

static void Op73M1(void)
{
   StackRelativeIndirectIndexed(true);
   ADC8();
}

static void Op73M0(void)
{
   StackRelativeIndirectIndexed(true);
   ADC16();
}

/* AND */
static void Op29M1(void)
{
   ICPU.Registers.AL &= *CPU.PC++;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeed;
#endif
   SetZN8(ICPU.Registers.AL);
}

static void Op29M0(void)
{
#ifdef FAST_LSB_WORD_ACCESS
   ICPU.Registers.A.W &= *(uint16_t*) CPU.PC;
#else
   ICPU.Registers.A.W &= *CPU.PC + (*(CPU.PC + 1) << 8);
#endif
   CPU.PC += 2;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2;
#endif
   SetZN16(ICPU.Registers.A.W);
}

static void Op25M1(void)
{
   Direct(true);
   AND8();
}

static void Op25M0(void)
{
   Direct(true);
   AND16();
}

static void Op35M1(void)
{
   DirectIndexedX(true);
   AND8();
}

static void Op35M0(void)
{
   DirectIndexedX(true);
   AND16();
}

static void Op32M1(void)
{
   DirectIndirect(true);
   AND8();
}

static void Op32M0(void)
{
   DirectIndirect(true);
   AND16();
}

static void Op21M1(void)
{
   DirectIndexedIndirect(true);
   AND8();
}

static void Op21M0(void)
{
   DirectIndexedIndirect(true);
   AND16();
}

static void Op31M1(void)
{
   DirectIndirectIndexed(true);
   AND8();
}

static void Op31M0(void)
{
   DirectIndirectIndexed(true);
   AND16();
}

static void Op27M1(void)
{
   DirectIndirectLong(true);
   AND8();
}

static void Op27M0(void)
{
   DirectIndirectLong(true);
   AND16();
}

static void Op37M1(void)
{
   DirectIndirectIndexedLong(true);
   AND8();
}

static void Op37M0(void)
{
   DirectIndirectIndexedLong(true);
   AND16();
}

static void Op2DM1(void)
{
   Absolute(true);
   AND8();
}

static void Op2DM0(void)
{
   Absolute(true);
   AND16();
}

static void Op3DM1(void)
{
   AbsoluteIndexedX(true);
   AND8();
}

static void Op3DM0(void)
{
   AbsoluteIndexedX(true);
   AND16();
}

static void Op39M1(void)
{
   AbsoluteIndexedY(true);
   AND8();
}

static void Op39M0(void)
{
   AbsoluteIndexedY(true);
   AND16();
}

static void Op2FM1(void)
{
   AbsoluteLong(true);
   AND8();
}

static void Op2FM0(void)
{
   AbsoluteLong(true);
   AND16();
}

static void Op3FM1(void)
{
   AbsoluteLongIndexedX(true);
   AND8();
}

static void Op3FM0(void)
{
   AbsoluteLongIndexedX(true);
   AND16();
}

static void Op23M1(void)
{
   StackRelative(true);
   AND8();
}

static void Op23M0(void)
{
   StackRelative(true);
   AND16();
}

static void Op33M1(void)
{
   StackRelativeIndirectIndexed(true);
   AND8();
}

static void Op33M0(void)
{
   StackRelativeIndirectIndexed(true);
   AND16();
}

/* ASL */
static void Op0AM1(void)
{
   A_ASL8();
}

static void Op0AM0(void)
{
   A_ASL16();
}

static void Op06M1(void)
{
   Direct(true);
   ASL8();
}

static void Op06M0(void)
{
   Direct(true);
   ASL16();
}

static void Op16M1(void)
{
   DirectIndexedX(true);
   ASL8();
}

static void Op16M0(void)
{
   DirectIndexedX(true);
   ASL16();
}

static void Op0EM1(void)
{
   Absolute(true);
   ASL8();
}

static void Op0EM0(void)
{
   Absolute(true);
   ASL16();
}

static void Op1EM1(void)
{
   AbsoluteIndexedX(true);
   ASL8();
}

static void Op1EM0(void)
{
   AbsoluteIndexedX(true);
   ASL16();
}

/* BIT */
static void Op89M1(void)
{
   ICPU._Zero = ICPU.Registers.AL & *CPU.PC++;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeed;
#endif
}

static void Op89M0(void)
{
#ifdef FAST_LSB_WORD_ACCESS
   ICPU._Zero = (ICPU.Registers.A.W & *(uint16_t*) CPU.PC) != 0;
#else
   ICPU._Zero = (ICPU.Registers.A.W & (*CPU.PC + (*(CPU.PC + 1) << 8))) != 0;
#endif
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2;
#endif
   CPU.PC += 2;
}

static void Op24M1(void)
{
   Direct(true);
   BIT8();
}

static void Op24M0(void)
{
   Direct(true);
   BIT16();
}

static void Op34M1(void)
{
   DirectIndexedX(true);
   BIT8();
}

static void Op34M0(void)
{
   DirectIndexedX(true);
   BIT16();
}

static void Op2CM1(void)
{
   Absolute(true);
   BIT8();
}

static void Op2CM0(void)
{
   Absolute(true);
   BIT16();
}

static void Op3CM1(void)
{
   AbsoluteIndexedX(true);
   BIT8();
}

static void Op3CM0(void)
{
   AbsoluteIndexedX(true);
   BIT16();
}

/* CMP */
static void OpC9M1(void)
{
   int32_t Int32 = (int32_t) ICPU.Registers.AL - (int32_t) *CPU.PC++;
   ICPU._Carry = Int32 >= 0;
   SetZN8((uint8_t) Int32);
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeed;
#endif
}

static void OpC9M0(void)
{
#ifdef FAST_LSB_WORD_ACCESS
   int32_t Int32 = (int32_t) ICPU.Registers.A.W - (int32_t) *(uint16_t*)CPU.PC;
#else
   int32_t Int32 = (int32_t) ICPU.Registers.A.W - (int32_t)(*CPU.PC + (*(CPU.PC + 1) << 8));
#endif
   ICPU._Carry = Int32 >= 0;
   SetZN16((uint16_t) Int32);
   CPU.PC += 2;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2;
#endif
}

static void OpC5M1(void)
{
   Direct(true);
   CMP8();
}

static void OpC5M0(void)
{
   Direct(true);
   CMP16();
}

static void OpD5M1(void)
{
   DirectIndexedX(true);
   CMP8();
}

static void OpD5M0(void)
{
   DirectIndexedX(true);
   CMP16();
}

static void OpD2M1(void)
{
   DirectIndirect(true);
   CMP8();
}

static void OpD2M0(void)
{
   DirectIndirect(true);
   CMP16();
}

static void OpC1M1(void)
{
   DirectIndexedIndirect(true);
   CMP8();
}

static void OpC1M0(void)
{
   DirectIndexedIndirect(true);
   CMP16();
}

static void OpD1M1(void)
{
   DirectIndirectIndexed(true);
   CMP8();
}

static void OpD1M0(void)
{
   DirectIndirectIndexed(true);
   CMP16();
}

static void OpC7M1(void)
{
   DirectIndirectLong(true);
   CMP8();
}

static void OpC7M0(void)
{
   DirectIndirectLong(true);
   CMP16();
}

static void OpD7M1(void)
{
   DirectIndirectIndexedLong(true);
   CMP8();
}

static void OpD7M0(void)
{
   DirectIndirectIndexedLong(true);
   CMP16();
}

static void OpCDM1(void)
{
   Absolute(true);
   CMP8();
}

static void OpCDM0(void)
{
   Absolute(true);
   CMP16();
}

static void OpDDM1(void)
{
   AbsoluteIndexedX(true);
   CMP8();
}

static void OpDDM0(void)
{
   AbsoluteIndexedX(true);
   CMP16();
}

static void OpD9M1(void)
{
   AbsoluteIndexedY(true);
   CMP8();
}

static void OpD9M0(void)
{
   AbsoluteIndexedY(true);
   CMP16();
}

static void OpCFM1(void)
{
   AbsoluteLong(true);
   CMP8();
}

static void OpCFM0(void)
{
   AbsoluteLong(true);
   CMP16();
}

static void OpDFM1(void)
{
   AbsoluteLongIndexedX(true);
   CMP8();
}

static void OpDFM0(void)
{
   AbsoluteLongIndexedX(true);
   CMP16();
}

static void OpC3M1(void)
{
   StackRelative(true);
   CMP8();
}

static void OpC3M0(void)
{
   StackRelative(true);
   CMP16();
}

static void OpD3M1(void)
{
   StackRelativeIndirectIndexed(true);
   CMP8();
}

static void OpD3M0(void)
{
   StackRelativeIndirectIndexed(true);
   CMP16();
}

/* CMX */
static void OpE0X1(void)
{
   int32_t Int32 = (int32_t) ICPU.Registers.XL - (int32_t) *CPU.PC++;
   ICPU._Carry = Int32 >= 0;
   SetZN8((uint8_t) Int32);
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeed;
#endif
}

static void OpE0X0(void)
{
#ifdef FAST_LSB_WORD_ACCESS
   int32_t Int32 = (int32_t) ICPU.Registers.X.W - (int32_t) *(uint16_t*)CPU.PC;
#else
   int32_t Int32 = (int32_t) ICPU.Registers.X.W - (int32_t)(*CPU.PC + (*(CPU.PC + 1) << 8));
#endif
   ICPU._Carry = Int32 >= 0;
   SetZN16((uint16_t) Int32);
   CPU.PC += 2;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2;
#endif
}

static void OpE4X1(void)
{
   Direct(true);
   CMX8();
}

static void OpE4X0(void)
{
   Direct(true);
   CMX16();
}

static void OpECX1(void)
{
   Absolute(true);
   CMX8();
}

static void OpECX0(void)
{
   Absolute(true);
   CMX16();
}

/* CMY */
static void OpC0X1(void)
{
   int32_t Int32 = (int32_t) ICPU.Registers.YL - (int32_t) *CPU.PC++;
   ICPU._Carry = Int32 >= 0;
   SetZN8((uint8_t) Int32);
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeed;
#endif
}

static void OpC0X0(void)
{
#ifdef FAST_LSB_WORD_ACCESS
   int32_t Int32 = (int32_t) ICPU.Registers.Y.W - (int32_t) *(uint16_t*)CPU.PC;
#else
   int32_t Int32 = (int32_t) ICPU.Registers.Y.W - (int32_t)(*CPU.PC + (*(CPU.PC + 1) << 8));
#endif
   ICPU._Carry = Int32 >= 0;
   SetZN16((uint16_t) Int32);
   CPU.PC += 2;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2;
#endif
}

static void OpC4X1(void)
{
   Direct(true);
   CMY8();
}

static void OpC4X0(void)
{
   Direct(true);
   CMY16();
}

static void OpCCX1(void)
{
   Absolute(true);
   CMY8();
}

static void OpCCX0(void)
{
   Absolute(true);
   CMY16();
}

/* DEC */
static void Op3AM1(void)
{
   A_DEC8();
}

static void Op3AM0(void)
{
   A_DEC16();
}

static void OpC6M1(void)
{
   Direct(true);
   DEC8();
}

static void OpC6M0(void)
{
   Direct(true);
   DEC16();
}

static void OpD6M1(void)
{
   DirectIndexedX(true);
   DEC8();
}

static void OpD6M0(void)
{
   DirectIndexedX(true);
   DEC16();
}

static void OpCEM1(void)
{
   Absolute(true);
   DEC8();
}

static void OpCEM0(void)
{
   Absolute(true);
   DEC16();
}

static void OpDEM1(void)
{
   AbsoluteIndexedX(true);
   DEC8();
}

static void OpDEM0(void)
{
   AbsoluteIndexedX(true);
   DEC16();
}

/* EOR */
static void Op49M1(void)
{
   ICPU.Registers.AL ^= *CPU.PC++;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeed;
#endif
   SetZN8(ICPU.Registers.AL);
}

static void Op49M0(void)
{
#ifdef FAST_LSB_WORD_ACCESS
   ICPU.Registers.A.W ^= *(uint16_t*) CPU.PC;
#else
   ICPU.Registers.A.W ^= *CPU.PC + (*(CPU.PC + 1) << 8);
#endif
   CPU.PC += 2;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2;
#endif
   SetZN16(ICPU.Registers.A.W);
}

static void Op45M1(void)
{
   Direct(true);
   EOR8();
}

static void Op45M0(void)
{
   Direct(true);
   EOR16();
}

static void Op55M1(void)
{
   DirectIndexedX(true);
   EOR8();
}

static void Op55M0(void)
{
   DirectIndexedX(true);
   EOR16();
}

static void Op52M1(void)
{
   DirectIndirect(true);
   EOR8();
}

static void Op52M0(void)
{
   DirectIndirect(true);
   EOR16();
}

static void Op41M1(void)
{
   DirectIndexedIndirect(true);
   EOR8();
}

static void Op41M0(void)
{
   DirectIndexedIndirect(true);
   EOR16();
}

static void Op51M1(void)
{
   DirectIndirectIndexed(true);
   EOR8();
}

static void Op51M0(void)
{
   DirectIndirectIndexed(true);
   EOR16();
}

static void Op47M1(void)
{
   DirectIndirectLong(true);
   EOR8();
}

static void Op47M0(void)
{
   DirectIndirectLong(true);
   EOR16();
}

static void Op57M1(void)
{
   DirectIndirectIndexedLong(true);
   EOR8();
}

static void Op57M0(void)
{
   DirectIndirectIndexedLong(true);
   EOR16();
}

static void Op4DM1(void)
{
   Absolute(true);
   EOR8();
}

static void Op4DM0(void)
{
   Absolute(true);
   EOR16();
}

static void Op5DM1(void)
{
   AbsoluteIndexedX(true);
   EOR8();
}

static void Op5DM0(void)
{
   AbsoluteIndexedX(true);
   EOR16();
}

static void Op59M1(void)
{
   AbsoluteIndexedY(true);
   EOR8();
}

static void Op59M0(void)
{
   AbsoluteIndexedY(true);
   EOR16();
}

static void Op4FM1(void)
{
   AbsoluteLong(true);
   EOR8();
}

static void Op4FM0(void)
{
   AbsoluteLong(true);
   EOR16();
}

static void Op5FM1(void)
{
   AbsoluteLongIndexedX(true);
   EOR8();
}

static void Op5FM0(void)
{
   AbsoluteLongIndexedX(true);
   EOR16();
}

static void Op43M1(void)
{
   StackRelative(true);
   EOR8();
}

static void Op43M0(void)
{
   StackRelative(true);
   EOR16();
}

static void Op53M1(void)
{
   StackRelativeIndirectIndexed(true);
   EOR8();
}

static void Op53M0(void)
{
   StackRelativeIndirectIndexed(true);
   EOR16();
}

/* INC */
static void Op1AM1(void)
{
   A_INC8();
}

static void Op1AM0(void)
{
   A_INC16();
}

static void OpE6M1(void)
{
   Direct(true);
   INC8();
}

static void OpE6M0(void)
{
   Direct(true);
   INC16();
}

static void OpF6M1(void)
{
   DirectIndexedX(true);
   INC8();
}

static void OpF6M0(void)
{
   DirectIndexedX(true);
   INC16();
}

static void OpEEM1(void)
{
   Absolute(true);
   INC8();
}

static void OpEEM0(void)
{
   Absolute(true);
   INC16();
}

static void OpFEM1(void)
{
   AbsoluteIndexedX(true);
   INC8();
}

static void OpFEM0(void)
{
   AbsoluteIndexedX(true);
   INC16();
}

/* LDA */
static void OpA9M1(void)
{
   ICPU.Registers.AL = *CPU.PC++;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeed;
#endif
   SetZN8(ICPU.Registers.AL);
}

static void OpA9M0(void)
{
#ifdef FAST_LSB_WORD_ACCESS
   ICPU.Registers.A.W = *(uint16_t*) CPU.PC;
#else
   ICPU.Registers.A.W = *CPU.PC + (*(CPU.PC + 1) << 8);
#endif

   CPU.PC += 2;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2;
#endif
   SetZN16(ICPU.Registers.A.W);
}

static void OpA5M1(void)
{
   Direct(true);
   LDA8();
}

static void OpA5M0(void)
{
   Direct(true);
   LDA16();
}

static void OpB5M1(void)
{
   DirectIndexedX(true);
   LDA8();
}

static void OpB5M0(void)
{
   DirectIndexedX(true);
   LDA16();
}

static void OpB2M1(void)
{
   DirectIndirect(true);
   LDA8();
}

static void OpB2M0(void)
{
   DirectIndirect(true);
   LDA16();
}

static void OpA1M1(void)
{
   DirectIndexedIndirect(true);
   LDA8();
}

static void OpA1M0(void)
{
   DirectIndexedIndirect(true);
   LDA16();
}

static void OpB1M1(void)
{
   DirectIndirectIndexed(true);
   LDA8();
}

static void OpB1M0(void)
{
   DirectIndirectIndexed(true);
   LDA16();
}

static void OpA7M1(void)
{
   DirectIndirectLong(true);
   LDA8();
}

static void OpA7M0(void)
{
   DirectIndirectLong(true);
   LDA16();
}

static void OpB7M1(void)
{
   DirectIndirectIndexedLong(true);
   LDA8();
}

static void OpB7M0(void)
{
   DirectIndirectIndexedLong(true);
   LDA16();
}

static void OpADM1(void)
{
   Absolute(true);
   LDA8();
}

static void OpADM0(void)
{
   Absolute(true);
   LDA16();
}

static void OpBDM1(void)
{
   AbsoluteIndexedX(true);
   LDA8();
}

static void OpBDM0(void)
{
   AbsoluteIndexedX(true);
   LDA16();
}

static void OpB9M1(void)
{
   AbsoluteIndexedY(true);
   LDA8();
}

static void OpB9M0(void)
{
   AbsoluteIndexedY(true);
   LDA16();
}

static void OpAFM1(void)
{
   AbsoluteLong(true);
   LDA8();
}

static void OpAFM0(void)
{
   AbsoluteLong(true);
   LDA16();
}

static void OpBFM1(void)
{
   AbsoluteLongIndexedX(true);
   LDA8();
}

static void OpBFM0(void)
{
   AbsoluteLongIndexedX(true);
   LDA16();
}

static void OpA3M1(void)
{
   StackRelative(true);
   LDA8();
}

static void OpA3M0(void)
{
   StackRelative(true);
   LDA16();
}

static void OpB3M1(void)
{
   StackRelativeIndirectIndexed(true);
   LDA8();
}

static void OpB3M0(void)
{
   StackRelativeIndirectIndexed(true);
   LDA16();
}

/* LDX */
static void OpA2X1(void)
{
   ICPU.Registers.XL = *CPU.PC++;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeed;
#endif
   SetZN8(ICPU.Registers.XL);
}

static void OpA2X0(void)
{
#ifdef FAST_LSB_WORD_ACCESS
   ICPU.Registers.X.W = *(uint16_t*) CPU.PC;
#else
   ICPU.Registers.X.W = *CPU.PC + (*(CPU.PC + 1) << 8);
#endif
   CPU.PC += 2;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2;
#endif
   SetZN16(ICPU.Registers.X.W);
}

static void OpA6X1(void)
{
   Direct(true);
   LDX8();
}

static void OpA6X0(void)
{
   Direct(true);
   LDX16();
}

static void OpB6X1(void)
{
   DirectIndexedY(true);
   LDX8();
}

static void OpB6X0(void)
{
   DirectIndexedY(true);
   LDX16();
}

static void OpAEX1(void)
{
   Absolute(true);
   LDX8();
}

static void OpAEX0(void)
{
   Absolute(true);
   LDX16();
}

static void OpBEX1(void)
{
   AbsoluteIndexedY(true);
   LDX8();
}

static void OpBEX0(void)
{
   AbsoluteIndexedY(true);
   LDX16();
}

/* LDY */
static void OpA0X1(void)
{
   ICPU.Registers.YL = *CPU.PC++;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeed;
#endif
   SetZN8(ICPU.Registers.YL);
}

static void OpA0X0(void)
{
#ifdef FAST_LSB_WORD_ACCESS
   ICPU.Registers.Y.W = *(uint16_t*) CPU.PC;
#else
   ICPU.Registers.Y.W = *CPU.PC + (*(CPU.PC + 1) << 8);
#endif

   CPU.PC += 2;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2;
#endif
   SetZN16(ICPU.Registers.Y.W);
}

static void OpA4X1(void)
{
   Direct(true);
   LDY8();
}

static void OpA4X0(void)
{
   Direct(true);
   LDY16();
}

static void OpB4X1(void)
{
   DirectIndexedX(true);
   LDY8();
}

static void OpB4X0(void)
{
   DirectIndexedX(true);
   LDY16();
}

static void OpACX1(void)
{
   Absolute(true);
   LDY8();
}

static void OpACX0(void)
{
   Absolute(true);
   LDY16();
}

static void OpBCX1(void)
{
   AbsoluteIndexedX(true);
   LDY8();
}

static void OpBCX0(void)
{
   AbsoluteIndexedX(true);
   LDY16();
}

/* LSR */
static void Op4AM1(void)
{
   A_LSR8();
}

static void Op4AM0(void)
{
   A_LSR16();
}

static void Op46M1(void)
{
   Direct(true);
   LSR8();
}

static void Op46M0(void)
{
   Direct(true);
   LSR16();
}

static void Op56M1(void)
{
   DirectIndexedX(true);
   LSR8();
}

static void Op56M0(void)
{
   DirectIndexedX(true);
   LSR16();
}

static void Op4EM1(void)
{
   Absolute(true);
   LSR8();
}

static void Op4EM0(void)
{
   Absolute(true);
   LSR16();
}

static void Op5EM1(void)
{
   AbsoluteIndexedX(true);
   LSR8();
}

static void Op5EM0(void)
{
   AbsoluteIndexedX(true);
   LSR16();
}

/* ORA */
static void Op09M1(void)
{
   ICPU.Registers.AL |= *CPU.PC++;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeed;
#endif
   SetZN8(ICPU.Registers.AL);
}

static void Op09M0(void)
{
#ifdef FAST_LSB_WORD_ACCESS
   ICPU.Registers.A.W |= *(uint16_t*) CPU.PC;
#else
   ICPU.Registers.A.W |= *CPU.PC + (*(CPU.PC + 1) << 8);
#endif
   CPU.PC += 2;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2;
#endif
   SetZN16(ICPU.Registers.A.W);
}

static void Op05M1(void)
{
   Direct(true);
   ORA8();
}

static void Op05M0(void)
{
   Direct(true);
   ORA16();
}

static void Op15M1(void)
{
   DirectIndexedX(true);
   ORA8();
}

static void Op15M0(void)
{
   DirectIndexedX(true);
   ORA16();
}

static void Op12M1(void)
{
   DirectIndirect(true);
   ORA8();
}

static void Op12M0(void)
{
   DirectIndirect(true);
   ORA16();
}

static void Op01M1(void)
{
   DirectIndexedIndirect(true);
   ORA8();
}

static void Op01M0(void)
{
   DirectIndexedIndirect(true);
   ORA16();
}

static void Op11M1(void)
{
   DirectIndirectIndexed(true);
   ORA8();
}

static void Op11M0(void)
{
   DirectIndirectIndexed(true);
   ORA16();
}

static void Op07M1(void)
{
   DirectIndirectLong(true);
   ORA8();
}

static void Op07M0(void)
{
   DirectIndirectLong(true);
   ORA16();
}

static void Op17M1(void)
{
   DirectIndirectIndexedLong(true);
   ORA8();
}

static void Op17M0(void)
{
   DirectIndirectIndexedLong(true);
   ORA16();
}

static void Op0DM1(void)
{
   Absolute(true);
   ORA8();
}

static void Op0DM0(void)
{
   Absolute(true);
   ORA16();
}

static void Op1DM1(void)
{
   AbsoluteIndexedX(true);
   ORA8();
}

static void Op1DM0(void)
{
   AbsoluteIndexedX(true);
   ORA16();
}

static void Op19M1(void)
{
   AbsoluteIndexedY(true);
   ORA8();
}

static void Op19M0(void)
{
   AbsoluteIndexedY(true);
   ORA16();
}

static void Op0FM1(void)
{
   AbsoluteLong(true);
   ORA8();
}

static void Op0FM0(void)
{
   AbsoluteLong(true);
   ORA16();
}

static void Op1FM1(void)
{
   AbsoluteLongIndexedX(true);
   ORA8();
}

static void Op1FM0(void)
{
   AbsoluteLongIndexedX(true);
   ORA16();
}

static void Op03M1(void)
{
   StackRelative(true);
   ORA8();
}

static void Op03M0(void)
{
   StackRelative(true);
   ORA16();
}

static void Op13M1(void)
{
   StackRelativeIndirectIndexed(true);
   ORA8();
}

static void Op13M0(void)
{
   StackRelativeIndirectIndexed(true);
   ORA16();
}

/* ROL */
static void Op2AM1(void)
{
   A_ROL8();
}

static void Op2AM0(void)
{
   A_ROL16();
}

static void Op26M1(void)
{
   Direct(true);
   ROL8();
}

static void Op26M0(void)
{
   Direct(true);
   ROL16();
}

static void Op36M1(void)
{
   DirectIndexedX(true);
   ROL8();
}

static void Op36M0(void)
{
   DirectIndexedX(true);
   ROL16();
}

static void Op2EM1(void)
{
   Absolute(true);
   ROL8();
}

static void Op2EM0(void)
{
   Absolute(true);
   ROL16();
}

static void Op3EM1(void)
{
   AbsoluteIndexedX(true);
   ROL8();
}

static void Op3EM0(void)
{
   AbsoluteIndexedX(true);
   ROL16();
}

/* ROR */
static void Op6AM1(void)
{
   A_ROR8();
}

static void Op6AM0(void)
{
   A_ROR16();
}

static void Op66M1(void)
{
   Direct(true);
   ROR8();
}

static void Op66M0(void)
{
   Direct(true);
   ROR16();
}

static void Op76M1(void)
{
   DirectIndexedX(true);
   ROR8();
}

static void Op76M0(void)
{
   DirectIndexedX(true);
   ROR16();
}

static void Op6EM1(void)
{
   Absolute(true);
   ROR8();
}

static void Op6EM0(void)
{
   Absolute(true);
   ROR16();
}

static void Op7EM1(void)
{
   AbsoluteIndexedX(true);
   ROR8();
}

static void Op7EM0(void)
{
   AbsoluteIndexedX(true);
   ROR16();
}

/* SBC */
static void OpE9M1(void)
{
   Immediate8();
   SBC8();
}

static void OpE9M0(void)
{
   Immediate16();
   SBC16();
}

static void OpE5M1(void)
{
   Direct(true);
   SBC8();
}

static void OpE5M0(void)
{
   Direct(true);
   SBC16();
}

static void OpF5M1(void)
{
   DirectIndexedX(true);
   SBC8();
}

static void OpF5M0(void)
{
   DirectIndexedX(true);
   SBC16();
}

static void OpF2M1(void)
{
   DirectIndirect(true);
   SBC8();
}

static void OpF2M0(void)
{
   DirectIndirect(true);
   SBC16();
}

static void OpE1M1(void)
{
   DirectIndexedIndirect(true);
   SBC8();
}

static void OpE1M0(void)
{
   DirectIndexedIndirect(true);
   SBC16();
}

static void OpF1M1(void)
{
   DirectIndirectIndexed(true);
   SBC8();
}

static void OpF1M0(void)
{
   DirectIndirectIndexed(true);
   SBC16();
}

static void OpE7M1(void)
{
   DirectIndirectLong(true);
   SBC8();
}

static void OpE7M0(void)
{
   DirectIndirectLong(true);
   SBC16();
}

static void OpF7M1(void)
{
   DirectIndirectIndexedLong(true);
   SBC8();
}

static void OpF7M0(void)
{
   DirectIndirectIndexedLong(true);
   SBC16();
}

static void OpEDM1(void)
{
   Absolute(true);
   SBC8();
}

static void OpEDM0(void)
{
   Absolute(true);
   SBC16();
}

static void OpFDM1(void)
{
   AbsoluteIndexedX(true);
   SBC8();
}

static void OpFDM0(void)
{
   AbsoluteIndexedX(true);
   SBC16();
}

static void OpF9M1(void)
{
   AbsoluteIndexedY(true);
   SBC8();
}

static void OpF9M0(void)
{
   AbsoluteIndexedY(true);
   SBC16();
}

static void OpEFM1(void)
{
   AbsoluteLong(true);
   SBC8();
}

static void OpEFM0(void)
{
   AbsoluteLong(true);
   SBC16();
}

static void OpFFM1(void)
{
   AbsoluteLongIndexedX(true);
   SBC8();
}

static void OpFFM0(void)
{
   AbsoluteLongIndexedX(true);
   SBC16();
}

static void OpE3M1(void)
{
   StackRelative(true);
   SBC8();
}

static void OpE3M0(void)
{
   StackRelative(true);
   SBC16();
}

static void OpF3M1(void)
{
   StackRelativeIndirectIndexed(true);
   SBC8();
}

static void OpF3M0(void)
{
   StackRelativeIndirectIndexed(true);
   SBC16();
}

/* STA */
static void Op85M1(void)
{
   Direct(false);
   STA8();
}

static void Op85M0(void)
{
   Direct(false);
   STA16();
}

static void Op95M1(void)
{
   DirectIndexedX(false);
   STA8();
}

static void Op95M0(void)
{
   DirectIndexedX(false);
   STA16();
}

static void Op92M1(void)
{
   DirectIndirect(false);
   STA8();
}

static void Op92M0(void)
{
   DirectIndirect(false);
   STA16();
}

static void Op81M1(void)
{
   DirectIndexedIndirect(false);
   STA8();
#ifndef SA1_OPCODES
   if (CheckIndex())
      CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op81M0(void)
{
   DirectIndexedIndirect(false);
   STA16();
#ifndef SA1_OPCODES
   if (CheckIndex())
      CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op91M1(void)
{
   DirectIndirectIndexed(false);
   STA8();
}

static void Op91M0(void)
{
   DirectIndirectIndexed(false);
   STA16();
}

static void Op87M1(void)
{
   DirectIndirectLong(false);
   STA8();
}

static void Op87M0(void)
{
   DirectIndirectLong(false);
   STA16();
}

static void Op97M1(void)
{
   DirectIndirectIndexedLong(false);
   STA8();
}

static void Op97M0(void)
{
   DirectIndirectIndexedLong(false);
   STA16();
}

static void Op8DM1(void)
{
   Absolute(false);
   STA8();
}

static void Op8DM0(void)
{
   Absolute(false);
   STA16();
}

static void Op9DM1(void)
{
   AbsoluteIndexedX(false);
   STA8();
}

static void Op9DM0(void)
{
   AbsoluteIndexedX(false);
   STA16();
}

static void Op99M1(void)
{
   AbsoluteIndexedY(false);
   STA8();
}

static void Op99M0(void)
{
   AbsoluteIndexedY(false);
   STA16();
}

static void Op8FM1(void)
{
   AbsoluteLong(false);
   STA8();
}

static void Op8FM0(void)
{
   AbsoluteLong(false);
   STA16();
}

static void Op9FM1(void)
{
   AbsoluteLongIndexedX(false);
   STA8();
}

static void Op9FM0(void)
{
   AbsoluteLongIndexedX(false);
   STA16();
}

static void Op83M1(void)
{
   StackRelative(false);
   STA8();
}

static void Op83M0(void)
{
   StackRelative(false);
   STA16();
}

static void Op93M1(void)
{
   StackRelativeIndirectIndexed(false);
   STA8();
}

static void Op93M0(void)
{
   StackRelativeIndirectIndexed(false);
   STA16();
}

/* STX */
static void Op86X1(void)
{
   Direct(false);
   STX8();
}

static void Op86X0(void)
{
   Direct(false);
   STX16();
}

static void Op96X1(void)
{
   DirectIndexedY(false);
   STX8();
}

static void Op96X0(void)
{
   DirectIndexedY(false);
   STX16();
}

static void Op8EX1(void)
{
   Absolute(false);
   STX8();
}

static void Op8EX0(void)
{
   Absolute(false);
   STX16();
}

/* STY */
static void Op84X1(void)
{
   Direct(false);
   STY8();
}

static void Op84X0(void)
{
   Direct(false);
   STY16();
}

static void Op94X1(void)
{
   DirectIndexedX(false);
   STY8();
}

static void Op94X0(void)
{
   DirectIndexedX(false);
   STY16();
}

static void Op8CX1(void)
{
   Absolute(false);
   STY8();
}

static void Op8CX0(void)
{
   Absolute(false);
   STY16();
}

/* STZ */
static void Op64M1(void)
{
   Direct(false);
   STZ8();
}

static void Op64M0(void)
{
   Direct(false);
   STZ16();
}

static void Op74M1(void)
{
   DirectIndexedX(false);
   STZ8();
}

static void Op74M0(void)
{
   DirectIndexedX(false);
   STZ16();
}

static void Op9CM1(void)
{
   Absolute(false);
   STZ8();
}

static void Op9CM0(void)
{
   Absolute(false);
   STZ16();
}

static void Op9EM1(void)
{
   AbsoluteIndexedX(false);
   STZ8();
}

static void Op9EM0(void)
{
   AbsoluteIndexedX(false);
   STZ16();
}

/* TRB */
static void Op14M1(void)
{
   Direct(true);
   TRB8();
}

static void Op14M0(void)
{
   Direct(true);
   TRB16();
}

static void Op1CM1(void)
{
   Absolute(true);
   TRB8();
}

static void Op1CM0(void)
{
   Absolute(true);
   TRB16();
}

/* TSB */
static void Op04M1(void)
{
   Direct(true);
   TSB8();
}

static void Op04M0(void)
{
   Direct(true);
   TSB16();
}

static void Op0CM1(void)
{
   Absolute(true);
   TSB8();
}

static void Op0CM0(void)
{
   Absolute(true);
   TSB16();
}

/* Branch Instructions */
#ifndef SA1_OPCODES
#define BranchCheck() \
   if(CPU.BranchSkip) \
   { \
      CPU.BranchSkip = false; \
      if (CPU.PC - CPU.PCBase > OpAddress)\
         return; \
   }
#else
#define BranchCheck()
#endif

#ifndef SA1_OPCODES
static INLINE void CPUShutdown(void)
{
   if (Settings.Shutdown && CPU.PC == CPU.WaitAddress)
   {
      /* Don't skip cycles with a pending NMI or IRQ - could cause delayed
       * interrupt. Interrupts are delayed for a few cycles already, but
       * the delay could allow the shutdown code to cycle skip again.
       * Was causing screen flashing on Top Gear 3000. */
      if (CPU.WaitCounter == 0 && !(CPU.Flags & (IRQ_PENDING_FLAG | NMI_FLAG)))
      {
         CPU.WaitAddress = NULL;
#ifndef USE_BLARGG_APU
         CPU.Cycles = CPU.NextEvent;
         if (IAPU.APUExecuting)
         {
            ICPU.CPUExecuting = false;
            do
            {
               APU_EXECUTE1();
            } while (APU.Cycles < CPU.NextEvent);
            ICPU.CPUExecuting = true;
         }
#endif
      }
      else if (CPU.WaitCounter >= 2)
         CPU.WaitCounter = 1;
      else
         CPU.WaitCounter--;
   }
}
#else
static INLINE void CPUShutdown(void)
{
   if (Settings.Shutdown && CPU.PC == CPU.WaitAddress)
   {
      if (CPU.WaitCounter >= 1)
      {
         SA1.Executing = false;
         SA1.CPUExecuting = false;
      }
      else
         CPU.WaitCounter++;
   }
}
#endif

/* From the speed-hacks branch of CatSFC */
static INLINE void ForceShutdown(void)
{
#ifndef SA1_OPCODES
   CPU.WaitAddress = NULL;
#ifndef USE_BLARGG_APU
   CPU.Cycles = CPU.NextEvent;
   if (IAPU.APUExecuting)
   {
      ICPU.CPUExecuting = false;
      do
      {
         APU_EXECUTE1();
      } while (APU.Cycles < CPU.NextEvent);
      ICPU.CPUExecuting = true;
   }
#endif
#else
   SA1.Executing = false;
   SA1.CPUExecuting = false;
#endif
}

/* BCC */
static void Op90(void)
{
   Relative();
   BranchCheck();
   if (!CheckCarry())
   {
      CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
      CPU.Cycles += ONE_CYCLE;
#endif
      CPUShutdown();
   }
}

/* BCS */
static void OpB0(void)
{
   Relative();
   BranchCheck();
   if (CheckCarry())
   {
      CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
      CPU.Cycles += ONE_CYCLE;
#endif
      CPUShutdown();
   }
}

/* BEQ */
static void OpF0(void)
{
   Relative();
   BranchCheck();
   if (CheckZero())
   {
      CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
      CPU.Cycles += ONE_CYCLE;
#endif
      CPUShutdown();
   }
}

/* BMI */
static void Op30(void)
{
   Relative();
   BranchCheck();
   if (CheckNegative())
   {
      CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
      CPU.Cycles += ONE_CYCLE;
#endif
      CPUShutdown();
   }
}

/* BNE */
static void OpD0(void)
{
   Relative();
   BranchCheck();
   if (!CheckZero())
   {
      CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
      CPU.Cycles += ONE_CYCLE;
#endif
      CPUShutdown();
   }
}

/* BPL */
static void Op10(void)
{
   Relative();
   BranchCheck();
   if (!CheckNegative())
   {
      CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
      CPU.Cycles += ONE_CYCLE;
#endif
      CPUShutdown();
   }
}

/* BRA */
static void Op80(void)
{
   Relative();
   CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPUShutdown();
}

/* BVC */
static void Op50(void)
{
   Relative();
   BranchCheck();
   if (!CheckOverflow())
   {
      CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
      CPU.Cycles += ONE_CYCLE;
#endif
      CPUShutdown();
   }
}

/* BVS */
static void Op70(void)
{
   Relative();
   BranchCheck();
   if (CheckOverflow())
   {
      CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
      CPU.Cycles += ONE_CYCLE;
#endif
      CPUShutdown();
   }
}

/* ClearFlag Instructions */
/* CLC */
static void Op18(void)
{
   ClearCarry();
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* CLD */
static void OpD8(void)
{
   ClearDecimal();
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* CLI */
static void Op58(void)
{
   ClearIRQ();
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* CLV */
static void OpB8(void)
{
   ClearOverflow();
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* DEX/DEY */
static void OpCAX1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   ICPU.Registers.XL--;
   SetZN8(ICPU.Registers.XL);
}

static void OpCAX0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   ICPU.Registers.X.W--;
   SetZN16(ICPU.Registers.X.W);
}

static void Op88X1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   ICPU.Registers.YL--;
   SetZN8(ICPU.Registers.YL);
}

static void Op88X0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   ICPU.Registers.Y.W--;
   SetZN16(ICPU.Registers.Y.W);
}

/* INX/INY */
static void OpE8X1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   ICPU.Registers.XL++;
   SetZN8(ICPU.Registers.XL);
}

static void OpE8X0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   ICPU.Registers.X.W++;
   SetZN16(ICPU.Registers.X.W);
}

static void OpC8X1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   ICPU.Registers.YL++;
   SetZN8(ICPU.Registers.YL);
}

static void OpC8X0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   CPU.WaitAddress = NULL;
   ICPU.Registers.Y.W++;
   SetZN16(ICPU.Registers.Y.W);
}

/* NOP */
static void OpEA(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* PUSH Instructions */
#define PushB(b)\
   S9xSetByte(b, ICPU.Registers.S.W--);

#define PushBE(b)\
   PushB(b);\
   ICPU.Registers.SH = 0x01

#define PushW(w)\
   S9xSetByte((w) >> 8, ICPU.Registers.S.W);\
   S9xSetByte((w) & 0xff, (ICPU.Registers.S.W - 1) & 0xffff);\
   ICPU.Registers.S.W -= 2

#define PushWE(w)\
   PushW(w); \
   ICPU.Registers.SH = 0x01

/* PEA NL */
static void OpF4E1(void)
{
   Absolute(false);
   PushWE((uint16_t)OpAddress);
}

static void OpF4(void)
{
   Absolute(false);
   PushW((uint16_t)OpAddress);
}

/* PEI NL */
static void OpD4E1(void)
{
   DirectIndirect(false);
   PushWE((uint16_t)OpAddress);
}

static void OpD4(void)
{
   DirectIndirect(false);
   PushW((uint16_t)OpAddress);
}

/* PER NL */
static void Op62E1(void)
{
   RelativeLong();
   PushWE((uint16_t)OpAddress);
}

static void Op62(void)
{
   RelativeLong();
   PushW((uint16_t)OpAddress);
}

/* PHA */
static void Op48E1(void)
{
   PushBE(ICPU.Registers.AL);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op48M1(void)
{
   PushB(ICPU.Registers.AL);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op48M0(void)
{
   PushW(ICPU.Registers.A.W);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* PHB */
static void Op8BE1(void)
{
   PushBE(ICPU.Registers.DB);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op8B(void)
{
   PushB(ICPU.Registers.DB);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* PHD NL */
static void Op0BE1(void)
{
   PushWE(ICPU.Registers.D.W);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op0B(void)
{
   PushW(ICPU.Registers.D.W);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* PHK */
static void Op4BE1(void)
{
   PushBE(ICPU.Registers.PB);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op4B(void)
{
   PushB(ICPU.Registers.PB);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* PHP */
static void Op08E1(void)
{
   S9xPackStatus();
   PushBE(ICPU.Registers.PL);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op08(void)
{
   S9xPackStatus();
   PushB(ICPU.Registers.PL);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* PHX */
static void OpDAE1(void)
{
   PushBE(ICPU.Registers.XL);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

static void OpDAX1(void)
{
   PushB(ICPU.Registers.XL);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

static void OpDAX0(void)
{
   PushW(ICPU.Registers.X.W);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* PHY */
static void Op5AE1(void)
{
   PushBE(ICPU.Registers.YL);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op5AX1(void)
{
   PushB(ICPU.Registers.YL);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op5AX0(void)
{
   PushW(ICPU.Registers.Y.W);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* PULL Instructions */
#define PullB(b)\
   b = S9xGetByte (++ICPU.Registers.S.W)

#define PullBE(b)\
   PullB(b);\
   ICPU.Registers.SH = 0x01

#define PullW(w)\
   w = S9xGetByte(++ICPU.Registers.S.W);\
   w |= (S9xGetByte(++ICPU.Registers.S.W) << 8)

#define PullWE(w)\
   PullW(w);\
   ICPU.Registers.SH = 0x01

/* PLA */
static void Op68E1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullBE(ICPU.Registers.AL);
   SetZN8(ICPU.Registers.AL);
}

static void Op68M1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullB(ICPU.Registers.AL);
   SetZN8(ICPU.Registers.AL);
}

static void Op68M0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullW(ICPU.Registers.A.W);
   SetZN16(ICPU.Registers.A.W);
}

/* PLB */
static void OpABE1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullBE(ICPU.Registers.DB);
   SetZN8(ICPU.Registers.DB);
   ICPU.ShiftedDB = ICPU.Registers.DB << 16;
}

static void OpAB(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullB(ICPU.Registers.DB);
   SetZN8(ICPU.Registers.DB);
   ICPU.ShiftedDB = ICPU.Registers.DB << 16;
}

/* PLD NL */
static void Op2BE1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullWE(ICPU.Registers.D.W);
   SetZN16(ICPU.Registers.D.W);
}

static void Op2B(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullW(ICPU.Registers.D.W);
   SetZN16(ICPU.Registers.D.W);
}

/* PLP */
static void Op28E1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullBE(ICPU.Registers.PL);
   S9xUnpackStatus();

   if (CheckIndex())
   {
      ICPU.Registers.XH = 0;
      ICPU.Registers.YH = 0;
   }
   S9xFixCycles();
}

static void Op28(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullB(ICPU.Registers.PL);
   S9xUnpackStatus();

   if (CheckIndex())
   {
      ICPU.Registers.XH = 0;
      ICPU.Registers.YH = 0;
   }
   S9xFixCycles();
}

/* PLX */
static void OpFAE1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullBE(ICPU.Registers.XL);
   SetZN8(ICPU.Registers.XL);
}

static void OpFAX1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullB(ICPU.Registers.XL);
   SetZN8(ICPU.Registers.XL);
}

static void OpFAX0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullW(ICPU.Registers.X.W);
   SetZN16(ICPU.Registers.X.W);
}

/* PLY */
static void Op7AE1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullBE(ICPU.Registers.YL);
   SetZN8(ICPU.Registers.YL);
}

static void Op7AX1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullB(ICPU.Registers.YL);
   SetZN8(ICPU.Registers.YL);
}

static void Op7AX0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   PullW(ICPU.Registers.Y.W);
   SetZN16(ICPU.Registers.Y.W);
}

/* SEC */
static void Op38(void)
{
   SetCarry();
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* SED */
static void OpF8(void)
{
   SetDecimal();
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* SEI */
static void Op78(void)
{
   SetIRQ();
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* TAX8 */
static void OpAAX1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.XL = ICPU.Registers.AL;
   SetZN8(ICPU.Registers.XL);
}

/* TAX16 */
static void OpAAX0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.X.W = ICPU.Registers.A.W;
   SetZN16(ICPU.Registers.X.W);
}

/* TAY8 */
static void OpA8X1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.YL = ICPU.Registers.AL;
   SetZN8(ICPU.Registers.YL);
}

/* TAY16 */
static void OpA8X0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.Y.W = ICPU.Registers.A.W;
   SetZN16(ICPU.Registers.Y.W);
}

static void Op5B(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.D.W = ICPU.Registers.A.W;
   SetZN16(ICPU.Registers.D.W);
}

static void Op1B(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.S.W = ICPU.Registers.A.W;
   if (CheckEmulation())
      ICPU.Registers.SH = 1;
}

static void Op7B(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.A.W = ICPU.Registers.D.W;
   SetZN16(ICPU.Registers.A.W);
}

static void Op3B(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.A.W = ICPU.Registers.S.W;
   SetZN16(ICPU.Registers.A.W);
}

static void OpBAX1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.XL = ICPU.Registers.SL;
   SetZN8(ICPU.Registers.XL);
}

static void OpBAX0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.X.W = ICPU.Registers.S.W;
   SetZN16(ICPU.Registers.X.W);
}

static void Op8AM1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.AL = ICPU.Registers.XL;
   SetZN8(ICPU.Registers.AL);
}

static void Op8AM0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.A.W = ICPU.Registers.X.W;
   SetZN16(ICPU.Registers.A.W);
}

static void Op9A(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.S.W = ICPU.Registers.X.W;
   if (CheckEmulation())
      ICPU.Registers.SH = 1;
}

static void Op9BX1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.YL = ICPU.Registers.XL;
   SetZN8(ICPU.Registers.YL);
}

static void Op9BX0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.Y.W = ICPU.Registers.X.W;
   SetZN16(ICPU.Registers.Y.W);
}

static void Op98M1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.AL = ICPU.Registers.YL;
   SetZN8(ICPU.Registers.AL);
}

static void Op98M0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.A.W = ICPU.Registers.Y.W;
   SetZN16(ICPU.Registers.A.W);
}

static void OpBBX1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.XL = ICPU.Registers.YL;
   SetZN8(ICPU.Registers.XL);
}

static void OpBBX0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   ICPU.Registers.X.W = ICPU.Registers.Y.W;
   SetZN16(ICPU.Registers.X.W);
}

/* XCE */
static void OpFB(void)
{
   uint8_t A1, A2;
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
   A1 = ICPU._Carry;
   A2 = ICPU.Registers.PH;
   ICPU._Carry = A2 & 1;
   ICPU.Registers.PH = A1;

   if (CheckEmulation())
   {
      SetFlags(MemoryFlag | IndexFlag);
      ICPU.Registers.SH = 1;
   }
   if (CheckIndex())
   {
      ICPU.Registers.XH = 0;
      ICPU.Registers.YH = 0;
   }
   S9xFixCycles();
}

/* BRK */
static void Op00(void)
{
   if (!CheckEmulation())
   {
      PushB(ICPU.Registers.PB);
      PushW(CPU.PC - CPU.PCBase + 1);
      S9xPackStatus();
      PushB(ICPU.Registers.PL);
      OpenBus = ICPU.Registers.PL;
      ClearDecimal();
      SetIRQ();

      ICPU.Registers.PB = 0;
      ICPU.ShiftedPB = 0;
      S9xSetPCBase(S9xGetWord(0xFFE6));
#ifndef SA1_OPCODES
      CPU.Cycles += TWO_CYCLES;
#endif
   }
   else
   {
      PushW(CPU.PC - CPU.PCBase);
      S9xPackStatus();
      PushB(ICPU.Registers.PL);
      OpenBus = ICPU.Registers.PL;
      ClearDecimal();
      SetIRQ();

      ICPU.Registers.PB = 0;
      ICPU.ShiftedPB = 0;
      S9xSetPCBase(S9xGetWord(0xFFFE));
#ifndef SA1_OPCODES
      CPU.Cycles += ONE_CYCLE;
#endif
   }
}

/* BRL */
static void Op82(void)
{
   RelativeLong();
   S9xSetPCBase(ICPU.ShiftedPB + OpAddress);
}

/* IRQ */
void S9xOpcode_IRQ(void)
{
   if (!CheckEmulation())
   {
      PushB(ICPU.Registers.PB);
      PushW(CPU.PC - CPU.PCBase);
      S9xPackStatus();
      PushB(ICPU.Registers.PL);
      OpenBus = ICPU.Registers.PL;
      ClearDecimal();
      SetIRQ();

      ICPU.Registers.PB = 0;
      ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
      S9xSA1SetPCBase(Memory.FillRAM [0x2207] | (Memory.FillRAM [0x2208] << 8));
#else
      if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x40))
         S9xSetPCBase(Memory.FillRAM [0x220e] | (Memory.FillRAM [0x220f] << 8));
      else
         S9xSetPCBase(S9xGetWord(0xFFEE));
      CPU.Cycles += TWO_CYCLES;
#endif
   }
   else
   {
      PushW(CPU.PC - CPU.PCBase);
      S9xPackStatus();
      PushB(ICPU.Registers.PL);
      OpenBus = ICPU.Registers.PL;
      ClearDecimal();
      SetIRQ();

      ICPU.Registers.PB = 0;
      ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
      S9xSA1SetPCBase(Memory.FillRAM [0x2207] | (Memory.FillRAM [0x2208] << 8));
#else
      if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x40))
         S9xSetPCBase(Memory.FillRAM [0x220e] | (Memory.FillRAM [0x220f] << 8));
      else
         S9xSetPCBase(S9xGetWord(0xFFFE));
      CPU.Cycles += ONE_CYCLE;
#endif
   }
}

/* NMI */
void S9xOpcode_NMI(void)
{
   if (!CheckEmulation())
   {
      PushB(ICPU.Registers.PB);
      PushW(CPU.PC - CPU.PCBase);
      S9xPackStatus();
      PushB(ICPU.Registers.PL);
      OpenBus = ICPU.Registers.PL;
      ClearDecimal();
      SetIRQ();

      ICPU.Registers.PB = 0;
      ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
      S9xSA1SetPCBase(Memory.FillRAM [0x2205] | (Memory.FillRAM [0x2206] << 8));
#else
      if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x20))
         S9xSetPCBase(Memory.FillRAM [0x220c] | (Memory.FillRAM [0x220d] << 8));
      else
         S9xSetPCBase(S9xGetWord(0xFFEA));
      CPU.Cycles += TWO_CYCLES;
#endif
   }
   else
   {
      PushW(CPU.PC - CPU.PCBase);
      S9xPackStatus();
      PushB(ICPU.Registers.PL);
      OpenBus = ICPU.Registers.PL;
      ClearDecimal();
      SetIRQ();

      ICPU.Registers.PB = 0;
      ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
      S9xSA1SetPCBase(Memory.FillRAM [0x2205] | (Memory.FillRAM [0x2206] << 8));
#else
      if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x20))
         S9xSetPCBase(Memory.FillRAM [0x220c] | (Memory.FillRAM [0x220d] << 8));
      else
         S9xSetPCBase(S9xGetWord(0xFFFA));
      CPU.Cycles += ONE_CYCLE;
#endif
   }
}

/* COP */
static void Op02(void)
{
   if (!CheckEmulation())
   {
      PushB(ICPU.Registers.PB);
      PushW(CPU.PC - CPU.PCBase + 1);
      S9xPackStatus();
      PushB(ICPU.Registers.PL);
      OpenBus = ICPU.Registers.PL;
      ClearDecimal();
      SetIRQ();

      ICPU.Registers.PB = 0;
      ICPU.ShiftedPB = 0;
      S9xSetPCBase(S9xGetWord(0xFFE4));
#ifndef SA1_OPCODES
      CPU.Cycles += TWO_CYCLES;
#endif
   }
   else
   {
      PushW(CPU.PC - CPU.PCBase);
      S9xPackStatus();
      PushB(ICPU.Registers.PL);
      OpenBus = ICPU.Registers.PL;
      ClearDecimal();
      SetIRQ();

      ICPU.Registers.PB = 0;
      ICPU.ShiftedPB = 0;
      S9xSetPCBase(S9xGetWord(0xFFF4));
#ifndef SA1_OPCODES
      CPU.Cycles += ONE_CYCLE;
#endif
   }
}

/* JML */
static void OpDC(void)
{
   AbsoluteIndirectLong(false);
   ICPU.Registers.PB = (uint8_t)(OpAddress >> 16);
   ICPU.ShiftedPB = OpAddress & 0xff0000;
   S9xSetPCBase(OpAddress);
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
}

static void Op5C(void)
{
   AbsoluteLong(false);
   ICPU.Registers.PB = (uint8_t)(OpAddress >> 16);
   ICPU.ShiftedPB = OpAddress & 0xff0000;
   S9xSetPCBase(OpAddress);
}

/* JMP */
static void Op4C(void)
{
   Absolute(false);
   S9xSetPCBase(ICPU.ShiftedPB + (OpAddress & 0xffff));
#ifdef SA1_OPCODES
   CPUShutdown();
#endif
}

static void Op6C(void)
{
   AbsoluteIndirect(false);
   S9xSetPCBase(ICPU.ShiftedPB + (OpAddress & 0xffff));
}

static void Op7C(void)
{
   AbsoluteIndexedIndirect(false);
   S9xSetPCBase(ICPU.ShiftedPB + OpAddress);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* JSL/RTL */
static void Op22E1(void)
{
   AbsoluteLong(false);
   PushB(ICPU.Registers.PB);
   PushWE(CPU.PC - CPU.PCBase - 1);
   ICPU.Registers.PB = (uint8_t)(OpAddress >> 16);
   ICPU.ShiftedPB = OpAddress & 0xff0000;
   S9xSetPCBase(OpAddress);
}

static void Op22(void)
{
   AbsoluteLong(false);
   PushB(ICPU.Registers.PB);
   PushW(CPU.PC - CPU.PCBase - 1);
   ICPU.Registers.PB = (uint8_t)(OpAddress >> 16);
   ICPU.ShiftedPB = OpAddress & 0xff0000;
   S9xSetPCBase(OpAddress);
}

static void Op6BE1(void)
{
   PullWE(ICPU.Registers.PC);
   PullB(ICPU.Registers.PB);
   ICPU.ShiftedPB = ICPU.Registers.PB << 16;
   S9xSetPCBase(ICPU.ShiftedPB + ((ICPU.Registers.PC + 1) & 0xffff));
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
}

static void Op6B(void)
{
   PullW(ICPU.Registers.PC);
   PullB(ICPU.Registers.PB);
   ICPU.ShiftedPB = ICPU.Registers.PB << 16;
   S9xSetPCBase(ICPU.ShiftedPB + ((ICPU.Registers.PC + 1) & 0xffff));
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
}

/* JSR/RTS */
static void Op20(void)
{
   Absolute(false);
   PushW(CPU.PC - CPU.PCBase - 1);
   S9xSetPCBase(ICPU.ShiftedPB + (OpAddress & 0xffff));
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

/* JSR a,x */
static void OpFCE1(void)
{
   AbsoluteIndexedIndirect(false);
   PushWE(CPU.PC - CPU.PCBase - 1);
   S9xSetPCBase(ICPU.ShiftedPB + OpAddress);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

static void OpFC(void)
{
   AbsoluteIndexedIndirect(false);
   PushW(CPU.PC - CPU.PCBase - 1);
   S9xSetPCBase(ICPU.ShiftedPB + OpAddress);
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op60(void)
{
   PullW(ICPU.Registers.PC);
   S9xSetPCBase(ICPU.ShiftedPB + ((ICPU.Registers.PC + 1) & 0xffff));
#ifndef SA1_OPCODES
   CPU.Cycles += ONE_CYCLE * 3;
#endif
}

/* MVN/MVP */
static void Op54X1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2 + TWO_CYCLES;
#endif

   ICPU.Registers.DB = *CPU.PC++;
   ICPU.ShiftedDB = ICPU.Registers.DB << 16;
   OpenBus = *CPU.PC++;

   S9xSetByte(S9xGetByte((OpenBus << 16) + ICPU.Registers.X.W), ICPU.ShiftedDB + ICPU.Registers.Y.W);

   ICPU.Registers.XL++;
   ICPU.Registers.YL++;
   ICPU.Registers.A.W--;
   if (ICPU.Registers.A.W != 0xffff)
      CPU.PC -= 3;
}

static void Op54X0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2 + TWO_CYCLES;
#endif

   ICPU.Registers.DB = *CPU.PC++;
   ICPU.ShiftedDB = ICPU.Registers.DB << 16;
   OpenBus = *CPU.PC++;

   S9xSetByte(S9xGetByte((OpenBus << 16) + ICPU.Registers.X.W), ICPU.ShiftedDB + ICPU.Registers.Y.W);

   ICPU.Registers.X.W++;
   ICPU.Registers.Y.W++;
   ICPU.Registers.A.W--;
   if (ICPU.Registers.A.W != 0xffff)
      CPU.PC -= 3;
}

static void Op44X1(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2 + TWO_CYCLES;
#endif
   ICPU.Registers.DB = *CPU.PC++;
   ICPU.ShiftedDB = ICPU.Registers.DB << 16;
   OpenBus = *CPU.PC++;
   S9xSetByte(S9xGetByte((OpenBus << 16) + ICPU.Registers.X.W), ICPU.ShiftedDB + ICPU.Registers.Y.W);

   ICPU.Registers.XL--;
   ICPU.Registers.YL--;
   ICPU.Registers.A.W--;
   if (ICPU.Registers.A.W != 0xffff)
      CPU.PC -= 3;
}

static void Op44X0(void)
{
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeedx2 + TWO_CYCLES;
#endif
   ICPU.Registers.DB = *CPU.PC++;
   ICPU.ShiftedDB = ICPU.Registers.DB << 16;
   OpenBus = *CPU.PC++;
   S9xSetByte(S9xGetByte((OpenBus << 16) + ICPU.Registers.X.W), ICPU.ShiftedDB + ICPU.Registers.Y.W);

   ICPU.Registers.X.W--;
   ICPU.Registers.Y.W--;
   ICPU.Registers.A.W--;
   if (ICPU.Registers.A.W != 0xffff)
      CPU.PC -= 3;
}

/* REP/SEP */
static void OpC2(void)
{
   uint8_t Work8 = ~*CPU.PC++;
   ICPU.Registers.PL &= Work8;
   ICPU._Carry &= Work8;
   ICPU._Overflow &= (Work8 >> 6);
   ICPU._Negative &= Work8;
   ICPU._Zero |= ~Work8 & Zero;

#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeed + ONE_CYCLE;
#endif
   if (CheckEmulation())
      SetFlags(MemoryFlag | IndexFlag);
   if (CheckIndex())
   {
      ICPU.Registers.XH = 0;
      ICPU.Registers.YH = 0;
   }
   S9xFixCycles();
}

static void OpE2(void)
{
   uint8_t Work8 = *CPU.PC++;
   ICPU.Registers.PL |= Work8;
   ICPU._Carry |= Work8 & 1;
   ICPU._Overflow |= (Work8 >> 6) & 1;
   ICPU._Negative |= Work8;
   if (Work8 & Zero)
      ICPU._Zero = 0;
#ifndef SA1_OPCODES
   CPU.Cycles += CPU.MemSpeed + ONE_CYCLE;
#endif
   if (CheckEmulation())
      SetFlags(MemoryFlag | IndexFlag);
   if (CheckIndex())
   {
      ICPU.Registers.XH = 0;
      ICPU.Registers.YH = 0;
   }
   S9xFixCycles();
}

/* XBA */
static void OpEB(void)
{
   uint8_t Work8 = ICPU.Registers.AL;
   ICPU.Registers.AL = ICPU.Registers.AH;
   ICPU.Registers.AH = Work8;
   SetZN8(ICPU.Registers.AL);
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
}

/* RTI */
static void Op40(void)
{
   PullB(ICPU.Registers.PL);
   S9xUnpackStatus();
   PullW(ICPU.Registers.PC);
   if (!CheckEmulation())
   {
      PullB(ICPU.Registers.PB);
      ICPU.ShiftedPB = ICPU.Registers.PB << 16;
   }
   else
      SetFlags(MemoryFlag | IndexFlag);
   S9xSetPCBase(ICPU.ShiftedPB + ICPU.Registers.PC);
   if (CheckIndex())
   {
      ICPU.Registers.XH = 0;
      ICPU.Registers.YH = 0;
   }
#ifndef SA1_OPCODES
   CPU.Cycles += TWO_CYCLES;
#endif
   S9xFixCycles();
}

/* WAI */
static void OpCB(void)
{
#ifdef SA1_OPCODES
   SA1.WaitingForInterrupt = true;
   SA1.PC--;
#else /* SA_OPCODES */
   CPU.WaitingForInterrupt = true;
   CPU.PC--;
   if (Settings.Shutdown)
   {
      CPU.Cycles = CPU.NextEvent;
#ifndef USE_BLARGG_APU
      if (IAPU.APUExecuting)
      {
         ICPU.CPUExecuting = false;
         do
         {
            APU_EXECUTE1();
         }
         while (APU.Cycles < CPU.NextEvent);
         ICPU.CPUExecuting = true;
      }
#endif
   }
#endif
}

/* Usually an STP opcode; SNESAdvance speed hack, not implemented in Snes9xTYL | Snes9x-Euphoria (from the speed-hacks branch of CatSFC) */
static void OpDB(void)
{
#ifndef NO_SPEEDHACKS
   int8_t BranchOffset;
   uint8_t NextByte = *CPU.PC++;

   ForceShutdown();

   BranchOffset = (NextByte & 0x7F) | ((NextByte & 0x40) << 1);
   /* ^ -64 .. +63, sign extend bit 6 into 7 for unpacking */
   OpAddress = ((int32_t) (CPU.PC - CPU.PCBase) + BranchOffset) & 0xffff;

   switch (NextByte & 0x80)
   {
   case 0x00: /* BNE */
      BranchCheck();
      if (!CheckZero ())
      {
         CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
         CPU.Cycles += ONE_CYCLE;
#endif
         CPUShutdown ();
      }
      return;
   case 0x80: /* BEQ */
      BranchCheck();
      if (CheckZero ())
      {
         CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
         CPU.Cycles += ONE_CYCLE;
#endif
         CPUShutdown ();
      }
      return;
   }
#else
     CPU.PC--;
     CPU.Flags |= DEBUG_MODE_FLAG;
#endif
}

/* SNESAdvance speed hack, as implemented in Snes9xTYL / Snes9x-Euphoria (from the speed-hacks branch of CatSFC) */
static void Op42(void)
{
#ifndef NO_SPEEDHACKS
   int8_t BranchOffset;
   uint8_t NextByte = *CPU.PC++;

   ForceShutdown();

   BranchOffset = 0xF0 | (NextByte & 0xF); /* always negative */
   OpAddress = ((int32_t) (CPU.PC - CPU.PCBase) + BranchOffset) & 0xffff;

   switch (NextByte & 0xF0)
   {
   case 0x10: /* BPL */
      BranchCheck();
      if (!CheckNegative ())
      {
         CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
         CPU.Cycles += ONE_CYCLE;
#endif
         CPUShutdown ();
      }
      return;
   case 0x30: /* BMI */
      BranchCheck();
      if (CheckNegative ())
      {
         CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
         CPU.Cycles += ONE_CYCLE;
#endif
         CPUShutdown ();
      }
      return;
   case 0x50: /* BVC */
      BranchCheck();
      if (!CheckOverflow ())
      {
         CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
         CPU.Cycles += ONE_CYCLE;
#endif
         CPUShutdown ();
      }
      return;
   case 0x70: /* BVS */
      BranchCheck();
      if (CheckOverflow ())
      {
         CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
         CPU.Cycles += ONE_CYCLE;
#endif
         CPUShutdown ();
      }
      return;
   case 0x80: /* BRA */
      CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
      CPU.Cycles += ONE_CYCLE;
#endif
      CPUShutdown ();
      return;
   case 0x90: /* BCC */
      BranchCheck();
      if (!CheckCarry ())
      {
         CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
         CPU.Cycles += ONE_CYCLE;
#endif
         CPUShutdown ();
      }
      return;
   case 0xB0: /* BCS */
      BranchCheck();
      if (CheckCarry ())
      {
         CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
         CPU.Cycles += ONE_CYCLE;
#endif
         CPUShutdown ();
      }
      return;
   case 0xD0: /* BNE */
      BranchCheck();
      if (!CheckZero ())
      {
         CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
         CPU.Cycles += ONE_CYCLE;
#endif
         CPUShutdown ();
      }
      return;
   case 0xF0: /* BEQ */
      BranchCheck();
      if (CheckZero ())
      {
         CPU.PC = CPU.PCBase + OpAddress;
#ifndef SA1_OPCODES
         CPU.Cycles += ONE_CYCLE;
#endif
         CPUShutdown ();
      }
      return;
   }
#endif
}

/* CPU-S9xOpcodes Definitions */
const SOpcodes S9xOpcodesM1X1[256] =
{
   {Op00},      {Op01M1},    {Op02},      {Op03M1},    {Op04M1},
   {Op05M1},    {Op06M1},    {Op07M1},    {Op08},      {Op09M1},
   {Op0AM1},    {Op0B},      {Op0CM1},    {Op0DM1},    {Op0EM1},
   {Op0FM1},    {Op10},      {Op11M1},    {Op12M1},    {Op13M1},
   {Op14M1},    {Op15M1},    {Op16M1},    {Op17M1},    {Op18},
   {Op19M1},    {Op1AM1},    {Op1B},      {Op1CM1},    {Op1DM1},
   {Op1EM1},    {Op1FM1},    {Op20},      {Op21M1},    {Op22},
   {Op23M1},    {Op24M1},    {Op25M1},    {Op26M1},    {Op27M1},
   {Op28},      {Op29M1},    {Op2AM1},    {Op2B},      {Op2CM1},
   {Op2DM1},    {Op2EM1},    {Op2FM1},    {Op30},      {Op31M1},
   {Op32M1},    {Op33M1},    {Op34M1},    {Op35M1},    {Op36M1},
   {Op37M1},    {Op38},      {Op39M1},    {Op3AM1},    {Op3B},
   {Op3CM1},    {Op3DM1},    {Op3EM1},    {Op3FM1},    {Op40},
   {Op41M1},    {Op42},      {Op43M1},    {Op44X1},    {Op45M1},
   {Op46M1},    {Op47M1},    {Op48M1},    {Op49M1},    {Op4AM1},
   {Op4B},      {Op4C},      {Op4DM1},    {Op4EM1},    {Op4FM1},
   {Op50},      {Op51M1},    {Op52M1},    {Op53M1},    {Op54X1},
   {Op55M1},    {Op56M1},    {Op57M1},    {Op58},      {Op59M1},
   {Op5AX1},    {Op5B},      {Op5C},      {Op5DM1},    {Op5EM1},
   {Op5FM1},    {Op60},      {Op61M1},    {Op62},      {Op63M1},
   {Op64M1},    {Op65M1},    {Op66M1},    {Op67M1},    {Op68M1},
   {Op69M1},    {Op6AM1},    {Op6B},      {Op6C},      {Op6DM1},
   {Op6EM1},    {Op6FM1},    {Op70},      {Op71M1},    {Op72M1},
   {Op73M1},    {Op74M1},    {Op75M1},    {Op76M1},    {Op77M1},
   {Op78},      {Op79M1},    {Op7AX1},    {Op7B},      {Op7C},
   {Op7DM1},    {Op7EM1},    {Op7FM1},    {Op80},      {Op81M1},
   {Op82},      {Op83M1},    {Op84X1},    {Op85M1},    {Op86X1},
   {Op87M1},    {Op88X1},    {Op89M1},    {Op8AM1},    {Op8B},
   {Op8CX1},    {Op8DM1},    {Op8EX1},    {Op8FM1},    {Op90},
   {Op91M1},    {Op92M1},    {Op93M1},    {Op94X1},    {Op95M1},
   {Op96X1},    {Op97M1},    {Op98M1},    {Op99M1},    {Op9A},
   {Op9BX1},    {Op9CM1},    {Op9DM1},    {Op9EM1},    {Op9FM1},
   {OpA0X1},    {OpA1M1},    {OpA2X1},    {OpA3M1},    {OpA4X1},
   {OpA5M1},    {OpA6X1},    {OpA7M1},    {OpA8X1},    {OpA9M1},
   {OpAAX1},    {OpAB},      {OpACX1},    {OpADM1},    {OpAEX1},
   {OpAFM1},    {OpB0},      {OpB1M1},    {OpB2M1},    {OpB3M1},
   {OpB4X1},    {OpB5M1},    {OpB6X1},    {OpB7M1},    {OpB8},
   {OpB9M1},    {OpBAX1},    {OpBBX1},    {OpBCX1},    {OpBDM1},
   {OpBEX1},    {OpBFM1},    {OpC0X1},    {OpC1M1},    {OpC2},
   {OpC3M1},    {OpC4X1},    {OpC5M1},    {OpC6M1},    {OpC7M1},
   {OpC8X1},    {OpC9M1},    {OpCAX1},    {OpCB},      {OpCCX1},
   {OpCDM1},    {OpCEM1},    {OpCFM1},    {OpD0},      {OpD1M1},
   {OpD2M1},    {OpD3M1},    {OpD4},      {OpD5M1},    {OpD6M1},
   {OpD7M1},    {OpD8},      {OpD9M1},    {OpDAX1},    {OpDB},
   {OpDC},      {OpDDM1},    {OpDEM1},    {OpDFM1},    {OpE0X1},
   {OpE1M1},    {OpE2},      {OpE3M1},    {OpE4X1},    {OpE5M1},
   {OpE6M1},    {OpE7M1},    {OpE8X1},    {OpE9M1},    {OpEA},
   {OpEB},      {OpECX1},    {OpEDM1},    {OpEEM1},    {OpEFM1},
   {OpF0},      {OpF1M1},    {OpF2M1},    {OpF3M1},    {OpF4},
   {OpF5M1},    {OpF6M1},    {OpF7M1},    {OpF8},      {OpF9M1},
   {OpFAX1},    {OpFB},      {OpFC},      {OpFDM1},    {OpFEM1},
   {OpFFM1}
};

const SOpcodes S9xOpcodesE1[256] =
{
    {Op00},	     {Op01M1},    {Op02},      {Op03M1},    {Op04M1},
    {Op05M1},    {Op06M1},    {Op07M1},    {Op08E1},    {Op09M1},
    {Op0AM1},    {Op0BE1},    {Op0CM1},    {Op0DM1},    {Op0EM1},
    {Op0FM1},    {Op10},      {Op11M1},    {Op12M1},    {Op13M1},
    {Op14M1},    {Op15M1},    {Op16M1},    {Op17M1},    {Op18},
    {Op19M1},    {Op1AM1},    {Op1B},      {Op1CM1},    {Op1DM1},
    {Op1EM1},    {Op1FM1},    {Op20},      {Op21M1},    {Op22E1},
    {Op23M1},    {Op24M1},    {Op25M1},    {Op26M1},    {Op27M1},
    {Op28},      {Op29M1},    {Op2AM1},    {Op2BE1},    {Op2CM1},
    {Op2DM1},    {Op2EM1},    {Op2FM1},    {Op30},      {Op31M1},
    {Op32M1},    {Op33M1},    {Op34M1},    {Op35M1},    {Op36M1},
    {Op37M1},    {Op38},      {Op39M1},    {Op3AM1},    {Op3B},
    {Op3CM1},    {Op3DM1},    {Op3EM1},    {Op3FM1},    {Op40},
    {Op41M1},    {Op42},      {Op43M1},    {Op44X1},    {Op45M1},
    {Op46M1},    {Op47M1},    {Op48E1},    {Op49M1},    {Op4AM1},
    {Op4BE1},    {Op4C},      {Op4DM1},    {Op4EM1},    {Op4FM1},
    {Op50},      {Op51M1},    {Op52M1},    {Op53M1},    {Op54X1},
    {Op55M1},    {Op56M1},    {Op57M1},    {Op58},      {Op59M1},
    {Op5AE1},    {Op5B},      {Op5C},      {Op5DM1},    {Op5EM1},
    {Op5FM1},    {Op60},      {Op61M1},    {Op62E1},    {Op63M1},
    {Op64M1},    {Op65M1},    {Op66M1},    {Op67M1},    {Op68E1},
    {Op69M1},    {Op6AM1},    {Op6BE1},    {Op6C},      {Op6DM1},
    {Op6EM1},    {Op6FM1},    {Op70},      {Op71M1},    {Op72M1},
    {Op73M1},    {Op74M1},    {Op75M1},    {Op76M1},    {Op77M1},
    {Op78},      {Op79M1},    {Op7AE1},    {Op7B},      {Op7C},
    {Op7DM1},    {Op7EM1},    {Op7FM1},    {Op80},      {Op81M1},
    {Op82},      {Op83M1},    {Op84X1},    {Op85M1},    {Op86X1},
    {Op87M1},    {Op88X1},    {Op89M1},    {Op8AM1},    {Op8BE1},
    {Op8CX1},    {Op8DM1},    {Op8EX1},    {Op8FM1},    {Op90},
    {Op91M1},    {Op92M1},    {Op93M1},    {Op94X1},    {Op95M1},
    {Op96X1},    {Op97M1},    {Op98M1},    {Op99M1},    {Op9A},
    {Op9BX1},    {Op9CM1},    {Op9DM1},    {Op9EM1},    {Op9FM1},
    {OpA0X1},    {OpA1M1},    {OpA2X1},    {OpA3M1},    {OpA4X1},
    {OpA5M1},    {OpA6X1},    {OpA7M1},    {OpA8X1},    {OpA9M1},
    {OpAAX1},    {OpABE1},    {OpACX1},    {OpADM1},    {OpAEX1},
    {OpAFM1},    {OpB0},      {OpB1M1},    {OpB2M1},    {OpB3M1},
    {OpB4X1},    {OpB5M1},    {OpB6X1},    {OpB7M1},    {OpB8},
    {OpB9M1},    {OpBAX1},    {OpBBX1},    {OpBCX1},    {OpBDM1},
    {OpBEX1},    {OpBFM1},    {OpC0X1},    {OpC1M1},    {OpC2},
    {OpC3M1},    {OpC4X1},    {OpC5M1},    {OpC6M1},    {OpC7M1},
    {OpC8X1},    {OpC9M1},    {OpCAX1},    {OpCB},      {OpCCX1},
    {OpCDM1},    {OpCEM1},    {OpCFM1},    {OpD0},      {OpD1M1},
    {OpD2M1},    {OpD3M1},    {OpD4E1},    {OpD5M1},    {OpD6M1},
    {OpD7M1},    {OpD8},      {OpD9M1},    {OpDAE1},    {OpDB},
    {OpDC},      {OpDDM1},    {OpDEM1},    {OpDFM1},    {OpE0X1},
    {OpE1M1},    {OpE2},      {OpE3M1},    {OpE4X1},    {OpE5M1},
    {OpE6M1},    {OpE7M1},    {OpE8X1},    {OpE9M1},    {OpEA},
    {OpEB},      {OpECX1},    {OpEDM1},    {OpEEM1},    {OpEFM1},
    {OpF0},      {OpF1M1},    {OpF2M1},    {OpF3M1},    {OpF4E1},
    {OpF5M1},    {OpF6M1},    {OpF7M1},    {OpF8},      {OpF9M1},
    {OpFAE1},    {OpFB},      {OpFCE1},    {OpFDM1},    {OpFEM1},
    {OpFFM1}
};

const SOpcodes S9xOpcodesM1X0[256] =
{
   {Op00},  {Op01M1},    {Op02},      {Op03M1},    {Op04M1},
   {Op05M1},    {Op06M1},    {Op07M1},    {Op08},      {Op09M1},
   {Op0AM1},    {Op0B},      {Op0CM1},    {Op0DM1},    {Op0EM1},
   {Op0FM1},    {Op10},      {Op11M1},    {Op12M1},    {Op13M1},
   {Op14M1},    {Op15M1},    {Op16M1},    {Op17M1},    {Op18},
   {Op19M1},    {Op1AM1},    {Op1B},      {Op1CM1},    {Op1DM1},
   {Op1EM1},    {Op1FM1},    {Op20},      {Op21M1},    {Op22},
   {Op23M1},    {Op24M1},    {Op25M1},    {Op26M1},    {Op27M1},
   {Op28},      {Op29M1},    {Op2AM1},    {Op2B},      {Op2CM1},
   {Op2DM1},    {Op2EM1},    {Op2FM1},    {Op30},      {Op31M1},
   {Op32M1},    {Op33M1},    {Op34M1},    {Op35M1},    {Op36M1},
   {Op37M1},    {Op38},      {Op39M1},    {Op3AM1},    {Op3B},
   {Op3CM1},    {Op3DM1},    {Op3EM1},    {Op3FM1},    {Op40},
   {Op41M1},    {Op42},      {Op43M1},    {Op44X0},    {Op45M1},
   {Op46M1},    {Op47M1},    {Op48M1},    {Op49M1},    {Op4AM1},
   {Op4B},      {Op4C},      {Op4DM1},    {Op4EM1},    {Op4FM1},
   {Op50},      {Op51M1},    {Op52M1},    {Op53M1},    {Op54X0},
   {Op55M1},    {Op56M1},    {Op57M1},    {Op58},      {Op59M1},
   {Op5AX0},    {Op5B},      {Op5C},      {Op5DM1},    {Op5EM1},
   {Op5FM1},    {Op60},      {Op61M1},    {Op62},      {Op63M1},
   {Op64M1},    {Op65M1},    {Op66M1},    {Op67M1},    {Op68M1},
   {Op69M1},    {Op6AM1},    {Op6B},      {Op6C},      {Op6DM1},
   {Op6EM1},    {Op6FM1},    {Op70},      {Op71M1},    {Op72M1},
   {Op73M1},    {Op74M1},    {Op75M1},    {Op76M1},    {Op77M1},
   {Op78},      {Op79M1},    {Op7AX0},    {Op7B},      {Op7C},
   {Op7DM1},    {Op7EM1},    {Op7FM1},    {Op80},      {Op81M1},
   {Op82},      {Op83M1},    {Op84X0},    {Op85M1},    {Op86X0},
   {Op87M1},    {Op88X0},    {Op89M1},    {Op8AM1},    {Op8B},
   {Op8CX0},    {Op8DM1},    {Op8EX0},    {Op8FM1},    {Op90},
   {Op91M1},    {Op92M1},    {Op93M1},    {Op94X0},    {Op95M1},
   {Op96X0},    {Op97M1},    {Op98M1},    {Op99M1},    {Op9A},
   {Op9BX0},    {Op9CM1},    {Op9DM1},    {Op9EM1},    {Op9FM1},
   {OpA0X0},    {OpA1M1},    {OpA2X0},    {OpA3M1},    {OpA4X0},
   {OpA5M1},    {OpA6X0},    {OpA7M1},    {OpA8X0},    {OpA9M1},
   {OpAAX0},    {OpAB},      {OpACX0},    {OpADM1},    {OpAEX0},
   {OpAFM1},    {OpB0},      {OpB1M1},    {OpB2M1},    {OpB3M1},
   {OpB4X0},    {OpB5M1},    {OpB6X0},    {OpB7M1},    {OpB8},
   {OpB9M1},    {OpBAX0},    {OpBBX0},    {OpBCX0},    {OpBDM1},
   {OpBEX0},    {OpBFM1},    {OpC0X0},    {OpC1M1},    {OpC2},
   {OpC3M1},    {OpC4X0},    {OpC5M1},    {OpC6M1},    {OpC7M1},
   {OpC8X0},    {OpC9M1},    {OpCAX0},    {OpCB},      {OpCCX0},
   {OpCDM1},    {OpCEM1},    {OpCFM1},    {OpD0},      {OpD1M1},
   {OpD2M1},    {OpD3M1},    {OpD4},      {OpD5M1},    {OpD6M1},
   {OpD7M1},    {OpD8},      {OpD9M1},    {OpDAX0},    {OpDB},
   {OpDC},      {OpDDM1},    {OpDEM1},    {OpDFM1},    {OpE0X0},
   {OpE1M1},    {OpE2},      {OpE3M1},    {OpE4X0},    {OpE5M1},
   {OpE6M1},    {OpE7M1},    {OpE8X0},    {OpE9M1},    {OpEA},
   {OpEB},      {OpECX0},    {OpEDM1},    {OpEEM1},    {OpEFM1},
   {OpF0},      {OpF1M1},    {OpF2M1},    {OpF3M1},    {OpF4},
   {OpF5M1},    {OpF6M1},    {OpF7M1},    {OpF8},      {OpF9M1},
   {OpFAX0},    {OpFB},      {OpFC},      {OpFDM1},    {OpFEM1},
   {OpFFM1}
};

const SOpcodes S9xOpcodesM0X0[256] =
{
   {Op00},  {Op01M0},    {Op02},      {Op03M0},    {Op04M0},
   {Op05M0},    {Op06M0},    {Op07M0},    {Op08},      {Op09M0},
   {Op0AM0},    {Op0B},      {Op0CM0},    {Op0DM0},    {Op0EM0},
   {Op0FM0},    {Op10},      {Op11M0},    {Op12M0},    {Op13M0},
   {Op14M0},    {Op15M0},    {Op16M0},    {Op17M0},    {Op18},
   {Op19M0},    {Op1AM0},    {Op1B},      {Op1CM0},    {Op1DM0},
   {Op1EM0},    {Op1FM0},    {Op20},      {Op21M0},    {Op22},
   {Op23M0},    {Op24M0},    {Op25M0},    {Op26M0},    {Op27M0},
   {Op28},      {Op29M0},    {Op2AM0},    {Op2B},      {Op2CM0},
   {Op2DM0},    {Op2EM0},    {Op2FM0},    {Op30},      {Op31M0},
   {Op32M0},    {Op33M0},    {Op34M0},    {Op35M0},    {Op36M0},
   {Op37M0},    {Op38},      {Op39M0},    {Op3AM0},    {Op3B},
   {Op3CM0},    {Op3DM0},    {Op3EM0},    {Op3FM0},    {Op40},
   {Op41M0},    {Op42},      {Op43M0},    {Op44X0},    {Op45M0},
   {Op46M0},    {Op47M0},    {Op48M0},    {Op49M0},    {Op4AM0},
   {Op4B},      {Op4C},      {Op4DM0},    {Op4EM0},    {Op4FM0},
   {Op50},      {Op51M0},    {Op52M0},    {Op53M0},    {Op54X0},
   {Op55M0},    {Op56M0},    {Op57M0},    {Op58},      {Op59M0},
   {Op5AX0},    {Op5B},      {Op5C},      {Op5DM0},    {Op5EM0},
   {Op5FM0},    {Op60},      {Op61M0},    {Op62},      {Op63M0},
   {Op64M0},    {Op65M0},    {Op66M0},    {Op67M0},    {Op68M0},
   {Op69M0},    {Op6AM0},    {Op6B},      {Op6C},      {Op6DM0},
   {Op6EM0},    {Op6FM0},    {Op70},      {Op71M0},    {Op72M0},
   {Op73M0},    {Op74M0},    {Op75M0},    {Op76M0},    {Op77M0},
   {Op78},      {Op79M0},    {Op7AX0},    {Op7B},      {Op7C},
   {Op7DM0},    {Op7EM0},    {Op7FM0},    {Op80},      {Op81M0},
   {Op82},      {Op83M0},    {Op84X0},    {Op85M0},    {Op86X0},
   {Op87M0},    {Op88X0},    {Op89M0},    {Op8AM0},    {Op8B},
   {Op8CX0},    {Op8DM0},    {Op8EX0},    {Op8FM0},    {Op90},
   {Op91M0},    {Op92M0},    {Op93M0},    {Op94X0},    {Op95M0},
   {Op96X0},    {Op97M0},    {Op98M0},    {Op99M0},    {Op9A},
   {Op9BX0},    {Op9CM0},    {Op9DM0},    {Op9EM0},    {Op9FM0},
   {OpA0X0},    {OpA1M0},    {OpA2X0},    {OpA3M0},    {OpA4X0},
   {OpA5M0},    {OpA6X0},    {OpA7M0},    {OpA8X0},    {OpA9M0},
   {OpAAX0},    {OpAB},      {OpACX0},    {OpADM0},    {OpAEX0},
   {OpAFM0},    {OpB0},      {OpB1M0},    {OpB2M0},    {OpB3M0},
   {OpB4X0},    {OpB5M0},    {OpB6X0},    {OpB7M0},    {OpB8},
   {OpB9M0},    {OpBAX0},    {OpBBX0},    {OpBCX0},    {OpBDM0},
   {OpBEX0},    {OpBFM0},    {OpC0X0},    {OpC1M0},    {OpC2},
   {OpC3M0},    {OpC4X0},    {OpC5M0},    {OpC6M0},    {OpC7M0},
   {OpC8X0},    {OpC9M0},    {OpCAX0},    {OpCB},      {OpCCX0},
   {OpCDM0},    {OpCEM0},    {OpCFM0},    {OpD0},      {OpD1M0},
   {OpD2M0},    {OpD3M0},    {OpD4},      {OpD5M0},    {OpD6M0},
   {OpD7M0},    {OpD8},      {OpD9M0},    {OpDAX0},    {OpDB},
   {OpDC},      {OpDDM0},    {OpDEM0},    {OpDFM0},    {OpE0X0},
   {OpE1M0},    {OpE2},      {OpE3M0},    {OpE4X0},    {OpE5M0},
   {OpE6M0},    {OpE7M0},    {OpE8X0},    {OpE9M0},    {OpEA},
   {OpEB},      {OpECX0},    {OpEDM0},    {OpEEM0},    {OpEFM0},
   {OpF0},      {OpF1M0},    {OpF2M0},    {OpF3M0},    {OpF4},
   {OpF5M0},    {OpF6M0},    {OpF7M0},    {OpF8},      {OpF9M0},
   {OpFAX0},    {OpFB},      {OpFC},      {OpFDM0},    {OpFEM0},
   {OpFFM0}
};

const SOpcodes S9xOpcodesM0X1[256] =
{
   {Op00},  {Op01M0},    {Op02},      {Op03M0},    {Op04M0},
   {Op05M0},    {Op06M0},    {Op07M0},    {Op08},      {Op09M0},
   {Op0AM0},    {Op0B},      {Op0CM0},    {Op0DM0},    {Op0EM0},
   {Op0FM0},    {Op10},      {Op11M0},    {Op12M0},    {Op13M0},
   {Op14M0},    {Op15M0},    {Op16M0},    {Op17M0},    {Op18},
   {Op19M0},    {Op1AM0},    {Op1B},      {Op1CM0},    {Op1DM0},
   {Op1EM0},    {Op1FM0},    {Op20},      {Op21M0},    {Op22},
   {Op23M0},    {Op24M0},    {Op25M0},    {Op26M0},    {Op27M0},
   {Op28},      {Op29M0},    {Op2AM0},    {Op2B},      {Op2CM0},
   {Op2DM0},    {Op2EM0},    {Op2FM0},    {Op30},      {Op31M0},
   {Op32M0},    {Op33M0},    {Op34M0},    {Op35M0},    {Op36M0},
   {Op37M0},    {Op38},      {Op39M0},    {Op3AM0},    {Op3B},
   {Op3CM0},    {Op3DM0},    {Op3EM0},    {Op3FM0},    {Op40},
   {Op41M0},    {Op42},      {Op43M0},    {Op44X1},    {Op45M0},
   {Op46M0},    {Op47M0},    {Op48M0},    {Op49M0},    {Op4AM0},
   {Op4B},      {Op4C},      {Op4DM0},    {Op4EM0},    {Op4FM0},
   {Op50},      {Op51M0},    {Op52M0},    {Op53M0},    {Op54X1},
   {Op55M0},    {Op56M0},    {Op57M0},    {Op58},      {Op59M0},
   {Op5AX1},    {Op5B},      {Op5C},      {Op5DM0},    {Op5EM0},
   {Op5FM0},    {Op60},      {Op61M0},    {Op62},      {Op63M0},
   {Op64M0},    {Op65M0},    {Op66M0},    {Op67M0},    {Op68M0},
   {Op69M0},    {Op6AM0},    {Op6B},      {Op6C},      {Op6DM0},
   {Op6EM0},    {Op6FM0},    {Op70},      {Op71M0},    {Op72M0},
   {Op73M0},    {Op74M0},    {Op75M0},    {Op76M0},    {Op77M0},
   {Op78},      {Op79M0},    {Op7AX1},    {Op7B},      {Op7C},
   {Op7DM0},    {Op7EM0},    {Op7FM0},    {Op80},      {Op81M0},
   {Op82},      {Op83M0},    {Op84X1},    {Op85M0},    {Op86X1},
   {Op87M0},    {Op88X1},    {Op89M0},    {Op8AM0},    {Op8B},
   {Op8CX1},    {Op8DM0},    {Op8EX1},    {Op8FM0},    {Op90},
   {Op91M0},    {Op92M0},    {Op93M0},    {Op94X1},    {Op95M0},
   {Op96X1},    {Op97M0},    {Op98M0},    {Op99M0},    {Op9A},
   {Op9BX1},    {Op9CM0},    {Op9DM0},    {Op9EM0},    {Op9FM0},
   {OpA0X1},    {OpA1M0},    {OpA2X1},    {OpA3M0},    {OpA4X1},
   {OpA5M0},    {OpA6X1},    {OpA7M0},    {OpA8X1},    {OpA9M0},
   {OpAAX1},    {OpAB},      {OpACX1},    {OpADM0},    {OpAEX1},
   {OpAFM0},    {OpB0},      {OpB1M0},    {OpB2M0},    {OpB3M0},
   {OpB4X1},    {OpB5M0},    {OpB6X1},    {OpB7M0},    {OpB8},
   {OpB9M0},    {OpBAX1},    {OpBBX1},    {OpBCX1},    {OpBDM0},
   {OpBEX1},    {OpBFM0},    {OpC0X1},    {OpC1M0},    {OpC2},
   {OpC3M0},    {OpC4X1},    {OpC5M0},    {OpC6M0},    {OpC7M0},
   {OpC8X1},    {OpC9M0},    {OpCAX1},    {OpCB},      {OpCCX1},
   {OpCDM0},    {OpCEM0},    {OpCFM0},    {OpD0},      {OpD1M0},
   {OpD2M0},    {OpD3M0},    {OpD4},      {OpD5M0},    {OpD6M0},
   {OpD7M0},    {OpD8},      {OpD9M0},    {OpDAX1},    {OpDB},
   {OpDC},      {OpDDM0},    {OpDEM0},    {OpDFM0},    {OpE0X1},
   {OpE1M0},    {OpE2},      {OpE3M0},    {OpE4X1},    {OpE5M0},
   {OpE6M0},    {OpE7M0},    {OpE8X1},    {OpE9M0},    {OpEA},
   {OpEB},      {OpECX1},    {OpEDM0},    {OpEEM0},    {OpEFM0},
   {OpF0},      {OpF1M0},    {OpF2M0},    {OpF3M0},    {OpF4},
   {OpF5M0},    {OpF6M0},    {OpF7M0},    {OpF8},      {OpF9M0},
   {OpFAX1},    {OpFB},      {OpFC},      {OpFDM0},    {OpFEM0},
   {OpFFM0}
};
