#include "h6280.h"

// flag-value table (for speed)
static const DRAM_ATTR UBYTE flnz_list[0x100] = {
	FL_Z, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 00-0F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 40-4F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 70-7F
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// 80-87
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// 90-97
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// A0-A7
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// B0-B7
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// C0-C7
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// D0-D7
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// E0-E7
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// F0-F7
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N
};

static const DRAM_ATTR UBYTE binbcd[0x100] = {
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

static const DRAM_ATTR UBYTE bcdbin[0x100] = {
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

#define OPCODE_FUNC static void inline __attribute__((__always_inline__)) __attribute__((flatten))

// pointer to the beginning of the Zero Page area
#define ZP_BASE (RAM)

// pointer to the beginning of the Stack Area
#define SP_BASE (RAM + 0x100)

// Addressing modes:
#define imm_operand(addr)  ({UWORD x = addr; PageR[x >> 13][x];})
#define abs_operand(x)     pce_read8(pce_read16(x))
#define absx_operand(x)    pce_read8(pce_read16(x)+reg_x)
#define absy_operand(x)    pce_read8(pce_read16(x)+reg_y)
#define zp_operand(x)      get_8bit_zp(imm_operand(x))
#define zpx_operand(x)     get_8bit_zp(imm_operand(x)+reg_x)
#define zpy_operand(x)     get_8bit_zp(imm_operand(x)+reg_y)
#define zpind_operand(x)   pce_read8(get_16bit_zp(imm_operand(x)))
#define zpindx_operand(x)  pce_read8(get_16bit_zp(imm_operand(x)+reg_x))
#define zpindy_operand(x)  pce_read8(get_16bit_zp(imm_operand(x))+reg_y)

// Flag check (flags 'N' and 'Z'):
#define chk_flnz_8bit(x) reg_p = ((reg_p & (~(FL_N|FL_T|FL_Z))) | FL_B | flnz_list[x]);

// Zero page access
#define get_8bit_zp(zp_addr) (*(ZP_BASE + (zp_addr)))
#define get_16bit_zp(zp_addr) ({UBYTE x = zp_addr; get_8bit_zp(x) | get_8bit_zp(x + 1) << 8;})
#define put_8bit_zp(zp_addr, byte) (*(ZP_BASE + (zp_addr)) = (byte))

// Stack access
#define push_8bit(byte) (*(SP_BASE + reg_s--) = (byte))
#define push_16bit(addr) ({UWORD x = addr; push_8bit(x >> 8); push_8bit(x & 0xFF);})
#define pull_8bit() (*(SP_BASE + ++reg_s))
#define pull_16bit() (pull_8bit() | pull_8bit() << 8)

//
// Implementation of actual opcodes:
//

static inline UBYTE
adc(UBYTE acc, UBYTE val)
{
	SWORD sig = (SBYTE)acc;
	UWORD usig = (UBYTE)acc;
	UWORD temp;

	if (!(reg_p & FL_D))
	{ /* binary mode */
		if (reg_p & FL_C)
		{
			usig++;
			sig++;
		}
		sig += (SBYTE)val;
		usig += (UBYTE)val;
		acc = (UBYTE)(usig & 0xFF);

		reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z | FL_C)) | (((sig > 127) || (sig < -128)) ? FL_V : 0) | ((usig > 255) ? FL_C : 0) | flnz_list[acc];
	}
	else
	{ /* decimal mode */

		// treatment of out-of-range accumulator
		// and operand values (non-BCD) is not
		// adequately defined.  Nor is overflow
		// flag treatment.

		// Zeo : rewrote using bcdbin and binbcd arrays to boost code speed and fix
		// residual bugs

		temp = bcdbin[usig] + bcdbin[val];

		if (reg_p & FL_C)
		{
			temp++;
		}

		acc = binbcd[temp];

		reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp > 99) ? FL_C : 0) | flnz_list[acc];

		Cycles++; /* decimal mode takes an extra cycle */
	}

	return acc;
}

OPCODE_FUNC sbc(UBYTE val)
{
	SWORD sig = (SBYTE)reg_a;
	UWORD usig = (UBYTE)reg_a;
	SWORD temp;

	if (!(reg_p & FL_D))
	{ /* binary mode */
		if (!(reg_p & FL_C))
		{
			usig--;
			sig--;
		}
		sig -= (SBYTE)val;
		usig -= (UBYTE)val;
		reg_a = (UBYTE)(usig & 0xFF);
		reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z | FL_C)) | (((sig > 127) || (sig < -128)) ? FL_V : 0) | ((usig > 255) ? 0 : FL_C) | flnz_list[reg_a]; /* FL_N, FL_Z */
	}
	else
	{ /* decimal mode */

		// treatment of out-of-range accumulator
		// and operand values (non-bcd) is not
		// adequately defined.  Nor is overflow
		// flag treatment.

		temp = (SWORD)(bcdbin[usig] - bcdbin[val]);

		if (!(reg_p & FL_C))
		{
			temp--;
		}

		reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp < 0) ? 0 : FL_C);

		while (temp < 0)
		{
			temp += 100;
		}

		reg_a = binbcd[temp];
		chk_flnz_8bit(reg_a);

		Cycles++; /* decimal mode takes an extra cycle */
	}
}

OPCODE_FUNC adc_abs(void)
{
	// if flag 'T' is set, use zero-page address specified by register 'X'
	// as the accumulator...

	if (reg_p & FL_T)
	{
		put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), abs_operand(reg_pc + 1)));
		Cycles += 8;
	}
	else
	{
		reg_a = adc(reg_a, abs_operand(reg_pc + 1));
		Cycles += 5;
	}
	reg_pc += 3;
}

OPCODE_FUNC adc_absx(void)
{
	if (reg_p & FL_T)
	{
		put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), absx_operand(reg_pc + 1)));
		Cycles += 8;
	}
	else
	{
		reg_a = adc(reg_a, absx_operand(reg_pc + 1));
		Cycles += 5;
	}
	reg_pc += 3;
}

OPCODE_FUNC adc_absy(void)
{
	if (reg_p & FL_T)
	{
		put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), absy_operand(reg_pc + 1)));
		Cycles += 8;
	}
	else
	{
		reg_a = adc(reg_a, absy_operand(reg_pc + 1));
		Cycles += 5;
	}
	reg_pc += 3;
}

OPCODE_FUNC adc_imm(void)
{
	if (reg_p & FL_T)
	{
		put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), imm_operand(reg_pc + 1)));
		Cycles += 5;
	}
	else
	{
		reg_a = adc(reg_a, imm_operand(reg_pc + 1));
		Cycles += 2;
	}
	reg_pc += 2;
}

OPCODE_FUNC adc_zp(void)
{
	if (reg_p & FL_T)
	{
		put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), zp_operand(reg_pc + 1)));
		Cycles += 7;
	}
	else
	{
		reg_a = adc(reg_a, zp_operand(reg_pc + 1));
		Cycles += 4;
	}
	reg_pc += 2;
}

OPCODE_FUNC adc_zpx(void)
{
	if (reg_p & FL_T)
	{
		put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), zpx_operand(reg_pc + 1)));
		Cycles += 7;
	}
	else
	{
		reg_a = adc(reg_a, zpx_operand(reg_pc + 1));
		Cycles += 4;
	}
	reg_pc += 2;
}

OPCODE_FUNC adc_zpind(void)
{
	if (reg_p & FL_T)
	{
		put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), zpind_operand(reg_pc + 1)));
		Cycles += 10;
	}
	else
	{
		reg_a = adc(reg_a, zpind_operand(reg_pc + 1));
		Cycles += 7;
	}
	reg_pc += 2;
}

OPCODE_FUNC adc_zpindx(void)
{
	if (reg_p & FL_T)
	{
		put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), zpindx_operand(reg_pc + 1)));
		Cycles += 10;
	}
	else
	{
		reg_a = adc(reg_a, zpindx_operand(reg_pc + 1));
		Cycles += 7;
	}
	reg_pc += 2;
}

