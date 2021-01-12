//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

// tlcs900h.cpp: implementation of the tlcs900h class.
//
//  010906 - Checked and fixed ADD/ADC/SUB/SBC/AND/OR/XOR/MUL/MULS/DAA for 100% correctness
//  010324 - Fixed a stupid flag bug in the rotate & shift instructions
//  001230 - Fixed and cleaned XORCF and ORCF instructions
// 991225 - Added the previous register bank
//  991219 - Finished redoing all the flag handling, time to do some bugfixing again...
//  991215 - Hopefully fixed the V flag in the ADD, ADC, SUB and SBC operations
//  991210 - Fixed some minor bugs after a cdoe read
//  991207 - Fixed a bug in DIVS and a bug in the bit test operations
//  991127 - Redid the signed conditionals, hopefully good this time
//
// TODO:
//  - Check state counting for rlc,rrc,rl4, rrc, sla, sra, srl
//  - Optimizing repeating instructions, such as LDIR, LDDIR, etc.
//    I took out the optimizing instructions to be able to implement
//    accurate timers. Might put these back as soon as i have the timers
//    up and running in a stable state....
//  - Implementation MULA operation incomplete (V flag)
//
//////////////////////////////////////////////////////////////////////

#ifndef __GP32__
#include "StdAfx.h"
#else
extern int turbo;
extern int fskip;
#endif
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "tlcs900h.h"
#include "memory.h"
//#include "mainemu.h"
#include "input.h"
#include "graphics.h"
#include "ngpBios.h"
#include "neopopsound.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ngpcdis.cpp
//
// Emulator for tlcs-900H based on:
//   Disassembler for a neogeo pocket color rom (TLCS-900H cpu)
//
// TODO:
//  - Fix Bugs!
//
//
// there don't seem to be any direct calls into the bios. The way bios calls seem to be
// implemented is through SWI 1. SWI 1 is called and some parameters are passed in some
// registers.
//
//
// the TLCS-900H has several registers:
// 8-bit registers: W, A, B, C, D, E, H, L
// 16-bit registers: WA, BC, DE, HL, IX, IY, IZ, SP
// 32-bit registers: XWA, XBC, XDE, XHL, XIZ, XIY, XIZ, XSP
//
// the encoding of general 8/16/32 bit registers is unknown at this point; ie, which
// register number refers to which register (part) name.
//   the encoding is a tad different for different size modes (B,W,L)
//
// probable setup of the register numbers:
// 0x0x - register bank 0
// 0x1x - register bank 1
// 0x2x - register bank 2
// 0x3x - register bank 3
// 0xDx - previous register bank (?)
// 0xEx - current register bank
// 0xFx - special register bank (XIX, XIY, XIZ, XSP)
//  RA, RW, QA, QW, etc.. the true LSB MSB order seems to give the best results :)
//
// Roms start at address 0x00200000
// There seems to be some RAM starting at 0x00004000
//
// Also some special additional registers from this particular member of the 900H
// seems to be getting used, presumably for communication purposes???
//
// Register bank #3 is the 'system' bank and set #2 is the regular work bank????
//
// I think the CPU is a TMP95CS64F, or a special SNK derivation thereof
//
//

unsigned char Ztable[256];            // zero and sign flags table for faster setting
unsigned char SZtable[256];            // zero and sign flags table for faster setting

//#define USE_PARITY_TABLE  //this is currently broken!
#ifdef USE_PARITY_TABLE
unsigned char parityVtable[256];            // zero and sign flags table for faster setting
#endif

//#define TCLS900H_PROFILING
#ifdef TCLS900H_PROFILING
struct profStruct
{
    unsigned int ticks, calls;
    unsigned int decode[256];
};

struct profStruct profile[256];

char *instr_names[256] =
    {
        "nop", "normal", "pushsr", "popsr", "tmax", "halt", "ei", "reti",
        "ld8I", "pushI", "ldw8I", "pushwI", "incf", "decf", "ret", "retd",
        "rcf", "scf", "ccf", "zcf", "pushA", "popA", "exFF", "ldf",
        "pushF", "popF", "jp16", "jp24", "call16", "call24", "calr", "udef",
        "ldRIB", "ldRIB", "ldRIB", "ldRIB", "ldRIB", "ldRIB", "ldRIB", "ldRIB",
        "pushRW", "pushRW", "pushRW", "pushRW", "pushRW", "pushRW", "pushRW", "pushRW",
        "ldRIW", "ldRIW", "ldRIW", "ldRIW", "ldRIW", "ldRIW", "ldRIW", "ldRIW",
        "pushRL", "pushRL", "pushRL", "pushRL", "pushRL", "pushRL", "pushRL", "pushRL",
        //
        "ldRIL", "ldRIL", "ldRIL", "ldRIL", "ldRIL", "ldRIL", "ldRIL", "ldRIL",
        "popRW", "popRW", "popRW", "popRW", "popRW", "popRW", "popRW", "popRW",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "popRL", "popRL", "popRL", "popRL", "popRL", "popRL", "popRL", "popRL",
        "jrcc0", "jrcc1", "jrcc2", "jrcc3", "jrcc4", "jrcc5", "jrcc6", "jrcc7",
        "jrcc8", "jrcc9", "jrccA", "jrccB", "jrccC", "jrccD", "jrccE", "jrccF",
        "jrlcc0", "jrlcc1", "jrlcc2", "jrlcc3", "jrlcc4", "jrlcc5", "jrlcc6", "jrlcc7",
        "jrlcc8", "jrlcc9", "jrlccA", "jrlccB", "jrlccC", "jrlccD", "jrlccE", "jrlccF",
        //
        "decode80", "decode80", "decode80", "decode80", "decode80", "decode80", "decode80", "decode80",
        "decode88", "decode88", "decode88", "decode88", "decode88", "decode88", "decode88", "decode88",
        "decode90", "decode90", "decode90", "decode90", "decode90", "decode90", "decode90", "decode90",
        "decode98", "decode98", "decode98", "decode98", "decode98", "decode98", "decode98", "decode98",
        "decodeA0", "decodeA0", "decodeA0", "decodeA0", "decodeA0", "decodeA0", "decodeA0", "decodeA0",
        "decodeA8", "decodeA8", "decodeA8", "decodeA8", "decodeA8", "decodeA8", "decodeA8", "decodeA8",
        "decodeB0", "decodeB0", "decodeB0", "decodeB0", "decodeB0", "decodeB0", "decodeB0", "decodeB0",
        "decodeB8", "decodeB8", "decodeB8", "decodeBB", "decodeB8", "decodeB8", "decodeB8", "decodeB8",
        //
        "decodeC0", "decodeC1", "decodeC2", "decodeC3", "decodeC4", "decodeC5", "udef", "decodeC7",
        "decodeC8", "decodeC8", "decodeC8", "decodeC8", "decodeC8", "decodeC8", "decodeC8", "decodeC8",
        "decodeD0", "decodeD1", "decodeD2", "decodeD3", "decodeD4", "decodeD5", "udef", "decodeD7",
        "decodeD8", "decodeD8", "decodeD8", "decodeD8", "decodeD8", "decodeD8", "decodeD8", "decodeD8",
        "decodeE0", "decodeE1", "decodeE2", "decodeE3", "decodeE4", "decodeE5", "udef", "decodeE7",
        "decodeE8", "decodeE8", "decodeE8", "decodeE8", "decodeE8", "decodeE8", "decodeE8", "decodeE8",
        "decodeF0", "decodeF1", "decodeF2", "decodeF3", "decodeF4", "decodeF5", "udef", "ldx",
        "swi", "swi", "swi", "swi", "swi", "swi", "swi", "swi"
    };

char *instr_table80[256] =
    {
        "udef", "udef", "udef", "udef", "pushM00", "udef", "rld00", "rrd00",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldi", "ldir", "ldd", "lddr", "cpiB", "cpirB", "cpdB", "cpdrB",
        "udef", "ld16M00", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldRM00", "ldRM00", "ldRM00", "ldRM00", "ldRM00", "ldRM00", "ldRM00", "ldRM00",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "exMRB00", "exMRB00", "exMRB00", "exMRB00", "exMRB00", "exMRB00", "exMRB00", "exMRB00",
        "addMI00", "adcMI00", "subMI00", "sbcMI00", "andMI00", "xorMI00", "orMI00", "cpMI00",
        //
        "mulRMB00", "mulRMB00", "mulRMB00", "mulRMB00", "mulRMB00", "mulRMB00", "mulRMB00", "mulRMB00",
        "mulsRMB00", "mulsRMB00", "mulsRMB00", "mulsRMB00", "mulsRMB00", "mulsRMB00", "mulsRMB00", "mulsRMB00",
        "divRMB00", "divRMB00", "divRMB00", "divRMB00", "divRMB00", "divRMB00", "divRMB00", "divRMB00",
        "divsRMB00", "divsRMB00", "divsRMB00", "divsRMB00", "divsRMB00", "divsRMB00", "divsRMB00", "divsRMB00",
        "inc3M00", "inc3M00", "inc3M00", "inc3M00", "inc3M00", "inc3M00", "inc3M00", "inc3M00",
        "dec3M00", "dec3M00", "dec3M00", "dec3M00", "dec3M00", "dec3M00", "dec3M00", "dec3M00",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "rlcM00", "rrcM00", "rlM00", "rrM00", "slaM00", "sraM00", "sllM00", "srlM00",
        //
        "addRMB00", "addRMB00", "addRMB00", "addRMB00", "addRMB00", "addRMB00", "addRMB00", "addRMB00",
        "addMRB00", "addMRB00", "addMRB00", "addMRB00", "addMRB00", "addMRB00", "addMRB00", "addMRB00",
        "adcRMB00", "adcRMB00", "adcRMB00", "adcRMB00", "adcRMB00", "adcRMB00", "adcRMB00", "adcRMB00",
        "adcMRB00", "adcMRB00", "adcMRB00", "adcMRB00", "adcMRB00", "adcMRB00", "adcMRB00", "adcMRB00",
        "subRMB00", "subRMB00", "subRMB00", "subRMB00", "subRMB00", "subRMB00", "subRMB00", "subRMB00",
        "subMRB00", "subMRB00", "subMRB00", "subMRB00", "subMRB00", "subMRB00", "subMRB00", "subMRB00",
        "sbcRMB00", "sbcRMB00", "sbcRMB00", "sbcRMB00", "sbcRMB00", "sbcRMB00", "sbcRMB00", "sbcRMB00",
        "sbcMRB00", "sbcMRB00", "sbcMRB00", "sbcMRB00", "sbcMRB00", "sbcMRB00", "sbcMRB00", "sbcMRB00",
        //
        "andRMB00", "andRMB00", "andRMB00", "andRMB00", "andRMB00", "andRMB00", "andRMB00", "andRMB00",
        "andMRB00", "andMRB00", "andMRB00", "andMRB00", "andMRB00", "andMRB00", "andMRB00", "andMRB00",
        "xorRMB00", "xorRMB00", "xorRMB00", "xorRMB00", "xorRMB00", "xorRMB00", "xorRMB00", "xorRMB00",
        "xorMRB00", "xorMRB00", "xorMRB00", "xorMRB00", "xorMRB00", "xorMRB00", "xorMRB00", "xorMRB00",
        "orRMB00", "orRMB00", "orRMB00", "orRMB00", "orRMB00", "orRMB00", "orRMB00", "orRMB00",
        "orMRB00", "orMRB00", "orMRB00", "orMRB00", "orMRB00", "orMRB00", "orMRB00", "orMRB00",
        "cpRMB00", "cpRMB00", "cpRMB00", "cpRMB00", "cpRMB00", "cpRMB00", "cpRMB00", "cpRMB00",
        "cpMRB00", "cpMRB00", "cpMRB00", "cpMRB00", "cpMRB00", "cpMRB00", "cpMRB00", "cpMRB00"
    };

char *instr_table90[256] =
    {
        "udef", "udef", "udef", "udef", "pushwM10", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldiw", "ldirw", "lddw", "lddrw", "cpiW", "cpirW", "cpdW", "cpdrW",
        "udef", "ldw16M10", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldRM10", "ldRM10", "ldRM10", "ldRM10", "ldRM10", "ldRM10", "ldRM10", "ldRM10",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "exMRW10", "exMRW10", "exMRW10", "exMRW10", "exMRW10", "exMRW10", "exMRW10", "exMRW10",
        "addwMI10", "adcwMI10", "subwMI10", "sbcwMI10", "andwMI10", "xorwMI10", "orwMI10", "cpwMI10",
        //
        "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10",
        "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10",
        "divRMW10", "divRMW10", "divRMW10", "divRMW10", "divRMW10", "divRMW10", "divRMW10", "divRMW10",
        "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10",
        "incw3M10", "incw3M10", "incw3M10", "incw3M10", "incw3M10", "incw3M10", "incw3M10", "incw3M10",
        "decw3M10", "decw3M10", "decw3M10", "decw3M10", "decw3M10", "decw3M10", "decw3M10", "decw3M10",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "rlcwM10", "rrcwM10", "rlwM10", "rrwM10", "slawM10", "srawM10", "sllwM10", "srlwM10",
        //
        "addRMW10", "addRMW10", "addRMW10", "addRMW10", "addRMW10", "addRMW10", "addRMW10", "addRMW10",
        "addMRW10", "addMRW10", "addMRW10", "addMRW10", "addMRW10", "addMRW10", "addMRW10", "addMRW10",
        "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10",
        "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10",
        "subRMW10", "subRMW10", "subRMW10", "subRMW10", "subRMW10", "subRMW10", "subRMW10", "subRMW10",
        "subMRW10", "subMRW10", "subMRW10", "subMRW10", "subMRW10", "subMRW10", "subMRW10", "subMRW10",
        "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10",
        "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10",
        //
        "andRMW10", "andRMW10", "andRMW10", "andRMW10", "andRMW10", "andRMW10", "andRMW10", "andRMW10",
        "andMRW10", "andMRW10", "andMRW10", "andMRW10", "andMRW10", "andMRW10", "andMRW10", "andMRW10",
        "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10",
        "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10",
        "orRMW10", "orRMW10", "orRMW10", "orRMW10", "orRMW10", "orRMW10", "orRMW10", "orRMW10",
        "orMRW10", "orMRW10", "orMRW10", "orMRW10", "orMRW10", "orMRW10", "orMRW10", "orMRW10",
        "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10",
        "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10"
    };

char *instr_table98[256] =
    {
        "udef", "udef", "udef", "udef", "pushwM10", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "ldw16M10", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldRM10", "ldRM10", "ldRM10", "ldRM10", "ldRM10", "ldRM10", "ldRM10", "ldRM10",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "exMRW10", "exMRW10", "exMRW10", "exMRW10", "exMRW10", "exMRW10", "exMRW10", "exMRW10",
        "addwMI10", "adcwMI10", "subwMI10", "sbcwMI10", "andwMI10", "xorwMI10", "orwMI10", "cpwMI10",
        //
        "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10",
        "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10",
        "divRMW10", "divRMW10", "divRMW10", "divRMW10", "divRMW10", "divRMW10", "divRMW10", "divRMW10",
        "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10",
        "incw3M10", "incw3M10", "incw3M10", "incw3M10", "incw3M10", "incw3M10", "incw3M10", "incw3M10",
        "decw3M10", "decw3M10", "decw3M10", "decw3M10", "decw3M10", "decw3M10", "decw3M10", "decw3M10",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "rlcwM10", "rrcwM10", "rlwM10", "rrwM10", "slawM10", "srawM10", "sllwM10", "srlwM10",
        //
        "addRMW10", "addRMW10", "addRMW10", "addRMW10", "addRMW10", "addRMW10", "addRMW10", "addRMW10",
        "addMRW10", "addMRW10", "addMRW10", "addMRW10", "addMRW10", "addMRW10", "addMRW10", "addMRW10",
        "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10",
        "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10",
        "subRMW10", "subRMW10", "subRMW10", "subRMW10", "subRMW10", "subRMW10", "subRMW10", "subRMW10",
        "subMRW10", "subMRW10", "subMRW10", "subMRW10", "subMRW10", "subMRW10", "subMRW10", "subMRW10",
        "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10",
        "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10",
        //
        "andRMW10", "andRMW10", "andRMW10", "andRMW10", "andRMW10", "andRMW10", "andRMW10", "andRMW10",
        "andMRW10", "andMRW10", "andMRW10", "andMRW10", "andMRW10", "andMRW10", "andMRW10", "andMRW10",
        "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10",
        "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10",
        "orRMW10", "orRMW10", "orRMW10", "orRMW10", "orRMW10", "orRMW10", "orRMW10", "orRMW10",
        "orMRW10", "orMRW10", "orMRW10", "orMRW10", "orMRW10", "orMRW10", "orMRW10", "orMRW10",
        "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10",
        "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10"
    };

char *instr_tableA0[256] =
    {
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldRM20", "ldRM20", "ldRM20", "ldRM20", "ldRM20", "ldRM20", "ldRM20", "ldRM20",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef ", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "addRML20", "addRML20", "addRML20", "addRML20", "addRML20", "addRML20", "addRML20", "addRML20",
        "addMRL20", "addMRL20", "addMRL20", "addMRL20", "addMRL20", "addMRL20", "addMRL20", "addMRL20",
        "adcRML20", "adcRML20", "adcRML20", "adcRML20", "adcRML20", "adcRML20", "adcRML20", "adcRML20",
        "adcMRL20", "adcMRL20", "adcMRL20", "adcMRL20", "adcMRL20", "adcMRL20", "adcMRL20", "adcMRL20",
        "subRML20", "subRML20", "subRML20", "subRML20", "subRML20", "subRML20", "subRML20", "subRML20",
        "subMRL20", "subMRL20", "subMRL20", "subMRL20", "subMRL20", "subMRL20", "subMRL20", "subMRL20",
        "sbcRML20", "sbcRML20", "sbcRML20", "sbcRML20", "sbcRML20", "sbcRML20", "sbcRML20", "sbcRML20",
        "sbcMRL20", "sbcMRL20", "sbcMRL20", "sbcMRL20", "sbcMRL20", "sbcMRL20", "sbcMRL20", "sbcMRL20",
        //
        "andRML20", "andRML20", "andRML20", "andRML20", "andRML20", "andRML20", "andRML20", "andRML20",
        "andMRL20", "andMRL20", "andMRL20", "andMRL20", "andMRL20", "andMRL20", "andMRL20", "andMRL20",
        "xorRML20", "xorRML20", "xorRML20", "xorRML20", "xorRML20", "xorRML20", "xorRML20", "xorRML20",
        "xorMRL20", "xorMRL20", "xorMRL20", "xorMRL20", "xorMRL20", "xorMRL20", "xorMRL20", "xorMRL20",
        "orRML20", "orRML20", "orRML20", "orRML20", "orRML20", "orRML20", "orRML20", "orRML20",
        "orMRL20", "orMRL20", "orMRL20", "orMRL20", "orMRL20", "orMRL20", "orMRL20", "orMRL20",
        "cpRML20", "cpRML20", "cpRML20", "cpRML20", "cpRML20", "cpRML20", "cpRML20", "cpRML20",
        "cpMRL20", "cpMRL20", "cpMRL20", "cpMRL20", "cpMRL20", "cpMRL20", "cpMRL20", "cpMRL20"
    };

char *instr_tableB0[256] =
    {
        "ldMI30", "udef", "ldwMI30", "udef", "popM30", "udef", "popwM30", "udef",
        "udef", "udef", "udef", "udef", "udef ", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "ldM1630", "udef", "ldwM1630", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30",
        "andcfAM30", "orcfAM30", "xorcfAM30", "ldcfAM30", "stcfAM30", "udef", "udef", "udef",
        "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W",
        "udef", "udef", "udef ", "udef", "udef", "udef", "udef", "udef",
        "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef ", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30",
        "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30",
        "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30",
        "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30",
        "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30",
        "tset3M30", "tset3M30", "tset3M30", "tset3M30", "tset3M30", "tset3M30", "tset3M30", "tset3M30",
        "res3M30", "res3M30", "res3M30", "res3M30", "res3M30", "res3M30", "res3M30", "res3M30",
        "set3M30", "set3M30", "set3M30", "set3M30", "set3M30", "set3M30", "set3M30", "set3M30",
        //
        "chg3M30", "chg3M30", "chg3M30", "chg3M30", "chg3M30", "chg3M30", "chg3M30", "chg3M30",
        "bit3M30", "bit3M30", "bit3M30", "bit3M30", "bit3M30", "bit3M30", "bit3M30", "bit3M30",
        "jpccM300", "jpccM301", "jpccM302", "jpccM303", "jpccM304", "jpccM305", "jpccM306", "jpccM307",
        "jpccM308", "jpccM309", "jpccM30A", "jpccM30B", "jpccM30C", "jpccM30D", "jpccM30E", "jpccM30F",
        "callccM300", "callccM301", "callccM302", "callccM303", "callccM304", "callccM305", "callccM306", "callccM307",
        "callccM308", "callccM309", "callccM30A", "callccM30B", "callccM30C", "callccM30D", "callccM30E", "callccM30F",
        "retcc0", "retcc1", "retcc2", "retcc3", "retcc4", "retcc5", "retcc6", "retcc7",
        "retcc8", "retcc9", "retccA", "retccB", "retccC", "retccD", "retccE", "retccF"
    };

char *instr_tableB8[256] =
    {
        "ldMI30", "udef", "ldwMI30", "udef", "popM30", "udef", "popwM30", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "ldM1630", "udef", "ldwM1630", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30",
        "andcfAM30", "orcfAM30", "xorcfAM30", "ldcfAM30", "stcfAM30", "udef", "udef", "udef",
        "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30",
        "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30",
        "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30",
        "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30",
        "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30",
        "tset3M30", "tset3M30", "tset3M30", "tset3M30", "tset3M30", "tset3M30", "tset3M30", "tset3M30",
        "res3M30", "res3M30", "res3M30", "res3M30", "res3M30", "res3M30", "res3M30", "res3M30",
        "set3M30", "set3M30", "set3M30", "set3M30", "set3M30", "set3M30", "set3M30", "set3M30",
        //
        "chg3M30", "chg3M30", "chg3M30", "chg3M30", "chg3M30", "chg3M30", "chg3M30", "chg3M30",
        "bit3M30", "bit3M30", "bit3M30", "bit3M30", "bit3M30", "bit3M30", "bit3M30", "bit3M30",
        "jpccM300", "jpccM301", "jpccM302", "jpccM303", "jpccM304", "jpccM305", "jpccM306", "jpccM307",
        "jpccM308", "jpccM309", "jpccM30A", "jpccM30B", "jpccM30C", "jpccM30D", "jpccM30E", "jpccM30F",
        "callccM300", "callccM301", "callccM302", "callccM303", "callccM304", "callccM305", "callccM306", "callccM307",
        "callccM308", "callccM309", "callccM30A", "callccM30B", "callccM30C", "callccM30D", "callccM30E", "callccM30F",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef"
    };

char *instr_tableC0[256] =
    {
        "udef", "udef", "udef", "udef", "pushM00", "udef", "rld00", "rrd00",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "ld16M00", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldRM00", "ldRM00", "ldRM00", "ldRM00", "ldRM00", "ldRM00", "ldRM00", "ldRM00",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "exMRB00", "exMRB00", "exMRB00", "exMRB00", "exMRB00", "exMRB00", "exMRB00", "exMRB00",
        "addMI00", "adcMI00", "subMI00", "sbcMI00", "andMI00", "xorMI00", "orMI00", "cpMI00",
        //
        "mulRMB00", "mulRMB00", "mulRMB00", "mulRMB00", "mulRMB00", "mulRMB00", "mulRMB00", "mulRMB00",
        "mulsRMB00", "mulsRMB00", "mulsRMB00", "mulsRMB00", "mulsRMB00", "mulsRMB00", "mulsRMB00", "mulsRMB00",
        "divRMB00", "divRMB00", "divRMB00", "divRMB00", "divRMB00", "divRMB00", "divRMB00", "divRMB00",
        "divsRMB00", "divsRMB00", "divsRMB00", "divsRMB00", "divsRMB00", "divsRMB00", "divsRMB00", "divsRMB00",
        "inc3M00", "inc3M00", "inc3M00", "inc3M00", "inc3M00", "inc3M00", "inc3M00", "inc3M00",
        "dec3M00", "dec3M00", "dec3M00", "dec3M00", "dec3M00", "dec3M00", "dec3M00", "dec3M00",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "rlcM00", "rrcM00", "rlM00", "rrM00", "slaM00", "sraM00", "sllM00", "srlM00",
        //
        "addRMB00", "addRMB00", "addRMB00", "addRMB00", "addRMB00", "addRMB00", "addRMB00", "addRMB00",
        "addMRB00", "addMRB00", "addMRB00", "addMRB00", "addMRB00", "addMRB00", "addMRB00", "addMRB00",
        "adcRMB00", "adcRMB00", "adcRMB00", "adcRMB00", "adcRMB00", "adcRMB00", "adcRMB00", "adcRMB00",
        "adcMRB00", "adcMRB00", "adcMRB00", "adcMRB00", "adcMRB00", "adcMRB00", "adcMRB00", "adcMRB00",
        "subRMB00", "subRMB00", "subRMB00", "subRMB00", "subRMB00", "subRMB00", "subRMB00", "subRMB00",
        "subMRB00", "subMRB00", "subMRB00", "subMRB00", "subMRB00", "subMRB00", "subMRB00", "subMRB00",
        "sbcRMB00", "sbcRMB00", "sbcRMB00", "sbcRMB00", "sbcRMB00", "sbcRMB00", "sbcRMB00", "sbcRMB00",
        "sbcMRB00", "sbcMRB00", "sbcMRB00", "sbcMRB00", "sbcMRB00", "sbcMRB00", "sbcMRB00", "sbcMRB00",
        //
        "andRMB00", "andRMB00", "andRMB00", "andRMB00", "andRMB00", "andRMB00", "andRMB00", "andRMB00",
        "andMRB00", "andMRB00", "andMRB00", "andMRB00", "andMRB00", "andMRB00", "andMRB00", "andMRB00",
        "xorRMB00", "xorRMB00", "xorRMB00", "xorRMB00", "xorRMB00", "xorRMB00", "xorRMB00", "xorRMB00",
        "xorMRB00", "xorMRB00", "xorMRB00", "xorMRB00", "xorMRB00", "xorMRB00", "xorMRB00", "xorMRB00",
        "orRMB00", "orRMB00", "orRMB00", "orRMB00", "orRMB00", "orRMB00", "orRMB00", "orRMB00",
        "orMRB00", "orMRB00", "orMRB00", "orMRB00", "orMRB00", "orMRB00", "orMRB00", "orMRB00",
        "cpRMB00", "cpRMB00", "cpRMB00", "cpRMB00", "cpRMB00", "cpRMB00", "cpRMB00", "cpRMB00",
        "cpMRB00", "cpMRB00", "cpMRB00", "cpMRB00", "cpMRB00", "cpMRB00", "cpMRB00", "cpMRB00"
    };

char *instr_tableC8[256] =
    {
        "udef", "udef", "udef", "ldrIB", "pushrB", "poprB", "cplrB", "negrB",
        "mulrIB", "mulsrIB", "divrIB", "divsrIB", "udef", "udef", "udef", "udef",
        "daar", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "bios", "udef", "djnzB", "udef", "udef", "udef",
        "andcf4rB", "orcf4rB", "xorcf4rB", "ldcf4rB", "stcf4rB", "udef", "udef", "udef",
        "andcfArB", "orcfArB", "xorcfArB", "ldcfArB", "stcfArB", "udef", "ldccrB", "ldcrcB",
        "res4rB", "set4rB", "chg4rB", "bit4rB", "tset4rB", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "mulRrB", "mulRrB", "mulRrB", "mulRrB", "mulRrB", "mulRrB", "mulRrB", "mulRrB",
        "mulsRrB", "mulsRrB", "mulsRrB", "mulsRrB", "mulsRrB", "mulsRrB", "mulsRrB", "mulsRrB",
        "divRrB", "divRrB", "divRrB", "divRrB", "divRrB", "divRrB", "divRrB", "divRrB",
        "divsRrB", "divsRrB", "divsRrB", "divsRrB", "divsRrB", "divsRrB", "divsRrB", "divsRrB",
        "inc3rB", "inc3rB", "inc3rB", "inc3rB", "inc3rB", "inc3rB", "inc3rB", "inc3rB",
        "dec3rB", "dec3rB", "dec3rB", "dec3rB", "dec3rB", "dec3rB", "dec3rB", "dec3rB",
        "sccB0", "sccB1", "sccB2", "sccB3", "sccB4", "sccB5", "sccB6", "sccB7",
        "sccB8", "sccB9", "sccBA", "sccBB", "sccBC", "sccBD", "sccBE", "sccBF",
        //
        "addRrB", "addRrB", "addRrB", "addRrB", "addRrB", "addRrB", "addRrB", "addRrB",
        "ldRrB", "ldRrB", "ldRrB", "ldRrB", "ldRrB", "ldRrB", "ldRrB", "ldRrB",
        "adcRrB", "adcRrB", "adcRrB", "adcRrB", "adcRrB", "adcRrB", "adcRrB", "adcRrB",
        "ldrRB", "ldrRB", "ldrRB", "ldrRB", "ldrRB", "ldrRB", "ldrRB", "ldrRB",
        "subRrB", "subRrB", "subRrB", "subRrB", "subRrB", "subRrB", "subRrB", "subRrB",
        "ldr3B", "ldr3B", "ldr3B", "ldr3B", "ldr3B", "ldr3B", "ldr3B", "ldr3B",
        "sbcRrB", "sbcRrB", "sbcRrB", "sbcRrB", "sbcRrB", "sbcRrB", "sbcRrB", "sbcRrB",
        "exRrB", "exRrB", "exRrB", "exRrB", "exRrB", "exRrB", "exRrB", "exRrB",
        //
        "andRrB", "andRrB", "andRrB", "andRrB", "andRrB", "andRrB", "andRrB", "andRrB",
        "addrIB", "adcrIB", "subrIB", "sbcrIB", "andrIB", "xorrIB", "orrIB", "cprIB",
        "xorRrB", "xorRrB", "xorRrB", "xorRrB", "xorRrB", "xorRrB", "xorRrB", "xorRrB",
        "cpr3B", "cpr3B", "cpr3B", "cpr3B", "cpr3B", "cpr3B", "cpr3B", "cpr3B",
        "orRrB", "orRrB", "orRrB", "orRrB", "orRrB", "orRrB", "orRrB", "orRrB",
        "rlc4rB", "rrc4rB", "rl4rB", "rr4rB", "sla4rB", "sra4rB", "sll4rB", "srl4rB",
        "cpRrB", "cpRrB", "cpRrB", "cpRrB", "cpRrB", "cpRrB", "cpRrB", "cpRrB",
        "rlcArB", "rrcArB", "rlArB", "rrArB", "slaArB", "sraArB", "sllArB", "srlArB"
    };

char *instr_tableD0[256] =
    {
        "udef", "udef", "udef", "udef", "pushwM10", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "ldw16M10", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldRM10", "ldRM10", "ldRM10", "ldRM10", "ldRM10", "ldRM10", "ldRM10", "ldRM10",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "exMRW10", "exMRW10", "exMRW10", "exMRW10", "exMRW10", "exMRW10", "exMRW10", "exMRW10",
        "addwMI10", "adcwMI10", "subwMI10", "sbcwMI10", "andwMI10", "xorwMI10", "orwMI10", "cpwMI10",
        //
        "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10", "mulRMW10",
        "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10", "mulsRMW10",
        "divRMW10", "divRMW10", "divRMW10", "divRMW10", "divRMW10", "divRMW10", "divRMW10", "divRMW10",
        "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10", "divsRMW10",
        "incw3M10", "incw3M10", "incw3M10", "incw3M10", "incw3M10", "incw3M10", "incw3M10", "incw3M10",
        "decw3M10", "decw3M10", "decw3M10", "decw3M10", "decw3M10", "decw3M10", "decw3M10", "decw3M10",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "rlcwM10", "rrcwM10", "rlwM10", "rrwM10", "slawM10", "srawM10", "sllwM10", "srlwM10",
        //
        "addRMW10", "addRMW10", "addRMW10", "addRMW10", "addRMW10", "addRMW10", "addRMW10", "addRMW10",
        "addMRW10", "addMRW10", "addMRW10", "addMRW10", "addMRW10", "addMRW10", "addMRW10", "addMRW10",
        "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10", "adcRMW10",
        "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10", "adcMRW10",
        "subRMW10", "subRMW10", "subRMW10", "subRMW10", "subRMW10", "subRMW10", "subRMW10", "subRMW10",
        "subMRW10", "subMRW10", "subMRW10", "subMRW10", "subMRW10", "subMRW10", "subMRW10", "subMRW10",
        "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10", "sbcRMW10",
        "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10", "sbcMRW10",
        //
        "andRMW10", "andRMW10", "andRMW10", "andRMW10", "andRMW10", "andRMW10", "andRMW10", "andRMW10",
        "andMRW10", "andMRW10", "andMRW10", "andMRW10", "andMRW10", "andMRW10", "andMRW10", "andMRW10",
        "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10", "xorRMW10",
        "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10", "xorMRW10",
        "orRMW10", "orRMW10", "orRMW10", "orRMW10", "orRMW10", "orRMW10", "orRMW10", "orRMW10",
        "orMRW10", "orMRW10", "orMRW10", "orMRW10", "orMRW10", "orMRW10", "orMRW10", "orMRW10",
        "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10", "cpRMW10",
        "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10", "cpMRW10"
    };

