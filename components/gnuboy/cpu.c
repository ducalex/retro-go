#include "gnuboy.h"
#include "hw.h"
#include "lcd.h"
#include "cpu.h"
#include "sound.h"

// For cycle accurate emulation this needs to be 1
// Anything above 10 have diminishing returns
#define COUNTERS_TICK_PERIOD 8

static const byte cycles_table[256] =
{
	1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1,
	1, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,
	3, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,
	3, 3, 2, 2, 3, 3, 3, 1, 3, 2, 2, 2, 1, 1, 2, 1,

	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,

	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,

	5, 3, 4, 4, 6, 4, 2, 4, 5, 4, 4, 1, 6, 6, 2, 4,
	5, 3, 4, 0, 6, 4, 2, 4, 5, 4, 4, 0, 6, 0, 2, 4,
	3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4,
	3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4,
};

static const byte cb_cycles_table[256] =
{
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,

	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,

	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,

	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
};

typedef uint32_t un32;
typedef uint16_t un16;
typedef uint8_t un8;
typedef int8_t n8;

#define ZFLAG(n) ( (n) ? 0 : FZ )

#define PUSH(w) ( (SP -= 2), (writew(SP, (w))) )
#define POP(w)  ( ((w) = readw(SP)), (SP += 2) )

#define FETCH (readb(PC++))

#define INC(r) { r++; F = (F & (FL|FC)) | ZFLAG(r) | ((r & 0x0F) == 0x00 ? FH : 0); }
#define DEC(r) { r--; F = (F & (FL|FC)) | FN | ZFLAG(r) | ((r & 0x0F) == 0x0F ? FH : 0); }

#define INCW(r) { r++; }
#define DECW(r) { r--; }

#define ADD(n) { \
W(acc) = (un16)A + (un16)(n); \
F = (ZFLAG(LB(acc))) | (FH & ((A ^ (n) ^ LB(acc)) << 1)) | (HB(acc) << 4); \
A = LB(acc); }

#define ADC(n) { \
W(acc) = (un16)A + (un16)(n) + (un16)((F&FC)>>4); \
F = (ZFLAG(LB(acc))) | (FH & ((A ^ (n) ^ LB(acc)) << 1)) | (HB(acc) << 4); \
A = LB(acc); }

#define ADDW(n) { \
temp = (un32)HL + (un32)(n); \
F &= FZ; \
if (0xFFFF - (n) < HL) F |= FC; \
if ((HL & 0xFFF) + ((n) & 0xFFF) > 0xFFF) F |= FH; \
HL = temp; }

#define CP(n) { \
W(acc) = (un16)A - (un16)(n); \
F = FN \
| (ZFLAG(LB(acc))) \
| (FH & ((A ^ (n) ^ LB(acc)) << 1)) \
| ((un8)(-(n8)HB(acc)) << 4); }

#define SUB(n) { CP((n)); A = LB(acc); }

#define SBC(n) { \
W(acc) = (un16)A - (un16)(n) - (un16)((F&FC)>>4); \
F = FN \
| (ZFLAG((n8)LB(acc))) \
| (FH & ((A ^ (n) ^ LB(acc)) << 1)) \
| ((un8)(-(n8)HB(acc)) << 4); \
A = LB(acc); }

#define AND(n) { A &= (n); F = ZFLAG(A) | FH; }
#define XOR(n) { A ^= (n); F = ZFLAG(A); }
#define OR(n)  { A |= (n); F = ZFLAG(A); }

#define RLCA(r) { r = (r>>7) | (r<<1); F = ((r&0x01)<<4); }
#define RRCA(r) { r = (r<<7) | (r>>1); F = ((r&0x80)>>3); }
#define RLA(r) { LB(acc) = ((r&0x80)>>3); r = (r<<1) | ((F&FC)>>4); F = LB(acc); }
#define RRA(r) { LB(acc) = ((r&0x01)<<4); r = (r>>1) | ((F&FC)<<3); F = LB(acc); }

#define RLC(r) { RLCA(r); F |= ZFLAG(r); }
#define RRC(r) { RRCA(r); F |= ZFLAG(r); }
#define RL(r)  { RLA(r); F |= ZFLAG(r); }
#define RR(r)  { RRA(r); F |= ZFLAG(r); }

#define SLA(r) { LB(acc) = ((r&0x80)>>3); r <<= 1; F = ZFLAG(r) | LB(acc); }
#define SRA(r) { LB(acc) = ((r&0x01)<<4); r = (un8)(((n8)r)>>1); F = ZFLAG(r) | LB(acc); }
#define SRL(r) { LB(acc) = ((r&0x01)<<4); r >>= 1; F = ZFLAG((r)) | LB(acc); }

#define CPL(r) { r = ~r; F |= (FH|FN); }

#define SCF { F = (F & (FZ)) | FC; }
#define CCF { F = (F & (FZ|FC)) ^ FC; }

#define SWAP(r) { r = ((r & 0x0F) << 4) | ((r & 0xF0) >> 4); F = ZFLAG(r); }

#define BIT(n,r) { F = (F & FC) | ZFLAG((r & (1 << (n)))) | FH; }
#define RES(n,r) { r &= ~(1 << (n)); }
#define SET(n,r) { r |= (1 << (n)); }

