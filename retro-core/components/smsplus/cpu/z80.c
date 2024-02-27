/*****************************************************************************
 *
 *   z80.c
 *   Portable Z80 emulator V3.9
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *   TODO:
 *    - If LD A,I or LD A,R is interrupted, P/V flag gets reset, even if IFF2
 *      was set before this instruction
 *    - Ideally, the tiny differences between Z80 types should be supported,
 *      currently known differences:
 *       - LD A,I/R P/V flag reset glitch is fixed on CMOS Z80
 *       - OUT (C),0 outputs 0 on NMOS Z80, $FF on CMOS Z80
 *       - SCF/CCF X/Y flags is ((flags | A) & 0x28) on SGS/SHARP/ZiLOG NMOS Z80,
 *         (flags & A & 0x28) on NEC NMOS Z80, other models unknown.
 *         However, people from the Speccy scene mention that SCF/CCF X/Y results
 *         are inconsistant and may be influenced by I and R registers.
 *      This Z80 emulator assumes a ZiLOG NMOS model.
 *
 *   Additional changes [Eke-Eke]:
 *    - Discarded multi-chip support (unused)
 *    - Fixed cycle counting for FD and DD prefixed instructions
 *    - Fixed behavior of chained FD and DD prefixes (R register should be only incremented by one
 *    - Implemented cycle-accurate INI/IND (needed by SMS emulation)
 *   Changes in 3.9:
 *    - Fixed cycle counts for LD IYL/IXL/IYH/IXH,n [Marshmellow]
 *    - Fixed X/Y flags in CCF/SCF/BIT, ZEXALL is happy now [hap]
 *    - Simplified DAA, renamed MEMPTR (3.8) to WZ, added TODO [hap]
 *    - Fixed IM2 interrupt cycles [eke]
 *   Changes in 3.8 [Miodrag Milanovic]:
 *   - Added MEMPTR register (according to informations provided
 *     by Vladimir Kladov
 *   - BIT n,(HL) now return valid values due to use of MEMPTR
 *   - Fixed BIT 6,(XY+o) undocumented instructions
 *   Changes in 3.7 [Aaron Giles]:
 *   - Changed NMI handling. NMIs are now latched in set_irq_state
 *     but are not taken there. Instead they are taken at the start of the
 *     execute loop.
 *   - Changed IRQ handling. IRQ state is set in set_irq_state but not taken
 *     except during the inner execute loop.
 *   - Removed x86 assembly hacks and obsolete timing loop catchers.
 *   Changes in 3.6:
 *   - Got rid of the code that would inexactly emulate a Z80, i.e. removed
 *     all the #if Z80_EXACT #else branches.
 *   - Removed leading underscores from local register name shortcuts as
 *     this violates the C99 standard.
 *   - Renamed the registers inside the Z80 context to lower case to avoid
 *     ambiguities (shortcuts would have had the same names as the fields
 *     of the structure).
 *   Changes in 3.5:
 *   - Implemented OTIR, INIR, etc. without look-up table for PF flag.
 *     [Ramsoft, Sean Young]
 *   Changes in 3.4:
 *   - Removed Z80-MSX specific code as it's not needed any more.
 *   - Implemented DAA without look-up table [Ramsoft, Sean Young]
 *   Changes in 3.3:
 *   - Fixed undocumented flags XF & YF in the non-asm versions of CP,
 *     and all the 16 bit arithmetic instructions. [Sean Young]
 *   Changes in 3.2:
 *   - Fixed undocumented flags XF & YF of RRCA, and CF and HF of
 *     INI/IND/OUTI/OUTD/INIR/INDR/OTIR/OTDR [Sean Young]
 *   Changes in 3.1:
 *   - removed the REPEAT_AT_ONCE execution of LDIR/CPIR etc. opcodes
 *     for readabilities sake and because the implementation was buggy
 *     (and I was not able to find the difference)
 *   Changes in 3.0:
 *   - 'finished' switch to dynamically overrideable cycle count tables
 *   Changes in 2.9:
 *   - added methods to access and override the cycle count tables
 *   - fixed handling and timing of multiple DD/FD prefixed opcodes
 *   Changes in 2.8:
 *   - OUTI/OUTD/OTIR/OTDR also pre-decrement the B register now.
 *     This was wrong because of a bug fix on the wrong side
 *     (astrocade sound driver).
 *   Changes in 2.7:
 *    - removed z80_vm specific code, it's not needed (and never was).
 *   Changes in 2.6:
 *    - BUSY_LOOP_HACKS needed to call change_pc() earlier, before
 *    checking the opcodes at the new address, because otherwise they
 *    might access the old (wrong or even NULL) banked memory region.
 *    Thanks to Sean Young for finding this nasty bug.
 *   Changes in 2.5:
 *    - Burning cycles always adjusts the ICount by a multiple of 4.
 *    - In REPEAT_AT_ONCE cases the R register wasn't incremented twice
 *    per repetition as it should have been. Those repeated opcodes
 *    could also underflow the ICount.
 *    - Simplified TIME_LOOP_HACKS for BC and added two more for DE + HL
 *    timing loops. I think those hacks weren't endian safe before too.
 *   Changes in 2.4:
 *    - z80_reset zaps the entire context, sets IX and IY to 0xffff(!) and
 *    sets the Z flag. With these changes the Tehkan World Cup driver
 *    _seems_ to work again.
 *   Changes in 2.3:
 *    - External termination of the execution loop calls z80_burn() and
 *    z80_vm_burn() to burn an amount of cycles (R adjustment)
 *    - Shortcuts which burn CPU cycles (BUSY_LOOP_HACKS and TIME_LOOP_HACKS)
 *    now also adjust the R register depending on the skipped opcodes.
 *   Changes in 2.2:
 *    - Fixed bugs in CPL, SCF and CCF instructions flag handling.
 *    - Changed variable EA and ARG16() function to UINT32; this
 *    produces slightly more efficient code.
 *    - The DD/FD XY CB opcodes where XY is 40-7F and Y is not 6/E
 *    are changed to calls to the X6/XE opcodes to reduce object size.
 *    They're hardly ever used so this should not yield a speed penalty.
 *   New in 2.0:
 *    - Optional more exact Z80 emulation (#define Z80_EXACT 1) according
 *    to a detailed description by Sean Young which can be found at:
 *      http://www.msxnet.org/tech/z80-documented.pdf
 *****************************************************************************/
#include "shared.h"
#include "z80.h"

/* Show debugging messages */
#define VERBOSE 0

#define ALWAYS_INLINE static inline __attribute__((always_inline))
#define NO_INLINE static __attribute__((noinline))

#if VERBOSE
#define LOG(x)  logerror x
#else
#define LOG(x)
#endif

unsigned char *cpu_readmap[64];
unsigned char *cpu_writemap[64];

int z80_cycle_count = 0;        /* running total of cycles executed */

#define CF  0x01
#define NF  0x02
#define PF  0x04
#define VF  PF
#define XF  0x08
#define HF  0x10
#define YF  0x20
#define ZF  0x40
#define SF  0x80

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

#define PCD  Z80.pc.d
#define PC Z80.pc.w.l

#define SPD Z80.sp.d
#define SP Z80.sp.w.l

#define AFD Z80.af.d
#define AF Z80.af.w.l
#define A Z80.af.b.h
#define F Z80.af.b.l

#define BCD Z80.bc.d
#define BC Z80.bc.w.l
#define B Z80.bc.b.h
#define C Z80.bc.b.l

#define DED Z80.de.d
#define DE Z80.de.w.l
#define D Z80.de.b.h
#define E Z80.de.b.l

#define HLD Z80.hl.d
#define HL Z80.hl.w.l
#define H Z80.hl.b.h
#define L Z80.hl.b.l

#define IXD Z80.ix.d
#define IX Z80.ix.w.l
#define HX Z80.ix.b.h
#define LX Z80.ix.b.l

#define IYD Z80.iy.d
#define IY Z80.iy.w.l
#define HY Z80.iy.b.h
#define LY Z80.iy.b.l

#define WZ   Z80.wz.w.l
#define WZ_H Z80.wz.b.h
#define WZ_L Z80.wz.b.l

#define I Z80.i
#define R Z80.r
#define R2 Z80.r2
#define IM Z80.im
#define IFF1 Z80.iff1
#define IFF2 Z80.iff2
#define HALT Z80.halt

Z80_Regs Z80;

static int z80_ICount = 0;
static int z80_exec = 0;               /* 1= in exec loop, 0= out of */
static int z80_requested_cycles = 0;   /* requested cycles to execute this timeslice */

static UINT32 EA;

#define SZ(i) ((i) ? ((i) & (SF|YF|XF)) : (ZF))   /* zero and sign flags */
#define SZ_BIT(i) ((i) ? ((i) & SF) : (ZF|PF))    /* zero, sign and parity/overflow (=zero) flags for BIT opcode */

static UINT8 SZP[256];      /* zero, sign and parity flags */
static UINT8 *SZHVC_add = 0;
static UINT8 *SZHVC_sub = 0;

static const UINT8 cc[6][0x100] = {
  { // Z80_TABLE_op
    4,10, 7, 6, 4, 4, 7, 4, 4,11, 7, 6, 4, 4, 7, 4,
    8,10, 7, 6, 4, 4, 7, 4,12,11, 7, 6, 4, 4, 7, 4,
    7,10,16, 6, 4, 4, 7, 4, 7,11,16, 6, 4, 4, 7, 4,
    7,10,13, 6,11,11,10, 4, 7,11,13, 6, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    7, 7, 7, 7, 7, 7, 4, 7, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    5,10,10,10,10,11, 7,11, 5,10,10, 0,10,17, 7,11,
    5,10,10,11,10,11, 7,11, 5, 4,10,11,10, 0, 7,11,
    5,10,10,19,10,11, 7,11, 5, 4,10, 4,10, 0, 7,11,
    5,10,10, 4,10,11, 7,11, 5, 6,10, 4,10, 0, 7,11
  },
  { // Z80_TABLE_cb
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
    8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
    8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
    8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8
  },
  { // Z80_TABLE_ed
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    12,12,15,20, 8,14, 8, 9,12,12,15,20, 8,14, 8, 9,
    12,12,15,20, 8,14, 8, 9,12,12,15,20, 8,14, 8, 9,
    12,12,15,20, 8,14, 8,18,12,12,15,20, 8,14, 8,18,
    12,12,15,20, 8,14, 8, 8,12,12,15,20, 8,14, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,
    16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
 },
 { // Z80_TABLE_xy                            /* illegal combo should return 4 + cc_op[i] */
    8,14,11,10, 8, 8,11, 8, 8,15,11,10, 8, 8,11, 8,
    12,14,11,10, 8, 8,11, 8,16,15,11,10, 8, 8,11, 8,
    11,14,20,10, 9, 9,12, 8,11,15,20,10, 9, 9,12, 8,
    11,14,17,10,23,23,19, 8,11,15,17,10, 8, 8,11, 8,
    8, 8, 8, 8, 9, 9,19, 8, 8, 8, 8, 8, 9, 9,19, 8,
    8, 8, 8, 8, 9, 9,19, 8, 8, 8, 8, 8, 9, 9,19, 8,
    9, 9, 9, 9, 9, 9,19, 9, 9, 9, 9, 9, 9, 9,19, 9,
    19,19,19,19,19,19, 8,19, 8, 8, 8, 8, 9, 9,19, 8,
    8, 8, 8, 8, 9, 9,19, 8, 8, 8, 8, 8, 9, 9,19, 8,
    8, 8, 8, 8, 9, 9,19, 8, 8, 8, 8, 8, 9, 9,19, 8,
    8, 8, 8, 8, 9, 9,19, 8, 8, 8, 8, 8, 9, 9,19, 8,
    8, 8, 8, 8, 9, 9,19, 8, 8, 8, 8, 8, 9, 9,19, 8,
    9,14,14,14,14,15,11,15, 9,14,14, 0,14,21,11,15,
    9,14,14,15,14,15,11,15, 9, 8,14,15,14, 4,11,15,
    9,14,14,23,14,15,11,15, 9, 8,14, 8,14, 4,11,15,
    9,14,14, 8,14,15,11,15, 9,10,14, 8,14, 4,11,15
  },
  { // Z80_TABLE_xycb
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
    20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
    20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
    20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
    20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23
  },
  { // Z80_TABLE_ex                     /* extra cycles if jr/jp/call taken and 'interrupt latency' on rst 0-7 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* DJNZ */
    5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,  /* JR NZ/JR Z */
    5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,  /* JR NC/JR C */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0,  /* INI/IND (cycle-accurate I/O port reads) */
    5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0,  /* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
    6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
    6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
    6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
    6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2
  },
};

/***************************************************************
 * define an opcode function
 ***************************************************************/
#define OP(opcode, code)  case opcode: code; return;

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define CC(prefix,opcode) z80_ICount -= cc[Z80_TABLE_##prefix][opcode]

/***************************************************************
 * Enter HALT state; write 1 to fake port on first execution
 ***************************************************************/
#define ENTER_HALT {                          \
  PC--;                                       \
  HALT = 1;                                   \
}

/***************************************************************
 * Leave HALT state; write 0 to fake port
 ***************************************************************/
#define LEAVE_HALT {                          \
  if( HALT ) {                                \
    HALT = 0;                                 \
    PC++;                                     \
  }                                           \
}

/***************************************************************
 * Input a byte from given I/O port
 ***************************************************************/
#define IN(port) Z80.port_read(port)

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
#define OUT(port,value) Z80.port_write(port, value)

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
#define RM(addr) (cpu_readmap[(addr) >> 10][(addr) & 0x03FF])
// #define RM(addr) Z80.mem_read(addr)

/***************************************************************
 * Read a word from given memory location
 ***************************************************************/
#define RM16(addr, r) {                         \
  ((PAIR*)r)->b.l = RM(addr);                   \
  ((PAIR*)r)->b.h = RM((addr+1)&0xffff);        \
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
#define WM(addr,value) Z80.mem_write(addr, value)

/***************************************************************
 * Write a word to given memory location
 ***************************************************************/
#define WM16(addr, r) {                         \
  WM(addr, ((PAIR*)r)->b.l);                    \
  WM((addr+1)&0xffff, ((PAIR*)r)->b.h);         \
}