char *instr_tableD8[256] =
    {
        "udef", "udef", "udef", "ldrIW", "pushrW", "poprW", "cplrW", "negrW",
        "mulrIW", "mulsrIW", "divrIW", "divsrIW", "udef", "udef", "bs1f", "bs1b",
        "udef", "udef", "extzrW", "extsrW", "paarW", "udef", "mirrr", "udef",
        "udef", "mular", "udef", "udef", "djnzW", "udef", "udef", "udef",
        "andcf4rW", "orcf4rW", "xorcf4rW", "ldcf4rW", "stcf4rW", "udef", "udef", "udef",
        "andcfArW", "orcfArW", "xorcfArW", "ldcfArW", "stcfArW", "udef", "ldccrW", "ldcrcW",
        "res4rW", "set4rW", "chg4rW", "bit4rW", "tset4rW", "udef", "udef", "udef",
        "minc1", "minc2", "minc4", "udef", "mdec1", "mdec2", "mdec4", "udef",
        //
        "mulRrW", "mulRrW", "mulRrW", "mulRrW", "mulRrW", "mulRrW", "mulRrW", "mulRrW",
        "mulsRrW", "mulsRrW", "mulsRrW", "mulsRrW", "mulsRrW", "mulsRrW", "mulsRrW", "mulsRrW",
        "divRrW", "divRrW", "divRrW", "divRrW", "divRrW", "divRrW", "divRrW", "divRrW",
        "divsRrW", "divsRrW", "divsRrW", "divsRrW", "divsRrW", "divsRrW", "divsRrW", "divsRrW",
        "inc3rW", "inc3rW", "inc3rW", "inc3rW", "inc3rW", "inc3rW", "inc3rW", "inc3rW",
        "dec3rW", "dec3rW", "dec3rW", "dec3rW", "dec3rW", "dec3rW", "dec3rW", "dec3rW",
        "sccW0", "sccW1", "sccW2", "sccW3", "sccW4", "sccW5", "sccW6", "sccW7",
        "sccW8", "sccW9", "sccWA", "sccWB", "sccWC", "sccWD", "sccWE", "sccWF",
        //
        "addRrW", "addRrW", "addRrW", "addRrW", "addRrW", "addRrW", "addRrW", "addRrW",
        "ldRrW", "ldRrW", "ldRrW", "ldRrW", "ldRrW", "ldRrW", "ldRrW", "ldRrW",
        "adcRrW", "adcRrW", "adcRrW", "adcRrW", "adcRrW", "adcRrW", "adcRrW", "adcRrW",
        "ldrRW", "ldrRW", "ldrRW", "ldrRW", "ldrRW", "ldrRW", "ldrRW", "ldrRW",
        "subRrW", "subRrW", "subRrW", "subRrW", "subRrW", "subRrW", "subRrW", "subRrW",
        "ldr3W", "ldr3W", "ldr3W", "ldr3W", "ldr3W", "ldr3W", "ldr3W", "ldr3W",
        "sbcRrW", "sbcRrW", "sbcRrW", "sbcRrW", "sbcRrW", "sbcRrW", "sbcRrW", "sbcRrW",
        "exRrW", "exRrW", "exRrW", "exRrW", "exRrW", "exRrW", "exRrW", "exRrW",
        //
        "andRrW", "andRrW", "andRrW", "andRrW", "andRrW", "andRrW", "andRrW", "andRrW",
        "addrIW", "adcrIW", "subrIW", "sbcrIW", "andrIW", "xorrIW", "orrIW", "cprIW",
        "xorRrW", "xorRrW", "xorRrW", "xorRrW", "xorRrW", "xorRrW", "xorRrW", "xorRrW",
        "cpr3W", "cpr3W", "cpr3W", "cpr3W", "cpr3W", "cpr3W", "cpr3W", "cpr3W",
        "orRrW", "orRrW", "orRrW", "orRrW", "orRrW", "orRrW", "orRrW", "orRrW",
        "rlc4rW", "rrc4rW", "rl4rW", "rr4rW", "sla4rW", "sra4rW", "sll4rW", "srl4rW",
        "cpRrW", "cpRrW", "cpRrW", "cpRrW", "cpRrW", "cpRrW", "cpRrW", "cpRrW",
        "rlcArW", "rrcArW", "rlArW", "rrArW", "slaArW", "sraArW", "sllArW", "srlArW"
    };

char *instr_tableE0[256] =
    {
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldRM20", "ldRM20", "ldRM20", "ldRM20", "ldRM20", "ldRM20", "ldRM20", "ldRM20",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef ", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "addRML20", "addRML20", "addRML20", "addRML20", "addRML20", "addRML20", "addRML20", "addRML20",
        "addMRL20", "addMRL20", "addMRL20", "addMRL20", "addMRL20", "addMRL20", "addMRL20", "addMRL20",
        "adcRML20", "adcRML20", "adcRML20", "adcRML20", "adcRML20", "adcRML20", "adcRML20", "adcRML20",
        "adcMRL20", "adcMRL20", "adcMRL20", "adcMRL20", "adcMRL20", "adcMRL20", "adcMRL20", "adcMRL20",
        "subRML20", "subRML20", "subRML20", "subRML20", "subRML20", "subRML20", "subRML20", "subRML20",
        "subMRL20", "subMRL20", "subMRL20", "subMRL20", "subMRL20", "subMRL20", "subMRL20", "subMRL20",
        "sbcRML20", "sbcRML20", "sbcRML20", "sbcRML20", "sbcRML20", "sbcRML20", "sbcRML20", "sbcRML20",
        "sbcMRL20", "sbcMRL20", "sbcMRL20", "sbcMRL20", "sbcMRL20", "sbcMRL20", "sbcMRL20", "sbcMRL20",
        //
        "andRML20", "andRML20", "andRML20", "andRML20", "andRML20", "andRML20", "andRML20", "andRML20",
        "andMRL20", "andMRL20", "andMRL20", "andMRL20", "andMRL20", "andMRL20", "andMRL20", "andMRL20",
        "xorRML20", "xorRML20", "xorRML20", "xorRML20", "xorRML20", "xorRML20", "xorRML20", "xorRML20",
        "xorMRL20", "xorMRL20", "xorMRL20", "xorMRL20", "xorMRL20", "xorMRL20", "xorMRL20", "xorMRL20",
        "orRML20", "orRML20", "orRML20", "orRML20", "orRML20", "orRML20", "orRML20", "orRML20",
        "orMRL20", "orMRL20", "orMRL20", "orMRL20", "orMRL20", "orMRL20", "orMRL20", "orMRL20",
        "cpRML20", "cpRML20", "cpRML20", "cpRML20", "cpRML20", "cpRML20", "cpRML20", "cpRML20",
        "cpMRL20", "cpMRL20", "cpMRL20", "cpMRL20", "cpMRL20", "cpMRL20", "cpMRL20", "cpMRL20"
    };

char *instr_tableE8[256] =
    {
        "udef", "udef", "udef", "ldrIL", "pushrL", "poprL", "udef", "udef",
        "udef", "udef", "udef", "udef", "link", "unlk", "udef", "udef",
        "udef", "udef", "extzrL", "extsrL", "paarL", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "ldccrL", "ldcrcL",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "inc3rL", "inc3rL", "inc3rL", "inc3rL", "inc3rL", "inc3rL", "inc3rL", "inc3rL",
        "dec3rL", "dec3rL", "dec3rL", "dec3rL", "dec3rL", "dec3rL", "dec3rL", "dec3rL",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "addRrL", "addRrL", "addRrL", "addRrL", "addRrL", "addRrL", "addRrL", "addRrL",
        "ldRrL", "ldRrL", "ldRrL", "ldRrL", "ldRrL", "ldRrL", "ldRrL", "ldRrL",
        "adcRrL", "adcRrL", "adcRrL", "adcRrL", "adcRrL", "adcRrL", "adcRrL", "adcRrL",
        "ldrRL", "ldrRL", "ldrRL", "ldrRL", "ldrRL", "ldrRL", "ldrRL", "ldrRL",
        "subRrL", "subRrL", "subRrL", "subRrL", "subRrL", "subRrL", "subRrL", "subRrL",
        "ldr3L", "ldr3L", "ldr3L", "ldr3L", "ldr3L", "ldr3L", "ldr3L", "ldr3L",
        "sbcRrL", "sbcRrL", "sbcRrL", "sbcRrL", "sbcRrL", "sbcRrL", "sbcRrL", "sbcRrL",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "andRrL", "andRrL", "andRrL", "andRrL", "andRrL", "andRrL", "andRrL", "andRrL",
        "addrIL", "adcrIL", "subrIL", "sbcrIL", "andrIL", "xorrIL", "orrIL", "cprIL",
        "xorRrL", "xorRrL", "xorRrL", "xorRrL", "xorRrL", "xorRrL", "xorRrL", "xorRrL",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "orRrL", "orRrL", "orRrL", "orRrL", "orRrL", "orRrL", "orRrL", "orRrL",
        "rlc4rL", "rrc4rL", "rl4rL", "rr4rL", "sla4rL", "sra4rL", "sll4rL", "srl4rL",
        "cpRrL", "cpRrL", "cpRrL", "cpRrL", "cpRrL", "cpRrL", "cpRrL", "cpRrL",
        "rlcArL", "rrcArL", "rlArL", "rrArL", "slaArL", "sraArL", "sllArL", "srlArL"
    };

char *instr_tableF0[256] =
    {
        "ldMI30", "udef", "ldwMI30", "udef", "popM30", "udef", "popwM30", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "ldM1630", "udef", "ldwM1630", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30", "ldaRMW30",
        "andcfAM30", "orcfAM30", "xorcfAM30", "ldcfAM30", "stcfAM30", "udef", "udef", "udef",
        "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30", "ldaRML30",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B", "ldMR30B",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W", "ldMR30W",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L", "ldMR30L",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        //
        "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30", "andcf3M30",
        "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30", "orcf3M30",
        "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30", "xorcf3M30",
        "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30", "ldcf3M30",
        "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30", "stcf3M30",
        "tset3M30", "tset3M30", "tset3M30", "tset3M30", "tset3M30", "tset3M30", "tset3M30", "tset3M30",
        "res3M30", "res3M30", "res3M30", "res3M30", "res3M30", "res3M30", "res3M30", "res3M30",
        "set3M30", "set3M30", "set3M30", "set3M30", "set3M30", "set3M30", "set3M30", "set3M30",
        //
        "chg3M30", "chg3M30", "chg3M30", "chg3M30", "chg3M30", "chg3M30", "chg3M30", "chg3M30",
        "bit3M30", "bit3M30", "bit3M30", "bit3M30", "bit3M30", "bit3M30", "bit3M30", "bit3M30",
        "jpccM300", "jpccM301", "jpccM302", "jpccM303", "jpccM304", "jpccM305", "jpccM306", "jpccM307",
        "jpccM308", "jpccM309", "jpccM30A", "jpccM30B", "jpccM30C", "jpccM30D", "jpccM30E", "jpccM30F",
        "callccM300", "callccM301", "callccM302", "callccM303", "callccM304", "callccM305", "callccM306", "callccM307",
        "callccM308", "callccM309", "callccM30A", "callccM30B", "callccM30C", "callccM30D", "callccM30E", "callccM30F",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef",
        "udef", "udef", "udef", "udef", "udef", "udef", "udef", "udef"
    };

void initTlcs900hProfile()
{
    memset(profile, 0, 256 * sizeof(profStruct));
}

void printTlcs900hProfile()
{
    FILE *profileFile;
    profileFile = fopen("profile.csv", "wt");
    if(!profileFile)
    {
        fprintf(stderr, "Couldn't open %s file\n", "profile.csv");
        return;
    }

    fprintf(profileFile, "Opcode, Calls, Ticks, TicksPerCall, Name\n");
    for(int op=0; op < 256; op++)
    {
        fprintf(profileFile, "0x%02X, %9d, %9d, %f, %s\n",
                op, profile[op].calls, profile[op].ticks, profile[op].calls?((double)profile[op].ticks)/((double)profile[op].calls):0, instr_names[op]);
    }

    unsigned char decodes[13] = {0x80,0x90,0x98,0xa0,0xb0,0xb8,0xc0,0xc8,0xd0,0xd8,0xe0,0xe8,0xf0};
    char** decodestr[13] = {instr_table80,instr_table90,instr_table98,instr_tableA0,
                            instr_tableB0,instr_tableB8,instr_tableC0,instr_tableC8,
                            instr_tableD0,instr_tableD8,instr_tableE0,instr_tableE8,
                            instr_tableF0};
    fprintf(profileFile, "\nDecodes\n");
    for(int d=0; d < 13; d++)
    {
        fprintf(profileFile, "\nDecode 0x%02X\n",decodes[d]);
        for(int sub=0; sub < 256; sub++)
            if (profile[decodes[d]].decode[sub])
            {
                fprintf(profileFile, "0x%02X, %s, %9d\n",sub, decodestr[d][sub], profile[decodes[d]].decode[sub]);
            }
    }

    fclose(profileFile);
}
#endif

// declare all registers
// XWA0, XBC0, XDE0, XHL0,    0,1,2,3
// XWA1, XBC1, XDE1, XHL1,    4,5,6,7
// XWA2, XBC2, XDE2, XHL2,    8,9,10,11
// XWA3, XBC3, XDE3, XHL3,    12,13,14,15
// XIX,  XIY,  XIZ,  XSP      16,17,18,19 XNSP = Normal Stack Pointer
// PC,   SR,   XSSP, XNSP     20,21,22,23 XSSP = System Stack Pointer, XSP = current stack pointer
//unsigned long gen_regs[24];
unsigned long gen_regsXWA0, gen_regsXBC0, gen_regsXDE0, gen_regsXHL0, gen_regsXWA1,
gen_regsXBC1, gen_regsXDE1, gen_regsXHL1, gen_regsXWA2, gen_regsXBC2,
gen_regsXDE2, gen_regsXHL2, gen_regsXWA3, gen_regsXBC3, gen_regsXDE3,
gen_regsXHL3, gen_regsXIX, gen_regsXIY, gen_regsXIZ, gen_regsXSP,
gen_regsSP, gen_regsXSSP, gen_regsXNSP;

#ifdef TARGET_GP2X
//it's defined in StdAfx.h
#ifndef GENREGSPC_AS_REG
unsigned long gen_regsPC;
#endif
#ifndef GENREGSSR_AS_REG
unsigned long gen_regsSR;
#endif
#else
unsigned long gen_regsPC, gen_regsSR;
#endif

// declare ldc registers
unsigned char __attribute__ ((__aligned__(4))) ldcRegs[64];

// declare struct for easy access to flags of flag register
//struct SR0 {
// unsigned long C0:1;
// unsigned long N0:1;
// unsigned long V0:1;
// unsigned long dummy0:1;
// unsigned long H0:1;
// unsigned long dummy1:1;
// unsigned long Z0:1;
// unsigned long S0:1;
// unsigned long RFP0:3;
// unsigned long MAXM0:1;
// unsigned long IFF0:3;
// unsigned long SYSM0:1;
//};
// lower byte of SR: F
//#define C ((*(struct SR0 *)(&gen_regsSR)).C0)
//#define N ((*(struct SR0 *)(&gen_regsSR)).N0)
//#define V ((*(struct SR0 *)(&gen_regsSR)).V0)
//#define H ((*(struct SR0 *)(&gen_regsSR)).H0)
//#define Z ((*(struct SR0 *)(&gen_regsSR)).Z0)
//#define S ((*(struct SR0 *)(&gen_regsSR)).S0)
#define CF 0x01
#define NF 0x02
#define VF 0x04
#define HF 0x10
#define ZF 0x40
#define SF 0x80
// upper byte of SR
//#define RFP  ((*(struct SR0 *)(&gen_regsSR)).RFP0)
//#define MAXM ((*(struct SR0 *)(&gen_regsSR)).MAXM0)
//#define IFF  ((*(struct SR0 *)(&gen_regsSR)).IFF0)
//#define SYSM ((*(struct SR0 *)(&gen_regsSR)).SYSM0)
// definition for F'
unsigned char F2;

#ifdef TARGET_GP2X
//it's defined in StdAfx.h
#else
unsigned char *my_pc = NULL;
#endif

//unsigned char *saved_my_pc;


// pointers to all register(parts) that could be accessed in byte mode
unsigned char *allregsB[256];
unsigned char *cregsB[8];
// pointers to all register(parts) that could be accessed in word mode
unsigned short *allregsW[256];
unsigned short *cregsW[8];
// pointers to all register(parts) that could be accessed in long mode
unsigned long *allregsL[256];
unsigned long *cregsL[8];

// number of states passed and the number of states until a new interrupt should occur
// 1 state = 100ns at 20 MHz
// 1 state =  80ns at 25 MHz
int state;
int checkstate;
int DMAstate;
// Clock multiplier to reflect the CPU speed
// 1 - 6144 kHz
// 2 - 3072 kHz
// 4 - 1536 kHz
// 8 -  768 kHz
//16 -  384 kHz
int tlcsClockMulti;
//
// is there an interrupt we need to honor?
#define INT_QUEUE_MAX 4
unsigned char interruptPendingLevel;
unsigned char pendingInterrupts[7][INT_QUEUE_MAX];

// used during decoding of instructions, will hold the arguments for the operands
unsigned long mem;
unsigned char *regB, memB;
unsigned short *regW, memW;
unsigned long *regL, memL;

// used during instruction decode
// lastbyte - to keep track of the last read byte; used when extra information is stored in the
//            opcode itself.
#ifndef TARGET_GP2X
unsigned char opcode;
#endif
//unsigned char opcode1, opcode2;

unsigned char lastbyte;

// for debugging
int  debugIndex, debugCount;

// wrapper
int  memoryCycles;


inline unsigned char mem_readB(unsigned long addr)
{
    if (addr > 0x200000)
        ++memoryCycles;
    return tlcsMemReadB(addr);
}

inline unsigned short mem_readW(unsigned long addr)
{
    if (addr > 0x200000)
        memoryCycles+=2;
    return tlcsMemReadW(addr);
}

inline unsigned long mem_readL(unsigned long addr)
{
    if (addr > 0x200000)
        memoryCycles+=4;
    return tlcsMemReadL(addr);
}

inline void mem_writeB(unsigned long addr, unsigned char data)
{
    // if (addr > 0x200000) memoryCycles++;
    tlcsMemWriteB(addr, data);
}

inline void tlcsFastMemWriteB(unsigned long addr,unsigned char data)
{
    //Flavor:  I think we're getting carried away with this addr&0xFFFFFF junk.  It's all over, now.  :(
	mainram[((addr&0xFFFFFF)-0x00004000)] = data;
}

inline void tlcsFastMemWriteW(unsigned long addr, unsigned short data)
{
    //Flavor:  I think we're getting carried away with this addr&0xFFFFFF junk.  It's all over, now.  :(
    unsigned char*ram = mainram+((addr&0xFFFFFF)-0x00004000);
    if (((unsigned long)ram)&0x1)
    {
        *(ram++) = data;
        *(ram) = data>>8;
    }
    else
        *((unsigned short*)ram) = data;
}

inline void tlcsFastMemWriteL(unsigned long addr, unsigned long data)
{
    //Flavor:  I think we're getting carried away with this addr&0xFFFFFF junk.  It's all over, now.  :(
    unsigned char*ram = mainram+((addr&0xFFFFFF)-0x00004000);
    if (((unsigned long)ram)&0x3)
    {
        *(ram++) = data;
        *(ram++) = data>>8;
        *(ram++) = data>>16;
        *ram     = data>>24;
    }
    else
        *((unsigned long*)ram) = data;
}


inline void tlcsMemWriteBaddrB(unsigned char addr, unsigned char data)
{
    switch(addr) {
    //case 0x80:	// CPU speed
    //    break;
    case 0xA0:	// L CH Sound Source Control Register
        if (cpuram[0xB8] == 0x55 && cpuram[0xB9] == 0xAA)
            Write_SoundChipNoise(data);//Flavor SN76496Write(0, data);
        break;
    case 0xA1:	// R CH Sound Source Control Register
        if (cpuram[0xB8] == 0x55 && cpuram[0xB9] == 0xAA)
            Write_SoundChipTone(data); //Flavor SN76496Write(0, data);
        break;
    case 0xA2:	// L CH DAC Control Register
        ngpSoundExecute();
        if (cpuram[0xB8] == 0xAA)
            dac_writeL(data); //Flavor DAC_data_w(0,data);
        break;
    /*case 0xA3:	// R CH DAC Control Register  //Flavor hack for mono only sound
        ngpSoundExecute();
        if (cpuram[0xB8] == 0xAA)
            dac_writeR(data);//Flavor DAC_data_w(1,data);
        break;*/
    case 0xB8:	// Z80 Reset
//				if (data == 0x55)	DAC_data_w(0,0);
    case 0xB9:	// Sourd Source Reset Control Register
        switch(data) {
        case 0x55:
            ngpSoundStart();
            break;
        case 0xAA:
            ngpSoundExecute();
            ngpSoundOff();
            break;
        }
        break;
    case 0xBA:
        ngpSoundExecute();
#ifdef DRZ80
        Z80_Cause_Interrupt(Z80_NMI_INT);
#else
        z80Interrupt(Z80NMI);
#endif
        break;
    }
    cpuram[addr] = data;
    return;
}


// write a word (data) to a memory address (addr)
inline void tlcsMemWriteW(unsigned long addr, unsigned short data)
{
    //Flavor:  I think we're getting carried away with this addr&0xFFFFFF junk.  It's all over, now.  :(
    if ((addr&0xFFFFFF)>0x00003fff && (addr&0xFFFFFF)<0x00018000)
    {
        tlcsFastMemWriteW(addr, data);
    }
    else
    {
        tlcsMemWriteB(addr, data & 0xFF);
        tlcsMemWriteB(addr+1, data >> 8);
    }
}

// write a word (data) to a memory address (addr)
inline void tlcsMemWriteWaddrB(unsigned long addr, unsigned short data)
{
    tlcsMemWriteBaddrB(addr, data & 0xFF);
    tlcsMemWriteBaddrB(addr+1, data >> 8);
}


#define mem_writeW tlcsMemWriteW
/*inline void mem_writeW(unsigned long addr, unsigned short data) {
// if (addr > 0x200000) memoryCycles+= 2;
 tlcsMemWriteW(addr, data);
}*/

// write a long word (data) to a memory address (addr)
inline void tlcsMemWriteL(unsigned long addr, unsigned long data)
{
    //Flavor:  I think we're getting carried away with this addr&0xFFFFFF junk.  It's all over, now.  :(
    addr&=0xFFFFFF;
    if (addr>0x00003fff && addr<0x00018000)
    {
        tlcsFastMemWriteL(addr, data);
    }
    else
    {
        tlcsMemWriteB(addr, (unsigned char)(data & 0xFF));
        tlcsFastMemWriteB(addr+1, (unsigned char)((data>>8) & 0xFF));
        tlcsFastMemWriteB(addr+2, (unsigned char)((data>>16) & 0xFF));
        tlcsFastMemWriteB(addr+3, (unsigned char)((data>>24) & 0xFF));
    }
}

#define mem_writeL tlcsMemWriteL
/*inline void mem_writeL(unsigned long addr, unsigned long data) {
// if (addr > 0x200000) memoryCycles+= 4;
 tlcsMemWriteL(addr, data);
}*/


inline unsigned char readbyte()
{
#ifdef TARGET_GP2X
    unsigned char __val asm("r0");//%0 and r0 are the same, now
#ifdef GENREGSPC_AS_REG

    asm volatile(
        "ldrb %0, [%2], #1\n\t"
        "add    %1, %1, #1"
        : "=r" (__val)
        : "r" (gen_regsPC), "r" (my_pc)
        : );
#else

    asm volatile(
        "ldr r3, %1\n\t"
        "ldrb %0, [%2], #1\n\t"
        "add r3, r3, #1\n\t"
        "str r3, %1"
    : "=r" (__val)
                : "m" (gen_regsPC), "r" (my_pc)
                : "r3");
#endif

    return __val;
#else

    ++gen_regsPC;
    return *(my_pc++);
#endif
}

inline unsigned char readbyteSetLastbyte()
{
#ifdef TARGET_GP2X
    unsigned char __val asm("r0");//%0 and r0 are the same, now
#ifdef GENREGSPC_AS_REG

    asm volatile(
        "ldrb %0, [%3], #1\n\t"
        "ldrb %1, [%3], #1\n\t"
        "add %2, %2, #2\n\t"
    : "=r" (__val), "=r" (lastbyte)
                : "r" (gen_regsPC), "r" (my_pc)
                : );
#else

    asm volatile(
        "ldr r3, %2\n\t"
        "ldrb %0, [%3], #1\n\t"
        "ldrb %1, [%3], #1\n\t"
        "add r3, r3, #2\n\t"
        "str r3, %2"
    : "=r" (__val), "=r" (lastbyte)
                : "m" (gen_regsPC), "r" (my_pc)
                : "r3");
#endif

    return __val;
#else

    register unsigned char i;
    register unsigned short j;

    gen_regsPC+= 2;
    if((unsigned long)my_pc & 0x01)   //not word aligned
    {
        i=*(my_pc++);
        lastbyte = *(my_pc++);
        return i;
    }
    else
    {
        j = *((unsigned short *)my_pc);
        lastbyte = j>>8;
        my_pc+=2;
        //return (j & 0xFF);
        return j;
    }
    return i;
#endif
}

inline unsigned short readword()
{
#ifdef TARGET_GP2X
    unsigned short __val asm("r0");//%0 and r0 are the same, now
#ifdef GENREGSPC_AS_REG

    asm volatile(
        "ldrb %0, [%2], #1\n\t"
        "ldrb r2, [%2], #1\n\t"
        "add %1, %1, #2\n\t"
        "orr %0, %0, r2, asl #8"
    : "=r" (__val)
                : "r" (gen_regsPC), "r" (my_pc)
                : "r2");
#else

    asm volatile(
        "ldr r3, %1\n\t"
        "ldrb %0, [%2], #1\n\t"
        "ldrb r2, [%2], #1\n\t"
        "add r3, r3, #2\n\t"
        "str r3, %1\n\t"
        "orr %0, %0, r2, asl #8"
    : "=r" (__val)
                : "m" (gen_regsPC), "r" (my_pc)
                : "r2","r3");
#endif

    return __val;
#else

    register unsigned short i;

    gen_regsPC+= 2;

    if((unsigned long)my_pc & 0x01)   //not word aligned
    {
        i = *(my_pc++);
        i |= (*(my_pc++) << 8);
        return i;
    }
    else
    {
        i = *((unsigned short *)my_pc);
        my_pc+=2;
        return i;
    }

    return i;
#endif
}

inline unsigned short readwordSetLastbyte()
{
#ifdef TARGET_GP2X
    unsigned short __val asm("r0");//%0 and r0 are the same, now
#ifdef GENREGSPC_AS_REG

    asm volatile(
        "ldrb %0, [%3], #1\n\t"
        "ldrb r2, [%3], #1\n\t"
        "ldrb %1, [%3], #1\n\t"
        "add %2, %2, #3\n\t"
        "orr %0, %0, r2, asl #8"
    : "=r" (__val), "=r" (lastbyte)
                : "r" (gen_regsPC), "r" (my_pc)
                : "r2");
#else

    asm volatile(
        "ldr r3, %2\n\t"
        "ldrb %0, [%3], #1\n\t"
        "ldrb r2, [%3], #1\n\t"
        "ldrb %1, [%3], #1\n\t"
        "add r3, r3, #3\n\t"
        "str r3, %2\n\t"
        "orr %0, %0, r2, asl #8"
    : "=r" (__val), "=r" (lastbyte)
                : "m" (gen_regsPC), "r" (my_pc)
                : "r2","r3");
#endif

    return __val;

#else

    register unsigned short i;
    register unsigned long j;

    gen_regsPC+= 3;
    if((unsigned long)my_pc & 0x03)   //not dword aligned
    {
        i = *(my_pc++);
        i |= (*(my_pc++) << 8);
        lastbyte = *(my_pc++);

        return i;
    }
    else
    {
        j = *((unsigned long *)my_pc);

        lastbyte = j>>16;//let "them" do the &0xFF

        my_pc+=3;
        return j;//let "them" do the &0xFFFF
    }

    return i;
#endif
}

/*
read24 is a bit interesting, because we can test the LS bit of my_pc
if it's 0, we read a word and then a byte
if it's 1, we read a byte and then a word

Or, we could just read a dword and forget about the MSB
*/
inline unsigned long read24()
{
#ifdef TARGET_GP2X
    register unsigned long __val asm("r0");//%0 and r0 are the same, now

#ifdef GENREGSPC_AS_REG

    asm volatile(
        "add %2, %2, #3\n"
        "bic r1,%1,#3 \n"
        "ldmia r1,{r0,r3} \n"
        "ands r1,%1,#3 \n"
        "movne r2,r1,lsl #3 \n"
        "movne r0,r0,lsr r2 \n"
        "rsbne r1,r2,#32 \n"
        "orrne r0,r0,r3,lsl r1\n"
        "and    r0, r0, #0xFFFFFF\n"
        "add    %1, %1, #3"
    : "=r" (__val)
                : "r"(my_pc), "r"(gen_regsPC)
                : "r1","r2","r3");
#else

    asm volatile(
        "ldr r3, %2\n"
        "add r3, r3, #3\n"
        "str r3, %2\n"
        "bic r1,%1,#3 \n"
        "ldmia r1,{r0,r3} \n"
        "ands r1,%1,#3 \n"
        "movne r2,r1,lsl #3 \n"
        "movne r0,r0,lsr r2 \n"
        "rsbne r1,r2,#32 \n"
        "orrne r0,r0,r3,lsl r1\n"
        "and    r0, r0, #0xFFFFFF\n"
        "add    %1, %1, #3"
    : "=r" (__val)
                : "r"(my_pc), "m"(gen_regsPC)
                : "r1","r2","r3");
#endif

    return __val;
#else

    register unsigned long i;

    gen_regsPC+= 3;
    if((unsigned long)my_pc & 0x03)   //not dword aligned
    {
        i = *(my_pc++);
        i |= (*(my_pc++) << 8);
        i |= (*(my_pc++) << 16);
        return i;
    }
    else
    {
        i = *((unsigned long*)my_pc) & 0x00FFFFFF;
        my_pc+=3;
        return i;
    }

    //return i;
#endif
}

inline unsigned long read24SetLastbyte()
{
#ifdef TARGET_GP2X
    register unsigned long __val asm("r0");//%0 and r0 are the same, now

#ifdef GENREGSPC_AS_REG

    asm volatile(
        "add %3, %3, #4\n"
        "bic r1,%2,#3 \n"
        "ldmia r1,{r0,r3} \n"
        "ands r1,%2,#3 \n"
        "movne r2,r1,lsl #3 \n"
        "movne r0,r0,lsr r2 \n"
        "rsbne r1,r2,#32 \n"
        "orrne r0,r0,r3,lsl r1\n"
        "and    %1, r0, #0xFF000000\n"
        "mov    %1, %1, lsr #24\n"
        "and    r0, r0, #0x00FFFFFF\n"
        "add    %2, %2, #4"
    : "=r" (__val), "=r" (lastbyte)
                : "r"(my_pc), "r"(gen_regsPC)
                : "r1","r2","r3");
#else

    asm volatile(
        "ldr r3, %3\n"
        "add r3, r3, #4\n"
        "str r3, %3\n"
        "bic r1,%2,#3 \n"
        "ldmia r1,{r0,r3} \n"
        "ands r1,%2,#3 \n"
        "movne r2,r1,lsl #3 \n"
        "movne r0,r0,lsr r2 \n"
        "rsbne r1,r2,#32 \n"
        "orrne r0,r0,r3,lsl r1\n"
        "and    %1, r0, #0xFF000000\n"
        "mov    %1, %1, lsr #24\n"
        "and    r0, r0, #0x00FFFFFF\n"
        "add    %2, %2, #4"
    : "=r" (__val), "=r" (lastbyte)
                : "r"(my_pc), "m"(gen_regsPC)
                : "r1","r2","r3");
#endif

    return __val;
#else

    register unsigned long i;

    gen_regsPC+= 4;
    if((unsigned long)my_pc & 0x03)   //not dword aligned
    {
        i = *(my_pc++);
        i |= (*(my_pc++) << 8);
        i |= (*(my_pc++) << 16);
        lastbyte = *(my_pc++);
        return i;
    }
    else
    {
        i = *((unsigned long*)my_pc);
        lastbyte = i >> 24;
        i &= 0x00FFFFFF;

        my_pc+=4;
        return i;
    }

    //return i;
#endif
}

inline unsigned long readlong()
{
#ifdef TARGET_GP2X
    register unsigned long __val asm("r0");//%0 and r0 are the same, now

#ifdef GENREGSPC_AS_REG

    asm volatile(
        "add %2, %2, #4\n"
        "bic r1,%1,#3 \n"
        "ldmia r1,{r0,r3} \n"
        "ands r1,%1,#3 \n"
        "movne r2,r1,lsl #3 \n"
        "movne r0,r0,lsr r2 \n"
        "rsbne r1,r2,#32 \n"
        "orrne r0,r0,r3,lsl r1\n"
        "add    %1, %1, #4"
    : "=r" (__val)
                : "r"(my_pc), "r"(gen_regsPC)
                : "r1","r2","r3");
#else

    asm volatile(
        "ldr r3, %2\n"
        "add r3, r3, #4\n"
        "str r3, %2\n"
        "bic r1,%1,#3 \n"
        "ldmia r1,{r0,r3} \n"
        "ands r1,%1,#3 \n"
        "movne r2,r1,lsl #3 \n"
        "movne r0,r0,lsr r2 \n"
        "rsbne r1,r2,#32 \n"
        "orrne r0,r0,r3,lsl r1\n"
        "add    %1, %1, #4"
    : "=r" (__val)
                : "r"(my_pc), "m"(gen_regsPC)
                : "r1","r2","r3");
#endif

    return __val;
#else

    register unsigned long i;

    gen_regsPC+= 4;
    if((unsigned long)my_pc & 0x03)   //not dword aligned
    {
        i = *(my_pc++);
        i |= (*(my_pc++) << 8);
        i |= (*(my_pc++) << 16);
        i |= (*(my_pc++) << 24);
        return i;
    }
    else
    {
        i = *((unsigned long*)my_pc);
        my_pc+=4;
        return i;
    }

    return i;
#endif
}

inline void doJumpByte()
{
#ifdef TARGET_GP2X
 #ifdef GENREGSPC_AS_REG

    asm volatile(
        "ldrsb r0, [%1]\n\t"
        "adds    r0, r0, #1\n\t"
        "adds    %0, %0, r0\n\t"
        "adds    %1, %1, r0"
        :
        : "r" (gen_regsPC), "r" (my_pc)
        : "r0");

 #else
 #error doJumpByte not implemented
 #endif

#else
    signed char d8 = readbyte();
    gen_regsPC+=d8;
    my_pc+=d8;
#endif
}

inline void skipJumpByte()
{
#ifdef TARGET_GP2X
 #ifdef GENREGSPC_AS_REG

    asm volatile(
        "add    %0, %0, #1\n\t"
        "add    %1, %1, #1"
        :
        : "r" (gen_regsPC), "r" (my_pc)
        :);

 #else
 #error skipJumpByte not implemented
 #endif

#else
    ++gen_regsPC;
    ++my_pc;
#endif
}

inline void doJumpWord()
{
#ifdef TARGET_GP2X
 #ifdef GENREGSPC_AS_REG

    asm volatile(
        "ldrsb    r0, [%1,#1]\n\t"
        "ldrb    r1, [%1]\n\t"
        "orr     r0, r1, r0, asl #8\n\t"
        "add    r0, r0, #2\n\t"
        "add    %0, %0, r0\n\t"
        "add    %1, %1, r0"
        :
        : "r" (gen_regsPC), "r" (my_pc)
        : "r0", "r1");

 #else
 #error doJumpWord not implemented
 #endif

#else
    signed short d16 = readword();
    gen_regsPC+=d16;
    my_pc+=d16;
#endif
}

inline void skipJumpWord()
{
#ifdef TARGET_GP2X
 #ifdef GENREGSPC_AS_REG

    asm volatile(
        "add    %0, %0, #2\n\t"
        "add    %1, %1, #2"
        :
        : "r" (gen_regsPC), "r" (my_pc)
        :);

 #else
 #error skipJumpWord not implemented
 #endif

#else
    gen_regsPC+=2;
    my_pc+=2;
#endif
}