OPCODE_FUNC adc_zpindy(void)
{
	if (reg_p & FL_T)
	{
		put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), zpindy_operand(reg_pc + 1)));
		Cycles += 10;
	}
	else
	{
		reg_a = adc(reg_a, zpindy_operand(reg_pc + 1));
		Cycles += 7;
	}
	reg_pc += 2;
}

OPCODE_FUNC and_abs(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp &= abs_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 8;
	}
	else
	{
		reg_a &= abs_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 5;
	}
	reg_pc += 3;
}

OPCODE_FUNC and_absx(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp &= absx_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 8;
	}
	else
	{
		reg_a &= absx_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 5;
	}
	reg_pc += 3;
}

OPCODE_FUNC and_absy(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp &= absy_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 8;
	}
	else
	{
		reg_a &= absy_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 5;
	}
	reg_pc += 3;
}

OPCODE_FUNC and_imm(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp &= imm_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 5;
	}
	else
	{
		reg_a &= imm_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 2;
	}
	reg_pc += 2;
}

OPCODE_FUNC and_zp(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp &= zp_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 7;
	}
	else
	{
		reg_a &= zp_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 4;
	}
	reg_pc += 2;
}

OPCODE_FUNC and_zpx(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp &= zpx_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 7;
	}
	else
	{
		reg_a &= zpx_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 4;
	}
	reg_pc += 2;
}

OPCODE_FUNC and_zpind(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp &= zpind_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 10;
	}
	else
	{
		reg_a &= zpind_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 7;
	}
	reg_pc += 2;
}

OPCODE_FUNC and_zpindx(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp &= zpindx_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 10;
	}
	else
	{
		reg_a &= zpindx_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 7;
	}
	reg_pc += 2;
}

OPCODE_FUNC and_zpindy(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp &= zpindy_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 10;
	}
	else
	{
		reg_a &= zpindy_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 7;
	}
	reg_pc += 2;
}

OPCODE_FUNC asl_a(void)
{
	UBYTE temp = reg_a;
	reg_a <<= 1;
	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp & 0x80) ? FL_C : 0) | flnz_list[reg_a];
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC asl_abs(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1);
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = temp1 << 1;

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | flnz_list[temp];
	pce_write8(temp_addr, temp);
	reg_pc += 3;
	Cycles += 7;
}

OPCODE_FUNC asl_absx(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1) + reg_x;
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = temp1 << 1;

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | flnz_list[temp];
	pce_write8(temp_addr, temp);
	reg_pc += 3;
	Cycles += 7;
}

OPCODE_FUNC asl_zp(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1);
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = temp1 << 1;

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | flnz_list[temp];
	put_8bit_zp(zp_addr, temp);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC asl_zpx(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1) + reg_x;
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = temp1 << 1;

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | flnz_list[temp];
	put_8bit_zp(zp_addr, temp);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC bbr(UBYTE bit)
{
	reg_p &= ~FL_T;
	if (zp_operand(reg_pc + 1) & (1 << bit))
	{
		reg_pc += 3;
		Cycles += 6;
	}
	else
	{
		reg_pc += (SBYTE)imm_operand(reg_pc + 2) + 3;
		Cycles += 8;
	}
}

OPCODE_FUNC bbs(UBYTE bit)
{
	reg_p &= ~FL_T;
	if (zp_operand(reg_pc + 1) & (1 << bit))
	{
		reg_pc += (SBYTE)imm_operand(reg_pc + 2) + 3;
		Cycles += 8;
	}
	else
	{
		reg_pc += 3;
		Cycles += 6;
	}
}

OPCODE_FUNC bcc(void)
{
	reg_p &= ~FL_T;
	if (reg_p & FL_C)
	{
		reg_pc += 2;
		Cycles += 2;
	}
	else
	{
		reg_pc += (SBYTE)imm_operand(reg_pc + 1) + 2;
		Cycles += 4;
	}
}

OPCODE_FUNC bcs(void)
{
	reg_p &= ~FL_T;
	if (reg_p & FL_C)
	{
		reg_pc += (SBYTE)imm_operand(reg_pc + 1) + 2;
		Cycles += 4;
	}
	else
	{
		reg_pc += 2;
		Cycles += 2;
	}
}

OPCODE_FUNC beq(void)
{
	reg_p &= ~FL_T;
	if (reg_p & FL_Z)
	{
		reg_pc += (SBYTE)imm_operand(reg_pc + 1) + 2;
		Cycles += 4;
	}
	else
	{
		reg_pc += 2;
		Cycles += 2;
	}
}

OPCODE_FUNC bit_abs(void)
{
	UBYTE temp = abs_operand(reg_pc + 1);
	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((reg_a & temp) ? 0 : FL_Z);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC bit_absx(void)
{
	UBYTE temp = absx_operand(reg_pc + 1);
	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((reg_a & temp) ? 0 : FL_Z);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC bit_imm(void)
{
	UBYTE temp = imm_operand(reg_pc + 1);
	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((reg_a & temp) ? 0 : FL_Z);
	reg_pc += 2;
	Cycles += 2;
}

OPCODE_FUNC bit_zp(void)
{
	UBYTE temp = zp_operand(reg_pc + 1);
	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((reg_a & temp) ? 0 : FL_Z);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC bit_zpx(void)
{
	UBYTE temp = zpx_operand(reg_pc + 1);
	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((reg_a & temp) ? 0 : FL_Z);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC bmi(void)
{
	reg_p &= ~FL_T;
	if (reg_p & FL_N)
	{
		reg_pc += (SBYTE)imm_operand(reg_pc + 1) + 2;
		Cycles += 4;
	}
	else
	{
		reg_pc += 2;
		Cycles += 2;
	}
}

OPCODE_FUNC bne(void)
{
	reg_p &= ~FL_T;
	if (reg_p & FL_Z)
	{
		reg_pc += 2;
		Cycles += 2;
	}
	else
	{
		reg_pc += (SBYTE)imm_operand(reg_pc + 1) + 2;
		Cycles += 4;
	}
}

OPCODE_FUNC bpl(void)
{
	reg_p &= ~FL_T;
	if (reg_p & FL_N)
	{
		reg_pc += 2;
		Cycles += 2;
	}
	else
	{
		reg_pc += (SBYTE)imm_operand(reg_pc + 1) + 2;
		Cycles += 4;
	}
}

OPCODE_FUNC bra(void)
{
	reg_p &= ~FL_T;
	reg_pc += (SBYTE)imm_operand(reg_pc + 1) + 2;
	Cycles += 4;
}

OPCODE_FUNC brk(void)
{
	MESSAGE_DEBUG("BRK opcode has been hit [PC = 0x%04x] at %s(%d)\n", reg_pc);
	reg_p &= ~FL_T;
	push_16bit(reg_pc + 2);
	push_8bit(reg_p | FL_B);
	reg_p = (reg_p & ~FL_D) | FL_I;
	reg_pc = pce_read16(VEC_BRK);
	Cycles += 8;
}

OPCODE_FUNC bsr(void)
{
	reg_p &= ~FL_T;
	push_16bit(reg_pc + 1);
	reg_pc += (SBYTE)imm_operand(reg_pc + 1) + 2;
	Cycles += 8;
}

OPCODE_FUNC bvc(void)
{
	reg_p &= ~FL_T;
	if (reg_p & FL_V)
	{
		reg_pc += 2;
		Cycles += 2;
	}
	else
	{
		reg_pc += (SBYTE)imm_operand(reg_pc + 1) + 2;
		Cycles += 4;
	}
}

OPCODE_FUNC bvs(void)
{
	reg_p &= ~FL_T;
	if (reg_p & FL_V)
	{
		reg_pc += (SBYTE)imm_operand(reg_pc + 1) + 2;
		Cycles += 4;
	}
	else
	{
		reg_pc += 2;
		Cycles += 2;
	}
}

OPCODE_FUNC cla(void)
{
	reg_p &= ~FL_T;
	reg_a = 0;
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC clc(void)
{
	reg_p &= ~(FL_T | FL_C);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC cld(void)
{
	reg_p &= ~(FL_T | FL_D);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC cli(void)
{
	reg_p &= ~(FL_T | FL_I);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC clv(void)
{
	reg_p &= ~(FL_V | FL_T);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC clx(void)
{
	reg_p &= ~FL_T;
	reg_x = 0;
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC cly(void)
{
	reg_p &= ~FL_T;
	reg_y = 0;
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC cmp_abs(void)
{
	UBYTE temp = abs_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_a < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_a - temp)];
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC cmp_absx(void)
{
	UBYTE temp = absx_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_a < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_a - temp)];
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC cmp_absy(void)
{
	UBYTE temp = absy_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_a < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_a - temp)];
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC cmp_imm(void)
{
	UBYTE temp = imm_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_a < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_a - temp)];
	reg_pc += 2;
	Cycles += 2;
}

OPCODE_FUNC cmp_zp(void)
{
	UBYTE temp = zp_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_a < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_a - temp)];
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC cmp_zpx(void)
{
	UBYTE temp = zpx_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_a < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_a - temp)];
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC cmp_zpind(void)
{
	UBYTE temp = zpind_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_a < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_a - temp)];
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC cmp_zpindx(void)
{
	UBYTE temp = zpindx_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_a < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_a - temp)];
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC cmp_zpindy(void)
{
	UBYTE temp = zpindy_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_a < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_a - temp)];
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC cpx_abs(void)
{
	UBYTE temp = abs_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_x < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_x - temp)];
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC cpx_imm(void)
{
	UBYTE temp = imm_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_x < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_x - temp)];
	reg_pc += 2;
	Cycles += 2;
}

