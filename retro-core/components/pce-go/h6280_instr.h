#include "h6280.h"

typedef signed char SBYTE;
typedef unsigned char UBYTE;
typedef signed short SWORD;
typedef unsigned short UWORD;

static const UBYTE bin2bcd[0x100] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
};

static const UBYTE bcd2bin[0x100] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0, 0, 0, 0, 0, 0,
	0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0, 0, 0, 0, 0, 0,
	0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0, 0, 0, 0, 0, 0,
	0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0, 0, 0, 0, 0, 0,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0, 0, 0, 0, 0, 0,
	0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0, 0, 0, 0, 0, 0,
	0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0, 0, 0, 0, 0, 0,
	0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0, 0, 0, 0, 0, 0,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0, 0, 0, 0, 0, 0,
	0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0, 0, 0, 0, 0, 0,
};

#define ALWAYS_INLINE inline __attribute__((__always_inline__))
#define OPCODE_FUNC static ALWAYS_INLINE void

// Lucky for us FL_N is at the 7th bit position so we can save a comparison :)
#define FLAG_NZ(val) ((val) == 0 ? FL_Z : ((val) & FL_N))

// pointer to the beginning of the Zero Page area
#define ZP_BASE (PCE.RAM)

// pointer to the beginning of the Stack Area
#define SP_BASE (PCE.RAM + 0x100)

// Addressing modes:
#define imm_operand(addr)  ({uint32_t a = (addr); PageR[a >> 13][a];})
#define abs_operand(x)     pce_read8(pce_read16(x))
#define absx_operand(x)    pce_read8(pce_read16(x)+CPU.X)
#define absy_operand(x)    pce_read8(pce_read16(x)+CPU.Y)
#define zp_operand(x)      get_8bit_zp(imm_operand(x))
#define zpx_operand(x)     get_8bit_zp(imm_operand(x)+CPU.X)
#define zpy_operand(x)     get_8bit_zp(imm_operand(x)+CPU.Y)
#define zpind_operand(x)   pce_read8(get_16bit_zp(imm_operand(x)))
#define zpindx_operand(x)  pce_read8(get_16bit_zp(imm_operand(x)+CPU.X))
#define zpindy_operand(x)  pce_read8(get_16bit_zp(imm_operand(x))+CPU.Y)

// Flag check (flags 'N' and 'Z'):
#define chk_flnz_8bit(x) CPU.P = ((CPU.P & (~(FL_N|FL_T|FL_Z))) | FLAG_NZ(x));

// Zero page access
#define get_8bit_zp(zp_addr) ({UBYTE x = zp_addr; *((UBYTE *)(ZP_BASE + (x)));})
//#define get_16bit_zp(zp_addr) ({UBYTE x = zp_addr; get_8bit_zp(x) | get_8bit_zp(x + 1) << 8;})
#define get_16bit_zp(zp_addr) (*((UWORD *)(ZP_BASE + (zp_addr))))
#define put_8bit_zp(zp_addr, byte) ({UBYTE x = zp_addr; *(ZP_BASE + (x)) = (byte);})

// Stack access
#define push_8bit(byte) ({*(SP_BASE + CPU.S) = (byte); CPU.S--;})
#define push_16bit(addr) ({UWORD x = addr; push_8bit(x >> 8); push_8bit(x & 0xFF);})
//#define pull_8bit() (*(SP_BASE + ++CPU.S))
#define pull_8bit(x) ({ ++CPU.S; x = *(SP_BASE + CPU.S);})
//#define pull_16bit() (pull_8bit() | pull_8bit() << 8)

//
// Implementation of actual opcodes:
//

static inline UBYTE
adc(UBYTE acc, UBYTE val)
{
	/* binary mode */
	if (!(CPU.P & FL_D))
	{
		SWORD sig = (SBYTE)acc;
		UWORD usig = (UBYTE)acc;

		if (CPU.P & FL_C)
		{
			usig++;
			sig++;
		}
		sig += (SBYTE)val;
		usig += (UBYTE)val;
		acc = (UBYTE)(usig & 0xFF);

		CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z | FL_C)) | (((sig > 127) || (sig < -128)) ? FL_V : 0) | ((usig > 255) ? FL_C : 0) | FLAG_NZ(acc);
	}

	/* decimal mode */
	else
	{
		uint32_t temp = bcd2bin[acc] + bcd2bin[val];

		if (CPU.P & FL_C)
		{
			temp++;
		}

		acc = bin2bcd[temp];

		CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp > 99) ? FL_C : 0) | FLAG_NZ(acc);

		Cycles++; /* decimal mode takes an extra cycle */
	}

	return acc;
}


static ALWAYS_INLINE void
sbc(UBYTE val)
{
	/* binary mode */
	if (!(CPU.P & FL_D))
	{
		SWORD sig = (SBYTE)CPU.A;
		UWORD usig = (UBYTE)CPU.A;

		if (!(CPU.P & FL_C))
		{
			usig--;
			sig--;
		}
		sig -= (SBYTE)val;
		usig -= (UBYTE)val;
		CPU.A = (UBYTE)(usig & 0xFF);
		CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z | FL_C)) | (((sig > 127) || (sig < -128)) ? FL_V : 0) | ((usig > 255) ? 0 : FL_C) | FLAG_NZ(CPU.A);
	}

	/* decimal mode */
	else
	{
		int temp = (int)bcd2bin[CPU.A] - bcd2bin[val];

		if (!(CPU.P & FL_C))
		{
			temp--;
		}

		CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp < 0) ? 0 : FL_C);

		while (temp < 0)
		{
			temp += 100;
		}

		CPU.A = bin2bcd[temp];
		chk_flnz_8bit(CPU.A);

		Cycles++; /* decimal mode takes an extra cycle */
	}
}

OPCODE_FUNC adc_abs(void)
{
	// if flag 'T' is set, use zero-page address specified by register 'X'
	// as the accumulator...

	if (CPU.P & FL_T)
	{
		put_8bit_zp(CPU.X, adc(get_8bit_zp(CPU.X), abs_operand(CPU.PC + 1)));
		Cycles += 8;
	}
	else
	{
		CPU.A = adc(CPU.A, abs_operand(CPU.PC + 1));
		Cycles += 5;
	}
	CPU.PC += 3;
}