#define cond0() (FALSE)
#define cond1() (((gen_regsSR & (SF|VF)) == SF) || ((gen_regsSR & (SF|VF)) == VF))
#define cond2() ((gen_regsSR & ZF) || (((gen_regsSR & (SF|VF)) == SF) || ((gen_regsSR & (SF|VF)) == VF)))
#define cond3() (gen_regsSR & (ZF|CF))
#define cond4() (gen_regsSR & VF)
#define cond5() (gen_regsSR & SF)
#define cond6() (gen_regsSR & ZF)
#define cond7() (gen_regsSR & CF)
#define cond8() (TRUE)
#define cond9() (!(((gen_regsSR & (SF|VF)) == SF) || ((gen_regsSR & (SF|VF)) == VF)))
#define notCond9() (((gen_regsSR & (SF|VF)) == SF) || ((gen_regsSR & (SF|VF)) == VF))
#define condA() (!((gen_regsSR & ZF) || (((gen_regsSR & (SF|VF)) == SF) || ((gen_regsSR & (SF|VF)) == VF))))
#define notCondA() ((gen_regsSR & ZF) || (((gen_regsSR & (SF|VF)) == SF) || ((gen_regsSR & (SF|VF)) == VF)))
#define condB() (!(gen_regsSR & (ZF|CF)))
#define notCondB() (gen_regsSR & (ZF|CF))
#define condC() (!(gen_regsSR & VF))
#define notCondC() (gen_regsSR & VF)
#define condD() (!(gen_regsSR & SF))
#define notCondD() (gen_regsSR & SF)
#define condE() (!(gen_regsSR & ZF))
#define notCondE() (gen_regsSR & ZF)
#define condF() (!(gen_regsSR & CF))
#define notCondF() (gen_regsSR & CF)

// status register check functions
inline unsigned char srF()
{
    return 0;
}
inline unsigned char srLT()
{
    return ((((gen_regsSR & (SF|VF)) == SF)
             || ((gen_regsSR & (SF|VF)) == VF)) ? 1 : 0);
}
inline unsigned char srLE()
{
    return (((((gen_regsSR & (SF|VF)) == SF)
              || ((gen_regsSR & (SF|VF)) == VF))
             || (gen_regsSR & ZF)) ? 1 : 0);
}
inline unsigned char srULE()
{
    return ((gen_regsSR & (ZF|CF)) ? 1 : 0);
}
inline unsigned char srOV()
{
    return ((gen_regsSR & VF) ? 1 : 0);
}
inline unsigned char srMI()
{
    return ((gen_regsSR & SF) ? 1 : 0);
}
inline unsigned char srZ()
{
    return ((gen_regsSR & ZF) ? 1 : 0);
}
inline unsigned char srC()
{
    return ((gen_regsSR & CF) ? 1 : 0);
}
inline unsigned char srT()
{
    return 1;
}
inline unsigned char srGE()
{
    return ((((gen_regsSR & (SF|VF)) == SF)
             || ((gen_regsSR & (SF|VF)) == VF)) ? 0 : 1);
}
//unsigned char srGE() { return ((((gen_regsSR & (SF|VF)) == (VF|SF))
//          || ((gen_regsSR & (SF|VF)) == 0)) ? 1 : 0); }
inline unsigned char srGT()
{
    return (((((gen_regsSR & (SF|VF)) == SF)
              || ((gen_regsSR & (SF|VF)) == VF))
             || (gen_regsSR & ZF)) ? 0 : 1);
}
//unsigned char srGT() { return (((((gen_regsSR & (SF|VF)) == (VF|SF))
//          || ((gen_regsSR & (SF|VF)) == 0))
//          && (gen_regsSR & ZF)) ? 1 : 0); }
inline unsigned char srUGT()
{
    return ((gen_regsSR & (ZF|CF)) ? 0 : 1);
}
inline unsigned char srNOV()
{
    return ((gen_regsSR & VF) ? 0 : 1);
}
inline unsigned char srPL()
{
    return ((gen_regsSR & SF) ? 0 : 1);
}
inline unsigned char srNZ()
{
    return ((gen_regsSR & ZF) ? 0 : 1);
}
inline unsigned char srNC()
{
    return ((gen_regsSR & CF) ? 0 : 1);
}

#define valCond0 srF
#define valCond1 srLT
#define valCond2 srLE
#define valCond3 srULE
#define valCond4 srOV
#define valCond5 srMI
#define valCond6 srZ
#define valCond7 srC
#define valCond8 srT
#define valCond9 srGE
#define valCondA srGT
#define valCondB srUGT
#define valCondC srNOV
#define valCondD srPL
#define valCondE srNZ
#define valCondF srNC


inline void set_cregs()  //optimized by Thor
{
    int i;
    static int lastbank = -1;

    i = (gen_regsSR & 0x0700)>>4;

    if (i==lastbank)
        return;
    lastbank = i;

    if (i)
    {
        memcpy(&allregsB[0xd0], &allregsB[i-16],32*sizeof(allregsB[0]));
        memcpy(&allregsW[0xd0], &allregsW[i-16],32*sizeof(allregsW[0]));
        memcpy(&allregsL[0xd0], &allregsL[i-16],32*sizeof(allregsL[0]));
    }
    else
    {
        // set previous register bank registers
        memcpy(&allregsB[0xd0], &allregsB[48],16*sizeof(allregsB[0]));
        memcpy(&allregsW[0xd0], &allregsW[48],16*sizeof(allregsW[0]));
        memcpy(&allregsL[0xd0], &allregsL[48],16*sizeof(allregsL[0]));

        // set current register bank registers
        memcpy(&allregsB[0xe0], &allregsB[0], 16*sizeof(allregsB[0]));
        memcpy(&allregsW[0xe0], &allregsW[0], 16*sizeof(allregsW[0]));
        memcpy(&allregsL[0xe0], &allregsL[0], 16*sizeof(allregsL[0]));
    }

    //This loop should get unrolled by the compiler
    for (i=0;i<4;i++)
    {
        // set the current set byte pointers
        // byte register are: W, A, B, C, D, E, H, L
        // however, in the all byte registers table
        // they are stored as: A W QA QW etc.
        cregsB[i*2] = allregsB[0xe0+4*i+1];
        cregsB[i*2+1] = allregsB[0xe0+4*i];

        // set the current set word and long word pointers
        cregsW[i] = allregsW[0xe0+4*i];
        cregsL[i] = allregsL[0xe0+4*i];
    }
}


//////////////////////////////////////////////////////////////////////////////
////////////////////////////// LD ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int ldRrB()
{
    *cregsB[lastbyte&7] = *regB;
    return 4;
}
int ldRrW()
{
    *cregsW[lastbyte&7] = *regW;
    return 4;
}
int ldRrL()
{
    *cregsL[lastbyte&7] = *regL;
    return 4;
}
int ldrRB()
{
    *regB = *cregsB[lastbyte&7];
    return 4;
}
int ldrRW()
{
    *regW = *cregsW[lastbyte&7];
    return 4;
}
int ldrRL()
{
    *regL = *cregsL[lastbyte&7];
    return 4;
}
int ldr3B()
{
    *regB = lastbyte&7;
    return 4;
}
int ldr3W()
{
    *regW = lastbyte&7;
    return 4;
}
int ldr3L()
{
    *regL = lastbyte&7;
    return 4;
}
int ldRIB()
{
    *cregsB[opcode&7] = readbyte();
    return 2;
}
int ldRIW()
{
    *cregsW[opcode&7] = readword();
    return 3;
}
int ldRIL()
{
    *cregsL[opcode&7] = readlong();
    return 5;
}
int ldrIB()
{
    *regB = readbyte();
    return 4;
}
int ldrIW()
{
    *regW = readword();
    return 4;
}
int ldrIL()
{
    *regL = readlong();
    return 6;
}
int ldRM00()
{
    *cregsB[lastbyte&7] = memB;
    return 4;
}
int ldRM10()
{
    *cregsW[lastbyte&7] = memW;
    return 4;
}
int ldRM20()
{
    *cregsL[lastbyte&7] = memL;
    return 6;
}
int ldMR30B()
{
    mem_writeB(mem,*cregsB[lastbyte&7]);
    return 4;
}
int ldMR30W()
{
    mem_writeW(mem,*cregsW[lastbyte&7]);
    return 4;
}
int ldMR30L()
{
	if (mem<0x38000) // ALEK Bug for MSG2, WHY o_O ?
    mem_writeL(mem,*cregsL[lastbyte&7]);
    return 6;
}
int ld8I()
{
    unsigned long num8 = readbyte();
    tlcsMemWriteBaddrB(num8,readbyte());
    return 5;
}
int ldw8I()
{
    unsigned long num8 = readbyte();
    tlcsMemWriteWaddrB(num8,readword());
    return 6;
}
int ldMI30()
{
    mem_writeB(mem,readbyte());
    return 5;
}
int ldwMI30()
{
    mem_writeW(mem,readword());
    return 6;
}
int ld16M00()
{
    mem_writeB(readword(),memB);
    return 8;
}
int ldw16M10()
{
    mem_writeW(readword(),memW);
    return 8;
}
int ldM1630()
{
    mem_writeB(mem,mem_readB(readword()));
    return 8;
}
int ldwM1630()
{
    mem_writeW(mem,mem_readW(readword()));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// PUSH //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define NG_PUSH_BYTE(src,s) tlcsFastMemWriteB(--gen_regsXSP,src); return s;
#define NG_PUSH_WORD(src,s) tlcsFastMemWriteW(gen_regsXSP-=2,src); return s;
#define NG_PUSH_LONG(src,s) tlcsFastMemWriteL(gen_regsXSP-=4,src); return s;

int pushsr()
{
    NG_PUSH_WORD((unsigned short)gen_regsSR&0x0000ffff, 4);
}
int pushF()
{
    NG_PUSH_BYTE((unsigned char)gen_regsSR&0x000000ff, 3);
}
int pushA()
{
    NG_PUSH_BYTE(*cregsB[1], 3);
}
int pushRW()
{
    NG_PUSH_WORD(*cregsW[opcode&7], 3);
}
int pushRL()
{
    NG_PUSH_LONG(*cregsL[opcode&7], 5);
}
int pushrB()
{
    NG_PUSH_BYTE(*regB, 5);
}
int pushrW()
{
    NG_PUSH_WORD(*regW, 5);
}
int pushrL()
{
    NG_PUSH_LONG(*regL, 7);
}
int pushI()
{
    NG_PUSH_BYTE(readbyte(), 4);
}
int pushwI()
{
    NG_PUSH_WORD(readword(), 5);
}
int pushM00()
{
    NG_PUSH_BYTE(memB, 7);
}
int pushwM10()
{
    NG_PUSH_WORD(memW, 7);
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// POP ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int popsr() // POP SR   00000011
{
    gen_regsSR = mem_readW(gen_regsXSP);
    gen_regsXSP+=2;
    set_cregs();
    return 6;
}

int popF()  // POP F   00011001
{
    gen_regsSR = (gen_regsSR&0xff00)|mem_readB(gen_regsXSP);
    gen_regsXSP+=1;
    return 4;
}

int popA()
{
    *cregsB[1] = mem_readB(gen_regsXSP);
    gen_regsXSP+=1;
    return 4;
}
int popRW()
{
    *cregsW[opcode&7] = mem_readW(gen_regsXSP);
    gen_regsXSP+=2;
    return 4;
}
int popRL()
{
    *cregsL[opcode&7] = mem_readL(gen_regsXSP);
    gen_regsXSP+=4;
    return 6;
}
int poprB()
{
    *regB = mem_readB(gen_regsXSP);
    gen_regsXSP+=1;
    return 6;
}
int poprW()
{
    *regW = mem_readW(gen_regsXSP);
    gen_regsXSP+=2;
    return 6;
}
int poprL()
{
    *regL = mem_readL(gen_regsXSP);
    gen_regsXSP+=4;
    return 8;
}
int popM30()
{
    mem_writeB(mem,mem_readB(gen_regsXSP));
    gen_regsXSP+=1;
    return 6;
}
int popwM30()
{
    mem_writeW(mem,mem_readW(gen_regsXSP));
    gen_regsXSP+=2;
    return 6;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// LDA ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int ldaRMW30()
{
    *cregsW[lastbyte&7] = (unsigned short)(mem&0x0000ffff);
    return 4;
}
int ldaRML30()
{
    *cregsL[lastbyte&7] = mem;
    return 4;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// LDAR //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int ldar() // LDAR R,$+4+d16 11110011 00010011 xxxxxxxx xxxxxxxx 001s0RRR
{
    unsigned short i = readword();
    unsigned char j;

    j = readbyte();
    if (j & 0x10)
    {  // long
        *cregsL[j&7] = gen_regsPC-1+(signed short)i;
    }
    else
    {  // word
        *cregsW[j&7] = (unsigned short)((gen_regsPC-1+(signed short)i)&0x0000ffff);
    }
    return 11;
}


//////////////////////////////////////////////////////////////////////////////
////////////////////////////// EX ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int exFF()  // EX F,F'  00010110
{
    unsigned char i = F2;

    F2 = (unsigned char)(gen_regsSR&0xff);
    gen_regsSR = (gen_regsSR&0xff00)|i;
    return 2;
}

int exRrB() // EX R,r  11001rrr 10111RRR
{
    unsigned char i = *cregsB[lastbyte&7];
    *cregsB[lastbyte&7] = *regB;
    *regB = i;
    return 5;
}

int exRrW() // EX R,r  11011rrr 10111RRR
{
    unsigned short i = *cregsW[lastbyte&7];
    *cregsW[lastbyte&7] = *regW;
    *regW = i;
    return 5;
}

int exMRB00() // EX (mem),R 10000mmm 00110RRR
{
    unsigned char i = memB;
    mem_writeB(mem,*cregsB[lastbyte&7]);
    *cregsB[lastbyte&7] = i;
    return 6;
}

int exMRW10() // EX (mem),R 10010mmm 00110RRR
{
    unsigned short i = memW;
    mem_writeW(mem,*cregsW[lastbyte&7]);
    *cregsW[lastbyte&7] = i;
    return 6;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// MIRR //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

const unsigned char mirrTable[256] = {  //Flavor speed hack
                                   0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
                                   0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
                                   0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
                                   0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
                                   0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
                                   0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
                                   0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
                                   0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
                                   0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
                                   0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
                                   0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
                                   0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
                                   0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
                                   0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
                                   0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
                                   0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};


int mirrr() // MIRR r  11011rrr 00010110
{
    unsigned short i=*regW;

    *regW=mirrTable[i&0xFF]<<8 | mirrTable[i>>8];

    return 4;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// LDxx //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int ldi()  // LDI (XDE+),(XHL+) 10000011 00010000
// LDI (XIX+),(XIY+) 10000101 00010000
{
    if (opcode&2)
    {
        // XDE/XHL
        mem_writeB((*cregsL[2])++,mem_readB((*cregsL[3])++));
        //*cregsL[2]+=1;
        //*cregsL[3]+=1;
    }
    else
    {
        // XIX/XIY
        mem_writeB(gen_regsXIX++,mem_readB(gen_regsXIY++));
        //mem_writeB(*cregsL[4],mem_readB(*cregsL[5]));
        //*cregsL[4]+=1;
        //*cregsL[5]+=1;
    }
    *cregsW[1]-=1; // BC
    gen_regsSR = gen_regsSR & ~(HF|VF|NF);
    gen_regsSR = gen_regsSR | ((*cregsW[1]) ? VF : 0);
    return 10;
}

int ldiw()  // LDIW (XDE+),(XHL+) 10010011 00010000
// LDIW (XIX+),(XIY+) 10010101 00010000
{
    if (opcode&2)
    {
        // XDE/XHL
        mem_writeW(*cregsL[2],mem_readW(*cregsL[3]));
        *cregsL[2]+=2;
        *cregsL[3]+=2;
    }
    else
    {
        // XIX/XIY
        mem_writeW(gen_regsXIX,mem_readW(gen_regsXIY));
        gen_regsXIX+=2;
        gen_regsXIY+=2;
        /*
        mem_writeW(*cregsL[4],mem_readW(*cregsL[5]));
        *cregsL[4]+=2;
        *cregsL[5]+=2;
        */
    }
    *cregsW[1]-=1; // BC
    gen_regsSR = gen_regsSR & ~(HF|VF|NF);
    gen_regsSR = gen_regsSR | ((*cregsW[1]) ? VF : 0);
    return 10;
}

int ldir()  // LDIR (XDE+),(XHL+) 10000011 00010001
// LDIR (XIX+),(XIY+) 10000101 00010001
{
    if (opcode&2)
    {
        // XDE/XHL
        mem_writeB((*cregsL[2])++,mem_readB((*cregsL[3])++));
    }
    else
    {
        // XIX/XIY
        mem_writeB(gen_regsXIX++,mem_readB(gen_regsXIY++));
    }

    gen_regsSR = gen_regsSR & ~(HF|VF|NF);
    if (--(*cregsW[1])) //BC
    {
        gen_regsPC-=2;
        my_pc-=2;
        gen_regsSR = gen_regsSR | VF;
        return 14;
    }
    else
    {
        return 10;
    }
}

int ldirw() // LDIRW (XDE+),(XHL+) 10010011 00010001
// LDIRW (XIX+),(XIY+) 10010101 00010001
{
    if (opcode&2)
    { // XDE/XHL
        mem_writeW(*cregsL[2],mem_readW(*cregsL[3]));
        *cregsL[2]+= 2;
        *cregsL[3]+= 2;
    }
    else
    {  // XIX/XIY
        mem_writeW(gen_regsXIX,mem_readW(gen_regsXIY));
        gen_regsXIX+=2;
        gen_regsXIY+=2;
//        mem_writeW(*cregsL[4],mem_readW(*cregsL[5]));
//        *cregsL[4]+= 2;
//        *cregsL[5]+= 2;
    }
    //*cregsW[1]-= 1; // BC
    gen_regsSR = gen_regsSR & ~(HF|VF|NF);
    if (--(*cregsW[1]))
    {
        gen_regsPC-=2;
        my_pc-=2;
        gen_regsSR = gen_regsSR | VF;
        return 14;
        //  return 14+4;
    }
    else
    {
        return 10;
        //  return 10+4;
    }
}

int ldd()  // LDD (XDE-),(XHL-) 10000011 00010010
// LDD (XIX-),(XIY-) 10000101 00010010
{
    if (opcode&2)
    { // XDE/XHL
        mem_writeB((*cregsL[2])--,mem_readB((*cregsL[3])--));
        //*cregsL[2]-= 1;
        //*cregsL[3]-= 1;
    }
    else
    {  // XIX/XIY
        mem_writeB(gen_regsXIX--,mem_readB(gen_regsXIY--));
        //mem_writeB(*cregsL[4],mem_readB(*cregsL[5]));
        //*cregsL[4]-= 1;
        //*cregsL[5]-= 1;
    }
    *cregsW[1]-=1; // BC
    gen_regsSR = gen_regsSR & ~(HF|VF|NF);
    gen_regsSR = gen_regsSR | ((*cregsW[1]) ? VF : 0);
    return 10;
}

int lddw()  // LDDW (XDE-),(XHL-) 10010011 00010010
// LDDW (XIX-),(XIY-) 10010101 00010010
{
    if (opcode&2)
    { // XDE/XHL
        mem_writeW(*cregsL[2],mem_readW(*cregsL[3]));
        *cregsL[2]-= 2;
        *cregsL[3]-= 2;
    }
    else
    {  // XIX/XIY
        mem_writeW(gen_regsXIX,mem_readW(gen_regsXIY));
        gen_regsXIX-=2;
        gen_regsXIY-=2;
//        mem_writeW(*cregsL[4],mem_readW(*cregsL[5]));
//        *cregsL[4]-= 2;
//        *cregsL[5]-= 2;
    }
    *cregsW[1]-=1; // BC
    gen_regsSR = gen_regsSR & ~(HF|VF|NF);
    gen_regsSR = gen_regsSR | ((*cregsW[1]) ? VF : 0);
    return 10;
}

int lddr()  // LDDR (XDE-),(XHL-) 10000011 00010011
// LDDR (XIX-),(XIY-) 10000101 00010011
{
    if (opcode&2)
    { // XDE/XHL
        mem_writeB((*cregsL[2])--,mem_readB((*cregsL[3])--));
        //*cregsL[2]-= 1;
        //*cregsL[3]-= 1;
    }
    else
    {  // XIX/XIY
        mem_writeB(gen_regsXIX--,mem_readB(gen_regsXIY--));
        //mem_writeB(*cregsL[4],mem_readB(*cregsL[5]));
        //*cregsL[4]-= 1;
        //*cregsL[5]-= 1;
    }
//    *cregsW[1]-=1; // BC
    gen_regsSR = gen_regsSR & ~(HF|VF|NF);
    if (--(*cregsW[1]))
    {
        gen_regsPC-=2;
        my_pc-=2;
        gen_regsSR = gen_regsSR | VF;
        return 14;
    }
    else
    {
        return 10;
    }
}

int lddrw() // LDDRW (XDE-),(XHL-) 10010011 00010011
// LDDRW (XIX-),(XIY-) 10010101 00010011
{
    if (opcode&2)
    { // XDE/XHL
        mem_writeW(*cregsL[2],mem_readW(*cregsL[3]));
        *cregsL[2]-= 2;
        *cregsL[3]-= 2;
    }
    else
    {  // XIX/XIY
        mem_writeW(gen_regsXIX,mem_readW(gen_regsXIY));
        gen_regsXIX-=2;
        gen_regsXIY-=2;
        //mem_writeW(*cregsL[4],mem_readW(*cregsL[5]));
        //*cregsL[4]-= 2;
        //*cregsL[5]-= 2;
    }
    //*cregsW[1]-= 1; // BC
    gen_regsSR = gen_regsSR & ~(HF|VF|NF);
    if (--(*cregsW[1]))
    {
        gen_regsPC-=2;
        my_pc-=2;
        gen_regsSR = gen_regsSR | VF;
        return 14;
    }
    else
    {
        return 10;
    }
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// CPxx //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// H flag is always set to 0, instead of being set to 1 when a borrow from bit
//   3 to bit 4 occurs as a result of src1-src2
// just hoping this never happens :/

int cpiB()  // CPI A,(R+)   10000RRR 00010100
{
    unsigned char i = (*cregsB[1]) - memB;

    *cregsL[opcode&7]+=1;
    *cregsW[1]-=1; // BC
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? NF : NF|ZF) |
                 ((*cregsW[1]) ? VF : 0);
    return 8;
}

int cpiW()  // CPI WA,(R+)   10010RRR 00010100
{
    unsigned short i = (*cregsW[0]) - memW;

    *cregsL[opcode&7]+= 2;
    *cregsW[1]-=1; // BC
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? NF : NF|ZF) |
                 ((*cregsW[1]) ? VF : 0);
    return 8;
}

int cpirB() // CPIR A,(R+)   10000RRR 00010101
{
    unsigned char i = (*cregsB[1]) - memB;

    *cregsL[opcode&7]+= 1;
    *cregsW[1]-= 1; // BC
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 NF | Ztable[i] | //((i) ? NF : NF|ZF) |
                 ((*cregsW[1]) ? VF : 0);
    if ((gen_regsSR & (ZF|VF)) == VF)
    {
        gen_regsPC-=2;
        my_pc-=2;
        return 14;
    }
    else
    {
        return 10;
    }
}

int cpirW() // CPIR WA,(R+)   10010RRR 00010101
{
    unsigned short i = (*cregsW[0]) - memW;

    *cregsL[opcode&7]+=2;
    *cregsW[1]-=1; // BC
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? NF : NF|ZF) |
                 ((*cregsW[1]) ? VF : 0);
    if ((gen_regsSR & (ZF|VF)) == VF)
    {
        gen_regsPC-=2;
        my_pc-=2;
        return 14;
    }
    else
    {
        return 10;
    }
}

int cpdB()  // CPD A,(R-)   10000RRR 00010110
{
    unsigned char i = (*cregsB[1]) - memB;

    *cregsL[opcode&7]-= 1;
    *cregsW[1]-= 1; // BC
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 NF | Ztable[i] | //((i) ? NF : NF|ZF) |
                 ((*cregsW[1]) ? VF : 0);
    return 8;
}

int cpdW()  // CPD WA,(R-)   10010RRR 00010110
{
    unsigned short i = (*cregsW[0]) - memW;

    *cregsL[opcode&7]-=2;
    *cregsW[1]-=1; // BC
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? NF : NF|ZF) |
                 ((*cregsW[1]) ? VF : 0);
    return 8;
}

int cpdrB() // CPDR A,(R-)   10000RRR 00010111
{
    unsigned char i = (*cregsB[1]) - memB;

    *cregsL[opcode&7]-= 1;
    *cregsW[1]-= 1; // BC
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 NF | Ztable[i] | //((i) ? NF : NF|ZF) |
                 ((*cregsW[1]) ? VF : 0);
    if ((gen_regsSR & (ZF|VF)) == VF)
    {
        gen_regsPC-=2;
        my_pc-=2;
        return 14;
    }
    else
    {
        return 10;
    }
}

int cpdrW() // CPDR WA,(R-)   10010RRR 00010111
{
    unsigned short i = (*cregsW[0]) - memW;

    *cregsL[opcode&7]-=2;
    *cregsW[1]-=1; // BC
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? NF : NF|ZF) |
                 ((*cregsW[1]) ? VF : 0);
    if ((gen_regsSR & (ZF|VF)) == VF)
    {
        gen_regsPC-=2;
        my_pc-=2;
        return 14;
    }
    else
    {
        return 10;
    }
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// ADD ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MyAddB(unsigned char i, unsigned char j)
{
    // 100% correct
    unsigned char oldi = i;

    i+= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? 0 : ZF) |
                 (((oldi^j)^i) & HF) |
                 (((i^oldi) & (i^j) & 0x80) ? VF : 0) |
                 ((i<oldi) ? CF : 0);
    return i;
}

inline unsigned short MyAddW(unsigned short i, unsigned short j)
{
    unsigned short oldi = i;

    i+= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? 0 : ZF) |
                 (((oldi^j)^i) & HF) |
                 (((i^oldi) & (i^j) & 0x8000) ? VF : 0) |
                 ((i<oldi) ? CF : 0);
    return i;
}

inline unsigned long MyAddL(unsigned long i, unsigned long j)
{
    unsigned long oldi = i;

    i+= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? 0 : ZF) |
                 (((i^oldi) & (i^j) & 0x80000000) ? VF : 0) |
                 ((i<oldi) ? CF : 0);
    return i;
}

int addRrB()
{
    *cregsB[lastbyte&7] = MyAddB(*cregsB[lastbyte&7],*regB);
    return 4;
}
int addRrW()
{
    *cregsW[lastbyte&7] = MyAddW(*cregsW[lastbyte&7],*regW);
    return 4;
}
int addRrL()
{
    *cregsL[lastbyte&7] = MyAddL(*cregsL[lastbyte&7],*regL);
    return 7;
}
int addrIB()
{
    *regB = MyAddB(*regB,readbyte());
    return 4;
}
int addrIW()
{
    *regW = MyAddW(*regW,readword());
    return 4;
}
int addrIL()
{
    *regL = MyAddL(*regL,readlong());
    return 7;
}
int addRMB00()
{
    *cregsB[lastbyte&7] = MyAddB(*cregsB[lastbyte&7],memB);
    return 4;
}
int addRMW10()
{
    *cregsW[lastbyte&7] = MyAddW(*cregsW[lastbyte&7],memW);
    return 4;
}
int addRML20()
{
    *cregsL[lastbyte&7] = MyAddL(*cregsL[lastbyte&7],memL);
    return 6;
}
int addMRB00()
{
    mem_writeB(mem,MyAddB(memB,*cregsB[lastbyte&7]));
    return 6;
}
int addMRW10()
{
    mem_writeW(mem,MyAddW(memW,*cregsW[lastbyte&7]));
    return 6;
}
int addMRL20()
{
    mem_writeL(mem,MyAddL(memL,*cregsL[lastbyte&7]));
    return 10;
}
int addMI00()
{
    mem_writeB(mem,MyAddB(memB,readbyte()));
    return 7;
}
int addwMI10()
{
    mem_writeW(mem,MyAddW(memW,readword()));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// ADC ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MyAdcB(unsigned char i, unsigned char j)
{
    // 100% correct
    unsigned char oldi = i;
    unsigned char oldC = (unsigned char)(gen_regsSR & CF);

    i+= j + oldC;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? 0 : ZF) |
                 (((oldi^j)^i) & HF) |
                 (((i^oldi) & (i^j) & 0x80) ? VF : 0) |
                 ((i<oldi || (i==oldi && oldC)) ? CF : 0);
    return i;
}

inline unsigned short MyAdcW(unsigned short i, unsigned short j)
{
    unsigned short oldi = i;
    unsigned short oldC = (unsigned short)(gen_regsSR & CF);

    i+= j + oldC;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? 0 : ZF) |
                 (((oldi^j)^i) & HF) |
                 (((i^oldi) & (i^j) & 0x8000) ? VF : 0) |
                 ((i<oldi || (i==oldi && oldC)) ? CF : 0);
    return i;
}

inline unsigned long MyAdcL(unsigned long i, unsigned long j)
{
    unsigned long oldi = i;
    unsigned long oldC = gen_regsSR & CF;

    i+= j + oldC;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? 0 : ZF) |
                 (((i^oldi) & (i^j) & 0x80000000) ? VF : 0) |
                 ((i<oldi || (i==oldi && oldC)) ? CF : 0);
    return i;
}

int adcRrB()
{
    *cregsB[lastbyte&7] = MyAdcB(*cregsB[lastbyte&7],*regB);
    return 4;
}
int adcRrW()
{
    *cregsW[lastbyte&7] = MyAdcW(*cregsW[lastbyte&7],*regW);
    return 4;
}
int adcRrL()
{
    *cregsL[lastbyte&7] = MyAdcL(*cregsL[lastbyte&7],*regL);
    return 7;
}
int adcrIB()
{
    *regB = MyAdcB(*regB,readbyte());
    return 4;
}
int adcrIW()
{
    *regW = MyAdcW(*regW,readword());
    return 4;
}
int adcrIL()
{
    *regL = MyAdcL(*regL,readlong());
    return 7;
}
int adcRMB00()
{
    *cregsB[lastbyte&7] = MyAdcB(*cregsB[lastbyte&7],memB);
    return 4;
}
int adcRMW10()
{
    *cregsW[lastbyte&7] = MyAdcW(*cregsW[lastbyte&7],memW);
    return 4;
}
int adcRML20()
{
    *cregsL[lastbyte&7] = MyAdcL(*cregsL[lastbyte&7],memL);
    return 6;
}
int adcMRB00()
{
    mem_writeB(mem,MyAdcB(memB,*cregsB[lastbyte&7]));
    return 6;
}
int adcMRW10()
{
    mem_writeW(mem,MyAdcW(memW,*cregsW[lastbyte&7]));
    return 6;
}
int adcMRL20()
{
    mem_writeL(mem,MyAdcL(memL,*cregsL[lastbyte&7]));
    return 10;
}
int adcMI00()
{
    mem_writeB(mem,MyAdcB(memB,readbyte()));
    return 7;
}
int adcwMI10()
{
    mem_writeW(mem,MyAdcW(memW,readword()));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// SUB ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MySubB(unsigned char i, unsigned char j)
{
#if 1//speed hack
    int res = i - j;
	gen_regsSR &= ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR |= SZtable[res & 0xFF] |                             // S/Z flag
        ((i ^ res ^ j) & HF) |                  // H flag
        (((j ^ i) & (i ^ res) & 0x80) >> 5) |       // V flag
        ((res >> 8) & CF) | NF;               // C/N flag
    return res;

#else
	// 100% correct
    unsigned char oldi = i;

    i-= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? NF : NF|ZF) |
                 (((oldi^j)^i) & HF) |
                 (((j^oldi) & (i^oldi) & 0x80) ? VF : 0) |
                 ((i>oldi) ? CF : 0);
    return i;
#endif
}

inline unsigned short MySubW(unsigned short i, unsigned short j)
{
#if 1//speed hack
    int res = i - j;
	gen_regsSR &= ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR |= ((res>>8) & SF) |
		(Ztable[(res>>8) & 0xFF] & Ztable[res & 0xFF]) |
        ((i ^ res ^ j) & HF) |                  // H flag
        (((j ^ i) & (i ^ res) & 0x8000) >> 13) |       // V flag
        ((res >> 16) & CF) | NF;               // C/N flag
    return res;

#else
	unsigned short oldi = i;

    i-= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? NF : NF|ZF) |
                 (((oldi^j)^i) & HF) |
                 (((j^oldi) & (i^oldi) & 0x8000) ? VF : 0) |
                 ((i>oldi) ? CF : 0);
    return i;
#endif
}

inline unsigned long MySubL(unsigned long i, unsigned long j)
{
#if 1//speed hack
    unsigned long oldi = i;

    i-= j;
	gen_regsSR &= ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR |= ((i>>24) & SF) |
	(Ztable[(i>>24) & 0xFF] & Ztable[(i>>16) & 0xFF] & Ztable[(i>>8) & 0xFF] & Ztable[i & 0xFF]) |
                 (((j^oldi) & (i^oldi) & 0x80000000) ? VF : 0) |
                 ((i>oldi) ? CF : 0) |
				 NF;
    return i;
#else
    unsigned long oldi = i;

    i-= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? NF : NF|ZF) |
                 (((j^oldi) & (i^oldi) & 0x80000000) ? VF : 0) |
                 ((i>oldi) ? CF : 0);
    return i;
#endif
}

int subRrB()
{
    *cregsB[lastbyte&7] = MySubB(*cregsB[lastbyte&7],*regB);
    return 4;
}
int subRrW()
{
    *cregsW[lastbyte&7] = MySubW(*cregsW[lastbyte&7],*regW);
    return 4;
}
int subRrL()
{
    *cregsL[lastbyte&7] = MySubL(*cregsL[lastbyte&7],*regL);
    return 7;
}
int subrIB()
{
    *regB = MySubB(*regB,readbyte());
    return 4;
}
int subrIW()
{
    *regW = MySubW(*regW,readword());
    return 4;
}
int subrIL()
{
    *regL = MySubL(*regL,readlong());
    return 7;
}
int subRMB00()
{
    *cregsB[lastbyte&7] = MySubB(*cregsB[lastbyte&7],memB);
    return 4;
}
int subRMW10()
{
    *cregsW[lastbyte&7] = MySubW(*cregsW[lastbyte&7],memW);
    return 4;
}
int subRML20()
{
    *cregsL[lastbyte&7] = MySubL(*cregsL[lastbyte&7],memL);
    return 6;
}
int subMRB00()
{
    mem_writeB(mem,MySubB(memB,*cregsB[lastbyte&7]));
    return 6;
}
int subMRW10()
{
    mem_writeW(mem,MySubW(memW,*cregsW[lastbyte&7]));
    return 6;
}
int subMRL20()
{
    mem_writeL(mem,MySubL(memL,*cregsL[lastbyte&7]));
    return 10;
}
int subMI00()
{
    mem_writeB(mem,MySubB(memB,readbyte()));
    return 7;
}
int subwMI10()
{
    mem_writeW(mem,MySubW(memW,readword()));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// SBC ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MySbcB(unsigned char i, unsigned char j)
{
    // 100% correct
    unsigned char oldi = i;
    unsigned char oldC = (unsigned char)(gen_regsSR & CF);

    i = i - j - oldC;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 SZtable[i & 0xFF] | NF |
                 (((oldi^j)^i) & HF) |
                 (((j^oldi) & (i^oldi) & 0x80) >> 5) |
                 ((i>oldi || (oldC && (j==0xFF))) ? CF : 0);
    return i;
}

inline unsigned short MySbcW(unsigned short i, unsigned short j)
{
    unsigned short oldi = i;
    unsigned short oldC = (unsigned short)(gen_regsSR & CF);

    i = i - j - oldC;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? NF : NF|ZF) |
                 (((oldi^j)^i) & HF) |
                 (((j^oldi) & (i^oldi) & 0x8000) ? VF : 0) |
                 ((i>oldi || (oldC && (j==0xFFFF))) ? CF : 0);
    return i;
}