/***************************************************************
 * ROP() is identical to RM() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
#define ROP() ({                                \
  UINT16 pc = PCD;                              \
  PC++;                                         \
  RM(pc);                                       \
})

/****************************************************************
 * ARG() is identical to ROP() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 ***************************************************************/
#define ARG() ({                                \
  UINT16 pc = PCD;                              \
  PC++;                                         \
  RM(pc);                                       \
})

#define ARG16() ((UINT32)(ARG() | ARG() << 8))

/***************************************************************
 * Calculate the effective address EA of an opcode using
 * IX+offset resp. IY+offset addressing.
 ***************************************************************/
#define EAX()  { EA = (UINT32)(UINT16)(IX + (INT8)ARG()); WZ = EA; }
#define EAY()  { EA = (UINT32)(UINT16)(IY + (INT8)ARG()); WZ = EA; }

/***************************************************************
 * POP
 ***************************************************************/
#define POP(DR) { RM16( SPD, &Z80.DR ); SP += 2; }

/***************************************************************
 * PUSH
 ***************************************************************/
#define PUSH(SR) { SP -= 2; WM16( SPD, &Z80.SR ); }

/***************************************************************
 * JP
 ***************************************************************/
#define JP {                                    \
  PCD = ARG16();                                \
  WZ = PCD;                                     \
}

/***************************************************************
 * JP_COND
 ***************************************************************/
#define JP_COND(cond) {                         \
  if (cond)                                     \
  {                                             \
    PCD = ARG16();                              \
    WZ = PCD;                                   \
  }                                             \
  else                                          \
  {                                             \
    WZ = ARG16(); /* implicit do PC += 2 */     \
  }                                             \
}

/***************************************************************
 * JR
 ***************************************************************/
#define JR() {                                              \
  INT8 arg = (INT8)ARG(); /* ARG() also increments PC */    \
  PC += arg;        /* so don't do PC += ARG() */           \
  WZ = PC;                                                  \
}

/***************************************************************
 * JR_COND
 ***************************************************************/
#define JR_COND(cond, opcode) {   \
  if (cond)                       \
  {                               \
    JR();                         \
    CC(ex, opcode);               \
  }                               \
  else PC++;                      \
}

/***************************************************************
 * CALL
 ***************************************************************/
#define CALL() {                  \
  EA = ARG16();                   \
  WZ = EA;                        \
  PUSH(pc);                       \
  PCD = EA;                       \
}

/***************************************************************
 * CALL_COND
 ***************************************************************/
#define CALL_COND(cond, opcode) { \
  if (cond)                       \
  {                               \
    EA = ARG16();                 \
    WZ = EA;                      \
    PUSH(pc);                     \
    PCD = EA;                     \
    CC(ex, opcode);               \
  }                               \
  else                            \
  {                               \
    WZ = ARG16();  /* implicit call PC+=2;   */ \
  }                               \
}

/***************************************************************
 * RET_COND
 ***************************************************************/
#define RET_COND(cond, opcode) {    \
  if (cond)                         \
  {                                 \
    POP(pc);                        \
    WZ = PC;                        \
    CC(ex, opcode);                 \
  }                                 \
}

/***************************************************************
 * RETN
 ***************************************************************/
#define RETN { \
  LOG(("Z80 #%d RETN IFF1:%d IFF2:%d\n", cpu_getactivecpu(), IFF1, IFF2)); \
  POP( pc ); \
  WZ = PC; \
  IFF1 = IFF2; \
}

/***************************************************************
 * RETI
 ***************************************************************/
#define RETI { \
  POP( pc ); \
  WZ = PC; \
/* according to http://www.msxnet.org/tech/z80-documented.pdf */ \
  IFF1 = IFF2; \
}

/***************************************************************
 * LD  R,A
 ***************************************************************/
#define LD_R_A {                        \
  R = A;                                \
  R2 = A & 0x80; /* keep bit 7 of R */  \
}

/***************************************************************
 * LD  A,R
 ***************************************************************/
#define LD_A_R {                         \
  A = (R & 0x7f) | R2;                   \
  F = (F & CF) | SZ(A) | ( IFF2 << 2 );  \
}

/***************************************************************
 * LD  I,A
 ***************************************************************/
#define LD_I_A {                        \
  I = A;                                \
}

/***************************************************************
 * LD  A,I
 ***************************************************************/
#define LD_A_I {                         \
  A = I;                                 \
  F = (F & CF) | SZ(A) | ( IFF2 << 2 );  \
}

/***************************************************************
 * RST
 ***************************************************************/
#define RST(addr)                         \
  PUSH( pc );                             \
  PCD = addr;                             \
  WZ = PC;                                \

/***************************************************************
 * INC  r8
 ***************************************************************/
#define INC(value) ({                     \
  UINT8 res = ((value) + 1);              \
  F = (F & CF) | SZ(res);                 \
  if (res == 0x80) F |= VF;               \
  else if ((res & 0x0f) == 0x00) F |= HF; \
  res;                                    \
})

/***************************************************************
 * DEC  r8
 ***************************************************************/
#define DEC(value) ({                     \
  UINT8 res = ((value) - 1);              \
  F = (F & CF) | SZ(res) | NF;            \
  if (res == 0x7f) F |= VF|HF;            \
  else if ((res & 0x0f) == 0x0f) F |= HF; \
  res;                                    \
})

/***************************************************************
 * RLCA
 ***************************************************************/
#define RLCA                                        \
  A = (A << 1) | (A >> 7);                          \
  F = (F & (SF | ZF | PF)) | (A & (YF | XF | CF))

/***************************************************************
 * RRCA
 ***************************************************************/
#define RRCA                                        \
  F = (F & (SF | ZF | PF)) | (A & CF);              \
  A = (A >> 1) | (A << 7);                          \
  F |= (A & (YF | XF) )

/***************************************************************
 * RLA
 ***************************************************************/
#define RLA {                                  \
  UINT8 res = (A << 1) | (F & CF);                  \
  UINT8 c = (A & 0x80) ? CF : 0;                    \
  F = (F & (SF | ZF | PF)) | c | (res & (YF | XF)); \
  A = res;                                  \
}

/***************************************************************
 * RRA
 ***************************************************************/
#define RRA {                                       \
  UINT8 res = (A >> 1) | (F << 7);                  \
  UINT8 c = (A & 0x01) ? CF : 0;                    \
  F = (F & (SF | ZF | PF)) | c | (res & (YF | XF)); \
  A = res;                                          \
}

/***************************************************************
 * RRD
 ***************************************************************/
#define RRD {                        \
  UINT8 n = RM(HL);                  \
  WZ = HL+1;                         \
  WM( HL, (n >> 4) | (A << 4) );     \
  A = (A & 0xf0) | (n & 0x0f);       \
  F = (F & CF) | SZP[A];             \
}

/***************************************************************
 * RLD
 ***************************************************************/
#define RLD {                        \
  UINT8 n = RM(HL);                  \
  WZ = HL+1;                         \
  WM( HL, (n << 4) | (A & 0x0f) );   \
  A = (A & 0xf0) | (n >> 4);         \
  F = (F & CF) | SZP[A];             \
}

/***************************************************************
 * ADD  A,n
 ***************************************************************/
#define ADD(value) {                        \
  UINT32 ah = AFD & 0xff00;                 \
  UINT32 res = (UINT8)((ah >> 8) + value);  \
  F = SZHVC_add[ah | res];                  \
  A = res;                                  \
}

/***************************************************************
 * ADC  A,n
 ***************************************************************/
#define ADC(value) {                           \
  UINT32 ah = AFD & 0xff00, c = AFD & 1;       \
  UINT32 res = (UINT8)((ah >> 8) + value + c); \
  F = SZHVC_add[(c << 16) | ah | res];         \
  A = res;                                     \
}

/***************************************************************
 * SUB  n
 ***************************************************************/
#define SUB(value) {                        \
  UINT32 ah = AFD & 0xff00;                 \
  UINT32 res = (UINT8)((ah >> 8) - value);  \
  F = SZHVC_sub[ah | res];                  \
  A = res;                                  \
}

/***************************************************************
 * SBC  A,n
 ***************************************************************/
#define SBC(value) {                            \
  UINT32 ah = AFD & 0xff00, c = AFD & 1;        \
  UINT32 res = (UINT8)((ah >> 8) - value - c);  \
  F = SZHVC_sub[(c<<16) | ah | res];            \
  A = res;                                      \
}

/***************************************************************
 * NEG
 ***************************************************************/
#define NEG {                          \
  UINT8 value = A;                     \
  A = 0;                               \
  SUB(value);                          \
}

/***************************************************************
 * DAA
 ***************************************************************/
#define DAA {                                         \
  UINT8 a = A;                                        \
  if (F & NF) {                                       \
    if ((F&HF) | ((A&0xf)>9)) a-=6;                   \
    if ((F&CF) | (A>0x99)) a-=0x60;                   \
  } else {                                            \
    if ((F&HF) | ((A&0xf)>9)) a+=6;                   \
    if ((F&CF) | (A>0x99)) a+=0x60;                   \
  }                                                   \
  F = (F&(CF|NF)) | (A>0x99) | ((A^a)&HF) | SZP[a];   \
  A = a;                                              \
}

/***************************************************************
 * AND  n
 ***************************************************************/
#define AND(value)    \
  A &= value;         \
  F = SZP[A] | HF

/***************************************************************
 * OR  n
 ***************************************************************/
#define OR(value)     \
  A |= value;         \
  F = SZP[A]

/***************************************************************
 * XOR  n
 ***************************************************************/
#define XOR(value)    \
  A ^= value;         \
  F = SZP[A]

/***************************************************************
 * CP  n
 ***************************************************************/
#define CP(value) {                         \
  UINT32 val = value;                       \
  UINT32 ah = AFD & 0xff00;                 \
  UINT32 res = (UINT8)((ah >> 8) - val);    \
  F = (SZHVC_sub[ah | res] & ~(YF | XF)) |  \
    (val & (YF | XF));                      \
}

