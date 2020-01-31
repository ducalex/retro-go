

#ifndef __CPUREGS_H__

#define __CPUREGS_H__



#include "defs.h"
#include "cpu.h"

#define LB(r) ((r).b[LO][LO])
#define HB(r) ((r).b[LO][HI])
#define W(r) ((r).w[LO])
#define DW(r) ((r).d)

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

#define xAF DW(cpu.af)
#define xBC DW(cpu.bc)
#define xDE DW(cpu.de)
#define xHL DW(cpu.hl)

#define xPC DW(cpu.pc)
#define xSP DW(cpu.sp)

#define IMA cpu.ima
#define IME cpu.ime
#define IF R_IF
#define IE R_IE

#define FZ 0x80
#define FN 0x40
#define FH 0x20
#define FC 0x10
#define FL 0x0F /* low unused portion of flags */


#endif /* __CPUREGS_H__ */
