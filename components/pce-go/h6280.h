#pragma once

#include <stdint.h>

void h6280_reset(void);
void h6280_run(int cycles);
void h6280_irq(int);
void h6280_dump_state(void);
void h6280_disassemble(void);

typedef struct
{
	/* Registers */
	uint16_t PC;
	uint8_t A;
	uint8_t X;
	uint8_t Y;
	uint8_t P;
	uint8_t S;

	/* Interrupts */
	uint8_t irq_mask;
	uint8_t irq_mask_delay;
	uint8_t irq_lines;

	/* Misc */
	uint32_t cycles;
	uint32_t halted;
} h6280_t;

// CPU Flags:
#define FL_N       0x80
#define FL_V       0x40
#define FL_T       0x20
#define FL_B       0x10
#define FL_D       0x08
#define FL_I       0x04
#define FL_Z       0x02
#define FL_C       0x01

// Interrupts
#define INT_IRQ2   0x01
#define INT_IRQ1   0x02
#define INT_TIMER  0x04
#define INT_MASK   0x07

// Vectors
#define VEC_RESET  0xFFFE
#define VEC_NMI    0xFFFC
#define VEC_TIMER  0xFFFA
#define VEC_IRQ1   0xFFF8
#define VEC_IRQ2   0xFFF6
#define VEC_BRK    0xFFF6

extern h6280_t CPU;