#define JR ( PC += 1+(n8)readb(PC) )
#define JP ( PC = readw(PC) )

#define NOJR   ( clen--,  PC++ )
#define NOJP   ( clen--,  PC+=2 )
#define NOCALL ( clen-=3, PC+=2 )
#define NORET  ( clen-=3 )

#define RST(n) { PUSH(PC); PC = (n); }

#define CALL ( PUSH(PC+2), JP )
#define RET ( POP(PC) )

#define EI ( IMA = 1 )
#define DI ( cpu.halted = IMA = IME = 0 )

#define COND_EXEC_INT(i, n) if (temp & i) { DI; PUSH(PC); R_IF &= ~i; PC = 0x40+((n)<<3); clen = 5; goto _skip; }

#define CB_REG_CASES(r, n) \
case 0x00|(n): RLC(r); break; \
case 0x08|(n): RRC(r); break; \
case 0x10|(n): RL(r); break; \
case 0x18|(n): RR(r); break; \
case 0x20|(n): SLA(r); break; \
case 0x28|(n): SRA(r); break; \
case 0x30|(n): SWAP(r); break; \
case 0x38|(n): SRL(r); break; \
case 0x40|(n): BIT(0, r); break; \
case 0x48|(n): BIT(1, r); break; \
case 0x50|(n): BIT(2, r); break; \
case 0x58|(n): BIT(3, r); break; \
case 0x60|(n): BIT(4, r); break; \
case 0x68|(n): BIT(5, r); break; \
case 0x70|(n): BIT(6, r); break; \
case 0x78|(n): BIT(7, r); break; \
case 0x80|(n): RES(0, r); break; \
case 0x88|(n): RES(1, r); break; \
case 0x90|(n): RES(2, r); break; \
case 0x98|(n): RES(3, r); break; \
case 0xA0|(n): RES(4, r); break; \
case 0xA8|(n): RES(5, r); break; \
case 0xB0|(n): RES(6, r); break; \
case 0xB8|(n): RES(7, r); break; \
case 0xC0|(n): SET(0, r); break; \
case 0xC8|(n): SET(1, r); break; \
case 0xD0|(n): SET(2, r); break; \
case 0xD8|(n): SET(3, r); break; \
case 0xE0|(n): SET(4, r); break; \
case 0xE8|(n): SET(5, r); break; \
case 0xF0|(n): SET(6, r); break; \
case 0xF8|(n): SET(7, r); break;


#define ALU_CASES(base, imm, op, label) \
case (imm): b = FETCH; goto label; \
case (base): b = B; goto label; \
case (base)+1: b = C; goto label; \
case (base)+2: b = D; goto label; \
case (base)+3: b = E; goto label; \
case (base)+4: b = H; goto label; \
case (base)+5: b = L; goto label; \
case (base)+6: b = readb(HL); goto label; \
case (base)+7: b = A; \
label: op(b); break;


static gb_cpu_t cpu;


gb_cpu_t *cpu_init(void)
{
	return &cpu;
}

/* reset */
void cpu_reset(bool hard)
{
	cpu.double_speed = 0;
	cpu.halted = 0;
	cpu.div = 0;
	cpu.timer = 0;

	IME = 0;
	IMA = 0;

	PC = hw.bios ? 0x0000 : 0x0100;
	SP = 0xFFFE;
	AF = (hw.hwtype == GB_HW_CGB) ? 0x11B0 : 0x01B0;
	BC = 0x0013;
	DE = 0x00D8;
	HL = 0x014D;
}

/* cnt - time to emulate, expressed in real clock cycles */
static inline void timer_advance(int cycles)
{
	cpu.div += (cycles << 2);

	R_DIV += (cpu.div >> 8);

	cpu.div &= 0xff;

	if (R_TAC & 0x04)
	{
		cpu.timer += (cycles << ((((-R_TAC) & 3) << 1) + 1));

		if (cpu.timer >= 512)
		{
			int tima = R_TIMA + (cpu.timer >> 9);
			cpu.timer &= 0x1ff;
			if (tima >= 256)
			{
				hw_interrupt(IF_TIMER, 1);
				hw_interrupt(IF_TIMER, 0);
				tima = R_TMA;
			}
			R_TIMA = tima;
		}
	}
}

/* cnt - time to emulate, expressed in real clock cycles */
static inline void serial_advance(int cycles)
{
	if (hw.serial > 0)
	{
		hw.serial -= cycles << 1;
		if (hw.serial <= 0)
		{
			R_SB = 0xFF;
			R_SC &= 0x7f;
			hw.serial = 0;
			hw_interrupt(IF_SERIAL, 1);
			hw_interrupt(IF_SERIAL, 0);
		}
	}
}