/***************************************************************
 * EX  AF,AF'
 ***************************************************************/
#define EX_AF {                                   \
  PAIR tmp;                                       \
  tmp = Z80.af; Z80.af = Z80.af2; Z80.af2 = tmp;  \
}

/***************************************************************
 * EX  DE,HL
 ***************************************************************/
#define EX_DE_HL {                              \
  PAIR tmp;                                     \
  tmp = Z80.de; Z80.de = Z80.hl; Z80.hl = tmp;  \
}

/***************************************************************
 * EXX
 ***************************************************************/
#define EXX {                                     \
  PAIR tmp;                                       \
  tmp = Z80.bc; Z80.bc = Z80.bc2; Z80.bc2 = tmp;  \
  tmp = Z80.de; Z80.de = Z80.de2; Z80.de2 = tmp;  \
  tmp = Z80.hl; Z80.hl = Z80.hl2; Z80.hl2 = tmp;  \
}

/***************************************************************
 * EX  (SP),r16
 ***************************************************************/
#define EXSP(DR)                   \
{                                  \
  PAIR tmp = { { 0, 0, 0, 0 } };   \
  RM16( SPD, &tmp );               \
  WM16( SPD, &Z80.DR );            \
  Z80.DR = tmp;                    \
  WZ = Z80.DR.d;                   \
}


/***************************************************************
 * ADD16
 ***************************************************************/
#define ADD16(DR,SR) {                              \
  UINT32 res = Z80.DR.d + Z80.SR.d;                 \
  WZ = Z80.DR.d + 1;                                \
  F = (F & (SF | ZF | VF)) |                        \
    (((Z80.DR.d ^ res ^ Z80.SR.d) >> 8) & HF) |     \
    ((res >> 16) & CF) | ((res >> 8) & (YF | XF));  \
  Z80.DR.w.l = (UINT16)res;                         \
}

/***************************************************************
 * ADC  r16,r16
 ***************************************************************/
#define ADC16(Reg) {                                    \
  UINT32 res = HLD + Z80.Reg.d + (F & CF);              \
  WZ = HL + 1;                                          \
  F = (((HLD ^ res ^ Z80.Reg.d) >> 8) & HF) |           \
    ((res >> 16) & CF) |                                \
    ((res >> 8) & (SF | YF | XF)) |                     \
    ((res & 0xffff) ? 0 : ZF) |                         \
    (((Z80.Reg.d ^ HLD ^ 0x8000) & (Z80.Reg.d ^ res) & 0x8000) >> 13); \
  HL = (UINT16)res;                                     \
}

/***************************************************************
 * SBC  r16,r16
 ***************************************************************/
#define SBC16(Reg) {                                    \
  UINT32 res = HLD - Z80.Reg.d - (F & CF);              \
  WZ = HL + 1;                                          \
  F = (((HLD ^ res ^ Z80.Reg.d) >> 8) & HF) | NF |      \
    ((res >> 16) & CF) |                                \
    ((res >> 8) & (SF | YF | XF)) |                     \
    ((res & 0xffff) ? 0 : ZF) |                         \
    (((Z80.Reg.d ^ HLD) & (HLD ^ res) &0x8000) >> 13);  \
  HL = (UINT16)res;                                     \
}

/***************************************************************
 * RLC  r8
 ***************************************************************/
#define RLC(value) ({                             \
  UINT32 res = (UINT8)(value);                    \
  UINT32 c = (res & 0x80) ? CF : 0;               \
  res = ((res << 1) | (res >> 7));                \
  F = SZP[res] | c;                               \
  res;                                            \
})

/***************************************************************
 * RRC  r8
 ***************************************************************/
#define RRC(value) ({                             \
  UINT8 res = (UINT8)(value);                     \
  UINT8 c = (res & 0x01) ? CF : 0;                \
  res = ((res >> 1) | (res << 7));                \
  F = SZP[res] | c;                               \
  res;                                            \
})

/***************************************************************
 * RL  r8
 ***************************************************************/
#define RL(value) ({                              \
  UINT8 res = (UINT8)(value);                     \
  UINT8 c = (res & 0x80) ? CF : 0;                \
  res = ((res << 1) | (F & CF));                  \
  F = SZP[res] | c;                               \
  res;                                            \
})

/***************************************************************
 * RR  r8
 ***************************************************************/
#define RR(value) ({                              \
  UINT8 res = (UINT8)(value);                     \
  UINT8 c = (res & 0x01) ? CF : 0;                \
  res = ((res >> 1) | (F << 7));                  \
  F = SZP[res] | c;                               \
  res;                                            \
})

/***************************************************************
 * SLA  r8
 ***************************************************************/
#define SLA(value) ({                             \
  UINT8 res = (UINT8)(value);                     \
  UINT8 c = (res & 0x80) ? CF : 0;                \
  res = (res << 1);                               \
  F = SZP[res] | c;                               \
  res;                                            \
})

/***************************************************************
 * SRA  r8
 ***************************************************************/
#define SRA(value) ({                             \
  UINT8 res = (UINT8)(value);                     \
  UINT8 c = (res & 0x01) ? CF : 0;                \
  res = ((res >> 1) | (res & 0x80));              \
  F = SZP[res] | c;                               \
  res;                                            \
})

/***************************************************************
 * SLL  r8
 ***************************************************************/
#define SLL(value) ({                             \
  UINT8 res = (UINT8)(value);                     \
  UINT8 c = (res & 0x80) ? CF : 0;                \
  res = ((res << 1) | 0x01);                      \
  F = SZP[res] | c;                               \
  res;                                            \
})

/***************************************************************
 * SRL  r8
 ***************************************************************/
#define SRL(value) ({                             \
  UINT8 res = (UINT8)(value);                     \
  UINT8 c = (res & 0x01) ? CF : 0;                \
  res = (res >> 1);                               \
  F = SZP[res] | c;                               \
  res;                                            \
})

/***************************************************************
 * BIT  bit,r8
 ***************************************************************/
#undef BIT
#define BIT(bit,reg)                      \
  F = (F & CF) | HF | SZ_BIT(reg & (1<<bit)) | (reg & (YF|XF));

/***************************************************************
 * BIT  bit,(HL)
 ***************************************************************/
#define BIT_HL(bit, reg)                    \
  F = (F & CF) | HF | SZ_BIT(reg & (1<<bit)) | (WZ_H & (YF|XF));

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
#define BIT_XY(bit,reg)                     \
  F = (F & CF) | HF | SZ_BIT(reg & (1<<bit)) | ((EA>>8) & (YF|XF));

/***************************************************************
 * RES  bit,r8
 ***************************************************************/
#define RES(bit, value) ((UINT8)(value) & ~(1<<(bit)))

/***************************************************************
 * SET  bit,r8
 ***************************************************************/
#define SET(bit, value) ((UINT8)(value) | (1<<(bit)))

/***************************************************************
 * LDI
 ***************************************************************/
#define LDI {                                             \
  UINT8 io = RM(HL);                                      \
  WM( DE, io );                                           \
  F &= SF | ZF | CF;                                      \
  if( (A + io) & 0x02 ) F |= YF; /* bit 1 -> flag 5 */    \
  if( (A + io) & 0x08 ) F |= XF; /* bit 3 -> flag 3 */    \
  HL++; DE++; BC--;                                       \
  if( BC ) F |= VF;                                       \
}

/***************************************************************
 * CPI
 ***************************************************************/
#define CPI {                                           \
  UINT8 val = RM(HL);                                   \
  UINT8 res = A - val;                                  \
  WZ++;                                                 \
  HL++; BC--;                                           \
  F = (F & CF) | (SZ(res)&~(YF|XF)) | ((A^val^res)&HF) | NF;  \
  if( F & HF ) res -= 1;                                \
  if( res & 0x02 ) F |= YF; /* bit 1 -> flag 5 */       \
  if( res & 0x08 ) F |= XF; /* bit 3 -> flag 3 */       \
  if( BC ) F |= VF;                                     \
}

/***************************************************************
 * INI
 ***************************************************************/
#define INI {                          \
  unsigned t;                          \
  UINT8 io = IN(BC);                      \
  WZ = BC + 1; \
  CC(ex,0xa2);                      \
  B--;                            \
  WM( HL, io );                        \
  HL++;                            \
  F = SZ(B);                          \
  t = (unsigned)((C + 1) & 0xff) + (unsigned)io;        \
  if( io & SF ) F |= NF;                    \
  if( t & 0x100 ) F |= HF | CF;                \
  F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;            \
}

/***************************************************************
 * OUTI
 ***************************************************************/
#define OUTI {                          \
  unsigned t;                          \
  UINT8 io = RM(HL);                      \
  B--;                            \
  WZ = BC + 1;  \
  OUT( BC, io );                        \
  HL++;                            \
  F = SZ(B);                          \
  t = (unsigned)L + (unsigned)io;                \
  if( io & SF ) F |= NF;                    \
  if( t & 0x100 ) F |= HF | CF;                \
  F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;            \
}

/***************************************************************
 * LDD
 ***************************************************************/
#define LDD {                          \
  UINT8 io = RM(HL);                      \
  WM( DE, io );                        \
  F &= SF | ZF | CF;                      \
  if( (A + io) & 0x02 ) F |= YF; /* bit 1 -> flag 5 */    \
  if( (A + io) & 0x08 ) F |= XF; /* bit 3 -> flag 3 */    \
  HL--; DE--; BC--;                      \
  if( BC ) F |= VF;                      \
}

/***************************************************************
 * CPD
 ***************************************************************/
#define CPD {                          \
  UINT8 val = RM(HL);                      \
  UINT8 res = A - val;                    \
  WZ--;  \
  HL--; BC--;                          \
  F = (F & CF) | (SZ(res)&~(YF|XF)) | ((A^val^res)&HF) | NF;  \
  if( F & HF ) res -= 1;                    \
  if( res & 0x02 ) F |= YF; /* bit 1 -> flag 5 */        \
  if( res & 0x08 ) F |= XF; /* bit 3 -> flag 3 */        \
  if( BC ) F |= VF;                      \
}

/***************************************************************
 * IND
 ***************************************************************/
#define IND {                          \
  unsigned t;                          \
  UINT8 io = IN(BC);                      \
  WZ = BC - 1;  \
  CC(ex,0xaa);                      \
  B--;                            \
  WM( HL, io );                        \
  HL--;                            \
  F = SZ(B);                          \
  t = ((unsigned)(C - 1) & 0xff) + (unsigned)io;        \
  if( io & SF ) F |= NF;                    \
  if( t & 0x100 ) F |= HF | CF;                \
  F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;            \
}

/***************************************************************
 * OUTD
 ***************************************************************/
#define OUTD {                          \
  unsigned t;                          \
  UINT8 io = RM(HL);                      \
  B--;                            \
  WZ = BC - 1;  \
  OUT( BC, io );                        \
  HL--;                            \
  F = SZ(B);                          \
  t = (unsigned)L + (unsigned)io;                \
  if( io & SF ) F |= NF;                    \
  if( t & 0x100 ) F |= HF | CF;                \
  F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;            \
}

/***************************************************************
 * LDIR
 ***************************************************************/
#define LDIR                  \
  LDI;                        \
  if( BC )  {                 \
    PC -= 2;                  \
    WZ = PC + 1;              \
    CC(ex,0xb0);              \
  }

/***************************************************************
 * CPIR
 ***************************************************************/
#define CPIR                  \
  CPI;                        \
  if( BC && !(F & ZF) ) {     \
    PC -= 2;                  \
    WZ = PC + 1;              \
    CC(ex,0xb1);              \
  }

/***************************************************************
 * INIR
 ***************************************************************/
#define INIR                  \
  INI;                        \
  if( B ) {                   \
    PC -= 2;                  \
    CC(ex,0xb2);              \
  }

/***************************************************************
 * OTIR
 ***************************************************************/
#define OTIR                  \
  OUTI;                       \
  if( B ) {                   \
    PC -= 2;                  \
    CC(ex,0xb3);              \
  }