OPCODE_FUNC adc_absx(void)
{
	if (CPU.P & FL_T)
	{
		put_8bit_zp(CPU.X, adc(get_8bit_zp(CPU.X), absx_operand(CPU.PC + 1)));
		Cycles += 8;
	}
	else
	{
		CPU.A = adc(CPU.A, absx_operand(CPU.PC + 1));
		Cycles += 5;
	}
	CPU.PC += 3;
}

OPCODE_FUNC adc_absy(void)
{
	if (CPU.P & FL_T)
	{
		put_8bit_zp(CPU.X, adc(get_8bit_zp(CPU.X), absy_operand(CPU.PC + 1)));
		Cycles += 8;
	}
	else
	{
		CPU.A = adc(CPU.A, absy_operand(CPU.PC + 1));
		Cycles += 5;
	}
	CPU.PC += 3;
}

OPCODE_FUNC adc_imm(void)
{
	if (CPU.P & FL_T)
	{
		put_8bit_zp(CPU.X, adc(get_8bit_zp(CPU.X), imm_operand(CPU.PC + 1)));
		Cycles += 5;
	}
	else
	{
		CPU.A = adc(CPU.A, imm_operand(CPU.PC + 1));
		Cycles += 2;
	}
	CPU.PC += 2;
}

OPCODE_FUNC adc_zp(void)
{
	if (CPU.P & FL_T)
	{
		put_8bit_zp(CPU.X, adc(get_8bit_zp(CPU.X), zp_operand(CPU.PC + 1)));
		Cycles += 7;
	}
	else
	{
		CPU.A = adc(CPU.A, zp_operand(CPU.PC + 1));
		Cycles += 4;
	}
	CPU.PC += 2;
}

OPCODE_FUNC adc_zpx(void)
{
	if (CPU.P & FL_T)
	{
		put_8bit_zp(CPU.X, adc(get_8bit_zp(CPU.X), zpx_operand(CPU.PC + 1)));
		Cycles += 7;
	}
	else
	{
		CPU.A = adc(CPU.A, zpx_operand(CPU.PC + 1));
		Cycles += 4;
	}
	CPU.PC += 2;
}

OPCODE_FUNC adc_zpind(void)
{
	if (CPU.P & FL_T)
	{
		put_8bit_zp(CPU.X, adc(get_8bit_zp(CPU.X), zpind_operand(CPU.PC + 1)));
		Cycles += 10;
	}
	else
	{
		CPU.A = adc(CPU.A, zpind_operand(CPU.PC + 1));
		Cycles += 7;
	}
	CPU.PC += 2;
}

OPCODE_FUNC adc_zpindx(void)
{
	if (CPU.P & FL_T)
	{
		put_8bit_zp(CPU.X, adc(get_8bit_zp(CPU.X), zpindx_operand(CPU.PC + 1)));
		Cycles += 10;
	}
	else
	{
		CPU.A = adc(CPU.A, zpindx_operand(CPU.PC + 1));
		Cycles += 7;
	}
	CPU.PC += 2;
}

OPCODE_FUNC adc_zpindy(void)
{
	if (CPU.P & FL_T)
	{
		put_8bit_zp(CPU.X, adc(get_8bit_zp(CPU.X), zpindy_operand(CPU.PC + 1)));
		Cycles += 10;
	}
	else
	{
		CPU.A = adc(CPU.A, zpindy_operand(CPU.PC + 1));
		Cycles += 7;
	}
	CPU.PC += 2;
}

OPCODE_FUNC and_abs(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp &= abs_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 8;
	}
	else
	{
		CPU.A &= abs_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 5;
	}
	CPU.PC += 3;
}

OPCODE_FUNC and_absx(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp &= absx_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 8;
	}
	else
	{
		CPU.A &= absx_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 5;
	}
	CPU.PC += 3;
}

OPCODE_FUNC and_absy(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp &= absy_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 8;
	}
	else
	{
		CPU.A &= absy_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 5;
	}
	CPU.PC += 3;
}

OPCODE_FUNC and_imm(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp &= imm_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 5;
	}
	else
	{
		CPU.A &= imm_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 2;
	}
	CPU.PC += 2;
}

OPCODE_FUNC and_zp(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp &= zp_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 7;
	}
	else
	{
		CPU.A &= zp_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 4;
	}
	CPU.PC += 2;
}

OPCODE_FUNC and_zpx(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp &= zpx_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 7;
	}
	else
	{
		CPU.A &= zpx_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 4;
	}
	CPU.PC += 2;
}

OPCODE_FUNC and_zpind(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp &= zpind_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 10;
	}
	else
	{
		CPU.A &= zpind_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 7;
	}
	CPU.PC += 2;
}

OPCODE_FUNC and_zpindx(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp &= zpindx_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 10;
	}
	else
	{
		CPU.A &= zpindx_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 7;
	}
	CPU.PC += 2;
}

OPCODE_FUNC and_zpindy(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp &= zpindy_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 10;
	}
	else
	{
		CPU.A &= zpindy_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 7;
	}
	CPU.PC += 2;
}

OPCODE_FUNC asl_a(void)
{
	UBYTE temp = CPU.A;
	CPU.A <<= 1;
	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp & 0x80) ? FL_C : 0) | FLAG_NZ(CPU.A);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC asl_abs(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1);
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = temp1 << 1;

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | FLAG_NZ(temp);
	pce_write8(temp_addr, temp);
	CPU.PC += 3;
	Cycles += 7;
}

OPCODE_FUNC asl_absx(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1) + CPU.X;
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = temp1 << 1;

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | FLAG_NZ(temp);
	pce_write8(temp_addr, temp);
	CPU.PC += 3;
	Cycles += 7;
}

