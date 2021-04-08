/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _CPUMACRO_H_
#define _CPUMACRO_H_

#define rOP8(OP, ADDR, WRAP, FUNC) \
static void Op##OP (void) \
{ \
	uint8	val = OpenBus = S9xGetByte(ADDR(READ)); \
	FUNC##8(val); \
}

#define rOP16(OP, ADDR, WRAP, FUNC) \
static void Op##OP (void) \
{ \
	uint16	val = S9xGetWord(ADDR(READ), WRAP); \
	OpenBus = (val >> 8); \
	FUNC##16(val); \
}

#define rOPC(OP, COND, ADDR, WRAP, FUNC) \
static void Op##OP (void) \
{ \
	if (Check##COND()) \
	{ \
		uint8	val = OpenBus = S9xGetByte(ADDR(READ)); \
		FUNC##8(val); \
	} \
	else \
	{ \
		uint16	val = S9xGetWord(ADDR(READ), WRAP); \
		OpenBus = (val >> 8); \
		FUNC##16(val); \
	} \
}

#define rOPM(OP, ADDR, WRAP, FUNC) \
rOPC(OP, Memory, ADDR, WRAP, FUNC)

#define rOPX(OP, ADDR, WRAP, FUNC) \
rOPC(OP, Index, ADDR, WRAP, FUNC)

#define wOP8(OP, ADDR, WRAP, FUNC) \
static void Op##OP (void) \
{ \
	FUNC##8(ADDR(WRITE)); \
}

#define wOP16(OP, ADDR, WRAP, FUNC) \
static void Op##OP (void) \
{ \
	FUNC##16(ADDR(WRITE), WRAP); \
}

#define wOPC(OP, COND, ADDR, WRAP, FUNC) \
static void Op##OP (void) \
{ \
	if (Check##COND()) \
		FUNC##8(ADDR(WRITE)); \
	else \
		FUNC##16(ADDR(WRITE), WRAP); \
}

#define wOPM(OP, ADDR, WRAP, FUNC) \
wOPC(OP, Memory, ADDR, WRAP, FUNC)

#define wOPX(OP, ADDR, WRAP, FUNC) \
wOPC(OP, Index, ADDR, WRAP, FUNC)

#define mOP8(OP, ADDR, WRAP, FUNC) \
static void Op##OP (void) \
{ \
	FUNC##8(ADDR(MODIFY)); \
}

#define mOP16(OP, ADDR, WRAP, FUNC) \
static void Op##OP (void) \
{ \
	FUNC##16(ADDR(MODIFY), WRAP); \
}

#define mOPC(OP, COND, ADDR, WRAP, FUNC) \
static void Op##OP (void) \
{ \
	if (Check##COND()) \
		FUNC##8(ADDR(MODIFY)); \
	else \
		FUNC##16(ADDR(MODIFY), WRAP); \
}

#define mOPM(OP, ADDR, WRAP, FUNC) \
mOPC(OP, Memory, ADDR, WRAP, FUNC)

#define bOP(OP, REL, COND, CHK, E) \
static void Op##OP (void) \
{ \
	int8 offset = REL(JUMP);\
	if (COND) \
	{ \
		uint16 newPC = (int16)Registers.PCw + offset; \
		AddCycles(ONE_CYCLE); \
		if (E && Registers.PCh != (newPC >> 8)) \
			AddCycles(ONE_CYCLE); \
		if ((Registers.PCw & ~MEMMAP_MASK) != (newPC & ~MEMMAP_MASK)) \
			S9xSetPCBase(ICPU.ShiftedPB + newPC); \
		else \
			Registers.PCw = newPC; \
	} \
}


static inline void SetZN16 (uint16 Work16)
{
	ClearFlags(Zero|Negative);

	if (Work16 == 0)
		SetZero();
	else if (Work16 & 0x8000)
		SetNegative();
}

static inline void SetZN8 (uint8 Work8)
{
	ClearFlags(Zero|Negative);

	if (Work8 == 0)
		SetZero();
	else if (Work8 & 0x80)
		SetNegative();
}

static inline void ADC16 (uint16 Work16)
{
	if (CheckDecimal())
	{
		uint32 carry = CheckCarry();
		uint32 result = (Registers.A.W & 0x000F) + (Work16 & 0x000F) + carry;

		if (result > 0x0009)
			result += 0x0006;
		carry = (result > 0x000F);

		result = (Registers.A.W & 0x00F0) + (Work16 & 0x00F0) + (result & 0x000F) + carry * 0x10;
		if (result > 0x009F)
			result += 0x0060;
		carry = (result > 0x00FF);

		result = (Registers.A.W & 0x0F00) + (Work16 & 0x0F00) + (result & 0x00FF) + carry * 0x100;
		if (result > 0x09FF)
			result += 0x0600;
		carry = (result > 0x0FFF);

		result = (Registers.A.W & 0xF000) + (Work16 & 0xF000) + (result & 0x0FFF) + carry * 0x1000;

		if ((Registers.A.W & 0x8000) == (Work16 & 0x8000) && (Registers.A.W & 0x8000) != (result & 0x8000))
			SetOverflow();
		else
			ClearOverflow();

		if (result > 0x9FFF)
			result += 0x6000;

		if (result > 0xFFFF)
			SetCarry();
		else
			ClearCarry();

		Registers.A.W = result & 0xFFFF;
		SetZN16(Registers.A.W);
	}
	else
	{
		uint32	Ans32 = Registers.A.W + Work16 + CheckCarry();

		SetCarryX(Ans32 >= 0x10000);

		if (~(Registers.A.W ^ Work16) & (Work16 ^ (uint16) Ans32) & 0x8000)
			SetOverflow();
		else
			ClearOverflow();

		Registers.A.W = (uint16) Ans32;
		SetZN16(Registers.A.W);
	}
}

static inline void ADC8 (uint8 Work8)
{
	if (CheckDecimal())
	{
		uint32 carry = CheckCarry();
		uint32 result = (Registers.AL & 0x0F) + (Work8 & 0x0F) + carry;

		if ( result > 0x09 )
			result += 0x06;
		carry = (result > 0x0F);

		result = (Registers.AL & 0xF0) + (Work8 & 0xF0) + (result & 0x0F) + (carry * 0x10);

		if ((Registers.AL & 0x80) == (Work8 & 0x80) && (Registers.AL & 0x80) != (result & 0x80))
			SetOverflow();
		else
			ClearOverflow();

		if (result > 0x9F)
			result += 0x60;

		if (result > 0xFF)
			SetCarry();
		else
			ClearCarry();

		Registers.AL = result & 0xFF;
		SetZN8(Registers.AL);
	}
	else
	{
		uint16	Ans16 = Registers.AL + Work8 + CheckCarry();

		SetCarryX(Ans16 >= 0x100);

		if (~(Registers.AL ^ Work8) & (Work8 ^ (uint8) Ans16) & 0x80)
			SetOverflow();
		else
			ClearOverflow();

		Registers.AL = (uint8) Ans16;
		SetZN8(Registers.AL);
	}
}

static inline void AND16 (uint16 Work16)
{
	Registers.A.W &= Work16;
	SetZN16(Registers.A.W);
}

static inline void AND8 (uint8 Work8)
{
	Registers.AL &= Work8;
	SetZN8(Registers.AL);
}

static inline void ASL16 (uint32 OpAddress, s9xwrap_t w)
{
	uint16	Work16 = S9xGetWord(OpAddress, w);
	SetCarryX((Work16 >> 15) & 1);
	Work16 <<= 1;
	AddCycles(ONE_CYCLE);
	S9xSetWord(Work16, OpAddress, w, WRITE_10);
	OpenBus = Work16 & 0xff;
	SetZN16(Work16);
}

static inline void ASL8 (uint32 OpAddress)
{
	uint8	Work8 = S9xGetByte(OpAddress);
	SetCarryX((Work8 >> 7) & 1);
	Work8 <<= 1;
	AddCycles(ONE_CYCLE);
	S9xSetByte(Work8, OpAddress);
	OpenBus = Work8;
	SetZN8(Work8);
}

static inline void BIT16 (uint16 Work16)
{
	ClearFlags(Overflow|Negative|Zero);
	SetFlags((Work16 >> 8) & (0x40|0x80));
	if ((Work16 & Registers.A.W) == 0)
		SetFlags(Zero);
}

static inline void BIT8 (uint8 Work8)
{
	ClearFlags(Overflow|Negative|Zero);
	SetFlags(Work8 & (0x40|0x80));
	if ((Work8 & Registers.AL) == 0)
		SetFlags(Zero);
}

static inline void CMP16 (uint16 val)
{
	int32	Int32 = (int32) Registers.A.W - (int32) val;
	SetCarryX(Int32 >= 0);
	SetZN16((uint16) Int32);
}

static inline void CMP8 (uint8 val)
{
	int16	Int16 = (int16) Registers.AL - (int16) val;
	SetCarryX(Int16 >= 0);
	SetZN8(Int16);
}

static inline void CPX16 (uint16 val)
{
	int32	Int32 = (int32) Registers.X.W - (int32) val;
	SetCarryX(Int32 >= 0);
	SetZN16((uint16) Int32);
}

static inline void CPX8 (uint8 val)
{
	int16	Int16 = (int16) Registers.XL - (int16) val;
	SetCarryX(Int16 >= 0);
	SetZN8(Int16);
}

static inline void CPY16 (uint16 val)
{
	int32	Int32 = (int32) Registers.Y.W - (int32) val;
	SetCarryX(Int32 >= 0);
	SetZN16((uint16) Int32);
}

static inline void CPY8 (uint8 val)
{
	int16	Int16 = (int16) Registers.YL - (int16) val;
	SetCarryX(Int16 >= 0);
	SetZN16((uint8) Int16);
}

static inline void DEC16 (uint32 OpAddress, s9xwrap_t w)
{
	uint16	Work16 = S9xGetWord(OpAddress, w) - 1;
	AddCycles(ONE_CYCLE);
	S9xSetWord(Work16, OpAddress, w, WRITE_10);
	OpenBus = Work16 & 0xff;
	SetZN16(Work16);
}

static inline void DEC8 (uint32 OpAddress)
{
	uint8	Work8 = S9xGetByte(OpAddress) - 1;
	AddCycles(ONE_CYCLE);
	S9xSetByte(Work8, OpAddress);
	OpenBus = Work8;
	SetZN8(Work8);
}

static inline void EOR16 (uint16 val)
{
	Registers.A.W ^= val;
	SetZN16(Registers.A.W);
}

static inline void EOR8 (uint8 val)
{
	Registers.AL ^= val;
	SetZN8(Registers.AL);
}

static inline void INC16 (uint32 OpAddress, s9xwrap_t w)
{
	uint16	Work16 = S9xGetWord(OpAddress, w) + 1;
	AddCycles(ONE_CYCLE);
	S9xSetWord(Work16, OpAddress, w, WRITE_10);
	OpenBus = Work16 & 0xff;
	SetZN16(Work16);
}

static inline void INC8 (uint32 OpAddress)
{
	uint8	Work8 = S9xGetByte(OpAddress) + 1;
	AddCycles(ONE_CYCLE);
	S9xSetByte(Work8, OpAddress);
	OpenBus = Work8;
	SetZN8(Work8);
}

static inline void LDA16 (uint16 val)
{
	Registers.A.W = val;
	SetZN16(Registers.A.W);
}

static inline void LDA8 (uint8 val)
{
	Registers.AL = val;
	SetZN8(Registers.AL);
}

static inline void LDX16 (uint16 val)
{
	Registers.X.W = val;
	SetZN16(Registers.X.W);
}

static inline void LDX8 (uint8 val)
{
	Registers.XL = val;
	SetZN8(Registers.XL);
}

static inline void LDY16 (uint16 val)
{
	Registers.Y.W = val;
	SetZN16(Registers.Y.W);
}

static inline void LDY8 (uint8 val)
{
	Registers.YL = val;
	SetZN8(Registers.YL);
}

static inline void LSR16 (uint32 OpAddress, s9xwrap_t w)
{
	uint16	Work16 = S9xGetWord(OpAddress, w);
	SetCarryX(Work16 & 1);
	Work16 >>= 1;
	AddCycles(ONE_CYCLE);
	S9xSetWord(Work16, OpAddress, w, WRITE_10);
	OpenBus = Work16 & 0xff;
	SetZN16(Work16);
}

static inline void LSR8 (uint32 OpAddress)
{
	uint8	Work8 = S9xGetByte(OpAddress);
	SetCarryX(Work8 & 1);
	Work8 >>= 1;
	AddCycles(ONE_CYCLE);
	S9xSetByte(Work8, OpAddress);
	OpenBus = Work8;
	SetZN8(Work8);
}

static inline void ORA16 (uint16 val)
{
	Registers.A.W |= val;
	SetZN16(Registers.A.W);
}

static inline void ORA8 (uint8 val)
{
	Registers.AL |= val;
	SetZN8(Registers.AL);
}

static inline void ROL16 (uint32 OpAddress, s9xwrap_t w)
{
	uint32	Work32 = (((uint32) S9xGetWord(OpAddress, w)) << 1) | CheckCarry();
	SetCarryX(Work32 >= 0x10000);
	AddCycles(ONE_CYCLE);
	S9xSetWord((uint16) Work32, OpAddress, w, WRITE_10);
	OpenBus = Work32 & 0xff;
	SetZN16((uint16) Work32);
}

static inline void ROL8 (uint32 OpAddress)
{
	uint16	Work16 = (((uint16) S9xGetByte(OpAddress)) << 1) | CheckCarry();
	SetCarryX(Work16 >= 0x100);
	AddCycles(ONE_CYCLE);
	S9xSetByte((uint8) Work16, OpAddress);
	OpenBus = Work16 & 0xff;
	SetZN8(Work16);
}

static inline void ROR16 (uint32 OpAddress, s9xwrap_t w)
{
	uint32	Work32 = ((uint32) S9xGetWord(OpAddress, w)) | (((uint32) CheckCarry()) << 16);
	SetCarryX(Work32 & 1);
	Work32 >>= 1;
	AddCycles(ONE_CYCLE);
	S9xSetWord((uint16) Work32, OpAddress, w, WRITE_10);
	OpenBus = Work32 & 0xff;
	SetZN16((uint16) Work32);
}

static inline void ROR8 (uint32 OpAddress)
{
	uint16	Work16 = ((uint16) S9xGetByte(OpAddress)) | (((uint16) CheckCarry()) << 8);
	SetCarryX(Work16 & 1);
	Work16 >>= 1;
	AddCycles(ONE_CYCLE);
	S9xSetByte((uint8) Work16, OpAddress);
	OpenBus = Work16 & 0xff;
	SetZN8(Work16);
}

static inline void SBC16 (uint16 Work16)
{
	if (CheckDecimal())
	{
		int result;
		int carry = CheckCarry();

		Work16 ^= 0xFFFF;

		result = (Registers.A.W & 0x000F) + (Work16 & 0x000F) + carry;
		if (result < 0x0010)
			result -= 0x0006;
		carry = (result > 0x000F);

		result = (Registers.A.W & 0x00F0) + (Work16 & 0x00F0) + (result & 0x000F) + carry * 0x10;
		if (result < 0x0100)
			result -= 0x0060;
		carry = (result > 0x00FF);

		result = (Registers.A.W & 0x0F00) + (Work16 & 0x0F00) + (result & 0x00FF) + carry * 0x100;
		if (result < 0x1000)
			result -= 0x0600;
		carry = (result > 0x0FFF);

		result = (Registers.A.W & 0xF000) + (Work16 & 0xF000) + (result & 0x0FFF) + carry * 0x1000;

		if (((Registers.A.W ^ Work16) & 0x8000) == 0 && ((Registers.A.W ^ result) & 0x8000))
			SetOverflow();
		else
			ClearOverflow();

		if (result < 0x10000)
			result -= 0x6000;

		if (result > 0xFFFF)
			SetCarry();
		else
			ClearCarry();

		Registers.A.W = result & 0xFFFF;
		SetZN16(Registers.A.W);
	}
	else
	{
		int32	Int32 = (int32) Registers.A.W - (int32) Work16 + (int32) CheckCarry() - 1;

		SetCarryX(Int32 >= 0);

		if ((Registers.A.W ^ Work16) & (Registers.A.W ^ (uint16) Int32) & 0x8000)
			SetOverflow();
		else
			ClearOverflow();

		Registers.A.W = (uint16) Int32;
		SetZN16(Registers.A.W);
	}
}

static inline void SBC8 (uint8 Work8)
{
	if (CheckDecimal())
	{
		int result;
		int carry = CheckCarry();

		Work8 ^= 0xFF;

		result = (Registers.AL & 0x0F) + (Work8 & 0x0F) + carry;
		if (result < 0x10)
			result -= 0x06;
		carry = (result > 0x0F);

		result = (Registers.AL & 0xF0) + (Work8 & 0xF0) + (result & 0x0F) + carry * 0x10;

		if ((Registers.AL & 0x80) == (Work8 & 0x80) && (Registers.AL & 0x80) != (result & 0x80))
			SetOverflow();
		else
			ClearOverflow();

		if (result < 0x100 )
			result -= 0x60;

		if (result > 0xFF)
			SetCarry();
		else
			ClearCarry();

		Registers.AL = result & 0xFF;
		SetZN8(Registers.AL);
	}
	else
	{
		int16	Int16 = (int16) Registers.AL - (int16) Work8 + (int16) CheckCarry() - 1;

		SetCarryX(Int16 >= 0);

		if ((Registers.AL ^ Work8) & (Registers.AL ^ (uint8) Int16) & 0x80)
			SetOverflow();
		else
			ClearOverflow();

		Registers.AL = (uint8) Int16;
		SetZN8(Registers.AL);
	}
}

static inline void STA16 (uint32 OpAddress, s9xwrap_t w)
{
	S9xSetWord(Registers.A.W, OpAddress, w, WRITE_01);
	OpenBus = Registers.AH;
}

static inline void STA8 (uint32 OpAddress)
{
	S9xSetByte(Registers.AL, OpAddress);
	OpenBus = Registers.AL;
}

static inline void STX16 (uint32 OpAddress, s9xwrap_t w)
{
	S9xSetWord(Registers.X.W, OpAddress, w, WRITE_01);
	OpenBus = Registers.XH;
}

static inline void STX8 (uint32 OpAddress)
{
	S9xSetByte(Registers.XL, OpAddress);
	OpenBus = Registers.XL;
}

static inline void STY16 (uint32 OpAddress, s9xwrap_t w)
{
	S9xSetWord(Registers.Y.W, OpAddress, w, WRITE_01);
	OpenBus = Registers.YH;
}

static inline void STY8 (uint32 OpAddress)
{
	S9xSetByte(Registers.YL, OpAddress);
	OpenBus = Registers.YL;
}

static inline void STZ16 (uint32 OpAddress, s9xwrap_t w)
{
	S9xSetWord(0, OpAddress, w, WRITE_01);
	OpenBus = 0;
}

static inline void STZ8 (uint32 OpAddress)
{
	S9xSetByte(0, OpAddress);
	OpenBus = 0;
}

static inline void TSB16 (uint32 OpAddress, s9xwrap_t w)
{
	uint16	Work16 = S9xGetWord(OpAddress, w);

	SetZeroX((Work16 & Registers.A.W) == 0);

	Work16 |= Registers.A.W;
	AddCycles(ONE_CYCLE);
	S9xSetWord(Work16, OpAddress, w, WRITE_10);
	OpenBus = Work16 & 0xff;
}

static inline void TSB8 (uint32 OpAddress)
{
	uint8	Work8 = S9xGetByte(OpAddress);

	SetZeroX((Work8 & Registers.AL) == 0);

	Work8 |= Registers.AL;
	AddCycles(ONE_CYCLE);
	S9xSetByte(Work8, OpAddress);
	OpenBus = Work8;
}

static inline void TRB16 (uint32 OpAddress, s9xwrap_t w)
{
	uint16	Work16 = S9xGetWord(OpAddress, w);

	SetZeroX((Work16 & Registers.A.W) == 0);

	Work16 &= ~Registers.A.W;
	AddCycles(ONE_CYCLE);
	S9xSetWord(Work16, OpAddress, w, WRITE_10);
	OpenBus = Work16 & 0xff;
}

static inline void TRB8 (uint32 OpAddress)
{
	uint8	Work8 = S9xGetByte(OpAddress);

	SetZeroX((Work8 & Registers.AL) == 0);

	Work8 &= ~Registers.AL;
	AddCycles(ONE_CYCLE);
	S9xSetByte(Work8, OpAddress);
	OpenBus = Work8;
}

static inline void S9xFixCycles (void)
{
	// 0bMX
	const S9xOpcode *OpcodeSets[4] = {S9xOpcodesM0X0, S9xOpcodesM0X1, S9xOpcodesM1X0, S9xOpcodesM1X1};
	ICPU.S9xOpcodes = CheckEmulation() ? S9xOpcodesSlow : OpcodeSets[(Registers.PL >> 4) & 3];
}

#endif