/***************************************************************
 * LDDR
 ***************************************************************/
#define LDDR                  \
  LDD;                        \
  if( BC ) {                  \
    PC -= 2;                  \
    WZ = PC + 1;              \
    CC(ex,0xb8);              \
  }

/***************************************************************
 * CPDR
 ***************************************************************/
#define CPDR                  \
  CPD;                        \
  if( BC && !(F & ZF) ) {     \
    PC -= 2;                  \
    WZ = PC + 1;              \
    CC(ex,0xb9);              \
  }

/***************************************************************
 * INDR
 ***************************************************************/
#define INDR                  \
  IND;                        \
  if( B ) {                   \
    PC -= 2;                  \
    CC(ex,0xba);              \
  }

/***************************************************************
 * OTDR
 ***************************************************************/
#define OTDR                  \
  OUTD;                       \
  if( B ) {                   \
    PC -= 2;                  \
    CC(ex,0xbb);              \
  }

/***************************************************************
 * EI
 ***************************************************************/
#define EI {                  \
  IFF1 = IFF2 = 1;            \
  Z80.after_ei = true;        \
}

/**********************************************************
* Default/dummy read/write functions
**********************************************************/
static UINT8 dummy_read(UINT16 addr)
{
  return 0;
}

static void dummy_write(UINT16 addr, UINT8 val)
{
  //
}

static int dummy_irq(int param)
{
  return 0xFF;
}

/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit operations with (IX+o)
**********************************************************/
NO_INLINE void EXEC_XYCB(UINT8 opcode)
{
  CC(xycb, opcode);

	UINT8 temp, *r;

	switch (opcode & 7)
	{
	case 0: r = &B; break;
	case 1: r = &C; break;
	case 2: r = &D; break;
	case 3: r = &E; break;
	case 4: r = &H; break;
	case 5: r = &L; break;
	case 6: r = &temp; break;
	case 7: r = &A; break;
	}

  opcode >>= 3;

  switch (opcode)
  {
    case 0x00: /* RLC  r=(XY+o)    */
      *r = RLC(RM(EA));
      break;

    case 0x01: /* RL   r=(XY+o)    */
      *r = RRC(RM(EA));
      break;

    case 0x02: /* RL   r=(XY+o)    */
      *r = RL(RM(EA));
      break;

    case 0x03: /* RR   r=(XY+o)    */
      *r = RR(RM(EA));
      break;

    case 0x04: /* SLA  r=(XY+o)    */
      *r = SLA(RM(EA));
      break;

    case 0x05: /* SRA  r=(XY+o)    */
      *r = SRA(RM(EA));
      break;

    case 0x06: /* SLL  r=(XY+o)    */
      *r = SLL(RM(EA));
      break;

    case 0x07: /* SRL  r=(XY+o)    */
      *r = SRL(RM(EA));
      break;

    case 0x08: /* BIT n, (XY+o)    */
    case 0x09:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
      BIT_XY((opcode & 7), RM(EA));
      return;

    case 0x10: /* RES  n,r=(XY+o)    */
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
      *r = RES((opcode & 7), RM(EA));
      break;

    case 0x18: /* SET  n,r=(XY+o)    */
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
    case 0x1D:
    case 0x1E:
    case 0x1F:
      *r = SET((opcode & 7), RM(EA));
      break;
  }
  WM(EA, *r);
}

/**********************************************************
 * opcodes with CB prefix
 * rotate, shift and bit operations
 **********************************************************/
NO_INLINE void EXEC_CB(UINT8 opcode)
{
  CC(cb, opcode);

	UINT8 hl, *r;

	switch (opcode & 7)
	{
	case 0: r = &B; break;
	case 1: r = &C; break;
	case 2: r = &D; break;
	case 3: r = &E; break;
	case 4: r = &H; break;
	case 5: r = &L; break;
	case 6: r = &hl; hl = RM(HL); break;
	case 7: r = &A; break;
	}

	opcode >>= 3;

	switch (opcode)
	{
	case 0x00: /* RLC  r           */
    *r = RLC(*r);
    break;

  case 0x01: /* RRC  B           */
    *r = RRC(*r);
    break;

  case 0x02: /* RL  r            */
    *r = RL(*r);
    break;

  case 0x03: /* RR  r            */
    *r = RR(*r);
    break;

  case 0x04: /* SLA  r           */
    *r = SLA(*r);
    break;

  case 0x05: /* SRA  r           */
    *r = SRA(*r);
    break;

  case 0x06: /* SLL  r           */
    *r = SLL(*r);
    break;

  case 0x07: /* SRL  r           */
    *r = SRL(*r);
    break;

	case 0x08: /* BIT  n,r         */
	case 0x09:
	case 0x0A:
	case 0x0B:
	case 0x0C:
	case 0x0D:
	case 0x0E:
	case 0x0F:
    BIT((opcode & 7), *r);
    return;

  case 0x10: /* RES  n,r         */
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
    *r = RES((opcode & 7), *r);
    break;

  case 0x18: /* SET  n,r         */
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
    *r = SET((opcode & 7), *r);
    break;
	}

	if (r == &hl)
		WM(HL, hl);
}

/**********************************************************
 * IX register related opcodes (DD prefix)
 **********************************************************/
NO_INLINE void EXEC_DD(UINT8 opcode)
{
exec_dd:
  CC(dd, opcode);

  switch(opcode)
  {
    OP(0x09, { ADD16(ix,bc);                                   }); /* ADD  IX,BC    */
    OP(0x19, { ADD16(ix,de);                                   }); /* ADD  IX,DE    */
    OP(0x21, { IX = ARG16();                                   }); /* LD   IX,w     */
    OP(0x22, { EA = ARG16(); WM16( EA, &Z80.ix ); WZ = EA+1;   }); /* LD   (w),IX   */
    OP(0x23, { IX++;                                           }); /* INC  IX       */
    OP(0x24, { HX = INC(HX);                                   }); /* INC  HX       */
    OP(0x25, { HX = DEC(HX);                                   }); /* DEC  HX       */
    OP(0x26, { HX = ARG();                                     }); /* LD   HX,n     */
    OP(0x29, { ADD16(ix,ix);                                   }); /* ADD  IX,IX    */
    OP(0x2a, { EA = ARG16(); RM16( EA, &Z80.ix ); WZ = EA+1;   }); /* LD   IX,(w)   */
    OP(0x2b, { IX--;                                           }); /* DEC  IX       */
    OP(0x2c, { LX = INC(LX);                                   }); /* INC  LX       */
    OP(0x2d, { LX = DEC(LX);                                   }); /* DEC  LX       */
    OP(0x2e, { LX = ARG();                                     }); /* LD   LX,n     */
    OP(0x34, { EAX(); WM( EA, INC(RM(EA)) );                   }); /* INC  (IX+o)   */
    OP(0x35, { EAX(); WM( EA, DEC(RM(EA)) );                   }); /* DEC  (IX+o)   */
    OP(0x36, { EAX(); WM( EA, ARG() );                         }); /* LD   (IX+o),n */
    OP(0x39, { ADD16(ix,sp);                                   }); /* ADD  IX,SP    */
    OP(0x44, { B = HX;                                         }); /* LD   B,HX     */
    OP(0x45, { B = LX;                                         }); /* LD   B,LX     */
    OP(0x46, { EAX(); B = RM(EA);                              }); /* LD   B,(IX+o) */
    OP(0x4c, { C = HX;                                         }); /* LD   C,HX     */
    OP(0x4d, { C = LX;                                         }); /* LD   C,LX     */
    OP(0x4e, { EAX(); C = RM(EA);                              }); /* LD   C,(IX+o) */
    OP(0x54, { D = HX;                                         }); /* LD   D,HX     */
    OP(0x55, { D = LX;                                         }); /* LD   D,LX     */
    OP(0x56, { EAX(); D = RM(EA);                              }); /* LD   D,(IX+o) */
    OP(0x5c, { E = HX;                                         }); /* LD   E,HX     */
    OP(0x5d, { E = LX;                                         }); /* LD   E,LX     */
    OP(0x5e, { EAX(); E = RM(EA);                              }); /* LD   E,(IX+o) */
    OP(0x60, { HX = B;                                         }); /* LD   HX,B     */
    OP(0x61, { HX = C;                                         }); /* LD   HX,C     */
    OP(0x62, { HX = D;                                         }); /* LD   HX,D     */
    OP(0x63, { HX = E;                                         }); /* LD   HX,E     */
    OP(0x64, {                                                 }); /* LD   HX,HX    */
    OP(0x65, { HX = LX;                                        }); /* LD   HX,LX    */
    OP(0x66, { EAX(); H = RM(EA);                              }); /* LD   H,(IX+o) */
    OP(0x67, { HX = A;                                         }); /* LD   HX,A     */
    OP(0x68, { LX = B;                                         }); /* LD   LX,B     */
    OP(0x69, { LX = C;                                         }); /* LD   LX,C     */
    OP(0x6a, { LX = D;                                         }); /* LD   LX,D     */
    OP(0x6b, { LX = E;                                         }); /* LD   LX,E     */
    OP(0x6c, { LX = HX;                                        }); /* LD   LX,HX    */
    OP(0x6d, {                                                 }); /* LD   LX,LX    */
    OP(0x6e, { EAX(); L = RM(EA);                              }); /* LD   L,(IX+o) */
    OP(0x6f, { LX = A;                                         }); /* LD   LX,A     */
    OP(0x70, { EAX(); WM( EA, B );                             }); /* LD   (IX+o),B */
    OP(0x71, { EAX(); WM( EA, C );                             }); /* LD   (IX+o),C */
    OP(0x72, { EAX(); WM( EA, D );                             }); /* LD   (IX+o),D */
    OP(0x73, { EAX(); WM( EA, E );                             }); /* LD   (IX+o),E */
    OP(0x74, { EAX(); WM( EA, H );                             }); /* LD   (IX+o),H */
    OP(0x75, { EAX(); WM( EA, L );                             }); /* LD   (IX+o),L */
    OP(0x77, { EAX(); WM( EA, A );                             }); /* LD   (IX+o),A */
    OP(0x7c, { A = HX;                                         }); /* LD   A,HX     */
    OP(0x7d, { A = LX;                                         }); /* LD   A,LX     */
    OP(0x7e, { EAX(); A = RM(EA);                              }); /* LD   A,(IX+o) */
    OP(0x84, { ADD(HX);                                        }); /* ADD  A,HX     */
    OP(0x85, { ADD(LX);                                        }); /* ADD  A,LX     */
    OP(0x86, { EAX(); ADD(RM(EA));                             }); /* ADD  A,(IX+o) */
    OP(0x8c, { ADC(HX);                                        }); /* ADC  A,HX     */
    OP(0x8d, { ADC(LX);                                        }); /* ADC  A,LX     */
    OP(0x8e, { EAX(); ADC(RM(EA));                             }); /* ADC  A,(IX+o) */
    OP(0x94, { SUB(HX);                                        }); /* SUB  HX       */
    OP(0x95, { SUB(LX);                                        }); /* SUB  LX       */
    OP(0x96, { EAX(); SUB(RM(EA));                             }); /* SUB  (IX+o)   */
    OP(0x9c, { SBC(HX);                                        }); /* SBC  A,HX     */
    OP(0x9d, { SBC(LX);                                        }); /* SBC  A,LX     */
    OP(0x9e, { EAX(); SBC(RM(EA));                             }); /* SBC  A,(IX+o) */
    OP(0xa4, { AND(HX);                                        }); /* AND  HX       */
    OP(0xa5, { AND(LX);                                        }); /* AND  LX       */
    OP(0xa6, { EAX(); AND(RM(EA));                             }); /* AND  (IX+o)   */
    OP(0xac, { XOR(HX);                                        }); /* XOR  HX       */
    OP(0xad, { XOR(LX);                                        }); /* XOR  LX       */
    OP(0xae, { EAX(); XOR(RM(EA));                             }); /* XOR  (IX+o)   */
    OP(0xb4, { OR(HX);                                         }); /* OR   HX       */
    OP(0xb5, { OR(LX);                                         }); /* OR   LX       */
    OP(0xb6, { EAX(); OR(RM(EA));                              }); /* OR   (IX+o)   */
    OP(0xbc, { CP(HX);                                         }); /* CP   HX       */
    OP(0xbd, { CP(LX);                                         }); /* CP   LX       */
    OP(0xbe, { EAX(); CP(RM(EA));                              }); /* CP   (IX+o)   */
    OP(0xcb, { EAX(); EXEC_XYCB(ARG());                        }); /* **** DD CB xx */
    OP(0xdd, { opcode = ROP(); goto exec_dd;                   }); /* **** DD DD xx */
    OP(0xe1, { POP( ix );                                      }); /* POP  IX       */
    OP(0xe3, { EXSP( ix );                                     }); /* EX   (SP),IX  */
    OP(0xe5, { PUSH( ix );                                     }); /* PUSH IX       */
    OP(0xe9, { PC = IX;                                        }); /* JP   (IX)     */
    OP(0xf9, { SP = IX;                                        }); /* LD   SP,IX    */
    // OP(0xfd, { EXEC_FD(ROP());                                 }); /* **** DD FD xx */
  }

  // Illegal OP codes map to main opcodes
  // EXEC_OP(opcode);
}