/* cpu_emulate()
	Emulate CPU for time no less than specified

	cycles - time to emulate, expressed in double-speed cycles
	returns number of cycles emulated

	Might emulate up to cycles+(11) time units (longest op takes 12
	cycles in single-speed mode)
*/
IRAM_ATTR int cpu_emulate(int cycles)
{
	int clen, temp;
	int remaining = cycles;
	int count = 0;
	byte op, b;
	cpu_reg_t acc;

	if (!cpu.double_speed)
		remaining >>= 1;

next:
	/* Skip idle cycles */
	if (cpu.halted) {
		clen = 1;
		goto _skip;
	}

	/* Handle interrupts */
	if (IME && (temp = R_IF & R_IE))
	{
		COND_EXEC_INT(IF_VBLANK, 0);
		COND_EXEC_INT(IF_STAT, 1);
		COND_EXEC_INT(IF_TIMER, 2);
		COND_EXEC_INT(IF_SERIAL, 3);
		COND_EXEC_INT(IF_PAD, 4);
	}
	IME = IMA;

	// if (cpu.disassemble)
	// 	debug_disassemble(PC, 1);

	op = FETCH;
	clen = cycles_table[op];

	switch(op)
	{
	case 0x00: /* NOP */
	case 0x40: /* LD B,B */
	case 0x49: /* LD C,C */
	case 0x52: /* LD D,D */
	case 0x5B: /* LD E,E */
	case 0x64: /* LD H,H */
	case 0x6D: /* LD L,L */
	case 0x7F: /* LD A,A */
		break;

	case 0x41: /* LD B,C */
		B = C; break;
	case 0x42: /* LD B,D */
		B = D; break;
	case 0x43: /* LD B,E */
		B = E; break;
	case 0x44: /* LD B,H */
		B = H; break;
	case 0x45: /* LD B,L */
		B = L; break;
	case 0x46: /* LD B,(HL) */
		B = readb(HL); break;
	case 0x47: /* LD B,A */
		B = A; break;

	case 0x48: /* LD C,B */
		C = B; break;
	case 0x4A: /* LD C,D */
		C = D; break;
	case 0x4B: /* LD C,E */
		C = E; break;
	case 0x4C: /* LD C,H */
		C = H; break;
	case 0x4D: /* LD C,L */
		C = L; break;
	case 0x4E: /* LD C,(HL) */
		C = readb(HL); break;
	case 0x4F: /* LD C,A */
		C = A; break;

	case 0x50: /* LD D,B */
		D = B; break;
	case 0x51: /* LD D,C */
		D = C; break;
	case 0x53: /* LD D,E */
		D = E; break;
	case 0x54: /* LD D,H */
		D = H; break;
	case 0x55: /* LD D,L */
		D = L; break;
	case 0x56: /* LD D,(HL) */
		D = readb(HL); break;
	case 0x57: /* LD D,A */
		D = A; break;

	case 0x58: /* LD E,B */
		E = B; break;
	case 0x59: /* LD E,C */
		E = C; break;
	case 0x5A: /* LD E,D */
		E = D; break;
	case 0x5C: /* LD E,H */
		E = H; break;
	case 0x5D: /* LD E,L */
		E = L; break;
	case 0x5E: /* LD E,(HL) */
		E = readb(HL); break;
	case 0x5F: /* LD E,A */
		E = A; break;

	case 0x60: /* LD H,B */
		H = B; break;
	case 0x61: /* LD H,C */
		H = C; break;
	case 0x62: /* LD H,D */
		H = D; break;
	case 0x63: /* LD H,E */
		H = E; break;
	case 0x65: /* LD H,L */
		H = L; break;
	case 0x66: /* LD H,(HL) */
		H = readb(HL); break;
	case 0x67: /* LD H,A */
		H = A; break;

	case 0x68: /* LD L,B */
		L = B; break;
	case 0x69: /* LD L,C */
		L = C; break;
	case 0x6A: /* LD L,D */
		L = D; break;
	case 0x6B: /* LD L,E */
		L = E; break;
	case 0x6C: /* LD L,H */
		L = H; break;
	case 0x6E: /* LD L,(HL) */
		L = readb(HL); break;
	case 0x6F: /* LD L,A */
		L = A; break;

	case 0x70: /* LD (HL),B */
		writeb(HL, B); break;
	case 0x71: /* LD (HL),C */
		writeb(HL, C); break;
	case 0x72: /* LD (HL),D */
		writeb(HL, D); break;
	case 0x73: /* LD (HL),E */
		writeb(HL, E); break;
	case 0x74: /* LD (HL),H */
		writeb(HL, H); break;
	case 0x75: /* LD (HL),L */
		writeb(HL, L); break;
	case 0x77: /* LD (HL),A */
		writeb(HL, A); break;

	case 0x78: /* LD A,B */
		A = B; break;
	case 0x79: /* LD A,C */
		A = C; break;
	case 0x7A: /* LD A,D */
		A = D; break;
	case 0x7B: /* LD A,E */
		A = E; break;
	case 0x7C: /* LD A,H */
		A = H; break;
	case 0x7D: /* LD A,L */
		A = L; break;
	case 0x7E: /* LD A,(HL) */
		A = readb(HL); break;

	case 0x01: /* LD BC,imm */
		BC = readw(PC); PC += 2; break;
	case 0x11: /* LD DE,imm */
		DE = readw(PC); PC += 2; break;
	case 0x21: /* LD HL,imm */
		HL = readw(PC); PC += 2; break;
	case 0x31: /* LD SP,imm */
		SP = readw(PC); PC += 2; break;

	case 0x02: /* LD (BC),A */
		writeb(BC, A); break;
	case 0x0A: /* LD A,(BC) */
		A = readb(BC); break;
	case 0x12: /* LD (DE),A */
		writeb(DE, A); break;
	case 0x1A: /* LD A,(DE) */
		A = readb(DE); break;

	case 0x22: /* LDI (HL),A */
		writeb(HL, A); HL++; break;
	case 0x2A: /* LDI A,(HL) */
		A = readb(HL); HL++; break;
	case 0x32: /* LDD (HL),A */
		writeb(HL, A); HL--; break;
	case 0x3A: /* LDD A,(HL) */
		A = readb(HL); HL--; break;

	case 0x06: /* LD B,imm */
		B = FETCH; break;
	case 0x0E: /* LD C,imm */
		C = FETCH; break;
	case 0x16: /* LD D,imm */
		D = FETCH; break;
	case 0x1E: /* LD E,imm */
		E = FETCH; break;
	case 0x26: /* LD H,imm */
		H = FETCH; break;
	case 0x2E: /* LD L,imm */
		L = FETCH; break;
	case 0x36: /* LD (HL),imm */
		writeb(HL, FETCH); break;
	case 0x3E: /* LD A,imm */
		A = FETCH; break;

	case 0x08: /* LD (imm),SP */
		writew(readw(PC), SP); PC += 2; break;
	case 0xEA: /* LD (imm),A */
		writeb(readw(PC), A); PC += 2; break;

	case 0xE0: /* LDH (imm),A */
		writeb(0xff00 + FETCH, A); break;
	case 0xE2: /* LDH (C),A */
		writeb(0xff00 + C, A); break;
	case 0xF0: /* LDH A,(imm) */
		A = readb(0xff00 + FETCH); break;
	case 0xF2: /* LDH A,(C) (undocumented) */
		A = readb(0xff00 + C); break;

	case 0xF8: /* LD HL,SP+imm */
		// https://gammpei.github.io/blog/posts/2018-03-04/how-to-write-a-game-boy-emulator-part-8-blarggs-cpu-test-roms-1-3-4-5-7-8-9-10-11.html
		b = FETCH;
		temp = (int)(SP) + (signed char)b;

		F &= ~(FZ | FN | FH | FC);

		if (((SP & 0xff) ^ b ^ temp) & 0x10) F |= FH;
		if ((SP & 0xff) + b > 0xff) F |= FC;

		HL = temp;
		break;
	case 0xF9: /* LD SP,HL */
		SP = HL; break;
	case 0xFA: /* LD A,(imm) */
		A = readb(readw(PC)); PC += 2; break;

		ALU_CASES(0x80, 0xC6, ADD, __ADD)
		ALU_CASES(0x88, 0xCE, ADC, __ADC)
		ALU_CASES(0x90, 0xD6, SUB, __SUB)
		ALU_CASES(0x98, 0xDE, SBC, __SBC)
		ALU_CASES(0xA0, 0xE6, AND, __AND)
		ALU_CASES(0xA8, 0xEE, XOR, __XOR)
		ALU_CASES(0xB0, 0xF6, OR, __OR)
		ALU_CASES(0xB8, 0xFE, CP, __CP)

	case 0x09: /* ADD HL,BC */
		ADDW(BC); break;
	case 0x19: /* ADD HL,DE */
		ADDW(DE); break;
	case 0x39: /* ADD HL,SP */
		ADDW(SP); break;
	case 0x29: /* ADD HL,HL */
		ADDW(HL); break;

	case 0x04: /* INC B */
		INC(B); break;
	case 0x0C: /* INC C */
		INC(C); break;
	case 0x14: /* INC D */
		INC(D); break;
	case 0x1C: /* INC E */
		INC(E); break;
	case 0x24: /* INC H */
		INC(H); break;
	case 0x2C: /* INC L */
		INC(L); break;
	case 0x34: /* INC (HL) */
		b = readb(HL);
		INC(b);
		writeb(HL, b);
		break;
	case 0x3C: /* INC A */
		INC(A); break;

	case 0x03: /* INC BC */
		INCW(BC); break;
	case 0x13: /* INC DE */
		INCW(DE); break;
	case 0x23: /* INC HL */
		INCW(HL); break;
	case 0x33: /* INC SP */
		INCW(SP); break;

	case 0x05: /* DEC B */
		DEC(B); break;
	case 0x0D: /* DEC C */
		DEC(C); break;
	case 0x15: /* DEC D */
		DEC(D); break;
	case 0x1D: /* DEC E */
		DEC(E); break;
	case 0x25: /* DEC H */
		DEC(H); break;
	case 0x2D: /* DEC L */
		DEC(L); break;
	case 0x35: /* DEC (HL) */
		b = readb(HL);
		DEC(b);
		writeb(HL, b);
		break;
	case 0x3D: /* DEC A */
		DEC(A); break;

	case 0x0B: /* DEC BC */
		DECW(BC); break;
	case 0x1B: /* DEC DE */
		DECW(DE); break;
	case 0x2B: /* DEC HL */
		DECW(HL); break;
	case 0x3B: /* DEC SP */
		DECW(SP); break;

	case 0x07: /* RLCA */
		RLCA(A); break;
	case 0x0F: /* RRCA */
		RRCA(A); break;
	case 0x17: /* RLA */
		RLA(A); break;
	case 0x1F: /* RRA */
		RRA(A); break;

	case 0x27: /* DAA */
		//http://forums.nesdev.com/viewtopic.php?t=9088
		temp = A;

		if (F & FN)
		{
			if (F & FH)	temp = (temp - 6) & 0xff;
			if (F & FC) temp -= 0x60;
		}
		else
		{
			if ((F & FH) || ((temp & 0x0f) > 9)) temp += 0x06;
			if ((F & FC) || (temp > 0x9f)) temp += 0x60;
		}

		A = (byte)temp;
		F &= ~(FH | FZ);

		if (temp & 0x100)   F |= FC;
		if (!(temp & 0xff)) F |= FZ;
		break;

	case 0x2F: /* CPL */
		CPL(A); break;

	case 0x18: /* JR */
		JR; break;
	case 0x20: /* JR NZ */
		if (!(F&FZ)) JR; else NOJR; break;
	case 0x28: /* JR Z */
		if (F&FZ) JR; else NOJR; break;
	case 0x30: /* JR NC */
		if (!(F&FC)) JR; else NOJR; break;
	case 0x38: /* JR C */
		if (F&FC) JR; else NOJR; break;

	case 0xC3: /* JP */
		JP; break;
	case 0xC2: /* JP NZ */
		if (!(F&FZ)) JP; else NOJP; break;
	case 0xCA: /* JP Z */
		if (F&FZ) JP; else NOJP; break;
	case 0xD2: /* JP NC */
		if (!(F&FC)) JP; else NOJP; break;
	case 0xDA: /* JP C */
		if (F&FC) JP; else NOJP; break;
	case 0xE9: /* JP HL */
		PC = HL; break;

	case 0xC9: /* RET */
		RET; break;
	case 0xC0: /* RET NZ */
		if (!(F&FZ)) RET; else NORET; break;
	case 0xC8: /* RET Z */
		if (F&FZ) RET; else NORET; break;
	case 0xD0: /* RET NC */
		if (!(F&FC)) RET; else NORET; break;
	case 0xD8: /* RET C */
		if (F&FC) RET; else NORET; break;
	case 0xD9: /* RETI */
		IME = IMA = 1; RET; break;

	case 0xCD: /* CALL */
		CALL; break;
	case 0xC4: /* CALL NZ */
		if (!(F&FZ)) CALL; else NOCALL; break;
	case 0xCC: /* CALL Z */
		if (F&FZ) CALL; else NOCALL; break;
	case 0xD4: /* CALL NC */
		if (!(F&FC)) CALL; else NOCALL; break;
	case 0xDC: /* CALL C */
		if (F&FC) CALL; else NOCALL; break;

	case 0xC7: /* RST 0 */
		RST(0x00); break;
	case 0xCF: /* RST 8 */
		RST(0x08); break;
	case 0xD7: /* RST 10 */
		RST(0x10); break;
	case 0xDF: /* RST 18 */
		RST(0x18); break;
	case 0xE7: /* RST 20 */
		RST(0x20); break;
	case 0xEF: /* RST 28 */
		RST(0x28); break;
	case 0xF7: /* RST 30 */
		RST(0x30); break;
	case 0xFF: /* RST 38 */
		RST(0x38); break;

	case 0xC1: /* POP BC */
		POP(BC); break;
	case 0xC5: /* PUSH BC */
		PUSH(BC); break;
	case 0xD1: /* POP DE */
		POP(DE); break;
	case 0xD5: /* PUSH DE */
		PUSH(DE); break;
	case 0xE1: /* POP HL */
		POP(HL); break;
	case 0xE5: /* PUSH HL */
		PUSH(HL); break;
	case 0xF1: /* POP AF */
		POP(AF); AF &= 0xfff0; break;
	case 0xF5: /* PUSH AF */
		PUSH(AF); break;

	case 0xE8: /* ADD SP,imm */
		// https://gammpei.github.io/blog/posts/2018-03-04/how-to-write-a-game-boy-emulator-part-8-blarggs-cpu-test-roms-1-3-4-5-7-8-9-10-11.html
		b = FETCH; // ADDSP(b); break;
		temp = SP + (signed char)b;

		F &= ~(FZ | FN | FH | FC);

		if ((SP ^ b ^ temp) & 0x10) F |= FH;
		if ((SP & 0xff) + b > 0xff) F |= FC;

		SP = temp;
		break;

	case 0xF3: /* DI */
		DI; break;
	case 0xFB: /* EI */
		EI; break;

	case 0x37: /* SCF */
		SCF; break;
	case 0x3F: /* CCF */
		CCF; break;

	case 0x10: /* STOP */
		PC++;
		if (R_KEY1 & 1)
		{
			cpu.double_speed ^= 1;
			R_KEY1 = (R_KEY1 & 0x7E) | ((cpu.double_speed & 1) << 7);
			break;
		}
		/* NOTE - we do not implement dmg STOP whatsoever */
		break;

	case 0x76: /* HALT */
		cpu.halted = 1;
		if (!IME)
		{
			MESSAGE_DEBUG("FIX ME: HALT requested with IME = 0\n");
		}
		break;

	case 0xCB: /* CB prefix */
		op = FETCH;
		clen = cb_cycles_table[op];
		switch (op)
		{
			CB_REG_CASES(B, 0);
			CB_REG_CASES(C, 1);
			CB_REG_CASES(D, 2);
			CB_REG_CASES(E, 3);
			CB_REG_CASES(H, 4);
			CB_REG_CASES(L, 5);
			CB_REG_CASES(A, 7);
		default:
			b = readb(HL);
			switch (op)
			{
				CB_REG_CASES(b, 6);
			}
			if ((op & 0xC0) != 0x40) /* exclude BIT */
				writeb(HL, b);
			break;
		}
		break;

	default:
		MESSAGE_ERROR("invalid opcode 0x%02X at address 0x%04X, rombank = %d\n",
			op, (PC-1) & 0xffff, hw.cart->rombank);
		break; // abort();
	}

_skip:

	remaining -= clen;
	count += clen;

#if COUNTERS_TICK_PERIOD > 1
	if (count >= COUNTERS_TICK_PERIOD || remaining <= 0)
#endif
	{
		/* Advance clock-bound counters */
		timer_advance(count);
		serial_advance(count);

		if (!cpu.double_speed)
			count <<= 1;

		/* Advance fixed-speed counters */
		lcd_emulate(count);
		sound_advance(count);
		// sound_emulate(count);

		// Here we could calculate when the next event is going to happen
		// So that we can skip even more cycles, but it doesn't seem to save
		// much more CPU and adds lots of complexity...
		count = 0;
	}

	if (remaining > 0)
		goto next;

	return cycles + -remaining;
}