inline unsigned long MySbcL(unsigned long i, unsigned long j)
{
    unsigned long oldi = i;
    unsigned long oldC = gen_regsSR & CF;

    i = i - j - oldC;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? NF : NF|ZF) |
                 (((j^oldi) & (i^oldi) & 0x80000000) ? VF : 0) |
                 ((i>oldi || (oldC && (j==0xFFFFFFFF))) ? CF : 0);
    return i;
}

int sbcRrB()
{
    *cregsB[lastbyte&7] = MySbcB(*cregsB[lastbyte&7],*regB);
    return 4;
}
int sbcRrW()
{
    *cregsW[lastbyte&7] = MySbcW(*cregsW[lastbyte&7],*regW);
    return 4;
}
int sbcRrL()
{
    *cregsL[lastbyte&7] = MySbcL(*cregsL[lastbyte&7],*regL);
    return 7;
}
int sbcrIB()
{
    *regB = MySbcB(*regB,readbyte());
    return 4;
}
int sbcrIW()
{
    *regW = MySbcW(*regW,readword());
    return 4;
}
int sbcrIL()
{
    *regL = MySbcL(*regL,readlong());
    return 7;
}
int sbcRMB00()
{
    *cregsB[lastbyte&7] = MySbcB(*cregsB[lastbyte&7],memB);
    return 4;
}
int sbcRMW10()
{
    *cregsW[lastbyte&7] = MySbcW(*cregsW[lastbyte&7],memW);
    return 4;
}
int sbcRML20()
{
    *cregsL[lastbyte&7] = MySbcL(*cregsL[lastbyte&7],memL);
    return 6;
}
int sbcMRB00()
{
    mem_writeB(mem,MySbcB(memB,*cregsB[lastbyte&7]));
    return 6;
}
int sbcMRW10()
{
    mem_writeW(mem,MySbcW(memW,*cregsW[lastbyte&7]));
    return 6;
}
int sbcMRL20()
{
    mem_writeL(mem,MySbcL(memL,*cregsL[lastbyte&7]));
    return 10;
}
int sbcMI00()
{
    mem_writeB(mem,MySbcB(memB,readbyte()));
    return 7;
}
int sbcwMI10()
{
    mem_writeW(mem,MySbcW(memW,readword()));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// CP ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int cpRrB()
{
    MySubB(*cregsB[lastbyte&7],*regB);
    return 4;
}
int cpRrW()
{
    MySubW(*cregsW[lastbyte&7],*regW);
    return 4;
}
int cpRrL()
{
    MySubL(*cregsL[lastbyte&7],*regL);
    return 7;
}
int cpr3B()
{
    MySubB(*regB,lastbyte&7);
    return 4;
}
int cpr3W()
{
    MySubW(*regW,lastbyte&7);
    return 4;
}
int cprIB()
{
    MySubB(*regB,readbyte());
    return 4;
}
int cprIW()
{
    MySubW(*regW,readword());
    return 4;
}
int cprIL()
{
    MySubL(*regL,readlong());
    return 7;
}
int cpRMB00()
{
    MySubB(*cregsB[lastbyte&7],memB);
    return 4;
}
int cpRMW10()
{
    MySubW(*cregsW[lastbyte&7],memW);
    return 4;
}
int cpRML20()
{
    MySubL(*cregsL[lastbyte&7],memL);
    return 6;
}
int cpMRB00()
{
    MySubB(memB,*cregsB[lastbyte&7]);
    return 6;
}
int cpMRW10()
{
    MySubW(memW,*cregsW[lastbyte&7]);
    return 6;
}
int cpMRL20()
{
    MySubL(memL,*cregsL[lastbyte&7]);
    return 6;
}
int cpMI00()
{
    MySubB(memB,readbyte());
    return 6;
}
int cpwMI10()
{
    MySubW(memW,readword());
    return 6;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// INC ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int inc3rB() // INC #3,r    11001rrr 01100xxx
{
    unsigned long oldC = gen_regsSR & CF;

    *regB = MyAddB(*regB,((lastbyte&7) ? (lastbyte&7) : 8));
    gen_regsSR = (gen_regsSR & ~CF) | oldC;
    return 4;
}

int inc3rW() // INC #3,r    11011rrr 01100xxx
{
    *regW+= ((lastbyte&7) ? (lastbyte&7) : 8);
    return 4;
}

int inc3rL() // INC #3,r    11101rrr 01100xxx
{
    *regL+= ((lastbyte&7) ? (lastbyte&7) : 8);
    return 4;
}

int inc3M00() // INC #3,(mem)   10000mmm 01100xxx
{
    unsigned long oldC = gen_regsSR & CF;

    mem_writeB(mem,MyAddB(memB,((lastbyte&7) ? (lastbyte&7) : 8)));
    gen_regsSR = (gen_regsSR & ~CF) | oldC;
    return 6;
}

int incw3M10() // INCW #3,(mem)  10010mmm 01100xxx
{
    unsigned long oldC = gen_regsSR & CF;

    mem_writeW(mem,MyAddW(memW,((lastbyte&7) ? (lastbyte&7) : 8)));
    gen_regsSR = (gen_regsSR & ~CF) | oldC;
    return 6;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// DEC ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int dec3rB() // DEC #3,r    11001rrr 01101xxx
{
    unsigned long oldC = gen_regsSR & CF;

    *regB = MySubB(*regB,((lastbyte&7) ? (lastbyte&7) : 8));
    gen_regsSR = (gen_regsSR & ~CF) | oldC;
    return 4;
}

int dec3rW() // DEC #3,r    11011rrr 01101xxx
{
    *regW-= ((lastbyte&7) ? (lastbyte&7) : 8);
    return 4;
}

int dec3rL() // DEC #3,r    11101rrr 01101xxx
{
    *regL-= ((lastbyte&7) ? (lastbyte&7) : 8);
    return 5;
}

int dec3M00() // DEC #3,(mem)   10000mmm 01101xxx
{
    unsigned long oldC = gen_regsSR & CF;

    mem_writeB(mem,MySubB(memB,((lastbyte&7) ? (lastbyte&7) : 8)));
    gen_regsSR = (gen_regsSR & ~CF) | oldC;
    return 6;
}

int decw3M10() // DECW #3,(mem)  10010mmm 01101xxx
{
    unsigned long oldC = gen_regsSR & CF;

    mem_writeW(mem,MySubW(memW,((lastbyte&7) ? (lastbyte&7) : 8)));
    gen_regsSR = (gen_regsSR & ~CF) | oldC;
    return 6;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// NEG ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int negrB()
{
    *regB = MySubB(0,*regB);
    return 5;
}
int negrW()
{
    *regW = MySubW(0,*regW);
    return 5;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// EXTZ //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int extzrW()
{
    *regW&=0x00ff;
    return 4;
}
int extzrL()
{
    *regL&=0x0000ffff;
    return 4;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// EXTS //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int extsrW() // EXTS r    11011rrr 00010011
{
    if (*regW&0x80)
        *regW|=0xff00;
    else
        *regW&=0x00ff;
    return 5;
}

int extsrL() // EXTS r    11101rrr 00010011
{
    if (*regL&0x8000)
        *regL|=0xffff0000;
    else
        *regL&=0x0000ffff;
    return 5;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// DAA ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline void parityB(unsigned char j)
{
#ifdef USE_PARITY_TABLE //speed hack
    gen_regsSR = (gen_regsSR & ~VF) | parityVtable[j];
#else
    unsigned char k=0, i;

    for (i=8;i!=0;--i)
    {
        if (j&1)
            ++k;
        j >>= 1;
    }
    gen_regsSR = (gen_regsSR & ~VF) | ((k&1) ? 0 : VF);
#endif
}

inline void parityW(unsigned short j)
{
#ifdef USE_PARITY_TABLE //speed hack
    gen_regsSR = (gen_regsSR & ~VF) | (parityVtable[j>>8] ^ parityVtable[j&0xFF]);
#else
    unsigned char k=0, i;

    for (i=16;i!=0;--i)
    {
        if (j&1)
            ++k;
        j >>= 1;
    }
    gen_regsSR = (gen_regsSR & ~VF) | ((k&1) ? 0 : VF);
#endif
}

inline void parityL(unsigned long j)
{
#ifdef USE_PARITY_TABLE //speed hack
    gen_regsSR = (gen_regsSR & ~VF) | (parityVtable[(j>>24)&0xFF] ^ parityVtable[(j>>16)&0xFF] ^ parityVtable[(j>>8)&0xFF] ^ parityVtable[j&0xFF]);
#else
    unsigned char k=0, i;

    for (i=32;i!=0;--i)
    {
        if (j&1)
            ++k;
        j >>= 1;
    }
    gen_regsSR = (gen_regsSR & ~VF) | ((k&1) ? 0 : VF);
#endif
}

int daar()  // DAA r    11001rrr 00010000
{
    // 100% with hardware
    unsigned char oldB = *regB;
    unsigned char addVal = 0;
    unsigned char setCy = 0;
    unsigned char i = (*regB & 0xf0);
    unsigned char j = (*regB & 0x0f);

    if (gen_regsSR & CF)
    { // C
        if (gen_regsSR & HF)
        { // H
            addVal = 0x66;
            setCy = 1;
        }
        else
        { // !H
            if      ((j<0x0a))
            {
                addVal = 0x60;
            }
            else
            {
                addVal = 0x66;
            }
            setCy = 1;
        }
    }
    else
    { // !C
        if (gen_regsSR & HF)
        { // H
            if  (oldB<0x9A)
            {
                addVal = 0x06;
            }
            else
            {
                addVal = 0x66;
            }
        }
        else
        { // !H
            if  ((i<0x90) && (j>0x09))
            {
                addVal = 0x06;
            }
            else if ((i>0x80) && (j>0x09))
            {
                addVal = 0x66;
            }
            else if ((i>0x90) && (j<0x0a))
            {
                addVal = 0x60;
            }
        }
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|CF|VF);
    if (gen_regsSR & NF)
    { // adjust after SUB, SBC or NEG operation
        *regB = oldB - addVal;
        gen_regsSR = gen_regsSR | ((*regB>oldB || setCy) ? CF : 0);
    }
    else
    { // adjust after ADD or ADC operation
        *regB = oldB + addVal;
        gen_regsSR = gen_regsSR | ((*regB<oldB || setCy) ? CF : 0);
    }
    gen_regsSR = gen_regsSR |
                 (*regB & SF) |
                 ((*regB) ? 0 : ZF) |
                 (((oldB^addVal)^*regB) & HF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[*regB];
#else
				 ;
    // calculate parity
    parityB(*regB);
#endif
    return 6;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// PAA ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int paarW()
{
    if (*regW&1)
        *regW+=1;
    return 4;
}
int paarL()
{
    if (*regL&1)
        *regL+=1;
    return 4;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// MUL ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int mulRrB() // MUL RR,r    11001rrr 01000RRR
{
    *cregsW[(lastbyte&7)>>1] = (*cregsB[lastbyte&7]) * (*regB);
    return 18;
}

int mulRrW() // MUL RR,r    11011rrr 01000RRR
{
    *cregsL[lastbyte&7] = (*cregsW[lastbyte&7]) * (*regW);
    return 26;
}

int mulrIB() // MUL rr,#    11001rrr 00001000 xxxxxxxx
{
    if (opcode>=0xc8)
    {
        *cregsW[(opcode&7)>>1] = (*cregsB[opcode&7]) * readbyte();
    }
    else
    {
        *((unsigned short *)regB) = (*regB) * readbyte();
    }
    return 18;
}

int mulrIW() // MUL rr,#    11011rrr 00001000 xxxxxxxx xxxxxxxx
{
    *((unsigned long *)regW) = (*regW) * readword();
    return 26;
}

int mulRMB00() // MUL RR,(mem)   10000mmm 01000RRR
{
    *cregsW[(lastbyte&7)>>1] = (*cregsB[lastbyte&7]) * memB;
    return 18;
}

int mulRMW10() // MUL RR,(mem)   10010mmm 01000RRR
{
    *cregsL[lastbyte&7] = (*cregsW[lastbyte&7]) * memW;
    return 26;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// MULS //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int mulsRrB() // MULS RR,r    11001rrr 01001RRR
{
    *cregsW[(lastbyte&7)>>1] = (signed char)(*cregsB[lastbyte&7]) * (signed char)(*regB);
    return 18;
}

int mulsRrW() // MULS RR,r    11011rrr 01001RRR
{
    *cregsL[lastbyte&7] = (signed short)(*cregsW[lastbyte&7]) * (signed short)(*regW);
    return 26;
}

int mulsrIB() // MULS rr,#    11001rrr 00001001 xxxxxxxx
{
    if (opcode>=0xc8)
    {
        *(cregsW[(opcode&7)>>1]) = (signed char)(*cregsB[opcode&7]) * (signed char)readbyte();
    }
    else
    {
        *((unsigned short *)regB) = (signed char)(*regB) * (signed char)readbyte();
    }
    return 18;
}

int mulsrIW() // MULS rr,#    11011rrr 00001001 xxxxxxxx xxxxxxxx
{
    *((unsigned long *)regW) = (signed short)(*regW) * (signed short)readword();
    return 26;
}

int mulsRMB00() // MULS RR,(mem)  10000mmm 01001RRR
{
    *cregsW[(lastbyte&7)>>1] = (signed char)(*cregsB[lastbyte&7]) * (signed char)memB;
    return 18;
}

int mulsRMW10() // MULS RR,(mem)  10010mmm 01001RRR
{
    *cregsL[lastbyte&7] = (signed short)(*cregsW[lastbyte&7]) * (signed short)memW;
    return 26;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// DIV ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned short myDivB(unsigned short i, unsigned char j)
{
    if (!j)
    {
        gen_regsSR|= VF;
        return (i<<8) | ((i>>8)^0xFF);
    }
/*
    ldiv_t res;
    if (i >= (0x0200 * j))
    {
        int diff = i - (0x0200 * j);
        int range = 256 - j;
        res = ldiv(diff, range);
        res.quot = 0x1FF - res.quot;
        res.rem = res.rem + j;
    }
    else
    {
        res = ldiv(i,j);
    }
*/
    ldiv_t res = ldiv(i,j);
    if (res.quot>0xFF)
        gen_regsSR|= VF;
    else
        gen_regsSR&= ~VF;
    return ((unsigned short)(res.quot & 0xFF)) | ((unsigned short)((res.rem & 0xFF) << 8));
}

inline unsigned int myDivW(unsigned int i, unsigned short j)
{
    if (!j)
    {
        gen_regsSR|= VF;
        return (i<<16) | ((i>>16)^0xFFFF);
    }
/* PacMan fix : when j>=128 -> overflow
    ldiv_t res;
    if (i >= (0x02000000 * (unsigned int)j))
    {
        int diff = i - (0x02000000 * j);
        int range = 0x1000000 - j;
        res = ldiv(diff, range);
        res.quot = 0x1FFFFFF - res.quot;
        res.rem = res.rem + j;
    }
    else
    {
        res = ldiv(i,j);
    }
*/
    ldiv_t res = ldiv(i,j);
    if (res.quot>0xFFFF)
        gen_regsSR|= VF;
    else
        gen_regsSR&= ~VF;
    return (res.quot & 0xFFFF) | ((res.rem & 0xFFFF) << 16);
}

int divRrB() // DIV RR,r    11001rrr 01010RRR
{
    *cregsW[(lastbyte&7)>>1] = myDivB(*cregsW[(lastbyte&7)>>1], *regB);
    return 22;
}

int divRrW() // DIV RR,r    11011rrr 01010RRR
{
    *cregsL[lastbyte&7] = myDivW(*cregsL[lastbyte&7], *regW);
    return 30;
}

int divrIB() // DIV rr,#    11001rrr 00001010 xxxxxxxx
{
    unsigned char i = readbyte();

    if (opcode>=0xc8)
    {
        *cregsW[(opcode&7)>>1] = myDivB(*cregsW[(opcode&7)>>1], i);
    }
    else
    {
        *((unsigned short *)regB) = myDivB(*(unsigned short *)regB, i);
    }
    return 22;
}

int divrIW() // DIV rr,#    11011rrr 00001010 xxxxxxxx xxxxxxxx
{
    unsigned short i = readword();

    *((unsigned long *)regW) = myDivW(*(unsigned long *)regW, i);
    return 30;
}

int divRMB00() // DIV RR,(mem)   10000mmm 01010RRR
{
    unsigned char i = memB;

    *cregsW[(lastbyte&7)>>1] = myDivB(*cregsW[(lastbyte&7)>>1], i);
    return 22;
}

int divRMW10() // DIV RR,(mem)   10010mmm 01010RRR
{
    unsigned short i = memW;

    *cregsL[lastbyte&7] = myDivW(*cregsL[lastbyte&7], i);
    return 30;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// DIVS //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned short myDivsB(signed short i, signed char j)
{
    if (!j)
    {
        gen_regsSR|= VF;
        if (i<1)
            return (i<<8) | (i^0xFF);
        else
            return (i<<8);
    }

    ldiv_t res = ldiv(i,j);
    if (res.quot>0xFF)
        gen_regsSR|= VF;
    else
        gen_regsSR&= ~VF;
    return ((unsigned short)(res.quot & 0xFF)) | ((unsigned short)((res.rem & 0xFF) << 8));
}

inline unsigned int myDivsW(signed int i, signed short j)
{
    if (!j)
    {
        gen_regsSR|= VF;
        return (i<<16) | (i^0xFFFF);
    }

    ldiv_t res = ldiv(i,j);
    if (res.quot>0xFFFF)
        gen_regsSR|= VF;
    else
        gen_regsSR&= ~VF;
    return (res.quot & 0xFFFF) | ((res.rem & 0xFFFF) << 16);
}

int divsRrB() // DIVS RR,r    11001rrr 01011RRR
{
    *cregsW[(lastbyte&7)>>1] = myDivsB((signed short)(*cregsW[(lastbyte&7)>>1]), (signed char)(*regB));
    return 24;
}

int divsRrW() // DIVS RR,r    11011rrr 01011RRR
{
    *cregsL[lastbyte&7] = myDivsW((signed long)(*cregsL[lastbyte&7]), (signed short)(*regW));
    return 32;
}

int divsrIB() // DIVS rr,#    11001rrr 00001011 xxxxxxxx
{
    signed char i = readbyte();

    if (opcode>=0xc8)
    {
        *cregsW[(opcode&7)>>1] = myDivsB((signed short)(*cregsW[(opcode&7)>>1]), i);
    }
    else
    {
        *((unsigned short *)regB) = myDivsB((*(signed short *)regB), i);
    }
    return 24;
}

int divsrIW() // DIVS rr,#    11011rrr 00001011 xxxxxxxx xxxxxxxx
{
    signed short i = readword();

    *((unsigned long *)regW) = myDivsW((*(signed long *)regW), i);
    return 32;
}

int divsRMB00() // DIVS RR,(mem)  10000mmm 01011RRR
{
    signed char i = memB;

    *cregsW[(lastbyte&7)>>1] = myDivsB((signed short)(*cregsW[(lastbyte&7)>>1]), i);
    return 24;
}

int divsRMW10() // DIVS RR,(mem)  10010mmm 01011RRR
{
    signed short i = memW;

    *cregsL[lastbyte&7] = myDivsW((signed long)(*cregsL[lastbyte&7]), i);
    return 32;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// MULA //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int mular() // MULA rr     11011rrr 00011001
{
    unsigned long *p;

    p = cregsL[opcode&7];
    *p+= (signed long)(((signed short)mem_readW(*cregsL[2]))*((signed short)mem_readW(*cregsL[3])));
    *cregsL[3]-=2; // XHL
    gen_regsSR = gen_regsSR & ~(SF|ZF|VF);
    gen_regsSR = gen_regsSR |
                 ((*p >> 24) & SF) |
                 ((*p) ? 0 : ZF);
    return 31;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// MINC //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int minc1() // MINC1 #,r    11011rrr 00111000 xxxxxxxx xxxxxxxx
{
    unsigned short i;

    i = readword();
    *regW = (((*regW & i) == i) ? *regW - i : *regW + 1);
    return 8;
}

int minc2() // MINC2 #,r    11011rrr 00111001 xxxxxxxx xxxxxxxx
{
    unsigned short i;

    i = readword();
    *regW = (((*regW & i) == i) ? *regW - i : *regW + 2);
    return 8;
}

int minc4() // MINC4 #,r    11011rrr 00111010 xxxxxxxx xxxxxxxx
{
    unsigned short i;

    i = readword();
    *regW = (((*regW & i) == i) ? *regW - i : *regW + 4);
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// MDEC //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int mdec1() // MDEC1 #,r    11011rrr 00111100 xxxxxxxx xxxxxxxx
{
    unsigned short i;

    i = readword();
    *regW = (((*regW & i) == i) ? *regW + i : *regW - 1);
    return 7;
}

int mdec2() // MDEC2 #,r    11011rrr 00111101 xxxxxxxx xxxxxxxx
{
    unsigned short i;

    i = readword();
    *regW = (((*regW & i) == i) ? *regW + i : *regW - 2);
    return 7;
}

int mdec4() // MDEC4 #,r    11011rrr 00111110 xxxxxxxx xxxxxxxx
{
    unsigned short i;

    i = readword();
    *regW = (((*regW & i) == i) ? *regW + i : *regW - 4);
    return 7;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// AND ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MyAndB(unsigned char i, unsigned char j)
{
    // 100% correct
    i&= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? HF : HF|ZF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[i];
#else
				 ;
    parityB(i);
#endif
    return i;
}

inline unsigned short MyAndW(unsigned short i, unsigned short j)
{
    i&= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? HF : HF|ZF);
    parityW(i);
    return i;
}

inline unsigned long MyAndL(unsigned long i, unsigned long j)
{
    i&= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? HF : HF|ZF);
    return i;
}

int andRrB()
{
    *cregsB[lastbyte&7] = MyAndB(*cregsB[lastbyte&7],*regB);
    return 4;
}
int andRrW()
{
    *cregsW[lastbyte&7] = MyAndW(*cregsW[lastbyte&7],*regW);
    return 4;
}
int andRrL()
{
    *cregsL[lastbyte&7] = MyAndL(*cregsL[lastbyte&7],*regL);
    return 7;
}
int andrIB()
{
    *regB = MyAndB(*regB,readbyte());
    return 4;
}
int andrIW()
{
    *regW = MyAndW(*regW,readword());
    return 4;
}
int andrIL()
{
    *regL = MyAndL(*regL,readlong());
    return 7;
}
int andRMB00()
{
    *cregsB[lastbyte&7] = MyAndB(*cregsB[lastbyte&7],memB);
    return 4;
}
int andRMW10()
{
    *cregsW[lastbyte&7] = MyAndW(*cregsW[lastbyte&7],memW);
    return 4;
}
int andRML20()
{
    *cregsL[lastbyte&7] = MyAndL(*cregsL[lastbyte&7],memL);
    return 6;
}
int andMRB00()
{
    mem_writeB(mem,MyAndB(memB,*cregsB[lastbyte&7]));
    return 6;
}
int andMRW10()
{
    mem_writeW(mem,MyAndW(memW,*cregsW[lastbyte&7]));
    return 6;
}
int andMRL20()
{
    mem_writeL(mem,MyAndL(memL,*cregsL[lastbyte&7]));
    return 10;
}
int andMI00()
{
    mem_writeB(mem,MyAndB(memB,readbyte()));
    return 7;
}
int andwMI10()
{
    mem_writeW(mem,MyAndW(memW,readword()));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// OR ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MyOrB(unsigned char i, unsigned char j)
{
    // 100% correct
    i|= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? 0 : ZF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[i];
#else
				 ;
    parityB(i);
#endif

    return i;
}

inline unsigned short MyOrW(unsigned short i, unsigned short j)
{
    i|= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? 0 : ZF);
    parityW(i);
    return i;
}

inline unsigned long MyOrL(unsigned long i, unsigned long j)
{
    i|= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? 0 : ZF);
    return i;
}

int orRrB()
{
    *cregsB[lastbyte&7] = MyOrB(*cregsB[lastbyte&7],*regB);
    return 4;
}
int orRrW()
{
    *cregsW[lastbyte&7] = MyOrW(*cregsW[lastbyte&7],*regW);
    return 4;
}
int orRrL()
{
    *cregsL[lastbyte&7] = MyOrL(*cregsL[lastbyte&7],*regL);
    return 7;
}
int orrIB()
{
    *regB = MyOrB(*regB,readbyte());
    return 4;
}
int orrIW()
{
    *regW = MyOrW(*regW,readword());
    return 4;
}
int orrIL()
{
    *regL = MyOrL(*regL,readlong());
    return 7;
}
int orRMB00()
{
    *cregsB[lastbyte&7] = MyOrB(*cregsB[lastbyte&7],memB);
    return 4;
}
int orRMW10()
{
    *cregsW[lastbyte&7] = MyOrW(*cregsW[lastbyte&7],memW);
    return 4;
}
int orRML20()
{
    *cregsL[lastbyte&7] = MyOrL(*cregsL[lastbyte&7],memL);
    return 6;
}
int orMRB00()
{
    mem_writeB(mem,MyOrB(memB,*cregsB[lastbyte&7]));
    return 6;
}
int orMRW10()
{
    mem_writeW(mem,MyOrW(memW,*cregsW[lastbyte&7]));
    return 6;
}
int orMRL20()
{
    mem_writeL(mem,MyOrL(memL,*cregsL[lastbyte&7]));
    return 10;
}
int orMI00()
{
    mem_writeB(mem,MyOrB(memB,readbyte()));
    return 7;
}
int orwMI10()
{
    mem_writeW(mem,MyOrW(memW,readword()));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// XOR ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MyXorB(unsigned char i, unsigned char j)
{
    // 100% correct
    i^= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? 0 : ZF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[i];
#else
				 ;
    parityB(i);
#endif

    return i;
}

inline unsigned short MyXorW(unsigned short i, unsigned short j)
{
    i^= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? 0 : ZF);
    parityW(i);
    return i;
}

inline unsigned long MyXorL(unsigned long i, unsigned long j)
{
    i^= j;
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? 0 : ZF);
    return i;
}

int xorRrB()
{
    *cregsB[lastbyte&7] = MyXorB(*cregsB[lastbyte&7],*regB);
    return 4;
}
int xorRrW()
{
    *cregsW[lastbyte&7] = MyXorW(*cregsW[lastbyte&7],*regW);
    return 4;
}
int xorRrL()
{
    *cregsL[lastbyte&7] = MyXorL(*cregsL[lastbyte&7],*regL);
    return 7;
}
int xorrIB()
{
    *regB = MyXorB(*regB,readbyte());
    return 4;
}
int xorrIW()
{
    *regW = MyXorW(*regW,readword());
    return 4;
}
int xorrIL()
{
    *regL = MyXorL(*regL,readlong());
    return 7;
}
int xorRMB00()
{
    *cregsB[lastbyte&7] = MyXorB(*cregsB[lastbyte&7],memB);
    return 4;
}
int xorRMW10()
{
    *cregsW[lastbyte&7] = MyXorW(*cregsW[lastbyte&7],memW);
    return 4;
}
int xorRML20()
{
    *cregsL[lastbyte&7] = MyXorL(*cregsL[lastbyte&7],memL);
    return 6;
}
int xorMRB00()
{
    mem_writeB(mem,MyXorB(memB,*cregsB[lastbyte&7]));
    return 6;
}
int xorMRW10()
{
    mem_writeW(mem,MyXorW(memW,*cregsW[lastbyte&7]));
    return 6;
}
int xorMRL20()
{
    mem_writeL(mem,MyXorL(memL,*cregsL[lastbyte&7]));
    return 10;
}
int xorMI00()
{
    mem_writeB(mem,MyXorB(memB,readbyte()));
    return 7;
}
int xorwMI10()
{
    mem_writeW(mem,MyXorW(memW,readword()));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// CPL ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int cplrB()
{
    *regB = ~*regB;
    gen_regsSR|= HF|NF;
    return 4;
}
int cplrW()
{
    *regW = ~*regW;
    gen_regsSR|= HF|NF;
    return 4;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// LDCF //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

const unsigned short power2[16] =
    {
        0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
        0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000
    };

#define LDCF(A,B) if (A & power2[B]) gen_regsSR|= CF; \
    else    gen_regsSR&= ~CF;

int ldcf4rB()
{
    LDCF(*regB,readbyte());
    return 4;
}
int ldcf4rW()
{
    LDCF(*regW,readbyte());
    return 4;
}
int ldcfArB()
{
    LDCF(*regB,*cregsB[1]);
    return 4;
}
int ldcfArW()
{
    LDCF(*regW,*cregsB[1]);
    return 4;
}
int ldcf3M30()
{
    LDCF(mem_readB(mem),lastbyte&7);
    return 8;
}
int ldcfAM30()
{
    LDCF(mem_readB(mem),*cregsB[1]);
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// STCF //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int stcf4rB() // STCF #4,r   11001rrr 00100100 0000xxxx
{
    if (gen_regsSR & CF)
        *regB|= power2[readbyte()];
    else
        *regB&= ~power2[readbyte()];
    return 4;
}

int stcf4rW() // STCF #4,r   11011rrr 00100100 0000xxxx
{
    if (gen_regsSR & CF)
        *regW|= power2[readbyte()];
    else
        *regW&= ~power2[readbyte()];
    return 4;
}

int stcfArB() // STCF A,r    11001rrr 00101100
{
    if (gen_regsSR & CF)
        *regB|= power2[*cregsB[1]];
    else
        *regB&= ~power2[*cregsB[1]];
    return 4;
}

int stcfArW() // STCF A,r    11011rrr 00101100
{
    if (gen_regsSR & CF)
        *regW|= power2[*cregsB[1]];
    else
        *regW&= ~power2[*cregsB[1]];
    return 4;
}

int stcf3M30() // STCF #3,(mem)  10110mmm 10100xxx
{
    if (gen_regsSR & CF)
        mem_writeB(mem,mem_readB(mem) | power2[lastbyte&7]);
    else
        mem_writeB(mem,mem_readB(mem) & ~power2[lastbyte&7]);
    return 8;
}

int stcfAM30() // STCF A,(mem)   10110mmm 00101100
{
    if (gen_regsSR & CF)
        mem_writeB(mem,mem_readB(mem) | power2[*cregsB[1]]);
    else
        mem_writeB(mem,mem_readB(mem) & ~power2[*cregsB[1]]);
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// ANDCF /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define ANDCF(A,B) if ((A & power2[B]) && (gen_regsSR & CF)) gen_regsSR|= CF; \
    else          gen_regsSR&= ~CF;

int andcf4rB()
{
    ANDCF(*regB, readbyte());
    return 4;
}
int andcf4rW()
{
    ANDCF(*regW, readbyte());
    return 4;
}
int andcfArB()
{
    ANDCF(*regB, *cregsB[1]);
    return 4;
}
int andcfArW()
{
    ANDCF(*regW, *cregsB[1]);
    return 4;
}
int andcf3M30()
{
    ANDCF(mem_readB(mem), lastbyte&7);
    return 8;
}
int andcfAM30()
{
    ANDCF(mem_readB(mem), *cregsB[1]);
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// ORCF //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define ORCF(A,B) gen_regsSR|= ((A >> B) & CF);

int orcf4rB()
{
    ORCF(*regB, readbyte());
    return 4;
}
int orcf4rW()
{
    ORCF(*regW, readbyte());
    return 4;
}
int orcfArB()
{
    ORCF(*regB, *cregsB[1]);
    return 4;
}
int orcfArW()
{
    ORCF(*regW, *cregsB[1]);
    return 4;
}
int orcf3M30()
{
    ORCF(mem_readB(mem), (lastbyte&7));
    return 8;
}
int orcfAM30()
{
    ORCF(mem_readB(mem), *cregsB[1]);
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// XORCF /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define XORCF(A,B) gen_regsSR^= ((A >> B) & CF);

int xorcf4rB()
{
    XORCF(*regB, readbyte());
    return 4;
}
int xorcf4rW()
{
    XORCF(*regW, readbyte());
    return 4;
}
int xorcfArB()
{
    XORCF(*regB, *cregsB[1]);
    return 4;
}
int xorcfArW()
{
    XORCF(*regW, *cregsB[1]);
    return 4;
}
int xorcf3M30()
{
    XORCF(mem_readB(mem), (lastbyte&7));
    return 8;
}
int xorcfAM30()
{
    XORCF(mem_readB(mem), *cregsB[1]);
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// xCF ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int rcf()
{
    gen_regsSR&= ~(HF|NF|CF);
    return 2;
}
int scf()
{
    gen_regsSR = (gen_regsSR & ~(HF|NF)) | CF;
    return 2;
}
int ccf()
{
    gen_regsSR = (gen_regsSR & ~NF) ^ CF;
    return 2;
}
int zcf()
{
    gen_regsSR = (gen_regsSR & ~(NF|CF)) | ((gen_regsSR & ZF) ? 0 : CF);
    return 2;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// BIT ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define BIT(A,B) gen_regsSR = (gen_regsSR & ~(ZF|NF)) | ((A & power2[B]) ? HF : HF|ZF);

int bit4rB()
{
    BIT(*regB, readbyte());
    return 4;
}
int bit4rW()
{
    BIT(*regW, readbyte());
    return 4;
}
int bit3M30()
{
    BIT(mem_readB(mem), lastbyte&7);
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// RES ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int res4rB()
{
    *regB&= ~power2[readbyte()];
    return 4;
}
int res4rW()
{
    *regW&= ~power2[readbyte()];
    return 4;
}
int res3M30()
{
    mem_writeB(mem,mem_readB(mem) & ~power2[lastbyte&7]);
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// SET ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int set4rB()
{
    *regB|= power2[readbyte()];
    return 4;
}
int set4rW()
{
    *regW|= power2[readbyte()];
    return 4;
}

//Flavor
int set3M30()
{
    mem_writeB(mem,  mem_readB(mem) | power2[lastbyte&7]);
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// CHG ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int chg4rB()
{
    *regB^= power2[readbyte()];
    return 4;
}
int chg4rW()
{
    *regW^= power2[readbyte()];
    return 4;
}
int chg3M30()
{
    mem_writeB(mem,mem_readB(mem) ^ power2[lastbyte&7]);
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// TSET //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int tset4rB() // TSET #4,r   11001rrr 00110100 0000xxxx
{
    unsigned char i = readbyte();

    gen_regsSR = gen_regsSR & ~(ZF|NF);
    gen_regsSR = gen_regsSR |
                 ((*regB&power2[i]) ? HF : HF|ZF);
    *regB |= power2[i];
    return 6;
}

int tset4rW() // TSET #4,r   11011rrr 00110100 0000xxxx
{
    unsigned char i = readbyte();

    gen_regsSR = gen_regsSR & ~(ZF|NF);
    gen_regsSR = gen_regsSR |
                 ((*regW&power2[i]) ? HF : HF|ZF);
    *regW |= power2[i];
    return 6;
}

int tset3M30() // TSET #3,(mem)  10110mmm 10101xxx
{
    unsigned char i = mem_readB(mem);

    gen_regsSR = gen_regsSR & ~(ZF|NF);
    gen_regsSR = gen_regsSR |
                 ((i&power2[i]) ? HF : HF|ZF);
    mem_writeB(mem,i | power2[lastbyte&7]);
    return 10;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// BS1 ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int bs1b() // BS1B A,r     11011rrr 00001111
{
    unsigned short i = *regW;

    if (i==0)
    {
        gen_regsSR |= VF;
        return 4;
    }

    unsigned char ret = 15;

    gen_regsSR &= ~VF;

    while (i<0x8000)
    {
        i = i<<1;
        ret -= 1;
    }

    *cregsB[1] = ret;
    return 4;
}

int bs1f() // BS1F A,r     11011rrr 00001110
{
    unsigned short i = *regW;

    if (i==0)
    {
        gen_regsSR|= VF;
        return 4;
    }

    unsigned char ret = 0;

    gen_regsSR&= ~VF;


    while (!(i&1))
    {
        i = i>>1;
        ret+=1;
    }

    *cregsB[1] = ret;

    return 4;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// MISC //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int nop()
{
    return 1;
}
int normal()
{
    gen_regsSR = gen_regsSR & 0x7fff;
    return 4;
}
int tmax()
{
    gen_regsSR = gen_regsSR | 0x0400;
    return 4;
}
int ei()
{
    gen_regsSR = (gen_regsSR & 0x8fff) | (readbyte()<<12);
    return 5;
}

int swi() // SWI #3     11111xxx
{
    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
    tlcsFastMemWriteW(gen_regsXSP-=2,gen_regsSR);
    // SYSM = 1;
    gen_regsPC = mem_readL(0x00FFFF00 + ((opcode&7)<<2)) & 0x00ffffff;
    my_pc = get_address(gen_regsPC);

    return 16;
}

int halt()
{
    --gen_regsPC;
    --my_pc;
    return 8;
}

int link() // LINK r,d16    11101rrr 00001100 xxxxxxxx xxxxxxxx
{
    tlcsFastMemWriteL(gen_regsXSP-=4,*regL);
    *regL = gen_regsXSP;
    gen_regsXSP+= (signed short)readword();
    return 10;
}

int unlk() // UNLK r     11101rrr 00001101
{
    gen_regsXSP = *regL;
    *regL = mem_readL(gen_regsXSP);
    gen_regsXSP+=4;
    return 8;
}

int ldf() // LDF #3     00010111 00000xxx
{
    gen_regsSR = (gen_regsSR & 0xf8ff) | ((readbyte() & 7) << 8);
    set_cregs();
    return 2;
}

int incf() // INCF      00001100
{
    unsigned long i = gen_regsSR & 0x0700;

    i = (i + 0x0100) & 0x0700;
    // for MAX mode
    i = i & 0x0300;
    gen_regsSR = (gen_regsSR & 0xf8ff) | i;
    set_cregs();
    return 2;
}

int decf() // DECF      00001101
{
    unsigned long i = gen_regsSR & 0x0700;

    i = (i - 0x0100) & 0x0700;
    // for MAX mode
    i = i & 0x0300;
    gen_regsSR = (gen_regsSR & 0xf8ff) | i;
    set_cregs();
    return 2;
}

int sccB0()
{
    *regB = valCond0();
    return 6;
}
int sccB1()
{
    *regB = valCond1();
    return 6;
}
int sccB2()
{
    *regB = valCond2();
    return 6;
}
int sccB3()
{
    *regB = valCond3();
    return 6;
}
int sccB4()
{
    *regB = valCond4();
    return 6;
}
int sccB5()
{
    *regB = valCond5();
    return 6;
}
int sccB6()
{
    *regB = valCond6();
    return 6;
}
int sccB7()
{
    *regB = valCond7();
    return 6;
}
int sccB8()
{
    *regB = valCond8();
    return 6;
}
int sccB9()
{
    *regB = valCond9();
    return 6;
}
int sccBA()
{
    *regB = valCondA();
    return 6;
}
int sccBB()
{
    *regB = valCondB();
    return 6;
}
int sccBC()
{
    *regB = valCondC();
    return 6;
}
int sccBD()
{
    *regB = valCondD();
    return 6;
}
int sccBE()
{
    *regB = valCondE();
    return 6;
}
int sccBF()
{
    *regB = valCondF();
    return 6;
}

int sccW0()
{
    *regW = valCond0();
    return 6;
}
int sccW1()
{
    *regW = valCond1();
    return 6;
}
int sccW2()
{
    *regW = valCond2();
    return 6;
}
int sccW3()
{
    *regW = valCond3();
    return 6;
}
int sccW4()
{
    *regW = valCond4();
    return 6;
}
int sccW5()
{
    *regW = valCond5();
    return 6;
}
int sccW6()
{
    *regW = valCond6();
    return 6;
}
int sccW7()
{
    *regW = valCond7();
    return 6;
}
int sccW8()
{
    *regW = valCond8();
    return 6;
}
int sccW9()
{
    *regW = valCond9();
    return 6;
}
int sccWA()
{
    *regW = valCondA();
    return 6;
}
int sccWB()
{
    *regW = valCondB();
    return 6;
}
int sccWC()
{
    *regW = valCondC();
    return 6;
}
int sccWD()
{
    *regW = valCondD();
    return 6;
}
int sccWE()
{
    *regW = valCondE();
    return 6;
}
int sccWF()
{
    *regW = valCondF();
    return 6;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// LDC/X /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int ldccrB()
{
    ldcRegs[readbyte()] = *regB;
    return 8;
}
int ldccrW()
{
    *((unsigned short *)&ldcRegs[readbyte()]) = *regW;
    return 8;
}
int ldccrL()
{
    *((unsigned long *)&ldcRegs[readbyte()]) = *regL;
    return 8;
}
int ldcrcB()
{
    *regB = ldcRegs[readbyte()];
    return 8;
}
int ldcrcW()
{
    *regW = *((unsigned short *)&ldcRegs[readbyte()]);
    return 8;
}
int ldcrcL()
{
    *regL = *((unsigned long *)&ldcRegs[readbyte()]);
    return 8;
}
int ldx()  // LDX (#8),#   11110111 00000000 xxxxxxxx 00000000 xxxxxxxx 00000000
{
    unsigned char num8, data;
    readbyte();
    num8 = readbyte();
    readbyte();
    data = readbyte();
    readbyte();
    mem_writeB(num8,data);
    return 9;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// RLC ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MyRlcB(unsigned char i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (i&0x80)
            i = (i << 1)|1;
        else
            i <<= 1;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? 0 : ZF) |
                 (i & CF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[i];
#else
				 ;
    parityB(i);
#endif

    return i;
}

inline unsigned short MyRlcW(unsigned short i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (i&0x8000)
            i = (i << 1)|1;
        else
            i <<= 1;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? 0 : ZF) |
                 (i & CF);
    parityW(i);
    return i;
}

inline unsigned long MyRlcL(unsigned long i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (i&0x80000000)
            i = (i << 1)|1;
        else
            i <<= 1;
        state+= 2;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? 0 : ZF) |
                 (i & CF);
    parityL(i);
    return i;
}

int rlc4rB()
{
    *regB = MyRlcB(*regB,readbyte());
    return 6;
}
int rlc4rW()
{
    *regW = MyRlcW(*regW,readbyte());
    return 6;
}
int rlc4rL()
{
    *regL = MyRlcL(*regL,readbyte());
    return 8;
}
int rlcArB()
{
    *regB = MyRlcB(*regB,*cregsB[1] & 0x0F);
    return 6;
}
int rlcArW()
{
    *regW = MyRlcW(*regW,*cregsB[1] & 0x0F);
    return 6;
}
int rlcArL()
{
    *regL = MyRlcL(*regL,*cregsB[1] & 0x0F);
    return 8;
}
int rlcM00()
{
    mem_writeB(mem,MyRlcB(memB,1));
    return 8;
}
int rlcwM10()
{
    mem_writeW(mem,MyRlcW(memW,1));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// RRC ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MyRrcB(unsigned char i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (i&1)
            i = (i >> 1)|0x80;
        else
            i >>= 1;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 ((i & SF) ? SF|CF : 0) |
                 ((i) ? 0 : ZF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[i];
#else
				 ;
    parityB(i);
#endif

    return i;
}

inline unsigned short MyRrcW(unsigned short i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (i&1)
            i = (i >> 1)|0x8000;
        else
            i >>= 1;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 (((i>>8) & 0x80) ? SF|CF : 0) |
                 ((i) ? 0 : ZF);
    parityW(i);
    return i;
}

inline unsigned long MyRrcL(unsigned long i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (i&1)
            i = (i >> 1)|0x80000000;
        else
            i >>= 1;
        state+= 2;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 (((i>>24) & 0x80) ? SF|CF : 0) |
                 ((i) ? 0 : ZF);
    parityL(i);
    return i;
}

int rrc4rB()
{
    *regB = MyRrcB(*regB,readbyte());
    return 6;
}
int rrc4rW()
{
    *regW = MyRrcW(*regW,readbyte());
    return 6;
}
int rrc4rL()
{
    *regL = MyRrcL(*regL,readbyte());
    return 8;
}
int rrcArB()
{
    *regB = MyRrcB(*regB,*cregsB[1] & 0x0F);
    return 6;
}
int rrcArW()
{
    *regW = MyRrcW(*regW,*cregsB[1] & 0x0F);
    return 6;
}
int rrcArL()
{
    *regL = MyRrcL(*regL,*cregsB[1] & 0x0F);
    return 8;
}
int rrcM00()
{
    mem_writeB(mem,MyRrcB(memB,1));
    return 8;
}
int rrcwM10()
{
    mem_writeW(mem,MyRrcW(memW,1));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// RL ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MyRlB(unsigned char i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (i&0x80)
        {
            i = (i << 1)|(unsigned char)(gen_regsSR & CF);
            gen_regsSR|= CF;
        }
        else
        {
            i = (i << 1)|(unsigned char)(gen_regsSR & CF);
            gen_regsSR&= ~CF;
        }
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? 0 : ZF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[i];
#else
				 ;
    parityB(i);
#endif

    return i;
}

inline unsigned short MyRlW(unsigned short i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (i&0x8000)
        {
            i = (i << 1)|(unsigned char)(gen_regsSR & CF);
            gen_regsSR|= CF;
        }
        else
        {
            i = (i << 1)|(unsigned char)(gen_regsSR & CF);
            gen_regsSR&= ~CF;
        }
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? 0 : ZF);
    parityW(i);
    return i;
}

inline unsigned long MyRlL(unsigned long i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (i&0x80000000)
        {
            i = (i << 1)|(gen_regsSR & CF);
            gen_regsSR|= CF;
        }
        else
        {
            i = (i << 1)|(gen_regsSR & CF);
            gen_regsSR&= ~CF;
        }
        state+= 2;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? 0 : ZF);
    parityL(i);
    return i;
}

int rl4rB()
{
    *regB = MyRlB(*regB,readbyte());
    return 6;
}
int rl4rW()
{
    *regW = MyRlW(*regW,readbyte());
    return 6;
}
int rl4rL()
{
    *regL = MyRlL(*regL,readbyte());
    return 8;
}
int rlArB()
{
    *regB = MyRlB(*regB,*cregsB[1] & 0x0F);
    return 6;
}
int rlArW()
{
    *regW = MyRlW(*regW,*cregsB[1] & 0x0F);
    return 6;
}
int rlArL()
{
    *regL = MyRlL(*regL,*cregsB[1] & 0x0F);
    return 8;
}
int rlM00()
{
    mem_writeB(mem,MyRlB(memB,1));
    return 8;
}
int rlwM10()
{
    mem_writeW(mem,MyRlW(memW,1));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// RR ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MyRrB(unsigned char i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (gen_regsSR & CF)
        {
            gen_regsSR = (gen_regsSR & ~CF) | (i & CF);
            i = (i >> 1)|0x80;
        }
        else
        {
            gen_regsSR = (gen_regsSR & ~CF) | (i & CF);
            i >>= 1;
        }
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? 0 : ZF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[i];
#else
				 ;
    parityB(i);
#endif

    return i;
}

inline unsigned short MyRrW(unsigned short i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (gen_regsSR & CF)
        {
            gen_regsSR = (gen_regsSR & ~CF) | (i & CF);
            i = (i >> 1)|0x8000;
        }
        else
        {
            gen_regsSR = (gen_regsSR & ~CF) | (i & CF);
            i >>= 1;
        }
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? 0 : ZF);
    parityW(i);
    return i;
}

inline unsigned long MyRrL(unsigned long i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (gen_regsSR & CF)
        {
            gen_regsSR = (gen_regsSR & ~CF) | (i & CF);
            i = (i >> 1)|0x80000000;
        }
        else
        {
            gen_regsSR = (gen_regsSR & ~CF) | (i & CF);
            i >>= 1;
        }
        state+= 2;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? 0 : ZF);
    parityL(i);
    return i;
}

int rr4rB()
{
    *regB = MyRrB(*regB,readbyte());
    return 6;
}
int rr4rW()
{
    *regW = MyRrW(*regW,readbyte());
    return 6;
}
int rr4rL()
{
    *regL = MyRrL(*regL,readbyte());
    return 8;
}
int rrArB()
{
    *regB = MyRrB(*regB,*cregsB[1] & 0x0F);
    return 6;
}
int rrArW()
{
    *regW = MyRrW(*regW,*cregsB[1] & 0x0F);
    return 6;
}
int rrArL()
{
    *regL = MyRrL(*regL,*cregsB[1] & 0x0F);
    return 8;
}
int rrM00()
{
    mem_writeB(mem,MyRrB(memB,1));
    return 8;
}
int rrwM10()
{
    mem_writeW(mem,MyRrW(memW,1));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// SLA ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MySlaB(unsigned char i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (i&0x80)
            gen_regsSR|= CF;
        else
            gen_regsSR&= ~CF;
        i <<= 1;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? 0 : ZF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[i];
#else
				 ;
    parityB(i);
#endif

    return i;
}

inline unsigned short MySlaW(unsigned short i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (i&0x8000)
            gen_regsSR|= CF;
        else
            gen_regsSR&= ~CF;
        i <<= 1;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? 0 : ZF);
    parityW(i);
    return i;
}

inline unsigned long MySlaL(unsigned long i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        if (i&0x80000000)
            gen_regsSR|= CF;
        else
            gen_regsSR&= ~CF;
        i <<= 1;
        state+= 2;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? 0 : ZF);
    parityL(i);
    return i;
}

int sla4rB()
{
    *regB = MySlaB(*regB,readbyte());
    return 6;
}
int sla4rW()
{
    *regW = MySlaW(*regW,readbyte());
    return 6;
}
int sla4rL()
{
    *regL = MySlaL(*regL,readbyte());
    return 8;
}
int slaArB()
{
    *regB = MySlaB(*regB,*cregsB[1] & 0x0F);
    return 6;
}
int slaArW()
{
    *regW = MySlaW(*regW,*cregsB[1] & 0x0F);
    return 6;
}
int slaArL()
{
    *regL = MySlaL(*regL,*cregsB[1] & 0x0F);
    return 8;
}
int slaM00()
{
    mem_writeB(mem,MySlaB(memB,1));
    return 8;
}
int slawM10()
{
    mem_writeW(mem,MySlaW(memW,1));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// SRA ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MySraB(unsigned char i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        gen_regsSR = (gen_regsSR & ~CF) | (i & CF);
        if (i&0x80)
            i = (i >> 1)|0x80;
        else
            i >>= 1;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? 0 : ZF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[i];
#else
				 ;
    parityB(i);
#endif

    return i;
}

inline unsigned short MySraW(unsigned short i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        gen_regsSR = (gen_regsSR & ~CF) | (i & CF);
        if (i&0x8000)
            i = (i >> 1)|0x8000;
        else
            i >>= 1;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? 0 : ZF);
    parityW(i);
    return i;
}

inline unsigned long MySraL(unsigned long i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        gen_regsSR = (gen_regsSR & ~CF) | (i & CF);
        if (i&0x80000000)
            i = (i >> 1)|0x80000000;
        else
            i >>= 1;
        state+= 2;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? 0 : ZF);
    parityL(i);
    return i;
}

int sra4rB()
{
    *regB = MySraB(*regB,readbyte());
    return 6;
}
int sra4rW()
{
    *regW = MySraW(*regW,readbyte());
    return 6;
}
int sra4rL()
{
    *regL = MySraL(*regL,readbyte());
    return 8;
}
int sraArB()
{
    *regB = MySraB(*regB,*cregsB[1] & 0x0F);
    return 6;
}
int sraArW()
{
    *regW = MySraW(*regW,*cregsB[1] & 0x0F);
    return 6;
}
int sraArL()
{
    *regL = MySraL(*regL,*cregsB[1] & 0x0F);
    return 8;
}
int sraM00()
{
    mem_writeB(mem,MySraB(memB,1));
    return 8;
}
int srawM10()
{
    mem_writeW(mem,MySraW(memW,1));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// SLL ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int sll4rB()
{
    *regB = MySlaB(*regB,readbyte());
    return 6;
}
int sll4rW()
{
    *regW = MySlaW(*regW,readbyte());
    return 6;
}
int sll4rL()
{
    *regL = MySlaL(*regL,readbyte());
    return 8;
}
int sllArB()
{
    *regB = MySlaB(*regB,*cregsB[1] & 0x0F);
    return 6;
}
int sllArW()
{
    *regW = MySlaW(*regW,*cregsB[1] & 0x0F);
    return 6;
}
int sllArL()
{
    *regL = MySlaL(*regL,*cregsB[1] & 0x0F);
    return 8;
}
int sllM00()
{
    mem_writeB(mem,MySlaB(memB,1));
    return 8;
}
int sllwM10()
{
    mem_writeW(mem,MySlaW(memW,1));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// SRL ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline unsigned char MySrlB(unsigned char i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        gen_regsSR = (gen_regsSR & ~CF) | (i & CF);
        i >>= 1;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 (i & SF) |
                 ((i) ? 0 : ZF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[i];
#else
				 ;
    parityB(i);
#endif

    return i;
}

inline unsigned short MySrlW(unsigned short i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        gen_regsSR = (gen_regsSR & ~CF) | (i & CF);
        i >>= 1;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 ((i>>8) & SF) |
                 ((i) ? 0 : ZF);
    parityW(i);
    return i;
}

inline unsigned long MySrlL(unsigned long i, unsigned char nr)
{
    nr = ((nr) ? nr : 16);
    for(;nr!=0;--nr)
    {
        gen_regsSR = (gen_regsSR & ~CF) | (i & CF);
        i >>= 1;
        state+= 2;
    }
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF);
    gen_regsSR = gen_regsSR |
                 ((i>>24) & SF) |
                 ((i) ? 0 : ZF);
    parityL(i);
    return i;
}

int srl4rB()
{
    *regB = MySrlB(*regB,readbyte());
    return 6;
}
int srl4rW()
{
    *regW = MySrlW(*regW,readbyte());
    return 6;
}
int srl4rL()
{
    *regL = MySrlL(*regL,readbyte());
    return 8;
}
int srlArB()
{
    *regB = MySrlB(*regB,*cregsB[1] & 0x0F);
    return 6;
}
int srlArW()
{
    *regW = MySrlW(*regW,*cregsB[1] & 0x0F);
    return 6;
}
int srlArL()
{
    *regL = MySrlL(*regL,*cregsB[1] & 0x0F);
    return 8;
}
int srlM00()
{
    mem_writeB(mem,MySrlB(memB,1));
    return 8;
}
int srlwM10()
{
    mem_writeW(mem,MySrlW(memW,1));
    return 8;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// RxD ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int rld00() // RLD A,(mem)   10000mmm 00000110
{
    unsigned char i,j;

    i = memB;
    j = (*cregsB[1])&0x0f;
    *cregsB[1] = ((*cregsB[1])&0xf0)|((i&0xf0)>>4);
    mem_writeB(mem,((i&0x0f)<<4)|j);
    // update flags
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 (*cregsB[1] & SF) |
                 ((*cregsB[1]) ? 0 : ZF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[*cregsB[1]];
#else
				 ;
   parityB(*cregsB[1]);
#endif

   return 12;
}

int rrd00() // RRD A,(mem)   10000mmm 00000111
{
    unsigned char i,j;

    i = memB;
    j = (*cregsB[1])&0x0f;
    *cregsB[1] = ((*cregsB[1])&0xf0)|(i&0x0f);
    mem_writeB(mem,((i>>4)|(j<<4)));
    // update flags
    gen_regsSR = gen_regsSR & ~(SF|ZF|HF|VF|NF|CF);
    gen_regsSR = gen_regsSR |
                 (*cregsB[1] & SF) |
                 ((*cregsB[1]) ? 0 : ZF)
#ifdef USE_PARITY_TABLE
				 | parityVtable[*cregsB[1]];
#else
				 ;
   parityB(*cregsB[1]);
#endif
    return 12;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// JP/JR /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int jp16()  // JP #16    00011010 xxxxxxxx xxxxxxxx
{
    gen_regsPC = readword();
    my_pc = get_address(gen_regsPC);
    return 7;
}

int jp24()  // JP #24    00011011 xxxxxxxx xxxxxxxx xxxxxxxx
{
    gen_regsPC = read24();
    my_pc = get_address(gen_regsPC);
    return 7;
}

int jrcc0()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    /*
    //cond0 always false
    if (cond0())
    {
        doJumpByte();
        return 8;
    }
*/
    skipJumpByte();
    return 4;
}

int jrcc1()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if (cond1())
    {
        doJumpByte();
        return 8;
    }

    skipJumpByte();
    return 4;
}

int jrcc2()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if (cond2())
    {
        doJumpByte();
        return 8;
    }

    skipJumpByte();
    return 4;
}

int jrcc3()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if (cond3())
    {
        doJumpByte();
        return 8;
    }

    skipJumpByte();
    return 4;
}

int jrcc4()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if (cond4())
    {
        doJumpByte();
        return 8;
    }

    skipJumpByte();
    return 4;
}

int jrcc5()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if (cond5())
    {
        doJumpByte();
        return 8;
    }

    skipJumpByte();
    return 4;
}

int jrcc6()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if (cond6())
    {
        doJumpByte();
        return 8;
    }
    skipJumpByte();
    return 4;
}

int jrcc7()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if (cond7())
    {
        doJumpByte();
        return 8;
    }

    skipJumpByte();
    return 4;
}

int jrcc8()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    //if (cond8())//cond8 is always TRUE
    //{
        doJumpByte();
        return 8;
    //}

    //skipJumpByte();
    //return 4;
}

int jrcc9()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if(notCond9())
    {
        skipJumpByte();
        return 4;
    }

    doJumpByte();
    return 8;
}

int jrccA()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if(notCondA())
    {
        skipJumpByte();
        return 4;
    }

    doJumpByte();
    return 8;
}

int jrccB()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if(notCondB())
    {
        skipJumpByte();
        return 4;
    }

    doJumpByte();
    return 8;
}

int jrccC()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if(notCondC())
    {
        skipJumpByte();
        return 4;
    }

    doJumpByte();
    return 8;
}

int jrccD()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if(notCondD())
    {
        skipJumpByte();
        return 4;
    }

    doJumpByte();
    return 8;
}

int jrccE()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if (notCondE())
    {
        // no need to read
        skipJumpByte();
        return 4;
    }
    doJumpByte();
    return 8;
}

int jrccF()  // JR cc,$+2+d8   0110cccc xxxxxxxx
{
    if(notCondF())
    {
        skipJumpByte();
        return 4;
    }

    doJumpByte();
    return 8;
}

int jrlcc0()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    /*
    //cond0 always false
    if (cond0())
    {
        doJumpWord();
        return 8;
    }
*/
    skipJumpWord();
    return 4;
}

int jrlcc1()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (cond1())
    {
        doJumpWord();
        return 8;
    }

    skipJumpWord();
    return 4;
}

int jrlcc2()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (cond2())
    {
        doJumpWord();
        return 8;
    }

    skipJumpWord();
    return 4;
}

int jrlcc3()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (cond3())
    {
        doJumpWord();
        return 8;
    }

    skipJumpWord();
    return 4;
}

int jrlcc4()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (cond4())
    {
        doJumpWord();
        return 8;
    }

    skipJumpWord();
    return 4;
}

int jrlcc5()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (cond5())
    {
        doJumpWord();
        return 8;
    }

    skipJumpWord();
    return 4;
}

int jrlcc6()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (cond6())
    {
        doJumpWord();
        return 8;
    }

    skipJumpWord();
    return 4;
}

int jrlcc7()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (cond7())
    {
        doJumpWord();
        return 8;
    }

    skipJumpWord();
    return 4;
}