OPCODE_FUNC asl_zp(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1);
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = temp1 << 1;

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | FLAG_NZ(temp);
	put_8bit_zp(zp_addr, temp);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC asl_zpx(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1) + CPU.X;
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = temp1 << 1;

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | FLAG_NZ(temp);
	put_8bit_zp(zp_addr, temp);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC bbr(UBYTE bit)
{
	CPU.P &= ~FL_T;
	if (zp_operand(CPU.PC + 1) & (1 << bit))
	{
		CPU.PC += 3;
		Cycles += 6;
	}
	else
	{
		CPU.PC += (SBYTE)imm_operand(CPU.PC + 2) + 3;
		Cycles += 8;
	}
}

OPCODE_FUNC bbs(UBYTE bit)
{
	CPU.P &= ~FL_T;
	if (zp_operand(CPU.PC + 1) & (1 << bit))
	{
		CPU.PC += (SBYTE)imm_operand(CPU.PC + 2) + 3;
		Cycles += 8;
	}
	else
	{
		CPU.PC += 3;
		Cycles += 6;
	}
}

OPCODE_FUNC bcc(void)
{
	CPU.P &= ~FL_T;
	if (CPU.P & FL_C)
	{
		CPU.PC += 2;
		Cycles += 2;
	}
	else
	{
		CPU.PC += (SBYTE)imm_operand(CPU.PC + 1) + 2;
		Cycles += 4;
	}
}

OPCODE_FUNC bcs(void)
{
	CPU.P &= ~FL_T;
	if (CPU.P & FL_C)
	{
		CPU.PC += (SBYTE)imm_operand(CPU.PC + 1) + 2;
		Cycles += 4;
	}
	else
	{
		CPU.PC += 2;
		Cycles += 2;
	}
}

OPCODE_FUNC beq(void)
{
	CPU.P &= ~FL_T;
	if (CPU.P & FL_Z)
	{
		CPU.PC += (SBYTE)imm_operand(CPU.PC + 1) + 2;
		Cycles += 4;
	}
	else
	{
		CPU.PC += 2;
		Cycles += 2;
	}
}

OPCODE_FUNC bit_abs(void)
{
	UBYTE temp = abs_operand(CPU.PC + 1);
	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((CPU.A & temp) ? 0 : FL_Z);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC bit_absx(void)
{
	UBYTE temp = absx_operand(CPU.PC + 1);
	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((CPU.A & temp) ? 0 : FL_Z);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC bit_imm(void)
{
	UBYTE temp = imm_operand(CPU.PC + 1);
	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((CPU.A & temp) ? 0 : FL_Z);
	CPU.PC += 2;
	Cycles += 2;
}

OPCODE_FUNC bit_zp(void)
{
	UBYTE temp = zp_operand(CPU.PC + 1);
	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((CPU.A & temp) ? 0 : FL_Z);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC bit_zpx(void)
{
	UBYTE temp = zpx_operand(CPU.PC + 1);
	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((CPU.A & temp) ? 0 : FL_Z);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC bmi(void)
{
	CPU.P &= ~FL_T;
	if (CPU.P & FL_N)
	{
		CPU.PC += (SBYTE)imm_operand(CPU.PC + 1) + 2;
		Cycles += 4;
	}
	else
	{
		CPU.PC += 2;
		Cycles += 2;
	}
}

OPCODE_FUNC bne(void)
{
	CPU.P &= ~FL_T;
	if (CPU.P & FL_Z)
	{
		CPU.PC += 2;
		Cycles += 2;
	}
	else
	{
		CPU.PC += (SBYTE)imm_operand(CPU.PC + 1) + 2;
		Cycles += 4;
	}
}

OPCODE_FUNC bpl(void)
{
	CPU.P &= ~FL_T;
	if (CPU.P & FL_N)
	{
		CPU.PC += 2;
		Cycles += 2;
	}
	else
	{
		CPU.PC += (SBYTE)imm_operand(CPU.PC + 1) + 2;
		Cycles += 4;
	}
}

OPCODE_FUNC bra(void)
{
	CPU.P &= ~FL_T;
	CPU.PC += (SBYTE)imm_operand(CPU.PC + 1) + 2;
	Cycles += 4;
}

OPCODE_FUNC brk(void)
{
	MESSAGE_DEBUG("BRK opcode has been hit [PC = 0x%04x] at %s(%d)\n", CPU.PC);
	CPU.P &= ~FL_T;
	push_16bit(CPU.PC + 2);
	push_8bit(CPU.P | FL_B);
	CPU.P = (CPU.P & ~FL_D) | FL_I;
	CPU.PC = pce_read16(VEC_BRK);
	Cycles += 8;
}

OPCODE_FUNC bsr(void)
{
	CPU.P &= ~FL_T;
	push_16bit(CPU.PC + 1);
	CPU.PC += (SBYTE)imm_operand(CPU.PC + 1) + 2;
	Cycles += 8;
}

OPCODE_FUNC bvc(void)
{
	CPU.P &= ~FL_T;
	if (CPU.P & FL_V)
	{
		CPU.PC += 2;
		Cycles += 2;
	}
	else
	{
		CPU.PC += (SBYTE)imm_operand(CPU.PC + 1) + 2;
		Cycles += 4;
	}
}

OPCODE_FUNC bvs(void)
{
	CPU.P &= ~FL_T;
	if (CPU.P & FL_V)
	{
		CPU.PC += (SBYTE)imm_operand(CPU.PC + 1) + 2;
		Cycles += 4;
	}
	else
	{
		CPU.PC += 2;
		Cycles += 2;
	}
}

OPCODE_FUNC cla(void)
{
	CPU.P &= ~FL_T;
	CPU.A = 0;
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC clc(void)
{
	CPU.P &= ~(FL_T | FL_C);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC cld(void)
{
	CPU.P &= ~(FL_T | FL_D);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC cli(void)
{
	CPU.P &= ~(FL_T | FL_I);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC clv(void)
{
	CPU.P &= ~(FL_V | FL_T);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC clx(void)
{
	CPU.P &= ~FL_T;
	CPU.X = 0;
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC cly(void)
{
	CPU.P &= ~FL_T;
	CPU.Y = 0;
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC cmp_abs(void)
{
	UBYTE temp = abs_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.A < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.A - temp));
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC cmp_absx(void)
{
	UBYTE temp = absx_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.A < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.A - temp));
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC cmp_absy(void)
{
	UBYTE temp = absy_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.A < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.A - temp));
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC cmp_imm(void)
{
	UBYTE temp = imm_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.A < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.A - temp));
	CPU.PC += 2;
	Cycles += 2;
}

OPCODE_FUNC cmp_zp(void)
{
	UBYTE temp = zp_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.A < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.A - temp));
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC cmp_zpx(void)
{
	UBYTE temp = zpx_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.A < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.A - temp));
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC cmp_zpind(void)
{
	UBYTE temp = zpind_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.A < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.A - temp));
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC cmp_zpindx(void)
{
	UBYTE temp = zpindx_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.A < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.A - temp));
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC cmp_zpindy(void)
{
	UBYTE temp = zpindy_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.A < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.A - temp));
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC cpx_abs(void)
{
	UBYTE temp = abs_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.X < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.X - temp));
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC cpx_imm(void)
{
	UBYTE temp = imm_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.X < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.X - temp));
	CPU.PC += 2;
	Cycles += 2;
}