OPCODE_FUNC cpx_zp(void)
{
	UBYTE temp = zp_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_x < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_x - temp)];
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC cpy_abs(void)
{
	UBYTE temp = abs_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_y < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_y - temp)];
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC cpy_imm(void)
{
	UBYTE temp = imm_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_y < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_y - temp)];
	reg_pc += 2;
	Cycles += 2;
}

OPCODE_FUNC cpy_zp(void)
{
	UBYTE temp = zp_operand(reg_pc + 1);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((reg_y < temp) ? 0 : FL_C) | flnz_list[(UBYTE)(reg_y - temp)];
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC dec_a(void)
{
	--reg_a;
	chk_flnz_8bit(reg_a);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC dec_abs(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1);
	UBYTE temp = pce_read8(temp_addr) - 1;
	chk_flnz_8bit(temp);
	pce_write8(temp_addr, temp);
	reg_pc += 3;
	Cycles += 7;
}

OPCODE_FUNC dec_absx(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1) + reg_x;
	UBYTE temp = pce_read8(temp_addr) - 1;
	chk_flnz_8bit(temp);
	pce_write8(temp_addr, temp);
	reg_pc += 3;
	Cycles += 7;
}

OPCODE_FUNC dec_zp(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1);
	UBYTE temp = get_8bit_zp(zp_addr) - 1;
	chk_flnz_8bit(temp);
	put_8bit_zp(zp_addr, temp);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC dec_zpx(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1) + reg_x;
	UBYTE temp = get_8bit_zp(zp_addr) - 1;
	chk_flnz_8bit(temp);
	put_8bit_zp(zp_addr, temp);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC dex(void)
{
	--reg_x;
	chk_flnz_8bit(reg_x);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC dey(void)
{
	--reg_y;
	chk_flnz_8bit(reg_y);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC eor_abs(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp ^= abs_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 8;
	}
	else
	{
		reg_a ^= abs_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 5;
	}
	reg_pc += 3;
}

OPCODE_FUNC eor_absx(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp ^= absx_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 8;
	}
	else
	{
		reg_a ^= absx_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 5;
	}
	reg_pc += 3;
}

OPCODE_FUNC eor_absy(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp ^= absy_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 8;
	}
	else
	{
		reg_a ^= absy_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 5;
	}
	reg_pc += 3;
}

OPCODE_FUNC eor_imm(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp ^= imm_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 5;
	}
	else
	{
		reg_a ^= imm_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 2;
	}
	reg_pc += 2;
}

OPCODE_FUNC eor_zp(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp ^= zp_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 7;
	}
	else
	{
		reg_a ^= zp_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 4;
	}
	reg_pc += 2;
}

OPCODE_FUNC eor_zpx(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp ^= zpx_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 7;
	}
	else
	{
		reg_a ^= zpx_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 4;
	}
	reg_pc += 2;
}

OPCODE_FUNC eor_zpind(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp ^= zpind_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 10;
	}
	else
	{
		reg_a ^= zpind_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 7;
	}
	reg_pc += 2;
}

OPCODE_FUNC eor_zpindx(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp ^= zpindx_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 10;
	}
	else
	{
		reg_a ^= zpindx_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 7;
	}
	reg_pc += 2;
}

OPCODE_FUNC eor_zpindy(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp ^= zpindy_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 10;
	}
	else
	{
		reg_a ^= zpindy_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 7;
	}
	reg_pc += 2;
}

OPCODE_FUNC halt(void)
{
	return;
}

OPCODE_FUNC inc_a(void)
{
	++reg_a;
	chk_flnz_8bit(reg_a);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC inc_abs(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1);
	UBYTE temp = pce_read8(temp_addr) + 1;
	chk_flnz_8bit(temp);
	pce_write8(temp_addr, temp);
	reg_pc += 3;
	Cycles += 7;
}

OPCODE_FUNC inc_absx(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1) + reg_x;
	UBYTE temp = pce_read8(temp_addr) + 1;
	chk_flnz_8bit(temp);
	pce_write8(temp_addr, temp);
	reg_pc += 3;
	Cycles += 7;
}