int jrlcc8()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    //if (cond8())  //cond8 is always TRUE
    //{
        doJumpWord();
        return 8;
    //}

    //skipJumpWord();
    //return 4;
}

int jrlcc9()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (notCond9())
    {
        skipJumpWord();
        return 4;
    }

    doJumpWord();
    return 8;
}

int jrlccA()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (notCondA())
    {
        skipJumpWord();
        return 4;
    }

    doJumpWord();
    return 8;
}

int jrlccB()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (notCondB())
    {
        skipJumpWord();
        return 4;
    }

    doJumpWord();
    return 8;
}

int jrlccC()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (notCondC())
    {
        skipJumpWord();
        return 4;
    }

    doJumpWord();
    return 8;
}

int jrlccD()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (notCondD())
    {
        skipJumpWord();
        return 4;
    }

    doJumpWord();
    return 8;
}

int jrlccE()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    //Thor
    if (notCondE())
    {
        skipJumpWord();
        return 4;
    }
    doJumpWord();
    return 8;
}

int jrlccF()  // JR cc,$+2+d16   0110cccc xxxxxxxx
{
    if (notCondF())
    {
        skipJumpWord();
        return 4;
    }

    doJumpWord();
    return 8;
}

int jpccM300() // JP cc,mem
{
    /*
    //cond0 always false
    if (cond0())
    {
        gen_regsPC = mem;
        my_pc = get_address(gen_regsPC);
        return 8;
    }
    else
*/
        return 4;
}

int jpccM301() // JP cc,mem
{
    if (cond1())
    {
        gen_regsPC = mem;
        my_pc = get_address(gen_regsPC);
        return 8;
    }
    else
        return 4;
}

int jpccM302() // JP cc,mem
{
    if (cond2())
    {
        gen_regsPC = mem;
        my_pc = get_address(gen_regsPC);
        return 8;
    }
    else
        return 4;
}

int jpccM303() // JP cc,mem
{
    if (cond3())
    {
        gen_regsPC = mem;
        my_pc = get_address(gen_regsPC);
        return 8;
    }
    else
        return 4;
}

int jpccM304() // JP cc,mem
{
    if (cond4())
    {
        gen_regsPC = mem;
        my_pc = get_address(gen_regsPC);
        return 8;
    }
    else
        return 4;
}

int jpccM305() // JP cc,mem
{
    if (cond5())
    {
        gen_regsPC = mem;
        my_pc = get_address(gen_regsPC);
        return 8;
    }
    else
        return 4;
}

int jpccM306() // JP cc,mem
{
    if (gen_regsSR & ZF)//cond6())
    {
        gen_regsPC = mem;
        my_pc = get_address(gen_regsPC);
        return 8;
    }
    else
        return 4;
}

int jpccM307() // JP cc,mem
{
    if (gen_regsSR & CF)//cond7())
    {
        gen_regsPC = mem;
        my_pc = get_address(gen_regsPC);
        return 8;
    }
    else
        return 4;
}

int jpccM308() // JP cc,mem
{
    //if (cond8()) //always TRUE
    //{
        gen_regsPC = mem;
        my_pc = get_address(gen_regsPC);
        return 8;
    //}
    //else
    //    return 4;
}

int jpccM309() // JP cc,mem
{
    if (notCond9())
    {
        return 4;
    }

    gen_regsPC = mem;
    my_pc = get_address(gen_regsPC);
    return 8;
}

int jpccM30A() // JP cc,mem
{
    if (notCondA())
    {
        return 4;
    }

    gen_regsPC = mem;
    my_pc = get_address(gen_regsPC);
    return 8;
}

int jpccM30B() // JP cc,mem
{
    if (notCondB())
    {
        return 4;
    }

    gen_regsPC = mem;
    my_pc = get_address(gen_regsPC);
    return 8;
}

int jpccM30C() // JP cc,mem
{
    if (notCondC())
    {
        return 4;
    }

    gen_regsPC = mem;
    my_pc = get_address(gen_regsPC);
    return 8;
}

int jpccM30D() // JP cc,mem
{
    if (notCondD())
    {
        return 4;
    }

    gen_regsPC = mem;
    my_pc = get_address(gen_regsPC);
    return 8;
}

int jpccM30E() // JP cc,mem
{
    if (notCondE())
    {
        return 4;
    }

    gen_regsPC = mem;
    my_pc = get_address(gen_regsPC);
    return 8;
}

int jpccM30F() // JP cc,mem
{
    if (notCondF())
    {
        return 4;
    }

    gen_regsPC = mem;
    my_pc = get_address(gen_regsPC);
    return 8;
}



//////////////////////////////////////////////////////////////////////////////
////////////////////////////// CALL //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int call16() // CALL #16    00011100 xxxxxxxx xxxxxxxx
{
    unsigned long address = readword();

    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
    my_pc = get_address(gen_regsPC = address);
    return 12;
}

int call24() // CALL #24    00011101 xxxxxxxx xxxxxxxx xxxxxxxx
{
    unsigned long address = read24();

    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
    my_pc = get_address(gen_regsPC = address);
    return 12;
}

int calr()  // CALR $+3+d16   00011110 xxxxxxxx xxxxxxxx
{
    signed short d16 = readword();

    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
    gen_regsPC+= d16;
    my_pc+= d16;
    return 12;
}

int callccM300() // CALL cc,mem  10110mmm 1110cccc
{
    /*
    //cond0 always false
    if (cond0())
    {
	    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
        my_pc = get_address(gen_regsPC = mem);
        return 12;
    }
    else
*/
        return 6;
}

int callccM301() // CALL cc,mem  10110mmm 1110cccc
{
    if (cond1())
    {
	    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
        my_pc = get_address(gen_regsPC = mem);
        return 12;
    }
    else
        return 6;
}

int callccM302() // CALL cc,mem  10110mmm 1110cccc
{
    if (cond2())
    {
	    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
        my_pc = get_address(gen_regsPC = mem);
        return 12;
    }
    else
        return 6;
}

int callccM303() // CALL cc,mem  10110mmm 1110cccc
{
    if (cond3())
    {
	    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
        my_pc = get_address(gen_regsPC = mem);
        return 12;
    }
    else
        return 6;
}

int callccM304() // CALL cc,mem  10110mmm 1110cccc
{
    if (cond4())
    {
	    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
        my_pc = get_address(gen_regsPC = mem);
        return 12;
    }
    else
        return 6;
}

int callccM305() // CALL cc,mem  10110mmm 1110cccc
{
    if (cond5())
    {
	    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
        my_pc = get_address(gen_regsPC = mem);
        return 12;
    }
    else
        return 6;
}

int callccM306() // CALL cc,mem  10110mmm 1110cccc
{
    if (gen_regsSR & ZF)//cond6())
    {
	    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
        my_pc = get_address(gen_regsPC = mem);
        return 12;
    }
    else
        return 6;
}

int callccM307() // CALL cc,mem  10110mmm 1110cccc
{
    if (gen_regsSR & CF)//cond7())
    {
	    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
        my_pc = get_address(gen_regsPC = mem);
        return 12;
    }
    else
        return 6;
}

int callccM308() // CALL cc,mem  10110mmm 1110cccc
{
    //if (cond8())//always TRUE
    //{
	    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
        my_pc = get_address(gen_regsPC = mem);
        return 12;
    //}
    //else
    //    return 6;
}

int callccM309() // CALL cc,mem  10110mmm 1110cccc
{
    if (notCond9())
        return 6;

    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
    my_pc = get_address(gen_regsPC = mem);
    return 12;
}

int callccM30A() // CALL cc,mem  10110mmm 1110cccc
{
    if (notCondA())
        return 6;

    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
    my_pc = get_address(gen_regsPC = mem);
    return 12;
}

int callccM30B() // CALL cc,mem  10110mmm 1110cccc
{
    if (notCondB())
        return 6;

    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
    my_pc = get_address(gen_regsPC = mem);
    return 12;
}

int callccM30C() // CALL cc,mem  10110mmm 1110cccc
{
    if (notCondC())
        return 6;

    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
    my_pc = get_address(gen_regsPC = mem);
    return 12;
}

int callccM30D() // CALL cc,mem  10110mmm 1110cccc
{
    if (notCondD())
        return 6;

    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
    my_pc = get_address(gen_regsPC = mem);
    return 12;
}

int callccM30E() // CALL cc,mem  10110mmm 1110cccc
{
    if (notCondE())
        return 6;

    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
    my_pc = get_address(gen_regsPC = mem);
    return 12;
}

