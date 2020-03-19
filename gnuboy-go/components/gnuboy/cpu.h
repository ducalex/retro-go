#ifndef __CPU_H__
#define __CPU_H__

#include "defs.h"

union reg
{
	byte b[2];
	word w;
};

struct cpu
{
	union reg pc, sp, bc, de, hl, af;

	int ime, ima;
	int speed;
	int halt;

	// Timers/counters
	int div, timer;
	int lcdc;
	int sound;
	int serial;
};

extern int debug_trace;
extern struct cpu cpu;

void cpu_reset();
int  cpu_emulate(int cycles);
void cpu_timers(int cnt);

#endif