/**********************************************************
 * IY register related opcodes (FD prefix)
 **********************************************************/
NO_INLINE void EXEC_FD(UINT8 opcode)
{
exec_fd:
  CC(fd, opcode);

  switch(opcode)
  {
    OP(0x09, { ADD16(iy,bc);                                   }); /* ADD  IY,BC    */
    OP(0x19, { ADD16(iy,de);                                   }); /* ADD  IY,DE    */
    OP(0x21, { IY = ARG16();                                   }); /* LD   IY,w     */
    OP(0x22, { EA = ARG16(); WM16( EA, &Z80.iy ); WZ = EA+1;   }); /* LD   (w),IY   */
    OP(0x23, { IY++;                                           }); /* INC  IY       */
    OP(0x24, { HY = INC(HY);                                   }); /* INC  HY       */
    OP(0x25, { HY = DEC(HY);                                   }); /* DEC  HY       */
    OP(0x26, { HY = ARG();                                     }); /* LD   HY,n     */
    OP(0x29, { ADD16(iy,iy);                                   }); /* ADD  IY,IY    */
    OP(0x2a, { EA = ARG16(); RM16( EA, &Z80.iy ); WZ = EA+1;   }); /* LD   IY,(w)   */
    OP(0x2b, { IY--;                                           }); /* DEC  IY       */
    OP(0x2c, { LY = INC(LY);                                   }); /* INC  LY       */
    OP(0x2d, { LY = DEC(LY);                                   }); /* DEC  LY       */
    OP(0x2e, { LY = ARG();                                     }); /* LD   LY,n     */
    OP(0x34, { EAY(); WM( EA, INC(RM(EA)) );                   }); /* INC  (IY+o)   */
    OP(0x35, { EAY(); WM( EA, DEC(RM(EA)) );                   }); /* DEC  (IY+o)   */
    OP(0x36, { EAY(); WM( EA, ARG() );                         }); /* LD   (IY+o),n */
    OP(0x39, { ADD16(iy,sp);                                   }); /* ADD  IY,SP    */
    OP(0x44, { B = HY;                                         }); /* LD   B,HY     */
    OP(0x45, { B = LY;                                         }); /* LD   B,LY     */
    OP(0x46, { EAY(); B = RM(EA);                              }); /* LD   B,(IY+o) */
    OP(0x4c, { C = HY;                                         }); /* LD   C,HY     */
    OP(0x4d, { C = LY;                                         }); /* LD   C,LY     */
    OP(0x4e, { EAY(); C = RM(EA);                              }); /* LD   C,(IY+o) */
    OP(0x54, { D = HY;                                         }); /* LD   D,HY     */
    OP(0x55, { D = LY;                                         }); /* LD   D,LY     */
    OP(0x56, { EAY(); D = RM(EA);                              }); /* LD   D,(IY+o) */
    OP(0x5c, { E = HY;                                         }); /* LD   E,HY     */
    OP(0x5d, { E = LY;                                         }); /* LD   E,LY     */
    OP(0x5e, { EAY(); E = RM(EA);                              }); /* LD   E,(IY+o) */
    OP(0x60, { HY = B;                                         }); /* LD   HY,B     */
    OP(0x61, { HY = C;                                         }); /* LD   HY,C     */
    OP(0x62, { HY = D;                                         }); /* LD   HY,D     */
    OP(0x63, { HY = E;                                         }); /* LD   HY,E     */
    OP(0x64, {                                                 }); /* LD   HY,HY    */
    OP(0x65, { HY = LY;                                        }); /* LD   HY,LY    */
    OP(0x66, { EAY(); H = RM(EA);                              }); /* LD   H,(IY+o) */
    OP(0x67, { HY = A;                                         }); /* LD   HY,A     */
    OP(0x68, { LY = B;                                         }); /* LD   LY,B     */
    OP(0x69, { LY = C;                                         }); /* LD   LY,C     */
    OP(0x6a, { LY = D;                                         }); /* LD   LY,D     */
    OP(0x6b, { LY = E;                                         }); /* LD   LY,E     */
    OP(0x6c, { LY = HY;                                        }); /* LD   LY,HY    */
    OP(0x6d, {                                                 }); /* LD   LY,LY    */
    OP(0x6e, { EAY(); L = RM(EA);                              }); /* LD   L,(IY+o) */
    OP(0x6f, { LY = A;                                         }); /* LD   LY,A     */
    OP(0x70, { EAY(); WM( EA, B );                             }); /* LD   (IY+o),B */
    OP(0x71, { EAY(); WM( EA, C );                             }); /* LD   (IY+o),C */
    OP(0x72, { EAY(); WM( EA, D );                             }); /* LD   (IY+o),D */
    OP(0x73, { EAY(); WM( EA, E );                             }); /* LD   (IY+o),E */
    OP(0x74, { EAY(); WM( EA, H );                             }); /* LD   (IY+o),H */
    OP(0x75, { EAY(); WM( EA, L );                             }); /* LD   (IY+o),L */
    OP(0x77, { EAY(); WM( EA, A );                             }); /* LD   (IY+o),A */
    OP(0x7c, { A = HY;                                         }); /* LD   A,HY     */
    OP(0x7d, { A = LY;                                         }); /* LD   A,LY     */
    OP(0x7e, { EAY(); A = RM(EA);                              }); /* LD   A,(IY+o) */
    OP(0x84, { ADD(HY);                                        }); /* ADD  A,HY     */
    OP(0x85, { ADD(LY);                                        }); /* ADD  A,LY     */
    OP(0x86, { EAY(); ADD(RM(EA));                             }); /* ADD  A,(IY+o) */
    OP(0x8c, { ADC(HY);                                        }); /* ADC  A,HY     */
    OP(0x8d, { ADC(LY);                                        }); /* ADC  A,LY     */
    OP(0x8e, { EAY(); ADC(RM(EA));                             }); /* ADC  A,(IY+o) */
    OP(0x94, { SUB(HY);                                        }); /* SUB  HY       */
    OP(0x95, { SUB(LY);                                        }); /* SUB  LY       */
    OP(0x96, { EAY(); SUB(RM(EA));                             }); /* SUB  (IY+o)   */
    OP(0x9c, { SBC(HY);                                        }); /* SBC  A,HY     */
    OP(0x9d, { SBC(LY);                                        }); /* SBC  A,LY     */
    OP(0x9e, { EAY(); SBC(RM(EA));                             }); /* SBC  A,(IY+o) */
    OP(0xa4, { AND(HY);                                        }); /* AND  HY       */
    OP(0xa5, { AND(LY);                                        }); /* AND  LY       */
    OP(0xa6, { EAY(); AND(RM(EA));                             }); /* AND  (IY+o)   */
    OP(0xac, { XOR(HY);                                        }); /* XOR  HY       */
    OP(0xad, { XOR(LY);                                        }); /* XOR  LY       */
    OP(0xae, { EAY(); XOR(RM(EA));                             }); /* XOR  (IY+o)   */
    OP(0xb4, { OR(HY);                                         }); /* OR   HY       */
    OP(0xb5, { OR(LY);                                         }); /* OR   LY       */
    OP(0xb6, { EAY(); OR(RM(EA));                              }); /* OR   (IY+o)   */
    OP(0xbc, { CP(HY);                                         }); /* CP   HY       */
    OP(0xbd, { CP(LY);                                         }); /* CP   LY       */
    OP(0xbe, { EAY(); CP(RM(EA));                              }); /* CP   (IY+o)   */
    OP(0xcb, { EAY(); EXEC_XYCB(ARG());                        }); /* **** FD CB xx */
    // OP(0xdd, { EXEC_DD(ROP());                                 }); /* **** FD DD xx */
    OP(0xe1, { POP( iy );                                      }); /* POP  IY       */
    OP(0xe3, { EXSP( iy );                                     }); /* EX   (SP),IY  */
    OP(0xe5, { PUSH( iy );                                     }); /* PUSH IY       */
    OP(0xe9, { PC = IY;                                        }); /* JP   (IY)     */
    OP(0xf9, { SP = IY;                                        }); /* LD   SP,IY    */
    OP(0xfd, { opcode = ROP(); goto exec_fd;                   }); /* **** FD FD xx */
  }

  // Illegal OP codes map to main opcodes
  // EXEC_OP(opcode);
}

/**********************************************************
 * special opcodes (ED prefix)
 **********************************************************/