OPCODE_FUNC cpx_zp(void)
{
	UBYTE temp = zp_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.X < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.X - temp));
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC cpy_abs(void)
{
	UBYTE temp = abs_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.Y < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.Y - temp));
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC cpy_imm(void)
{
	UBYTE temp = imm_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.Y < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.Y - temp));
	CPU.PC += 2;
	Cycles += 2;
}

OPCODE_FUNC cpy_zp(void)
{
	UBYTE temp = zp_operand(CPU.PC + 1);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((CPU.Y < temp) ? 0 : FL_C) | FLAG_NZ((UBYTE)(CPU.Y - temp));
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC dec_a(void)
{
	--CPU.A;
	chk_flnz_8bit(CPU.A);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC dec_abs(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1);
	UBYTE temp = pce_read8(temp_addr) - 1;
	chk_flnz_8bit(temp);
	pce_write8(temp_addr, temp);
	CPU.PC += 3;
	Cycles += 7;
}

OPCODE_FUNC dec_absx(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1) + CPU.X;
	UBYTE temp = pce_read8(temp_addr) - 1;
	chk_flnz_8bit(temp);
	pce_write8(temp_addr, temp);
	CPU.PC += 3;
	Cycles += 7;
}

OPCODE_FUNC dec_zp(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1);
	UBYTE temp = get_8bit_zp(zp_addr) - 1;
	chk_flnz_8bit(temp);
	put_8bit_zp(zp_addr, temp);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC dec_zpx(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1) + CPU.X;
	UBYTE temp = get_8bit_zp(zp_addr) - 1;
	chk_flnz_8bit(temp);
	put_8bit_zp(zp_addr, temp);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC dex(void)
{
	--CPU.X;
	chk_flnz_8bit(CPU.X);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC dey(void)
{
	--CPU.Y;
	chk_flnz_8bit(CPU.Y);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC eor_abs(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp ^= abs_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 8;
	}
	else
	{
		CPU.A ^= abs_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 5;
	}
	CPU.PC += 3;
}

OPCODE_FUNC eor_absx(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp ^= absx_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 8;
	}
	else
	{
		CPU.A ^= absx_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 5;
	}
	CPU.PC += 3;
}

OPCODE_FUNC eor_absy(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp ^= absy_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 8;
	}
	else
	{
		CPU.A ^= absy_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 5;
	}
	CPU.PC += 3;
}

OPCODE_FUNC eor_imm(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp ^= imm_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 5;
	}
	else
	{
		CPU.A ^= imm_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 2;
	}
	CPU.PC += 2;
}

OPCODE_FUNC eor_zp(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp ^= zp_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 7;
	}
	else
	{
		CPU.A ^= zp_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 4;
	}
	CPU.PC += 2;
}

OPCODE_FUNC eor_zpx(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp ^= zpx_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 7;
	}
	else
	{
		CPU.A ^= zpx_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 4;
	}
	CPU.PC += 2;
}

OPCODE_FUNC eor_zpind(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp ^= zpind_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 10;
	}
	else
	{
		CPU.A ^= zpind_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 7;
	}
	CPU.PC += 2;
}

OPCODE_FUNC eor_zpindx(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp ^= zpindx_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 10;
	}
	else
	{
		CPU.A ^= zpindx_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 7;
	}
	CPU.PC += 2;
}

OPCODE_FUNC eor_zpindy(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp ^= zpindy_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 10;
	}
	else
	{
		CPU.A ^= zpindy_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 7;
	}
	CPU.PC += 2;
}

OPCODE_FUNC halt(void)
{
	return;
}

OPCODE_FUNC inc_a(void)
{
	++CPU.A;
	chk_flnz_8bit(CPU.A);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC inc_abs(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1);
	UBYTE temp = pce_read8(temp_addr) + 1;
	chk_flnz_8bit(temp);
	pce_write8(temp_addr, temp);
	CPU.PC += 3;
	Cycles += 7;
}

OPCODE_FUNC inc_absx(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1) + CPU.X;
	UBYTE temp = pce_read8(temp_addr) + 1;
	chk_flnz_8bit(temp);
	pce_write8(temp_addr, temp);
	CPU.PC += 3;
	Cycles += 7;
}