int callccM30F() // CALL cc,mem  10110mmm 1110cccc
{
    if (notCondF())
        return 6;

    tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
    my_pc = get_address(gen_regsPC = mem);
    return 12;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// DJNZ //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int djnzB() // DJNZ r,$+3+d8  11001rrr 00011100 xxxxxxxx
{
    if (--*regB)
    {
        signed char d8 = readbyte();
        gen_regsPC+= d8;
        my_pc+= d8;
        return 11;
    }
    else
    {
        ++gen_regsPC;
        ++my_pc;
        return 7;
    }
}

int djnzW() // DJNZ r,$+3+d8  11011rrr 00011100 xxxxxxxx
{
    if (--*regW)
    {
        signed char d8 = readbyte();
        gen_regsPC+= d8;
        my_pc+= d8;
        return 11;
    }
    else
    {
        ++gen_regsPC;
        ++my_pc;
        return 7;
    }
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// RET ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int ret()  // RET     00001110
{
    gen_regsPC = mem_readL(gen_regsXSP);
    my_pc = get_address(gen_regsPC);
    gen_regsXSP+= 4;
    return 9;
}

int retcc0() // RET cc    10110000 1111cccc
{
    /*
    //cond0 always false
    if (cond0())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
*/
        return 6;
}

int retcc1() // RET cc    10110000 1111cccc
{
    if (cond1())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retcc2() // RET cc    10110000 1111cccc
{
    if (cond2())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retcc3() // RET cc    10110000 1111cccc
{
    if (cond3())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retcc4() // RET cc    10110000 1111cccc
{
    if (cond4())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retcc5() // RET cc    10110000 1111cccc
{
    if (cond5())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retcc6() // RET cc    10110000 1111cccc
{
    if (gen_regsSR & ZF)//cond6())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retcc7() // RET cc    10110000 1111cccc
{
    if (gen_regsSR & CF)//cond7())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retcc8() // RET cc    10110000 1111cccc
{
    //if (cond8())//always TRUE
    //{
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    //}
    //else
    //    return 6;
}

int retcc9() // RET cc    10110000 1111cccc
{
    if (cond9())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retccA() // RET cc    10110000 1111cccc
{
    if (condA())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retccB() // RET cc    10110000 1111cccc
{
    if (condB())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retccC() // RET cc    10110000 1111cccc
{
    if (condC())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retccD() // RET cc    10110000 1111cccc
{
    if (condD())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retccE() // RET cc    10110000 1111cccc
{
    if (notCondE())
        return 6;
    gen_regsPC = mem_readL(gen_regsXSP);
    gen_regsXSP+= 4;
    my_pc = get_address(gen_regsPC);
    return 12;
}

int retccF() // RET cc    10110000 1111cccc
{
    if (condF())
    {
        gen_regsPC = mem_readL(gen_regsXSP);
        gen_regsXSP+= 4;
        my_pc = get_address(gen_regsPC);
        return 12;
    }
    else
        return 6;
}

int retd()  // RETD d16    00001111 xxxxxxxx xxxxxxxx
{
    signed short d16 = readword();

    gen_regsPC = mem_readL(gen_regsXSP);
    my_pc = get_address(gen_regsPC);
    gen_regsXSP+= d16 + 4;
    return 9;
}

int reti()  // RETI     00000111
{
#ifdef TARGET_GP2X
    register byte *gA asm("r4");
    register unsigned long localXSP = gen_regsXSP;

    gA = get_address(localXSP);
    localXSP += 6;
    gen_regsXSP = localXSP;


    if(gA == 0)
    {
        gen_regsSR = gen_regsPC = 0;
    }
    else
    {
        asm volatile(
            "ldrb %0, [%2], #1\n"
            "ldrb r2, [%2], #1\n"
            "orr %0, %0, r2, asl #8\n"
            "bic r1,%2,#3 \n"
            "ldmia r1,{r0,r3} \n"
            "ands r1,%2,#3 \n"
            "movne r2,r1,lsl #3 \n"
            "movne r0,r0,lsr r2 \n"
            "rsbne r1,r2,#32 \n"
            "orrne r0,r0,r3,lsl r1\n"
            "mov    %1,r0"
        : "=r"(gen_regsSR), "=r"(gen_regsPC)
                    : "r"(gA)
                    : "r0", "r1","r2","r3");
    }

#else
    gen_regsSR = mem_readW(gen_regsXSP);
    gen_regsXSP+= 2;
    gen_regsPC = mem_readL(gen_regsXSP);
    gen_regsXSP+= 4;
#endif

    my_pc = get_address(gen_regsPC);
    set_cregs();

    return 12;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// undefined /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int udef()
{
    //dbg_print("*** Call to udef() in tlcs900h core ***\n");
    m_bIsActive = false;
    return 1;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////// BIOS calls  /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline int VECT_SHUTDOWN()
{
    return 10;
}

inline int VECT_CLOCKGEARSET(unsigned char speed, unsigned char regen)
{
    switch(speed)
    {
        case 0:
        tlcsClockMulti = 1;
        break;
        case 1:
        tlcsClockMulti = 2;
        break;
        case 2:
        tlcsClockMulti = 4;
        break;
        case 3:
        tlcsClockMulti = 8;
        break;
        case 4:
        tlcsClockMulti = 16;
        break;
    }
    return 10;
}

inline unsigned char makeBCD(int i)
{
    int upper;

    if (i>=99)
        return 0x99;
    upper = i/10;
    return (upper<<4)|(i-(upper*10));
}

inline int VECT_RTCGET(unsigned int dest)
{
    // dest+0 // year
    // dest+1 // month  all in BCD
    // dest+2 // day
    // dest+3 // hours
    // dest+4 // minutes
    // dest+5 // nr year after leap : day of the week
    unsigned char *d = get_address(dest);

    //Flavor hacked

    /*CTime now = CTime::GetCurrentTime();
    int year = now.GetYear() % 100;

    d[0] = makeBCD(year);
    d[1] = makeBCD(now.GetMonth());
    d[2] = makeBCD(now.GetDay());
    d[3] = makeBCD(now.GetHour());
    d[4] = makeBCD(now.GetMinute());
    d[5] = makeBCD(now.GetSecond());
    d[6] = ((year % 4)<<4)|(now.GetDayOfWeek()-1);*/

    int year = 2006 % 100;

    d[0] = makeBCD(year);
    d[1] = makeBCD(2);
    d[2] = makeBCD(28);
    d[3] = makeBCD(11);
    d[4] = makeBCD(47);
    d[5] = makeBCD(00);
    d[6] = ((year % 4)<<4)|(5-1);

    return 100;
}


inline int VECT_INTLVSET(unsigned char interrupt, unsigned char level)
{
    //   0 - Interrupt from RTC alarm
    //   1 - Interrupt from the Z80 CPU
    //   2 - Interrupt from the 8 bit timer 0
    //   3 - Interrupt from the 8 bit timer 1
    //   4 - Interrupt from the 8 bit timer 2
    //   5 - Interrupt from the 8 bit timer 3
    //   6 - End of transfer interrupt from DMA channel 0
    //   7 - End of transfer interrupt from DMA channel 1
    //   8 - End of transfer interrupt from DMA channel 2
    //   9 - End of transfer interrupt from DMA channel 3
    switch(interrupt)
    {
        case 0x00:
        cpuram[0x70] = (cpuram[0x70] & 0xf0) |  (level & 0x07);
        break;
        case 0x01:
        cpuram[0x71] = (cpuram[0x71] & 0x0f) | ((level & 0x07)<<4);
        break;
        case 0x02:
        cpuram[0x73] = (cpuram[0x73] & 0xf0) |  (level & 0x07);
        break;
        case 0x03:
        cpuram[0x73] = (cpuram[0x73] & 0x0f) | ((level & 0x07)<<4);
        break;
        case 0x04:
        cpuram[0x74] = (cpuram[0x74] & 0xf0) |  (level & 0x07);
        break;
        case 0x05:
        cpuram[0x74] = (cpuram[0x74] & 0x0f) | ((level & 0x07)<<4);
        break;
        case 0x06:
        cpuram[0x79] = (cpuram[0x79] & 0xf0) |  (level & 0x07);
        break;
        case 0x07:
        cpuram[0x79] = (cpuram[0x79] & 0x0f) | ((level & 0x07)<<4);
        break;
        case 0x08:
        cpuram[0x7a] = (cpuram[0x7a] & 0xf0) |  (level & 0x07);
        break;
        case 0x09:
        cpuram[0x7a] = (cpuram[0x7a] & 0x0f) | ((level & 0x07)<<4);
        break;
    }
    return 100;
}

inline int VECT_SYSFONTSET(unsigned char parms)
{
    ngpBiosSYSFONTSET(get_address(0xA000),(parms>>4)&0x03,parms&0x03);
    return 100;
}

inline int VECT_ALARMSET(unsigned char day, unsigned char hour, unsigned char minute, unsigned char *result)
{
    *result = 0;  // SYS_SUCCESS
    return 100;
}

inline int VECT_FLASHWRITE(unsigned char chip, unsigned short size, unsigned int from, unsigned int to, unsigned char *result)
{
    unsigned char *fromAddr;
    //toAddr = get_address(((chip) ? 0x00800000 : 0x00200000)+to);
    fromAddr = get_address(from);

    vectFlashWrite(chip, to, fromAddr, size * 256);

    *result = 0;
    return 100;
}

inline int VECT_FLASHERS(unsigned char chip, unsigned char blockNum, unsigned char *result)
{
    vectFlashErase(chip, blockNum);
    *result = 0;
    return 100;
}

inline int VECT_FLASHALLERS(unsigned char chip, unsigned char *result)
{
    vectFlashChipErase(chip);
    *result = 0;
    return 100;
}

inline int VECT_GEMODESET(unsigned char mode)
{
    mem_writeB(0x87F0,0xAA);  // allow GE MODE registers to be written to
    if (mode < 0x10)
    {
        // set B/W mode
        mem_writeB(0x87E2,0x80);
        mem_writeB(0x6F95,0x00);
    }
    else
    {
        // set color mode
        mem_writeB(0x87E2,0x00);
        mem_writeB(0x6F95,0x10);
    }
    mem_writeB(0x87F0,0x55);  // disallow GE MODE registers to be written to
    return 20;
}

// execute a bios function
inline int doBios(unsigned char biosNr)
{
    switch (biosNr)
    {
        case 0x00:
        // VECT_SHUTDOWN
        // Power off the Neogeo pocket unit
        // no params, no result
        return VECT_SHUTDOWN();
        case 0x01:
        // VECT_CLOCKGEARSET
        // Change the CPU speed of the Neogeo Pocket
        // param: RB3 - Set Clock Speed
        //    00 - 6.144 MHz
        //    01 - 3.072 MHz
        //    02 - 1.536 MHz
        //    03 -  .768 MHz
        //    04 -  .384 MHz
        // param: RC3 - Auto clock speed regeneration with panel switch input
        //    0  - No
        //    !0 - Yes
        return VECT_CLOCKGEARSET(*allregsB[0x35], *allregsB[0x34]);
        break;
        case 0x02: // VECT_RTCGET
        // Obtain real time clock information
        // param: XHL3
        //   data written:
        //   +0 : Year (00-90 = 2000-2090, 91-99 = 1991 - 1999)
        //   +1 : Month (1 - 12)
        //   +2 : Day (1-31)
        //   +3 : Hour (0 - 23)
        //   +4 : Minute (0 - 59)
        //   +5 : Second (0 - 59)
        //   +6 : upper nibble: years after leap year
        //        lower nibble: 0 - sun, 1 - mon, 2 - tue,...., 6 - sat)
        // all BCD
        return VECT_RTCGET(*allregsL[0x3C]);
        case 0x04:
        // VECT_INTLVSET
        // param: RB3 - interrupt level (0-5)
        // param: RC3 - interrupt number to set
        //   0 - Interrupt from RTC alarm
        //   1 - Interrupt from the Z80 CPU
        //   2 - Interrupt from the 8 bit timer 0
        //   3 - Interrupt from the 8 bit timer 1
        //   4 - Interrupt from the 8 bit timer 2
        //   5 - Interrupt from the 8 bit timer 3
        //   6 - End of transfer interrupt from DMA channel 0
        //   7 - End of transfer interrupt from DMA channel 1
        //   8 - End of transfer interrupt from DMA channel 2
        //   9 - End of transfer interrupt from DMA channel 3
        return VECT_INTLVSET(*allregsB[0x34], *allregsB[0x35]);
        case 0x05:
        return VECT_SYSFONTSET(*allregsB[0x30]);
        case 0x06:  // store something in saveram
        // copy BC3*0x0100 bytes of data from XHL3 to saveram + DE3
        return VECT_FLASHWRITE(*allregsB[0x30],*allregsW[0x34],*allregsL[0x3C],*allregsL[0x38],allregsB[0x30]);
        case 0x07:  //erase the chip
        return VECT_FLASHALLERS(*allregsB[0x30],allregsB[0x30]);
        break;
        case 0x08:  // set current memory save block (64 kb blocks); prepare for write
                    //erase a block
					// parm: RB3
        *cregsB[1] = 0;   // ld a,00
        return VECT_FLASHERS(*allregsB[0x30],*allregsB[0x35],allregsB[0x30]);
        case 0x09:
        return VECT_ALARMSET(*allregsB[0x36],*allregsB[0x35],*allregsB[0x34],allregsB[0x30]);
        case 0x0e:
        return VECT_GEMODESET(0x10);  // Implementation wrong in ngpc bios???, always set to color
        case 0x14:
        *cregsB[1] = 1;   // ld a,01
        break;
        case 0x17:
        *cregsW[0] = 0;   // ld wa,0000
        break;
        case 0x18:
        *cregsW[0] = 0;   // ld wa,0000
        break;
        case 0x19:
        *cregsB[2] = 0;   // ld b,00
        break;
    }
    return 100;      // let's just take a value... correct?
}

int bios()
{
    unsigned char biosNr = readbyte();

    return doBios(biosNr);
}

// instructions where the highest bit of the first byte is set to 1
// x0000xxx xxxxxxxx
int (*decode_table80[256])() =
    {
        udef,  udef,  udef,  udef,  pushM00, udef,  rld00,  rrd00,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        ldi,  ldir,  ldd,  lddr,  cpiB,  cpirB,  cpdB,  cpdrB,
        udef,  ld16M00, udef,  udef,  udef,  udef,  udef,  udef,
        ldRM00,  ldRM00,  ldRM00,  ldRM00,  ldRM00,  ldRM00,  ldRM00,  ldRM00,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        exMRB00, exMRB00, exMRB00, exMRB00, exMRB00, exMRB00, exMRB00, exMRB00,
        addMI00, adcMI00, subMI00, sbcMI00, andMI00, xorMI00, orMI00,  cpMI00,
        //
        mulRMB00, mulRMB00, mulRMB00, mulRMB00, mulRMB00, mulRMB00, mulRMB00, mulRMB00,
        mulsRMB00, mulsRMB00, mulsRMB00, mulsRMB00, mulsRMB00, mulsRMB00, mulsRMB00, mulsRMB00,
        divRMB00, divRMB00, divRMB00, divRMB00, divRMB00, divRMB00, divRMB00, divRMB00,
        divsRMB00, divsRMB00, divsRMB00, divsRMB00, divsRMB00, divsRMB00, divsRMB00, divsRMB00,
        inc3M00, inc3M00, inc3M00, inc3M00, inc3M00, inc3M00, inc3M00, inc3M00,
        dec3M00, dec3M00, dec3M00, dec3M00, dec3M00, dec3M00, dec3M00, dec3M00,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        rlcM00,  rrcM00,  rlM00,  rrM00,  slaM00,  sraM00,  sllM00,  srlM00,
        //
        addRMB00, addRMB00, addRMB00, addRMB00, addRMB00, addRMB00, addRMB00, addRMB00,
        addMRB00, addMRB00, addMRB00, addMRB00, addMRB00, addMRB00, addMRB00, addMRB00,
        adcRMB00, adcRMB00, adcRMB00, adcRMB00, adcRMB00, adcRMB00, adcRMB00, adcRMB00,
        adcMRB00, adcMRB00, adcMRB00, adcMRB00, adcMRB00, adcMRB00, adcMRB00, adcMRB00,
        subRMB00, subRMB00, subRMB00, subRMB00, subRMB00, subRMB00, subRMB00, subRMB00,
        subMRB00, subMRB00, subMRB00, subMRB00, subMRB00, subMRB00, subMRB00, subMRB00,
        sbcRMB00, sbcRMB00, sbcRMB00, sbcRMB00, sbcRMB00, sbcRMB00, sbcRMB00, sbcRMB00,
        sbcMRB00, sbcMRB00, sbcMRB00, sbcMRB00, sbcMRB00, sbcMRB00, sbcMRB00, sbcMRB00,
        //
        andRMB00, andRMB00, andRMB00, andRMB00, andRMB00, andRMB00, andRMB00, andRMB00,
        andMRB00, andMRB00, andMRB00, andMRB00, andMRB00, andMRB00, andMRB00, andMRB00,
        xorRMB00, xorRMB00, xorRMB00, xorRMB00, xorRMB00, xorRMB00, xorRMB00, xorRMB00,
        xorMRB00, xorMRB00, xorMRB00, xorMRB00, xorMRB00, xorMRB00, xorMRB00, xorMRB00,
        orRMB00, orRMB00, orRMB00, orRMB00, orRMB00, orRMB00, orRMB00, orRMB00,
        orMRB00, orMRB00, orMRB00, orMRB00, orMRB00, orMRB00, orMRB00, orMRB00,
        cpRMB00, cpRMB00, cpRMB00, cpRMB00, cpRMB00, cpRMB00, cpRMB00, cpRMB00,
        cpMRB00, cpMRB00, cpMRB00, cpMRB00, cpMRB00, cpMRB00, cpMRB00, cpMRB00
    };
// x0010xxx xxxxxxxx
int (*decode_table90[256])() =
    {
        udef,  udef,  udef,  udef,  pushwM10, udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        ldiw,  ldirw,  lddw,  lddrw,  cpiW,  cpirW,  cpdW,  cpdrW,
        udef,  ldw16M10, udef,  udef,  udef,  udef,  udef,  udef,
        ldRM10,  ldRM10,  ldRM10,  ldRM10,  ldRM10,  ldRM10,  ldRM10,  ldRM10,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        exMRW10, exMRW10, exMRW10, exMRW10, exMRW10, exMRW10, exMRW10, exMRW10,
        addwMI10, adcwMI10, subwMI10, sbcwMI10, andwMI10, xorwMI10, orwMI10, cpwMI10,
        //
        mulRMW10, mulRMW10, mulRMW10, mulRMW10, mulRMW10, mulRMW10, mulRMW10, mulRMW10,
        mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10,
        divRMW10, divRMW10, divRMW10, divRMW10, divRMW10, divRMW10, divRMW10, divRMW10,
        divsRMW10, divsRMW10, divsRMW10, divsRMW10, divsRMW10, divsRMW10, divsRMW10, divsRMW10,
        incw3M10, incw3M10, incw3M10, incw3M10, incw3M10, incw3M10, incw3M10, incw3M10,
        decw3M10, decw3M10, decw3M10, decw3M10, decw3M10, decw3M10, decw3M10, decw3M10,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        rlcwM10, rrcwM10, rlwM10,  rrwM10,  slawM10, srawM10, sllwM10, srlwM10,
        //
        addRMW10, addRMW10, addRMW10, addRMW10, addRMW10, addRMW10, addRMW10, addRMW10,
        addMRW10, addMRW10, addMRW10, addMRW10, addMRW10, addMRW10, addMRW10, addMRW10,
        adcRMW10, adcRMW10, adcRMW10, adcRMW10, adcRMW10, adcRMW10, adcRMW10, adcRMW10,
        adcMRW10, adcMRW10, adcMRW10, adcMRW10, adcMRW10, adcMRW10, adcMRW10, adcMRW10,
        subRMW10, subRMW10, subRMW10, subRMW10, subRMW10, subRMW10, subRMW10, subRMW10,
        subMRW10, subMRW10, subMRW10, subMRW10, subMRW10, subMRW10, subMRW10, subMRW10,
        sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10,
        sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10,
        //
        andRMW10, andRMW10, andRMW10, andRMW10, andRMW10, andRMW10, andRMW10, andRMW10,
        andMRW10, andMRW10, andMRW10, andMRW10, andMRW10, andMRW10, andMRW10, andMRW10,
        xorRMW10, xorRMW10, xorRMW10, xorRMW10, xorRMW10, xorRMW10, xorRMW10, xorRMW10,
        xorMRW10, xorMRW10, xorMRW10, xorMRW10, xorMRW10, xorMRW10, xorMRW10, xorMRW10,
        orRMW10, orRMW10, orRMW10, orRMW10, orRMW10, orRMW10, orRMW10, orRMW10,
        orMRW10, orMRW10, orMRW10, orMRW10, orMRW10, orMRW10, orMRW10, orMRW10,
        cpRMW10, cpRMW10, cpRMW10, cpRMW10, cpRMW10, cpRMW10, cpRMW10, cpRMW10,
        cpMRW10, cpMRW10, cpMRW10, cpMRW10, cpMRW10, cpMRW10, cpMRW10, cpMRW10
    };
// x0011xxx xxxxxxxx
int (*decode_table98[256])() =
    {
        udef,  udef,  udef,  udef,  pushwM10, udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  ldw16M10, udef,  udef,  udef,  udef,  udef,  udef,
        ldRM10,  ldRM10,  ldRM10,  ldRM10,  ldRM10,  ldRM10,  ldRM10,  ldRM10,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        exMRW10, exMRW10, exMRW10, exMRW10, exMRW10, exMRW10, exMRW10, exMRW10,
        addwMI10, adcwMI10, subwMI10, sbcwMI10, andwMI10, xorwMI10, orwMI10, cpwMI10,
        //
        mulRMW10, mulRMW10, mulRMW10, mulRMW10, mulRMW10, mulRMW10, mulRMW10, mulRMW10,
        mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10,
        divRMW10, divRMW10, divRMW10, divRMW10, divRMW10, divRMW10, divRMW10, divRMW10,
        divsRMW10, divsRMW10, divsRMW10, divsRMW10, divsRMW10, divsRMW10, divsRMW10, divsRMW10,
        incw3M10, incw3M10, incw3M10, incw3M10, incw3M10, incw3M10, incw3M10, incw3M10,
        decw3M10, decw3M10, decw3M10, decw3M10, decw3M10, decw3M10, decw3M10, decw3M10,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        rlcwM10, rrcwM10, rlwM10,  rrwM10,  slawM10, srawM10, sllwM10, srlwM10,
        //
        addRMW10, addRMW10, addRMW10, addRMW10, addRMW10, addRMW10, addRMW10, addRMW10,
        addMRW10, addMRW10, addMRW10, addMRW10, addMRW10, addMRW10, addMRW10, addMRW10,
        adcRMW10, adcRMW10, adcRMW10, adcRMW10, adcRMW10, adcRMW10, adcRMW10, adcRMW10,
        adcMRW10, adcMRW10, adcMRW10, adcMRW10, adcMRW10, adcMRW10, adcMRW10, adcMRW10,
        subRMW10, subRMW10, subRMW10, subRMW10, subRMW10, subRMW10, subRMW10, subRMW10,
        subMRW10, subMRW10, subMRW10, subMRW10, subMRW10, subMRW10, subMRW10, subMRW10,
        sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10,
        sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10,
        //
        andRMW10, andRMW10, andRMW10, andRMW10, andRMW10, andRMW10, andRMW10, andRMW10,
        andMRW10, andMRW10, andMRW10, andMRW10, andMRW10, andMRW10, andMRW10, andMRW10,
        xorRMW10, xorRMW10, xorRMW10, xorRMW10, xorRMW10, xorRMW10, xorRMW10, xorRMW10,
        xorMRW10, xorMRW10, xorMRW10, xorMRW10, xorMRW10, xorMRW10, xorMRW10, xorMRW10,
        orRMW10, orRMW10, orRMW10, orRMW10, orRMW10, orRMW10, orRMW10, orRMW10,
        orMRW10, orMRW10, orMRW10, orMRW10, orMRW10, orMRW10, orMRW10, orMRW10,
        cpRMW10, cpRMW10, cpRMW10, cpRMW10, cpRMW10, cpRMW10, cpRMW10, cpRMW10,
        cpMRW10, cpMRW10, cpMRW10, cpMRW10, cpMRW10, cpMRW10, cpMRW10, cpMRW10
    };
// x0100xxx xxxxxxxx
int (*decode_tableA0[256])() =
    {
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        ldRM20,  ldRM20,  ldRM20,  ldRM20,  ldRM20,  ldRM20,  ldRM20,  ldRM20,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef , udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //
        addRML20, addRML20, addRML20, addRML20, addRML20, addRML20, addRML20, addRML20,
        addMRL20, addMRL20, addMRL20, addMRL20, addMRL20, addMRL20, addMRL20, addMRL20,
        adcRML20, adcRML20, adcRML20, adcRML20, adcRML20, adcRML20, adcRML20, adcRML20,
        adcMRL20, adcMRL20, adcMRL20, adcMRL20, adcMRL20, adcMRL20, adcMRL20, adcMRL20,
        subRML20, subRML20, subRML20, subRML20, subRML20, subRML20, subRML20, subRML20,
        subMRL20, subMRL20, subMRL20, subMRL20, subMRL20, subMRL20, subMRL20, subMRL20,
        sbcRML20, sbcRML20, sbcRML20, sbcRML20, sbcRML20, sbcRML20, sbcRML20, sbcRML20,
        sbcMRL20, sbcMRL20, sbcMRL20, sbcMRL20, sbcMRL20, sbcMRL20, sbcMRL20, sbcMRL20,
        //
        andRML20, andRML20, andRML20, andRML20, andRML20, andRML20, andRML20, andRML20,
        andMRL20, andMRL20, andMRL20, andMRL20, andMRL20, andMRL20, andMRL20, andMRL20,
        xorRML20, xorRML20, xorRML20, xorRML20, xorRML20, xorRML20, xorRML20, xorRML20,
        xorMRL20, xorMRL20, xorMRL20, xorMRL20, xorMRL20, xorMRL20, xorMRL20, xorMRL20,
        orRML20, orRML20, orRML20, orRML20, orRML20, orRML20, orRML20, orRML20,
        orMRL20, orMRL20, orMRL20, orMRL20, orMRL20, orMRL20, orMRL20, orMRL20,
        cpRML20, cpRML20, cpRML20, cpRML20, cpRML20, cpRML20, cpRML20, cpRML20,
        cpMRL20, cpMRL20, cpMRL20, cpMRL20, cpMRL20, cpMRL20, cpMRL20, cpMRL20
    };
// x0110xxx xxxxxxxx
int (*decode_tableB0[256])() =
    {
        ldMI30,  udef,  ldwMI30, udef,  popM30,  udef,  popwM30, udef,
        udef,  udef,  udef,  udef,  udef , udef,  udef,  udef,
        udef,  udef,  udef,  udef,  ldM1630, udef,  ldwM1630, udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30,
        andcfAM30, orcfAM30, xorcfAM30, ldcfAM30, stcfAM30, udef,  udef,  udef,
        ldaRML30, ldaRML30, ldaRML30, ldaRML30, ldaRML30, ldaRML30, ldaRML30, ldaRML30,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //
        ldMR30B, ldMR30B, ldMR30B, ldMR30B, ldMR30B, ldMR30B, ldMR30B, ldMR30B,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        ldMR30W, ldMR30W, ldMR30W, ldMR30W, ldMR30W, ldMR30W, ldMR30W, ldMR30W,
        udef,  udef,  udef , udef,  udef,  udef,  udef,  udef,
        ldMR30L, ldMR30L, ldMR30L, ldMR30L, ldMR30L, ldMR30L, ldMR30L, ldMR30L,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef , udef,  udef,  udef,  udef,  udef,  udef,
        //
        andcf3M30, andcf3M30, andcf3M30, andcf3M30, andcf3M30, andcf3M30, andcf3M30, andcf3M30,
        orcf3M30, orcf3M30, orcf3M30, orcf3M30, orcf3M30, orcf3M30, orcf3M30, orcf3M30,
        xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30,
        ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30,
        stcf3M30, stcf3M30, stcf3M30, stcf3M30, stcf3M30, stcf3M30, stcf3M30, stcf3M30,
        tset3M30, tset3M30, tset3M30, tset3M30, tset3M30, tset3M30, tset3M30, tset3M30,
        res3M30, res3M30, res3M30, res3M30, res3M30, res3M30, res3M30, res3M30,
        set3M30, set3M30, set3M30, set3M30, set3M30, set3M30, set3M30, set3M30,
        //
        chg3M30, chg3M30, chg3M30, chg3M30, chg3M30, chg3M30, chg3M30, chg3M30,
        bit3M30, bit3M30, bit3M30, bit3M30, bit3M30, bit3M30, bit3M30, bit3M30,
        jpccM300, jpccM301, jpccM302, jpccM303, jpccM304, jpccM305, jpccM306, jpccM307,
        jpccM308, jpccM309, jpccM30A, jpccM30B, jpccM30C, jpccM30D, jpccM30E, jpccM30F,
        callccM300, callccM301, callccM302, callccM303, callccM304, callccM305, callccM306, callccM307,
        callccM308, callccM309, callccM30A, callccM30B, callccM30C, callccM30D, callccM30E, callccM30F,
        retcc0,  retcc1,  retcc2,  retcc3,  retcc4,  retcc5,  retcc6,  retcc7,
        retcc8,  retcc9,  retccA,  retccB,  retccC,  retccD,  retccE,  retccF
    };
// x0111xxx xxxxxxxx
int (*decode_tableB8[256])() =
    {
        ldMI30,  udef,  ldwMI30, udef,  popM30,  udef,  popwM30, udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  ldM1630, udef,  ldwM1630, udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30,
        andcfAM30, orcfAM30, xorcfAM30, ldcfAM30, stcfAM30, udef,  udef,  udef,
        ldaRML30, ldaRML30, ldaRML30, ldaRML30, ldaRML30, ldaRML30, ldaRML30, ldaRML30,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //
        ldMR30B, ldMR30B, ldMR30B, ldMR30B, ldMR30B, ldMR30B, ldMR30B, ldMR30B,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        ldMR30W, ldMR30W, ldMR30W, ldMR30W, ldMR30W, ldMR30W, ldMR30W, ldMR30W,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        ldMR30L, ldMR30L, ldMR30L, ldMR30L, ldMR30L, ldMR30L, ldMR30L, ldMR30L,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //
        andcf3M30, andcf3M30, andcf3M30, andcf3M30, andcf3M30, andcf3M30, andcf3M30, andcf3M30,
        orcf3M30, orcf3M30, orcf3M30, orcf3M30, orcf3M30, orcf3M30, orcf3M30, orcf3M30,
        xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30,
        ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30,
        stcf3M30, stcf3M30, stcf3M30, stcf3M30, stcf3M30, stcf3M30, stcf3M30, stcf3M30,
        tset3M30, tset3M30, tset3M30, tset3M30, tset3M30, tset3M30, tset3M30, tset3M30,
        res3M30, res3M30, res3M30, res3M30, res3M30, res3M30, res3M30, res3M30,
        set3M30, set3M30, set3M30, set3M30, set3M30, set3M30, set3M30, set3M30,
        //
        chg3M30, chg3M30, chg3M30, chg3M30, chg3M30, chg3M30, chg3M30, chg3M30,
        bit3M30, bit3M30, bit3M30, bit3M30, bit3M30, bit3M30, bit3M30, bit3M30,
        jpccM300, jpccM301, jpccM302, jpccM303, jpccM304, jpccM305, jpccM306, jpccM307,
        jpccM308, jpccM309, jpccM30A, jpccM30B, jpccM30C, jpccM30D, jpccM30E, jpccM30F,
        callccM300, callccM301, callccM302, callccM303, callccM304, callccM305, callccM306, callccM307,
        callccM308, callccM309, callccM30A, callccM30B, callccM30C, callccM30D, callccM30E, callccM30F,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef
    };
// x1000xxx xxxxxxxx
int (*decode_tableC0[256])() =
    {
        udef,  udef,  udef,  udef,  pushM00, udef,  rld00,  rrd00,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  ld16M00, udef,  udef,  udef,  udef,  udef,  udef,
        ldRM00,  ldRM00,  ldRM00,  ldRM00,  ldRM00,  ldRM00,  ldRM00,  ldRM00,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        exMRB00, exMRB00, exMRB00, exMRB00, exMRB00, exMRB00, exMRB00, exMRB00,
        addMI00, adcMI00, subMI00, sbcMI00, andMI00, xorMI00, orMI00,  cpMI00,
        //
        mulRMB00, mulRMB00, mulRMB00, mulRMB00, mulRMB00, mulRMB00, mulRMB00, mulRMB00,
        mulsRMB00, mulsRMB00, mulsRMB00, mulsRMB00, mulsRMB00, mulsRMB00, mulsRMB00, mulsRMB00,
        divRMB00, divRMB00, divRMB00, divRMB00, divRMB00, divRMB00, divRMB00, divRMB00,
        divsRMB00, divsRMB00, divsRMB00, divsRMB00, divsRMB00, divsRMB00, divsRMB00, divsRMB00,
        inc3M00, inc3M00, inc3M00, inc3M00, inc3M00, inc3M00, inc3M00, inc3M00,
        dec3M00, dec3M00, dec3M00, dec3M00, dec3M00, dec3M00, dec3M00, dec3M00,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        rlcM00,  rrcM00,  rlM00,  rrM00,  slaM00,  sraM00,  sllM00,  srlM00,
        //
        addRMB00, addRMB00, addRMB00, addRMB00, addRMB00, addRMB00, addRMB00, addRMB00,
        addMRB00, addMRB00, addMRB00, addMRB00, addMRB00, addMRB00, addMRB00, addMRB00,
        adcRMB00, adcRMB00, adcRMB00, adcRMB00, adcRMB00, adcRMB00, adcRMB00, adcRMB00,
        adcMRB00, adcMRB00, adcMRB00, adcMRB00, adcMRB00, adcMRB00, adcMRB00, adcMRB00,
        subRMB00, subRMB00, subRMB00, subRMB00, subRMB00, subRMB00, subRMB00, subRMB00,
        subMRB00, subMRB00, subMRB00, subMRB00, subMRB00, subMRB00, subMRB00, subMRB00,
        sbcRMB00, sbcRMB00, sbcRMB00, sbcRMB00, sbcRMB00, sbcRMB00, sbcRMB00, sbcRMB00,
        sbcMRB00, sbcMRB00, sbcMRB00, sbcMRB00, sbcMRB00, sbcMRB00, sbcMRB00, sbcMRB00,
        //
        andRMB00, andRMB00, andRMB00, andRMB00, andRMB00, andRMB00, andRMB00, andRMB00,
        andMRB00, andMRB00, andMRB00, andMRB00, andMRB00, andMRB00, andMRB00, andMRB00,
        xorRMB00, xorRMB00, xorRMB00, xorRMB00, xorRMB00, xorRMB00, xorRMB00, xorRMB00,
        xorMRB00, xorMRB00, xorMRB00, xorMRB00, xorMRB00, xorMRB00, xorMRB00, xorMRB00,
        orRMB00, orRMB00, orRMB00, orRMB00, orRMB00, orRMB00, orRMB00, orRMB00,
        orMRB00, orMRB00, orMRB00, orMRB00, orMRB00, orMRB00, orMRB00, orMRB00,
        cpRMB00, cpRMB00, cpRMB00, cpRMB00, cpRMB00, cpRMB00, cpRMB00, cpRMB00,
        cpMRB00, cpMRB00, cpMRB00, cpMRB00, cpMRB00, cpMRB00, cpMRB00, cpMRB00
    };
// x1001xxx xxxxxxxx
int (*decode_tableC8[256])() =
    {
        udef,  udef,  udef,  ldrIB,  pushrB,  poprB,  cplrB,  negrB,
        mulrIB,  mulsrIB, divrIB,  divsrIB, udef,  udef,  udef,  udef,
        daar,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  bios,  udef,  djnzB,  udef,  udef,  udef,
        andcf4rB, orcf4rB, xorcf4rB, ldcf4rB, stcf4rB, udef,  udef,  udef,
        andcfArB, orcfArB, xorcfArB, ldcfArB, stcfArB, udef,  ldccrB,  ldcrcB,
        res4rB,  set4rB,  chg4rB,  bit4rB,  tset4rB, udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //
        mulRrB,  mulRrB,  mulRrB,  mulRrB,  mulRrB,  mulRrB,  mulRrB,  mulRrB,
        mulsRrB, mulsRrB, mulsRrB, mulsRrB, mulsRrB, mulsRrB, mulsRrB, mulsRrB,
        divRrB,  divRrB,  divRrB,  divRrB,  divRrB,  divRrB,  divRrB,  divRrB,
        divsRrB, divsRrB, divsRrB, divsRrB, divsRrB, divsRrB, divsRrB, divsRrB,
        inc3rB,  inc3rB,  inc3rB,  inc3rB,  inc3rB,  inc3rB,  inc3rB,  inc3rB,
        dec3rB,  dec3rB,  dec3rB,  dec3rB,  dec3rB,  dec3rB,  dec3rB,  dec3rB,
        sccB0,  sccB1,  sccB2,  sccB3,  sccB4,  sccB5,  sccB6,  sccB7,
        sccB8,  sccB9,  sccBA,  sccBB,  sccBC,  sccBD,  sccBE,  sccBF,
        //
        addRrB,  addRrB,  addRrB,  addRrB,  addRrB,  addRrB,  addRrB,  addRrB,
        ldRrB,  ldRrB,  ldRrB,  ldRrB,  ldRrB,  ldRrB,  ldRrB,  ldRrB,
        adcRrB,  adcRrB,  adcRrB,  adcRrB,  adcRrB,  adcRrB,  adcRrB,  adcRrB,
        ldrRB,  ldrRB,  ldrRB,  ldrRB,  ldrRB,  ldrRB,  ldrRB,  ldrRB,
        subRrB,  subRrB,  subRrB,  subRrB,  subRrB,  subRrB,  subRrB,  subRrB,
        ldr3B,  ldr3B,  ldr3B,  ldr3B,  ldr3B,  ldr3B,  ldr3B,  ldr3B,
        sbcRrB,  sbcRrB,  sbcRrB,  sbcRrB,  sbcRrB,  sbcRrB,  sbcRrB,  sbcRrB,
        exRrB,  exRrB,  exRrB,  exRrB,  exRrB,  exRrB,  exRrB,  exRrB,
        //
        andRrB,  andRrB,  andRrB,  andRrB,  andRrB,  andRrB,  andRrB,  andRrB,
        addrIB,  adcrIB,  subrIB,  sbcrIB,  andrIB,  xorrIB,  orrIB,  cprIB,
        xorRrB,  xorRrB,  xorRrB,  xorRrB,  xorRrB,  xorRrB,  xorRrB,  xorRrB,
        cpr3B,  cpr3B,  cpr3B,  cpr3B,  cpr3B,  cpr3B,  cpr3B,  cpr3B,
        orRrB,  orRrB,  orRrB,  orRrB,  orRrB,  orRrB,  orRrB,  orRrB,
        rlc4rB,  rrc4rB,  rl4rB,  rr4rB,  sla4rB,  sra4rB,  sll4rB,  srl4rB,
        cpRrB,  cpRrB,  cpRrB,  cpRrB,  cpRrB,  cpRrB,  cpRrB,  cpRrB,
        rlcArB,  rrcArB,  rlArB,  rrArB,  slaArB,  sraArB,  sllArB,  srlArB
    };
// x1010xxx xxxxxxxx
int (*decode_tableD0[256])() =
    {
        udef,  udef,  udef,  udef,  pushwM10, udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  ldw16M10, udef,  udef,  udef,  udef,  udef,  udef,
        ldRM10,  ldRM10,  ldRM10,  ldRM10,  ldRM10,  ldRM10,  ldRM10,  ldRM10,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        exMRW10, exMRW10, exMRW10, exMRW10, exMRW10, exMRW10, exMRW10, exMRW10,
        addwMI10, adcwMI10, subwMI10, sbcwMI10, andwMI10, xorwMI10, orwMI10, cpwMI10,
        //
        mulRMW10, mulRMW10, mulRMW10, mulRMW10, mulRMW10, mulRMW10, mulRMW10, mulRMW10,
        mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10, mulsRMW10,
        divRMW10, divRMW10, divRMW10, divRMW10, divRMW10, divRMW10, divRMW10, divRMW10,
        divsRMW10, divsRMW10, divsRMW10, divsRMW10, divsRMW10, divsRMW10, divsRMW10, divsRMW10,
        incw3M10, incw3M10, incw3M10, incw3M10, incw3M10, incw3M10, incw3M10, incw3M10,
        decw3M10, decw3M10, decw3M10, decw3M10, decw3M10, decw3M10, decw3M10, decw3M10,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        rlcwM10, rrcwM10, rlwM10,  rrwM10,  slawM10, srawM10, sllwM10, srlwM10,
        //
        addRMW10, addRMW10, addRMW10, addRMW10, addRMW10, addRMW10, addRMW10, addRMW10,
        addMRW10, addMRW10, addMRW10, addMRW10, addMRW10, addMRW10, addMRW10, addMRW10,
        adcRMW10, adcRMW10, adcRMW10, adcRMW10, adcRMW10, adcRMW10, adcRMW10, adcRMW10,
        adcMRW10, adcMRW10, adcMRW10, adcMRW10, adcMRW10, adcMRW10, adcMRW10, adcMRW10,
        subRMW10, subRMW10, subRMW10, subRMW10, subRMW10, subRMW10, subRMW10, subRMW10,
        subMRW10, subMRW10, subMRW10, subMRW10, subMRW10, subMRW10, subMRW10, subMRW10,
        sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10, sbcRMW10,
        sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10, sbcMRW10,
        //
        andRMW10, andRMW10, andRMW10, andRMW10, andRMW10, andRMW10, andRMW10, andRMW10,
        andMRW10, andMRW10, andMRW10, andMRW10, andMRW10, andMRW10, andMRW10, andMRW10,
        xorRMW10, xorRMW10, xorRMW10, xorRMW10, xorRMW10, xorRMW10, xorRMW10, xorRMW10,
        xorMRW10, xorMRW10, xorMRW10, xorMRW10, xorMRW10, xorMRW10, xorMRW10, xorMRW10,
        orRMW10, orRMW10, orRMW10, orRMW10, orRMW10, orRMW10, orRMW10, orRMW10,
        orMRW10, orMRW10, orMRW10, orMRW10, orMRW10, orMRW10, orMRW10, orMRW10,
        cpRMW10, cpRMW10, cpRMW10, cpRMW10, cpRMW10, cpRMW10, cpRMW10, cpRMW10,
        cpMRW10, cpMRW10, cpMRW10, cpMRW10, cpMRW10, cpMRW10, cpMRW10, cpMRW10
    };
// x1011xxx xxxxxxxx
int (*decode_tableD8[256])() =
    {
        udef,  udef,  udef,  ldrIW,  pushrW,  poprW,  cplrW,  negrW,
        mulrIW,  mulsrIW, divrIW,  divsrIW, udef,  udef,  bs1f,  bs1b,
        udef,  udef,  extzrW,  extsrW,  paarW,  udef,  mirrr,  udef,
        udef,  mular,  udef,  udef,  djnzW,  udef,  udef,  udef,
        andcf4rW, orcf4rW, xorcf4rW, ldcf4rW, stcf4rW, udef,  udef,  udef,
        andcfArW, orcfArW, xorcfArW, ldcfArW, stcfArW, udef,  ldccrW,  ldcrcW,
        res4rW,  set4rW,  chg4rW,  bit4rW,  tset4rW, udef,  udef,  udef,
        minc1,  minc2,  minc4,  udef,  mdec1,  mdec2,  mdec4,  udef,
        //
        mulRrW,  mulRrW,  mulRrW,  mulRrW,  mulRrW,  mulRrW,  mulRrW,  mulRrW,
        mulsRrW, mulsRrW, mulsRrW, mulsRrW, mulsRrW, mulsRrW, mulsRrW, mulsRrW,
        divRrW,  divRrW,  divRrW,  divRrW,  divRrW,  divRrW,  divRrW,  divRrW,
        divsRrW, divsRrW, divsRrW, divsRrW, divsRrW, divsRrW, divsRrW, divsRrW,
        inc3rW,  inc3rW,  inc3rW,  inc3rW,  inc3rW,  inc3rW,  inc3rW,  inc3rW,
        dec3rW,  dec3rW,  dec3rW,  dec3rW,  dec3rW,  dec3rW,  dec3rW,  dec3rW,
        sccW0,  sccW1,  sccW2,  sccW3,  sccW4,  sccW5,  sccW6,  sccW7,
        sccW8,  sccW9,  sccWA,  sccWB,  sccWC,  sccWD,  sccWE,  sccWF,
        //
        addRrW,  addRrW,  addRrW,  addRrW,  addRrW,  addRrW,  addRrW,  addRrW,
        ldRrW,  ldRrW,  ldRrW,  ldRrW,  ldRrW,  ldRrW,  ldRrW,  ldRrW,
        adcRrW,  adcRrW,  adcRrW,  adcRrW,  adcRrW,  adcRrW,  adcRrW,  adcRrW,
        ldrRW,  ldrRW,  ldrRW,  ldrRW,  ldrRW,  ldrRW,  ldrRW,  ldrRW,
        subRrW,  subRrW,  subRrW,  subRrW,  subRrW,  subRrW,  subRrW,  subRrW,
        ldr3W,  ldr3W,  ldr3W,  ldr3W,  ldr3W,  ldr3W,  ldr3W,  ldr3W,
        sbcRrW,  sbcRrW,  sbcRrW,  sbcRrW,  sbcRrW,  sbcRrW,  sbcRrW,  sbcRrW,
        exRrW,  exRrW,  exRrW,  exRrW,  exRrW,  exRrW,  exRrW,  exRrW,
        //
        andRrW,  andRrW,  andRrW,  andRrW,  andRrW,  andRrW,  andRrW,  andRrW,
        addrIW,  adcrIW,  subrIW,  sbcrIW,  andrIW,  xorrIW,  orrIW,  cprIW,
        xorRrW,  xorRrW,  xorRrW,  xorRrW,  xorRrW,  xorRrW,  xorRrW,  xorRrW,
        cpr3W,  cpr3W,  cpr3W,  cpr3W,  cpr3W,  cpr3W,  cpr3W,  cpr3W,
        orRrW,  orRrW,  orRrW,  orRrW,  orRrW,  orRrW,  orRrW,  orRrW,
        rlc4rW,  rrc4rW,  rl4rW,  rr4rW,  sla4rW,  sra4rW,  sll4rW,  srl4rW,
        cpRrW,  cpRrW,  cpRrW,  cpRrW,  cpRrW,  cpRrW,  cpRrW,  cpRrW,
        rlcArW,  rrcArW,  rlArW,  rrArW,  slaArW,  sraArW,  sllArW,  srlArW
    };
// x1100xxx xxxxxxxx
int (*decode_tableE0[256])() =
    {
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        ldRM20,  ldRM20,  ldRM20,  ldRM20,  ldRM20,  ldRM20,  ldRM20,  ldRM20,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef , udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //
        addRML20, addRML20, addRML20, addRML20, addRML20, addRML20, addRML20, addRML20,
        addMRL20, addMRL20, addMRL20, addMRL20, addMRL20, addMRL20, addMRL20, addMRL20,
        adcRML20, adcRML20, adcRML20, adcRML20, adcRML20, adcRML20, adcRML20, adcRML20,
        adcMRL20, adcMRL20, adcMRL20, adcMRL20, adcMRL20, adcMRL20, adcMRL20, adcMRL20,
        subRML20, subRML20, subRML20, subRML20, subRML20, subRML20, subRML20, subRML20,
        subMRL20, subMRL20, subMRL20, subMRL20, subMRL20, subMRL20, subMRL20, subMRL20,
        sbcRML20, sbcRML20, sbcRML20, sbcRML20, sbcRML20, sbcRML20, sbcRML20, sbcRML20,
        sbcMRL20, sbcMRL20, sbcMRL20, sbcMRL20, sbcMRL20, sbcMRL20, sbcMRL20, sbcMRL20,
        //
        andRML20, andRML20, andRML20, andRML20, andRML20, andRML20, andRML20, andRML20,
        andMRL20, andMRL20, andMRL20, andMRL20, andMRL20, andMRL20, andMRL20, andMRL20,
        xorRML20, xorRML20, xorRML20, xorRML20, xorRML20, xorRML20, xorRML20, xorRML20,
        xorMRL20, xorMRL20, xorMRL20, xorMRL20, xorMRL20, xorMRL20, xorMRL20, xorMRL20,
        orRML20, orRML20, orRML20, orRML20, orRML20, orRML20, orRML20, orRML20,
        orMRL20, orMRL20, orMRL20, orMRL20, orMRL20, orMRL20, orMRL20, orMRL20,
        cpRML20, cpRML20, cpRML20, cpRML20, cpRML20, cpRML20, cpRML20, cpRML20,
        cpMRL20, cpMRL20, cpMRL20, cpMRL20, cpMRL20, cpMRL20, cpMRL20, cpMRL20
    };
// x1101xxx xxxxxxxx
int (*decode_tableE8[256])() =
    {
        udef,  udef,  udef,  ldrIL,  pushrL,  poprL,  udef,  udef,
        udef,  udef,  udef,  udef,  link,  unlk,  udef,  udef,
        udef,  udef,  extzrL,  extsrL,  paarL,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  ldccrL,  ldcrcL,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        inc3rL,  inc3rL,  inc3rL,  inc3rL,  inc3rL,  inc3rL,  inc3rL,  inc3rL,
        dec3rL,  dec3rL,  dec3rL,  dec3rL,  dec3rL,  dec3rL,  dec3rL,  dec3rL,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //
        addRrL,  addRrL,  addRrL,  addRrL,  addRrL,  addRrL,  addRrL,  addRrL,
        ldRrL,  ldRrL,  ldRrL,  ldRrL,  ldRrL,  ldRrL,  ldRrL,  ldRrL,
        adcRrL,  adcRrL,  adcRrL,  adcRrL,  adcRrL,  adcRrL,  adcRrL,  adcRrL,
        ldrRL,  ldrRL,  ldrRL,  ldrRL,  ldrRL,  ldrRL,  ldrRL,  ldrRL,
        subRrL,  subRrL,  subRrL,  subRrL,  subRrL,  subRrL,  subRrL,  subRrL,
        ldr3L,  ldr3L,  ldr3L,  ldr3L,  ldr3L,  ldr3L,  ldr3L,  ldr3L,
        sbcRrL,  sbcRrL,  sbcRrL,  sbcRrL,  sbcRrL,  sbcRrL,  sbcRrL,  sbcRrL,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //
        andRrL,  andRrL,  andRrL,  andRrL,  andRrL,  andRrL,  andRrL,  andRrL,
        addrIL,  adcrIL,  subrIL,  sbcrIL,  andrIL,  xorrIL,  orrIL,  cprIL,
        xorRrL,  xorRrL,  xorRrL,  xorRrL,  xorRrL,  xorRrL,  xorRrL,  xorRrL,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        orRrL,  orRrL,  orRrL,  orRrL,  orRrL,  orRrL,  orRrL,  orRrL,
        rlc4rL,  rrc4rL,  rl4rL,  rr4rL,  sla4rL,  sra4rL,  sll4rL,  srl4rL,
        cpRrL,  cpRrL,  cpRrL,  cpRrL,  cpRrL,  cpRrL,  cpRrL,  cpRrL,
        rlcArL,  rrcArL,  rlArL,  rrArL,  slaArL,  sraArL,  sllArL,  srlArL
    };
// x1110xxx xxxxxxxx
int (*decode_tableF0[256])() =
    {
        //00
        ldMI30,  udef,  ldwMI30, udef,  popM30,  udef,  popwM30, udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  ldM1630, udef,  ldwM1630, udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30, ldaRMW30,
        andcfAM30, orcfAM30, xorcfAM30, ldcfAM30, stcfAM30, udef,  udef,  udef,
        ldaRML30, ldaRML30, ldaRML30, ldaRML30, ldaRML30, ldaRML30, ldaRML30, ldaRML30,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //40
        ldMR30B, ldMR30B, ldMR30B, ldMR30B, ldMR30B, ldMR30B, ldMR30B, ldMR30B,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        ldMR30W, ldMR30W, ldMR30W, ldMR30W, ldMR30W, ldMR30W, ldMR30W, ldMR30W,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        ldMR30L, ldMR30L, ldMR30L, ldMR30L, ldMR30L, ldMR30L, ldMR30L, ldMR30L,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        //80
        andcf3M30, andcf3M30, andcf3M30, andcf3M30, andcf3M30, andcf3M30, andcf3M30, andcf3M30,
        orcf3M30, orcf3M30, orcf3M30, orcf3M30, orcf3M30, orcf3M30, orcf3M30, orcf3M30,
        xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30, xorcf3M30,
        ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30, ldcf3M30,
        stcf3M30, stcf3M30, stcf3M30, stcf3M30, stcf3M30, stcf3M30, stcf3M30, stcf3M30,
        tset3M30, tset3M30, tset3M30, tset3M30, tset3M30, tset3M30, tset3M30, tset3M30,
        res3M30, res3M30, res3M30, res3M30, res3M30, res3M30, res3M30, res3M30,
        set3M30, set3M30, set3M30, set3M30, set3M30, set3M30, set3M30, set3M30,
        //C0
        chg3M30, chg3M30, chg3M30, chg3M30, chg3M30, chg3M30, chg3M30, chg3M30,
        bit3M30, bit3M30, bit3M30, bit3M30, bit3M30, bit3M30, bit3M30, bit3M30,
        jpccM300, jpccM301, jpccM302, jpccM303, jpccM304, jpccM305, jpccM306, jpccM307,
        jpccM308, jpccM309, jpccM30A, jpccM30B, jpccM30C, jpccM30D, jpccM30E, jpccM30F,
        callccM300, callccM301, callccM302, callccM303, callccM304, callccM305, callccM306, callccM307,
        callccM308, callccM309, callccM30A, callccM30B, callccM30C, callccM30D, callccM30E, callccM30F,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
    };

//////////////////////////////////////////////////////////////////////////////
//////////////////////// decode instructions /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int decode80()  // (XWA) (XBC) (XDE) (XHL) (XIX) (XIY) (XIZ) (XSP) scr.B
{
    mem = *cregsL[opcode&7];
    memB = mem_readB(mem);
    lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0x80].decode[lastbyte]++;
#endif
    return decode_table80[lastbyte]();
}

int decode88()  // (XWA+d) (XBC+d) (XDE+d) (XHL+d) (XIX+d) (XIY+d) (XIZ+d) (XSP+d) scr.B
{
    mem = (*cregsL[opcode&7])+(signed char)readbyteSetLastbyte();
    memB = mem_readB(mem);
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0x80].decode[lastbyte]++;
#endif
    return 2 + decode_table80[lastbyte]();
}

int decode90()  // (XWA) (XBC) (XDE) (XHL) (XIX) (XIY) (XIZ) (XSP) scr.W
{
    mem = *cregsL[opcode&7];
    memW = mem_readW(mem);
    lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0x90].decode[lastbyte]++;
#endif
    return decode_table90[lastbyte]();
}

int decode98()  // (XWA+d) (XBC+d) (XDE+d) (XHL+d) (XIX+d) (XIY+d) (XIZ+d) (XSP+d) scr.W
{
    mem = (*cregsL[opcode&7])+(signed char)readbyteSetLastbyte();
    memW = mem_readW(mem);
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0x98].decode[lastbyte]++;
#endif
    return 2 + decode_table98[lastbyte]();
}

int decodeA0()  // (XWA) (XBC) (XDE) (XHL) (XIX) (XIY) (XIZ) (XSP) scr.L
{
    mem = *cregsL[opcode&7];
    memL = mem_readL(mem);
    lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xa0].decode[lastbyte]++;
#endif
    return decode_tableA0[lastbyte]();
}

int decodeA8()  // (XWA+d) (XBC+d) (XDE+d) (XHL+d) (XIX+d) (XIY+d) (XIZ+d) (XSP+d) scr.L
{
    mem = (*cregsL[opcode&7])+(signed char)readbyteSetLastbyte();
    memL = mem_readL(mem);
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xa0].decode[lastbyte]++;
#endif
    return 2 + decode_tableA0[lastbyte]();
}

int decodeB0()  // (XWA) (XBC) (XDE) (XHL) (XIX) (XIY) (XIZ) (XSP) dst
{
    mem = *cregsL[opcode&7];
    lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xb0].decode[lastbyte]++;
#endif
    return decode_tableB0[lastbyte]();
}

int decodeB8()  // (XWA+d) (XBC+d) (XDE+d) (XHL+d) (XIX+d) (XIY+d) (XIZ+d) (XSP+d) dst
{
    mem = (*cregsL[opcode&7])+(signed char)readbyteSetLastbyte();
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xb8].decode[lastbyte]++;
#endif
    return 2 + decode_tableB8[lastbyte]();
}

int decodeBB()  // (XWA+d) (XBC+d) (XDE+d) (XHL+d) (XIX+d) (XIY+d) (XIZ+d) (XSP+d) dst
{
    mem = (*cregsL[opcode&7])+(signed char)readbyteSetLastbyte();
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xb8].decode[lastbyte]++;
#endif
    return 2 + decode_tableB8[lastbyte]();
}

int decodeC0()  // (n)                scr.B
{
    mem = readbyteSetLastbyte();
    memB = mem_readB(mem);
#ifdef TCLS900H_PROFILING
    profile[0xc0].decode[lastbyte]++;
#endif
    return 2 + decode_tableC0[lastbyte]();
}

int decodeC1()  //   (nn)             scr.B
{
    mem = readwordSetLastbyte();
    memB = mem_readB(mem);
#ifdef TCLS900H_PROFILING
    profile[0xc0].decode[lastbyte]++;
#endif
    return 2 + decode_tableC0[lastbyte]();
}

int decodeC2()  //     (nnn)           scr.B
{
    mem = read24SetLastbyte();
    memB = mem_readB(mem);
#ifdef TCLS900H_PROFILING
    profile[0xc0].decode[lastbyte]++;
#endif
    return 3 + decode_tableC0[lastbyte]();
}

int decodeC3()  //       (mem)         scr.B
{
    unsigned char reg = readbyte();
    signed short d16;
    int    retval = 0;

    switch(reg&0x03)
    {
        case 0x00:
        mem = *allregsL[reg];
        retval = 5;
        break;
        case 0x01:
        mem = *allregsL[reg]+(signed short)readword();
        retval = 5;
        break;
        case 0x02:
        break;
        case 0x03:
        switch (reg)
        {
            case 0x03:
            reg = readbyte();
            mem = *allregsL[reg]+(signed char)(*allregsB[readbyte()]);
            //   mem = *allregsL[reg]+*allregsB[readbyte()];
            retval = 8;
            break;
            case 0x07:
            reg = readbyte();
            mem = *allregsL[reg]+(signed short)(*allregsW[readbyte()]);
            //   mem = *allregsL[reg]+*allregsW[readbyte()];
            retval = 8;
            break;
            case 0x13:
            d16 = (signed short)readword();
            mem = gen_regsPC + d16;
            retval = 5;
            break;
            default:
            break;
        }
    }
    memB = mem_readB(mem);
    lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xc0].decode[lastbyte]++;
#endif
    return retval + decode_tableC0[lastbyte]();
}

int decodeC4()  //         (-xrr)       scr.B
{
    unsigned char reg = readbyteSetLastbyte();

    mem = ((*allregsL[reg])-= 1<<(reg&3));  // pre-decrement
    memB = mem_readB(mem);
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xc0].decode[lastbyte]++;
#endif
    return 3 + decode_tableC0[lastbyte]();
}

int decodeC5()  //           (xrr+)     scr.B
{
    unsigned char reg = readbyteSetLastbyte();

    mem = *allregsL[reg];
    memB = mem_readB(mem);
    *allregsL[reg]+= 1<<(reg&3);    // post-increment
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xc0].decode[lastbyte]++;
#endif
    return 3 + decode_tableC0[lastbyte]();
}

int decodeC7()  // r                reg.B
{
    regB = allregsB[readbyteSetLastbyte()];
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xc8].decode[lastbyte]++;
#endif
    return 1 + decode_tableC8[lastbyte]();
}

int decodeC8()  // W  A  B  C  D  E  H  L  reg.B
{
    regB = cregsB[opcode&7];
    lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xc8].decode[lastbyte]++;
#endif
    return decode_tableC8[lastbyte]();
}

int decodeD0()  // (n)                scr.W
{
    mem = readbyteSetLastbyte();
    memW = mem_readW(mem);
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xd0].decode[lastbyte]++;
#endif
    return 2 + decode_tableD0[lastbyte]();
}

int decodeD1()  //   (nn)             scr.W
{
    mem = readwordSetLastbyte();
    memW = mem_readW(mem);
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xd0].decode[lastbyte]++;
#endif
    return 2 + decode_tableD0[lastbyte]();
}