NO_INLINE void EXEC_ED(UINT8 opcode)
{
  CC(ed, opcode);

  switch(opcode)
  {
    OP(0x40, {B = IN(BC); F = (F & CF) | SZP[B];               }); /* IN   B,(C)   */
    OP(0x41, {OUT(BC, B);                                      }); /* OUT  (C),B   */
    OP(0x42, {SBC16( bc );                                     }); /* SBC  HL,BC   */
    OP(0x43, {EA = ARG16(); WM16( EA, &Z80.bc ); WZ = EA+1;    }); /* LD   (w),BC  */
    OP(0x44, {NEG;                                             }); /* NEG          */
    OP(0x45, {RETN;                                            }); /* RETN;        */
    OP(0x46, {IM = 0;                                          }); /* IM   0       */
    OP(0x47, {LD_I_A;                                          }); /* LD   I,A     */
    OP(0x48, {C = IN(BC); F = (F & CF) | SZP[C];               }); /* IN   C,(C)   */
    OP(0x49, {OUT(BC, C);                                      }); /* OUT  (C),C   */
    OP(0x4a, {ADC16( bc );                                     }); /* ADC  HL,BC   */
    OP(0x4b, {EA = ARG16(); RM16( EA, &Z80.bc ); WZ = EA+1;    }); /* LD   BC,(w)  */
    OP(0x4c, {NEG;                                             }); /* NEG          */
    OP(0x4d, {RETI;                                            }); /* RETI         */
    OP(0x4e, {IM = 0;                                          }); /* IM   0       */
    OP(0x4f, {LD_R_A;                                          }); /* LD   R,A     */
    OP(0x50, {D = IN(BC); F = (F & CF) | SZP[D];               }); /* IN   D,(C)   */
    OP(0x51, {OUT(BC, D);                                      }); /* OUT  (C),D   */
    OP(0x52, {SBC16( de );                                     }); /* SBC  HL,DE   */
    OP(0x53, {EA = ARG16(); WM16( EA, &Z80.de ); WZ = EA+1;    }); /* LD   (w),DE  */
    OP(0x54, {NEG;                                             }); /* NEG          */
    OP(0x55, {RETN;                                            }); /* RETN;        */
    OP(0x56, {IM = 1;                                          }); /* IM   1       */
    OP(0x57, {LD_A_I;                                          }); /* LD   A,I     */
    OP(0x58, {E = IN(BC); F = (F & CF) | SZP[E];               }); /*IN   E,(C)   */
    OP(0x59, {OUT(BC, E);                                      }); /*OUT  (C),E   */
    OP(0x5a, {ADC16( de );                                     }); /*ADC  HL,DE   */
    OP(0x5b, {EA = ARG16(); RM16( EA, &Z80.de ); WZ = EA+1;    }); /*LD   DE,(w)  */
    OP(0x5c, {NEG;                                             }); /*NEG          */
    OP(0x5d, {RETI;                                            }); /*RETI         */
    OP(0x5e, {IM = 2;                                          }); /*IM   2       */
    OP(0x5f, {LD_A_R;                                          }); /*LD   A,R     */
    OP(0x60, {H = IN(BC); F = (F & CF) | SZP[H];               }); /*IN   H,(C)   */
    OP(0x61, {OUT(BC, H);                                      }); /*OUT  (C),H   */
    OP(0x62, {SBC16( hl );                                     }); /*SBC  HL,HL   */
    OP(0x63, {EA = ARG16(); WM16( EA, &Z80.hl ); WZ = EA+1;    }); /*LD   (w),HL  */
    OP(0x64, {NEG;                                             }); /*NEG          */
    OP(0x65, {RETN;                                            }); /*RETN;        */
    OP(0x66, {IM = 0;                                          }); /*IM   0       */
    OP(0x67, {RRD;                                             }); /*RRD  (HL)    */
    OP(0x68, {L = IN(BC); F = (F & CF) | SZP[L];               }); /*IN   L,(C)   */
    OP(0x69, {OUT(BC, L);                                      }); /*OUT  (C),L   */
    OP(0x6a, {ADC16( hl );                                     }); /*ADC  HL,HL   */
    OP(0x6b, {EA = ARG16(); RM16( EA, &Z80.hl ); WZ = EA+1;    }); /*LD   HL,(w)  */
    OP(0x6c, {NEG;                                             }); /*NEG          */
    OP(0x6d, {RETI;                                            }); /*RETI         */
    OP(0x6e, {IM = 0;                                          }); /*IM   0       */
    OP(0x6f, {RLD;                                             }); /*RLD  (HL)    */
    OP(0x70, {{UINT8 res = IN(BC); F = (F & CF) | SZP[res];}   }); /*IN   0,(C)   */
    OP(0x71, {OUT(BC, 0);                                      }); /*OUT  (C),0   */
    OP(0x72, {SBC16( sp );                                     }); /*SBC  HL,SP   */
    OP(0x73, {EA = ARG16(); WM16( EA, &Z80.sp ); WZ = EA+1;    }); /*LD   (w),SP  */
    OP(0x74, {NEG;                                             }); /*NEG          */
    OP(0x75, {RETN;                                            }); /*RETN;        */
    OP(0x76, {IM = 1;                                          }); /*IM   1       */
    OP(0x78, {A = IN(BC); F = (F & CF) | SZP[A]; WZ = BC+1;    }); /*IN   E,(C)   */
    OP(0x79, {OUT(BC, A); WZ = BC + 1;                         }); /*OUT  (C),A   */
    OP(0x7a, {ADC16( sp );                                     }); /*ADC  HL,SP   */
    OP(0x7b, {EA = ARG16(); RM16( EA, &Z80.sp ); WZ = EA+1;    }); /*LD   SP,(w)  */
    OP(0x7c, {NEG;                                             }); /*NEG          */
    OP(0x7d, {RETI;                                            }); /*RETI         */
    OP(0x7e, {IM = 2;                                          }); /*IM   2       */
    OP(0xa0, {LDI;                                             }); /*LDI          */
    OP(0xa1, {CPI;                                             }); /*CPI          */
    OP(0xa2, {INI;                                             }); /*INI          */
    OP(0xa3, {OUTI;                                            }); /*OUTI         */
    OP(0xa8, {LDD;                                             }); /*LDD          */
    OP(0xa9, {CPD;                                             }); /*CPD          */
    OP(0xaa, {IND;                                             }); /*IND          */
    OP(0xab, {OUTD;                                            }); /*OUTD         */
    OP(0xb0, {LDIR;                                            }); /*LDIR         */
    OP(0xb1, {CPIR;                                            }); /*CPIR         */
    OP(0xb2, {INIR;                                            }); /*INIR         */
    OP(0xb3, {OTIR;                                            }); /*OTIR         */
    OP(0xb8, {LDDR;                                            }); /*LDDR         */
    OP(0xb9, {CPDR;                                            }); /*CPDR         */
    OP(0xba, {INDR;                                            }); /*INDR         */
    OP(0xbb, {OTDR;                                            }); /*OTDR         */
  }

  // Illegal OP codes are NO-OP or crash
}

/**********************************************************
 * main opcodes
 **********************************************************/