OPCODE_FUNC inc_zp(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1);
	UBYTE temp = get_8bit_zp(zp_addr) + 1;
	chk_flnz_8bit(temp);
	put_8bit_zp(zp_addr, temp);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC inc_zpx(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1) + reg_x;
	UBYTE temp = get_8bit_zp(zp_addr) + 1;
	chk_flnz_8bit(temp);
	put_8bit_zp(zp_addr, temp);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC inx(void)
{
	++reg_x;
	chk_flnz_8bit(reg_x);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC iny(void)
{
	++reg_y;
	chk_flnz_8bit(reg_y);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC jmp(void)
{
	reg_p &= ~FL_T;
	reg_pc = pce_read16(reg_pc + 1);
	Cycles += 4;
}

OPCODE_FUNC jmp_absind(void)
{
	reg_p &= ~FL_T;
	reg_pc = pce_read16(pce_read16(reg_pc + 1));
	Cycles += 7;
}

OPCODE_FUNC jmp_absindx(void)
{
	reg_p &= ~FL_T;
	reg_pc = pce_read16(pce_read16(reg_pc + 1) + reg_x);
	Cycles += 7;
}

OPCODE_FUNC jsr(void)
{
	reg_p &= ~FL_T;
	push_16bit(reg_pc + 2);
	reg_pc = pce_read16(reg_pc + 1);
	Cycles += 7;
}

OPCODE_FUNC lda_abs(void)
{
	reg_a = abs_operand(reg_pc + 1);
	chk_flnz_8bit(reg_a);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC lda_absx(void)
{
	reg_a = absx_operand(reg_pc + 1);
	chk_flnz_8bit(reg_a);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC lda_absy(void)
{
	reg_a = absy_operand(reg_pc + 1);
	chk_flnz_8bit(reg_a);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC lda_imm(void)
{
	reg_a = imm_operand(reg_pc + 1);
	chk_flnz_8bit(reg_a);
	reg_pc += 2;
	Cycles += 2;
}

OPCODE_FUNC lda_zp(void)
{
	reg_a = zp_operand(reg_pc + 1);
	chk_flnz_8bit(reg_a);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC lda_zpx(void)
{
	reg_a = zpx_operand(reg_pc + 1);
	chk_flnz_8bit(reg_a);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC lda_zpind(void)
{
	reg_a = zpind_operand(reg_pc + 1);
	chk_flnz_8bit(reg_a);
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC lda_zpindx(void)
{
	reg_a = zpindx_operand(reg_pc + 1);
	chk_flnz_8bit(reg_a);
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC lda_zpindy(void)
{
	reg_a = zpindy_operand(reg_pc + 1);
	chk_flnz_8bit(reg_a);
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC ldx_abs(void)
{
	reg_x = abs_operand(reg_pc + 1);
	chk_flnz_8bit(reg_x);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC ldx_absy(void)
{
	reg_x = absy_operand(reg_pc + 1);
	chk_flnz_8bit(reg_x);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC ldx_imm(void)
{
	reg_x = imm_operand(reg_pc + 1);
	chk_flnz_8bit(reg_x);
	reg_pc += 2;
	Cycles += 2;
}

OPCODE_FUNC ldx_zp(void)
{
	reg_x = zp_operand(reg_pc + 1);
	chk_flnz_8bit(reg_x);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC ldx_zpy(void)
{
	reg_x = zpy_operand(reg_pc + 1);
	chk_flnz_8bit(reg_x);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC ldy_abs(void)
{
	reg_y = abs_operand(reg_pc + 1);
	chk_flnz_8bit(reg_y);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC ldy_absx(void)
{
	reg_y = absx_operand(reg_pc + 1);
	chk_flnz_8bit(reg_y);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC ldy_imm(void)
{
	reg_y = imm_operand(reg_pc + 1);
	chk_flnz_8bit(reg_y);
	reg_pc += 2;
	Cycles += 2;
}

OPCODE_FUNC ldy_zp(void)
{
	reg_y = zp_operand(reg_pc + 1);
	chk_flnz_8bit(reg_y);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC ldy_zpx(void)
{
	reg_y = zpx_operand(reg_pc + 1);
	chk_flnz_8bit(reg_y);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC lsr_a(void)
{
	UBYTE temp = reg_a;
	reg_a /= 2;
	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp & 1) ? FL_C : 0) | flnz_list[reg_a];
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC lsr_abs(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1);
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = temp1 / 2;

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 1) ? FL_C : 0) | flnz_list[temp];
	pce_write8(temp_addr, temp);
	reg_pc += 3;
	Cycles += 7;
}

OPCODE_FUNC lsr_absx(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1) + reg_x;
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = temp1 / 2;

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 1) ? FL_C : 0) | flnz_list[temp];
	pce_write8(temp_addr, temp);
	reg_pc += 3;
	Cycles += 7;
}

OPCODE_FUNC lsr_zp(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1);
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = temp1 / 2;

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 1) ? FL_C : 0) | flnz_list[temp];
	put_8bit_zp(zp_addr, temp);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC lsr_zpx(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1) + reg_x;
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = temp1 / 2;

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 1) ? FL_C : 0) | flnz_list[temp];
	put_8bit_zp(zp_addr, temp);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC nop(void)
{
	reg_p &= ~FL_T;
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC ora_abs(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp |= abs_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 8;
	}
	else
	{
		reg_a |= abs_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 5;
	}
	reg_pc += 3;
}

OPCODE_FUNC ora_absx(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp |= absx_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 8;
	}
	else
	{
		reg_a |= absx_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 5;
	}
	reg_pc += 3;
}

OPCODE_FUNC ora_absy(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp |= absy_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 8;
	}
	else
	{
		reg_a |= absy_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 5;
	}
	reg_pc += 3;
}

OPCODE_FUNC ora_imm(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp |= imm_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 5;
	}
	else
	{
		reg_a |= imm_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 2;
	}
	reg_pc += 2;
}

OPCODE_FUNC ora_zp(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp |= zp_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 7;
	}
	else
	{
		reg_a |= zp_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 4;
	}
	reg_pc += 2;
}

OPCODE_FUNC ora_zpx(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp |= zpx_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 7;
	}
	else
	{
		reg_a |= zpx_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 4;
	}
	reg_pc += 2;
}

OPCODE_FUNC ora_zpind(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp |= zpind_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 10;
	}
	else
	{
		reg_a |= zpind_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 7;
	}
	reg_pc += 2;
}

OPCODE_FUNC ora_zpindx(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp |= zpindx_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 10;
	}
	else
	{
		reg_a |= zpindx_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 7;
	}
	reg_pc += 2;
}

OPCODE_FUNC ora_zpindy(void)
{
	if (reg_p & FL_T)
	{
		UBYTE temp = get_8bit_zp(reg_x);
		temp |= zpindy_operand(reg_pc + 1);
		chk_flnz_8bit(temp);
		put_8bit_zp(reg_x, temp);
		Cycles += 10;
	}
	else
	{
		reg_a |= zpindy_operand(reg_pc + 1);
		chk_flnz_8bit(reg_a);
		Cycles += 7;
	}
	reg_pc += 2;
}

OPCODE_FUNC pha(void)
{
	reg_p &= ~FL_T;
	push_8bit(reg_a);
	reg_pc++;
	Cycles += 3;
}

OPCODE_FUNC php(void)
{
	reg_p &= ~FL_T;
	push_8bit(reg_p);
	reg_pc++;
	Cycles += 3;
}

OPCODE_FUNC phx(void)
{
	reg_p &= ~FL_T;
	push_8bit(reg_x);
	reg_pc++;
	Cycles += 3;
}

OPCODE_FUNC phy(void)
{
	reg_p &= ~FL_T;
	push_8bit(reg_y);
	reg_pc++;
	Cycles += 3;
}

OPCODE_FUNC pla(void)
{
	reg_a = pull_8bit();
	chk_flnz_8bit(reg_a);
	reg_pc++;
	Cycles += 4;
}

OPCODE_FUNC plp(void)
{
	reg_p = pull_8bit();
	reg_pc++;
	Cycles += 4;
}

OPCODE_FUNC plx(void)
{
	reg_x = pull_8bit();
	chk_flnz_8bit(reg_x);
	reg_pc++;
	Cycles += 4;
}

OPCODE_FUNC ply(void)
{
	reg_y = pull_8bit();
	chk_flnz_8bit(reg_y);
	reg_pc++;
	Cycles += 4;
}

OPCODE_FUNC rmb(UBYTE bit)
{
	UBYTE temp = imm_operand(reg_pc + 1);
	reg_p &= ~FL_T;
	put_8bit_zp(temp, get_8bit_zp(temp) & (~(1 << bit)));
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC rol_a(void)
{
	UBYTE temp = reg_a;
	reg_a = (reg_a << 1) + (reg_p & FL_C);
	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp & 0x80) ? FL_C : 0) | flnz_list[reg_a];
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC rol_abs(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1);
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = (temp1 << 1) + (reg_p & FL_C);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | flnz_list[temp];
	Cycles += 7;
	pce_write8(temp_addr, temp);
	reg_pc += 3;
}

OPCODE_FUNC rol_absx(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1) + reg_x;
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = (temp1 << 1) + (reg_p & FL_C);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | flnz_list[temp];
	Cycles += 7;
	pce_write8(temp_addr, temp);
	reg_pc += 3;
}

OPCODE_FUNC rol_zp(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1);
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = (temp1 << 1) + (reg_p & FL_C);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | flnz_list[temp];
	put_8bit_zp(zp_addr, temp);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC rol_zpx(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1) + reg_x;
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = (temp1 << 1) + (reg_p & FL_C);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x80) ? FL_C : 0) | flnz_list[temp];
	put_8bit_zp(zp_addr, temp);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC ror_a(void)
{
	UBYTE temp = reg_a;
	reg_a = (reg_a >> 1) + ((reg_p & FL_C) ? 0x80 : 0);
	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp & 0x01) ? FL_C : 0) | flnz_list[reg_a];
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC ror_abs(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1);
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = (temp1 >> 1) + ((reg_p & FL_C) ? 0x80 : 0);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x01) ? FL_C : 0) | flnz_list[temp];
	Cycles += 7;
	pce_write8(temp_addr, temp);
	reg_pc += 3;
}

OPCODE_FUNC ror_absx(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1) + reg_x;
	UBYTE temp1 = pce_read8(temp_addr);
	UBYTE temp = (temp1 >> 1) + ((reg_p & FL_C) ? 0x80 : 0);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x01) ? FL_C : 0) | flnz_list[temp];
	Cycles += 7;
	pce_write8(temp_addr, temp);
	reg_pc += 3;
}

