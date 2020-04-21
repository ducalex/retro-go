/****************************************************************************
 globals.h
 ****************************************************************************/

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "hard_pce.h"
#include "defs.h"

//
// Bitmask values for PCE status flags:
//
#define FL_N 0x80
#define FL_V 0x40
#define FL_T 0x20
#define FL_B 0x10
#define FL_D 0x08
#define FL_I 0x04
#define FL_Z 0x02
#define FL_C 0x01

//
// Globals for PCE memory-addressing:
//
extern uchar * base[256];
extern uchar * zp_base;
extern uchar * sp_base;
extern uchar * mmr_base[8];

//
// Globals for PCE CPU registers:
//
/*
extern uint16  reg_pc_;
extern uchar   reg_a;
extern uchar   reg_x;
extern uchar   reg_y;
extern uchar   reg_p;
extern uchar   reg_s;
*/
extern uchar * mmr;

//
// Globals which hold emulation-realted info:
//
extern uchar   halt_flag;

//
// Tables for disassembly/execution of PCE opcodes:
//
extern mode_struct addr_info[];
extern operation optable[];


#endif
