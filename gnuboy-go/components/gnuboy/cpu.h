#ifndef __CPU_H__
#define __CPU_H__

#include "emu.h"

/* Quick access CPU registers */
#define LB(r) ((r).b[LO])
#define HB(r) ((r).b[HI])
#define W(r) ((r).w)

#define A HB(cpu.af)
#define F LB(cpu.af)
#define B HB(cpu.bc)
#define C LB(cpu.bc)
#define D HB(cpu.de)
#define E LB(cpu.de)
#define H HB(cpu.hl)
#define L LB(cpu.hl)

#define AF W(cpu.af)
#define BC W(cpu.bc)
#define DE W(cpu.de)
#define HL W(cpu.hl)

#define PC W(cpu.pc)
#define SP W(cpu.sp)

#define IMA cpu.ima
#define IME cpu.ime

#define FZ 0x80
#define FN 0x40
#define FH 0x20
#define FC 0x10
#define FL 0x0F /* low unused portion of flags */

typedef union
{
	byte b[2];
	word w;
} cpu_reg_t;

typedef struct
{
	cpu_reg_t pc, sp, bc, de, hl, af;

	un32 timer, div;
	un32 ime, ima;
	un32 halted;
	un32 double_speed;
	un32 disassemble;
} cpu_t;

extern cpu_t cpu;

void cpu_reset();
int  cpu_emulate(int cycles);
void cpu_burn(int cycles);

#endif