OPCODE_FUNC inc_zp(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1);
	UBYTE temp = get_8bit_zp(zp_addr) + 1;
	chk_flnz_8bit(temp);
	put_8bit_zp(zp_addr, temp);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC inc_zpx(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1) + CPU.X;
	UBYTE temp = get_8bit_zp(zp_addr) + 1;
	chk_flnz_8bit(temp);
	put_8bit_zp(zp_addr, temp);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC inx(void)
{
	++CPU.X;
	chk_flnz_8bit(CPU.X);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC iny(void)
{
	++CPU.Y;
	chk_flnz_8bit(CPU.Y);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC jmp(void)
{
	CPU.P &= ~FL_T;
	CPU.PC = pce_read16(CPU.PC + 1);
	Cycles += 4;
}

OPCODE_FUNC jmp_absind(void)
{
	CPU.P &= ~FL_T;
	CPU.PC = pce_read16(pce_read16(CPU.PC + 1));
	Cycles += 7;
}

OPCODE_FUNC jmp_absindx(void)
{
	CPU.P &= ~FL_T;
	CPU.PC = pce_read16(pce_read16(CPU.PC + 1) + CPU.X);
	Cycles += 7;
}

OPCODE_FUNC jsr(void)
{
	CPU.P &= ~FL_T;
	push_16bit(CPU.PC + 2);
	CPU.PC = pce_read16(CPU.PC + 1);
	Cycles += 7;
}

OPCODE_FUNC lda_abs(void)
{
	CPU.A = abs_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.A);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC lda_absx(void)
{
	CPU.A = absx_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.A);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC lda_absy(void)
{
	CPU.A = absy_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.A);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC lda_imm(void)
{
	CPU.A = imm_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.A);
	CPU.PC += 2;
	Cycles += 2;
}

OPCODE_FUNC lda_zp(void)
{
	CPU.A = zp_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.A);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC lda_zpx(void)
{
	CPU.A = zpx_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.A);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC lda_zpind(void)
{
	CPU.A = zpind_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.A);
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC lda_zpindx(void)
{
	CPU.A = zpindx_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.A);
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC lda_zpindy(void)
{
	CPU.A = zpindy_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.A);
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC ldx_abs(void)
{
	CPU.X = abs_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.X);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC ldx_absy(void)
{
	CPU.X = absy_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.X);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC ldx_imm(void)
{
	CPU.X = imm_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.X);
	CPU.PC += 2;
	Cycles += 2;
}

OPCODE_FUNC ldx_zp(void)
{
	CPU.X = zp_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.X);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC ldx_zpy(void)
{
	CPU.X = zpy_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.X);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC ldy_abs(void)
{
	CPU.Y = abs_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.Y);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC ldy_absx(void)
{
	CPU.Y = absx_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.Y);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC ldy_imm(void)
{
	CPU.Y = imm_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.Y);
	CPU.PC += 2;
	Cycles += 2;
}

OPCODE_FUNC ldy_zp(void)
{
	CPU.Y = zp_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.Y);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC ldy_zpx(void)
{
	CPU.Y = zpx_operand(CPU.PC + 1);
	chk_flnz_8bit(CPU.Y);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC lsr_a(void)
{
	UBYTE temp = CPU.A;
	CPU.A /= 2;
	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp & 1) ? FL_C : 0) | FLAG_NZ(CPU.A);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC lsr_abs(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1);
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = temp1 / 2;

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 1) ? FL_C : 0) | FLAG_NZ(temp);
	pce_write8(temp_addr, temp);
	CPU.PC += 3;
	Cycles += 7;
}

OPCODE_FUNC lsr_absx(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1) + CPU.X;
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = temp1 / 2;

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 1) ? FL_C : 0) | FLAG_NZ(temp);
	pce_write8(temp_addr, temp);
	CPU.PC += 3;
	Cycles += 7;
}

OPCODE_FUNC lsr_zp(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1);
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = temp1 / 2;

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 1) ? FL_C : 0) | FLAG_NZ(temp);
	put_8bit_zp(zp_addr, temp);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC lsr_zpx(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1) + CPU.X;
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = temp1 / 2;

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 1) ? FL_C : 0) | FLAG_NZ(temp);
	put_8bit_zp(zp_addr, temp);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC nop(void)
{
	CPU.P &= ~FL_T;
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC ora_abs(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp |= abs_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 8;
	}
	else
	{
		CPU.A |= abs_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 5;
	}
	CPU.PC += 3;
}

OPCODE_FUNC ora_absx(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp |= absx_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 8;
	}
	else
	{
		CPU.A |= absx_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 5;
	}
	CPU.PC += 3;
}

OPCODE_FUNC ora_absy(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp |= absy_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 8;
	}
	else
	{
		CPU.A |= absy_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 5;
	}
	CPU.PC += 3;
}

OPCODE_FUNC ora_imm(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp |= imm_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 5;
	}
	else
	{
		CPU.A |= imm_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 2;
	}
	CPU.PC += 2;
}

OPCODE_FUNC ora_zp(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp |= zp_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 7;
	}
	else
	{
		CPU.A |= zp_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 4;
	}
	CPU.PC += 2;
}

OPCODE_FUNC ora_zpx(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp |= zpx_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 7;
	}
	else
	{
		CPU.A |= zpx_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 4;
	}
	CPU.PC += 2;
}

OPCODE_FUNC ora_zpind(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp |= zpind_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 10;
	}
	else
	{
		CPU.A |= zpind_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 7;
	}
	CPU.PC += 2;
}

OPCODE_FUNC ora_zpindx(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp |= zpindx_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 10;
	}
	else
	{
		CPU.A |= zpindx_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 7;
	}
	CPU.PC += 2;
}