ALWAYS_INLINE void EXEC_OP(UINT8 opcode)
{
  CC(op, opcode);

  switch (opcode)
  {
    OP(0x00, {                                                               }); /* NOP              */
    OP(0x01, { BC = ARG16();                                                 }); /* LD   BC,w        */
    OP(0x02, { WM( BC, A ); WZ_L = (BC + 1) & 0xFF;  WZ_H = A;               }); /* LD   (BC),A      */
    OP(0x03, { BC++;                                                         }); /* INC  BC          */
    OP(0x04, { B = INC(B);                                                   }); /* INC  B           */
    OP(0x05, { B = DEC(B);                                                   }); /* DEC  B           */
    OP(0x06, { B = ARG();                                                    }); /* LD   B,n         */
    OP(0x07, { RLCA;                                                         }); /* RLCA             */

    OP(0x08, { EX_AF;                                                        }); /* EX   AF,AF'      */
    OP(0x09, { ADD16(hl, bc);                                                }); /* ADD  HL,BC       */
    OP(0x0a, { A = RM( BC ); WZ=BC+1;                                        }); /* LD   A,(BC)      */
    OP(0x0b, { BC--;                                                         }); /* DEC  BC          */
    OP(0x0c, { C = INC(C);                                                   }); /* INC  C           */
    OP(0x0d, { C = DEC(C);                                                   }); /* DEC  C           */
    OP(0x0e, { C = ARG();                                                    }); /* LD   C,n         */
    OP(0x0f, { RRCA;                                                         }); /* RRCA             */

    OP(0x10, { B--; JR_COND( B, 0x10 );                                      }); /* DJNZ o           */
    OP(0x11, { DE = ARG16();                                                 }); /* LD   DE,w        */
    OP(0x12, { WM( DE, A ); WZ_L = (DE + 1) & 0xFF;  WZ_H = A;               }); /* LD   (DE),A      */
    OP(0x13, { DE++;                                                         }); /* INC  DE          */
    OP(0x14, { D = INC(D);                                                   }); /* INC  D           */
    OP(0x15, { D = DEC(D);                                                   }); /* DEC  D           */
    OP(0x16, { D = ARG();                                                    }); /* LD   D,n         */
    OP(0x17, { RLA;                                                          }); /* RLA              */

    OP(0x18, { JR();                                                         }); /* JR   o           */
    OP(0x19, { ADD16(hl, de);                                                }); /* ADD  HL,DE       */
    OP(0x1a, { A = RM( DE ); WZ=DE+1;                                        }); /* LD   A,(DE)      */
    OP(0x1b, { DE--;                                                         }); /* DEC  DE          */
    OP(0x1c, { E = INC(E);                                                   }); /* INC  E           */
    OP(0x1d, { E = DEC(E);                                                   }); /* DEC  E           */
    OP(0x1e, { E = ARG();                                                    }); /* LD   E,n         */
    OP(0x1f, { RRA;                                                          }); /* RRA              */

    OP(0x20, { JR_COND( !(F & ZF), 0x20 );                                   }); /* JR   NZ,o        */
    OP(0x21, { HL = ARG16();                                                 }); /* LD   HL,w        */
    OP(0x22, { EA = ARG16(); WM16( EA, &Z80.hl ); WZ = EA+1;                 }); /* LD   (w),HL      */
    OP(0x23, { HL++;                                                         }); /* INC  HL          */
    OP(0x24, { H = INC(H);                                                   }); /* INC  H           */
    OP(0x25, { H = DEC(H);                                                   }); /* DEC  H           */
    OP(0x26, { H = ARG();                                                    }); /* LD   H,n         */
    OP(0x27, { DAA;                                                          }); /* DAA              */

    OP(0x28, { JR_COND( F & ZF, 0x28 );                                      }); /* JR   Z,o         */
    OP(0x29, { ADD16(hl, hl);                                                }); /* ADD  HL,HL       */
    OP(0x2a, { EA = ARG16(); RM16( EA, &Z80.hl ); WZ = EA+1;                 }); /* LD   HL,(w)      */
    OP(0x2b, { HL--;                                                         }); /* DEC  HL          */
    OP(0x2c, { L = INC(L);                                                   }); /* INC  L           */
    OP(0x2d, { L = DEC(L);                                                   }); /* DEC  L           */
    OP(0x2e, { L = ARG();                                                    }); /* LD   L,n         */
    OP(0x2f, { A ^= 0xff; F = (F&(SF|ZF|PF|CF))|HF|NF|(A&(YF|XF));           }); /* CPL              */

    OP(0x30, { JR_COND( !(F & CF), 0x30 );                                   }); /* JR   NC,o        */
    OP(0x31, { SP = ARG16();                                                 }); /* LD   SP,w        */
    OP(0x32, { EA = ARG16(); WM( EA, A ); WZ_L=(EA+1)&0xFF;WZ_H=A;           }); /* LD   (w),A       */
    OP(0x33, { SP++;                                                         }); /* INC  SP          */
    OP(0x34, { WM( HL, INC(RM(HL)) );                                        }); /* INC  (HL)        */
    OP(0x35, { WM( HL, DEC(RM(HL)) );                                        }); /* DEC  (HL)        */
    OP(0x36, { WM( HL, ARG() );                                              }); /* LD   (HL),n      */
    OP(0x37, { F = (F & (SF|ZF|YF|XF|PF)) | CF | (A & (YF|XF));              }); /* SCF              */

    OP(0x38, { JR_COND( F & CF, 0x38 );                                      }); /* JR   C,o         */
    OP(0x39, { ADD16(hl, sp);                                                }); /* ADD  HL,SP       */
    OP(0x3a, { EA = ARG16(); A = RM( EA ); WZ = EA+1;                        }); /* LD   A,(w)       */
    OP(0x3b, { SP--;                                                         }); /* DEC  SP          */
    OP(0x3c, { A = INC(A);                                                   }); /* INC  A           */
    OP(0x3d, { A = DEC(A);                                                   }); /* DEC  A           */
    OP(0x3e, { A = ARG();                                                    }); /* LD   A,n         */
    OP(0x3f, { F = ((F&(SF|ZF|YF|XF|PF|CF))|((F&CF)<<4)|(A&(YF|XF)))^CF;     }); /* CCF              */

    OP(0x40, {                                                               }); /* LD   B,B         */
    OP(0x41, { B = C;                                                        }); /* LD   B,C         */
    OP(0x42, { B = D;                                                        }); /* LD   B,D         */
    OP(0x43, { B = E;                                                        }); /* LD   B,E         */
    OP(0x44, { B = H;                                                        }); /* LD   B,H         */
    OP(0x45, { B = L;                                                        }); /* LD   B,L         */
    OP(0x46, { B = RM(HL);                                                   }); /* LD   B,(HL)      */
    OP(0x47, { B = A;                                                        }); /* LD   B,A         */

    OP(0x48, { C = B;                                                        }); /* LD   C,B         */
    OP(0x49, {                                                               }); /* LD   C,C         */
    OP(0x4a, { C = D;                                                        }); /* LD   C,D         */
    OP(0x4b, { C = E;                                                        }); /* LD   C,E         */
    OP(0x4c, { C = H;                                                        }); /* LD   C,H         */
    OP(0x4d, { C = L;                                                        }); /* LD   C,L         */
    OP(0x4e, { C = RM(HL);                                                   }); /* LD   C,(HL)      */
    OP(0x4f, { C = A;                                                        }); /* LD   C,A         */

    OP(0x50, { D = B;                                                        }); /* LD   D,B         */
    OP(0x51, { D = C;                                                        }); /* LD   D,C         */
    OP(0x52, {                                                               }); /* LD   D,D         */
    OP(0x53, { D = E;                                                        }); /* LD   D,E         */
    OP(0x54, { D = H;                                                        }); /* LD   D,H         */
    OP(0x55, { D = L;                                                        }); /* LD   D,L         */
    OP(0x56, { D = RM(HL);                                                   }); /* LD   D,(HL)      */
    OP(0x57, { D = A;                                                        }); /* LD   D,A         */

    OP(0x58, { E = B;                                                        }); /* LD   E,B         */
    OP(0x59, { E = C;                                                        }); /* LD   E,C         */
    OP(0x5a, { E = D;                                                        }); /* LD   E,D         */
    OP(0x5b, {                                                               }); /* LD   E,E         */
    OP(0x5c, { E = H;                                                        }); /* LD   E,H         */
    OP(0x5d, { E = L;                                                        }); /* LD   E,L         */
    OP(0x5e, { E = RM(HL);                                                   }); /* LD   E,(HL)      */
    OP(0x5f, { E = A;                                                        }); /* LD   E,A         */

    OP(0x60, { H = B;                                                        }); /* LD   H,B         */
    OP(0x61, { H = C;                                                        }); /* LD   H,C         */
    OP(0x62, { H = D;                                                        }); /* LD   H,D         */
    OP(0x63, { H = E;                                                        }); /* LD   H,E         */
    OP(0x64, {                                                               }); /* LD   H,H         */
    OP(0x65, { H = L;                                                        }); /* LD   H,L         */
    OP(0x66, { H = RM(HL);                                                   }); /* LD   H,(HL)      */
    OP(0x67, { H = A;                                                        }); /* LD   H,A         */

    OP(0x68, { L = B;                                                        }); /* LD   L,B         */
    OP(0x69, { L = C;                                                        }); /* LD   L,C         */
    OP(0x6a, { L = D;                                                        }); /* LD   L,D         */
    OP(0x6b, { L = E;                                                        }); /* LD   L,E         */
    OP(0x6c, { L = H;                                                        }); /* LD   L,H         */
    OP(0x6d, {                                                               }); /* LD   L,L         */
    OP(0x6e, { L = RM(HL);                                                   }); /* LD   L,(HL)      */
    OP(0x6f, { L = A;                                                        }); /* LD   L,A         */

    OP(0x70, { WM( HL, B );                                                  }); /* LD   (HL),B      */
    OP(0x71, { WM( HL, C );                                                  }); /* LD   (HL),C      */
    OP(0x72, { WM( HL, D );                                                  }); /* LD   (HL),D      */
    OP(0x73, { WM( HL, E );                                                  }); /* LD   (HL),E      */
    OP(0x74, { WM( HL, H );                                                  }); /* LD   (HL),H      */
    OP(0x75, { WM( HL, L );                                                  }); /* LD   (HL),L      */
    OP(0x76, { ENTER_HALT;                                                   }); /* HALT             */
    OP(0x77, { WM( HL, A );                                                  }); /* LD   (HL),A      */

    OP(0x78, { A = B;                                                        }); /* LD   A,B         */
    OP(0x79, { A = C;                                                        }); /* LD   A,C         */
    OP(0x7a, { A = D;                                                        }); /* LD   A,D         */
    OP(0x7b, { A = E;                                                        }); /* LD   A,E         */
    OP(0x7c, { A = H;                                                        }); /* LD   A,H         */
    OP(0x7d, { A = L;                                                        }); /* LD   A,L         */
    OP(0x7e, { A = RM(HL);                                                   }); /* LD   A,(HL)      */
    OP(0x7f, {                                                               }); /* LD   A,A         */

    OP(0x80, { ADD(B);                                                       }); /* ADD  A,B         */
    OP(0x81, { ADD(C);                                                       }); /* ADD  A,C         */
    OP(0x82, { ADD(D);                                                       }); /* ADD  A,D         */
    OP(0x83, { ADD(E);                                                       }); /* ADD  A,E         */
    OP(0x84, { ADD(H);                                                       }); /* ADD  A,H         */
    OP(0x85, { ADD(L);                                                       }); /* ADD  A,L         */
    OP(0x86, { ADD(RM(HL));                                                  }); /* ADD  A,(HL)      */
    OP(0x87, { ADD(A);                                                       }); /* ADD  A,A         */

    OP(0x88, { ADC(B);                                                       }); /* ADC  A,B         */
    OP(0x89, { ADC(C);                                                       }); /* ADC  A,C         */
    OP(0x8a, { ADC(D);                                                       }); /* ADC  A,D         */
    OP(0x8b, { ADC(E);                                                       }); /* ADC  A,E         */
    OP(0x8c, { ADC(H);                                                       }); /* ADC  A,H         */
    OP(0x8d, { ADC(L);                                                       }); /* ADC  A,L         */
    OP(0x8e, { ADC(RM(HL));                                                  }); /* ADC  A,(HL)      */
    OP(0x8f, { ADC(A);                                                       }); /* ADC  A,A         */

    OP(0x90, { SUB(B);                                                       }); /* SUB  B           */
    OP(0x91, { SUB(C);                                                       }); /* SUB  C           */
    OP(0x92, { SUB(D);                                                       }); /* SUB  D           */
    OP(0x93, { SUB(E);                                                       }); /* SUB  E           */
    OP(0x94, { SUB(H);                                                       }); /* SUB  H           */
    OP(0x95, { SUB(L);                                                       }); /* SUB  L           */
    OP(0x96, { SUB(RM(HL));                                                  }); /* SUB  (HL)        */
    OP(0x97, { SUB(A);                                                       }); /* SUB  A           */

    OP(0x98, { SBC(B);                                                       }); /* SBC  A,B         */
    OP(0x99, { SBC(C);                                                       }); /* SBC  A,C         */
    OP(0x9a, { SBC(D);                                                       }); /* SBC  A,D         */
    OP(0x9b, { SBC(E);                                                       }); /* SBC  A,E         */
    OP(0x9c, { SBC(H);                                                       }); /* SBC  A,H         */
    OP(0x9d, { SBC(L);                                                       }); /* SBC  A,L         */
    OP(0x9e, { SBC(RM(HL));                                                  }); /* SBC  A,(HL)      */
    OP(0x9f, { SBC(A);                                                       }); /* SBC  A,A         */

    OP(0xa0, { AND(B);                                                       }); /* AND  B           */
    OP(0xa1, { AND(C);                                                       }); /* AND  C           */
    OP(0xa2, { AND(D);                                                       }); /* AND  D           */
    OP(0xa3, { AND(E);                                                       }); /* AND  E           */
    OP(0xa4, { AND(H);                                                       }); /* AND  H           */
    OP(0xa5, { AND(L);                                                       }); /* AND  L           */
    OP(0xa6, { AND(RM(HL));                                                  }); /* AND  (HL)        */
    OP(0xa7, { AND(A);                                                       }); /* AND  A           */

    OP(0xa8, { XOR(B);                                                       }); /* XOR  B           */
    OP(0xa9, { XOR(C);                                                       }); /* XOR  C           */
    OP(0xaa, { XOR(D);                                                       }); /* XOR  D           */
    OP(0xab, { XOR(E);                                                       }); /* XOR  E           */
    OP(0xac, { XOR(H);                                                       }); /* XOR  H           */
    OP(0xad, { XOR(L);                                                       }); /* XOR  L           */
    OP(0xae, { XOR(RM(HL));                                                  }); /* XOR  (HL)        */
    OP(0xaf, { XOR(A);                                                       }); /* XOR  A           */

    OP(0xb0, { OR(B);                                                        }); /* OR   B           */
    OP(0xb1, { OR(C);                                                        }); /* OR   C           */
    OP(0xb2, { OR(D);                                                        }); /* OR   D           */
    OP(0xb3, { OR(E);                                                        }); /* OR   E           */
    OP(0xb4, { OR(H);                                                        }); /* OR   H           */
    OP(0xb5, { OR(L);                                                        }); /* OR   L           */
    OP(0xb6, { OR(RM(HL));                                                   }); /* OR   (HL)        */
    OP(0xb7, { OR(A);                                                        }); /* OR   A           */

    OP(0xb8, { CP(B);                                                        }); /* CP   B           */
    OP(0xb9, { CP(C);                                                        }); /* CP   C           */
    OP(0xba, { CP(D);                                                        }); /* CP   D           */
    OP(0xbb, { CP(E);                                                        }); /* CP   E           */
    OP(0xbc, { CP(H);                                                        }); /* CP   H           */
    OP(0xbd, { CP(L);                                                        }); /* CP   L           */
    OP(0xbe, { CP(RM(HL));                                                   }); /* CP   (HL)        */
    OP(0xbf, { CP(A);                                                        }); /* CP   A           */

    OP(0xc0, { RET_COND( !(F & ZF), 0xc0 );                                  }); /* RET  NZ          */
    OP(0xc1, { POP( bc );                                                    }); /* POP  BC          */
    OP(0xc2, { JP_COND( !(F & ZF) );                                         }); /* JP   NZ,a        */
    OP(0xc3, { JP;                                                           }); /* JP   a           */
    OP(0xc4, { CALL_COND( !(F & ZF), 0xc4 );                                 }); /* CALL NZ,a        */
    OP(0xc5, { PUSH( bc );                                                   }); /* PUSH BC          */
    OP(0xc6, { ADD(ARG());                                                   }); /* ADD  A,n         */
    OP(0xc7, { RST(0x00);                                                    }); /* RST  0           */

    OP(0xc8, { RET_COND( F & ZF, 0xc8 );                                     }); /* RET  Z           */
    OP(0xc9, { POP( pc ); WZ=PCD;                                            }); /* RET              */
    OP(0xca, { JP_COND( F & ZF );                                            }); /* JP   Z,a         */
    OP(0xcb, { R++; EXEC_CB(ROP());                                          }); /* **** CB xx       */
    OP(0xcc, { CALL_COND( F & ZF, 0xcc );                                    }); /* CALL Z,a         */
    OP(0xcd, { CALL();                                                       }); /* CALL a           */
    OP(0xce, { ADC(ARG());                                                   }); /* ADC  A,n         */
    OP(0xcf, { RST(0x08);                                                    }); /* RST  1           */

    OP(0xd0, { RET_COND( !(F & CF), 0xd0 );                                                       }); /* RET  NC          */
    OP(0xd1, { POP( de );                                                                         }); /* POP  DE          */
    OP(0xd2, { JP_COND( !(F & CF) );                                                              }); /* JP   NC,a        */
    OP(0xd3, { UINT32 n = ARG() | (A << 8); OUT(n, A); WZ_L = ((n & 0xff) + 1) & 0xff;  WZ_H = A; }); /* OUT  (n),A       */
    OP(0xd4, { CALL_COND( !(F & CF), 0xd4 );                                                      }); /* CALL NC,a        */
    OP(0xd5, { PUSH( de );                                                                        }); /* PUSH DE          */
    OP(0xd6, { SUB(ARG());                                                                        }); /* SUB  n           */
    OP(0xd7, { RST(0x10);                                                                         }); /* RST  2           */

    OP(0xd8, { RET_COND( F & CF, 0xd8 );                                     }); /* RET  C           */
    OP(0xd9, { EXX;                                                          }); /* EXX              */
    OP(0xda, { JP_COND( F & CF );                                            }); /* JP   C,a         */
    OP(0xdb, { UINT32 n = ARG() | (A << 8); A = IN(n); WZ = n + 1;           }); /* IN   A,(n)       */
    OP(0xdc, { CALL_COND( F & CF, 0xdc );                                    }); /* CALL C,a         */
    OP(0xdd, { R++; EXEC_DD(ROP());                                          }); /* **** DD xx       */
    OP(0xde, { SBC(ARG());                                                   }); /* SBC  A,n         */
    OP(0xdf, { RST(0x18);                                                    }); /* RST  3           */

    OP(0xe0, { RET_COND( !(F & PF), 0xe0 );                                  }); /* RET  PO          */
    OP(0xe1, { POP( hl );                                                    }); /* POP  HL          */
    OP(0xe2, { JP_COND( !(F & PF) );                                         }); /* JP   PO,a        */
    OP(0xe3, { EXSP( hl );                                                   }); /* EX   HL,(SP)     */
    OP(0xe4, { CALL_COND( !(F & PF), 0xe4 );                                 }); /* CALL PO,a        */
    OP(0xe5, { PUSH( hl );                                                   }); /* PUSH HL          */
    OP(0xe6, { AND(ARG());                                                   }); /* AND  n           */
    OP(0xe7, { RST(0x20);                                                    }); /* RST  4           */

    OP(0xe8, { RET_COND( F & PF, 0xe8 );                                     }); /* RET  PE          */
    OP(0xe9, { PC = HL;                                                      }); /* JP   (HL)        */
    OP(0xea, { JP_COND( F & PF );                                            }); /* JP   PE,a        */
    OP(0xeb, { EX_DE_HL;                                                     }); /* EX   DE,HL       */
    OP(0xec, { CALL_COND( F & PF, 0xec );                                    }); /* CALL PE,a        */
    OP(0xed, { R++; EXEC_ED(ROP());                                          }); /* **** ED xx       */
    OP(0xee, { XOR(ARG());                                                   }); /* XOR  n           */
    OP(0xef, { RST(0x28);                                                    }); /* RST  5           */

    OP(0xf0, { RET_COND( !(F & SF), 0xf0 );                                  }); /* RET  P           */
    OP(0xf1, { POP( af );                                                    }); /* POP  AF          */
    OP(0xf2, { JP_COND( !(F & SF) );                                         }); /* JP   P,a         */
    OP(0xf3, { IFF1 = IFF2 = 0;                                              }); /* DI               */
    OP(0xf4, { CALL_COND( !(F & SF), 0xf4 );                                 }); /* CALL P,a         */
    OP(0xf5, { PUSH( af );                                                   }); /* PUSH AF          */
    OP(0xf6, { OR(ARG());                                                    }); /* OR   n           */
    OP(0xf7, { RST(0x30);                                                    }); /* RST  6           */

    OP(0xf8, { RET_COND( F & SF, 0xf8 );                                     }); /* RET  M           */
    OP(0xf9, { SP = HL;                                                      }); /* LD   SP,HL       */
    OP(0xfa, { JP_COND(F & SF);                                              }); /* JP   M,a         */
    OP(0xfb, { EI;                                                           }); /* EI               */
    OP(0xfc, { CALL_COND( F & SF, 0xfc );                                    }); /* CALL M,a         */
    OP(0xfd, { R++; EXEC_FD(ROP());                                          }); /* **** FD xx       */
    OP(0xfe, { CP(ARG());                                                    }); /* CP   n           */
    OP(0xff, { RST(0x38);                                                    }); /* RST  7           */
  }
}