#ifdef CPU_DISASSEMBLER

static const char *mnemonic_table[256] =
{
	"NOP",            "LD BC,%w",  "LD (BC),A",     "INC BC",    "INC B",      "DEC B",     "LD B,%b",     "RLCA",      "LD (%w),SP", "ADD HL,BC",  "LD A,(BC)",   "DEC BC",    "INC C",     "DEC C",     "LD C,%b",     "RRCA",
	"STOP",           "LD DE,%w",  "LD (DE),A",     "INC DE",    "INC D",      "DEC D",     "LD D,%b",     "RLA",       "JR %o",      "ADD HL,DE",  "LD A,(DE)",   "DEC DE",    "INC E",     "DEC E",     "LD E,%b",     "RRA",
	"JR NZ,%o",       "LD HL,%w",  "LD (HLI),A",    "INC HL",    "INC H",      "DEC H",     "LD H,%b",     "DAA",       "JR Z,%o",    "ADD HL,HL",  "LD A,(HLI)",  "DEC HL",    "INC L",     "DEC L",     "LD L,%b",     "CPL",
	"JR NC,%o",       "LD SP,%w",  "LD (HLD),A",    "INC SP",    "INC (HL)",   "DEC (HL)",  "LD (HL),%b",  "SCF",       "JR C,%o",    "ADD HL,SP",  "LD A,(HLD)",  "DEC SP",    "INC A",     "DEC A",     "LD A,%b",     "CCF",
	"LD B,B",         "LD B,C",    "LD B,D",        "LD B,E",    "LD B,H",     "LD B,L",    "LD B,(HL)",   "LD B,A",    "LD C,B",     "LD C,C",     "LD C,D",      "LD C,E",    "LD C,H",    "LD C,L",    "LD C,(HL)",   "LD C,A",
	"LD D,B",         "LD D,C",    "LD D,D",        "LD D,E",    "LD D,H",     "LD D,L",    "LD D,(HL)",   "LD D,A",    "LD E,B",     "LD E,C",     "LD E,D",      "LD E,E",    "LD E,H",    "LD E,L",    "LD E,(HL)",   "LD E,A",
	"LD H,B",         "LD H,C",    "LD H,D",        "LD H,E",    "LD H,H",     "LD H,L",    "LD H,(HL)",   "LD H,A",    "LD L,B",     "LD L,C",     "LD L,D",      "LD L,E",    "LD L,H",    "LD L,L",    "LD L,(HL)",   "LD L,A",
	"LD (HL),B",      "LD (HL),C", "LD (HL),D",     "LD (HL),E", "LD (HL),H",  "LD (HL),L", "HALT",        "LD (HL),A", "LD A,B",     "LD A,C",     "LD A,D",      "LD A,E",    "LD A,H",    "LD A,L",    "LD A,(HL)",   "LD A,A",
	"ADD A,B",        "ADD A,C",   "ADD A,D",       "ADD A,E",   "ADD A,H",    "ADD A,L",   "ADD A,(HL)",  "ADD A,A",   "ADC A,B",    "ADC A,C",    "ADC A,D",     "ADC A,E",   "ADC A,H",   "ADC A,L",   "ADC A,(HL)",  "ADC A",
	"SUB B",          "SUB C",     "SUB D",         "SUB E",     "SUB H",      "SUB L",     "SUB (HL)",    "SUB A",     "SBC A,B",    "SBC A,C",    "SBC A,D",     "SBC A,E",   "SBC A,H",   "SBC A,L",   "SBC A,(HL)",  "SBC A,A",
	"AND B",          "AND C",     "AND D",         "AND E",     "AND H",      "AND L",     "AND (HL)",    "AND A",     "XOR B",      "XOR C",      "XOR D",       "XOR E",     "XOR H",     "XOR L",     "XOR (HL)",    "XOR A",
	"OR B",           "OR C",      "OR D",          "OR E",      "OR H",       "OR L",      "OR (HL)",     "OR A",      "CP B",       "CP C",       "CP D",        "CP E",      "CP H",      "CP L",      "CP (HL)",     "CP A",
	"RET NZ",         "POP BC",    "JP NZ,%w",      "JP %w",     "CALL NZ,%w", "PUSH BC",   "ADD A,%b",    "RST 0h",    "RET Z",      "RET",        "JP Z,%w",     "*INVALID*", "CALL Z,%w", "CALL %w",   "ADC A,%b",    "RST 8h",
	"RET NC",         "POP DE",    "JP NC,%w",      "*INVALID*", "CALL NC,%w", "PUSH DE",   "SUB %b",      "RST 10h",   "RET C",      "RETI",       "JP C,%w",     "*INVALID*", "CALL C,%w", "*INVALID*", "SBC A,%b",    "RST 18h",
	"LD (FF00+%b),A", "POP HL",    "LD (FF00+C),A", "*INVALID*", "*INVALID*",  "PUSH HL",   "AND %b",      "RST 20h",   "ADD SP,%o",  "JP HL",      "LD (%w),A",   "*INVALID*", "*INVALID*", "*INVALID*", "XOR %b",      "RST 28h",
	"LD A,(FF00+%b)", "POP AF",    "LD A,(FF00+C)", "DI",        "*INVALID*",  "PUSH AF",    "OR %b",      "RST 30h",   "LD HL,SP%o", "LD SP,HL",   "LD A,(%w)",   "EI",        "*INVALID*", "*INVALID*", "CP %b",       "RST 38h",
};

