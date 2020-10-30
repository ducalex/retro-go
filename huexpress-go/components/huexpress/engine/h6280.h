#ifndef H6280_H_
#define H6280_H_

#include "hard_pce.h"

extern void exe_go(void);
extern void dump_cpu_registers();

typedef struct op {
   int (*func_exe)(void);
   int16  addr_mode;
   const char opname[4];
} operation_t;

extern operation_t optable_runtime[256];

// CPU Flags:
#define FL_N 0x80
#define FL_V 0x40
#define FL_T 0x20
#define FL_B 0x10
#define FL_D 0x08
#define FL_I 0x04
#define FL_Z 0x02
#define FL_C 0x01

// Interrupts
#define INT_NONE        0		/* No interrupt required      */
#define INT_IRQ         1		/* Standard IRQ interrupt     */
#define INT_NMI         2		/* Non-maskable interrupt     */
#define INT_QUIT        3		/* Exit the emulation         */
#define INT_TIMER       4
#define INT_IRQ2        8

// Interrupt flags
#define FL_IRQ2    0x01
#define FL_IRQ1    0x02
#define FL_TIQ     0x04

// Vectors
#define	VEC_RESET	0xFFFE
#define	VEC_NMI		0xFFFC
#define	VEC_TIMER	0xFFFA
#define	VEC_IRQ		0xFFF8
#define	VEC_IRQ2	   0xFFF6
#define	VEC_BRK		0xFFF6

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


#define get_8bit_addr(addr) Read8(addr)
#define put_8bit_addr(addr, byte) Write8(addr, byte)
#define get_16bit_addr(addr) Read16(addr)

#endif