int decodeD2()  //     (nnn)           scr.W
{
    mem = read24SetLastbyte();
    memW = mem_readW(mem);
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xd0].decode[lastbyte]++;
#endif
    return 3 + decode_tableD0[lastbyte]();
}

int decodeD3()  //       (mem)         scr.W
{
    unsigned char reg = readbyte();
    signed short d16;
    int    retval = 0;

    switch(reg&0x03)
    {
        case 0x00:
        mem = *allregsL[reg];
        retval = 5;
        break;
        case 0x01:
        mem = *allregsL[reg]+(signed short)readword();
        retval = 5;
        break;
        case 0x02:
        break;
        case 0x03:
        switch (reg)
        {
            case 0x03:
            reg = readbyte();
            mem = *allregsL[reg]+(signed char)(*allregsB[readbyte()]);
            //   mem = *allregsL[reg]+*allregsB[readbyte()];
            retval = 8;
            break;
            case 0x07:
            reg = readbyte();
            mem = *allregsL[reg]+(signed short)(*allregsW[readbyte()]);
            //   mem = *allregsL[reg]+*allregsW[readbyte()];
            retval = 8;
            break;
            case 0x13:
            d16 = (signed short)readword();
            mem = gen_regsPC + d16;
            retval = 5;
            break;
            default:
            break;
        }
    }
    memW = mem_readW(mem);
    lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xd0].decode[lastbyte]++;
#endif
    return retval + decode_tableD0[lastbyte]();
}

int decodeD4()  //         (-xrr)       scr.W
{
    unsigned char reg = readbyteSetLastbyte();

    mem = ((*allregsL[reg])-= 1<<(reg&3)); // pre-decrement
    memW = mem_readW(mem);
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xd0].decode[lastbyte]++;
#endif
    return 3 + decode_tableD0[lastbyte]();
}

int decodeD5()  //           (xrr+)     scr.W
{
    unsigned char reg = readbyteSetLastbyte();

    mem = (*allregsL[reg]);
    memW = mem_readW(mem);
    *allregsL[reg]+= 1<<(reg&3);   // post-increment
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xd0].decode[lastbyte]++;
#endif
    return 3 + decode_tableD0[lastbyte]();
}

int decodeD7()  // r                reg.W
{
    regW = allregsW[readbyteSetLastbyte()];
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xd8].decode[lastbyte]++;
#endif
    return 1 + decode_tableD8[lastbyte]();
}

int decodeD8()  // WA  BC  DE  HL  IX  IY  IZ  SP  reg.W
{
    regW = cregsW[opcode&7];
    lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xd8].decode[lastbyte]++;
#endif
    return decode_tableD8[lastbyte]();
}

int decodeE0()  // (n)                scr.L
{
    mem = readbyteSetLastbyte();
    memL = mem_readL(mem);
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xe0].decode[lastbyte]++;
#endif
    return 2 + decode_tableE0[lastbyte]();
}

int decodeE1()  //   (nn)             scr.L
{
    mem = readwordSetLastbyte();
    memL = mem_readL(mem);
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xe0].decode[lastbyte]++;
#endif
    return 2 + decode_tableE0[lastbyte]();
}

int decodeE2()  //     (nnn)           scr.L
{
    mem = read24SetLastbyte();
    memL = mem_readL(mem);
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xe0].decode[lastbyte]++;
#endif
    return 3 + decode_tableE0[lastbyte]();
}

int decodeE3()  //       (mem)         scr.L
{
    unsigned char reg = readbyte();
    signed short d16;
    int    retval = 0;

    switch(reg&0x03)
    {
        case 0x00:
        mem = *allregsL[reg];
        retval = 5;
        break;
        case 0x01:
        mem = *allregsL[reg]+(signed short)readword();
        retval = 5;
        break;
        case 0x02:
        break;
        case 0x03:
        switch (reg)
        {
            case 0x03:
            reg = readbyte();
            mem = *allregsL[reg]+(signed char)(*allregsB[readbyte()]);
            //   mem = *allregsL[reg]+*allregsB[readbyte()];
            retval = 8;
            break;
            case 0x07:
            reg = readbyte();
            mem = *allregsL[reg]+(signed short)(*allregsW[readbyte()]);
            //   mem = *allregsL[reg]+*allregsW[readbyte()];
            retval = 8;
            break;
            case 0x13:
            d16 = (signed short)readword();
            mem = gen_regsPC + d16;
            retval = 5;
            break;
            default:
            break;
        }
    }
    memL = mem_readL(mem);
    lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xe0].decode[lastbyte]++;
#endif
    return retval + decode_tableE0[lastbyte]();
}

int decodeE4()  //         (-xrr)       scr.L
{
    unsigned char reg = readbyteSetLastbyte();

    mem = ((*allregsL[reg])-= 1<<(reg&3)); // pre-decrement
    memL = mem_readL(mem);
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xe0].decode[lastbyte]++;
#endif
    return 3 + decode_tableE0[lastbyte]();
}

int decodeE5()  //           (xrr+)     scr.L
{
    unsigned char reg = readbyteSetLastbyte();

    mem = (*allregsL[reg]);
    memL = mem_readL(mem);
    *allregsL[reg]+= 1<<(reg&3);   // post-increment
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xe0].decode[lastbyte]++;
#endif
    return 3 + decode_tableE0[lastbyte]();
}

int decodeE7()  // r                reg.L
{
    regL = allregsL[readbyteSetLastbyte()];
    //lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xe8].decode[lastbyte]++;
#endif
    return 1 + decode_tableE8[lastbyte]();
}

int decodeE8()  // XWA  XBC  XDE  XHL  XIX  XIY  XIZ  XSP  reg.L
{
    regL = cregsL[opcode&7];
    lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xe8].decode[lastbyte]++;
#endif
    return decode_tableE8[lastbyte]();
}

int decodeF0()  // (n)                dst
{
    mem = readbyteSetLastbyte();
#ifdef TCLS900H_PROFILING
    profile[0xf0].decode[lastbyte]++;
#endif
    return 2 + decode_tableF0[lastbyte]();
}

int decodeF1()  //   (nn)             dst
{
    mem = readwordSetLastbyte();
#ifdef TCLS900H_PROFILING
    profile[0xf0].decode[lastbyte]++;
#endif
    return 2 + decode_tableF0[lastbyte]();
}

int decodeF2()  //     (nnn)           dst
{
    mem = read24SetLastbyte();
#ifdef TCLS900H_PROFILING
    profile[0xf0].decode[lastbyte]++;
#endif
    return 3 + decode_tableF0[lastbyte]();
}

int decodeF3()  //       (mem)         dst
{
    unsigned char reg = readbyte();
    signed short d16;
    int    retval = 0;

    switch(reg&0x03)
    {
        case 0x00:
        mem = *allregsL[reg];
        retval = 5;
        break;
        case 0x01:
        mem = *allregsL[reg]+(signed short)readword();
        retval = 5;
        break;
        case 0x02:
        break;
        case 0x03:
        switch(reg)
        {
            case 0x03:
            reg = readbyte();
            mem = *allregsL[reg]+(signed char)(*allregsB[readbyte()]);
            //   mem = *allregsL[reg]+*allregsB[readbyte()];
            retval = 8;
            break;
            case 0x07:
            reg = readbyte();
            mem = *allregsL[reg]+(signed short)(*allregsW[readbyte()]);
            //   mem = *allregsL[reg]+*allregsW[readbyte()];
            retval = 8;
            break;
            case 0x13:
            d16 = (signed short)readword();
            mem = gen_regsPC + d16;
            retval = 5;
            break;
            default:
            break;
        }
    }
    lastbyte = readbyte();
#ifdef TCLS900H_PROFILING
    profile[0xf0].decode[lastbyte]++;
#endif
    return retval + decode_tableF0[lastbyte]();
}

int decodeF4()  //         (-xrr)       dst
{
    unsigned char reg;

    reg = readbyteSetLastbyte();
    mem = (*allregsL[reg]-= 1<<(reg&3));
#ifdef TCLS900H_PROFILING
    profile[0xf0].decode[lastbyte]++;
#endif
    return 3 + decode_tableF0[lastbyte]();
}

int decodeF5()  //           (xrr+)     dst
{
    unsigned char reg;
    int  retval;

    reg = readbyteSetLastbyte();
    mem = (*allregsL[reg]);
#ifdef TCLS900H_PROFILING
    profile[0xf0].decode[lastbyte]++;
#endif
    retval = 3 + decode_tableF0[lastbyte]();
    *allregsL[reg]+= 1<<(reg&3);
    return retval;
}

// main instruction decode table
int (*instr_table[256])()=
    {
        nop,  normal,  pushsr,  popsr,  tmax,  halt,  ei,   reti,
        ld8I,  pushI,  ldw8I,  pushwI,  incf,  decf,  ret,  retd,
        rcf,  scf,  ccf,  zcf,  pushA,  popA,  exFF,  ldf,
        pushF,  popF,  jp16,  jp24,  call16,  call24,  calr,  udef,
        ldRIB,  ldRIB,  ldRIB,  ldRIB,  ldRIB,  ldRIB,  ldRIB,  ldRIB,
        pushRW,  pushRW,  pushRW,  pushRW,  pushRW,  pushRW,  pushRW,  pushRW,
        ldRIW,  ldRIW,  ldRIW,  ldRIW,  ldRIW,  ldRIW,  ldRIW,  ldRIW,
        pushRL,  pushRL,  pushRL,  pushRL,  pushRL,  pushRL,  pushRL,  pushRL,
        //
        ldRIL,  ldRIL,  ldRIL,  ldRIL,  ldRIL,  ldRIL,  ldRIL,  ldRIL,
        popRW,  popRW,  popRW,  popRW,  popRW,  popRW,  popRW,  popRW,
        udef,  udef,  udef,  udef,  udef,  udef,  udef,  udef,
        popRL,  popRL,  popRL,  popRL,  popRL,  popRL,  popRL,  popRL,
        jrcc0,  jrcc1,  jrcc2,  jrcc3,  jrcc4,  jrcc5,  jrcc6,  jrcc7,
        jrcc8,  jrcc9,  jrccA,  jrccB,  jrccC,  jrccD,  jrccE,  jrccF,
        jrlcc0,  jrlcc1,  jrlcc2,  jrlcc3,  jrlcc4,  jrlcc5,  jrlcc6,  jrlcc7,
        jrlcc8,  jrlcc9,  jrlccA,  jrlccB,  jrlccC,  jrlccD,  jrlccE,  jrlccF,
        //
        decode80, decode80, decode80, decode80, decode80, decode80, decode80, decode80,
        decode88, decode88, decode88, decode88, decode88, decode88, decode88, decode88,
        decode90, decode90, decode90, decode90, decode90, decode90, decode90, decode90,
        decode98, decode98, decode98, decode98, decode98, decode98, decode98, decode98,
        decodeA0, decodeA0, decodeA0, decodeA0, decodeA0, decodeA0, decodeA0, decodeA0,
        decodeA8, decodeA8, decodeA8, decodeA8, decodeA8, decodeA8, decodeA8, decodeA8,
        decodeB0, decodeB0, decodeB0, decodeB0, decodeB0, decodeB0, decodeB0, decodeB0,
        decodeB8, decodeB8, decodeB8, decodeBB, decodeB8, decodeB8, decodeB8, decodeB8,
        //
        decodeC0, decodeC1, decodeC2, decodeC3, decodeC4, decodeC5, udef,  decodeC7,
        decodeC8, decodeC8, decodeC8, decodeC8, decodeC8, decodeC8, decodeC8, decodeC8,
        decodeD0, decodeD1, decodeD2, decodeD3, decodeD4, decodeD5, udef,  decodeD7,
        decodeD8, decodeD8, decodeD8, decodeD8, decodeD8, decodeD8, decodeD8, decodeD8,
        decodeE0, decodeE1, decodeE2, decodeE3, decodeE4, decodeE5, udef,  decodeE7,
        decodeE8, decodeE8, decodeE8, decodeE8, decodeE8, decodeE8, decodeE8, decodeE8,
        decodeF0, decodeF1, decodeF2, decodeF3, decodeF4, decodeF5, udef,  ldx,
        swi,  swi,  swi,  swi,  swi,  swi,  swi,  swi
    };