OPCODE_FUNC ora_zpindy(void)
{
	if (CPU.P & FL_T)
	{
		UBYTE temp = get_8bit_zp(CPU.X);
		temp |= zpindy_operand(CPU.PC + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(CPU.X, temp);
		Cycles += 10;
	}
	else
	{
		CPU.A |= zpindy_operand(CPU.PC + 1);
		chk_flnz_8bit(CPU.A);
		Cycles += 7;
	}
	CPU.PC += 2;
}

OPCODE_FUNC pha(void)
{
	CPU.P &= ~FL_T;
	push_8bit(CPU.A);
	CPU.PC++;
	Cycles += 3;
}

OPCODE_FUNC php(void)
{
	CPU.P &= ~FL_T;
	push_8bit(CPU.P);
	CPU.PC++;
	Cycles += 3;
}

OPCODE_FUNC phx(void)
{
	CPU.P &= ~FL_T;
	push_8bit(CPU.X);
	CPU.PC++;
	Cycles += 3;
}

OPCODE_FUNC phy(void)
{
	CPU.P &= ~FL_T;
	push_8bit(CPU.Y);
	CPU.PC++;
	Cycles += 3;
}

OPCODE_FUNC pla(void)
{
	pull_8bit(CPU.A);
	chk_flnz_8bit(CPU.A);
	CPU.PC++;
	Cycles += 4;
}

OPCODE_FUNC plp(void)
{
	pull_8bit(CPU.P);
	CPU.PC++;
	Cycles += 4;
}

OPCODE_FUNC plx(void)
{
	pull_8bit(CPU.X);
	chk_flnz_8bit(CPU.X);
	CPU.PC++;
	Cycles += 4;
}

OPCODE_FUNC ply(void)
{
	pull_8bit(CPU.Y);
	chk_flnz_8bit(CPU.Y);
	CPU.PC++;
	Cycles += 4;
}

OPCODE_FUNC rmb(UBYTE bit)
{
	UBYTE temp = imm_operand(CPU.PC + 1);
	CPU.P &= ~FL_T;
	put_8bit_zp(temp, get_8bit_zp(temp) & (~(1 << bit)));
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC rol_a(void)
{
	UBYTE temp = CPU.A;
	CPU.A = (CPU.A << 1) + (CPU.P & FL_C);
	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp & 0x80) ? FL_C : 0) | FLAG_NZ(CPU.A);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC rol_abs(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1);
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = (temp1 << 1) + (CPU.P & FL_C);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | FLAG_NZ(temp);
	Cycles += 7;
	pce_write8(temp_addr, temp);
	CPU.PC += 3;
}

OPCODE_FUNC rol_absx(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1) + CPU.X;
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = (temp1 << 1) + (CPU.P & FL_C);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | FLAG_NZ(temp);
	Cycles += 7;
	pce_write8(temp_addr, temp);
	CPU.PC += 3;
}

OPCODE_FUNC rol_zp(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1);
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = (temp1 << 1) + (CPU.P & FL_C);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | FLAG_NZ(temp);
	put_8bit_zp(zp_addr, temp);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC rol_zpx(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1) + CPU.X;
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = (temp1 << 1) + (CPU.P & FL_C);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | FLAG_NZ(temp);
	put_8bit_zp(zp_addr, temp);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC ror_a(void)
{
	UBYTE temp = CPU.A;
	CPU.A = (CPU.A >> 1) + ((CPU.P & FL_C) ? 0x80 : 0);
	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp & 0x01) ? FL_C : 0) | FLAG_NZ(CPU.A);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC ror_abs(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1);
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = (temp1 >> 1) + ((CPU.P & FL_C) ? 0x80 : 0);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x01) ? FL_C : 0) | FLAG_NZ(temp);
	Cycles += 7;
	pce_write8(temp_addr, temp);
	CPU.PC += 3;
}

OPCODE_FUNC ror_absx(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1) + CPU.X;
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = (temp1 >> 1) + ((CPU.P & FL_C) ? 0x80 : 0);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x01) ? FL_C : 0) | FLAG_NZ(temp);
	Cycles += 7;
	pce_write8(temp_addr, temp);
	CPU.PC += 3;
}

OPCODE_FUNC ror_zp(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1);
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = (temp1 >> 1) + ((CPU.P & FL_C) ? 0x80 : 0);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x01) ? FL_C : 0) | FLAG_NZ(temp);
	put_8bit_zp(zp_addr, temp);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC ror_zpx(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1) + CPU.X;
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = (temp1 >> 1) + ((CPU.P & FL_C) ? 0x80 : 0);

	CPU.P = (CPU.P & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x01) ? FL_C : 0) | FLAG_NZ(temp);
	put_8bit_zp(zp_addr, temp);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC rti(void)
{
	/* FL_B reset in RTI */
	pull_8bit(CPU.P);
	CPU.P &= ~FL_B;
	uint8_t t, t2;
	pull_8bit(t);
	pull_8bit(t2);
	CPU.PC = (t | t2 << 8);
	Cycles += 7;
}

OPCODE_FUNC rts(void)
{
	CPU.P &= ~FL_T;
	uint8_t t, t2;
	pull_8bit(t);
	pull_8bit(t2);
	CPU.PC = (t | t2 << 8) + 1;
	Cycles += 7;
}

OPCODE_FUNC sax(void)
{
	UBYTE temp = CPU.X;
	CPU.P &= ~FL_T;
	CPU.X = CPU.A;
	CPU.A = temp;
	CPU.PC++;
	Cycles += 3;
}

OPCODE_FUNC say(void)
{
	UBYTE temp = CPU.Y;
	CPU.P &= ~FL_T;
	CPU.Y = CPU.A;
	CPU.A = temp;
	CPU.PC++;
	Cycles += 3;
}

