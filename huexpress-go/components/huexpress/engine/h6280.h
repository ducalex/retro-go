#ifndef H6280_H_
#define H6280_H_

#include "hard_pce.h"

extern void h6280_reset(void);
extern void h6280_run(void);
extern void h6280_irq(int);
extern void h6280_debug(void);
extern void h6280_print_state(void);

typedef struct op {
   int (*func_exe)(void);
   short addr_mode;
   const char opname[4];
} operation_t;

typedef struct {
	// CPU Registers
	uint16_t reg_pc;
	uint8_t reg_a;
	uint8_t reg_x;
	uint8_t reg_y;
	uint8_t reg_p;
	uint8_t reg_s;
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

// Addressing modes
#define AM_IMPL      0			/* implicit              */
#define AM_IMMED     1			/* immediate             */
#define AM_REL       2			/* relative              */
#define AM_ZP        3			/* zero page             */
#define AM_ZPX       4			/* zero page, x          */
#define AM_ZPY       5			/* zero page, y          */
#define AM_ZPIND     6			/* zero page indirect    */
#define AM_ZPINDX    7			/* zero page indirect, x */
#define AM_ZPINDY    8			/* zero page indirect, y */
#define AM_ABS       9			/* absolute              */
#define AM_ABSX     10			/* absolute, x           */
#define AM_ABSY     11			/* absolute, y           */
#define AM_ABSIND   12			/* absolute indirect     */
#define AM_ABSINDX  13			/* absolute indirect     */
#define AM_PSREL    14			/* pseudo-relative       */
#define AM_TST_ZP   15			/* special 'TST' addressing mode  */
#define AM_TST_ABS  16			/* special 'TST' addressing mode  */
#define AM_TST_ZPX  17			/* special 'TST' addressing mode  */
#define AM_TST_ABSX 18			/* special 'TST' addressing mode  */
#define AM_XFER     19			/* special 7-byte transfer addressing mode  */

// CPU registers
#define reg_pc CPU.reg_pc
#define reg_a  CPU.reg_a
#define reg_x  CPU.reg_x
#define reg_y  CPU.reg_y
#define reg_p  CPU.reg_p
#define reg_s  CPU.reg_s

typedef signed char SBYTE;
typedef unsigned char UBYTE;
typedef signed short SWORD;
typedef unsigned short UWORD;

extern h6280_t CPU;
extern operation_t optable_runtime[256];

#endif