void tlcs_init()
{
    int i,j;

#ifdef TCLS900H_PROFILING

    initTlcs900hProfile();
#endif

    // flags tables initialisation
    for (i = 0; i < 256; i++)
    {
        if (!i)
			Ztable[i] |= ZF;
		SZtable[i] = (i & SF) | Ztable[i];

#ifdef USE_PARITY_TABLE
        int k=0;
        j=i;
        for (int loop=0;loop<8;loop++)
        {
            if (j&1)
                k++;
            j = j>>1;
        }
		parityVtable[i] = ((k&1) ? 0 : VF);
#endif
	}

    // initialize values of all registers
    gen_regsXWA0 = gen_regsXBC0 = gen_regsXDE0 = gen_regsXHL0 = 0;
    gen_regsXWA1 = gen_regsXBC1 = gen_regsXDE1 = gen_regsXHL1 = 0;
    gen_regsXWA2 = gen_regsXBC2 = gen_regsXDE2 = gen_regsXHL2 = 0;
    gen_regsXWA3 = gen_regsXBC3 = gen_regsXDE3 = gen_regsXHL3 = 0;
    gen_regsXIX  = gen_regsXIY  = gen_regsXIZ = 0x00001000;
    // these are the settings for a real 900H, we will set them to what we want <grin>
    gen_regsXSSP = gen_regsXSP  = 0x100;
    gen_regsPC   = mem_readL(0xFFFF00)&0x00ffffff;
    gen_regsSR   = 0;
    F2             = 0;
    for (j=0;j<64;j++)
        ldcRegs[j] = 0;
    // running in MAX and SYSTEM mode
    gen_regsSR = 0xf800;  // IFF 7, MAXM and SYSM
    state = 0;
    checkstate = 0;


    // initialize pointer structure for access to all registers in byte mode
    allregsB[0x00] = (unsigned char *)&gen_regsXWA0;
    allregsB[0x04] = (unsigned char *)&gen_regsXBC0;
    allregsB[0x08] = (unsigned char *)&gen_regsXDE0;
    allregsB[0x0c] = (unsigned char *)&gen_regsXHL0;
    allregsB[0x10] = (unsigned char *)&gen_regsXWA1;
    allregsB[0x14] = (unsigned char *)&gen_regsXBC1;
    allregsB[0x18] = (unsigned char *)&gen_regsXDE1;
    allregsB[0x1c] = (unsigned char *)&gen_regsXHL1;
    allregsB[0x20] = (unsigned char *)&gen_regsXWA2;
    allregsB[0x24] = (unsigned char *)&gen_regsXBC2;
    allregsB[0x28] = (unsigned char *)&gen_regsXDE2;
    allregsB[0x2c] = (unsigned char *)&gen_regsXHL2;
    allregsB[0x30] = (unsigned char *)&gen_regsXWA3;
    allregsB[0x34] = (unsigned char *)&gen_regsXBC3;
    allregsB[0x38] = (unsigned char *)&gen_regsXDE3;
    allregsB[0x3c] = (unsigned char *)&gen_regsXHL3;
    allregsB[0xf0] = (unsigned char *)&gen_regsXIX;
    allregsB[0xf4] = (unsigned char *)&gen_regsXIY;
    allregsB[0xf8] = (unsigned char *)&gen_regsXIZ;
    allregsB[0xfc] = (unsigned char *)&gen_regsXSP;

    for (j=0;j<256;j+=4)
    {
        allregsB[j+1] = allregsB[j]+1;
        allregsB[j+2] = allregsB[j]+2;
        allregsB[j+3] = allregsB[j]+3;
    }

    // initialize pointer structure for access to all registers in word mode
    allregsW[0x00] = (unsigned short *)&gen_regsXWA0;
    allregsW[0x04] = (unsigned short *)&gen_regsXBC0;
    allregsW[0x08] = (unsigned short *)&gen_regsXDE0;
    allregsW[0x0c] = (unsigned short *)&gen_regsXHL0;
    allregsW[0x10] = (unsigned short *)&gen_regsXWA1;
    allregsW[0x14] = (unsigned short *)&gen_regsXBC1;
    allregsW[0x18] = (unsigned short *)&gen_regsXDE1;
    allregsW[0x1c] = (unsigned short *)&gen_regsXHL1;
    allregsW[0x20] = (unsigned short *)&gen_regsXWA2;
    allregsW[0x24] = (unsigned short *)&gen_regsXBC2;
    allregsW[0x28] = (unsigned short *)&gen_regsXDE2;
    allregsW[0x2c] = (unsigned short *)&gen_regsXHL2;
    allregsW[0x30] = (unsigned short *)&gen_regsXWA3;
    allregsW[0x34] = (unsigned short *)&gen_regsXBC3;
    allregsW[0x38] = (unsigned short *)&gen_regsXDE3;
    allregsW[0x3c] = (unsigned short *)&gen_regsXHL3;
    allregsW[0xf0] = (unsigned short *)&gen_regsXIX;
    allregsW[0xf4] = (unsigned short *)&gen_regsXIY;
    allregsW[0xf8] = (unsigned short *)&gen_regsXIZ;
    allregsW[0xfc] = (unsigned short *)&gen_regsXSP;

    for (j=2;j<256;j+=4)
        allregsW[j] = allregsW[j-2]+1;

    // initialize pointer structure for access to all registers in long mode
    allregsL[0x00] = allregsL[0x01] = allregsL[0x02] = allregsL[0x03] = &gen_regsXWA0;
    allregsL[0x04] = allregsL[0x05] = allregsL[0x06] = allregsL[0x07] = &gen_regsXBC0;
    allregsL[0x08] = allregsL[0x09] = allregsL[0x0a] = allregsL[0x0b] = &gen_regsXDE0;
    allregsL[0x0c] = allregsL[0x0d] = allregsL[0x0e] = allregsL[0x0f] = &gen_regsXHL0;
    allregsL[0x10] = allregsL[0x11] = allregsL[0x12] = allregsL[0x13] = &gen_regsXWA1;
    allregsL[0x14] = allregsL[0x15] = allregsL[0x16] = allregsL[0x17] = &gen_regsXBC1;
    allregsL[0x18] = allregsL[0x19] = allregsL[0x1a] = allregsL[0x1b] = &gen_regsXDE1;
    allregsL[0x1c] = allregsL[0x1d] = allregsL[0x1e] = allregsL[0x1f] = &gen_regsXHL1;
    allregsL[0x20] = allregsL[0x21] = allregsL[0x22] = allregsL[0x23] = &gen_regsXWA2;
    allregsL[0x24] = allregsL[0x25] = allregsL[0x26] = allregsL[0x27] = &gen_regsXBC2;
    allregsL[0x28] = allregsL[0x29] = allregsL[0x2a] = allregsL[0x2b] = &gen_regsXDE2;
    allregsL[0x2c] = allregsL[0x2d] = allregsL[0x2e] = allregsL[0x2f] = &gen_regsXHL2;
    allregsL[0x30] = allregsL[0x31] = allregsL[0x32] = allregsL[0x33] = &gen_regsXWA3;
    allregsL[0x34] = allregsL[0x35] = allregsL[0x36] = allregsL[0x37] = &gen_regsXBC3;
    allregsL[0x38] = allregsL[0x39] = allregsL[0x3a] = allregsL[0x3b] = &gen_regsXDE3;
    allregsL[0x3c] = allregsL[0x3d] = allregsL[0x3e] = allregsL[0x3f] = &gen_regsXHL3;
    allregsL[0xf0] = allregsL[0xf1] = allregsL[0xf2] = allregsL[0xf3] = &gen_regsXIX;
    allregsL[0xf4] = allregsL[0xf5] = allregsL[0xf6] = allregsL[0xf7] = &gen_regsXIY;
    allregsL[0xf8] = allregsL[0xf9] = allregsL[0xfa] = allregsL[0xfb] = &gen_regsXIZ;
    allregsL[0xfc] = allregsL[0xfd] = allregsL[0xfe] = allregsL[0xff] = &gen_regsXSP;

    cregsW[4] = allregsW[0xf0];
    cregsW[5] = allregsW[0xf4];
    cregsW[6] = allregsW[0xf8];
    cregsW[7] = allregsW[0xfc];

    cregsL[4] = allregsL[0xf0];
    cregsL[5] = allregsL[0xf4];
    cregsL[6] = allregsL[0xf8];
    cregsL[7] = allregsL[0xfc];

    set_cregs();

    // neogeo pocket color specific settings for running rom dumps
    gen_regsPC = mem_readL(0x0020001c) & 0x00ffffff;

    gen_regsXNSP = gen_regsXSP = 0x00006C00;
    my_pc = get_address(gen_regsPC);

    tlcsClockMulti = 1;
    DMAstate = 0;

    interruptPendingLevel = 0;
    for(i=0; i<7; i++)
    {
        for(j=0; j<INT_QUEUE_MAX; j++)
        {
            pendingInterrupts[i][j] = 0;
        }
    }
    debugIndex = 0;
    debugCount = 1;
    //saved_my_pc = my_pc;
}

void tlcs_reinit()
{
    int j;

    // initialize pointer structure for access to all registers in byte mode
    allregsB[0x00] = (unsigned char *)&gen_regsXWA0;
    allregsB[0x04] = (unsigned char *)&gen_regsXBC0;
    allregsB[0x08] = (unsigned char *)&gen_regsXDE0;
    allregsB[0x0c] = (unsigned char *)&gen_regsXHL0;
    allregsB[0x10] = (unsigned char *)&gen_regsXWA1;
    allregsB[0x14] = (unsigned char *)&gen_regsXBC1;
    allregsB[0x18] = (unsigned char *)&gen_regsXDE1;
    allregsB[0x1c] = (unsigned char *)&gen_regsXHL1;
    allregsB[0x20] = (unsigned char *)&gen_regsXWA2;
    allregsB[0x24] = (unsigned char *)&gen_regsXBC2;
    allregsB[0x28] = (unsigned char *)&gen_regsXDE2;
    allregsB[0x2c] = (unsigned char *)&gen_regsXHL2;
    allregsB[0x30] = (unsigned char *)&gen_regsXWA3;
    allregsB[0x34] = (unsigned char *)&gen_regsXBC3;
    allregsB[0x38] = (unsigned char *)&gen_regsXDE3;
    allregsB[0x3c] = (unsigned char *)&gen_regsXHL3;
    allregsB[0xf0] = (unsigned char *)&gen_regsXIX;
    allregsB[0xf4] = (unsigned char *)&gen_regsXIY;
    allregsB[0xf8] = (unsigned char *)&gen_regsXIZ;
    allregsB[0xfc] = (unsigned char *)&gen_regsXSP;

    for (j=0;j<256;j+=4)
    {
        allregsB[j+1] = allregsB[j]+1;
        allregsB[j+2] = allregsB[j]+2;
        allregsB[j+3] = allregsB[j]+3;
    }

    // initialize pointer structure for access to all registers in word mode
    allregsW[0x00] = (unsigned short *)&gen_regsXWA0;
    allregsW[0x04] = (unsigned short *)&gen_regsXBC0;
    allregsW[0x08] = (unsigned short *)&gen_regsXDE0;
    allregsW[0x0c] = (unsigned short *)&gen_regsXHL0;
    allregsW[0x10] = (unsigned short *)&gen_regsXWA1;
    allregsW[0x14] = (unsigned short *)&gen_regsXBC1;
    allregsW[0x18] = (unsigned short *)&gen_regsXDE1;
    allregsW[0x1c] = (unsigned short *)&gen_regsXHL1;
    allregsW[0x20] = (unsigned short *)&gen_regsXWA2;
    allregsW[0x24] = (unsigned short *)&gen_regsXBC2;
    allregsW[0x28] = (unsigned short *)&gen_regsXDE2;
    allregsW[0x2c] = (unsigned short *)&gen_regsXHL2;
    allregsW[0x30] = (unsigned short *)&gen_regsXWA3;
    allregsW[0x34] = (unsigned short *)&gen_regsXBC3;
    allregsW[0x38] = (unsigned short *)&gen_regsXDE3;
    allregsW[0x3c] = (unsigned short *)&gen_regsXHL3;
    allregsW[0xf0] = (unsigned short *)&gen_regsXIX;
    allregsW[0xf4] = (unsigned short *)&gen_regsXIY;
    allregsW[0xf8] = (unsigned short *)&gen_regsXIZ;
    allregsW[0xfc] = (unsigned short *)&gen_regsXSP;

    for (j=2;j<256;j+=4)
        allregsW[j] = allregsW[j-2]+1;

    // initialize pointer structure for access to all registers in long mode
    allregsL[0x00] = allregsL[0x01] = allregsL[0x02] = allregsL[0x03] = &gen_regsXWA0;
    allregsL[0x04] = allregsL[0x05] = allregsL[0x06] = allregsL[0x07] = &gen_regsXBC0;
    allregsL[0x08] = allregsL[0x09] = allregsL[0x0a] = allregsL[0x0b] = &gen_regsXDE0;
    allregsL[0x0c] = allregsL[0x0d] = allregsL[0x0e] = allregsL[0x0f] = &gen_regsXHL0;
    allregsL[0x10] = allregsL[0x11] = allregsL[0x12] = allregsL[0x13] = &gen_regsXWA1;
    allregsL[0x14] = allregsL[0x15] = allregsL[0x16] = allregsL[0x17] = &gen_regsXBC1;
    allregsL[0x18] = allregsL[0x19] = allregsL[0x1a] = allregsL[0x1b] = &gen_regsXDE1;
    allregsL[0x1c] = allregsL[0x1d] = allregsL[0x1e] = allregsL[0x1f] = &gen_regsXHL1;
    allregsL[0x20] = allregsL[0x21] = allregsL[0x22] = allregsL[0x23] = &gen_regsXWA2;
    allregsL[0x24] = allregsL[0x25] = allregsL[0x26] = allregsL[0x27] = &gen_regsXBC2;
    allregsL[0x28] = allregsL[0x29] = allregsL[0x2a] = allregsL[0x2b] = &gen_regsXDE2;
    allregsL[0x2c] = allregsL[0x2d] = allregsL[0x2e] = allregsL[0x2f] = &gen_regsXHL2;
    allregsL[0x30] = allregsL[0x31] = allregsL[0x32] = allregsL[0x33] = &gen_regsXWA3;
    allregsL[0x34] = allregsL[0x35] = allregsL[0x36] = allregsL[0x37] = &gen_regsXBC3;
    allregsL[0x38] = allregsL[0x39] = allregsL[0x3a] = allregsL[0x3b] = &gen_regsXDE3;
    allregsL[0x3c] = allregsL[0x3d] = allregsL[0x3e] = allregsL[0x3f] = &gen_regsXHL3;
    allregsL[0xf0] = allregsL[0xf1] = allregsL[0xf2] = allregsL[0xf3] = &gen_regsXIX;
    allregsL[0xf4] = allregsL[0xf5] = allregsL[0xf6] = allregsL[0xf7] = &gen_regsXIY;
    allregsL[0xf8] = allregsL[0xf9] = allregsL[0xfa] = allregsL[0xfb] = &gen_regsXIZ;
    allregsL[0xfc] = allregsL[0xfd] = allregsL[0xfe] = allregsL[0xff] = &gen_regsXSP;

    cregsW[4] = allregsW[0xf0];
    cregsW[5] = allregsW[0xf4];
    cregsW[6] = allregsW[0xf8];
    cregsW[7] = allregsW[0xfc];

    cregsL[4] = allregsL[0xf0];
    cregsL[5] = allregsL[0xf4];
    cregsL[6] = allregsL[0xf8];
    cregsL[7] = allregsL[0xfc];

    set_cregs();
    my_pc = get_address(gen_regsPC);

    debugIndex = 0;
    debugCount = 1;
    //saved_my_pc = my_pc;
}

/*int check_pc(unsigned long addr)
{
 if (gen_regsPC==addr) return 1;
 else     return 0;
}*/

void tlcsDMA(unsigned char vector);

// handle receiving of interrupt i
// handle receiving of interrupt i
inline void tlcs_interrupt(int irq)
{
    // list of possible interrupts:
    // 00 - INT0 boot ??
    // 01 - INTAD
    // 02 - INT4 VBlank
    // 03 - INT5 interrupt from z80
    // 04 - INT6
    // 05 - INT7
    // 06 - INTT0
    // 07 - INTT1
    // 08 - INTT2
    // 09 - INTT3
    // 0A - INTTR4
    // 0B - INTTR5
    // 0C - INTTR6
    // 0D - INTTR7
    // 0E - INTRX0
    // 0F - INTTX0
    // 10 - INTRX1
    // 11 - INTTX1
    // 12 - INTTC0
    // 13 - INTTC1
    // 14 - INTTC2
    // 15 - INTTC3
    const static unsigned char vector[0x16] =
        {
            0x28, 0x70, 0x2C, 0x30, 0x34, 0x38,
            0x40, 0x44, 0x48, 0x4C,
            0x50, 0x54, 0x58, 0x5C,
            0x60, 0x64, 0x68, 0x6C,
            0x74, 0x78, 0x7C, 0x80
        };
    const static unsigned char DMAvector[0x16] =
        {
            0x0A, 0x1C, 0x0B, 0x0C, 0x0D, 0x0E,
            0x10, 0x11, 0x12, 0x13,
            0x14, 0x15, 0x16, 0x17,
            0x18, 0x19, 0x1A, 0x1B,
            0x00, 0x00, 0x00, 0x00
        };
    int i;
    int level = ((irq&1) ? ((cpuram[0x70+(irq>>1)])>>4)&7 : (cpuram[0x70+(irq>>1)])&7 );
    if (level==7)
        level = 0;

    // Check for timer3 interrupt => generate irq to z80
    if (irq == 0x09)
    {
        ngpSoundExecute();
        ngpSoundInterrupt();
    }
    if (DMAvector[irq])
        tlcsDMA(DMAvector[irq]);
    if (level)
    {
        // acknowledge that we have an interrupt
        i=0;
        while (i<INT_QUEUE_MAX)
        {
            // check if this interrupt is already pending, if so, ignore it
            if (pendingInterrupts[level-1][i] == vector[irq])
                break;
            // add vector of interrupt to queue
            if (pendingInterrupts[level-1][i] == 0)
            {
                pendingInterrupts[level-1][i] = vector[irq];
                break;
            }
            i++;
        }
        if (level > interruptPendingLevel)
            interruptPendingLevel = level;
    }
}

void tlcs_interrupt_wrapper(int irq)
{
    tlcs_interrupt(irq);
}

#define DMAS0 ((unsigned long *)&ldcRegs[0x00])
#define DMAS1 ((unsigned long *)&ldcRegs[0x04])
#define DMAS2 ((unsigned long *)&ldcRegs[0x08])
#define DMAS3 ((unsigned long *)&ldcRegs[0x0C])
#define DMAD0 ((unsigned long *)&ldcRegs[0x10])
#define DMAD1 ((unsigned long *)&ldcRegs[0x14])
#define DMAD2 ((unsigned long *)&ldcRegs[0x18])
#define DMAD3 ((unsigned long *)&ldcRegs[0x1C])
#define DMAC0 ((unsigned short *)&ldcRegs[0x20])
#define DMAC1 ((unsigned short *)&ldcRegs[0x24])
#define DMAC2 ((unsigned short *)&ldcRegs[0x28])
#define DMAC3 ((unsigned short *)&ldcRegs[0x2C])
#define DMAM0 &ldcRegs[0x22]
#define DMAM1 &ldcRegs[0x26]
#define DMAM2 &ldcRegs[0x2A]
#define DMAM3 &ldcRegs[0x2E]
#define DMA0V cpuram[0x7C]
#define DMA1V cpuram[0x7D]
#define DMA2V cpuram[0x7E]
#define DMA3V cpuram[0x7F]

// handle DMA
inline void tlcsDMAchannel(unsigned char *mode, unsigned long *src, unsigned long *dest, unsigned short *count, unsigned char *vector, int channel)
{
    switch (*mode)
    {
        case 0x00:
        mem_writeB(*dest,mem_readB(*src));
        *dest+= 1;
        DMAstate+= 8;
        break;
        case 0x01:
        mem_writeW(*dest,mem_readW(*src));
        *dest+= 2;
        DMAstate+= 8;
        break;
        case 0x02:
        mem_writeL(*dest,mem_readL(*src));
        *dest+= 4;
        DMAstate+= 12;
        break;
        case 0x04:
        mem_writeB(*dest,mem_readB(*src));
        *dest-= 1;
        DMAstate+= 8;
        break;
        case 0x05:
        mem_writeW(*dest,mem_readW(*src));
        *dest-= 2;
        DMAstate+= 8;
        break;
        case 0x06:
        mem_writeL(*dest,mem_readL(*src));
        *dest-= 4;
        DMAstate+= 12;
        break;
        case 0x08:
        mem_writeB(*dest,mem_readB(*src));
        *src+= 1;
        DMAstate+= 8;
        break;
        case 0x09:
        mem_writeW(*dest,mem_readW(*src));
        *src+= 2;
        DMAstate+= 8;
        break;
        case 0x0A:
        mem_writeL(*dest,mem_readL(*src));
        *src+= 4;
        DMAstate+= 12;
        break;
        case 0x0C:
        mem_writeB(*dest,mem_readB(*src));
        *src-= 1;
        DMAstate+= 8;
        break;
        case 0x0D:
        mem_writeW(*dest,mem_readW(*src));
        *src-= 2;
        DMAstate+= 8;
        break;
        case 0x0E:
        mem_writeL(*dest,mem_readL(*src));
        *src-= 4;
        DMAstate+= 12;
        break;
        case 0x10:
        mem_writeB(*dest,mem_readB(*src));
        DMAstate+= 8;
        break;
        case 0x11:
        mem_writeW(*dest,mem_readW(*src));
        DMAstate+= 8;
        break;
        case 0x12:
        mem_writeL(*dest,mem_readL(*src));
        DMAstate+= 12;
        break;
        case 0x14:
        *src+= 1;
        DMAstate+= 5;
        break;
    }
    *count-= 1;
    if (!*count)
    {
        *vector = 0;
        tlcs_interrupt(0x12 + channel);
    }
}

inline void tlcsDMA(unsigned char vector)
{
    if (vector == DMA0V)
    {
        tlcsDMAchannel(DMAM0, DMAS0, DMAD0, DMAC0, &DMA0V, 0);
        return;
    }
    else if (vector == DMA1V)
    {
        tlcsDMAchannel(DMAM1, DMAS1, DMAD1, DMAC1, &DMA1V, 1);
        return;
    }
    else if (vector == DMA2V)
    {
        tlcsDMAchannel(DMAM2, DMAS2, DMAD2, DMAC2, &DMA2V, 2);
        return;
    }
    else if (vector == DMA3V)
    {
        tlcsDMAchannel(DMAM3, DMAS3, DMAD3, DMAC3, &DMA3V, 3);
        return;
    }
}

// timer related cpu registers
#define T8RUN cpuram[0x20]
#define TREG0 cpuram[0x22]
#define TREG1 cpuram[0x23]
#define T01MOD cpuram[0x24]
#define TREG2 cpuram[0x26]
#define TREG3 cpuram[0x27]
#define T23MOD cpuram[0x28]
int    timer0 = 0;
int    timer1 = 0;
int    timer2 = 0;
int    timer3 = 0;


//Flavor  Something is wrong with this timer code!!!
#define TIMER_BASE_RATE  (240)  //what NeoPop seemed to use
//#define TIMER_BASE_RATE  (64*4)  //what MHE originally used
//#define TIMER_BASE_RATE  (56*2)  //this seems to be okay for PacMan, but CFC2 is horribly sped up

#define TIMER_T1_RATE  (1 * TIMER_BASE_RATE)
#define TIMER_T4_RATE  (4 * TIMER_BASE_RATE)
#define TIMER_T16_RATE  (16 * TIMER_BASE_RATE)
#define TIMER_T256_RATE  (256 * TIMER_BASE_RATE)


// only timer mode 00, and maybe the other two 8 bit timer modes as well
// 16bit combined timer mode not supported yet (might be easy to implement though)
inline void tlcs1Timer(int stateChange, int timerNr, int run, int mode,
                       int *t0, int *t1,
                       int check0, int check1)
{
    int overflow0 = 0, overflow1 = 0;

    if (run&1)
    {
        if (!check0)
            check0 = 256;
        switch(mode&3)
        {
            //Flavor hacking according to NeoPop
            case 0:
            if(timerNr==0)
            {
                if (*t0 >= check0)
                {
                    overflow0 = 1;
                    *t0-= check0;
                    //horiz interrupt?
                }
            }
            //else nothing
            break;
            case 1:
            *t0+= stateChange;
            if(timerNr==0)
            {
                if (*t0 >= check0*TIMER_T1_RATE)
                {
                    overflow0 = 1;
                    *t0-= check0*TIMER_T1_RATE;
                }
            }
            else
            {
                if (*t0 >= check0*56) //HACK - Fixes DAC
                {
                    overflow0 = 1;
                    *t0-= check0*56;
                }
            }
            break;
            case 2:
            *t0+= stateChange;
            if (*t0 >= check0*TIMER_T4_RATE)
            {
                overflow0 = 1;
                *t0-= check0*TIMER_T4_RATE;
            }
            break;
            case 3:
            *t0+= stateChange;
            if (*t0 >= check0*TIMER_T16_RATE)
            {
                overflow0 = 1;
                *t0-= check0*TIMER_T16_RATE;
            }
            break;
        }
        if (overflow0)
            tlcs_interrupt(6+timerNr);
    }
    else
        *t0 = 0;


    if (run&2)
    {
        if (!check1)
            check1 = 256;
        switch((mode>>2)&3)
        {
            case 0:
            *t1+= overflow0;
            if (*t1 >= check1)
            {
                overflow1 = 1;
                *t1-= check1;
            }
            break;
            case 1:
            *t1+= stateChange;
            if (*t1 >= check1*TIMER_T1_RATE)
            {
                overflow1 = 1;
                *t1-= check1*TIMER_T1_RATE;
            }
            break;
            case 2:
            *t1+= stateChange;
            if (*t1 >= check1*TIMER_T16_RATE)
            {
                overflow1 = 1;
                *t1-= check1*TIMER_T16_RATE;
            }
            break;
            case 3:
            *t1+= stateChange;
            if (*t1 >= check1*TIMER_T256_RATE)
            {
                overflow1 = 1;
                *t1-= check1*TIMER_T256_RATE;
            }
            break;
        }
        if (overflow1)
            tlcs_interrupt(7+timerNr);
    }
    else
        *t1 = 0;
}

// check and perform timer functions
inline void tlcsTimers(int stateChange)
{
    //Flavor, unroll these two calls into one function

    // // check for updates
    // if (0) {
    //  TREG0 = cpuram[0x22] ? cpuram[0x22] : 256;
    //  TREG1 = cpuram[0x23] ? cpuram[0x23] : 256;
    //  TREG2 = cpuram[0x26] ? cpuram[0x26] : 256;
    //  TREG3 = cpuram[0x27] ? cpuram[0x27] : 256;
    // }
    // timer 0 + 1
    tlcs1Timer(stateChange, 0, T8RUN&3, T01MOD, &timer0, &timer1, TREG0, TREG1);
    // timer 2 + 3
    tlcs1Timer(stateChange, 2, (T8RUN>>2)&3, T23MOD, &timer2, &timer3, TREG2, TREG3);
}

inline void tlcsTI0()
{
    if (((T01MOD & 3) == 0) && (T8RUN & 1))
        timer0+= 1;
}

inline int tlcs_step()
{
    int clocks = DMAstate;
#ifdef TCLS900H_PROFILING

    unsigned int startTime = 0;
#endif

    DMAstate = 0;
    memoryCycles = 0;

    // check and handle a pending interrupt
    if (interruptPendingLevel > (unsigned char)((gen_regsSR & 0x7000)>>12))
    {
        int i;
        // push PC
		tlcsFastMemWriteL(gen_regsXSP-=4,gen_regsPC);
        // push SR
		tlcsFastMemWriteW(gen_regsXSP-=2,gen_regsSR);
        gen_regsSR = (gen_regsSR & 0x8fff) | (interruptPendingLevel<<12);
        gen_regsPC = mem_readL(0x00FFFF00 + pendingInterrupts[interruptPendingLevel-1][0]);
        my_pc = get_address(gen_regsPC);

        // remove interrupt vector from interrupt queue
        for(i=1; i<INT_QUEUE_MAX; i++)
        {
            pendingInterrupts[interruptPendingLevel-1][i-1] =
                pendingInterrupts[interruptPendingLevel-1][i];
        }

        pendingInterrupts[interruptPendingLevel-1][INT_QUEUE_MAX-1] = 0;
        // calculate new interruptPendingLevel
        interruptPendingLevel = 0;

        for(i=6; i>=0; i--)
        {
            if (pendingInterrupts[i][0] != 0)
            {
                interruptPendingLevel = i+1;
                break;
            }
        }
        clocks+= 18;
    }

    opcode = readbyte();

    //printf("tlcs_step: PC=0x%X opcode=0x%X\n", gen_regsPC-1, opcode);


#ifdef TCLS900H_PROFILING

    startTime = SDL_GetTicks();
#endif

    clocks+= instr_table[opcode]();

#ifdef TCLS900H_PROFILING

    profile[opcode].ticks += SDL_GetTicks() - startTime;
    profile[opcode].calls++;
#endif

    clocks+= memoryCycles;

    // Timer processing
    //tlcsTimers(clocks); //Flavor:  should it be (tlcsClockMulti*clocks)???

    return /*tlcsClockMulti * */ clocks;
}


//extern unsigned char *ngpScX;
extern unsigned char *ngpScY;
int ngOverflow = 0;

#define FRAMESKIP    //undef this to do no FRAME skipping

#ifdef FRAMESKIP
//#define AUTO_FRAMESKIP
#define FIXED_FRAMESKIP 0
#define MAX_SKIPFRAMES 1
#endif

#ifdef AUTO_FRAMESKIP
inline void tlcs_execute(int cycles, int skipFrames)// skipFrames=how many frames to skip for each frame rendered
#else
inline void tlcs_execute(int cycles)
#endif
{
    int elapsed;
    int hCounter = ngOverflow;

#ifdef FRAMESKIP
#ifdef FIXED_FRAMESKIP

    static int frame=FIXED_FRAMESKIP;
#else

    static int frame=1;
#endif
#endif

#ifdef TCLS900H_PROFILING

    static unsigned int steps=1;
#endif

    UpdateInputState();

    while(cycles > 0)
    {
#ifdef __GP32__
		int maxc = turbo ? (515>>(tlcsClockMulti-1)) : 56;
	    for (elapsed = tlcs_step();elapsed<maxc; elapsed += tlcs_step());
#else
        if (1)
        {
            //call a bunch of steps
            for (elapsed = tlcs_step();elapsed<(515>>(tlcsClockMulti-1)); elapsed += tlcs_step());
        }
        else
        {
            //call enough steps that would fit in the smallest timer increment
            for (elapsed = tlcs_step();elapsed<56; elapsed += tlcs_step());
        }
#endif

        tlcsTimers(elapsed);
        elapsed*=tlcsClockMulti;
        soundStep(elapsed);

        hCounter-= elapsed;
        //        *ngpScX = hCounter>>2;

#ifdef TCLS900H_PROFILING

        if(steps++ == 25000000)
        {
            m_bIsActive = false;
            break;
        }
#endif

        if (hCounter < 0)
        {
            // time equivalent to 1 horizontal line has passed
#ifdef FRAMESKIP
            //graphicsBlitLine(frame == 0);
            myGraphicsBlitLine(frame==0);
#else

            //graphicsBlitLine(true);
            myGraphicsBlitLine(true);
#endif

            ngpSoundExecute();
            hCounter+= 515;
            // now check what needs to be done at the
            // beginning of this new line
            if (*scanlineY < 151 || *scanlineY == 198)
            {
                // HBlank
                if (tlcsMemReadB(0x8000)&0x40)
                    tlcsTI0();
            }
            else if (*scanlineY == 152)
            {
                //system_sound_update(1);//Flavor

                // VBlank
                if (tlcsMemReadB(0x8000)&0x80)
                    tlcs_interrupt(2);
#ifdef FRAMESKIP
#ifdef FIXED_FRAMESKIP

                if(frame == 0)
#ifdef __GP32__
                    frame = fskip;
#else
                    frame = FIXED_FRAMESKIP;
#endif
#else
                if(frame == 0)
                {
                    frame = skipFrames;
                    SDL_Rect numRect = drawNumber(skipFrames, 10, 24);
                    SDL_UpdateRect(screen, numRect.x, numRect.y, numRect.w, numRect.h);
                }
#endif
                else
                    --frame;
            }
#endif

        }
        cycles-= elapsed;
    }
    ngOverflow = hCounter + cycles;

    //graphics_paint();  //paint the screen, if it's dirty

    //MHE used to sound update here!?!?

    return;
}


//Flavor, this auto-frameskip code is messed up
void ngpc_run()
{
    int currTick=0,lastTick=0;
#ifdef AUTO_FRAMESKIP

    unsigned int skipFrames=0;
#endif

    while(m_bIsActive)  //should be some way to exit
    {
#ifndef __GP32__
        currTick = SDL_GetTicks();
        //if ((currTick - lastTick) >= (1000/m_emuInfo.fps))
        if((currTick - lastTick) < (1000/HOST_FPS))
        {
#ifdef AUTO_FRAMESKIP
            if(skipFrames)
            {
                --skipFrames;
            }
#endif
            while((currTick - lastTick) < (1000/HOST_FPS))
            {
                currTick = SDL_GetTicks();
            }
        }
#ifdef AUTO_FRAMESKIP
        else
        {
            if(skipFrames<MAX_SKIPFRAMES) //cap skipFrames
                skipFrames++;
        }
#endif
        lastTick = currTick;
#endif

#ifdef AUTO_FRAMESKIP

        tlcs_execute((6*1024*1024) / HOST_FPS, skipFrames);
#else

        tlcs_execute((6*1024*1024) / HOST_FPS);
#endif

    }

#ifdef TCLS900H_PROFILING
    printTlcs900hProfile();
#endif
}