OPCODE_FUNC ror_zp(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1);
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = (temp1 >> 1) + ((reg_p & FL_C) ? 0x80 : 0);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x01) ? FL_C : 0) | flnz_list[temp];
	put_8bit_zp(zp_addr, temp);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC ror_zpx(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1) + reg_x;
	UBYTE temp1 = get_8bit_zp(zp_addr);
	UBYTE temp = (temp1 >> 1) + ((reg_p & FL_C) ? 0x80 : 0);

	reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) | ((temp1 & 0x01) ? FL_C : 0) | flnz_list[temp];
	put_8bit_zp(zp_addr, temp);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC rti(void)
{
	/* FL_B reset in RTI */
	reg_p = pull_8bit() & ~FL_B;
	reg_pc = pull_16bit();
	Cycles += 7;
}

OPCODE_FUNC rts(void)
{
	reg_p &= ~FL_T;
	reg_pc = pull_16bit() + 1;
	Cycles += 7;
}

OPCODE_FUNC sax(void)
{
	UBYTE temp = reg_x;
	reg_p &= ~FL_T;
	reg_x = reg_a;
	reg_a = temp;
	reg_pc++;
	Cycles += 3;
}

OPCODE_FUNC say(void)
{
	UBYTE temp = reg_y;
	reg_p &= ~FL_T;
	reg_y = reg_a;
	reg_a = temp;
	reg_pc++;
	Cycles += 3;
}