static const char *cb_mnemonic_table[256] =
{
	"RLC B",   "RLC C",   "RLC D",   "RLC E",   "RLC H",   "RLC L",   "RLC (HL)",   "RLC A",
	"RRC B",   "RRC C",   "RRC D",   "RRC E",   "RRC H",   "RRC L",   "RRC (HL)",   "RRC A",
	"RL B",    "RL C",    "RL D",    "RL E",    "RL H",    "RL L",    "RL (HL)",    "RL A",
	"RR B",    "RR C",    "RR D",    "RR E",    "RR H",    "RR L",    "RR (HL)",    "RR A",
	"SLA B",   "SLA C",   "SLA D",   "SLA E",   "SLA H",   "SLA L",   "SLA (HL)",   "SLA A",
	"SRA B",   "SRA C",   "SRA D",   "SRA E",   "SRA H",   "SRA L",   "SRA (HL)",   "SRA A",
	"SWAP B",  "SWAP C",  "SWAP D",  "SWAP E",  "SWAP H",  "SWAP L",  "SWAP (HL)",  "SWAP A",
	"SRL B",   "SRL C",   "SRL D",   "SRL E",   "SRL H",   "SRL L",   "SRL (HL)",   "SRL A",
	"BIT 0,B", "BIT 0,C", "BIT 0,D", "BIT 0,E", "BIT 0,H", "BIT 0,L", "BIT 0,(HL)", "BIT 0,A",
	"BIT 1,B", "BIT 1,C", "BIT 1,D", "BIT 1,E", "BIT 1,H", "BIT 1,L", "BIT 1,(HL)", "BIT 1,A",
	"BIT 2,B", "BIT 2,C", "BIT 2,D", "BIT 2,E", "BIT 2,H", "BIT 2,L", "BIT 2,(HL)", "BIT 2,A",
	"BIT 3,B", "BIT 3,C", "BIT 3,D", "BIT 3,E", "BIT 3,H", "BIT 3,L", "BIT 3,(HL)", "BIT 3,A",
	"BIT 4,B", "BIT 4,C", "BIT 4,D", "BIT 4,E", "BIT 4,H", "BIT 4,L", "BIT 4,(HL)", "BIT 4,A",
	"BIT 5,B", "BIT 5,C", "BIT 5,D", "BIT 5,E", "BIT 5,H", "BIT 5,L", "BIT 5,(HL)", "BIT 5,A",
	"BIT 6,B", "BIT 6,C", "BIT 6,D", "BIT 6,E", "BIT 6,H", "BIT 6,L", "BIT 6,(HL)", "BIT 6,A",
	"BIT 7,B", "BIT 7,C", "BIT 7,D", "BIT 7,E", "BIT 7,H", "BIT 7,L", "BIT 7,(HL)", "BIT 7,A",
	"RES 0,B", "RES 0,C", "RES 0,D", "RES 0,E", "RES 0,H", "RES 0,L", "RES 0,(HL)", "RES 0,A",
	"RES 1,B", "RES 1,C", "RES 1,D", "RES 1,E", "RES 1,H", "RES 1,L", "RES 1,(HL)", "RES 1,A",
	"RES 2,B", "RES 2,C", "RES 2,D", "RES 2,E", "RES 2,H", "RES 2,L", "RES 2,(HL)", "RES 2,A",
	"RES 3,B", "RES 3,C", "RES 3,D", "RES 3,E", "RES 3,H", "RES 3,L", "RES 3,(HL)", "RES 3,A",
	"RES 4,B", "RES 4,C", "RES 4,D", "RES 4,E", "RES 4,H", "RES 4,L", "RES 4,(HL)", "RES 4,A",
	"RES 5,B", "RES 5,C", "RES 5,D", "RES 5,E", "RES 5,H", "RES 5,L", "RES 5,(HL)", "RES 5,A",
	"RES 6,B", "RES 6,C", "RES 6,D", "RES 6,E", "RES 6,H", "RES 6,L", "RES 6,(HL)", "RES 6,A",
	"RES 7,B", "RES 7,C", "RES 7,D", "RES 7,E", "RES 7,H", "RES 7,L", "RES 7,(HL)", "RES 7,A",
	"SET 0,B", "SET 0,C", "SET 0,D", "SET 0,E", "SET 0,H", "SET 0,L", "SET 0,(HL)", "SET 0,A",
	"SET 1,B", "SET 1,C", "SET 1,D", "SET 1,E", "SET 1,H", "SET 1,L", "SET 1,(HL)", "SET 1,A",
	"SET 2,B", "SET 2,C", "SET 2,D", "SET 2,E", "SET 2,H", "SET 2,L", "SET 2,(HL)", "SET 2,A",
	"SET 3,B", "SET 3,C", "SET 3,D", "SET 3,E", "SET 3,H", "SET 3,L", "SET 3,(HL)", "SET 3,A",
	"SET 4,B", "SET 4,C", "SET 4,D", "SET 4,E", "SET 4,H", "SET 4,L", "SET 4,(HL)", "SET 4,A",
	"SET 5,B", "SET 5,C", "SET 5,D", "SET 5,E", "SET 5,H", "SET 5,L", "SET 5,(HL)", "SET 5,A",
	"SET 6,B", "SET 6,C", "SET 6,D", "SET 6,E", "SET 6,H", "SET 6,L", "SET 6,(HL)", "SET 6,A",
	"SET 7,B", "SET 7,C", "SET 7,D", "SET 7,E", "SET 7,H", "SET 7,L", "SET 7,(HL)", "SET 7,A",
};

