//  h6280.c - CPU Emulation
//
#include <stdio.h>
#include <stdlib.h>

#include "hard_pce.h"
#include "gfx.h"
#include "pce.h"
#include "utils.h"

// flag-value table (for speed)
DRAM_ATTR const uchar flnz_list[256] = {
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

#define OP_CALL_THROUGH_LOOKUP (*optable_runtime[opcode].func_exe)();

// pointer to the beginning of the Zero Page area
#define zp_base (RAM)

// pointer to the beginning of the Stack Area
#define sp_base (RAM + 0x100)

// Addressing modes:
#define imm_operand(addr) ((uchar) (PageR[(addr) >> 13][(addr)]))
#define abs_operand(x)     get_8bit_addr(get_16bit_addr(x))
#define absx_operand(x)    get_8bit_addr(get_16bit_addr(x)+reg_x)
#define absy_operand(x)    get_8bit_addr(get_16bit_addr(x)+reg_y)
#define zp_operand(x)      get_8bit_zp(imm_operand(x))
#define zpx_operand(x)     get_8bit_zp(imm_operand(x)+reg_x)
#define zpy_operand(x)     get_8bit_zp(imm_operand(x)+reg_y)
#define zpind_operand(x)   get_8bit_addr(get_16bit_zp(imm_operand(x)))
#define zpindx_operand(x)  get_8bit_addr(get_16bit_zp(imm_operand(x)+reg_x))
#define zpindy_operand(x)  get_8bit_addr(get_16bit_zp(imm_operand(x))+reg_y)

// Flag check (flags 'N' and 'Z'):
#define chk_flnz_8bit(x) reg_p = ((reg_p & (~(FL_N|FL_T|FL_Z))) | flnz_list_get(x));

// Memory access
#define get_8bit_zp(zp_addr) (*(zp_base + (zp_addr)))
#define put_8bit_zp(zp_addr, byte) (*(zp_base + (zp_addr)) = (byte))
#define get_16bit_zp(zp_addr) (*(zp_base + zp_addr) | (*(zp_base + (zp_addr + 1)) << 8))
// #define get_16bit_zp(zp_addr) *((uint16*)(zp_base + zp_addr))
#define push_8bit(byte) (*(sp_base + reg_s--) = (byte))
#define pull_8bit() (*(sp_base + ++reg_s))
#define pull_16bit() (pull_8bit() | pull_8bit() << 8)

static inline void
push_16bit(uint16 addr)
{
	*(sp_base + reg_s--) = (uchar) (addr >> 8);
	*(sp_base + reg_s--) = (uchar) (addr & 0xFF);
}

static uint32 cycles = 0;

#include "h6280_opcodes.h"
#include "h6280_opcodes_define.h"

#define Int6502(Type)                                       \
{                                                           \
    uint16 J;                                               \
    if ((Type == INT_NMI) || (!(reg_p & FL_I))) {           \
        cycles += 7;                                        \
        push_16bit(reg_pc);                                 \
        push_8bit(reg_p);                                   \
        reg_p = (reg_p & ~FL_D);                            \
        if (Type == INT_NMI) {                              \
            J = VEC_NMI;                                    \
        } else {                                            \
            reg_p |= FL_I;                                  \
            switch (Type) {                                 \
            case INT_IRQ:                                   \
                J = VEC_IRQ;                                \
                break;                                      \
            case INT_IRQ2:                                  \
                J = VEC_IRQ2;                               \
                break;                                      \
            case INT_TIMER:                                 \
                J = VEC_TIMER;                              \
                break;                                      \
            }                                               \
        }                                                   \
        reg_pc = get_16bit_addr((uint16) J);                \
    }                                                       \
}


// Execute instructions as a machine would, including all
// important (known) interrupts, hardware functions, and
// actual video display on the hardware
//
// Until the following happens:
// (1) An unknown instruction is to be executed
// (2) An unknown hardware access is performed
// (3) <ESC> key is hit

IRAM_ATTR void
exe_go(void)
{
    uchar I;

	// Temp variables used by CPU instructions
    uchar temp, temp1, temp2;
    uint16 addr, from, to, len, alternate;

	cycles = 0;
	cyclecount = 0;
	PCERunning = 1;

    while (true) // PCERunning
	{
		while (cycles <= 455)
		{
			uchar opcode = imm_operand(reg_pc);
			// OP_CALL_THROUGH_LOOKUP;

			switch (opcode)
			{
			case 0x00: // {brek, AM_IMMED, "BRK"}
				OP_CALL_THROUGH_LOOKUP
				break;
			case 0x01: // {ora_zpindx, AM_ZPINDX, "ORA"}
				_OPCODE_ora__(zpindx_operand, 10, 7, 2)
				break;
			case 0x02: // {sxy, AM_IMPL, "SXY"}
				temp = reg_y;
				reg_p &= ~FL_T;
				reg_y = reg_x;
				reg_x = temp;
				reg_pc++;
				cycles += 3;
				break;
			case 0x03: // {st0, AM_IMMED, "ST0"}
				_OPCODE_st__(0)
				break;
			case 0x04: // {tsb_zp, AM_ZP, "TSB"}
				addr = imm_operand(reg_pc + 1);
				temp = get_8bit_zp(addr);
				temp1 = reg_a | temp;

				reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
					| ((temp1 & 0x80) ? FL_N : 0)
					| ((temp1 & 0x40) ? FL_V : 0)
					| ((temp & reg_a) ? 0 : FL_Z);
				put_8bit_zp(addr, temp1);
				reg_pc += 2;
				cycles += 6;
				break;
			case 0x05: // {ora_zp, AM_ZP, "ORA"}
				_OPCODE_ora__(zp_operand, 7, 4, 2)
				break;
			case 0x06: // {asl_zp, AM_ZP, "ASL"}
				_OPCODE_asl_zp(0)
				break;
			case 0x07: // {rmb0, AM_ZP, "RMB0"}
				_OPCODE_rmb_(0x01)
				break;
			case 0x08: // {php, AM_IMPL, "PHP"}
				_OPCODE_ph_(reg_p)
				break;
			case 0x09: // {ora_imm, AM_IMMED, "ORA"}
				_OPCODE_ora__(imm_operand, 5, 2, 2)
				break;
			case 0x0A: // {asl_a, AM_IMPL, "ASL"}
				temp1 = reg_a;
				reg_a <<= 1;
				reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
					| ((temp1 & 0x80) ? FL_C : 0)
					| flnz_list_get(reg_a);
				cycles += 2;
				reg_pc++;
				break;
			case 0x0C: // {tsb_abs, AM_ABS, "TSB"}
				addr = get_16bit_addr(reg_pc + 1);
				temp = get_8bit_addr(addr);
				temp1 = reg_a | temp;

				reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
					| ((temp1 & 0x80) ? FL_N : 0)
					| ((temp1 & 0x40) ? FL_V : 0)
					| ((temp & reg_a) ? 0 : FL_Z);
				cycles += 7;
				put_8bit_addr(addr, temp1);
				reg_pc += 3;
				break;
			case 0x0D: // {ora_abs, AM_ABS, "ORA"}
				_OPCODE_ora__(abs_operand, 8, 5, 3)
				break;
			case 0x0E: // {asl_abs, AM_ABS, "ASL"}
				_OPCODE_asl_abs(0)
				break;
			case 0x0F: // {bbr0, AM_PSREL, "BBR0"}
				_OPCODE_bbr_(0x01)
				break;

			case 0x10: // {bpl, AM_REL, "BPL"}
				_OPCODE_branch(!(reg_p & FL_N))
				break;
			case 0x11: // {ora_zpindy, AM_ZPINDY, "ORA"}
				_OPCODE_ora__(zpindy_operand, 10, 7, 2)
				break;
			case 0x12: // {ora_zpind, AM_ZPIND, "ORA"}
				_OPCODE_ora__(zpind_operand, 10, 7, 2)
				break;
			case 0x13: // {st1, AM_IMMED, "ST1"}
				_OPCODE_st__(2)
				break;
			case 0x14: // {trb_zp, AM_ZP, "TRB"}
				addr = imm_operand(reg_pc + 1);
				temp = get_8bit_zp(addr);
				temp1 = (~reg_a) & temp;

				reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
					| ((temp1 & 0x80) ? FL_N : 0)
					| ((temp1 & 0x40) ? FL_V : 0)
					| ((temp & reg_a) ? 0 : FL_Z);
				put_8bit_zp(addr, temp1);
				reg_pc += 2;
				cycles += 6;
				break;
			case 0x15: // {ora_zpx, AM_ZPX, "ORA"}
				_OPCODE_ora__(zpx_operand, 7, 4, 2)
				break;
			case 0x16: // {asl_zpx, AM_ZPX, "ASL"}
				OP_CALL_THROUGH_LOOKUP
				// _OPCODE_asl_zp(reg_x)
				break;
			case 0x17: // {rmb1, AM_ZP, "RMB1"}
				_OPCODE_rmb_(0x02)
				break;
			case 0x18: // {clc, AM_IMPL, "CLC"}
				reg_p &= ~(FL_T | FL_C);
				reg_pc++;
				cycles += 2;
				break;
			case 0x19: // {ora_absy, AM_ABSY, "ORA"}
				_OPCODE_ora__(absy_operand, 8, 5, 3)
				break;
			case 0x1A: // {inc_a, AM_IMPL, "INC"}
				++reg_a;
				chk_flnz_8bit(reg_a);
				reg_pc++;
				cycles += 2;
				break;
			case 0x1C: // {trb_abs, AM_ABS, "TRB"}
				addr = get_16bit_addr(reg_pc + 1);
				temp = get_8bit_addr(addr);
				temp1 = (~reg_a) & temp;

				reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
					| ((temp1 & 0x80) ? FL_N : 0)
					| ((temp1 & 0x40) ? FL_V : 0)
					| ((temp & reg_a) ? 0 : FL_Z);
				cycles += 7;
				put_8bit_addr(addr, temp1);
				reg_pc += 3;
				break;
			case 0x1D: // {ora_absx, AM_ABSX, "ORA"}
				_OPCODE_ora__(absx_operand, 8, 5, 3)
				break;
			case 0x1E: // {asl_absx, AM_ABSX, "ASL"}
				_OPCODE_asl_abs(reg_x)
				break;
			case 0x1F: // {bbr1, AM_PSREL, "BBR1"}
				_OPCODE_bbr_(0x02)
				break;

			case 0x20: // {jsr, AM_ABS, "JSR"}
				reg_p &= ~FL_T;
				push_16bit(reg_pc + 2);
				reg_pc = get_16bit_addr(reg_pc + 1);
				cycles += 7;
				break;
			case 0x21: // {and_zpindx, AM_ZPINDX, "AND"}
				OP_CALL_THROUGH_LOOKUP
				break;
			case 0x22: // {sax, AM_IMPL, "SAX"}
				temp = reg_x;
				reg_p &= ~FL_T;
				reg_x = reg_a;
				reg_a = temp;
				reg_pc++;
				cycles += 3;
				break;
			case 0x23: // {st2, AM_IMMED, "ST2"}
				_OPCODE_st__(3)
				break;
			case 0x24: // {bit_zp, AM_ZP, "BIT"}
				_OPCODE_bit__(zp_operand, 4, 2)
				break;
			case 0x25: // {and_zp, AM_ZP, "AND"}
				_OPCODE_and__(zp_operand, 7, 4, 2)
				break;
			case 0x26: // {rol_zp, AM_ZP, "ROL"}
				_OPCODE_rol_zp(0)
				break;
			case 0x27: // {rmb2, AM_ZP, "RMB2"}
				_OPCODE_rmb_(0x04)
				break;
			case 0x28: // {plp, AM_IMPL, "PLP"}
				reg_p = pull_8bit();
				reg_pc++;
				cycles += 4;
				break;
			case 0x29: // {and_imm, AM_IMMED, "AND"}
				_OPCODE_and__(imm_operand, 5, 2, 2)
				break;
			case 0x2A: // {rol_a, AM_IMPL, "ROL"}
				temp = reg_a;
				reg_a = (reg_a << 1) + ((reg_p & FL_C) ? 1 : 0);
				reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
					| ((temp & 0x80) ? FL_C : 0)
					| flnz_list_get(reg_a);
				reg_pc++;
				cycles += 2;
				break;
			case 0x2C: // {bit_abs, AM_ABS, "BIT"}
				_OPCODE_bit__(abs_operand, 5, 3)
				break;
			case 0x2D: // {and_abs, AM_ABS, "AND"}
				_OPCODE_and__(abs_operand, 8, 5, 3)
				break;
			case 0x2E: // {rol_abs, AM_ABS, "ROL"}
				_OPCODE_rol_abs(0)
				break;
			case 0x2F: // {bbr2, AM_PSREL, "BBR2"}
				_OPCODE_bbr_(0x04)
				break;

			case 0x30: // {bmi, AM_REL, "BMI"}
				_OPCODE_branch(reg_p & FL_N)
				break;
			case 0x31: // {and_zpindy, AM_ZPINDY, "AND"}
				_OPCODE_and__(zpindy_operand, 10, 7, 2)
				break;
			case 0x32: // {and_zpind, AM_ZPIND, "AND"}
				OP_CALL_THROUGH_LOOKUP
				break;
			case 0x34: // {bit_zpx, AM_ZPX, "BIT"}
				OP_CALL_THROUGH_LOOKUP
				break;
			case 0x35: // {and_zpx, AM_ZPX, "AND"}
				_OPCODE_and__(zpx_operand, 7, 4, 2)
				break;
			case 0x36: // {rol_zpx, AM_ZPX, "ROL"}
				_OPCODE_rol_zp(reg_x)
				break;
			case 0x37: // {rmb3, AM_ZP, "RMB3"}
				_OPCODE_rmb_(0x08)
				break;
			case 0x38: // {sec, AM_IMPL, "SEC"}
				reg_p = (reg_p | FL_C) & ~FL_T;
				reg_pc++;
				cycles += 2;
				break;
			case 0x39: // {and_absy, AM_ABSY, "AND"}
				_OPCODE_and__(absy_operand, 8, 5, 3)
				break;
			case 0x3A: // {dec_a, AM_IMPL, "DEC"}
				--reg_a;
				chk_flnz_8bit(reg_a);
				reg_pc++;
				cycles += 2;
				break;
			case 0x3C: // {bit_absx, AM_ABSX, "BIT"}
				_OPCODE_bit__(absx_operand, 5, 3)
				break;
			case 0x3D: // {and_absx, AM_ABSX, "AND"}
				_OPCODE_and__(absx_operand, 8, 5, 3)
				break;
			case 0x3E: // {rol_absx, AM_ABSX, "ROL"}
				_OPCODE_rol_abs(reg_x)
				break;
			case 0x3F: // {bbr3, AM_PSREL, "BBR3"}
				_OPCODE_bbr_(0x08)
				break;

			case 0x40: // {rti, AM_IMPL, "RTI"}
				reg_p = pull_8bit() & ~FL_B;
				reg_pc = pull_16bit();
				cycles += 7;
				break;
			case 0x41: // {eor_zpindx, AM_ZPINDX, "EOR"}
				OP_CALL_THROUGH_LOOKUP
				break;
			case 0x42: // {say, AM_IMPL, "SAY"}
				temp = reg_y;
				reg_p &= ~FL_T;
				reg_y = reg_a;
				reg_a = temp;
				reg_pc++;
				cycles += 3;
				break;
			case 0x43: // {tma, AM_IMMED, "TMA"}
				temp = imm_operand(reg_pc + 1);

			#if defined(KERNEL_DEBUG)
				if (temp == 0) {
					fprintf(stderr, "TMA with argument 0\n");
					Log("TMA with argument 0\n");
				} else if (!one_bit_set(temp)) {
					fprintf(stderr, "TMA with unusual argument 0x%02x\n", temp);
					Log("TMA with unusual argument 0x%02x\n", temp);
				}
			#endif

				for (uchar i = 0; i < 8; i++) {
					if (temp & (1 << i)) {
						reg_a = mmr[i];
					}
				}
				reg_p &= ~FL_T;
				reg_pc += 2;
				cycles += 4;
				break;
			case 0x44: // {bsr, AM_REL, "BSR"}
				reg_p &= ~FL_T;
				push_16bit(reg_pc + 1);
				reg_pc += (SBYTE) imm_operand(reg_pc + 1) + 2;
				cycles += 8;
				break;
			case 0x45: // {eor_zp, AM_ZP, "EOR"}
				_OPCODE_eor__(zp_operand, 7, 4, 2)
				break;
			case 0x46: // {lsr_zp, AM_ZP, "LSR"}
				_OPCODE_lsr_zp_(0)
				break;
			case 0x47: // {rmb4, AM_ZP, "RMB4"}
				_OPCODE_rmb_(0x10)
				break;
			case 0x48: // {pha, AM_IMPL, "PHA"}
				_OPCODE_ph_(reg_a)
				break;
			case 0x49: // {eor_imm, AM_IMMED, "EOR"}
				_OPCODE_eor__(imm_operand, 5, 2, 2)
				break;
			case 0x4A: // {lsr_a, AM_IMPL, "LSR"}
				temp = reg_a;
				reg_a /= 2;
				reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
					| ((temp & 1) ? FL_C : 0)
					| flnz_list_get(reg_a);
				reg_pc++;
				cycles += 2;
				break;
			case 0x4C: // {jmp, AM_ABS, "JMP"}
				reg_p &= ~FL_T;
				reg_pc = get_16bit_addr(reg_pc + 1);
				cycles += 4;
				break;
			case 0x4D: // {eor_abs, AM_ABS, "EOR"}
				_OPCODE_eor__(abs_operand, 8, 5, 3)
				break;
			case 0x4E: // {lsr_abs, AM_ABS, "LSR"}
				_OPCODE_lsr_abs_(0)
				break;
			case 0x4F: // {bbr4, AM_PSREL, "BBR4"}
				_OPCODE_bbr_(0x10)
				break;

			case 0x50: // {bvc, AM_REL, "BVC"}
				_OPCODE_branch(!(reg_p & FL_V))
				break;
			case 0x51: // {eor_zpindy, AM_ZPINDY, "EOR"}
				_OPCODE_eor__(zpindy_operand, 10, 7, 2)
				break;
			case 0x52: // {eor_zpind, AM_ZPIND, "EOR"}
				OP_CALL_THROUGH_LOOKUP
				break;
			case 0x53: // {tam, AM_IMMED, "TAM"}
				temp = imm_operand(reg_pc + 1);

			#if defined(KERNEL_DEBUG)
				if (temp == 0) {
					fprintf(stderr, "TAM with argument 0\n");
					Log("TAM with argument 0\n");
				} else if (!one_bit_set(temp)) {
					fprintf(stderr, "TAM with unusual argument 0x%02x\n", temp);
					Log("TAM with unusual argument 0x%02x\n", temp);
				}
			#endif

				for (uchar i = 0; i < 8; i++) {
					if (temp & (1 << i)) {
						bank_set(i, reg_a);
					}
				}

				reg_p &= ~FL_T;
				reg_pc += 2;
				cycles += 5;
				break;
			case 0x54: // {nop, AM_IMPL, "CSL"}
				_OPCODE_nop_
				break;
			case 0x55: // {eor_zpx, AM_ZPX, "EOR"}
				_OPCODE_eor__(zpx_operand, 7, 4, 2)
				break;
			case 0x56: // {lsr_zpx, AM_ZPX, "LSR"}
				OP_CALL_THROUGH_LOOKUP
				break;
			case 0x57: // {rmb5, AM_ZP, "RMB5"}
				_OPCODE_rmb_(0x20)
				break;
			case 0x58: // {cli, AM_IMPL, "CLI"}
				reg_p &= ~(FL_T | FL_I);
				reg_pc++;
				cycles += 2;
				break;
			case 0x59: // {eor_absy, AM_ABSY, "EOR"}
				_OPCODE_eor__(absy_operand, 8, 5, 3)
				break;
			case 0x5A: // {phy, AM_IMPL, "PHY"}
				_OPCODE_ph_(reg_y)
				break;
			case 0x5D: // {eor_absx, AM_ABSX, "EOR"}
				_OPCODE_eor__(absx_operand, 8, 5, 3)
				break;
			case 0x5E: // {lsr_absx, AM_ABSX, "LSR"}
				_OPCODE_lsr_abs_(reg_x)
				break;
			case 0x5F: // {bbr5, AM_PSREL, "BBR5"}
				_OPCODE_bbr_(0x20)
				break;

			case 0x60: // {rts, AM_IMPL, "RTS"}
				reg_p &= ~FL_T;
				reg_pc = pull_16bit() + 1;
				cycles += 7;
				break;
			case 0x61: // {adc_zpindx, AM_ZPINDX, "ADC"}
				OP_CALL_THROUGH_LOOKUP
				break;
			case 0x62: // {cla, AM_IMPL, "CLA"}
				reg_p &= ~FL_T;
				reg_a = 0;
				reg_pc++;
				cycles += 2;
				break;
			case 0x64: // {stz_zp, AM_ZP, "STZ"}
				_OPCODE_st__zp(0, 0)
				break;
			case 0x65: // {adc_zp, AM_ZP, "ADC"}
				_OPCODE_adc_zp
				break;
			case 0x66: // {ror_zp, AM_ZP, "ROR"}
				_OPCODE_ror_zp_(0)
				break;
			case 0x67: // {rmb6, AM_ZP, "RMB6"}
				_OPCODE_rmb_(0x40)
				break;
			case 0x68: // {pla, AM_IMPL, "PLA"}
				_OPCODE_pl_(reg_a)
				break;
			case 0x69: // {adc_imm, AM_IMMED, "ADC"}
				if (reg_p & FL_T) {
					put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), imm_operand(reg_pc + 1)));
					cycles += 5;
				} else {
					reg_a = adc(reg_a, imm_operand(reg_pc + 1));
					cycles += 2;
				}
				reg_pc += 2;
				break;
			case 0x6A: // {ror_a, AM_IMPL, "ROR"}
				temp = reg_a;
				reg_a = (reg_a / 2) + ((reg_p & FL_C) ? 0x80 : 0);
				reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
					| ((temp & 0x01) ? FL_C : 0)
					| flnz_list_get(reg_a);
				reg_pc++;
				cycles += 2;
				break;
			case 0x6C: // {jmp_absind, AM_ABSIND, "JMP"}
				reg_p &= ~FL_T;
				reg_pc = get_16bit_addr(get_16bit_addr(reg_pc + 1));
				cycles += 7;
				break;
			case 0x6D: // {adc_abs, AM_ABS, "ADC"}
				if (reg_p & FL_T) {
					put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), abs_operand(reg_pc + 1)));
					cycles += 8;
				} else {
					reg_a = adc(reg_a, abs_operand(reg_pc + 1));
					cycles += 5;
				}
				reg_pc += 3;
				break;
			case 0x6E: // {ror_abs, AM_ABS, "ROR"}
				_OPCODE_ror_abs_(0)
				break;
			case 0x6F: // {bbr6, AM_PSREL, "BBR6"}
				_OPCODE_bbr_(0x40)
				break;

			case 0x70: // {bvs, AM_REL, "BVS"}
				_OPCODE_branch(reg_p & FL_V)
				break;
			case 0x71: // {adc_zpindy, AM_ZPINDY, "ADC"}
				if (reg_p & FL_T) {
					put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), zpindy_operand(reg_pc + 1)));
					cycles += 10;
				} else {
					reg_a = adc(reg_a, zpindy_operand(reg_pc + 1));
					cycles += 7;
				}
				reg_pc += 2;
				break;
			case 0x72: // {adc_zpind, AM_ZPIND, "ADC"}
				if (reg_p & FL_T) {
					put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), zpind_operand(reg_pc + 1)));
					cycles += 10;
				} else {
					reg_a = adc(reg_a, zpind_operand(reg_pc + 1));
					cycles += 7;
				}
				reg_pc += 2;
				break;
			case 0x73: // {tii, AM_XFER, "TII"}
				reg_p &= ~FL_T;
				from = get_16bit_addr(reg_pc + 1);
				to = get_16bit_addr(reg_pc + 3);
				len = get_16bit_addr(reg_pc + 5);

				cycles += (6 * len) + 17;
				while (len-- != 0) {
					put_8bit_addr(to++, get_8bit_addr(from++));
				}
				reg_pc += 7;
				break;
			case 0x74: // {stz_zpx, AM_ZPX, "STZ"}
				_OPCODE_st__zp(0, reg_x)
				break;
			case 0x75: // {adc_zpx, AM_ZPX, "ADC"}
				_OPCODE_adc_zpx
				break;
			case 0x76: // {ror_zpx, AM_ZPX, "ROR"}
				OP_CALL_THROUGH_LOOKUP
				break;
			case 0x77: // {rmb7, AM_ZP, "RMB7"}
				_OPCODE_rmb_(0x80)
				break;
			case 0x78: // {sei, AM_IMPL, "SEI"}
				#ifdef DUMP_ON_SEI
					int i;
					Log("MMR[7]\n");
					for (i = 0xE000; i < 0xE100; i++) {
						Log("%02X ", get_8bit_addr(i));
					}
				#endif
				reg_p = (reg_p | FL_I) & ~FL_T;
				reg_pc++;
				cycles += 2;
				break;
			case 0x79: // {adc_absy, AM_ABSY, "ADC"}
				_OPCODE_add_abc_(absy_operand)
				break;
			case 0x7A: // {ply, AM_IMPL, "PLY"}
				_OPCODE_pl_(reg_y)
				break;
			case 0x7C: // {jmp_absindx, AM_ABSINDX, "JMP"}
				reg_p &= ~FL_T;
				reg_pc = get_16bit_addr(get_16bit_addr(reg_pc + 1) + reg_x);
				cycles += 7;
				break;
			case 0x7D: // {adc_absx, AM_ABSX, "ADC"}
				if (reg_p & FL_T) {
					put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), absx_operand(reg_pc + 1)));
					cycles += 8;
				} else {
					reg_a = adc(reg_a, absx_operand(reg_pc + 1));
					cycles += 5;
				}
				reg_pc += 3;
				break;
			case 0x7E: // {ror_absx, AM_ABSX, "ROR"}
				_OPCODE_ror_abs_(reg_x)
				break;
			case 0x7F: // {bbr7, AM_PSREL, "BBR7"}
				_OPCODE_bbr_(0x80)
				break;

			case 0x80: // {bra, AM_REL, "BRA"}
				_OPCODE_branch(true)
				break;
			case 0x81: // {sta_zpindx, AM_ZPINDX, "STA"}
				_OPCODE_sta__zpind(reg_x)
				break;
			case 0x82: // {clx, AM_IMPL, "CLX"}
				reg_p &= ~FL_T;
				reg_x = 0;
				reg_pc++;
				cycles += 2;
				break;
			case 0x83: // {tstins_zp, AM_TST_ZP, "TST"}
				_OPCODE_tstins_(zp_operand, 7, 3)
				break;
			case 0x84: // {sty_zp, AM_ZP, "STY"}
				_OPCODE_st__zp(reg_y, 0)
				break;
			case 0x85: // {sta_zp, AM_ZP, "STA"}
				_OPCODE_st__zp(reg_a,0)
				break;
			case 0x86: // {stx_zp, AM_ZP, "STX"}
				_OPCODE_st__zp(reg_x, 0)
				break;
			case 0x87: // {smb0, AM_ZP, "SMB0"}
				_OPCODE_smb_(0x01)
				break;
			case 0x88: // {dey, AM_IMPL, "DEY"}
				--reg_y;
				chk_flnz_8bit(reg_y);
				reg_pc++;
				cycles += 2;
				break;
			case 0x89: // {bit_imm, AM_IMMED, "BIT"}
				_OPCODE_bit__(imm_operand, 2, 2)
				break;
			case 0x8A: // {txa, AM_IMPL, "TXA"}
				reg_a = reg_x;
				chk_flnz_8bit(reg_x);
				reg_pc++;
				cycles += 2;
				break;
			case 0x8C: // {sty_abs, AM_ABS, "STY"}
				_OPCODE_st__abs(reg_y, 0)
				break;
			case 0x8D: // {sta_abs, AM_ABS, "STA"}
				_OPCODE_st__abs(reg_a, 0)
				break;
			case 0x8E: // {stx_abs, AM_ABS, "STX"}
				_OPCODE_st__abs(reg_x, 0)
				break;
			case 0x8F: // {bbs0, AM_PSREL, "BBS0"}
				_OPCODE_bbs_(0x01)
				break;

			case 0x90: // {bcc, AM_REL, "BCC"}
				_OPCODE_branch(!(reg_p & FL_C))
				break;
			case 0x91: // {sta_zpindy, AM_ZPINDY, "STA"}
				_OPCODE_sta__zpind(reg_y)
				break;
			case 0x92: // {sta_zpind, AM_ZPIND, "STA"}
				_OPCODE_sta__zpind(0)
				break;
			case 0x93: // {tstins_abs, AM_TST_ABS, "TST"}
				_OPCODE_tstins_(abs_operand, 8, 4)
				break;
			case 0x94: // {sty_zpx, AM_ZPX, "STY"}
				_OPCODE_st__zp(reg_y, reg_x)
				break;
			case 0x95: // {sta_zpx, AM_ZPX, "STA"}
				_OPCODE_st__zp(reg_a, reg_x)
				break;
			case 0x96: // {stx_zpy, AM_ZPY, "STX"}
				_OPCODE_st__zp(reg_x, reg_y)
				break;
			case 0x97: // {smb1, AM_ZP, "SMB1"}
				_OPCODE_smb_(0x02)
				break;
			case 0x98: // {tya, AM_IMPL, "TYA"}
				reg_a = reg_y;
				chk_flnz_8bit(reg_y);
				reg_pc++;
				cycles += 2;
				break;
			case 0x99: // {sta_absy, AM_ABSY, "STA"}
				_OPCODE_st__abs(reg_a, reg_y)
				break;
			case 0x9A: // {txs, AM_IMPL, "TXS"}
				reg_p &= ~FL_T;
				reg_s = reg_x;
				reg_pc++;
				cycles += 2;
				break;
			case 0x9C: // {stz_abs, AM_ABS, "STZ"}
				_OPCODE_st__abs(0,0)
				break;
			case 0x9D: // {sta_absx, AM_ABSX, "STA"}
				_OPCODE_st__abs(reg_a, reg_x)
				break;
			case 0x9E: // {stz_absx, AM_ABSX, "STZ"}
				_OPCODE_st__abs(0,reg_x)
				break;
			case 0x9F: // {bbs1, AM_PSREL, "BBS1"}
				_OPCODE_bbs_(0x02)
				break;

			case 0xA0: // {ldy_imm, AM_IMMED, "LDY"}
				_OPCODE_ld__imm(reg_y)
				break;
			case 0xA1: // {lda_zpindx, AM_ZPINDX, "LDA"}
				_OPCODE_ld__ind(reg_a, zpindx_operand)
				break;
			case 0xA2: // {ldx_imm, AM_IMMED, "LDX"}
				_OPCODE_ld__imm(reg_x)
				break;
			case 0xA3: // {tstins_zpx, AM_TST_ZPX, "TST"}
				_OPCODE_tstins_(zpx_operand, 7, 3)
				break;
			case 0xA4: // {ldy_zp, AM_ZP, "LDY"}
				_OPCODE_ld__(reg_y, zp_operand)
				break;
			case 0xA5: // {lda_zp, AM_ZP, "LDA"}
				_OPCODE_ld__(reg_a, zp_operand)
				// OP_CALL_THROUGH_LOOKUP
				break;
			case 0xA6: // {ldx_zp, AM_ZP, "LDX"}
				_OPCODE_ld__(reg_x, zp_operand)
				break;
			case 0xA7: // {smb2, AM_ZP, "SMB2"}
				_OPCODE_smb_(0x04)
				break;
			case 0xA8: // {tay, AM_IMPL, "TAY"}
				reg_y = reg_a;
				chk_flnz_8bit(reg_a);
				reg_pc++;
				cycles += 2;
				break;
			case 0xA9: // {lda_imm, AM_IMMED, "LDA"}
				_OPCODE_ld__imm(reg_a)
				break;
			case 0xAA: // {tax, AM_IMPL, "TAX"}
				reg_x = reg_a;
				chk_flnz_8bit(reg_a);
				reg_pc++;
				cycles += 2;
				break;
			case 0xAC: // {ldy_abs, AM_ABS, "LDY"}
				_OPCODE_ld__abs(reg_y, abs_operand)
				break;
			case 0xAD: // {lda_abs, AM_ABS, "LDA"}
				_OPCODE_ld__abs(reg_a, abs_operand)
				break;
			case 0xAE: // {ldx_abs, AM_ABS, "LDX"}
				_OPCODE_ld__abs(reg_x, abs_operand)
				break;
			case 0xAF: // {bbs2, AM_PSREL, "BBS2"}
				_OPCODE_bbs_(0x04)
				break;

			case 0xB0: // {bcs, AM_REL, "BCS"}
				_OPCODE_branch(reg_p & FL_C)
				break;
			case 0xB1: // {lda_zpindy, AM_ZPINDY, "LDA"}
				_OPCODE_ld__ind(reg_a, zpindy_operand)
				break;
			case 0xB2: // {lda_zpind, AM_ZPIND, "LDA"}
				_OPCODE_ld__ind(reg_a, zpind_operand)
				break;
			case 0xB3: // {tstins_absx, AM_TST_ABSX, "TST"}
				_OPCODE_tstins_(absx_operand, 8, 4)
				break;
			case 0xB4: // {ldy_zpx, AM_ZPX, "LDY"}
				_OPCODE_ld__(reg_y, zpx_operand)
				break;
			case 0xB5: // {lda_zpx, AM_ZPX, "LDA"}
				_OPCODE_ld__(reg_a, zpx_operand)
				break;
			case 0xB6: // {ldx_zpy, AM_ZPY, "LDX"}
				_OPCODE_ld__(reg_x, zpy_operand)
				break;
			case 0xB7: // {smb3, AM_ZP, "SMB3"}
				_OPCODE_smb_(0x08)
				break;
			case 0xB8: // {clv, AM_IMPL, "CLV"}
				reg_p &= ~(FL_V | FL_T);
				reg_pc++;
				cycles += 2;
				break;
			case 0xB9: // {lda_absy, AM_ABSY, "LDA"}
				_OPCODE_ld__abs(reg_a, absy_operand)
				break;
			case 0xBA: // {tsx, AM_IMPL, "TSX"}
				reg_x = reg_s;
				chk_flnz_8bit(reg_s);
				reg_pc++;
				cycles += 2;
				break;
			case 0xBC: // {ldy_absx, AM_ABSX, "LDY"}
				_OPCODE_ld__abs(reg_y, absx_operand)
				break;
			case 0xBD: // {lda_absx, AM_ABSX, "LDA"}
				_OPCODE_ld__abs(reg_a, absx_operand)
				break;
			case 0xBE: // {ldx_absy, AM_ABSY, "LDX"}
				_OPCODE_ld__abs(reg_x, absy_operand)
				break;
			case 0xBF: // {bbs3, AM_PSREL, "BBS3"}
				_OPCODE_bbs_(0x08)
				break;

			case 0xC0: // {cpy_imm, AM_IMMED, "CPY"}
				_OPCODE_cp__(imm_operand, reg_y, 2, 2)
				break;
			case 0xC1: // {cmp_zpindx, AM_ZPINDX, "CMP"}
				OP_CALL_THROUGH_LOOKUP
				break;
			case 0xC2: // {cly, AM_IMPL, "CLY"}
				reg_p &= ~FL_T;
				reg_y = 0;
				reg_pc++;
				cycles += 2;
				break;
			case 0xC3: // {tdd, AM_XFER, "TDD"}
				OP_CALL_THROUGH_LOOKUP
				break;
			case 0xC4: // {cpy_zp, AM_ZP, "CPY"}
				_OPCODE_cp__(zp_operand, reg_y, 4, 2)
				break;
			case 0xC5: // {cmp_zp, AM_ZP, "CMP"}
				_OPCODE_cmp__(zp_operand, 4, 2);
				break;
			case 0xC6: // {dec_zp, AM_ZP, "DEC"}
				_OPCODE_dec_zp_(0)
				break;
			case 0xC7: // {smb4, AM_ZP, "SMB4"}
				_OPCODE_smb_(0x10)
				break;
			case 0xC8: // {iny, AM_IMPL, "INY"}
				++reg_y;
				chk_flnz_8bit(reg_y);
				reg_pc++;
				cycles += 2;
				break;
			case 0xC9: // {cmp_imm, AM_IMMED, "CMP"}
				_OPCODE_cmp__(imm_operand, 2, 2)
				break;
			case 0xCA: // {dex, AM_IMPL, "DEX"}
				--reg_x;
				chk_flnz_8bit(reg_x);
				reg_pc++;
				cycles += 2;
				break;
			case 0xCC: // {cpy_abs, AM_ABS, "CPY"}
				_OPCODE_cp__(abs_operand, reg_y, 5, 3)
				break;
			case 0xCD: // {cmp_abs, AM_ABS, "CMP"}
				_OPCODE_cmp__(abs_operand, 5, 3)
				break;
			case 0xCE: // {dec_abs, AM_ABS, "DEC"}
				_OPCODE_dec_abs_(0)
				break;
			case 0xCF: // {bbs4, AM_PSREL, "BBS4"}
				_OPCODE_bbs_(0x10)
				break;

			case 0xD0: // {bne, AM_REL, "BNE"}
				_OPCODE_branch(!(reg_p & FL_Z))
				break;
			case 0xD1: // {cmp_zpindy, AM_ZPINDY, "CMP"}
				_OPCODE_cmp__(zpindy_operand, 7, 2)
				break;
			case 0xD2: // {cmp_zpind, AM_ZPIND, "CMP"}
				_OPCODE_cmp__(zpind_operand, 7, 2)
				break;
			case 0xD3: // {tin, AM_XFER, "TIN"}
				reg_p &= ~FL_T;
				from = get_16bit_addr(reg_pc + 1);
				to = get_16bit_addr(reg_pc + 3);
				len = get_16bit_addr(reg_pc + 5);

				cycles += (6 * len) + 17;
				while (len-- != 0) {
					put_8bit_addr(to, get_8bit_addr(from++));
				}
				reg_pc += 7;
				break;
			case 0xD4: // {nop, AM_IMPL, "CSH"}
				_OPCODE_nop_
				break;
			case 0xD5: // {cmp_zpx, AM_ZPX, "CMP"}
				_OPCODE_cmp__(zpx_operand, 4, 2)
				break;
			case 0xD6: // {dec_zpx, AM_ZPX, "DEC"}
				_OPCODE_dec_zp_(reg_x)
				break;
			case 0xD7: // {smb5, AM_ZP, "SMB5"}
				_OPCODE_smb_(0x20)
				break;
			case 0xD8: // {cld, AM_IMPL, "CLD"}
				reg_p &= ~(FL_T | FL_D);
				reg_pc++;
				cycles += 2;
				break;
			case 0xD9: // {cmp_absy, AM_ABSY, "CMP"}
				_OPCODE_cmp__(absy_operand, 5, 3)
				break;
			case 0xDA: // {phx, AM_IMPL, "PHX"}
				_OPCODE_ph_(reg_x)
				break;
			case 0xDD: // {cmp_absx, AM_ABSX, "CMP"}
				_OPCODE_cmp__(absx_operand, 5, 3)
				break;
			case 0xDE: // {dec_absx, AM_ABSX, "DEC"}
				_OPCODE_dec_abs_(reg_x)
				break;
			case 0xDF: // {bbs5, AM_PSREL, "BBS5"}
				_OPCODE_bbs_(0x20)
				break;

			case 0xE0: // {cpx_imm, AM_IMMED, "CPX"}
				_OPCODE_cp__(imm_operand, reg_x, 2, 2)
				break;
			case 0xE1: // {sbc_zpindx, AM_ZPINDX, "SBC"}
				_OPCODE_sbc__zpind(zpindx_operand)
				break;
			case 0xE3:  // {tia, AM_XFER, "TIA"} $E3
				reg_p &= ~FL_T;
				from = get_16bit_addr(reg_pc + 1);
				to = get_16bit_addr(reg_pc + 3);
				len = get_16bit_addr(reg_pc + 5);
				alternate = 0;

				cycles += (6 * len) + 17;
				while (len-- != 0) {
					put_8bit_addr(to + alternate, get_8bit_addr(from++));
					alternate ^= 1;
				}
				reg_pc += 7;
				break;
			case 0xE4: // {cpx_zp, AM_ZP, "CPX"}
				_OPCODE_cpx_zp
				break;
			case 0xE5: // {sbc_zp, AM_ZP, "SBC"}
				_OPCODE_sbc__zp(zp_operand)
				break;
			case 0xE6: // {inc_zp, AM_ZP, "INC"}
				_OPCODE_inc_zp_(0)
				break;
			case 0xE7: // {smb6, AM_ZP, "SMB6"}
				_OPCODE_smb_(0x40)
				break;
			case 0xE8: // {inx, AM_IMPL, "INX"}
				++reg_x;
				chk_flnz_8bit(reg_x);
				reg_pc++;
				cycles += 2;
				break;
			case 0xE9: // {sbc_imm, AM_IMMED, "SBC"}
				_OPCODE_sbc__imm
				break;
			case 0xEA: // {nop, AM_IMPL, "NOP"}
				_OPCODE_nop_
				break;
			case 0xEC: // {cpx_abs, AM_ABS, "CPX"}
				_OPCODE_cp__(abs_operand, reg_x, 5, 3)
				break;
			case 0xED: // {sbc_abs, AM_ABS, "SBC"}
				sbc(abs_operand(reg_pc + 1));
				reg_pc += 3;
				cycles += 5;
				break;
			case 0xEE: // {inc_abs, AM_ABS, "INC"}
				_OPCODE_inc_abs_(0)
				break;
			case 0xEF: // {bbs6, AM_PSREL, "BBS6"}
				_OPCODE_bbs_(0x40)
				break;

			case 0xF0: // {beq, AM_REL, "BEQ"}
				_OPCODE_branch(reg_p & FL_Z)
				break;
			case 0xF1: // {sbc_zpindy, AM_ZPINDY, "SBC"}
				_OPCODE_sbc__zpind(zpindy_operand)
				break;
			case 0xF2: // {sbc_zpind, AM_ZPIND, "SBC"}
				_OPCODE_sbc__zpind(zpind_operand)
				break;
			case 0xF3: // {tai, AM_XFER, "TAI"}
				reg_p &= ~FL_T;
				from = get_16bit_addr(reg_pc + 1);
				to = get_16bit_addr(reg_pc + 3);
				len = get_16bit_addr(reg_pc + 5);
				alternate = 0;

				cycles += (6 * len) + 17;
				while (len-- != 0) {
					put_8bit_addr(to++, get_8bit_addr(from + alternate));
					alternate ^= 1;
				}
				reg_pc += 7;
				break;
			case 0xF4: // {set, AM_IMPL, "SET"}
				reg_p |= FL_T;
				reg_pc++;
				cycles += 2;
				break;
			case 0xF5: // {sbc_zpx, AM_ZPX, "SBC"}
				_OPCODE_sbc__zp(zpx_operand)
				break;
			case 0xF6: // {inc_zpx, AM_ZPX, "INC"}
				_OPCODE_inc_zp_(reg_x)
				break;
			case 0xF7: // {smb7, AM_ZP, "SMB7"}
				_OPCODE_smb_(0x80)
				break;
			case 0xF8: // {sed, AM_IMPL, "SED"}
				reg_p = (reg_p | FL_D) & ~FL_T;
				reg_pc++;
				cycles += 2;
				break;
			case 0xF9: // {sbc_absy, AM_ABSY, "SBC"}
				sbc(absy_operand(reg_pc + 1));
				reg_pc += 3;
				cycles += 5;
				break;
			case 0xFA: // {plx, AM_IMPL, "PLX"}
				_OPCODE_pl_(reg_x)
				break;
			case 0xFD: // {sbc_absx, AM_ABSX, "SBC"}
				sbc(absx_operand(reg_pc + 1));
				reg_pc += 3;
				cycles += 5;
				break;
			case 0xFE: // {inc_absx, AM_ABSX, "INC"}
				_OPCODE_inc_abs_(reg_x)
				break;
			case 0xFF: // {bbs7, AM_PSREL, "BBS7"}
				_OPCODE_bbs_(0x80)
				break;

			default:
				MESSAGE_ERROR("OpCode %02X not supported!", opcode);
				// abort() ?
				break;
			}
		}

        // HSYNC stuff - count cycles:
        /*if (cycles > 455) */ {
            cyclecount += cycles;
            // cycles -= 455;
            // scanline++;

            // Log("Calling periodic handler\n");

            I = Loop6502();     /* Call the periodic handler */

            // _ICount += _IPeriod;
            /* Reset the cycle counter */
            cycles = 0;

            // Log("Requested interrupt is %d\n",I);

            /* if (I == INT_QUIT) return; */

            if (I)
                Int6502(I);     /* Interrupt if needed  */

            if ((uint) (cyclecount - cyclecountold) > (uint) TimerPeriod * 2)
				cyclecountold = cyclecount;

        } /*else*/ {
            if (cyclecount - cyclecountold >= TimerPeriod) {
                cyclecountold += TimerPeriod;
                if ((I = TimerInt()))
                    Int6502(I);
            }
        }
    }
    MESSAGE_ERROR("Abnormal exit from the cpu loop\n");
}


void
dump_cpu_registers()
{
	int i;

	fprintf(stderr, "Dumping PCE core\n");

	Log("PC = 0x%04x\n", reg_pc);
	Log("A = 0x%02x\n", reg_a);
	Log("X = 0x%02x\n", reg_x);
	Log("Y = 0x%02x\n", reg_y);
	Log("P = 0x%02x\n", reg_p);
	Log("S = 0x%02x\n", reg_s);

	for (i = 0; i < 8; i++) {
		Log("MMR[%d] = 0x%02x\n", i, mmr[i]);
	}

	// TODO: Add zero page dump

	for (i = 0x2000; i < 0xFFFF; i++) {

		if ((i & 0xF) == 0) {
			Log("%04X: ", i);
		}

		Log("%02x ", get_8bit_addr((uint16) i));
		if ((i & 0xF) == 0xF) {
			Log("\n");
		}
		if ((i & 0x1FFF) == 0x1FFF) {
			Log("\n-------------------------------------------------------------\n");
		}
	}

}