OPCODE_FUNC sbc_abs(void)
{
	sbc(abs_operand(reg_pc + 1));
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC sbc_absx(void)
{
	sbc(absx_operand(reg_pc + 1));
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC sbc_absy(void)
{
	sbc(absy_operand(reg_pc + 1));
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC sbc_imm(void)
{
	sbc(imm_operand(reg_pc + 1));
	reg_pc += 2;
	Cycles += 2;
}

OPCODE_FUNC sbc_zp(void)
{
	sbc(zp_operand(reg_pc + 1));
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC sbc_zpx(void)
{
	sbc(zpx_operand(reg_pc + 1));
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC sbc_zpind(void)
{
	sbc(zpind_operand(reg_pc + 1));
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC sbc_zpindx(void)
{
	sbc(zpindx_operand(reg_pc + 1));
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC sbc_zpindy(void)
{
	sbc(zpindy_operand(reg_pc + 1));
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC sec(void)
{
	reg_p = (reg_p | FL_C) & ~FL_T;
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC sed(void)
{
	reg_p = (reg_p | FL_D) & ~FL_T;
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC sei(void)
{
	reg_p = (reg_p | FL_I) & ~FL_T;
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC set(void)
{
	reg_p |= FL_T;
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC smb(UBYTE bit)
{
	UBYTE temp = imm_operand(reg_pc + 1);
	reg_p &= ~FL_T;
	put_8bit_zp(temp, get_8bit_zp(temp) | (1 << bit));
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC st0(void)
{
	reg_p &= ~FL_T;
	IO_write(0, imm_operand(reg_pc + 1));
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC st1(void)
{
	reg_p &= ~FL_T;
	IO_write(2, imm_operand(reg_pc + 1));
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC st2(void)
{
	reg_p &= ~FL_T;
	IO_write(3, imm_operand(reg_pc + 1));
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC sta_abs(void)
{
	reg_p &= ~FL_T;
	pce_write8(pce_read16(reg_pc + 1), reg_a);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC sta_absx(void)
{
	reg_p &= ~FL_T;
	pce_write8(pce_read16(reg_pc + 1) + reg_x, reg_a);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC sta_absy(void)
{
	reg_p &= ~FL_T;
	pce_write8(pce_read16(reg_pc + 1) + reg_y, reg_a);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC sta_zp(void)
{
	reg_p &= ~FL_T;
	put_8bit_zp(imm_operand(reg_pc + 1), reg_a);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC sta_zpx(void)
{
	reg_p &= ~FL_T;
	put_8bit_zp(imm_operand(reg_pc + 1) + reg_x, reg_a);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC sta_zpind(void)
{
	reg_p &= ~FL_T;
	pce_write8(get_16bit_zp(imm_operand(reg_pc + 1)), reg_a);
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC sta_zpindx(void)
{
	reg_p &= ~FL_T;
	pce_write8(get_16bit_zp(imm_operand(reg_pc + 1) + reg_x), reg_a);
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC sta_zpindy(void)
{
	reg_p &= ~FL_T;
	pce_write8(get_16bit_zp(imm_operand(reg_pc + 1)) + reg_y, reg_a);
	reg_pc += 2;
	Cycles += 7;
}

OPCODE_FUNC stx_abs(void)
{
	reg_p &= ~FL_T;
	pce_write8(pce_read16(reg_pc + 1), reg_x);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC stx_zp(void)
{
	reg_p &= ~FL_T;
	put_8bit_zp(imm_operand(reg_pc + 1), reg_x);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC stx_zpy(void)
{
	reg_p &= ~FL_T;
	put_8bit_zp(imm_operand(reg_pc + 1) + reg_y, reg_x);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC sty_abs(void)
{
	reg_p &= ~FL_T;
	pce_write8(pce_read16(reg_pc + 1), reg_y);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC sty_zp(void)
{
	reg_p &= ~FL_T;
	put_8bit_zp(imm_operand(reg_pc + 1), reg_y);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC sty_zpx(void)
{
	reg_p &= ~FL_T;
	put_8bit_zp(imm_operand(reg_pc + 1) + reg_x, reg_y);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC stz_abs(void)
{
	reg_p &= ~FL_T;
	pce_write8(pce_read16(reg_pc + 1), 0);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC stz_absx(void)
{
	reg_p &= ~FL_T;
	pce_write8((pce_read16(reg_pc + 1) + reg_x), 0);
	reg_pc += 3;
	Cycles += 5;
}

OPCODE_FUNC stz_zp(void)
{
	reg_p &= ~FL_T;
	put_8bit_zp(imm_operand(reg_pc + 1), 0);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC stz_zpx(void)
{
	reg_p &= ~FL_T;
	put_8bit_zp(imm_operand(reg_pc + 1) + reg_x, 0);
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC sxy(void)
{
	UBYTE temp = reg_y;
	reg_p &= ~FL_T;
	reg_y = reg_x;
	reg_x = temp;
	reg_pc++;
	Cycles += 3;
}

OPCODE_FUNC tai(void)
{
	reg_p &= ~FL_T;
	UWORD from = pce_read16(reg_pc + 1);
	UWORD to = pce_read16(reg_pc + 3);
	UWORD len = pce_read16(reg_pc + 5);
	UWORD alternate = 0;

	Cycles += (6 * len) + 17;
	while (len-- != 0)
	{
		pce_write8(to++, pce_read8(from + alternate));
		alternate ^= 1;
	}
	reg_pc += 7;
}

OPCODE_FUNC tam(void)
{
	UBYTE bitfld = imm_operand(reg_pc + 1);

	for (int i = 0; i < 8; i++)
	{
		if (bitfld & (1 << i))
		{
			pce_bank_set(i, reg_a);
		}
	}

	reg_p &= ~FL_T;
	reg_pc += 2;
	Cycles += 5;
}

OPCODE_FUNC tax(void)
{
	reg_x = reg_a;
	chk_flnz_8bit(reg_a);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC tay(void)
{
	reg_y = reg_a;
	chk_flnz_8bit(reg_a);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC tdd(void)
{
	reg_p &= ~FL_T;
	UWORD from = pce_read16(reg_pc + 1);
	UWORD to = pce_read16(reg_pc + 3);
	UWORD len = pce_read16(reg_pc + 5);

	Cycles += (6 * len) + 17;
	while (len-- != 0)
	{
		pce_write8(to--, pce_read8(from--));
	}
	reg_pc += 7;
}

OPCODE_FUNC tia(void)
{
	reg_p &= ~FL_T;
	UWORD from = pce_read16(reg_pc + 1);
	UWORD to = pce_read16(reg_pc + 3);
	UWORD len = pce_read16(reg_pc + 5);
	UWORD alternate = 0;

	Cycles += (6 * len) + 17;
	while (len-- != 0)
	{
		pce_write8(to + alternate, pce_read8(from++));
		alternate ^= 1;
	}
	reg_pc += 7;
}

OPCODE_FUNC tii(void)
{
	reg_p &= ~FL_T;
	UWORD from = pce_read16(reg_pc + 1);
	UWORD to = pce_read16(reg_pc + 3);
	UWORD len = pce_read16(reg_pc + 5);

	Cycles += (6 * len) + 17;
	while (len-- != 0)
	{
		pce_write8(to++, pce_read8(from++));
	}
	reg_pc += 7;
}

OPCODE_FUNC tin(void)
{
	reg_p &= ~FL_T;
	UWORD from = pce_read16(reg_pc + 1);
	UWORD to = pce_read16(reg_pc + 3);
	UWORD len = pce_read16(reg_pc + 5);

	Cycles += (6 * len) + 17;
	while (len-- != 0)
	{
		pce_write8(to, pce_read8(from++));
	}
	reg_pc += 7;
}

OPCODE_FUNC tma(void)
{
	UBYTE bitfld = imm_operand(reg_pc + 1);

	for (int i = 0; i < 8; i++)
	{
		if (bitfld & (1 << i))
		{
			reg_a = MMR[i];
		}
	}
	reg_p &= ~FL_T;
	reg_pc += 2;
	Cycles += 4;
}

OPCODE_FUNC trb_abs(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1);
	UBYTE temp = pce_read8(temp_addr);
	UBYTE temp1 = (~reg_a) & temp;

	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp1 & (FL_N | FL_V)) | ((temp & reg_a) ? 0 : FL_Z);
	pce_write8(temp_addr, temp1);
	reg_pc += 3;
	Cycles += 7;
}

OPCODE_FUNC trb_zp(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1);
	UBYTE temp = get_8bit_zp(zp_addr);
	UBYTE temp1 = (~reg_a) & temp;

	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp1 & (FL_N | FL_V)) | ((temp & reg_a) ? 0 : FL_Z);
	put_8bit_zp(zp_addr, temp1);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC tsb_abs(void)
{
	UWORD temp_addr = pce_read16(reg_pc + 1);
	UBYTE temp = pce_read8(temp_addr);
	UBYTE temp1 = reg_a | temp;

	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp1 & (FL_N | FL_V)) | ((temp & reg_a) ? 0 : FL_Z);
	pce_write8(temp_addr, temp1);
	reg_pc += 3;
	Cycles += 7;
}

OPCODE_FUNC tsb_zp(void)
{
	UBYTE zp_addr = imm_operand(reg_pc + 1);
	UBYTE temp = get_8bit_zp(zp_addr);
	UBYTE temp1 = reg_a | temp;

	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp1 & (FL_N | FL_V)) | ((temp & reg_a) ? 0 : FL_Z);
	put_8bit_zp(zp_addr, temp1);
	reg_pc += 2;
	Cycles += 6;
}

OPCODE_FUNC tstins_abs(void)
{
	UBYTE imm_addr = imm_operand(reg_pc + 1);
	UBYTE temp = abs_operand(reg_pc + 2);

	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((temp & imm_addr) ? 0 : FL_Z);
	reg_pc += 4;
	Cycles += 8;
}

OPCODE_FUNC tstins_absx(void)
{
	UBYTE imm_addr = imm_operand(reg_pc + 1);
	UBYTE temp = absx_operand(reg_pc + 2);

	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((temp & imm_addr) ? 0 : FL_Z);
	reg_pc += 4;
	Cycles += 8;
}

OPCODE_FUNC tstins_zp(void)
{
	UBYTE imm_addr = imm_operand(reg_pc + 1);
	UBYTE temp = zp_operand(reg_pc + 2);

	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((temp & imm_addr) ? 0 : FL_Z);
	reg_pc += 3;
	Cycles += 7;
}

OPCODE_FUNC tstins_zpx(void)
{
	UBYTE imm_addr = imm_operand(reg_pc + 1);
	UBYTE temp = zpx_operand(reg_pc + 2);

	reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) | (temp & (FL_N | FL_V)) | ((temp & imm_addr) ? 0 : FL_Z);
	reg_pc += 3;
	Cycles += 7;
}

OPCODE_FUNC tsx(void)
{
	reg_x = reg_s;
	chk_flnz_8bit(reg_s);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC txa(void)
{
	reg_a = reg_x;
	chk_flnz_8bit(reg_x);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC txs(void)
{
	reg_p &= ~FL_T;
	reg_s = reg_x;
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC tya(void)
{
	reg_a = reg_y;
	chk_flnz_8bit(reg_y);
	reg_pc++;
	Cycles += 2;
}

OPCODE_FUNC interrupt(int type)
{
	// pcetech.txt says the program should clear the irq line by reading 0x1403
	// however in practice it seems to break many games if we don't clear it?

	TRACE_CPU("CPU interrupt: %d\n", irq);
	push_16bit(reg_pc);
	push_8bit(reg_p);
	reg_p &= ~(FL_D|FL_T);
	reg_p |= FL_I;
	if (type & INT_IRQ1) {
		io.irq_status &= ~INT_IRQ1;
		reg_pc = pce_read16(VEC_IRQ1);
	} else if (type & INT_IRQ2) {
		io.irq_status &= ~INT_IRQ2;
		reg_pc = pce_read16(VEC_IRQ2);
	} else {
		io.irq_status &= ~VEC_TIMER;
		reg_pc = pce_read16(VEC_TIMER);
	}
	Cycles += 7;
}

#if 0
/* now define table contents: */
operation_t optable_runtime[256] = {
	{brek, AM_IMMED, "BRK"},			/* $00 */
	{ora_zpindx, AM_ZPINDX, "ORA"},		/* $01 */
	{sxy, AM_IMPL, "SXY"},				/* $02 */
	{st0, AM_IMMED, "ST0"},				/* $03 */
	{tsb_zp, AM_ZP, "TSB"},				/* $04 */
	{ora_zp, AM_ZP, "ORA"},				/* $05 */
	{asl_zp, AM_ZP, "ASL"},				/* $06 */
	{rmb0, AM_ZP, "RMB0"},				/* $07 */
	{php, AM_IMPL, "PHP"},				/* $08 */
	{ora_imm, AM_IMMED, "ORA"},			/* $09 */
	{asl_a, AM_IMPL, "ASL"},			/* $0A */
	{handle_bp0, AM_IMPL, "BP0"},		/* $0B */
	{tsb_abs, AM_ABS, "TSB"},			/* $0C */
	{ora_abs, AM_ABS, "ORA"},			/* $0D */
	{asl_abs, AM_ABS, "ASL"},			/* $0E */
	{bbr0, AM_PSREL, "BBR0"},			/* $0F */
	{bpl, AM_REL, "BPL"},				/* $10 */
	{ora_zpindy, AM_ZPINDY, "ORA"},		/* $11 */
	{ora_zpind, AM_ZPIND, "ORA"},		/* $12 */
	{st1, AM_IMMED, "ST1"},				/* $13 */
	{trb_zp, AM_ZP, "TRB"},				/* $14 */
	{ora_zpx, AM_ZPX, "ORA"},			/* $15 */
	{asl_zpx, AM_ZPX, "ASL"},			/* $16 */
	{rmb1, AM_ZP, "RMB1"},				/* $17 */
	{clc, AM_IMPL, "CLC"},				/* $18 */
	{ora_absy, AM_ABSY, "ORA"},			/* $19 */
	{inc_a, AM_IMPL, "INC"},			/* $1A */
	{handle_bp1, AM_IMPL, "BP1"},		/* $1B */
	{trb_abs, AM_ABS, "TRB"},			/* $1C */
	{ora_absx, AM_ABSX, "ORA"},			/* $1D */
	{asl_absx, AM_ABSX, "ASL"},			/* $1E */
	{bbr1, AM_PSREL, "BBR1"},			/* $1F */
	{jsr, AM_ABS, "JSR"},				/* $20 */
	{and_zpindx, AM_ZPINDX, "AND"},		/* $21 */
	{sax, AM_IMPL, "SAX"},				/* $22 */
	{st2, AM_IMMED, "ST2"},				/* $23 */
	{bit_zp, AM_ZP, "BIT"},				/* $24 */
	{and_zp, AM_ZP, "AND"},				/* $25 */
	{rol_zp, AM_ZP, "ROL"},				/* $26 */
	{rmb2, AM_ZP, "RMB2"},				/* $27 */
	{plp, AM_IMPL, "PLP"},				/* $28 */
	{and_imm, AM_IMMED, "AND"},			/* $29 */
	{rol_a, AM_IMPL, "ROL"},			/* $2A */
	{handle_bp2, AM_IMPL, "BP2"},		/* $2B */
	{bit_abs, AM_ABS, "BIT"},			/* $2C */
	{and_abs, AM_ABS, "AND"},			/* $2D */
	{rol_abs, AM_ABS, "ROL"},			/* $2E */
	{bbr2, AM_PSREL, "BBR2"},			/* $2F */
	{bmi, AM_REL, "BMI"},				/* $30 */
	{and_zpindy, AM_ZPINDY, "AND"},		/* $31 */
	{and_zpind, AM_ZPIND, "AND"},		/* $32 */
	{halt, AM_IMPL, "???"},				/* $33 */
	{bit_zpx, AM_ZPX, "BIT"},			/* $34 */
	{and_zpx, AM_ZPX, "AND"},			/* $35 */
	{rol_zpx, AM_ZPX, "ROL"},			/* $36 */
	{rmb3, AM_ZP, "RMB3"},				/* $37 */
	{sec, AM_IMPL, "SEC"},				/* $38 */
	{and_absy, AM_ABSY, "AND"},			/* $39 */
	{dec_a, AM_IMPL, "DEC"},			/* $3A */
	{handle_bp3, AM_IMPL, "BP3"},		/* $3B */
	{bit_absx, AM_ABSX, "BIT"},			/* $3C */
	{and_absx, AM_ABSX, "AND"},			/* $3D */
	{rol_absx, AM_ABSX, "ROL"},			/* $3E */
	{bbr3, AM_PSREL, "BBR3"},			/* $3F */
	{rti, AM_IMPL, "RTI"},				/* $40 */
	{eor_zpindx, AM_ZPINDX, "EOR"},		/* $41 */
	{say, AM_IMPL, "SAY"},				/* $42 */
	{tma, AM_IMMED, "TMA"},				/* $43 */
	{bsr, AM_REL, "BSR"},				/* $44 */
	{eor_zp, AM_ZP, "EOR"},				/* $45 */
	{lsr_zp, AM_ZP, "LSR"},				/* $46 */
	{rmb4, AM_ZP, "RMB4"},				/* $47 */
	{pha, AM_IMPL, "PHA"},				/* $48 */
	{eor_imm, AM_IMMED, "EOR"},			/* $49 */
	{lsr_a, AM_IMPL, "LSR"},			/* $4A */
	{handle_bp4, AM_IMPL, "BP4"},		/* $4B */
	{jmp, AM_ABS, "JMP"},				/* $4C */
	{eor_abs, AM_ABS, "EOR"},			/* $4D */
	{lsr_abs, AM_ABS, "LSR"},			/* $4E */
	{bbr4, AM_PSREL, "BBR4"},			/* $4F */
	{bvc, AM_REL, "BVC"},				/* $50 */
	{eor_zpindy, AM_ZPINDY, "EOR"},		/* $51 */
	{eor_zpind, AM_ZPIND, "EOR"},		/* $52 */
	{tam, AM_IMMED, "TAM"},				/* $53 */
	{nop, AM_IMPL, "CSL"},				/* $54 */
	{eor_zpx, AM_ZPX, "EOR"},			/* $55 */
	{lsr_zpx, AM_ZPX, "LSR"},			/* $56 */
	{rmb5, AM_ZP, "RMB5"},				/* $57 */
	{cli, AM_IMPL, "CLI"},				/* $58 */
	{eor_absy, AM_ABSY, "EOR"},			/* $59 */
	{phy, AM_IMPL, "PHY"},				/* $5A */
	{handle_bp5, AM_IMPL, "BP5"},		/* $5B */
	{halt, AM_IMPL, "???"},				/* $5C */
	{eor_absx, AM_ABSX, "EOR"},			/* $5D */
	{lsr_absx, AM_ABSX, "LSR"},			/* $5E */
	{bbr5, AM_PSREL, "BBR5"},			/* $5F */
	{rts, AM_IMPL, "RTS"},				/* $60 */
	{adc_zpindx, AM_ZPINDX, "ADC"},		/* $61 */
	{cla, AM_IMPL, "CLA"},				/* $62 */
	{halt, AM_IMPL, "???"},				/* $63 */
	{stz_zp, AM_ZP, "STZ"},				/* $64 */
	{adc_zp, AM_ZP, "ADC"},				/* $65 */
	{ror_zp, AM_ZP, "ROR"},				/* $66 */
	{rmb6, AM_ZP, "RMB6"},				/* $67 */
	{pla, AM_IMPL, "PLA"},				/* $68 */
	{adc_imm, AM_IMMED, "ADC"},			/* $69 */
	{ror_a, AM_IMPL, "ROR"},			/* $6A */
	{handle_bp6, AM_IMPL, "BP6"},		/* $6B */
	{jmp_absind, AM_ABSIND, "JMP"},		/* $6C */
	{adc_abs, AM_ABS, "ADC"},			/* $6D */
	{ror_abs, AM_ABS, "ROR"},			/* $6E */
	{bbr6, AM_PSREL, "BBR6"},			/* $6F */
	{bvs, AM_REL, "BVS"},				/* $70 */
	{adc_zpindy, AM_ZPINDY, "ADC"},		/* $71 */
	{adc_zpind, AM_ZPIND, "ADC"},		/* $72 */
	{tii, AM_XFER, "TII"},				/* $73 */
	{stz_zpx, AM_ZPX, "STZ"},			/* $74 */
	{adc_zpx, AM_ZPX, "ADC"},			/* $75 */
	{ror_zpx, AM_ZPX, "ROR"},			/* $76 */
	{rmb7, AM_ZP, "RMB7"},				/* $77 */
	{sei, AM_IMPL, "SEI"},				/* $78 */
	{adc_absy, AM_ABSY, "ADC"},			/* $79 */
	{ply, AM_IMPL, "PLY"},				/* $7A */
	{handle_bp7, AM_IMPL, "BP7"},		/* $7B */
	{jmp_absindx, AM_ABSINDX, "JMP"},	/* $7C */
	{adc_absx, AM_ABSX, "ADC"},			/* $7D */
	{ror_absx, AM_ABSX, "ROR"},			/* $7E */
	{bbr7, AM_PSREL, "BBR7"},			/* $7F */
	{bra, AM_REL, "BRA"},				/* $80 */
	{sta_zpindx, AM_ZPINDX, "STA"},		/* $81 */
	{clx, AM_IMPL, "CLX"},				/* $82 */
	{tstins_zp, AM_TST_ZP, "TST"},		/* $83 */
	{sty_zp, AM_ZP, "STY"},				/* $84 */
	{sta_zp, AM_ZP, "STA"},				/* $85 */
	{stx_zp, AM_ZP, "STX"},				/* $86 */
	{smb0, AM_ZP, "SMB0"},				/* $87 */
	{dey, AM_IMPL, "DEY"},				/* $88 */
	{bit_imm, AM_IMMED, "BIT"},			/* $89 */
	{txa, AM_IMPL, "TXA"},				/* $8A */
	{handle_bp8, AM_IMPL, "BP8"},		/* $8B */
	{sty_abs, AM_ABS, "STY"},			/* $8C */
	{sta_abs, AM_ABS, "STA"},			/* $8D */
	{stx_abs, AM_ABS, "STX"},			/* $8E */
	{bbs0, AM_PSREL, "BBS0"},			/* $8F */
	{bcc, AM_REL, "BCC"},				/* $90 */
	{sta_zpindy, AM_ZPINDY, "STA"},		/* $91 */
	{sta_zpind, AM_ZPIND, "STA"},		/* $92 */
	{tstins_abs, AM_TST_ABS, "TST"},	/* $93 */
	{sty_zpx, AM_ZPX, "STY"},			/* $94 */
	{sta_zpx, AM_ZPX, "STA"},			/* $95 */
	{stx_zpy, AM_ZPY, "STX"},			/* $96 */
	{smb1, AM_ZP, "SMB1"},				/* $97 */
	{tya, AM_IMPL, "TYA"},				/* $98 */
	{sta_absy, AM_ABSY, "STA"},			/* $99 */
	{txs, AM_IMPL, "TXS"},				/* $9A */
	{handle_bp9, AM_IMPL, "BP9"},		/* $9B */
	{stz_abs, AM_ABS, "STZ"},			/* $9C */
	{sta_absx, AM_ABSX, "STA"},			/* $9D */
	{stz_absx, AM_ABSX, "STZ"},			/* $9E */
	{bbs1, AM_PSREL, "BBS1"},			/* $9F */
	{ldy_imm, AM_IMMED, "LDY"},			/* $A0 */
	{lda_zpindx, AM_ZPINDX, "LDA"},		/* $A1 */
	{ldx_imm, AM_IMMED, "LDX"},			/* $A2 */
	{tstins_zpx, AM_TST_ZPX, "TST"},	/* $A3 */
	{ldy_zp, AM_ZP, "LDY"},				/* $A4 */
	{lda_zp, AM_ZP, "LDA"},				/* $A5 */
	{ldx_zp, AM_ZP, "LDX"},				/* $A6 */
	{smb2, AM_ZP, "SMB2"},				/* $A7 */
	{tay, AM_IMPL, "TAY"},				/* $A8 */
	{lda_imm, AM_IMMED, "LDA"},			/* $A9 */
	{tax, AM_IMPL, "TAX"},				/* $AA */
	{handle_bp10, AM_IMPL, "BPA"},		/* $AB */
	{ldy_abs, AM_ABS, "LDY"},			/* $AC */
	{lda_abs, AM_ABS, "LDA"},			/* $AD */
	{ldx_abs, AM_ABS, "LDX"},			/* $AE */
	{bbs2, AM_PSREL, "BBS2"},			/* $AF */
	{bcs, AM_REL, "BCS"},				/* $B0 */
	{lda_zpindy, AM_ZPINDY, "LDA"},		/* $B1 */
	{lda_zpind, AM_ZPIND, "LDA"},		/* $B2 */
	{tstins_absx, AM_TST_ABSX, "TST"},	/* $B3 */
	{ldy_zpx, AM_ZPX, "LDY"},			/* $B4 */
	{lda_zpx, AM_ZPX, "LDA"},			/* $B5 */
	{ldx_zpy, AM_ZPY, "LDX"},			/* $B6 */
	{smb3, AM_ZP, "SMB3"},				/* $B7 */
	{clv, AM_IMPL, "CLV"},				/* $B8 */
	{lda_absy, AM_ABSY, "LDA"},			/* $B9 */
	{tsx, AM_IMPL, "TSX"},				/* $BA */
	{handle_bp11, AM_IMPL, "BPB"},		/* $BB */
	{ldy_absx, AM_ABSX, "LDY"},			/* $BC */
	{lda_absx, AM_ABSX, "LDA"},			/* $BD */
	{ldx_absy, AM_ABSY, "LDX"},			/* $BE */
	{bbs3, AM_PSREL, "BBS3"},			/* $BF */
	{cpy_imm, AM_IMMED, "CPY"},			/* $C0 */
	{cmp_zpindx, AM_ZPINDX, "CMP"},		/* $C1 */
	{cly, AM_IMPL, "CLY"},				/* $C2 */
	{tdd, AM_XFER, "TDD"},				/* $C3 */
	{cpy_zp, AM_ZP, "CPY"},				/* $C4 */
	{cmp_zp, AM_ZP, "CMP"},				/* $C5 */
	{dec_zp, AM_ZP, "DEC"},				/* $C6 */
	{smb4, AM_ZP, "SMB4"},				/* $C7 */
	{iny, AM_IMPL, "INY"},				/* $C8 */
	{cmp_imm, AM_IMMED, "CMP"},			/* $C9 */
	{dex, AM_IMPL, "DEX"},				/* $CA */
	{handle_bp12, AM_IMPL, "BPC"},		/* $CB */
	{cpy_abs, AM_ABS, "CPY"},			/* $CC */
	{cmp_abs, AM_ABS, "CMP"},			/* $CD */
	{dec_abs, AM_ABS, "DEC"},			/* $CE */
	{bbs4, AM_PSREL, "BBS4"},			/* $CF */
	{bne, AM_REL, "BNE"},				/* $D0 */
	{cmp_zpindy, AM_ZPINDY, "CMP"},		/* $D1 */
	{cmp_zpind, AM_ZPIND, "CMP"},		/* $D2 */
	{tin, AM_XFER, "TIN"},				/* $D3 */
	{nop, AM_IMPL, "CSH"},				/* $D4 */
	{cmp_zpx, AM_ZPX, "CMP"},			/* $D5 */
	{dec_zpx, AM_ZPX, "DEC"},			/* $D6 */
	{smb5, AM_ZP, "SMB5"},				/* $D7 */
	{cld, AM_IMPL, "CLD"},				/* $D8 */
	{cmp_absy, AM_ABSY, "CMP"},			/* $D9 */
	{phx, AM_IMPL, "PHX"},				/* $DA */
	{handle_bp13, AM_IMPL, "BPD"},		/* $DB */
	{halt, AM_IMPL, "???"},				/* $DC */
	{cmp_absx, AM_ABSX, "CMP"},			/* $DD */
	{dec_absx, AM_ABSX, "DEC"},			/* $DE */
	{bbs5, AM_PSREL, "BBS5"},			/* $DF */
	{cpx_imm, AM_IMMED, "CPX"},			/* $E0 */
	{sbc_zpindx, AM_ZPINDX, "SBC"},		/* $E1 */
	{halt, AM_IMPL, "???"},				/* $E2 */
	{tia, AM_XFER, "TIA"},				/* $E3 */
	{cpx_zp, AM_ZP, "CPX"},				/* $E4 */
	{sbc_zp, AM_ZP, "SBC"},				/* $E5 */
	{inc_zp, AM_ZP, "INC"},				/* $E6 */
	{smb6, AM_ZP, "SMB6"},				/* $E7 */
	{inx, AM_IMPL, "INX"},				/* $E8 */
	{sbc_imm, AM_IMMED, "SBC"},			/* $E9 */
	{nop, AM_IMPL, "NOP"},				/* $EA */
	{handle_bp14, AM_IMPL, "BPE"},		/* $EB */
	{cpx_abs, AM_ABS, "CPX"},			/* $EC */
	{sbc_abs, AM_ABS, "SBC"},			/* $ED */
	{inc_abs, AM_ABS, "INC"},			/* $EE */
	{bbs6, AM_PSREL, "BBS6"},			/* $EF */
	{beq, AM_REL, "BEQ"},				/* $F0 */
	{sbc_zpindy, AM_ZPINDY, "SBC"},		/* $F1 */
	{sbc_zpind, AM_ZPIND, "SBC"},		/* $F2 */
	{tai, AM_XFER, "TAI"},				/* $F3 */
	{set, AM_IMPL, "SET"},				/* $F4 */
	{sbc_zpx, AM_ZPX, "SBC"},			/* $F5 */
	{inc_zpx, AM_ZPX, "INC"},			/* $F6 */
	{smb7, AM_ZP, "SMB7"},				/* $F7 */
	{sed, AM_IMPL, "SED"},				/* $F8 */
	{sbc_absy, AM_ABSY, "SBC"},			/* $F9 */
	{plx, AM_IMPL, "PLX"},				/* $FA */
	{handle_bp15, AM_IMPL, "BPF"},		/* $FB */
	{halt, AM_IMPL, "???"},				/* $FC */
	{sbc_absx, AM_ABSX, "SBC"},			/* $FD */
	{inc_absx, AM_ABSX, "INC"},			/* $FE */
	{bbs7, AM_PSREL, "BBS7"}			/* $FF */
};
#endif