ALWAYS_INLINE void take_interrupt(void)
{
  int irq_vector;

  /* Check if processor was halted */
  LEAVE_HALT;

  /* Clear both interrupt flip flops */
  IFF1 = IFF2 = 0;

  /* call back the cpu interface to retrieve the vector */
  irq_vector = (*Z80.irq_callback)(0);

  LOG(("Z80 #%d single int. irq_vector $%02x\n", cpu_getactivecpu(), irq_vector));

  /* Interrupt mode 2. Call [Z80.i:databyte] */
  if (IM == 2)
  {
    irq_vector = (irq_vector & 0xff) | (I << 8);
    PUSH(pc);
    RM16(irq_vector, &Z80.pc);
    LOG(("Z80 #%d IM2 [$%04x] = $%04x\n", cpu_getactivecpu(), irq_vector, PCD));
    /* CALL $xxxx + 'interrupt latency' cycles */
    z80_ICount -= cc[Z80_TABLE_op][0xcd] + cc[Z80_TABLE_ex][0xff];
  }
  /* Interrupt mode 1. RST 38h */
  else if (IM == 1)
  {
    LOG(("Z80 #%d IM1 $0038\n", cpu_getactivecpu()));
    PUSH(pc);
    PCD = 0x0038;
    /* RST $38 + 'interrupt latency' cycles */
    z80_ICount -= cc[Z80_TABLE_op][0xff] + cc[Z80_TABLE_ex][0xff];
  }
  else
  {
    /* Interrupt mode 0. We check for CALL and JP instructions, */
    /* if neither of these were found we assume a 1 byte opcode */
    /* was placed on the databus                */
    LOG(("Z80 #%d IM0 $%04x\n", cpu_getactivecpu(), irq_vector));
    switch (irq_vector & 0xff0000)
    {
    case 0xcd0000: /* call */
      PUSH(pc);
      PCD = irq_vector & 0xffff;
      /* CALL $xxxx + 'interrupt latency' cycles */
      z80_ICount -= cc[Z80_TABLE_op][0xcd] + cc[Z80_TABLE_ex][0xff];
      break;
    case 0xc30000: /* jump */
      PCD = irq_vector & 0xffff;
      /* JP $xxxx + 2 cycles */
      z80_ICount -= cc[Z80_TABLE_op][0xc3] + cc[Z80_TABLE_ex][0xff];
      break;
    default: /* rst (or other opcodes?) */
      PUSH(pc);
      PCD = irq_vector & 0x0038;
      /* RST $xx + 2 cycles */
      z80_ICount -= cc[Z80_TABLE_op][0xff] + cc[Z80_TABLE_ex][0xff];
      break;
    }
  }
  WZ = PCD;
}

/****************************************************************************
 * Processor initialization
 ****************************************************************************/
void z80_init(int index, int clock, const void *config, int (*irq_callback)(int))
{
  int i, p;

  if( !SZHVC_add || !SZHVC_sub )
  {
    int oldval, newval, val;
    UINT8 *padd, *padc, *psub, *psbc;
    /* allocate big flag arrays once */
    SZHVC_add = (UINT8 *)malloc(2*256*256);
    SZHVC_sub = (UINT8 *)malloc(2*256*256);
    if( !SZHVC_add || !SZHVC_sub )
    {
      return;
    }
    padd = &SZHVC_add[  0*256];
    padc = &SZHVC_add[256*256];
    psub = &SZHVC_sub[  0*256];
    psbc = &SZHVC_sub[256*256];
    for (oldval = 0; oldval < 256; oldval++)
    {
      for (newval = 0; newval < 256; newval++)
      {
        /* add or adc w/o carry set */
        val = newval - oldval;
        *padd = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
        *padd |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
        if( (newval & 0x0f) < (oldval & 0x0f) ) *padd |= HF;
        if( newval < oldval ) *padd |= CF;
        if( (val^oldval^0x80) & (val^newval) & 0x80 ) *padd |= VF;
        padd++;

        /* adc with carry set */
        val = newval - oldval - 1;
        *padc = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
        *padc |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
        if( (newval & 0x0f) <= (oldval & 0x0f) ) *padc |= HF;
        if( newval <= oldval ) *padc |= CF;
        if( (val^oldval^0x80) & (val^newval) & 0x80 ) *padc |= VF;
        padc++;

        /* cp, sub or sbc w/o carry set */
        val = oldval - newval;
        *psub = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
        *psub |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
        if( (newval & 0x0f) > (oldval & 0x0f) ) *psub |= HF;
        if( newval > oldval ) *psub |= CF;
        if( (val^oldval) & (oldval^newval) & 0x80 ) *psub |= VF;
        psub++;

        /* sbc with carry set */
        val = oldval - newval - 1;
        *psbc = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
        *psbc |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
        if( (newval & 0x0f) >= (oldval & 0x0f) ) *psbc |= HF;
        if( newval >= oldval ) *psbc |= CF;
        if( (val^oldval) & (oldval^newval) & 0x80 ) *psbc |= VF;
        psbc++;
      }
    }
  }

  for (i = 0; i < 256; i++)
  {
    p = 0;
    if( i&0x01 ) ++p;
    if( i&0x02 ) ++p;
    if( i&0x04 ) ++p;
    if( i&0x08 ) ++p;
    if( i&0x10 ) ++p;
    if( i&0x20 ) ++p;
    if( i&0x40 ) ++p;
    if( i&0x80 ) ++p;
    SZP[i] = SZ(i) | ((p & 1) ? 0 : PF);
  }

  /* Reset registers to their initial values */
  memset(&Z80, 0, sizeof(Z80));
  IX = IY = 0xffff; /* IX and IY are FFFF after a reset! */
  F = ZF;      /* Zero flag is set */
  SP = 0xdff0; /* fix Shadow Dancer & Ace of Aces (normally set by BIOS) */
  Z80.daisy = config;
  Z80.irq_callback = irq_callback ? irq_callback : dummy_irq;
  Z80.mem_read = dummy_read;
  Z80.mem_write = dummy_write;
  Z80.port_read = dummy_read;
  Z80.port_write = dummy_write;
}

/****************************************************************************
 * Do a reset
 ****************************************************************************/
void z80_reset(void)
{
  PC = 0x0000;
  I = 0;
  R = 0;
  R2 = 0;
  IM = 0;
  IFF1 = IFF2 = 0;
  HALT = 0;
  WZ = PCD;
  Z80.after_ei = false;
}

void z80_exit(void)
{
  if (SZHVC_add) free(SZHVC_add);
  SZHVC_add = NULL;
  if (SZHVC_sub) free(SZHVC_sub);
  SZHVC_sub = NULL;
}

/****************************************************************************
 * Execute 'cycles' T-states. Return number of T-states really executed
 ****************************************************************************/
IRAM_ATTR int z80_execute(int cycles)
{
  z80_ICount = cycles;
  z80_requested_cycles = z80_ICount;
  z80_exec = 1;

  /* check for NMIs on the way in; they can only be set externally */
  /* via timers, and can't be dynamically enabled, so it is safe */
  /* to just check here */
  if (Z80.nmi_pending)
  {
    LOG(("Z80 #%d take NMI\n", cpu_getactivecpu()));
    LEAVE_HALT;      /* Check if processor was halted */

    IFF1 = 0;
    PUSH(pc);
    PCD = 0x0066;
    WZ = PCD;
    z80_ICount -= 11;
    Z80.nmi_pending = false;
  }

  while( z80_ICount > 0 )
  {
    /* check for IRQs before each instruction */
    if (Z80.irq_state != CLEAR_LINE && IFF1 && !Z80.after_ei)
      take_interrupt();
    Z80.after_ei = false;

    if (z80_ICount > 0)
    {
      R++;
      EXEC_OP(ROP());
    }
  }

  z80_exec = 0;
  z80_cycle_count += (cycles - z80_ICount);

  return cycles - z80_ICount;
}

/****************************************************************************
 * Burn 'cycles' T-states. Adjust R register for the lost time
 ****************************************************************************/
void z80_burn(int cycles)
{
  if( cycles > 0 )
  {
    /* NOP takes 4 cycles per instruction */
    int n = (cycles + 3) / 4;
    R += n;
    z80_ICount -= 4 * n;
  }
}

/****************************************************************************
 * Get all registers in given buffer
 ****************************************************************************/
void z80_get_context(void *dst)
{
  if( dst )
    *(Z80_Regs*)dst = Z80;
}

/****************************************************************************
 * Set all registers to given values
 ****************************************************************************/
void z80_set_context(void *src)
{
  if( src )
    Z80 = *(Z80_Regs*)src;
}

/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
void z80_set_irq_line(int irqline, int state)
{
  if (irqline == INPUT_LINE_NMI)
  {
    /* mark an NMI pending on the rising edge */
    if (Z80.nmi_state == CLEAR_LINE && state != CLEAR_LINE)
      Z80.nmi_pending = true;
    Z80.nmi_state = state;
  }
  else
  {
    /* update the IRQ state via the daisy chain */
    Z80.irq_state = state;

    /* the main execute loop will take the interrupt */
  }
}

/****************************************************************************
 *
 ****************************************************************************/
void z80_reset_cycle_count(void)
{
  z80_cycle_count = 0;
}

int z80_get_elapsed_cycles(void)
{
  if(z80_exec == 1)
  {
    // inside execution loop
    return z80_cycle_count + (z80_requested_cycles - z80_ICount);
  }

  return z80_cycle_count;
}