OPCODE_FUNC sbc_abs(void)
{
	sbc(abs_operand(CPU.PC + 1));
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC sbc_absx(void)
{
	sbc(absx_operand(CPU.PC + 1));
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC sbc_absy(void)
{
	sbc(absy_operand(CPU.PC + 1));
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC sbc_imm(void)
{
	sbc(imm_operand(CPU.PC + 1));
	CPU.PC += 2;
	Cycles += 2;
}

OPCODE_FUNC sbc_zp(void)
{
	sbc(zp_operand(CPU.PC + 1));
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC sbc_zpx(void)
{
	sbc(zpx_operand(CPU.PC + 1));
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC sbc_zpind(void)
{
	sbc(zpind_operand(CPU.PC + 1));
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC sbc_zpindx(void)
{
	sbc(zpindx_operand(CPU.PC + 1));
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC sbc_zpindy(void)
{
	sbc(zpindy_operand(CPU.PC + 1));
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC sec(void)
{
	CPU.P = (CPU.P | FL_C) & ~FL_T;
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC sed(void)
{
	CPU.P = (CPU.P | FL_D) & ~FL_T;
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC sei(void)
{
	CPU.P = (CPU.P | FL_I) & ~FL_T;
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC set(void)
{
	CPU.P |= FL_T;
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC smb(UBYTE bit)
{
	UBYTE temp = imm_operand(CPU.PC + 1);
	CPU.P &= ~FL_T;
	put_8bit_zp(temp, get_8bit_zp(temp) | (1 << bit));
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC st0(void)
{
	CPU.P &= ~FL_T;
	pce_writeIO(0, imm_operand(CPU.PC + 1));
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC st1(void)
{
	CPU.P &= ~FL_T;
	pce_writeIO(2, imm_operand(CPU.PC + 1));
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC st2(void)
{
	CPU.P &= ~FL_T;
	pce_writeIO(3, imm_operand(CPU.PC + 1));
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC sta_abs(void)
{
	CPU.P &= ~FL_T;
	pce_write8(pce_read16(CPU.PC + 1), CPU.A);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC sta_absx(void)
{
	CPU.P &= ~FL_T;
	pce_write8(pce_read16(CPU.PC + 1) + CPU.X, CPU.A);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC sta_absy(void)
{
	CPU.P &= ~FL_T;
	pce_write8(pce_read16(CPU.PC + 1) + CPU.Y, CPU.A);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC sta_zp(void)
{
	CPU.P &= ~FL_T;
	put_8bit_zp(imm_operand(CPU.PC + 1), CPU.A);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC sta_zpx(void)
{
	CPU.P &= ~FL_T;
	put_8bit_zp(imm_operand(CPU.PC + 1) + CPU.X, CPU.A);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC sta_zpind(void)
{
	CPU.P &= ~FL_T;
	pce_write8(get_16bit_zp(imm_operand(CPU.PC + 1)), CPU.A);
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC sta_zpindx(void)
{
	CPU.P &= ~FL_T;
	pce_write8(get_16bit_zp(imm_operand(CPU.PC + 1) + CPU.X), CPU.A);
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC sta_zpindy(void)
{
	CPU.P &= ~FL_T;
	pce_write8(get_16bit_zp(imm_operand(CPU.PC + 1)) + CPU.Y, CPU.A);
	CPU.PC += 2;
	Cycles += 7;
}

OPCODE_FUNC stx_abs(void)
{
	CPU.P &= ~FL_T;
	pce_write8(pce_read16(CPU.PC + 1), CPU.X);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC stx_zp(void)
{
	CPU.P &= ~FL_T;
	put_8bit_zp(imm_operand(CPU.PC + 1), CPU.X);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC stx_zpy(void)
{
	CPU.P &= ~FL_T;
	put_8bit_zp(imm_operand(CPU.PC + 1) + CPU.Y, CPU.X);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC sty_abs(void)
{
	CPU.P &= ~FL_T;
	pce_write8(pce_read16(CPU.PC + 1), CPU.Y);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC sty_zp(void)
{
	CPU.P &= ~FL_T;
	put_8bit_zp(imm_operand(CPU.PC + 1), CPU.Y);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC sty_zpx(void)
{
	CPU.P &= ~FL_T;
	put_8bit_zp(imm_operand(CPU.PC + 1) + CPU.X, CPU.Y);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC stz_abs(void)
{
	CPU.P &= ~FL_T;
	pce_write8(pce_read16(CPU.PC + 1), 0);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC stz_absx(void)
{
	CPU.P &= ~FL_T;
	pce_write8((pce_read16(CPU.PC + 1) + CPU.X), 0);
	CPU.PC += 3;
	Cycles += 5;
}

OPCODE_FUNC stz_zp(void)
{
	CPU.P &= ~FL_T;
	put_8bit_zp(imm_operand(CPU.PC + 1), 0);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC stz_zpx(void)
{
	CPU.P &= ~FL_T;
	put_8bit_zp(imm_operand(CPU.PC + 1) + CPU.X, 0);
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC sxy(void)
{
	UBYTE temp = CPU.Y;
	CPU.P &= ~FL_T;
	CPU.Y = CPU.X;
	CPU.X = temp;
	CPU.PC++;
	Cycles += 3;
}

OPCODE_FUNC tai(void)
{
	CPU.P &= ~FL_T;
	UWORD from = pce_read16(CPU.PC + 1);
	UWORD to = pce_read16(CPU.PC + 3);
	UWORD len = pce_read16(CPU.PC + 5);
	if ( len == 0 ) len = 0xffff;
	UWORD alternate = 0;

	Cycles += (6 * len) + 17;
	while (len-- != 0)
	{
		pce_write8(to++, pce_read8(from + alternate));
		alternate ^= 1;
	}
	CPU.PC += 7;
}

OPCODE_FUNC csh(void)
{
	PCE.Timer.cycles_per_line = 455; /* 21477270 / 3 / 60 / 263 */ /* 7.16 Mhz CPU clock */
	CPU.PC++;
	Cycles+=3;
}

OPCODE_FUNC csl(void)
{
	PCE.Timer.cycles_per_line = 113; /* 21477270 / 12 / 60 / 263 */ /* 1.78 Mhz CPU clock */
	CPU.PC++;
	Cycles+=3;
}

static int tamwrite = -1;
static int tamread = 0;

OPCODE_FUNC tam(void)
{
	UBYTE bitfld = imm_operand(CPU.PC + 1);

	tamwrite = -1;
	for (int i = 0; i < 8; i++)
	{
		if (bitfld & (1 << i))
		{
			pce_bank_set(i, CPU.A);
			tamwrite = CPU.A;
		}
	}

	CPU.P &= ~FL_T;
	CPU.PC += 2;
	Cycles += 5;
}

OPCODE_FUNC tax(void)
{
	CPU.X = CPU.A;
	chk_flnz_8bit(CPU.A);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC tay(void)
{
	CPU.Y = CPU.A;
	chk_flnz_8bit(CPU.A);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC tdd(void)
{
	CPU.P &= ~FL_T;
	UWORD from = pce_read16(CPU.PC + 1);
	UWORD to = pce_read16(CPU.PC + 3);
	UWORD len = pce_read16(CPU.PC + 5);
	if ( len == 0 ) len = 0xffff;

	Cycles += (6 * len) + 17;
	while (len-- != 0)
	{
		pce_write8(to--, pce_read8(from--));
	}
	CPU.PC += 7;
}

OPCODE_FUNC tia(void)
{
	CPU.P &= ~FL_T;
	UWORD from = pce_read16(CPU.PC + 1);
	UWORD to = pce_read16(CPU.PC + 3);
	UWORD len = pce_read16(CPU.PC + 5);
	if ( len == 0 ) len = 0xffff;
	UWORD alternate = 0;

	Cycles += (6 * len) + 17;
	while (len-- != 0)
	{
		pce_write8(to + alternate, pce_read8(from++));
		alternate ^= 1;
	}
	CPU.PC += 7;
}

OPCODE_FUNC tii(void)
{
	CPU.P &= ~FL_T;
	UWORD from = pce_read16(CPU.PC + 1);
	UWORD to = pce_read16(CPU.PC + 3);
	UWORD len = pce_read16(CPU.PC + 5);
	if ( len == 0 ) len = 0xffff;

	Cycles += (6 * len) + 17;
	while (len-- != 0)
	{
		pce_write8(to++, pce_read8(from++));
	}
	CPU.PC += 7;
}

OPCODE_FUNC tin(void)
{
	CPU.P &= ~FL_T;
	UWORD from = pce_read16(CPU.PC + 1);
	UWORD to = pce_read16(CPU.PC + 3);
	UWORD len = pce_read16(CPU.PC + 5);
	if ( len == 0 ) len = 0xffff;

	Cycles += (6 * len) + 17;
	while (len-- != 0)
	{
		pce_write8(to, pce_read8(from++));
	}
	CPU.PC += 7;
}

OPCODE_FUNC tma(void)
{
	UBYTE bitfld = imm_operand(CPU.PC + 1);

	if ( bitfld & 0xff )
	{
		for (int i = 0; i < 8; i++)
		{
			if (bitfld & (1 << i))
			{
				CPU.A = PCE.MMR[i];
			}
		}
		tamread = CPU.A;
	} else {
		CPU.A = ( tamwrite != -1 ) ? tamwrite : tamread ;
	}
	CPU.P &= ~FL_T;
	CPU.PC += 2;
	Cycles += 4;
}

OPCODE_FUNC trb_abs(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1);
	UBYTE temp = pce_read8(temp_addr);
	UBYTE temp1 = (~CPU.A) & temp;

	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp1 & (FL_N | FL_V)) | ((temp & CPU.A) ? 0 : FL_Z);
	pce_write8(temp_addr, temp1);
	CPU.PC += 3;
	Cycles += 7;
}

OPCODE_FUNC trb_zp(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1);
	UBYTE temp = get_8bit_zp(zp_addr);
	UBYTE temp1 = (~CPU.A) & temp;

	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp1 & (FL_N | FL_V)) | ((temp & CPU.A) ? 0 : FL_Z);
	put_8bit_zp(zp_addr, temp1);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC tsb_abs(void)
{
	UWORD temp_addr = pce_read16(CPU.PC + 1);
	UBYTE temp = pce_read8(temp_addr);
	UBYTE temp1 = CPU.A | temp;

	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp1 & (FL_N | FL_V)) | ((temp & CPU.A) ? 0 : FL_Z);
	pce_write8(temp_addr, temp1);
	CPU.PC += 3;
	Cycles += 7;
}

OPCODE_FUNC tsb_zp(void)
{
	UBYTE zp_addr = imm_operand(CPU.PC + 1);
	UBYTE temp = get_8bit_zp(zp_addr);
	UBYTE temp1 = CPU.A | temp;

	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp1 & (FL_N | FL_V)) | ((temp & CPU.A) ? 0 : FL_Z);
	put_8bit_zp(zp_addr, temp1);
	CPU.PC += 2;
	Cycles += 6;
}

OPCODE_FUNC tstins_abs(void)
{
	UBYTE imm_addr = imm_operand(CPU.PC + 1);
	UBYTE temp = abs_operand(CPU.PC + 2);

	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((temp & imm_addr) ? 0 : FL_Z);
	CPU.PC += 4;
	Cycles += 8;
}

OPCODE_FUNC tstins_absx(void)
{
	UBYTE imm_addr = imm_operand(CPU.PC + 1);
	UBYTE temp = absx_operand(CPU.PC + 2);

	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((temp & imm_addr) ? 0 : FL_Z);
	CPU.PC += 4;
	Cycles += 8;
}

OPCODE_FUNC tstins_zp(void)
{
	UBYTE imm_addr = imm_operand(CPU.PC + 1);
	UBYTE temp = zp_operand(CPU.PC + 2);

	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((temp & imm_addr) ? 0 : FL_Z);
	CPU.PC += 3;
	Cycles += 7;
}

OPCODE_FUNC tstins_zpx(void)
{
	UBYTE imm_addr = imm_operand(CPU.PC + 1);
	UBYTE temp = zpx_operand(CPU.PC + 2);

	CPU.P = (CPU.P & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((temp & imm_addr) ? 0 : FL_Z);
	CPU.PC += 3;
	Cycles += 7;
}

OPCODE_FUNC tsx(void)
{
	CPU.X = CPU.S;
	chk_flnz_8bit(CPU.S);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC txa(void)
{
	CPU.A = CPU.X;
	chk_flnz_8bit(CPU.X);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC txs(void)
{
	CPU.P &= ~FL_T;
	CPU.S = CPU.X;
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC tya(void)
{
	CPU.A = CPU.Y;
	chk_flnz_8bit(CPU.Y);
	CPU.PC++;
	Cycles += 2;
}

OPCODE_FUNC interrupt(unsigned type)
{
	// pcetech.txt says the program should clear the irq line by reading 0x1403
	// however in practice it seems to break many games if we don't clear it?

	TRACE_CPU("CPU interrupt: %d\n", type);
	push_16bit(CPU.PC);
	push_8bit(CPU.P);
	CPU.P &= ~(FL_D|FL_T);
	CPU.P |= FL_I;
	if (type & INT_IRQ1) {
		CPU.irq_lines &= ~INT_IRQ1;
		CPU.PC = pce_read16(VEC_IRQ1);
	} else if (type & INT_IRQ2) {
		CPU.irq_lines &= ~INT_IRQ2;
		CPU.PC = pce_read16(VEC_IRQ2);
	} else {
		CPU.irq_lines &= ~INT_TIMER;
		CPU.PC = pce_read16(VEC_TIMER);
	}
	Cycles += 7;
}