void cpu_disassemble(unsigned pc, int count)
{
	while (count-- > 0)
	{
		const char *pattern;
		char operands[16]
		char mnemonic[64];
		byte ops[3];
		int j = 0, k = 0;
		int baseaddr = pc;
		int opcode = readb(pc++);

		ops[k++] = opcode;

		if (opcode == 0xCB)
		{
			opcode = ops[k++] = readb(pc++);
			pattern = cb_mnemonic_table[opcode];
		}
		else
		{
			pattern = mnemonic_table[opcode];
		}

		while (*pattern)
		{
			if (*pattern == '%')
			{
				pattern++;
				switch (*pattern)
				{
				case 'B':
				case 'b':
					ops[k] = readb(pc++);
					j += sprintf(mnemonic + j, "%02Xh", ops[k++]);
					break;
				case 'W':
				case 'w':
					ops[k] = readb(pc++);
					ops[k+1] = readb(pc++);
					j += sprintf(mnemonic + j, "%04Xh", ((ops[k+1] << 8) | ops[k]));
					k += 2;
					break;
				case 'O':
				case 'o':
					ops[k] = readb(pc++);
					j += sprintf(mnemonic + j, "%+d", (n8)(ops[k++]));
					break;
				}
			}
			else
			{
				mnemonic[j++] = *pattern++;
			}
		}
		mnemonic[j] = 0;

		if (k == 3)
			sprintf(operands, "%02X %02X %02X", ops[0], ops[1], ops[2]);
		else if (k == 2)
			sprintf(operands, "%02X", ops[0], ops[1]);
		else
			sprintf(operands, "%02X", ops[0]);

		printf(
			"%04X: %-10.10s %-16.16s"
			" SP=%04X.%04X BC=%04X.%02X.%02X DE=%04X.%02X"
			" HL=%04X.%02X A=%02X F=%02X %c%c%c%c%c"
			" IE=%02X IF=%02X LCDC=%02X STAT=%02X LY=%02X LYC=%02X"
			" \n",
			baseaddr,
			operands,
			mnemonic,
			SP, readw(SP),
			BC, readb(BC), readb(0xFF00 | C),
			DE, readb(DE),
			HL, readb(HL), A,
			F, (IME ? 'I' : '-'),
			((F & 0x80) ? 'Z' : '-'),
			((F & 0x40) ? 'N' : '-'),
			((F & 0x20) ? 'H' : '-'),
			((F & 0x10) ? 'C' : '-'),
			R_IE, R_IF, R_LCDC, R_STAT, R_LY, R_LYC
		);
	}
}

#endif
