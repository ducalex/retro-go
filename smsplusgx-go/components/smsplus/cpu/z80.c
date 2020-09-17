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
#include <esp_attr.h>

/* execute main opcodes inside a big switch statement */
#define BIG_SWITCH 0

/* Show debugging messages */
#define VERBOSE 0

#define INLINE static inline

#if VERBOSE
#define LOG(x)  logerror x
#else
#define LOG(x)
#endif

int z80_cycle_count = 0;        /* running total of cycles executed */

#define cpu_readbyte(a)   (cpu_readmap[(a) >> 10][(a) & 0x03FF])

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

static UINT8 SZ[256];       /* zero and sign flags */
static UINT8 SZ_BIT[256];   /* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static UINT8 SZP[256];      /* zero, sign and parity flags */
static UINT8 SZHV_inc[256]; /* zero, sign, half carry and overflow flags INC r8 */
static UINT8 SZHV_dec[256]; /* zero, sign, half carry and overflow flags DEC r8 */

static UINT8 *SZHVC_add = 0;
static UINT8 *SZHVC_sub = 0;

DRAM_ATTR static const UINT8 cc[6][0x100] = {
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

typedef void (*funcptr)(void);

#define PROTOTYPES(prefix) \
  INLINE void prefix##_00(void); INLINE void prefix##_01(void); INLINE void prefix##_02(void); INLINE void prefix##_03(void); \
  INLINE void prefix##_04(void); INLINE void prefix##_05(void); INLINE void prefix##_06(void); INLINE void prefix##_07(void); \
  INLINE void prefix##_08(void); INLINE void prefix##_09(void); INLINE void prefix##_0a(void); INLINE void prefix##_0b(void); \
  INLINE void prefix##_0c(void); INLINE void prefix##_0d(void); INLINE void prefix##_0e(void); INLINE void prefix##_0f(void); \
  INLINE void prefix##_10(void); INLINE void prefix##_11(void); INLINE void prefix##_12(void); INLINE void prefix##_13(void); \
  INLINE void prefix##_14(void); INLINE void prefix##_15(void); INLINE void prefix##_16(void); INLINE void prefix##_17(void); \
  INLINE void prefix##_18(void); INLINE void prefix##_19(void); INLINE void prefix##_1a(void); INLINE void prefix##_1b(void); \
  INLINE void prefix##_1c(void); INLINE void prefix##_1d(void); INLINE void prefix##_1e(void); INLINE void prefix##_1f(void); \
  INLINE void prefix##_20(void); INLINE void prefix##_21(void); INLINE void prefix##_22(void); INLINE void prefix##_23(void); \
  INLINE void prefix##_24(void); INLINE void prefix##_25(void); INLINE void prefix##_26(void); INLINE void prefix##_27(void); \
  INLINE void prefix##_28(void); INLINE void prefix##_29(void); INLINE void prefix##_2a(void); INLINE void prefix##_2b(void); \
  INLINE void prefix##_2c(void); INLINE void prefix##_2d(void); INLINE void prefix##_2e(void); INLINE void prefix##_2f(void); \
  INLINE void prefix##_30(void); INLINE void prefix##_31(void); INLINE void prefix##_32(void); INLINE void prefix##_33(void); \
  INLINE void prefix##_34(void); INLINE void prefix##_35(void); INLINE void prefix##_36(void); INLINE void prefix##_37(void); \
  INLINE void prefix##_38(void); INLINE void prefix##_39(void); INLINE void prefix##_3a(void); INLINE void prefix##_3b(void); \
  INLINE void prefix##_3c(void); INLINE void prefix##_3d(void); INLINE void prefix##_3e(void); INLINE void prefix##_3f(void); \
  INLINE void prefix##_40(void); INLINE void prefix##_41(void); INLINE void prefix##_42(void); INLINE void prefix##_43(void); \
  INLINE void prefix##_44(void); INLINE void prefix##_45(void); INLINE void prefix##_46(void); INLINE void prefix##_47(void); \
  INLINE void prefix##_48(void); INLINE void prefix##_49(void); INLINE void prefix##_4a(void); INLINE void prefix##_4b(void); \
  INLINE void prefix##_4c(void); INLINE void prefix##_4d(void); INLINE void prefix##_4e(void); INLINE void prefix##_4f(void); \
  INLINE void prefix##_50(void); INLINE void prefix##_51(void); INLINE void prefix##_52(void); INLINE void prefix##_53(void); \
  INLINE void prefix##_54(void); INLINE void prefix##_55(void); INLINE void prefix##_56(void); INLINE void prefix##_57(void); \
  INLINE void prefix##_58(void); INLINE void prefix##_59(void); INLINE void prefix##_5a(void); INLINE void prefix##_5b(void); \
  INLINE void prefix##_5c(void); INLINE void prefix##_5d(void); INLINE void prefix##_5e(void); INLINE void prefix##_5f(void); \
  INLINE void prefix##_60(void); INLINE void prefix##_61(void); INLINE void prefix##_62(void); INLINE void prefix##_63(void); \
  INLINE void prefix##_64(void); INLINE void prefix##_65(void); INLINE void prefix##_66(void); INLINE void prefix##_67(void); \
  INLINE void prefix##_68(void); INLINE void prefix##_69(void); INLINE void prefix##_6a(void); INLINE void prefix##_6b(void); \
  INLINE void prefix##_6c(void); INLINE void prefix##_6d(void); INLINE void prefix##_6e(void); INLINE void prefix##_6f(void); \
  INLINE void prefix##_70(void); INLINE void prefix##_71(void); INLINE void prefix##_72(void); INLINE void prefix##_73(void); \
  INLINE void prefix##_74(void); INLINE void prefix##_75(void); INLINE void prefix##_76(void); INLINE void prefix##_77(void); \
  INLINE void prefix##_78(void); INLINE void prefix##_79(void); INLINE void prefix##_7a(void); INLINE void prefix##_7b(void); \
  INLINE void prefix##_7c(void); INLINE void prefix##_7d(void); INLINE void prefix##_7e(void); INLINE void prefix##_7f(void); \
  INLINE void prefix##_80(void); INLINE void prefix##_81(void); INLINE void prefix##_82(void); INLINE void prefix##_83(void); \
  INLINE void prefix##_84(void); INLINE void prefix##_85(void); INLINE void prefix##_86(void); INLINE void prefix##_87(void); \
  INLINE void prefix##_88(void); INLINE void prefix##_89(void); INLINE void prefix##_8a(void); INLINE void prefix##_8b(void); \
  INLINE void prefix##_8c(void); INLINE void prefix##_8d(void); INLINE void prefix##_8e(void); INLINE void prefix##_8f(void); \
  INLINE void prefix##_90(void); INLINE void prefix##_91(void); INLINE void prefix##_92(void); INLINE void prefix##_93(void); \
  INLINE void prefix##_94(void); INLINE void prefix##_95(void); INLINE void prefix##_96(void); INLINE void prefix##_97(void); \
  INLINE void prefix##_98(void); INLINE void prefix##_99(void); INLINE void prefix##_9a(void); INLINE void prefix##_9b(void); \
  INLINE void prefix##_9c(void); INLINE void prefix##_9d(void); INLINE void prefix##_9e(void); INLINE void prefix##_9f(void); \
  INLINE void prefix##_a0(void); INLINE void prefix##_a1(void); INLINE void prefix##_a2(void); INLINE void prefix##_a3(void); \
  INLINE void prefix##_a4(void); INLINE void prefix##_a5(void); INLINE void prefix##_a6(void); INLINE void prefix##_a7(void); \
  INLINE void prefix##_a8(void); INLINE void prefix##_a9(void); INLINE void prefix##_aa(void); INLINE void prefix##_ab(void); \
  INLINE void prefix##_ac(void); INLINE void prefix##_ad(void); INLINE void prefix##_ae(void); INLINE void prefix##_af(void); \
  INLINE void prefix##_b0(void); INLINE void prefix##_b1(void); INLINE void prefix##_b2(void); INLINE void prefix##_b3(void); \
  INLINE void prefix##_b4(void); INLINE void prefix##_b5(void); INLINE void prefix##_b6(void); INLINE void prefix##_b7(void); \
  INLINE void prefix##_b8(void); INLINE void prefix##_b9(void); INLINE void prefix##_ba(void); INLINE void prefix##_bb(void); \
  INLINE void prefix##_bc(void); INLINE void prefix##_bd(void); INLINE void prefix##_be(void); INLINE void prefix##_bf(void); \
  INLINE void prefix##_c0(void); INLINE void prefix##_c1(void); INLINE void prefix##_c2(void); INLINE void prefix##_c3(void); \
  INLINE void prefix##_c4(void); INLINE void prefix##_c5(void); INLINE void prefix##_c6(void); INLINE void prefix##_c7(void); \
  INLINE void prefix##_c8(void); INLINE void prefix##_c9(void); INLINE void prefix##_ca(void); INLINE void prefix##_cb(void); \
  INLINE void prefix##_cc(void); INLINE void prefix##_cd(void); INLINE void prefix##_ce(void); INLINE void prefix##_cf(void); \
  INLINE void prefix##_d0(void); INLINE void prefix##_d1(void); INLINE void prefix##_d2(void); INLINE void prefix##_d3(void); \
  INLINE void prefix##_d4(void); INLINE void prefix##_d5(void); INLINE void prefix##_d6(void); INLINE void prefix##_d7(void); \
  INLINE void prefix##_d8(void); INLINE void prefix##_d9(void); INLINE void prefix##_da(void); INLINE void prefix##_db(void); \
  INLINE void prefix##_dc(void); INLINE void prefix##_dd(void); INLINE void prefix##_de(void); INLINE void prefix##_df(void); \
  INLINE void prefix##_e0(void); INLINE void prefix##_e1(void); INLINE void prefix##_e2(void); INLINE void prefix##_e3(void); \
  INLINE void prefix##_e4(void); INLINE void prefix##_e5(void); INLINE void prefix##_e6(void); INLINE void prefix##_e7(void); \
  INLINE void prefix##_e8(void); INLINE void prefix##_e9(void); INLINE void prefix##_ea(void); INLINE void prefix##_eb(void); \
  INLINE void prefix##_ec(void); INLINE void prefix##_ed(void); INLINE void prefix##_ee(void); INLINE void prefix##_ef(void); \
  INLINE void prefix##_f0(void); INLINE void prefix##_f1(void); INLINE void prefix##_f2(void); INLINE void prefix##_f3(void); \
  INLINE void prefix##_f4(void); INLINE void prefix##_f5(void); INLINE void prefix##_f6(void); INLINE void prefix##_f7(void); \
  INLINE void prefix##_f8(void); INLINE void prefix##_f9(void); INLINE void prefix##_fa(void); INLINE void prefix##_fb(void); \
  INLINE void prefix##_fc(void); INLINE void prefix##_fd(void); INLINE void prefix##_fe(void); INLINE void prefix##_ff(void); \
  static const funcptr __attribute__((unused)) Z80##prefix[0x100] = {  \
    prefix##_00,prefix##_01,prefix##_02,prefix##_03,prefix##_04,prefix##_05,prefix##_06,prefix##_07, \
    prefix##_08,prefix##_09,prefix##_0a,prefix##_0b,prefix##_0c,prefix##_0d,prefix##_0e,prefix##_0f, \
    prefix##_10,prefix##_11,prefix##_12,prefix##_13,prefix##_14,prefix##_15,prefix##_16,prefix##_17, \
    prefix##_18,prefix##_19,prefix##_1a,prefix##_1b,prefix##_1c,prefix##_1d,prefix##_1e,prefix##_1f, \
    prefix##_20,prefix##_21,prefix##_22,prefix##_23,prefix##_24,prefix##_25,prefix##_26,prefix##_27, \
    prefix##_28,prefix##_29,prefix##_2a,prefix##_2b,prefix##_2c,prefix##_2d,prefix##_2e,prefix##_2f, \
    prefix##_30,prefix##_31,prefix##_32,prefix##_33,prefix##_34,prefix##_35,prefix##_36,prefix##_37, \
    prefix##_38,prefix##_39,prefix##_3a,prefix##_3b,prefix##_3c,prefix##_3d,prefix##_3e,prefix##_3f, \
    prefix##_40,prefix##_41,prefix##_42,prefix##_43,prefix##_44,prefix##_45,prefix##_46,prefix##_47, \
    prefix##_48,prefix##_49,prefix##_4a,prefix##_4b,prefix##_4c,prefix##_4d,prefix##_4e,prefix##_4f, \
    prefix##_50,prefix##_51,prefix##_52,prefix##_53,prefix##_54,prefix##_55,prefix##_56,prefix##_57, \
    prefix##_58,prefix##_59,prefix##_5a,prefix##_5b,prefix##_5c,prefix##_5d,prefix##_5e,prefix##_5f, \
    prefix##_60,prefix##_61,prefix##_62,prefix##_63,prefix##_64,prefix##_65,prefix##_66,prefix##_67, \
    prefix##_68,prefix##_69,prefix##_6a,prefix##_6b,prefix##_6c,prefix##_6d,prefix##_6e,prefix##_6f, \
    prefix##_70,prefix##_71,prefix##_72,prefix##_73,prefix##_74,prefix##_75,prefix##_76,prefix##_77, \
    prefix##_78,prefix##_79,prefix##_7a,prefix##_7b,prefix##_7c,prefix##_7d,prefix##_7e,prefix##_7f, \
    prefix##_80,prefix##_81,prefix##_82,prefix##_83,prefix##_84,prefix##_85,prefix##_86,prefix##_87, \
    prefix##_88,prefix##_89,prefix##_8a,prefix##_8b,prefix##_8c,prefix##_8d,prefix##_8e,prefix##_8f, \
    prefix##_90,prefix##_91,prefix##_92,prefix##_93,prefix##_94,prefix##_95,prefix##_96,prefix##_97, \
    prefix##_98,prefix##_99,prefix##_9a,prefix##_9b,prefix##_9c,prefix##_9d,prefix##_9e,prefix##_9f, \
    prefix##_a0,prefix##_a1,prefix##_a2,prefix##_a3,prefix##_a4,prefix##_a5,prefix##_a6,prefix##_a7, \
    prefix##_a8,prefix##_a9,prefix##_aa,prefix##_ab,prefix##_ac,prefix##_ad,prefix##_ae,prefix##_af, \
    prefix##_b0,prefix##_b1,prefix##_b2,prefix##_b3,prefix##_b4,prefix##_b5,prefix##_b6,prefix##_b7, \
    prefix##_b8,prefix##_b9,prefix##_ba,prefix##_bb,prefix##_bc,prefix##_bd,prefix##_be,prefix##_bf, \
    prefix##_c0,prefix##_c1,prefix##_c2,prefix##_c3,prefix##_c4,prefix##_c5,prefix##_c6,prefix##_c7, \
    prefix##_c8,prefix##_c9,prefix##_ca,prefix##_cb,prefix##_cc,prefix##_cd,prefix##_ce,prefix##_cf, \
    prefix##_d0,prefix##_d1,prefix##_d2,prefix##_d3,prefix##_d4,prefix##_d5,prefix##_d6,prefix##_d7, \
    prefix##_d8,prefix##_d9,prefix##_da,prefix##_db,prefix##_dc,prefix##_dd,prefix##_de,prefix##_df, \
    prefix##_e0,prefix##_e1,prefix##_e2,prefix##_e3,prefix##_e4,prefix##_e5,prefix##_e6,prefix##_e7, \
    prefix##_e8,prefix##_e9,prefix##_ea,prefix##_eb,prefix##_ec,prefix##_ed,prefix##_ee,prefix##_ef, \
    prefix##_f0,prefix##_f1,prefix##_f2,prefix##_f3,prefix##_f4,prefix##_f5,prefix##_f6,prefix##_f7, \
    prefix##_f8,prefix##_f9,prefix##_fa,prefix##_fb,prefix##_fc,prefix##_fd,prefix##_fe,prefix##_ff  \
  }

PROTOTYPES(op);
PROTOTYPES(cb);
PROTOTYPES(dd);
PROTOTYPES(ed);
PROTOTYPES(fd);
PROTOTYPES(xycb);


/***************************************************************
 * define an opcode function
 ***************************************************************/
#define OP(prefix,opcode)  INLINE void prefix##_##opcode(void)

INLINE void illegal_op()
{
  #if VERBOSE
  logerror("Z80 #%d ill. opcode $ed $%02x\n",cpu_getactivecpu(), cpu_readop((PCD-1)&0xffff);
  #endif
}

// Illegal OP but executes the same opcode in the "op" prefix
#define OP_ILLEGAL1(prefix, opcode) INLINE void prefix##_##opcode(void) {op_##opcode();}

// Illegal OP, NO-OP or abort
#define OP_ILLEGAL2(prefix, opcode) INLINE void prefix##_##opcode(void) {} // illegal_op();

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define CC(prefix,opcode) z80_ICount -= cc[Z80_TABLE_##prefix][opcode]

/***************************************************************
 * execute an opcode
 ***************************************************************/
#define EXEC(prefix,opcode)      \
{                                \
  unsigned op = opcode;          \
  CC(prefix,op);                 \
  (*Z80##prefix[op])();          \
}

#if BIG_SWITCH
#define EXEC_INLINE(prefix,opcode)  \
{                                   \
  unsigned op = opcode;             \
  CC(prefix,op);                    \
  switch(op)                        \
  {                                 \
  case 0x00:prefix##_##00();break; case 0x01:prefix##_##01();break; case 0x02:prefix##_##02();break; case 0x03:prefix##_##03();break; \
  case 0x04:prefix##_##04();break; case 0x05:prefix##_##05();break; case 0x06:prefix##_##06();break; case 0x07:prefix##_##07();break; \
  case 0x08:prefix##_##08();break; case 0x09:prefix##_##09();break; case 0x0a:prefix##_##0a();break; case 0x0b:prefix##_##0b();break; \
  case 0x0c:prefix##_##0c();break; case 0x0d:prefix##_##0d();break; case 0x0e:prefix##_##0e();break; case 0x0f:prefix##_##0f();break; \
  case 0x10:prefix##_##10();break; case 0x11:prefix##_##11();break; case 0x12:prefix##_##12();break; case 0x13:prefix##_##13();break; \
  case 0x14:prefix##_##14();break; case 0x15:prefix##_##15();break; case 0x16:prefix##_##16();break; case 0x17:prefix##_##17();break; \
  case 0x18:prefix##_##18();break; case 0x19:prefix##_##19();break; case 0x1a:prefix##_##1a();break; case 0x1b:prefix##_##1b();break; \
  case 0x1c:prefix##_##1c();break; case 0x1d:prefix##_##1d();break; case 0x1e:prefix##_##1e();break; case 0x1f:prefix##_##1f();break; \
  case 0x20:prefix##_##20();break; case 0x21:prefix##_##21();break; case 0x22:prefix##_##22();break; case 0x23:prefix##_##23();break; \
  case 0x24:prefix##_##24();break; case 0x25:prefix##_##25();break; case 0x26:prefix##_##26();break; case 0x27:prefix##_##27();break; \
  case 0x28:prefix##_##28();break; case 0x29:prefix##_##29();break; case 0x2a:prefix##_##2a();break; case 0x2b:prefix##_##2b();break; \
  case 0x2c:prefix##_##2c();break; case 0x2d:prefix##_##2d();break; case 0x2e:prefix##_##2e();break; case 0x2f:prefix##_##2f();break; \
  case 0x30:prefix##_##30();break; case 0x31:prefix##_##31();break; case 0x32:prefix##_##32();break; case 0x33:prefix##_##33();break; \
  case 0x34:prefix##_##34();break; case 0x35:prefix##_##35();break; case 0x36:prefix##_##36();break; case 0x37:prefix##_##37();break; \
  case 0x38:prefix##_##38();break; case 0x39:prefix##_##39();break; case 0x3a:prefix##_##3a();break; case 0x3b:prefix##_##3b();break; \
  case 0x3c:prefix##_##3c();break; case 0x3d:prefix##_##3d();break; case 0x3e:prefix##_##3e();break; case 0x3f:prefix##_##3f();break; \
  case 0x40:prefix##_##40();break; case 0x41:prefix##_##41();break; case 0x42:prefix##_##42();break; case 0x43:prefix##_##43();break; \
  case 0x44:prefix##_##44();break; case 0x45:prefix##_##45();break; case 0x46:prefix##_##46();break; case 0x47:prefix##_##47();break; \
  case 0x48:prefix##_##48();break; case 0x49:prefix##_##49();break; case 0x4a:prefix##_##4a();break; case 0x4b:prefix##_##4b();break; \
  case 0x4c:prefix##_##4c();break; case 0x4d:prefix##_##4d();break; case 0x4e:prefix##_##4e();break; case 0x4f:prefix##_##4f();break; \
  case 0x50:prefix##_##50();break; case 0x51:prefix##_##51();break; case 0x52:prefix##_##52();break; case 0x53:prefix##_##53();break; \
  case 0x54:prefix##_##54();break; case 0x55:prefix##_##55();break; case 0x56:prefix##_##56();break; case 0x57:prefix##_##57();break; \
  case 0x58:prefix##_##58();break; case 0x59:prefix##_##59();break; case 0x5a:prefix##_##5a();break; case 0x5b:prefix##_##5b();break; \
  case 0x5c:prefix##_##5c();break; case 0x5d:prefix##_##5d();break; case 0x5e:prefix##_##5e();break; case 0x5f:prefix##_##5f();break; \
  case 0x60:prefix##_##60();break; case 0x61:prefix##_##61();break; case 0x62:prefix##_##62();break; case 0x63:prefix##_##63();break; \
  case 0x64:prefix##_##64();break; case 0x65:prefix##_##65();break; case 0x66:prefix##_##66();break; case 0x67:prefix##_##67();break; \
  case 0x68:prefix##_##68();break; case 0x69:prefix##_##69();break; case 0x6a:prefix##_##6a();break; case 0x6b:prefix##_##6b();break; \
  case 0x6c:prefix##_##6c();break; case 0x6d:prefix##_##6d();break; case 0x6e:prefix##_##6e();break; case 0x6f:prefix##_##6f();break; \
  case 0x70:prefix##_##70();break; case 0x71:prefix##_##71();break; case 0x72:prefix##_##72();break; case 0x73:prefix##_##73();break; \
  case 0x74:prefix##_##74();break; case 0x75:prefix##_##75();break; case 0x76:prefix##_##76();break; case 0x77:prefix##_##77();break; \
  case 0x78:prefix##_##78();break; case 0x79:prefix##_##79();break; case 0x7a:prefix##_##7a();break; case 0x7b:prefix##_##7b();break; \
  case 0x7c:prefix##_##7c();break; case 0x7d:prefix##_##7d();break; case 0x7e:prefix##_##7e();break; case 0x7f:prefix##_##7f();break; \
  case 0x80:prefix##_##80();break; case 0x81:prefix##_##81();break; case 0x82:prefix##_##82();break; case 0x83:prefix##_##83();break; \
  case 0x84:prefix##_##84();break; case 0x85:prefix##_##85();break; case 0x86:prefix##_##86();break; case 0x87:prefix##_##87();break; \
  case 0x88:prefix##_##88();break; case 0x89:prefix##_##89();break; case 0x8a:prefix##_##8a();break; case 0x8b:prefix##_##8b();break; \
  case 0x8c:prefix##_##8c();break; case 0x8d:prefix##_##8d();break; case 0x8e:prefix##_##8e();break; case 0x8f:prefix##_##8f();break; \
  case 0x90:prefix##_##90();break; case 0x91:prefix##_##91();break; case 0x92:prefix##_##92();break; case 0x93:prefix##_##93();break; \
  case 0x94:prefix##_##94();break; case 0x95:prefix##_##95();break; case 0x96:prefix##_##96();break; case 0x97:prefix##_##97();break; \
  case 0x98:prefix##_##98();break; case 0x99:prefix##_##99();break; case 0x9a:prefix##_##9a();break; case 0x9b:prefix##_##9b();break; \
  case 0x9c:prefix##_##9c();break; case 0x9d:prefix##_##9d();break; case 0x9e:prefix##_##9e();break; case 0x9f:prefix##_##9f();break; \
  case 0xa0:prefix##_##a0();break; case 0xa1:prefix##_##a1();break; case 0xa2:prefix##_##a2();break; case 0xa3:prefix##_##a3();break; \
  case 0xa4:prefix##_##a4();break; case 0xa5:prefix##_##a5();break; case 0xa6:prefix##_##a6();break; case 0xa7:prefix##_##a7();break; \
  case 0xa8:prefix##_##a8();break; case 0xa9:prefix##_##a9();break; case 0xaa:prefix##_##aa();break; case 0xab:prefix##_##ab();break; \
  case 0xac:prefix##_##ac();break; case 0xad:prefix##_##ad();break; case 0xae:prefix##_##ae();break; case 0xaf:prefix##_##af();break; \
  case 0xb0:prefix##_##b0();break; case 0xb1:prefix##_##b1();break; case 0xb2:prefix##_##b2();break; case 0xb3:prefix##_##b3();break; \
  case 0xb4:prefix##_##b4();break; case 0xb5:prefix##_##b5();break; case 0xb6:prefix##_##b6();break; case 0xb7:prefix##_##b7();break; \
  case 0xb8:prefix##_##b8();break; case 0xb9:prefix##_##b9();break; case 0xba:prefix##_##ba();break; case 0xbb:prefix##_##bb();break; \
  case 0xbc:prefix##_##bc();break; case 0xbd:prefix##_##bd();break; case 0xbe:prefix##_##be();break; case 0xbf:prefix##_##bf();break; \
  case 0xc0:prefix##_##c0();break; case 0xc1:prefix##_##c1();break; case 0xc2:prefix##_##c2();break; case 0xc3:prefix##_##c3();break; \
  case 0xc4:prefix##_##c4();break; case 0xc5:prefix##_##c5();break; case 0xc6:prefix##_##c6();break; case 0xc7:prefix##_##c7();break; \
  case 0xc8:prefix##_##c8();break; case 0xc9:prefix##_##c9();break; case 0xca:prefix##_##ca();break; case 0xcb:prefix##_##cb();break; \
  case 0xcc:prefix##_##cc();break; case 0xcd:prefix##_##cd();break; case 0xce:prefix##_##ce();break; case 0xcf:prefix##_##cf();break; \
  case 0xd0:prefix##_##d0();break; case 0xd1:prefix##_##d1();break; case 0xd2:prefix##_##d2();break; case 0xd3:prefix##_##d3();break; \
  case 0xd4:prefix##_##d4();break; case 0xd5:prefix##_##d5();break; case 0xd6:prefix##_##d6();break; case 0xd7:prefix##_##d7();break; \
  case 0xd8:prefix##_##d8();break; case 0xd9:prefix##_##d9();break; case 0xda:prefix##_##da();break; case 0xdb:prefix##_##db();break; \
  case 0xdc:prefix##_##dc();break; case 0xdd:prefix##_##dd();break; case 0xde:prefix##_##de();break; case 0xdf:prefix##_##df();break; \
  case 0xe0:prefix##_##e0();break; case 0xe1:prefix##_##e1();break; case 0xe2:prefix##_##e2();break; case 0xe3:prefix##_##e3();break; \
  case 0xe4:prefix##_##e4();break; case 0xe5:prefix##_##e5();break; case 0xe6:prefix##_##e6();break; case 0xe7:prefix##_##e7();break; \
  case 0xe8:prefix##_##e8();break; case 0xe9:prefix##_##e9();break; case 0xea:prefix##_##ea();break; case 0xeb:prefix##_##eb();break; \
  case 0xec:prefix##_##ec();break; case 0xed:prefix##_##ed();break; case 0xee:prefix##_##ee();break; case 0xef:prefix##_##ef();break; \
  case 0xf0:prefix##_##f0();break; case 0xf1:prefix##_##f1();break; case 0xf2:prefix##_##f2();break; case 0xf3:prefix##_##f3();break; \
  case 0xf4:prefix##_##f4();break; case 0xf5:prefix##_##f5();break; case 0xf6:prefix##_##f6();break; case 0xf7:prefix##_##f7();break; \
  case 0xf8:prefix##_##f8();break; case 0xf9:prefix##_##f9();break; case 0xfa:prefix##_##fa();break; case 0xfb:prefix##_##fb();break; \
  case 0xfc:prefix##_##fc();break; case 0xfd:prefix##_##fd();break; case 0xfe:prefix##_##fe();break; case 0xff:prefix##_##ff();break; \
  }                                                                                                                                   \
}
#else
#define EXEC_INLINE EXEC
#endif


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
  if( HALT )                                  \
  {                                           \
    HALT = 0;                                 \
    PC++;                                     \
  }                                           \
}

/***************************************************************
 * Input a byte from given I/O port
 ***************************************************************/
#define IN(port) ((UINT8)cpu_readport16(port))

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
#define OUT(port,value) cpu_writeport16(port,value)

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
#define RM(addr) (UINT8)cpu_readbyte(addr)

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
#define WM(addr,value) cpu_writemem16(addr,value)

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
  cpu_readbyte(pc);                               \
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
  cpu_readbyte(pc);                           \
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
  R = A;                            \
  R2 = A & 0x80;        /* keep bit 7 of R */      \
}

/***************************************************************
 * LD  A,R
 ***************************************************************/
#define LD_A_R {                        \
  A = (R & 0x7f) | R2;                    \
  F = (F & CF) | SZ[A] | ( IFF2 << 2 );            \
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
  F = (F & CF) | SZ[A] | ( IFF2 << 2 );  \
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
  F = (F & CF) | SZHV_inc[res];           \
  res;                                    \
})

/***************************************************************
 * DEC  r8
 ***************************************************************/
#define DEC(value) ({                     \
  UINT8 res = ((value) - 1);              \
  F = (F & CF) | SZHV_dec[res];           \
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
#define RLA {                          				\
  UINT8 res = (A << 1) | (F & CF);                  \
  UINT8 c = (A & 0x80) ? CF : 0;                    \
  F = (F & (SF | ZF | PF)) | c | (res & (YF | XF)); \
  A = res;                          				\
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
#define RRD {                                       \
  UINT8 n = RM(HL);                                 \
  WZ = HL+1;                                    \
  WM( HL, (n >> 4) | (A << 4) );                    \
  A = (A & 0xf0) | (n & 0x0f);                      \
  F = (F & CF) | SZP[A];                            \
}

/***************************************************************
 * RLD
 ***************************************************************/
#define RLD {                          \
  UINT8 n = RM(HL);                      \
  WZ = HL+1;                     \
  WM( HL, (n << 4) | (A & 0x0f) );              \
  A = (A & 0xf0) | (n >> 4);                  \
  F = (F & CF) | SZP[A];                    \
}

/***************************************************************
 * ADD  A,n
 ***************************************************************/
#define ADD(value)                        \
{                                \
  UINT32 ah = AFD & 0xff00;                  \
  UINT32 res = (UINT8)((ah >> 8) + value);          \
  F = SZHVC_add[ah | res];                  \
  A = res;                          \
}

/***************************************************************
 * ADC  A,n
 ***************************************************************/
#define ADC(value)                        \
{                                \
  UINT32 ah = AFD & 0xff00, c = AFD & 1;            \
  UINT32 res = (UINT8)((ah >> 8) + value + c);        \
  F = SZHVC_add[(c << 16) | ah | res];            \
  A = res;                          \
}

/***************************************************************
 * SUB  n
 ***************************************************************/
#define SUB(value)                        \
{                                \
  UINT32 ah = AFD & 0xff00;                  \
  UINT32 res = (UINT8)((ah >> 8) - value);          \
  F = SZHVC_sub[ah | res];                  \
  A = res;                          \
}

/***************************************************************
 * SBC  A,n
 ***************************************************************/
#define SBC(value)                        \
{                                \
  UINT32 ah = AFD & 0xff00, c = AFD & 1;            \
  UINT32 res = (UINT8)((ah >> 8) - value - c);        \
  F = SZHVC_sub[(c<<16) | ah | res];              \
  A = res;                          \
}

/***************************************************************
 * NEG
 ***************************************************************/
#define NEG {                          \
  UINT8 value = A;                      \
  A = 0;                            \
  SUB(value);                         \
}

/***************************************************************
 * DAA
 ***************************************************************/
#define DAA {                          \
	UINT8 a = A;											\
	if (F & NF) {											\
		if ((F&HF) | ((A&0xf)>9)) a-=6;				\
		if ((F&CF) | (A>0x99)) a-=0x60;				\
	}															\
	else {														\
		if ((F&HF) | ((A&0xf)>9)) a+=6;				\
		if ((F&CF) | (A>0x99)) a+=0x60;				\
	}															\
                                \
	F = (F&(CF|NF)) | (A>0x99) | ((A^a)&HF) | SZP[a]; \
	A = a;													\
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
#define EXSP(DR)                  \
{                                 \
  PAIR tmp = { { 0, 0, 0, 0 } };  \
  RM16( SPD, &tmp );              \
  WM16( SPD, &Z80.DR );                    \
  Z80.DR = tmp;                        \
  WZ = Z80.DR.d;                       \
}


/***************************************************************
 * ADD16
 ***************************************************************/
#define ADD16(DR,SR)                      \
{                                \
  UINT32 res = Z80.DR.d + Z80.SR.d;              \
  WZ = Z80.DR.d + 1;                       \
  F = (F & (SF | ZF | VF)) |                  \
    (((Z80.DR.d ^ res ^ Z80.SR.d) >> 8) & HF) |       \
    ((res >> 16) & CF) | ((res >> 8) & (YF | XF));       \
  Z80.DR.w.l = (UINT16)res;                  \
}

/***************************************************************
 * ADC  r16,r16
 ***************************************************************/
#define ADC16(Reg)                        \
{                                \
  UINT32 res = HLD + Z80.Reg.d + (F & CF);          \
  WZ = HL + 1;                       \
  F = (((HLD ^ res ^ Z80.Reg.d) >> 8) & HF) |          \
    ((res >> 16) & CF) |                  \
    ((res >> 8) & (SF | YF | XF)) |              \
    ((res & 0xffff) ? 0 : ZF) |               \
    (((Z80.Reg.d ^ HLD ^ 0x8000) & (Z80.Reg.d ^ res) & 0x8000) >> 13); \
  HL = (UINT16)res;                      \
}

/***************************************************************
 * SBC  r16,r16
 ***************************************************************/
#define SBC16(Reg)                        \
{                                \
  UINT32 res = HLD - Z80.Reg.d - (F & CF);          \
  WZ = HL + 1;                       \
  F = (((HLD ^ res ^ Z80.Reg.d) >> 8) & HF) | NF |      \
    ((res >> 16) & CF) |                  \
    ((res >> 8) & (SF | YF | XF)) |             \
    ((res & 0xffff) ? 0 : ZF) |               \
    (((Z80.Reg.d ^ HLD) & (HLD ^ res) &0x8000) >> 13);    \
  HL = (UINT16)res;                      \
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
  res = (res >> 1);                              \
  F = SZP[res] | c;                               \
  res;                                            \
})

/***************************************************************
 * BIT  bit,r8
 ***************************************************************/
#undef BIT
#define BIT(bit,reg)                      \
  F = (F & CF) | HF | (SZ_BIT[reg & (1<<bit)] & ~(YF|XF)) | (reg & (YF|XF)); \

/***************************************************************
 * BIT  bit,(HL)
 ***************************************************************/
#define BIT_HL(bit, reg)                    \
  F = (F & CF) | HF | (SZ_BIT[reg & (1<<bit)] & ~(YF|XF)) | (WZ_H & (YF|XF)); \

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
#define BIT_XY(bit,reg)                     \
  F = (F & CF) | HF | (SZ_BIT[reg & (1<<bit)] & ~(YF|XF)) | ((EA>>8) & (YF|XF))

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
#define LDI {                          \
  UINT8 io = RM(HL);                      \
  WM( DE, io );                        \
  F &= SF | ZF | CF;                      \
  if( (A + io) & 0x02 ) F |= YF; /* bit 1 -> flag 5 */    \
  if( (A + io) & 0x08 ) F |= XF; /* bit 3 -> flag 3 */    \
  HL++; DE++; BC--;                      \
  if( BC ) F |= VF;                      \
}

/***************************************************************
 * CPI
 ***************************************************************/
#define CPI {                          \
  UINT8 val = RM(HL);                      \
  UINT8 res = A - val;                    \
  WZ++;  \
  HL++; BC--;                          \
  F = (F & CF) | (SZ[res]&~(YF|XF)) | ((A^val^res)&HF) | NF;  \
  if( F & HF ) res -= 1;                    \
  if( res & 0x02 ) F |= YF; /* bit 1 -> flag 5 */        \
  if( res & 0x08 ) F |= XF; /* bit 3 -> flag 3 */        \
  if( BC ) F |= VF;                      \
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
  F = SZ[B];                          \
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
  F = SZ[B];                          \
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
  F = (F & CF) | (SZ[res]&~(YF|XF)) | ((A^val^res)&HF) | NF;  \
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
  F = SZ[B];                          \
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
  F = SZ[B];                          \
  t = (unsigned)L + (unsigned)io;                \
  if( io & SF ) F |= NF;                    \
  if( t & 0x100 ) F |= HF | CF;                \
  F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;            \
}

/***************************************************************
 * LDIR
 ***************************************************************/
#define LDIR                          \
  LDI;                            \
  if( BC )                          \
  {                              \
    PC -= 2;                        \
    WZ = PC + 1;  \
    CC(ex,0xb0);                      \
  }

/***************************************************************
 * CPIR
 ***************************************************************/
#define CPIR                          \
  CPI;                            \
  if( BC && !(F & ZF) )                    \
  {                              \
    PC -= 2;                        \
   WZ = PC + 1;  \
    CC(ex,0xb1);                      \
  }

/***************************************************************
 * INIR
 ***************************************************************/
#define INIR                          \
  INI;                            \
  if( B )                            \
  {                              \
    PC -= 2;                        \
    CC(ex,0xb2);                      \
  }

/***************************************************************
 * OTIR
 ***************************************************************/
#define OTIR                          \
  OUTI;                            \
  if( B )                            \
  {                              \
    PC -= 2;                        \
    CC(ex,0xb3);                      \
  }

/***************************************************************
 * LDDR
 ***************************************************************/
#define LDDR                          \
  LDD;                            \
  if( BC )                          \
  {                              \
    PC -= 2;                        \
    WZ = PC + 1;                  \
    CC(ex,0xb8);                      \
  }

/***************************************************************
 * CPDR
 ***************************************************************/
#define CPDR                          \
  CPD;                            \
  if( BC && !(F & ZF) )                    \
  {                              \
    PC -= 2;                        \
   WZ = PC + 1;  \
    CC(ex,0xb9);                      \
  }

/***************************************************************
 * INDR
 ***************************************************************/
#define INDR                          \
  IND;                            \
  if( B )                            \
  {                              \
    PC -= 2;                        \
    CC(ex,0xba);                      \
  }

/***************************************************************
 * OTDR
 ***************************************************************/
#define OTDR                          \
  OUTD;                            \
  if( B )                            \
  {                              \
    PC -= 2;                        \
    CC(ex,0xbb);                      \
  }

/***************************************************************
 * EI
 ***************************************************************/
#define EI {                          \
  IFF1 = IFF2 = 1;                      \
  Z80.after_ei = TRUE;                    \
}

/**********************************************************
 * opcodes with CB prefix
 * rotate, shift and bit operations
 **********************************************************/
OP(cb,00) { B = RLC(B);                      } /* RLC  B           */
OP(cb,01) { C = RLC(C);                      } /* RLC  C           */
OP(cb,02) { D = RLC(D);                      } /* RLC  D           */
OP(cb,03) { E = RLC(E);                      } /* RLC  E           */
OP(cb,04) { H = RLC(H);                      } /* RLC  H           */
OP(cb,05) { L = RLC(L);                      } /* RLC  L           */
OP(cb,06) { WM( HL, RLC(RM(HL)) );           } /* RLC  (HL)        */
OP(cb,07) { A = RLC(A);                      } /* RLC  A           */

OP(cb,08) { B = RRC(B);                      } /* RRC  B           */
OP(cb,09) { C = RRC(C);                      } /* RRC  C           */
OP(cb,0a) { D = RRC(D);                      } /* RRC  D           */
OP(cb,0b) { E = RRC(E);                      } /* RRC  E           */
OP(cb,0c) { H = RRC(H);                      } /* RRC  H           */
OP(cb,0d) { L = RRC(L);                      } /* RRC  L           */
OP(cb,0e) { WM( HL, RRC(RM(HL)) );           } /* RRC  (HL)        */
OP(cb,0f) { A = RRC(A);                      } /* RRC  A           */

OP(cb,10) { B = RL(B);                       } /* RL   B           */
OP(cb,11) { C = RL(C);                       } /* RL   C           */
OP(cb,12) { D = RL(D);                       } /* RL   D           */
OP(cb,13) { E = RL(E);                       } /* RL   E           */
OP(cb,14) { H = RL(H);                       } /* RL   H           */
OP(cb,15) { L = RL(L);                       } /* RL   L           */
OP(cb,16) { WM( HL, RL(RM(HL)) );            } /* RL   (HL)        */
OP(cb,17) { A = RL(A);                       } /* RL   A           */

OP(cb,18) { B = RR(B);                       } /* RR   B           */
OP(cb,19) { C = RR(C);                       } /* RR   C           */
OP(cb,1a) { D = RR(D);                       } /* RR   D           */
OP(cb,1b) { E = RR(E);                       } /* RR   E           */
OP(cb,1c) { H = RR(H);                       } /* RR   H           */
OP(cb,1d) { L = RR(L);                       } /* RR   L           */
OP(cb,1e) { WM( HL, RR(RM(HL)) );            } /* RR   (HL)        */
OP(cb,1f) { A = RR(A);                       } /* RR   A           */

OP(cb,20) { B = SLA(B);                      } /* SLA  B           */
OP(cb,21) { C = SLA(C);                      } /* SLA  C           */
OP(cb,22) { D = SLA(D);                      } /* SLA  D           */
OP(cb,23) { E = SLA(E);                      } /* SLA  E           */
OP(cb,24) { H = SLA(H);                      } /* SLA  H           */
OP(cb,25) { L = SLA(L);                      } /* SLA  L           */
OP(cb,26) { WM( HL, SLA(RM(HL)) );           } /* SLA  (HL)        */
OP(cb,27) { A = SLA(A);                      } /* SLA  A           */

OP(cb,28) { B = SRA(B);                      } /* SRA  B           */
OP(cb,29) { C = SRA(C);                      } /* SRA  C           */
OP(cb,2a) { D = SRA(D);                      } /* SRA  D           */
OP(cb,2b) { E = SRA(E);                      } /* SRA  E           */
OP(cb,2c) { H = SRA(H);                      } /* SRA  H           */
OP(cb,2d) { L = SRA(L);                      } /* SRA  L           */
OP(cb,2e) { WM( HL, SRA(RM(HL)) );           } /* SRA  (HL)        */
OP(cb,2f) { A = SRA(A);                      } /* SRA  A           */

OP(cb,30) { B = SLL(B);                      } /* SLL  B           */
OP(cb,31) { C = SLL(C);                      } /* SLL  C           */
OP(cb,32) { D = SLL(D);                      } /* SLL  D           */
OP(cb,33) { E = SLL(E);                      } /* SLL  E           */
OP(cb,34) { H = SLL(H);                      } /* SLL  H           */
OP(cb,35) { L = SLL(L);                      } /* SLL  L           */
OP(cb,36) { WM( HL, SLL(RM(HL)) );           } /* SLL  (HL)        */
OP(cb,37) { A = SLL(A);                      } /* SLL  A           */

OP(cb,38) { B = SRL(B);                      } /* SRL  B           */
OP(cb,39) { C = SRL(C);                      } /* SRL  C           */
OP(cb,3a) { D = SRL(D);                      } /* SRL  D           */
OP(cb,3b) { E = SRL(E);                      } /* SRL  E           */
OP(cb,3c) { H = SRL(H);                      } /* SRL  H           */
OP(cb,3d) { L = SRL(L);                      } /* SRL  L           */
OP(cb,3e) { WM( HL, SRL(RM(HL)) );           } /* SRL  (HL)        */
OP(cb,3f) { A = SRL(A);                      } /* SRL  A           */

OP(cb,40) { BIT(0,B);                        } /* BIT  0,B         */
OP(cb,41) { BIT(0,C);                        } /* BIT  0,C         */
OP(cb,42) { BIT(0,D);                        } /* BIT  0,D         */
OP(cb,43) { BIT(0,E);                        } /* BIT  0,E         */
OP(cb,44) { BIT(0,H);                        } /* BIT  0,H         */
OP(cb,45) { BIT(0,L);                        } /* BIT  0,L         */
OP(cb,46) { BIT_HL(0,RM(HL));                } /* BIT  0,(HL)      */
OP(cb,47) { BIT(0,A);                        } /* BIT  0,A         */

OP(cb,48) { BIT(1,B);                        } /* BIT  1,B         */
OP(cb,49) { BIT(1,C);                        } /* BIT  1,C         */
OP(cb,4a) { BIT(1,D);                        } /* BIT  1,D         */
OP(cb,4b) { BIT(1,E);                        } /* BIT  1,E         */
OP(cb,4c) { BIT(1,H);                        } /* BIT  1,H         */
OP(cb,4d) { BIT(1,L);                        } /* BIT  1,L         */
OP(cb,4e) { BIT_HL(1,RM(HL));                } /* BIT  1,(HL)      */
OP(cb,4f) { BIT(1,A);                        } /* BIT  1,A         */

OP(cb,50) { BIT(2,B);                        } /* BIT  2,B         */
OP(cb,51) { BIT(2,C);                        } /* BIT  2,C         */
OP(cb,52) { BIT(2,D);                        } /* BIT  2,D         */
OP(cb,53) { BIT(2,E);                        } /* BIT  2,E         */
OP(cb,54) { BIT(2,H);                        } /* BIT  2,H         */
OP(cb,55) { BIT(2,L);                        } /* BIT  2,L         */
OP(cb,56) { BIT_HL(2,RM(HL));                } /* BIT  2,(HL)      */
OP(cb,57) { BIT(2,A);                        } /* BIT  2,A         */

OP(cb,58) { BIT(3,B);                        } /* BIT  3,B         */
OP(cb,59) { BIT(3,C);                        } /* BIT  3,C         */
OP(cb,5a) { BIT(3,D);                        } /* BIT  3,D         */
OP(cb,5b) { BIT(3,E);                        } /* BIT  3,E         */
OP(cb,5c) { BIT(3,H);                        } /* BIT  3,H         */
OP(cb,5d) { BIT(3,L);                        } /* BIT  3,L         */
OP(cb,5e) { BIT_HL(3,RM(HL));                } /* BIT  3,(HL)      */
OP(cb,5f) { BIT(3,A);                        } /* BIT  3,A         */

OP(cb,60) { BIT(4,B);                        } /* BIT  4,B         */
OP(cb,61) { BIT(4,C);                        } /* BIT  4,C         */
OP(cb,62) { BIT(4,D);                        } /* BIT  4,D         */
OP(cb,63) { BIT(4,E);                        } /* BIT  4,E         */
OP(cb,64) { BIT(4,H);                        } /* BIT  4,H         */
OP(cb,65) { BIT(4,L);                        } /* BIT  4,L         */
OP(cb,66) { BIT_HL(4,RM(HL));                } /* BIT  4,(HL)      */
OP(cb,67) { BIT(4,A);                        } /* BIT  4,A         */

OP(cb,68) { BIT(5,B);                        } /* BIT  5,B         */
OP(cb,69) { BIT(5,C);                        } /* BIT  5,C         */
OP(cb,6a) { BIT(5,D);                        } /* BIT  5,D         */
OP(cb,6b) { BIT(5,E);                        } /* BIT  5,E         */
OP(cb,6c) { BIT(5,H);                        } /* BIT  5,H         */
OP(cb,6d) { BIT(5,L);                        } /* BIT  5,L         */
OP(cb,6e) { BIT_HL(5,RM(HL));                } /* BIT  5,(HL)      */
OP(cb,6f) { BIT(5,A);                        } /* BIT  5,A         */

OP(cb,70) { BIT(6,B);                        } /* BIT  6,B         */
OP(cb,71) { BIT(6,C);                        } /* BIT  6,C         */
OP(cb,72) { BIT(6,D);                        } /* BIT  6,D         */
OP(cb,73) { BIT(6,E);                        } /* BIT  6,E         */
OP(cb,74) { BIT(6,H);                        } /* BIT  6,H         */
OP(cb,75) { BIT(6,L);                        } /* BIT  6,L         */
OP(cb,76) { BIT_HL(6,RM(HL));                } /* BIT  6,(HL)      */
OP(cb,77) { BIT(6,A);                        } /* BIT  6,A         */

OP(cb,78) { BIT(7,B);                        } /* BIT  7,B         */
OP(cb,79) { BIT(7,C);                        } /* BIT  7,C         */
OP(cb,7a) { BIT(7,D);                        } /* BIT  7,D         */
OP(cb,7b) { BIT(7,E);                        } /* BIT  7,E         */
OP(cb,7c) { BIT(7,H);                        } /* BIT  7,H         */
OP(cb,7d) { BIT(7,L);                        } /* BIT  7,L         */
OP(cb,7e) { BIT_HL(7,RM(HL));                } /* BIT  7,(HL)      */
OP(cb,7f) { BIT(7,A);                        } /* BIT  7,A         */

OP(cb,80) { B = RES(0,B);                    } /* RES  0,B         */
OP(cb,81) { C = RES(0,C);                    } /* RES  0,C         */
OP(cb,82) { D = RES(0,D);                    } /* RES  0,D         */
OP(cb,83) { E = RES(0,E);                    } /* RES  0,E         */
OP(cb,84) { H = RES(0,H);                    } /* RES  0,H         */
OP(cb,85) { L = RES(0,L);                    } /* RES  0,L         */
OP(cb,86) { WM( HL, RES(0,RM(HL)) );         } /* RES  0,(HL)      */
OP(cb,87) { A = RES(0,A);                    } /* RES  0,A         */

OP(cb,88) { B = RES(1,B);                    } /* RES  1,B         */
OP(cb,89) { C = RES(1,C);                    } /* RES  1,C         */
OP(cb,8a) { D = RES(1,D);                    } /* RES  1,D         */
OP(cb,8b) { E = RES(1,E);                    } /* RES  1,E         */
OP(cb,8c) { H = RES(1,H);                    } /* RES  1,H         */
OP(cb,8d) { L = RES(1,L);                    } /* RES  1,L         */
OP(cb,8e) { WM( HL, RES(1,RM(HL)) );         } /* RES  1,(HL)      */
OP(cb,8f) { A = RES(1,A);                    } /* RES  1,A         */

OP(cb,90) { B = RES(2,B);                    } /* RES  2,B         */
OP(cb,91) { C = RES(2,C);                    } /* RES  2,C         */
OP(cb,92) { D = RES(2,D);                    } /* RES  2,D         */
OP(cb,93) { E = RES(2,E);                    } /* RES  2,E         */
OP(cb,94) { H = RES(2,H);                    } /* RES  2,H         */
OP(cb,95) { L = RES(2,L);                    } /* RES  2,L         */
OP(cb,96) { WM( HL, RES(2,RM(HL)) );         } /* RES  2,(HL)      */
OP(cb,97) { A = RES(2,A);                    } /* RES  2,A         */

OP(cb,98) { B = RES(3,B);                    } /* RES  3,B         */
OP(cb,99) { C = RES(3,C);                    } /* RES  3,C         */
OP(cb,9a) { D = RES(3,D);                    } /* RES  3,D         */
OP(cb,9b) { E = RES(3,E);                    } /* RES  3,E         */
OP(cb,9c) { H = RES(3,H);                    } /* RES  3,H         */
OP(cb,9d) { L = RES(3,L);                    } /* RES  3,L         */
OP(cb,9e) { WM( HL, RES(3,RM(HL)) );         } /* RES  3,(HL)      */
OP(cb,9f) { A = RES(3,A);                    } /* RES  3,A         */

OP(cb,a0) { B = RES(4,B);                    } /* RES  4,B         */
OP(cb,a1) { C = RES(4,C);                    } /* RES  4,C         */
OP(cb,a2) { D = RES(4,D);                    } /* RES  4,D         */
OP(cb,a3) { E = RES(4,E);                    } /* RES  4,E         */
OP(cb,a4) { H = RES(4,H);                    } /* RES  4,H         */
OP(cb,a5) { L = RES(4,L);                    } /* RES  4,L         */
OP(cb,a6) { WM( HL, RES(4,RM(HL)) );         } /* RES  4,(HL)      */
OP(cb,a7) { A = RES(4,A);                    } /* RES  4,A         */

OP(cb,a8) { B = RES(5,B);                    } /* RES  5,B         */
OP(cb,a9) { C = RES(5,C);                    } /* RES  5,C         */
OP(cb,aa) { D = RES(5,D);                    } /* RES  5,D         */
OP(cb,ab) { E = RES(5,E);                    } /* RES  5,E         */
OP(cb,ac) { H = RES(5,H);                    } /* RES  5,H         */
OP(cb,ad) { L = RES(5,L);                    } /* RES  5,L         */
OP(cb,ae) { WM( HL, RES(5,RM(HL)) );         } /* RES  5,(HL)      */
OP(cb,af) { A = RES(5,A);                    } /* RES  5,A         */

OP(cb,b0) { B = RES(6,B);                    } /* RES  6,B         */
OP(cb,b1) { C = RES(6,C);                    } /* RES  6,C         */
OP(cb,b2) { D = RES(6,D);                    } /* RES  6,D         */
OP(cb,b3) { E = RES(6,E);                    } /* RES  6,E         */
OP(cb,b4) { H = RES(6,H);                    } /* RES  6,H         */
OP(cb,b5) { L = RES(6,L);                    } /* RES  6,L         */
OP(cb,b6) { WM( HL, RES(6,RM(HL)) );         } /* RES  6,(HL)      */
OP(cb,b7) { A = RES(6,A);                    } /* RES  6,A         */

OP(cb,b8) { B = RES(7,B);                    } /* RES  7,B         */
OP(cb,b9) { C = RES(7,C);                    } /* RES  7,C         */
OP(cb,ba) { D = RES(7,D);                    } /* RES  7,D         */
OP(cb,bb) { E = RES(7,E);                    } /* RES  7,E         */
OP(cb,bc) { H = RES(7,H);                    } /* RES  7,H         */
OP(cb,bd) { L = RES(7,L);                    } /* RES  7,L         */
OP(cb,be) { WM( HL, RES(7,RM(HL)) );         } /* RES  7,(HL)      */
OP(cb,bf) { A = RES(7,A);                    } /* RES  7,A         */

OP(cb,c0) { B = SET(0,B);                    } /* SET  0,B         */
OP(cb,c1) { C = SET(0,C);                    } /* SET  0,C         */
OP(cb,c2) { D = SET(0,D);                    } /* SET  0,D         */
OP(cb,c3) { E = SET(0,E);                    } /* SET  0,E         */
OP(cb,c4) { H = SET(0,H);                    } /* SET  0,H         */
OP(cb,c5) { L = SET(0,L);                    } /* SET  0,L         */
OP(cb,c6) { WM( HL, SET(0,RM(HL)) );         } /* SET  0,(HL)      */
OP(cb,c7) { A = SET(0,A);                    } /* SET  0,A         */

OP(cb,c8) { B = SET(1,B);                    } /* SET  1,B         */
OP(cb,c9) { C = SET(1,C);                    } /* SET  1,C         */
OP(cb,ca) { D = SET(1,D);                    } /* SET  1,D         */
OP(cb,cb) { E = SET(1,E);                    } /* SET  1,E         */
OP(cb,cc) { H = SET(1,H);                    } /* SET  1,H         */
OP(cb,cd) { L = SET(1,L);                    } /* SET  1,L         */
OP(cb,ce) { WM( HL, SET(1,RM(HL)) );         } /* SET  1,(HL)      */
OP(cb,cf) { A = SET(1,A);                    } /* SET  1,A         */

OP(cb,d0) { B = SET(2,B);                    } /* SET  2,B         */
OP(cb,d1) { C = SET(2,C);                    } /* SET  2,C         */
OP(cb,d2) { D = SET(2,D);                    } /* SET  2,D         */
OP(cb,d3) { E = SET(2,E);                    } /* SET  2,E         */
OP(cb,d4) { H = SET(2,H);                    } /* SET  2,H         */
OP(cb,d5) { L = SET(2,L);                    } /* SET  2,L         */
OP(cb,d6) { WM( HL, SET(2,RM(HL)) );         } /* SET  2,(HL)      */
OP(cb,d7) { A = SET(2,A);                    } /* SET  2,A         */

OP(cb,d8) { B = SET(3,B);                    } /* SET  3,B         */
OP(cb,d9) { C = SET(3,C);                    } /* SET  3,C         */
OP(cb,da) { D = SET(3,D);                    } /* SET  3,D         */
OP(cb,db) { E = SET(3,E);                    } /* SET  3,E         */
OP(cb,dc) { H = SET(3,H);                    } /* SET  3,H         */
OP(cb,dd) { L = SET(3,L);                    } /* SET  3,L         */
OP(cb,de) { WM( HL, SET(3,RM(HL)) );         } /* SET  3,(HL)      */
OP(cb,df) { A = SET(3,A);                    } /* SET  3,A         */

OP(cb,e0) { B = SET(4,B);                    } /* SET  4,B         */
OP(cb,e1) { C = SET(4,C);                    } /* SET  4,C         */
OP(cb,e2) { D = SET(4,D);                    } /* SET  4,D         */
OP(cb,e3) { E = SET(4,E);                    } /* SET  4,E         */
OP(cb,e4) { H = SET(4,H);                    } /* SET  4,H         */
OP(cb,e5) { L = SET(4,L);                    } /* SET  4,L         */
OP(cb,e6) { WM( HL, SET(4,RM(HL)) );         } /* SET  4,(HL)      */
OP(cb,e7) { A = SET(4,A);                    } /* SET  4,A         */

OP(cb,e8) { B = SET(5,B);                    } /* SET  5,B         */
OP(cb,e9) { C = SET(5,C);                    } /* SET  5,C         */
OP(cb,ea) { D = SET(5,D);                    } /* SET  5,D         */
OP(cb,eb) { E = SET(5,E);                    } /* SET  5,E         */
OP(cb,ec) { H = SET(5,H);                    } /* SET  5,H         */
OP(cb,ed) { L = SET(5,L);                    } /* SET  5,L         */
OP(cb,ee) { WM( HL, SET(5,RM(HL)) );         } /* SET  5,(HL)      */
OP(cb,ef) { A = SET(5,A);                    } /* SET  5,A         */

OP(cb,f0) { B = SET(6,B);                    } /* SET  6,B         */
OP(cb,f1) { C = SET(6,C);                    } /* SET  6,C         */
OP(cb,f2) { D = SET(6,D);                    } /* SET  6,D         */
OP(cb,f3) { E = SET(6,E);                    } /* SET  6,E         */
OP(cb,f4) { H = SET(6,H);                    } /* SET  6,H         */
OP(cb,f5) { L = SET(6,L);                    } /* SET  6,L         */
OP(cb,f6) { WM( HL, SET(6,RM(HL)) );         } /* SET  6,(HL)      */
OP(cb,f7) { A = SET(6,A);                    } /* SET  6,A         */

OP(cb,f8) { B = SET(7,B);                    } /* SET  7,B         */
OP(cb,f9) { C = SET(7,C);                    } /* SET  7,C         */
OP(cb,fa) { D = SET(7,D);                    } /* SET  7,D         */
OP(cb,fb) { E = SET(7,E);                    } /* SET  7,E         */
OP(cb,fc) { H = SET(7,H);                    } /* SET  7,H         */
OP(cb,fd) { L = SET(7,L);                    } /* SET  7,L         */
OP(cb,fe) { WM( HL, SET(7,RM(HL)) );         } /* SET  7,(HL)      */
OP(cb,ff) { A = SET(7,A);                    } /* SET  7,A         */


/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit operations with (IX+o)
**********************************************************/
OP(xycb,00) { B = RLC( RM(EA) ); WM( EA,B );            } /* RLC  B=(XY+o)    */
OP(xycb,01) { C = RLC( RM(EA) ); WM( EA,C );            } /* RLC  C=(XY+o)    */
OP(xycb,02) { D = RLC( RM(EA) ); WM( EA,D );            } /* RLC  D=(XY+o)    */
OP(xycb,03) { E = RLC( RM(EA) ); WM( EA,E );            } /* RLC  E=(XY+o)    */
OP(xycb,04) { H = RLC( RM(EA) ); WM( EA,H );            } /* RLC  H=(XY+o)    */
OP(xycb,05) { L = RLC( RM(EA) ); WM( EA,L );            } /* RLC  L=(XY+o)    */
OP(xycb,06) { WM( EA, RLC( RM(EA) ) );                  } /* RLC  (XY+o)      */
OP(xycb,07) { A = RLC( RM(EA) ); WM( EA,A );            } /* RLC  A=(XY+o)    */

OP(xycb,08) { B = RRC( RM(EA) ); WM( EA,B );            } /* RRC  B=(XY+o)    */
OP(xycb,09) { C = RRC( RM(EA) ); WM( EA,C );            } /* RRC  C=(XY+o)    */
OP(xycb,0a) { D = RRC( RM(EA) ); WM( EA,D );            } /* RRC  D=(XY+o)    */
OP(xycb,0b) { E = RRC( RM(EA) ); WM( EA,E );            } /* RRC  E=(XY+o)    */
OP(xycb,0c) { H = RRC( RM(EA) ); WM( EA,H );            } /* RRC  H=(XY+o)    */
OP(xycb,0d) { L = RRC( RM(EA) ); WM( EA,L );            } /* RRC  L=(XY+o)    */
OP(xycb,0e) { WM( EA,RRC( RM(EA) ) );                   } /* RRC  (XY+o)      */
OP(xycb,0f) { A = RRC( RM(EA) ); WM( EA,A );            } /* RRC  A=(XY+o)    */

OP(xycb,10) { B = RL( RM(EA) ); WM( EA,B );             } /* RL   B=(XY+o)    */
OP(xycb,11) { C = RL( RM(EA) ); WM( EA,C );             } /* RL   C=(XY+o)    */
OP(xycb,12) { D = RL( RM(EA) ); WM( EA,D );             } /* RL   D=(XY+o)    */
OP(xycb,13) { E = RL( RM(EA) ); WM( EA,E );             } /* RL   E=(XY+o)    */
OP(xycb,14) { H = RL( RM(EA) ); WM( EA,H );             } /* RL   H=(XY+o)    */
OP(xycb,15) { L = RL( RM(EA) ); WM( EA,L );             } /* RL   L=(XY+o)    */
OP(xycb,16) { WM( EA,RL( RM(EA) ) );                    } /* RL   (XY+o)      */
OP(xycb,17) { A = RL( RM(EA) ); WM( EA,A );             } /* RL   A=(XY+o)    */

OP(xycb,18) { B = RR( RM(EA) ); WM( EA,B );             } /* RR   B=(XY+o)    */
OP(xycb,19) { C = RR( RM(EA) ); WM( EA,C );             } /* RR   C=(XY+o)    */
OP(xycb,1a) { D = RR( RM(EA) ); WM( EA,D );             } /* RR   D=(XY+o)    */
OP(xycb,1b) { E = RR( RM(EA) ); WM( EA,E );             } /* RR   E=(XY+o)    */
OP(xycb,1c) { H = RR( RM(EA) ); WM( EA,H );             } /* RR   H=(XY+o)    */
OP(xycb,1d) { L = RR( RM(EA) ); WM( EA,L );             } /* RR   L=(XY+o)    */
OP(xycb,1e) { WM( EA,RR( RM(EA) ) );                    } /* RR   (XY+o)      */
OP(xycb,1f) { A = RR( RM(EA) ); WM( EA,A );             } /* RR   A=(XY+o)    */

OP(xycb,20) { B = SLA( RM(EA) ); WM( EA,B );            } /* SLA  B=(XY+o)    */
OP(xycb,21) { C = SLA( RM(EA) ); WM( EA,C );            } /* SLA  C=(XY+o)    */
OP(xycb,22) { D = SLA( RM(EA) ); WM( EA,D );            } /* SLA  D=(XY+o)    */
OP(xycb,23) { E = SLA( RM(EA) ); WM( EA,E );            } /* SLA  E=(XY+o)    */
OP(xycb,24) { H = SLA( RM(EA) ); WM( EA,H );            } /* SLA  H=(XY+o)    */
OP(xycb,25) { L = SLA( RM(EA) ); WM( EA,L );            } /* SLA  L=(XY+o)    */
OP(xycb,26) { WM( EA,SLA( RM(EA) ) );                   } /* SLA  (XY+o)      */
OP(xycb,27) { A = SLA( RM(EA) ); WM( EA,A );            } /* SLA  A=(XY+o)    */

OP(xycb,28) { B = SRA( RM(EA) ); WM( EA,B );            } /* SRA  B=(XY+o)    */
OP(xycb,29) { C = SRA( RM(EA) ); WM( EA,C );            } /* SRA  C=(XY+o)    */
OP(xycb,2a) { D = SRA( RM(EA) ); WM( EA,D );            } /* SRA  D=(XY+o)    */
OP(xycb,2b) { E = SRA( RM(EA) ); WM( EA,E );            } /* SRA  E=(XY+o)    */
OP(xycb,2c) { H = SRA( RM(EA) ); WM( EA,H );            } /* SRA  H=(XY+o)    */
OP(xycb,2d) { L = SRA( RM(EA) ); WM( EA,L );            } /* SRA  L=(XY+o)    */
OP(xycb,2e) { WM( EA,SRA( RM(EA) ) );                   } /* SRA  (XY+o)      */
OP(xycb,2f) { A = SRA( RM(EA) ); WM( EA,A );            } /* SRA  A=(XY+o)    */

OP(xycb,30) { B = SLL( RM(EA) ); WM( EA,B );            } /* SLL  B=(XY+o)    */
OP(xycb,31) { C = SLL( RM(EA) ); WM( EA,C );            } /* SLL  C=(XY+o)    */
OP(xycb,32) { D = SLL( RM(EA) ); WM( EA,D );            } /* SLL  D=(XY+o)    */
OP(xycb,33) { E = SLL( RM(EA) ); WM( EA,E );            } /* SLL  E=(XY+o)    */
OP(xycb,34) { H = SLL( RM(EA) ); WM( EA,H );            } /* SLL  H=(XY+o)    */
OP(xycb,35) { L = SLL( RM(EA) ); WM( EA,L );            } /* SLL  L=(XY+o)    */
OP(xycb,36) { WM( EA,SLL( RM(EA) ) );                   } /* SLL  (XY+o)      */
OP(xycb,37) { A = SLL( RM(EA) ); WM( EA,A );            } /* SLL  A=(XY+o)    */

OP(xycb,38) { B = SRL( RM(EA) ); WM( EA,B );            } /* SRL  B=(XY+o)    */
OP(xycb,39) { C = SRL( RM(EA) ); WM( EA,C );            } /* SRL  C=(XY+o)    */
OP(xycb,3a) { D = SRL( RM(EA) ); WM( EA,D );            } /* SRL  D=(XY+o)    */
OP(xycb,3b) { E = SRL( RM(EA) ); WM( EA,E );            } /* SRL  E=(XY+o)    */
OP(xycb,3c) { H = SRL( RM(EA) ); WM( EA,H );            } /* SRL  H=(XY+o)    */
OP(xycb,3d) { L = SRL( RM(EA) ); WM( EA,L );            } /* SRL  L=(XY+o)    */
OP(xycb,3e) { WM( EA,SRL( RM(EA) ) );                   } /* SRL  (XY+o)      */
OP(xycb,3f) { A = SRL( RM(EA) ); WM( EA,A );            } /* SRL  A=(XY+o)    */

OP(xycb,40) { xycb_46();                                } /* BIT  0,(XY+o)    */
OP(xycb,41) { xycb_46();                                } /* BIT  0,(XY+o)    */
OP(xycb,42) { xycb_46();                                } /* BIT  0,(XY+o)    */
OP(xycb,43) { xycb_46();                                } /* BIT  0,(XY+o)    */
OP(xycb,44) { xycb_46();                                } /* BIT  0,(XY+o)    */
OP(xycb,45) { xycb_46();                                } /* BIT  0,(XY+o)    */
OP(xycb,46) { BIT_XY(0,RM(EA));                         } /* BIT  0,(XY+o)    */
OP(xycb,47) { xycb_46();                                } /* BIT  0,(XY+o)    */

OP(xycb,48) { xycb_4e();                                } /* BIT  1,(XY+o)    */
OP(xycb,49) { xycb_4e();                                } /* BIT  1,(XY+o)    */
OP(xycb,4a) { xycb_4e();                                } /* BIT  1,(XY+o)    */
OP(xycb,4b) { xycb_4e();                                } /* BIT  1,(XY+o)    */
OP(xycb,4c) { xycb_4e();                                } /* BIT  1,(XY+o)    */
OP(xycb,4d) { xycb_4e();                                } /* BIT  1,(XY+o)    */
OP(xycb,4e) { BIT_XY(1,RM(EA));                         } /* BIT  1,(XY+o)    */
OP(xycb,4f) { xycb_4e();                                } /* BIT  1,(XY+o)    */

OP(xycb,50) { xycb_56();                                } /* BIT  2,(XY+o)    */
OP(xycb,51) { xycb_56();                                } /* BIT  2,(XY+o)    */
OP(xycb,52) { xycb_56();                                } /* BIT  2,(XY+o)    */
OP(xycb,53) { xycb_56();                                } /* BIT  2,(XY+o)    */
OP(xycb,54) { xycb_56();                                } /* BIT  2,(XY+o)    */
OP(xycb,55) { xycb_56();                                } /* BIT  2,(XY+o)    */
OP(xycb,56) { BIT_XY(2,RM(EA));                         } /* BIT  2,(XY+o)    */
OP(xycb,57) { xycb_56();                                } /* BIT  2,(XY+o)    */

OP(xycb,58) { xycb_5e();                                } /* BIT  3,(XY+o)    */
OP(xycb,59) { xycb_5e();                                } /* BIT  3,(XY+o)    */
OP(xycb,5a) { xycb_5e();                                } /* BIT  3,(XY+o)    */
OP(xycb,5b) { xycb_5e();                                } /* BIT  3,(XY+o)    */
OP(xycb,5c) { xycb_5e();                                } /* BIT  3,(XY+o)    */
OP(xycb,5d) { xycb_5e();                                } /* BIT  3,(XY+o)    */
OP(xycb,5e) { BIT_XY(3,RM(EA));                         } /* BIT  3,(XY+o)    */
OP(xycb,5f) { xycb_5e();                                } /* BIT  3,(XY+o)    */

OP(xycb,60) { xycb_66();                                } /* BIT  4,(XY+o)    */
OP(xycb,61) { xycb_66();                                } /* BIT  4,(XY+o)    */
OP(xycb,62) { xycb_66();                                } /* BIT  4,(XY+o)    */
OP(xycb,63) { xycb_66();                                } /* BIT  4,(XY+o)    */
OP(xycb,64) { xycb_66();                                } /* BIT  4,(XY+o)    */
OP(xycb,65) { xycb_66();                                } /* BIT  4,(XY+o)    */
OP(xycb,66) { BIT_XY(4,RM(EA));                         } /* BIT  4,(XY+o)    */
OP(xycb,67) { xycb_66();                                } /* BIT  4,(XY+o)    */

OP(xycb,68) { xycb_6e();                                } /* BIT  5,(XY+o)    */
OP(xycb,69) { xycb_6e();                                } /* BIT  5,(XY+o)    */
OP(xycb,6a) { xycb_6e();                                } /* BIT  5,(XY+o)    */
OP(xycb,6b) { xycb_6e();                                } /* BIT  5,(XY+o)    */
OP(xycb,6c) { xycb_6e();                                } /* BIT  5,(XY+o)    */
OP(xycb,6d) { xycb_6e();                                } /* BIT  5,(XY+o)    */
OP(xycb,6e) { BIT_XY(5,RM(EA));                         } /* BIT  5,(XY+o)    */
OP(xycb,6f) { xycb_6e();                                } /* BIT  5,(XY+o)    */

OP(xycb,70) { xycb_76();                                } /* BIT  6,(XY+o)    */
OP(xycb,71) { xycb_76();                                } /* BIT  6,(XY+o)    */
OP(xycb,72) { xycb_76();                                } /* BIT  6,(XY+o)    */
OP(xycb,73) { xycb_76();                                } /* BIT  6,(XY+o)    */
OP(xycb,74) { xycb_76();                                } /* BIT  6,(XY+o)    */
OP(xycb,75) { xycb_76();                                } /* BIT  6,(XY+o)    */
OP(xycb,76) { BIT_XY(6,RM(EA));                         } /* BIT  6,(XY+o)    */
OP(xycb,77) { xycb_76();                                } /* BIT  6,(XY+o)    */

OP(xycb,78) { xycb_7e();                                } /* BIT  7,(XY+o)    */
OP(xycb,79) { xycb_7e();                                } /* BIT  7,(XY+o)    */
OP(xycb,7a) { xycb_7e();                                } /* BIT  7,(XY+o)    */
OP(xycb,7b) { xycb_7e();                                } /* BIT  7,(XY+o)    */
OP(xycb,7c) { xycb_7e();                                } /* BIT  7,(XY+o)    */
OP(xycb,7d) { xycb_7e();                                } /* BIT  7,(XY+o)    */
OP(xycb,7e) { BIT_XY(7,RM(EA));                         } /* BIT  7,(XY+o)    */
OP(xycb,7f) { xycb_7e();                                } /* BIT  7,(XY+o)    */

OP(xycb,80) { B = RES(0, RM(EA) ); WM( EA,B );          } /* RES  0,B=(XY+o)  */
OP(xycb,81) { C = RES(0, RM(EA) ); WM( EA,C );          } /* RES  0,C=(XY+o)  */
OP(xycb,82) { D = RES(0, RM(EA) ); WM( EA,D );          } /* RES  0,D=(XY+o)  */
OP(xycb,83) { E = RES(0, RM(EA) ); WM( EA,E );          } /* RES  0,E=(XY+o)  */
OP(xycb,84) { H = RES(0, RM(EA) ); WM( EA,H );          } /* RES  0,H=(XY+o)  */
OP(xycb,85) { L = RES(0, RM(EA) ); WM( EA,L );          } /* RES  0,L=(XY+o)  */
OP(xycb,86) { WM( EA, RES(0,RM(EA)) );                  } /* RES  0,(XY+o)    */
OP(xycb,87) { A = RES(0, RM(EA) ); WM( EA,A );          } /* RES  0,A=(XY+o)  */

OP(xycb,88) { B = RES(1, RM(EA) ); WM( EA,B );          } /* RES  1,B=(XY+o)  */
OP(xycb,89) { C = RES(1, RM(EA) ); WM( EA,C );          } /* RES  1,C=(XY+o)  */
OP(xycb,8a) { D = RES(1, RM(EA) ); WM( EA,D );          } /* RES  1,D=(XY+o)  */
OP(xycb,8b) { E = RES(1, RM(EA) ); WM( EA,E );          } /* RES  1,E=(XY+o)  */
OP(xycb,8c) { H = RES(1, RM(EA) ); WM( EA,H );          } /* RES  1,H=(XY+o)  */
OP(xycb,8d) { L = RES(1, RM(EA) ); WM( EA,L );          } /* RES  1,L=(XY+o)  */
OP(xycb,8e) { WM( EA, RES(1,RM(EA)) );                  } /* RES  1,(XY+o)    */
OP(xycb,8f) { A = RES(1, RM(EA) ); WM( EA,A );          } /* RES  1,A=(XY+o)  */

OP(xycb,90) { B = RES(2, RM(EA) ); WM( EA,B );          } /* RES  2,B=(XY+o)  */
OP(xycb,91) { C = RES(2, RM(EA) ); WM( EA,C );          } /* RES  2,C=(XY+o)  */
OP(xycb,92) { D = RES(2, RM(EA) ); WM( EA,D );          } /* RES  2,D=(XY+o)  */
OP(xycb,93) { E = RES(2, RM(EA) ); WM( EA,E );          } /* RES  2,E=(XY+o)  */
OP(xycb,94) { H = RES(2, RM(EA) ); WM( EA,H );          } /* RES  2,H=(XY+o)  */
OP(xycb,95) { L = RES(2, RM(EA) ); WM( EA,L );          } /* RES  2,L=(XY+o)  */
OP(xycb,96) { WM( EA, RES(2,RM(EA)) );                  } /* RES  2,(XY+o)    */
OP(xycb,97) { A = RES(2, RM(EA) ); WM( EA,A );          } /* RES  2,A=(XY+o)  */

OP(xycb,98) { B = RES(3, RM(EA) ); WM( EA,B );          } /* RES  3,B=(XY+o)  */
OP(xycb,99) { C = RES(3, RM(EA) ); WM( EA,C );          } /* RES  3,C=(XY+o)  */
OP(xycb,9a) { D = RES(3, RM(EA) ); WM( EA,D );          } /* RES  3,D=(XY+o)  */
OP(xycb,9b) { E = RES(3, RM(EA) ); WM( EA,E );          } /* RES  3,E=(XY+o)  */
OP(xycb,9c) { H = RES(3, RM(EA) ); WM( EA,H );          } /* RES  3,H=(XY+o)  */
OP(xycb,9d) { L = RES(3, RM(EA) ); WM( EA,L );          } /* RES  3,L=(XY+o)  */
OP(xycb,9e) { WM( EA, RES(3,RM(EA)) );                  } /* RES  3,(XY+o)    */
OP(xycb,9f) { A = RES(3, RM(EA) ); WM( EA,A );          } /* RES  3,A=(XY+o)  */

OP(xycb,a0) { B = RES(4, RM(EA) ); WM( EA,B );          } /* RES  4,B=(XY+o)  */
OP(xycb,a1) { C = RES(4, RM(EA) ); WM( EA,C );          } /* RES  4,C=(XY+o)  */
OP(xycb,a2) { D = RES(4, RM(EA) ); WM( EA,D );          } /* RES  4,D=(XY+o)  */
OP(xycb,a3) { E = RES(4, RM(EA) ); WM( EA,E );          } /* RES  4,E=(XY+o)  */
OP(xycb,a4) { H = RES(4, RM(EA) ); WM( EA,H );          } /* RES  4,H=(XY+o)  */
OP(xycb,a5) { L = RES(4, RM(EA) ); WM( EA,L );          } /* RES  4,L=(XY+o)  */
OP(xycb,a6) { WM( EA, RES(4,RM(EA)) );                  } /* RES  4,(XY+o)    */
OP(xycb,a7) { A = RES(4, RM(EA) ); WM( EA,A );          } /* RES  4,A=(XY+o)  */

OP(xycb,a8) { B = RES(5, RM(EA) ); WM( EA,B );          } /* RES  5,B=(XY+o)  */
OP(xycb,a9) { C = RES(5, RM(EA) ); WM( EA,C );          } /* RES  5,C=(XY+o)  */
OP(xycb,aa) { D = RES(5, RM(EA) ); WM( EA,D );          } /* RES  5,D=(XY+o)  */
OP(xycb,ab) { E = RES(5, RM(EA) ); WM( EA,E );          } /* RES  5,E=(XY+o)  */
OP(xycb,ac) { H = RES(5, RM(EA) ); WM( EA,H );          } /* RES  5,H=(XY+o)  */
OP(xycb,ad) { L = RES(5, RM(EA) ); WM( EA,L );          } /* RES  5,L=(XY+o)  */
OP(xycb,ae) { WM( EA, RES(5,RM(EA)) );                  } /* RES  5,(XY+o)    */
OP(xycb,af) { A = RES(5, RM(EA) ); WM( EA,A );          } /* RES  5,A=(XY+o)  */

OP(xycb,b0) { B = RES(6, RM(EA) ); WM( EA,B );          } /* RES  6,B=(XY+o)  */
OP(xycb,b1) { C = RES(6, RM(EA) ); WM( EA,C );          } /* RES  6,C=(XY+o)  */
OP(xycb,b2) { D = RES(6, RM(EA) ); WM( EA,D );          } /* RES  6,D=(XY+o)  */
OP(xycb,b3) { E = RES(6, RM(EA) ); WM( EA,E );          } /* RES  6,E=(XY+o)  */
OP(xycb,b4) { H = RES(6, RM(EA) ); WM( EA,H );          } /* RES  6,H=(XY+o)  */
OP(xycb,b5) { L = RES(6, RM(EA) ); WM( EA,L );          } /* RES  6,L=(XY+o)  */
OP(xycb,b6) { WM( EA, RES(6,RM(EA)) );                  } /* RES  6,(XY+o)    */
OP(xycb,b7) { A = RES(6, RM(EA) ); WM( EA,A );          } /* RES  6,A=(XY+o)  */

OP(xycb,b8) { B = RES(7, RM(EA) ); WM( EA,B );          } /* RES  7,B=(XY+o)  */
OP(xycb,b9) { C = RES(7, RM(EA) ); WM( EA,C );          } /* RES  7,C=(XY+o)  */
OP(xycb,ba) { D = RES(7, RM(EA) ); WM( EA,D );          } /* RES  7,D=(XY+o)  */
OP(xycb,bb) { E = RES(7, RM(EA) ); WM( EA,E );          } /* RES  7,E=(XY+o)  */
OP(xycb,bc) { H = RES(7, RM(EA) ); WM( EA,H );          } /* RES  7,H=(XY+o)  */
OP(xycb,bd) { L = RES(7, RM(EA) ); WM( EA,L );          } /* RES  7,L=(XY+o)  */
OP(xycb,be) { WM( EA, RES(7,RM(EA)) );                  } /* RES  7,(XY+o)    */
OP(xycb,bf) { A = RES(7, RM(EA) ); WM( EA,A );          } /* RES  7,A=(XY+o)  */

OP(xycb,c0) { B = SET(0, RM(EA) ); WM( EA,B );          } /* SET  0,B=(XY+o)  */
OP(xycb,c1) { C = SET(0, RM(EA) ); WM( EA,C );          } /* SET  0,C=(XY+o)  */
OP(xycb,c2) { D = SET(0, RM(EA) ); WM( EA,D );          } /* SET  0,D=(XY+o)  */
OP(xycb,c3) { E = SET(0, RM(EA) ); WM( EA,E );          } /* SET  0,E=(XY+o)  */
OP(xycb,c4) { H = SET(0, RM(EA) ); WM( EA,H );          } /* SET  0,H=(XY+o)  */
OP(xycb,c5) { L = SET(0, RM(EA) ); WM( EA,L );          } /* SET  0,L=(XY+o)  */
OP(xycb,c6) { WM( EA, SET(0,RM(EA)) );                  } /* SET  0,(XY+o)    */
OP(xycb,c7) { A = SET(0, RM(EA) ); WM( EA,A );          } /* SET  0,A=(XY+o)  */

OP(xycb,c8) { B = SET(1, RM(EA) ); WM( EA,B );          } /* SET  1,B=(XY+o)  */
OP(xycb,c9) { C = SET(1, RM(EA) ); WM( EA,C );          } /* SET  1,C=(XY+o)  */
OP(xycb,ca) { D = SET(1, RM(EA) ); WM( EA,D );          } /* SET  1,D=(XY+o)  */
OP(xycb,cb) { E = SET(1, RM(EA) ); WM( EA,E );          } /* SET  1,E=(XY+o)  */
OP(xycb,cc) { H = SET(1, RM(EA) ); WM( EA,H );          } /* SET  1,H=(XY+o)  */
OP(xycb,cd) { L = SET(1, RM(EA) ); WM( EA,L );          } /* SET  1,L=(XY+o)  */
OP(xycb,ce) { WM( EA, SET(1,RM(EA)) );                  } /* SET  1,(XY+o)    */
OP(xycb,cf) { A = SET(1, RM(EA) ); WM( EA,A );          } /* SET  1,A=(XY+o)  */

OP(xycb,d0) { B = SET(2, RM(EA) ); WM( EA,B );          } /* SET  2,B=(XY+o)  */
OP(xycb,d1) { C = SET(2, RM(EA) ); WM( EA,C );          } /* SET  2,C=(XY+o)  */
OP(xycb,d2) { D = SET(2, RM(EA) ); WM( EA,D );          } /* SET  2,D=(XY+o)  */
OP(xycb,d3) { E = SET(2, RM(EA) ); WM( EA,E );          } /* SET  2,E=(XY+o)  */
OP(xycb,d4) { H = SET(2, RM(EA) ); WM( EA,H );          } /* SET  2,H=(XY+o)  */
OP(xycb,d5) { L = SET(2, RM(EA) ); WM( EA,L );          } /* SET  2,L=(XY+o)  */
OP(xycb,d6) { WM( EA, SET(2,RM(EA)) );                  } /* SET  2,(XY+o)    */
OP(xycb,d7) { A = SET(2, RM(EA) ); WM( EA,A );          } /* SET  2,A=(XY+o)  */

OP(xycb,d8) { B = SET(3, RM(EA) ); WM( EA,B );          } /* SET  3,B=(XY+o)  */
OP(xycb,d9) { C = SET(3, RM(EA) ); WM( EA,C );          } /* SET  3,C=(XY+o)  */
OP(xycb,da) { D = SET(3, RM(EA) ); WM( EA,D );          } /* SET  3,D=(XY+o)  */
OP(xycb,db) { E = SET(3, RM(EA) ); WM( EA,E );          } /* SET  3,E=(XY+o)  */
OP(xycb,dc) { H = SET(3, RM(EA) ); WM( EA,H );          } /* SET  3,H=(XY+o)  */
OP(xycb,dd) { L = SET(3, RM(EA) ); WM( EA,L );          } /* SET  3,L=(XY+o)  */
OP(xycb,de) { WM( EA, SET(3,RM(EA)) );                  } /* SET  3,(XY+o)    */
OP(xycb,df) { A = SET(3, RM(EA) ); WM( EA,A );          } /* SET  3,A=(XY+o)  */

OP(xycb,e0) { B = SET(4, RM(EA) ); WM( EA,B );          } /* SET  4,B=(XY+o)  */
OP(xycb,e1) { C = SET(4, RM(EA) ); WM( EA,C );          } /* SET  4,C=(XY+o)  */
OP(xycb,e2) { D = SET(4, RM(EA) ); WM( EA,D );          } /* SET  4,D=(XY+o)  */
OP(xycb,e3) { E = SET(4, RM(EA) ); WM( EA,E );          } /* SET  4,E=(XY+o)  */
OP(xycb,e4) { H = SET(4, RM(EA) ); WM( EA,H );          } /* SET  4,H=(XY+o)  */
OP(xycb,e5) { L = SET(4, RM(EA) ); WM( EA,L );          } /* SET  4,L=(XY+o)  */
OP(xycb,e6) { WM( EA, SET(4,RM(EA)) );                  } /* SET  4,(XY+o)    */
OP(xycb,e7) { A = SET(4, RM(EA) ); WM( EA,A );          } /* SET  4,A=(XY+o)  */

OP(xycb,e8) { B = SET(5, RM(EA) ); WM( EA,B );          } /* SET  5,B=(XY+o)  */
OP(xycb,e9) { C = SET(5, RM(EA) ); WM( EA,C );          } /* SET  5,C=(XY+o)  */
OP(xycb,ea) { D = SET(5, RM(EA) ); WM( EA,D );          } /* SET  5,D=(XY+o)  */
OP(xycb,eb) { E = SET(5, RM(EA) ); WM( EA,E );          } /* SET  5,E=(XY+o)  */
OP(xycb,ec) { H = SET(5, RM(EA) ); WM( EA,H );          } /* SET  5,H=(XY+o)  */
OP(xycb,ed) { L = SET(5, RM(EA) ); WM( EA,L );          } /* SET  5,L=(XY+o)  */
OP(xycb,ee) { WM( EA, SET(5,RM(EA)) );                  } /* SET  5,(XY+o)    */
OP(xycb,ef) { A = SET(5, RM(EA) ); WM( EA,A );          } /* SET  5,A=(XY+o)  */

OP(xycb,f0) { B = SET(6, RM(EA) ); WM( EA,B );          } /* SET  6,B=(XY+o)  */
OP(xycb,f1) { C = SET(6, RM(EA) ); WM( EA,C );          } /* SET  6,C=(XY+o)  */
OP(xycb,f2) { D = SET(6, RM(EA) ); WM( EA,D );          } /* SET  6,D=(XY+o)  */
OP(xycb,f3) { E = SET(6, RM(EA) ); WM( EA,E );          } /* SET  6,E=(XY+o)  */
OP(xycb,f4) { H = SET(6, RM(EA) ); WM( EA,H );          } /* SET  6,H=(XY+o)  */
OP(xycb,f5) { L = SET(6, RM(EA) ); WM( EA,L );          } /* SET  6,L=(XY+o)  */
OP(xycb,f6) { WM( EA, SET(6,RM(EA)) );                  } /* SET  6,(XY+o)    */
OP(xycb,f7) { A = SET(6, RM(EA) ); WM( EA,A );          } /* SET  6,A=(XY+o)  */

OP(xycb,f8) { B = SET(7, RM(EA) ); WM( EA,B );          } /* SET  7,B=(XY+o)  */
OP(xycb,f9) { C = SET(7, RM(EA) ); WM( EA,C );          } /* SET  7,C=(XY+o)  */
OP(xycb,fa) { D = SET(7, RM(EA) ); WM( EA,D );          } /* SET  7,D=(XY+o)  */
OP(xycb,fb) { E = SET(7, RM(EA) ); WM( EA,E );          } /* SET  7,E=(XY+o)  */
OP(xycb,fc) { H = SET(7, RM(EA) ); WM( EA,H );          } /* SET  7,H=(XY+o)  */
OP(xycb,fd) { L = SET(7, RM(EA) ); WM( EA,L );          } /* SET  7,L=(XY+o)  */
OP(xycb,fe) { WM( EA, SET(7,RM(EA)) );                  } /* SET  7,(XY+o)    */
OP(xycb,ff) { A = SET(7, RM(EA) ); WM( EA,A );          } /* SET  7,A=(XY+o)  */

/**********************************************************
 * main opcodes
 **********************************************************/
OP(op,00) {                                                                                                } /* NOP              */
OP(op,01) { BC = ARG16();                                                                                  } /* LD   BC,w        */
OP(op,02) { WM( BC, A ); WZ_L = (BC + 1) & 0xFF;  WZ_H = A;                                                } /* LD   (BC),A      */
OP(op,03) { BC++;                                                                                          } /* INC  BC          */
OP(op,04) { B = INC(B);                                                                                    } /* INC  B           */
OP(op,05) { B = DEC(B);                                                                                    } /* DEC  B           */
OP(op,06) { B = ARG();                                                                                     } /* LD   B,n         */
OP(op,07) { RLCA;                                                                                          } /* RLCA             */

OP(op,08) { EX_AF;                                                                                         } /* EX   AF,AF'      */
OP(op,09) { ADD16(hl, bc);                                                                                 } /* ADD  HL,BC       */
OP(op,0a) { A = RM( BC ); WZ=BC+1;                                                                         } /* LD   A,(BC)      */
OP(op,0b) { BC--;                                                                                          } /* DEC  BC          */
OP(op,0c) { C = INC(C);                                                                                    } /* INC  C           */
OP(op,0d) { C = DEC(C);                                                                                    } /* DEC  C           */
OP(op,0e) { C = ARG();                                                                                     } /* LD   C,n         */
OP(op,0f) { RRCA;                                                                                          } /* RRCA             */

OP(op,10) { B--; JR_COND( B, 0x10 );                                                                       } /* DJNZ o           */
OP(op,11) { DE = ARG16();                                                                                  } /* LD   DE,w        */
OP(op,12) { WM( DE, A ); WZ_L = (DE + 1) & 0xFF;  WZ_H = A;                                                } /* LD   (DE),A      */
OP(op,13) { DE++;                                                                                          } /* INC  DE          */
OP(op,14) { D = INC(D);                                                                                    } /* INC  D           */
OP(op,15) { D = DEC(D);                                                                                    } /* DEC  D           */
OP(op,16) { D = ARG();                                                                                     } /* LD   D,n         */
OP(op,17) { RLA;                                                                                           } /* RLA              */

OP(op,18) { JR();                                                                                          } /* JR   o           */
OP(op,19) { ADD16(hl, de);                                                                                 } /* ADD  HL,DE       */
OP(op,1a) { A = RM( DE ); WZ=DE+1;                                                                         } /* LD   A,(DE)      */
OP(op,1b) { DE--;                                                                                          } /* DEC  DE          */
OP(op,1c) { E = INC(E);                                                                                    } /* INC  E           */
OP(op,1d) { E = DEC(E);                                                                                    } /* DEC  E           */
OP(op,1e) { E = ARG();                                                                                     } /* LD   E,n         */
OP(op,1f) { RRA;                                                                                           } /* RRA              */

OP(op,20) { JR_COND( !(F & ZF), 0x20 );                                                                    } /* JR   NZ,o        */
OP(op,21) { HL = ARG16();                                                                                  } /* LD   HL,w        */
OP(op,22) { EA = ARG16(); WM16( EA, &Z80.hl ); WZ = EA+1;                                                  } /* LD   (w),HL      */
OP(op,23) { HL++;                                                                                          } /* INC  HL          */
OP(op,24) { H = INC(H);                                                                                    } /* INC  H           */
OP(op,25) { H = DEC(H);                                                                                    } /* DEC  H           */
OP(op,26) { H = ARG();                                                                                     } /* LD   H,n         */
OP(op,27) { DAA;                                                                                           } /* DAA              */

OP(op,28) { JR_COND( F & ZF, 0x28 );                                                                       } /* JR   Z,o         */
OP(op,29) { ADD16(hl, hl);                                                                                 } /* ADD  HL,HL       */
OP(op,2a) { EA = ARG16(); RM16( EA, &Z80.hl ); WZ = EA+1;                                                  } /* LD   HL,(w)      */
OP(op,2b) { HL--;                                                                                          } /* DEC  HL          */
OP(op,2c) { L = INC(L);                                                                                    } /* INC  L           */
OP(op,2d) { L = DEC(L);                                                                                    } /* DEC  L           */
OP(op,2e) { L = ARG();                                                                                     } /* LD   L,n         */
OP(op,2f) { A ^= 0xff; F = (F&(SF|ZF|PF|CF))|HF|NF|(A&(YF|XF));                                            } /* CPL              */

OP(op,30) { JR_COND( !(F & CF), 0x30 );                                                                    } /* JR   NC,o        */
OP(op,31) { SP = ARG16();                                                                                  } /* LD   SP,w        */
OP(op,32) { EA = ARG16(); WM( EA, A ); WZ_L=(EA+1)&0xFF;WZ_H=A;                                            } /* LD   (w),A       */
OP(op,33) { SP++;                                                                                          } /* INC  SP          */
OP(op,34) { WM( HL, INC(RM(HL)) );                                                                         } /* INC  (HL)        */
OP(op,35) { WM( HL, DEC(RM(HL)) );                                                                         } /* DEC  (HL)        */
OP(op,36) { WM( HL, ARG() );                                                                               } /* LD   (HL),n      */
OP(op,37) { F = (F & (SF|ZF|YF|XF|PF)) | CF | (A & (YF|XF));                                               } /* SCF              */

OP(op,38) { JR_COND( F & CF, 0x38 );                                                                       } /* JR   C,o         */
OP(op,39) { ADD16(hl, sp);                                                                                 } /* ADD  HL,SP       */
OP(op,3a) { EA = ARG16(); A = RM( EA ); WZ = EA+1;                                                         } /* LD   A,(w)       */
OP(op,3b) { SP--;                                                                                          } /* DEC  SP          */
OP(op,3c) { A = INC(A);                                                                                    } /* INC  A           */
OP(op,3d) { A = DEC(A);                                                                                    } /* DEC  A           */
OP(op,3e) { A = ARG();                                                                                     } /* LD   A,n         */
OP(op,3f) { F = ((F&(SF|ZF|YF|XF|PF|CF))|((F&CF)<<4)|(A&(YF|XF)))^CF;                                      } /* CCF              */

OP(op,40) {                                                                                                } /* LD   B,B         */
OP(op,41) { B = C;                                                                                         } /* LD   B,C         */
OP(op,42) { B = D;                                                                                         } /* LD   B,D         */
OP(op,43) { B = E;                                                                                         } /* LD   B,E         */
OP(op,44) { B = H;                                                                                         } /* LD   B,H         */
OP(op,45) { B = L;                                                                                         } /* LD   B,L         */
OP(op,46) { B = RM(HL);                                                                                    } /* LD   B,(HL)      */
OP(op,47) { B = A;                                                                                         } /* LD   B,A         */

OP(op,48) { C = B;                                                                                         } /* LD   C,B         */
OP(op,49) {                                                                                                } /* LD   C,C         */
OP(op,4a) { C = D;                                                                                         } /* LD   C,D         */
OP(op,4b) { C = E;                                                                                         } /* LD   C,E         */
OP(op,4c) { C = H;                                                                                         } /* LD   C,H         */
OP(op,4d) { C = L;                                                                                         } /* LD   C,L         */
OP(op,4e) { C = RM(HL);                                                                                    } /* LD   C,(HL)      */
OP(op,4f) { C = A;                                                                                         } /* LD   C,A         */

OP(op,50) { D = B;                                                                                         } /* LD   D,B         */
OP(op,51) { D = C;                                                                                         } /* LD   D,C         */
OP(op,52) {                                                                                                } /* LD   D,D         */
OP(op,53) { D = E;                                                                                         } /* LD   D,E         */
OP(op,54) { D = H;                                                                                         } /* LD   D,H         */
OP(op,55) { D = L;                                                                                         } /* LD   D,L         */
OP(op,56) { D = RM(HL);                                                                                    } /* LD   D,(HL)      */
OP(op,57) { D = A;                                                                                         } /* LD   D,A         */

OP(op,58) { E = B;                                                                                         } /* LD   E,B         */
OP(op,59) { E = C;                                                                                         } /* LD   E,C         */
OP(op,5a) { E = D;                                                                                         } /* LD   E,D         */
OP(op,5b) {                                                                                                } /* LD   E,E         */
OP(op,5c) { E = H;                                                                                         } /* LD   E,H         */
OP(op,5d) { E = L;                                                                                         } /* LD   E,L         */
OP(op,5e) { E = RM(HL);                                                                                    } /* LD   E,(HL)      */
OP(op,5f) { E = A;                                                                                         } /* LD   E,A         */

OP(op,60) { H = B;                                                                                         } /* LD   H,B         */
OP(op,61) { H = C;                                                                                         } /* LD   H,C         */
OP(op,62) { H = D;                                                                                         } /* LD   H,D         */
OP(op,63) { H = E;                                                                                         } /* LD   H,E         */
OP(op,64) {                                                                                                } /* LD   H,H         */
OP(op,65) { H = L;                                                                                         } /* LD   H,L         */
OP(op,66) { H = RM(HL);                                                                                    } /* LD   H,(HL)      */
OP(op,67) { H = A;                                                                                         } /* LD   H,A         */

OP(op,68) { L = B;                                                                                         } /* LD   L,B         */
OP(op,69) { L = C;                                                                                         } /* LD   L,C         */
OP(op,6a) { L = D;                                                                                         } /* LD   L,D         */
OP(op,6b) { L = E;                                                                                         } /* LD   L,E         */
OP(op,6c) { L = H;                                                                                         } /* LD   L,H         */
OP(op,6d) {                                                                                                } /* LD   L,L         */
OP(op,6e) { L = RM(HL);                                                                                    } /* LD   L,(HL)      */
OP(op,6f) { L = A;                                                                                         } /* LD   L,A         */

OP(op,70) { WM( HL, B );                                                                                   } /* LD   (HL),B      */
OP(op,71) { WM( HL, C );                                                                                   } /* LD   (HL),C      */
OP(op,72) { WM( HL, D );                                                                                   } /* LD   (HL),D      */
OP(op,73) { WM( HL, E );                                                                                   } /* LD   (HL),E      */
OP(op,74) { WM( HL, H );                                                                                   } /* LD   (HL),H      */
OP(op,75) { WM( HL, L );                                                                                   } /* LD   (HL),L      */
OP(op,76) { ENTER_HALT;                                                                                    } /* HALT             */
OP(op,77) { WM( HL, A );                                                                                   } /* LD   (HL),A      */

OP(op,78) { A = B;                                                                                         } /* LD   A,B         */
OP(op,79) { A = C;                                                                                         } /* LD   A,C         */
OP(op,7a) { A = D;                                                                                         } /* LD   A,D         */
OP(op,7b) { A = E;                                                                                         } /* LD   A,E         */
OP(op,7c) { A = H;                                                                                         } /* LD   A,H         */
OP(op,7d) { A = L;                                                                                         } /* LD   A,L         */
OP(op,7e) { A = RM(HL);                                                                                    } /* LD   A,(HL)      */
OP(op,7f) {                                                                                                } /* LD   A,A         */

OP(op,80) { ADD(B);                                                                                        } /* ADD  A,B         */
OP(op,81) { ADD(C);                                                                                        } /* ADD  A,C         */
OP(op,82) { ADD(D);                                                                                        } /* ADD  A,D         */
OP(op,83) { ADD(E);                                                                                        } /* ADD  A,E         */
OP(op,84) { ADD(H);                                                                                        } /* ADD  A,H         */
OP(op,85) { ADD(L);                                                                                        } /* ADD  A,L         */
OP(op,86) { ADD(RM(HL));                                                                                   } /* ADD  A,(HL)      */
OP(op,87) { ADD(A);                                                                                        } /* ADD  A,A         */

OP(op,88) { ADC(B);                                                                                        } /* ADC  A,B         */
OP(op,89) { ADC(C);                                                                                        } /* ADC  A,C         */
OP(op,8a) { ADC(D);                                                                                        } /* ADC  A,D         */
OP(op,8b) { ADC(E);                                                                                        } /* ADC  A,E         */
OP(op,8c) { ADC(H);                                                                                        } /* ADC  A,H         */
OP(op,8d) { ADC(L);                                                                                        } /* ADC  A,L         */
OP(op,8e) { ADC(RM(HL));                                                                                   } /* ADC  A,(HL)      */
OP(op,8f) { ADC(A);                                                                                        } /* ADC  A,A         */

OP(op,90) { SUB(B);                                                                                        } /* SUB  B           */
OP(op,91) { SUB(C);                                                                                        } /* SUB  C           */
OP(op,92) { SUB(D);                                                                                        } /* SUB  D           */
OP(op,93) { SUB(E);                                                                                        } /* SUB  E           */
OP(op,94) { SUB(H);                                                                                        } /* SUB  H           */
OP(op,95) { SUB(L);                                                                                        } /* SUB  L           */
OP(op,96) { SUB(RM(HL));                                                                                   } /* SUB  (HL)        */
OP(op,97) { SUB(A);                                                                                        } /* SUB  A           */

OP(op,98) { SBC(B);                                                                                        } /* SBC  A,B         */
OP(op,99) { SBC(C);                                                                                        } /* SBC  A,C         */
OP(op,9a) { SBC(D);                                                                                        } /* SBC  A,D         */
OP(op,9b) { SBC(E);                                                                                        } /* SBC  A,E         */
OP(op,9c) { SBC(H);                                                                                        } /* SBC  A,H         */
OP(op,9d) { SBC(L);                                                                                        } /* SBC  A,L         */
OP(op,9e) { SBC(RM(HL));                                                                                   } /* SBC  A,(HL)      */
OP(op,9f) { SBC(A);                                                                                        } /* SBC  A,A         */

OP(op,a0) { AND(B);                                                                                        } /* AND  B           */
OP(op,a1) { AND(C);                                                                                        } /* AND  C           */
OP(op,a2) { AND(D);                                                                                        } /* AND  D           */
OP(op,a3) { AND(E);                                                                                        } /* AND  E           */
OP(op,a4) { AND(H);                                                                                        } /* AND  H           */
OP(op,a5) { AND(L);                                                                                        } /* AND  L           */
OP(op,a6) { AND(RM(HL));                                                                                   } /* AND  (HL)        */
OP(op,a7) { AND(A);                                                                                        } /* AND  A           */

OP(op,a8) { XOR(B);                                                                                        } /* XOR  B           */
OP(op,a9) { XOR(C);                                                                                        } /* XOR  C           */
OP(op,aa) { XOR(D);                                                                                        } /* XOR  D           */
OP(op,ab) { XOR(E);                                                                                        } /* XOR  E           */
OP(op,ac) { XOR(H);                                                                                        } /* XOR  H           */
OP(op,ad) { XOR(L);                                                                                        } /* XOR  L           */
OP(op,ae) { XOR(RM(HL));                                                                                   } /* XOR  (HL)        */
OP(op,af) { XOR(A);                                                                                        } /* XOR  A           */

OP(op,b0) { OR(B);                                                                                         } /* OR   B           */
OP(op,b1) { OR(C);                                                                                         } /* OR   C           */
OP(op,b2) { OR(D);                                                                                         } /* OR   D           */
OP(op,b3) { OR(E);                                                                                         } /* OR   E           */
OP(op,b4) { OR(H);                                                                                         } /* OR   H           */
OP(op,b5) { OR(L);                                                                                         } /* OR   L           */
OP(op,b6) { OR(RM(HL));                                                                                    } /* OR   (HL)        */
OP(op,b7) { OR(A);                                                                                         } /* OR   A           */

OP(op,b8) { CP(B);                                                                                         } /* CP   B           */
OP(op,b9) { CP(C);                                                                                         } /* CP   C           */
OP(op,ba) { CP(D);                                                                                         } /* CP   D           */
OP(op,bb) { CP(E);                                                                                         } /* CP   E           */
OP(op,bc) { CP(H);                                                                                         } /* CP   H           */
OP(op,bd) { CP(L);                                                                                         } /* CP   L           */
OP(op,be) { CP(RM(HL));                                                                                    } /* CP   (HL)        */
OP(op,bf) { CP(A);                                                                                         } /* CP   A           */

OP(op,c0) { RET_COND( !(F & ZF), 0xc0 );                                                                   } /* RET  NZ          */
OP(op,c1) { POP( bc );                                                                                     } /* POP  BC          */
OP(op,c2) { JP_COND( !(F & ZF) );                                                                          } /* JP   NZ,a        */
OP(op,c3) { JP;                                                                                            } /* JP   a           */
OP(op,c4) { CALL_COND( !(F & ZF), 0xc4 );                                                                  } /* CALL NZ,a        */
OP(op,c5) { PUSH( bc );                                                                                    } /* PUSH BC          */
OP(op,c6) { ADD(ARG());                                                                                    } /* ADD  A,n         */
OP(op,c7) { RST(0x00);                                                                                     } /* RST  0           */

OP(op,c8) { RET_COND( F & ZF, 0xc8 );                                                                      } /* RET  Z           */
OP(op,c9) { POP( pc ); WZ=PCD;                                                                             } /* RET              */
OP(op,ca) { JP_COND( F & ZF );                                                                             } /* JP   Z,a         */
OP(op,cb) { R++; EXEC(cb,ROP());                                                                           } /* **** CB xx       */
OP(op,cc) { CALL_COND( F & ZF, 0xcc );                                                                     } /* CALL Z,a         */
OP(op,cd) { CALL();                                                                                        } /* CALL a           */
OP(op,ce) { ADC(ARG());                                                                                    } /* ADC  A,n         */
OP(op,cf) { RST(0x08);                                                                                     } /* RST  1           */

OP(op,d0) { RET_COND( !(F & CF), 0xd0 );                                                                   } /* RET  NC          */
OP(op,d1) { POP( de );                                                                                     } /* POP  DE          */
OP(op,d2) { JP_COND( !(F & CF) );                                                                          } /* JP   NC,a        */
OP(op,d3) { unsigned n = ARG() | (A << 8); OUT( n, A ); WZ_L = ((n & 0xff) + 1) & 0xff;  WZ_H = A;         } /* OUT  (n),A       */
OP(op,d4) { CALL_COND( !(F & CF), 0xd4 );                                                                  } /* CALL NC,a        */
OP(op,d5) { PUSH( de );                                                                                    } /* PUSH DE          */
OP(op,d6) { SUB(ARG());                                                                                    } /* SUB  n           */
OP(op,d7) { RST(0x10);                                                                                     } /* RST  2           */

OP(op,d8) { RET_COND( F & CF, 0xd8 );                                                                      } /* RET  C           */
OP(op,d9) { EXX;                                                                                           } /* EXX              */
OP(op,da) { JP_COND( F & CF );                                                                             } /* JP   C,a         */
OP(op,db) { unsigned n = ARG() | (A << 8); A = IN( n ); WZ = n + 1;                                        } /* IN   A,(n)       */
OP(op,dc) { CALL_COND( F & CF, 0xdc );                                                                     } /* CALL C,a         */
OP(op,dd) { R++; EXEC(dd,ROP());                                                                           } /* **** DD xx       */
OP(op,de) { SBC(ARG());                                                                                    } /* SBC  A,n         */
OP(op,df) { RST(0x18);                                                                                     } /* RST  3           */

OP(op,e0) { RET_COND( !(F & PF), 0xe0 );                                                                   } /* RET  PO          */
OP(op,e1) { POP( hl );                                                                                     } /* POP  HL          */
OP(op,e2) { JP_COND( !(F & PF) );                                                                          } /* JP   PO,a        */
OP(op,e3) { EXSP( hl );                                                                                    } /* EX   HL,(SP)     */
OP(op,e4) { CALL_COND( !(F & PF), 0xe4 );                                                                  } /* CALL PO,a        */
OP(op,e5) { PUSH( hl );                                                                                    } /* PUSH HL          */
OP(op,e6) { AND(ARG());                                                                                    } /* AND  n           */
OP(op,e7) { RST(0x20);                                                                                     } /* RST  4           */

OP(op,e8) { RET_COND( F & PF, 0xe8 );                                                                      } /* RET  PE          */
OP(op,e9) { PC = HL;                                                                                       } /* JP   (HL)        */
OP(op,ea) { JP_COND( F & PF );                                                                             } /* JP   PE,a        */
OP(op,eb) { EX_DE_HL;                                                                                      } /* EX   DE,HL       */
OP(op,ec) { CALL_COND( F & PF, 0xec );                                                                     } /* CALL PE,a        */
OP(op,ed) { R++; EXEC(ed,ROP());                                                                           } /* **** ED xx       */
OP(op,ee) { XOR(ARG());                                                                                    } /* XOR  n           */
OP(op,ef) { RST(0x28);                                                                                     } /* RST  5           */

OP(op,f0) { RET_COND( !(F & SF), 0xf0 );                                                                   } /* RET  P           */
OP(op,f1) { POP( af );                                                                                     } /* POP  AF          */
OP(op,f2) { JP_COND( !(F & SF) );                                                                          } /* JP   P,a         */
OP(op,f3) { IFF1 = IFF2 = 0;                                                                               } /* DI               */
OP(op,f4) { CALL_COND( !(F & SF), 0xf4 );                                                                  } /* CALL P,a         */
OP(op,f5) { PUSH( af );                                                                                    } /* PUSH AF          */
OP(op,f6) { OR(ARG());                                                                                     } /* OR   n           */
OP(op,f7) { RST(0x30);                                                                                     } /* RST  6           */

OP(op,f8) { RET_COND( F & SF, 0xf8 );                                                                      } /* RET  M           */
OP(op,f9) { SP = HL;                                                                                       } /* LD   SP,HL       */
OP(op,fa) { JP_COND(F & SF);                                                                               } /* JP   M,a         */
OP(op,fb) { EI;                                                                                            } /* EI               */
OP(op,fc) { CALL_COND( F & SF, 0xfc );                                                                     } /* CALL M,a         */
OP(op,fd) { R++; EXEC(fd,ROP());                                                                           } /* **** FD xx       */
OP(op,fe) { CP(ARG());                                                                                     } /* CP   n           */
OP(op,ff) { RST(0x38);                                                                                     } /* RST  7           */

/**********************************************************
 * IX register related opcodes (DD prefix)
 **********************************************************/
OP_ILLEGAL1(dd,00)                                              /* DB   DD       */
OP_ILLEGAL1(dd,01)                                              /* DB   DD       */
OP_ILLEGAL1(dd,02)                                              /* DB   DD       */
OP_ILLEGAL1(dd,03)                                              /* DB   DD       */
OP_ILLEGAL1(dd,04)                                              /* DB   DD       */
OP_ILLEGAL1(dd,05)                                              /* DB   DD       */
OP_ILLEGAL1(dd,06)                                              /* DB   DD       */
OP_ILLEGAL1(dd,07)                                              /* DB   DD       */

OP_ILLEGAL1(dd,08)                                              /* DB   DD       */
OP(dd,09) { ADD16(ix,bc);                                     } /* ADD  IX,BC    */
OP_ILLEGAL1(dd,0a)                                              /* DB   DD       */
OP_ILLEGAL1(dd,0b)                                              /* DB   DD       */
OP_ILLEGAL1(dd,0c)                                              /* DB   DD       */
OP_ILLEGAL1(dd,0d)                                              /* DB   DD       */
OP_ILLEGAL1(dd,0e)                                              /* DB   DD       */
OP_ILLEGAL1(dd,0f)                                              /* DB   DD       */

OP_ILLEGAL1(dd,10)                                              /* DB   DD       */
OP_ILLEGAL1(dd,11)                                              /* DB   DD       */
OP_ILLEGAL1(dd,12)                                              /* DB   DD       */
OP_ILLEGAL1(dd,13)                                              /* DB   DD       */
OP_ILLEGAL1(dd,14)                                              /* DB   DD       */
OP_ILLEGAL1(dd,15)                                              /* DB   DD       */
OP_ILLEGAL1(dd,16)                                              /* DB   DD       */
OP_ILLEGAL1(dd,17)                                              /* DB   DD       */

OP_ILLEGAL1(dd,18)                                              /* DB   DD       */
OP(dd,19) { ADD16(ix,de);                                     } /* ADD  IX,DE    */
OP_ILLEGAL1(dd,1a)                                              /* DB   DD       */
OP_ILLEGAL1(dd,1b)                                              /* DB   DD       */
OP_ILLEGAL1(dd,1c)                                              /* DB   DD       */
OP_ILLEGAL1(dd,1d)                                              /* DB   DD       */
OP_ILLEGAL1(dd,1e)                                              /* DB   DD       */
OP_ILLEGAL1(dd,1f)                                              /* DB   DD       */

OP_ILLEGAL1(dd,20)                                              /* DB   DD       */
OP(dd,21) { IX = ARG16();                                     } /* LD   IX,w     */
OP(dd,22) { EA = ARG16(); WM16( EA, &Z80.ix ); WZ = EA+1; } /* LD   (w),IX   */
OP(dd,23) { IX++;                                             } /* INC  IX       */
OP(dd,24) { HX = INC(HX);                                     } /* INC  HX       */
OP(dd,25) { HX = DEC(HX);                                     } /* DEC  HX       */
OP(dd,26) { HX = ARG();                                       } /* LD   HX,n     */
OP_ILLEGAL1(dd,27)                                              /* DB   DD       */

OP_ILLEGAL1(dd,28)                                              /* DB   DD       */
OP(dd,29) { ADD16(ix,ix);                                     } /* ADD  IX,IX    */
OP(dd,2a) { EA = ARG16(); RM16( EA, &Z80.ix ); WZ = EA+1; } /* LD   IX,(w)   */
OP(dd,2b) { IX--;                                             } /* DEC  IX       */
OP(dd,2c) { LX = INC(LX);                                     } /* INC  LX       */
OP(dd,2d) { LX = DEC(LX);                                     } /* DEC  LX       */
OP(dd,2e) { LX = ARG();                                       } /* LD   LX,n     */
OP_ILLEGAL1(dd,2f)                                              /* DB   DD       */

OP_ILLEGAL1(dd,30)                                              /* DB   DD       */
OP_ILLEGAL1(dd,31)                                              /* DB   DD       */
OP_ILLEGAL1(dd,32)                                              /* DB   DD       */
OP_ILLEGAL1(dd,33)                                              /* DB   DD       */
OP(dd,34) { EAX(); WM( EA, INC(RM(EA)) );                     } /* INC  (IX+o)   */
OP(dd,35) { EAX(); WM( EA, DEC(RM(EA)) );                     } /* DEC  (IX+o)   */
OP(dd,36) { EAX(); WM( EA, ARG() );                           } /* LD   (IX+o),n */
OP_ILLEGAL1(dd,37)                                              /* DB   DD       */

OP_ILLEGAL1(dd,38)                                              /* DB   DD       */
OP(dd,39) { ADD16(ix,sp);                                     } /* ADD  IX,SP    */
OP_ILLEGAL1(dd,3a)                                              /* DB   DD       */
OP_ILLEGAL1(dd,3b)                                              /* DB   DD       */
OP_ILLEGAL1(dd,3c)                                              /* DB   DD       */
OP_ILLEGAL1(dd,3d)                                              /* DB   DD       */
OP_ILLEGAL1(dd,3e)                                              /* DB   DD       */
OP_ILLEGAL1(dd,3f)                                              /* DB   DD       */

OP_ILLEGAL1(dd,40)                                              /* DB   DD       */
OP_ILLEGAL1(dd,41)                                              /* DB   DD       */
OP_ILLEGAL1(dd,42)                                              /* DB   DD       */
OP_ILLEGAL1(dd,43)                                              /* DB   DD       */
OP(dd,44) { B = HX;                                           } /* LD   B,HX     */
OP(dd,45) { B = LX;                                           } /* LD   B,LX     */
OP(dd,46) { EAX(); B = RM(EA);                                } /* LD   B,(IX+o) */
OP_ILLEGAL1(dd,47)                                              /* DB   DD       */

OP_ILLEGAL1(dd,48)                                              /* DB   DD       */
OP_ILLEGAL1(dd,49)                                              /* DB   DD       */
OP_ILLEGAL1(dd,4a)                                              /* DB   DD       */
OP_ILLEGAL1(dd,4b)                                              /* DB   DD       */
OP(dd,4c) { C = HX;                                           } /* LD   C,HX     */
OP(dd,4d) { C = LX;                                           } /* LD   C,LX     */
OP(dd,4e) { EAX(); C = RM(EA);                                } /* LD   C,(IX+o) */
OP_ILLEGAL1(dd,4f)                                              /* DB   DD       */

OP_ILLEGAL1(dd,50)                                              /* DB   DD       */
OP_ILLEGAL1(dd,51)                                              /* DB   DD       */
OP_ILLEGAL1(dd,52)                                              /* DB   DD       */
OP_ILLEGAL1(dd,53)                                              /* DB   DD       */
OP(dd,54) { D = HX;                                           } /* LD   D,HX     */
OP(dd,55) { D = LX;                                           } /* LD   D,LX     */
OP(dd,56) { EAX(); D = RM(EA);                                } /* LD   D,(IX+o) */
OP_ILLEGAL1(dd,57)                                              /* DB   DD       */

OP_ILLEGAL1(dd,58)                                              /* DB   DD       */
OP_ILLEGAL1(dd,59)                                              /* DB   DD       */
OP_ILLEGAL1(dd,5a)                                              /* DB   DD       */
OP_ILLEGAL1(dd,5b)                                              /* DB   DD       */
OP(dd,5c) { E = HX;                                           } /* LD   E,HX     */
OP(dd,5d) { E = LX;                                           } /* LD   E,LX     */
OP(dd,5e) { EAX(); E = RM(EA);                                } /* LD   E,(IX+o) */
OP_ILLEGAL1(dd,5f)                                              /* DB   DD       */

OP(dd,60) { HX = B;                                           } /* LD   HX,B     */
OP(dd,61) { HX = C;                                           } /* LD   HX,C     */
OP(dd,62) { HX = D;                                           } /* LD   HX,D     */
OP(dd,63) { HX = E;                                           } /* LD   HX,E     */
OP(dd,64) {                                                   } /* LD   HX,HX    */
OP(dd,65) { HX = LX;                                          } /* LD   HX,LX    */
OP(dd,66) { EAX(); H = RM(EA);                                  } /* LD   H,(IX+o) */
OP(dd,67) { HX = A;                                           } /* LD   HX,A     */

OP(dd,68) { LX = B;                                           } /* LD   LX,B     */
OP(dd,69) { LX = C;                                           } /* LD   LX,C     */
OP(dd,6a) { LX = D;                                           } /* LD   LX,D     */
OP(dd,6b) { LX = E;                                           } /* LD   LX,E     */
OP(dd,6c) { LX = HX;                                          } /* LD   LX,HX    */
OP(dd,6d) {                                                   } /* LD   LX,LX    */
OP(dd,6e) { EAX(); L = RM(EA);                                } /* LD   L,(IX+o) */
OP(dd,6f) { LX = A;                                           } /* LD   LX,A     */

OP(dd,70) { EAX(); WM( EA, B );                                 } /* LD   (IX+o),B */
OP(dd,71) { EAX(); WM( EA, C );                                 } /* LD   (IX+o),C */
OP(dd,72) { EAX(); WM( EA, D );                                 } /* LD   (IX+o),D */
OP(dd,73) { EAX(); WM( EA, E );                                 } /* LD   (IX+o),E */
OP(dd,74) { EAX(); WM( EA, H );                                 } /* LD   (IX+o),H */
OP(dd,75) { EAX(); WM( EA, L );                                 } /* LD   (IX+o),L */
OP_ILLEGAL1(dd,76)                                              /* DB   DD       */
OP(dd,77) { EAX(); WM( EA, A );                                 } /* LD   (IX+o),A */

OP_ILLEGAL1(dd,78)                                              /* DB   DD       */
OP_ILLEGAL1(dd,79)                                              /* DB   DD       */
OP_ILLEGAL1(dd,7a)                                              /* DB   DD       */
OP_ILLEGAL1(dd,7b)                                              /* DB   DD       */
OP(dd,7c) { A = HX;                                           } /* LD   A,HX     */
OP(dd,7d) { A = LX;                                           } /* LD   A,LX     */
OP(dd,7e) { EAX(); A = RM(EA);                                } /* LD   A,(IX+o) */
OP_ILLEGAL1(dd,7f)                                              /* DB   DD       */

OP_ILLEGAL1(dd,80)                                              /* DB   DD       */
OP_ILLEGAL1(dd,81)                                              /* DB   DD       */
OP_ILLEGAL1(dd,82)                                              /* DB   DD       */
OP_ILLEGAL1(dd,83)                                              /* DB   DD       */
OP(dd,84) { ADD(HX);                                          } /* ADD  A,HX     */
OP(dd,85) { ADD(LX);                                          } /* ADD  A,LX     */
OP(dd,86) { EAX(); ADD(RM(EA));                                 } /* ADD  A,(IX+o) */
OP_ILLEGAL1(dd,87)                                              /* DB   DD       */

OP_ILLEGAL1(dd,88)                                              /* DB   DD       */
OP_ILLEGAL1(dd,89)                                              /* DB   DD       */
OP_ILLEGAL1(dd,8a)                                              /* DB   DD       */
OP_ILLEGAL1(dd,8b)                                              /* DB   DD       */
OP(dd,8c) { ADC(HX);                                          } /* ADC  A,HX     */
OP(dd,8d) { ADC(LX);                                          } /* ADC  A,LX     */
OP(dd,8e) { EAX(); ADC(RM(EA));                               } /* ADC  A,(IX+o) */
OP_ILLEGAL1(dd,8f)                                              /* DB   DD       */

OP_ILLEGAL1(dd,90)                                              /* DB   DD       */
OP_ILLEGAL1(dd,91)                                              /* DB   DD       */
OP_ILLEGAL1(dd,92)                                              /* DB   DD       */
OP_ILLEGAL1(dd,93)                                              /* DB   DD       */
OP(dd,94) { SUB(HX);                                          } /* SUB  HX       */
OP(dd,95) { SUB(LX);                                          } /* SUB  LX       */
OP(dd,96) { EAX(); SUB(RM(EA));                               } /* SUB  (IX+o)   */
OP_ILLEGAL1(dd,97)                                              /* DB   DD       */

OP_ILLEGAL1(dd,98)                                              /* DB   DD       */
OP_ILLEGAL1(dd,99)                                              /* DB   DD       */
OP_ILLEGAL1(dd,9a)                                              /* DB   DD       */
OP_ILLEGAL1(dd,9b)                                              /* DB   DD       */
OP(dd,9c) { SBC(HX);                                          } /* SBC  A,HX     */
OP(dd,9d) { SBC(LX);                                          } /* SBC  A,LX     */
OP(dd,9e) { EAX(); SBC(RM(EA));                               } /* SBC  A,(IX+o) */
OP_ILLEGAL1(dd,9f)                                              /* DB   DD       */

OP_ILLEGAL1(dd,a0)                                              /* DB   DD       */
OP_ILLEGAL1(dd,a1)                                              /* DB   DD       */
OP_ILLEGAL1(dd,a2)                                              /* DB   DD       */
OP_ILLEGAL1(dd,a3)                                              /* DB   DD       */
OP(dd,a4) { AND(HX);                                          } /* AND  HX       */
OP(dd,a5) { AND(LX);                                          } /* AND  LX       */
OP(dd,a6) { EAX(); AND(RM(EA));                                 } /* AND  (IX+o)   */
OP_ILLEGAL1(dd,a7)                                              /* DB   DD       */

OP_ILLEGAL1(dd,a8)                                              /* DB   DD       */
OP_ILLEGAL1(dd,a9)                                              /* DB   DD       */
OP_ILLEGAL1(dd,aa)                                              /* DB   DD       */
OP_ILLEGAL1(dd,ab)                                              /* DB   DD       */
OP(dd,ac) { XOR(HX);                                          } /* XOR  HX       */
OP(dd,ad) { XOR(LX);                                          } /* XOR  LX       */
OP(dd,ae) { EAX(); XOR(RM(EA));                               } /* XOR  (IX+o)   */
OP_ILLEGAL1(dd,af)                                              /* DB   DD       */

OP_ILLEGAL1(dd,b0)                                              /* DB   DD       */
OP_ILLEGAL1(dd,b1)                                              /* DB   DD       */
OP_ILLEGAL1(dd,b2)                                              /* DB   DD       */
OP_ILLEGAL1(dd,b3)                                              /* DB   DD       */
OP(dd,b4) { OR(HX);                                           } /* OR   HX       */
OP(dd,b5) { OR(LX);                                           } /* OR   LX       */
OP(dd,b6) { EAX(); OR(RM(EA));                                } /* OR   (IX+o)   */
OP_ILLEGAL1(dd,b7)                                              /* DB   DD       */

OP_ILLEGAL1(dd,b8)                                              /* DB   DD       */
OP_ILLEGAL1(dd,b9)                                              /* DB   DD       */
OP_ILLEGAL1(dd,ba)                                              /* DB   DD       */
OP_ILLEGAL1(dd,bb)                                              /* DB   DD       */
OP(dd,bc) { CP(HX);                                           } /* CP   HX       */
OP(dd,bd) { CP(LX);                                           } /* CP   LX       */
OP(dd,be) { EAX(); CP(RM(EA));                                } /* CP   (IX+o)   */
OP_ILLEGAL1(dd,bf)                                              /* DB   DD       */

OP_ILLEGAL1(dd,c0)                                              /* DB   DD       */
OP_ILLEGAL1(dd,c1)                                              /* DB   DD       */
OP_ILLEGAL1(dd,c2)                                              /* DB   DD       */
OP_ILLEGAL1(dd,c3)                                              /* DB   DD       */
OP_ILLEGAL1(dd,c4)                                              /* DB   DD       */
OP_ILLEGAL1(dd,c5)                                              /* DB   DD       */
OP_ILLEGAL1(dd,c6)                                              /* DB   DD       */
OP_ILLEGAL1(dd,c7)                                              /* DB   DD       */

OP_ILLEGAL1(dd,c8)                                              /* DB   DD       */
OP_ILLEGAL1(dd,c9)                                              /* DB   DD       */
OP_ILLEGAL1(dd,ca)                                              /* DB   DD       */
OP(dd,cb) { EAX(); EXEC(xycb,ARG());                            } /* **** DD CB xx */
OP_ILLEGAL1(dd,cc)                                              /* DB   DD       */
OP_ILLEGAL1(dd,cd)                                              /* DB   DD       */
OP_ILLEGAL1(dd,ce)                                              /* DB   DD       */
OP_ILLEGAL1(dd,cf)                                              /* DB   DD       */

OP_ILLEGAL1(dd,d0)                                              /* DB   DD       */
OP_ILLEGAL1(dd,d1)                                              /* DB   DD       */
OP_ILLEGAL1(dd,d2)                                              /* DB   DD       */
OP_ILLEGAL1(dd,d3)                                              /* DB   DD       */
OP_ILLEGAL1(dd,d4)                                              /* DB   DD       */
OP_ILLEGAL1(dd,d5)                                              /* DB   DD       */
OP_ILLEGAL1(dd,d6)                                              /* DB   DD       */
OP_ILLEGAL1(dd,d7)                                              /* DB   DD       */

OP_ILLEGAL1(dd,d8)                                              /* DB   DD       */
OP_ILLEGAL1(dd,d9)                                              /* DB   DD       */
OP_ILLEGAL1(dd,da)                                              /* DB   DD       */
OP_ILLEGAL1(dd,db)                                              /* DB   DD       */
OP_ILLEGAL1(dd,dc)                                              /* DB   DD       */
OP(dd,dd) { EXEC(dd,ROP());                                   } /* **** DD DD xx */
OP_ILLEGAL1(dd,de)                                              /* DB   DD       */
OP_ILLEGAL1(dd,df)                                              /* DB   DD       */

OP_ILLEGAL1(dd,e0)                                              /* DB   DD       */
OP(dd,e1) { POP( ix );                                        } /* POP  IX       */
OP_ILLEGAL1(dd,e2)                                              /* DB   DD       */
OP(dd,e3) { EXSP( ix );                                       } /* EX   (SP),IX  */
OP_ILLEGAL1(dd,e4)                                              /* DB   DD       */
OP(dd,e5) { PUSH( ix );                                       } /* PUSH IX       */
OP_ILLEGAL1(dd,e6)                                              /* DB   DD       */
OP_ILLEGAL1(dd,e7)                                              /* DB   DD       */

OP_ILLEGAL1(dd,e8)                                              /* DB   DD       */
OP(dd,e9) { PC = IX;                                          } /* JP   (IX)     */
OP_ILLEGAL1(dd,ea)                                              /* DB   DD       */
OP_ILLEGAL1(dd,eb)                                              /* DB   DD       */
OP_ILLEGAL1(dd,ec)                                              /* DB   DD       */
OP_ILLEGAL1(dd,ed)                                              /* DB   DD       */
OP_ILLEGAL1(dd,ee)                                              /* DB   DD       */
OP_ILLEGAL1(dd,ef)                                              /* DB   DD       */

OP_ILLEGAL1(dd,f0)                                              /* DB   DD       */
OP_ILLEGAL1(dd,f1)                                              /* DB   DD       */
OP_ILLEGAL1(dd,f2)                                              /* DB   DD       */
OP_ILLEGAL1(dd,f3)                                              /* DB   DD       */
OP_ILLEGAL1(dd,f4)                                              /* DB   DD       */
OP_ILLEGAL1(dd,f5)                                              /* DB   DD       */
OP_ILLEGAL1(dd,f6)                                              /* DB   DD       */
OP_ILLEGAL1(dd,f7)                                              /* DB   DD       */

OP_ILLEGAL1(dd,f8)                                              /* DB   DD       */
OP(dd,f9) { SP = IX;                                          } /* LD   SP,IX    */
OP_ILLEGAL1(dd,fa)                                              /* DB   DD       */
OP_ILLEGAL1(dd,fb)                                              /* DB   DD       */
OP_ILLEGAL1(dd,fc)                                              /* DB   DD       */
OP(dd,fd) { EXEC(fd,ROP());                                   } /* **** DD FD xx */
OP_ILLEGAL1(dd,fe)                                              /* DB   DD       */
OP_ILLEGAL1(dd,ff)                                              /* DB   DD       */

/**********************************************************
 * IY register related opcodes (FD prefix)
 **********************************************************/
OP_ILLEGAL1(fd,00)                                              /* DB   FD       */
OP_ILLEGAL1(fd,01)                                              /* DB   FD       */
OP_ILLEGAL1(fd,02)                                              /* DB   FD       */
OP_ILLEGAL1(fd,03)                                              /* DB   FD       */
OP_ILLEGAL1(fd,04)                                              /* DB   FD       */
OP_ILLEGAL1(fd,05)                                              /* DB   FD       */
OP_ILLEGAL1(fd,06)                                              /* DB   FD       */
OP_ILLEGAL1(fd,07)                                              /* DB   FD       */

OP_ILLEGAL1(fd,08)                                              /* DB   FD       */
OP(fd,09) { ADD16(iy,bc);                                     } /* ADD  IY,BC    */
OP_ILLEGAL1(fd,0a)                                              /* DB   FD       */
OP_ILLEGAL1(fd,0b)                                              /* DB   FD       */
OP_ILLEGAL1(fd,0c)                                              /* DB   FD       */
OP_ILLEGAL1(fd,0d)                                              /* DB   FD       */
OP_ILLEGAL1(fd,0e)                                              /* DB   FD       */
OP_ILLEGAL1(fd,0f)                                              /* DB   FD       */

OP_ILLEGAL1(fd,10)                                              /* DB   FD       */
OP_ILLEGAL1(fd,11)                                              /* DB   FD       */
OP_ILLEGAL1(fd,12)                                              /* DB   FD       */
OP_ILLEGAL1(fd,13)                                              /* DB   FD       */
OP_ILLEGAL1(fd,14)                                              /* DB   FD       */
OP_ILLEGAL1(fd,15)                                              /* DB   FD       */
OP_ILLEGAL1(fd,16)                                              /* DB   FD       */
OP_ILLEGAL1(fd,17)                                              /* DB   FD       */

OP_ILLEGAL1(fd,18)                                              /* DB   FD       */
OP(fd,19) { ADD16(iy,de);                                     } /* ADD  IY,DE    */
OP_ILLEGAL1(fd,1a)                                              /* DB   FD       */
OP_ILLEGAL1(fd,1b)                                              /* DB   FD       */
OP_ILLEGAL1(fd,1c)                                              /* DB   FD       */
OP_ILLEGAL1(fd,1d)                                              /* DB   FD       */
OP_ILLEGAL1(fd,1e)                                              /* DB   FD       */
OP_ILLEGAL1(fd,1f)                                              /* DB   FD       */

OP_ILLEGAL1(fd,20)                                              /* DB   FD       */
OP(fd,21) { IY = ARG16();                                     } /* LD   IY,w     */
OP(fd,22) { EA = ARG16(); WM16( EA, &Z80.iy ); WZ = EA+1; } /* LD   (w),IY   */
OP(fd,23) { IY++;                                             } /* INC  IY       */
OP(fd,24) { HY = INC(HY);                                     } /* INC  HY       */
OP(fd,25) { HY = DEC(HY);                                     } /* DEC  HY       */
OP(fd,26) { HY = ARG();                                       } /* LD   HY,n     */
OP_ILLEGAL1(fd,27)                                              /* DB   FD       */

OP_ILLEGAL1(fd,28)                                              /* DB   FD       */
OP(fd,29) { ADD16(iy,iy);                                     } /* ADD  IY,IY    */
OP(fd,2a) { EA = ARG16(); RM16( EA, &Z80.iy ); WZ = EA+1; } /* LD   IY,(w)   */
OP(fd,2b) { IY--;                                             } /* DEC  IY       */
OP(fd,2c) { LY = INC(LY);                                     } /* INC  LY       */
OP(fd,2d) { LY = DEC(LY);                                     } /* DEC  LY       */
OP(fd,2e) { LY = ARG();                                       } /* LD   LY,n     */
OP_ILLEGAL1(fd,2f)                                              /* DB   FD       */

OP_ILLEGAL1(fd,30)                                              /* DB   FD       */
OP_ILLEGAL1(fd,31)                                              /* DB   FD       */
OP_ILLEGAL1(fd,32)                                              /* DB   FD       */
OP_ILLEGAL1(fd,33)                                              /* DB   FD       */
OP(fd,34) { EAY(); WM( EA, INC(RM(EA)) );                     } /* INC  (IY+o)   */
OP(fd,35) { EAY(); WM( EA, DEC(RM(EA)) );                     } /* DEC  (IY+o)   */
OP(fd,36) { EAY(); WM( EA, ARG() );                           } /* LD   (IY+o),n */
OP_ILLEGAL1(fd,37)                                              /* DB   FD       */

OP_ILLEGAL1(fd,38)                                              /* DB   FD       */
OP(fd,39) { ADD16(iy,sp);                                     } /* ADD  IY,SP    */
OP_ILLEGAL1(fd,3a)                                              /* DB   FD       */
OP_ILLEGAL1(fd,3b)                                              /* DB   FD       */
OP_ILLEGAL1(fd,3c)                                              /* DB   FD       */
OP_ILLEGAL1(fd,3d)                                              /* DB   FD       */
OP_ILLEGAL1(fd,3e)                                              /* DB   FD       */
OP_ILLEGAL1(fd,3f)                                              /* DB   FD       */

OP_ILLEGAL1(fd,40)                                              /* DB   FD       */
OP_ILLEGAL1(fd,41)                                              /* DB   FD       */
OP_ILLEGAL1(fd,42)                                              /* DB   FD       */
OP_ILLEGAL1(fd,43)                                              /* DB   FD       */
OP(fd,44) { B = HY;                                           } /* LD   B,HY     */
OP(fd,45) { B = LY;                                           } /* LD   B,LY     */
OP(fd,46) { EAY(); B = RM(EA);                                } /* LD   B,(IY+o) */
OP_ILLEGAL1(fd,47)                                              /* DB   FD       */

OP_ILLEGAL1(fd,48)                                              /* DB   FD       */
OP_ILLEGAL1(fd,49)                                              /* DB   FD       */
OP_ILLEGAL1(fd,4a)                                              /* DB   FD       */
OP_ILLEGAL1(fd,4b)                                              /* DB   FD       */
OP(fd,4c) { C = HY;                                           } /* LD   C,HY     */
OP(fd,4d) { C = LY;                                           } /* LD   C,LY     */
OP(fd,4e) { EAY(); C = RM(EA);                                } /* LD   C,(IY+o) */
OP_ILLEGAL1(fd,4f)                                              /* DB   FD       */

OP_ILLEGAL1(fd,50)                                              /* DB   FD       */
OP_ILLEGAL1(fd,51)                                              /* DB   FD       */
OP_ILLEGAL1(fd,52)                                              /* DB   FD       */
OP_ILLEGAL1(fd,53)                                              /* DB   FD       */
OP(fd,54) { D = HY;                                           } /* LD   D,HY     */
OP(fd,55) { D = LY;                                           } /* LD   D,LY     */
OP(fd,56) { EAY(); D = RM(EA);                                } /* LD   D,(IY+o) */
OP_ILLEGAL1(fd,57)                                              /* DB   FD       */

OP_ILLEGAL1(fd,58)                                              /* DB   FD       */
OP_ILLEGAL1(fd,59)                                              /* DB   FD       */
OP_ILLEGAL1(fd,5a)                                              /* DB   FD       */
OP_ILLEGAL1(fd,5b)                                              /* DB   FD       */
OP(fd,5c) { E = HY;                                           } /* LD   E,HY     */
OP(fd,5d) { E = LY;                                           } /* LD   E,LY     */
OP(fd,5e) { EAY(); E = RM(EA);                                } /* LD   E,(IY+o) */
OP_ILLEGAL1(fd,5f)                                              /* DB   FD       */

OP(fd,60) { HY = B;                                           } /* LD   HY,B     */
OP(fd,61) { HY = C;                                           } /* LD   HY,C     */
OP(fd,62) { HY = D;                                           } /* LD   HY,D     */
OP(fd,63) { HY = E;                                           } /* LD   HY,E     */
OP(fd,64) {                                                   } /* LD   HY,HY    */
OP(fd,65) { HY = LY;                                          } /* LD   HY,LY    */
OP(fd,66) { EAY(); H = RM(EA);                                } /* LD   H,(IY+o) */
OP(fd,67) { HY = A;                                           } /* LD   HY,A     */

OP(fd,68) { LY = B;                                           } /* LD   LY,B     */
OP(fd,69) { LY = C;                                           } /* LD   LY,C     */
OP(fd,6a) { LY = D;                                           } /* LD   LY,D     */
OP(fd,6b) { LY = E;                                           } /* LD   LY,E     */
OP(fd,6c) { LY = HY;                                          } /* LD   LY,HY    */
OP(fd,6d) {                                                   } /* LD   LY,LY    */
OP(fd,6e) { EAY(); L = RM(EA);                                } /* LD   L,(IY+o) */
OP(fd,6f) { LY = A;                                           } /* LD   LY,A     */

OP(fd,70) { EAY(); WM( EA, B );                                 } /* LD   (IY+o),B */
OP(fd,71) { EAY(); WM( EA, C );                                 } /* LD   (IY+o),C */
OP(fd,72) { EAY(); WM( EA, D );                                 } /* LD   (IY+o),D */
OP(fd,73) { EAY(); WM( EA, E );                                 } /* LD   (IY+o),E */
OP(fd,74) { EAY(); WM( EA, H );                                 } /* LD   (IY+o),H */
OP(fd,75) { EAY(); WM( EA, L );                                 } /* LD   (IY+o),L */
OP_ILLEGAL1(fd,76)                                              /* DB   FD       */
OP(fd,77) { EAY(); WM( EA, A );                                 } /* LD   (IY+o),A */

OP_ILLEGAL1(fd,78)                                              /* DB   FD       */
OP_ILLEGAL1(fd,79)                                              /* DB   FD       */
OP_ILLEGAL1(fd,7a)                                              /* DB   FD       */
OP_ILLEGAL1(fd,7b)                                              /* DB   FD       */
OP(fd,7c) { A = HY;                                           } /* LD   A,HY     */
OP(fd,7d) { A = LY;                                           } /* LD   A,LY     */
OP(fd,7e) { EAY(); A = RM(EA);                                } /* LD   A,(IY+o) */
OP_ILLEGAL1(fd,7f)                                              /* DB   FD       */

OP_ILLEGAL1(fd,80)                                              /* DB   FD       */
OP_ILLEGAL1(fd,81)                                              /* DB   FD       */
OP_ILLEGAL1(fd,82)                                              /* DB   FD       */
OP_ILLEGAL1(fd,83)                                              /* DB   FD       */
OP(fd,84) { ADD(HY);                                          } /* ADD  A,HY     */
OP(fd,85) { ADD(LY);                                          } /* ADD  A,LY     */
OP(fd,86) { EAY(); ADD(RM(EA));                               } /* ADD  A,(IY+o) */
OP_ILLEGAL1(fd,87)                                              /* DB   FD       */

OP_ILLEGAL1(fd,88)                                              /* DB   FD       */
OP_ILLEGAL1(fd,89)                                              /* DB   FD       */
OP_ILLEGAL1(fd,8a)                                              /* DB   FD       */
OP_ILLEGAL1(fd,8b)                                              /* DB   FD       */
OP(fd,8c) { ADC(HY);                                          } /* ADC  A,HY     */
OP(fd,8d) { ADC(LY);                                          } /* ADC  A,LY     */
OP(fd,8e) { EAY(); ADC(RM(EA));                                 } /* ADC  A,(IY+o) */
OP_ILLEGAL1(fd,8f)                                              /* DB   FD       */

OP_ILLEGAL1(fd,90)                                              /* DB   FD       */
OP_ILLEGAL1(fd,91)                                              /* DB   FD       */
OP_ILLEGAL1(fd,92)                                              /* DB   FD       */
OP_ILLEGAL1(fd,93)                                              /* DB   FD       */
OP(fd,94) { SUB(HY);                                          } /* SUB  HY       */
OP(fd,95) { SUB(LY);                                          } /* SUB  LY       */
OP(fd,96) { EAY(); SUB(RM(EA));                               } /* SUB  (IY+o)   */
OP_ILLEGAL1(fd,97)                                              /* DB   FD       */

OP_ILLEGAL1(fd,98)                                              /* DB   FD       */
OP_ILLEGAL1(fd,99)                                              /* DB   FD       */
OP_ILLEGAL1(fd,9a)                                              /* DB   FD       */
OP_ILLEGAL1(fd,9b)                                              /* DB   FD       */
OP(fd,9c) { SBC(HY);                                          } /* SBC  A,HY     */
OP(fd,9d) { SBC(LY);                                          } /* SBC  A,LY     */
OP(fd,9e) { EAY(); SBC(RM(EA));                                 } /* SBC  A,(IY+o) */
OP_ILLEGAL1(fd,9f)                                              /* DB   FD       */

OP_ILLEGAL1(fd,a0)                                              /* DB   FD       */
OP_ILLEGAL1(fd,a1)                                              /* DB   FD       */
OP_ILLEGAL1(fd,a2)                                              /* DB   FD       */
OP_ILLEGAL1(fd,a3)                                              /* DB   FD       */
OP(fd,a4) { AND(HY);                                          } /* AND  HY       */
OP(fd,a5) { AND(LY);                                          } /* AND  LY       */
OP(fd,a6) { EAY(); AND(RM(EA));                                 } /* AND  (IY+o)   */
OP_ILLEGAL1(fd,a7)                                              /* DB   FD       */

OP_ILLEGAL1(fd,a8)                                              /* DB   FD       */
OP_ILLEGAL1(fd,a9)                                              /* DB   FD       */
OP_ILLEGAL1(fd,aa)                                              /* DB   FD       */
OP_ILLEGAL1(fd,ab)                                              /* DB   FD       */
OP(fd,ac) { XOR(HY);                                          } /* XOR  HY       */
OP(fd,ad) { XOR(LY);                                          } /* XOR  LY       */
OP(fd,ae) { EAY(); XOR(RM(EA));                               } /* XOR  (IY+o)   */
OP_ILLEGAL1(fd,af)                                              /* DB   FD       */

OP_ILLEGAL1(fd,b0)                                              /* DB   FD       */
OP_ILLEGAL1(fd,b1)                                              /* DB   FD       */
OP_ILLEGAL1(fd,b2)                                              /* DB   FD       */
OP_ILLEGAL1(fd,b3)                                              /* DB   FD       */
OP(fd,b4) { OR(HY);                                           } /* OR   HY       */
OP(fd,b5) { OR(LY);                                           } /* OR   LY       */
OP(fd,b6) { EAY(); OR(RM(EA));                                  } /* OR   (IY+o)   */
OP_ILLEGAL1(fd,b7)                                              /* DB   FD       */

OP_ILLEGAL1(fd,b8)                                              /* DB   FD       */
OP_ILLEGAL1(fd,b9)                                              /* DB   FD       */
OP_ILLEGAL1(fd,ba)                                              /* DB   FD       */
OP_ILLEGAL1(fd,bb)                                              /* DB   FD       */
OP(fd,bc) { CP(HY);                                           } /* CP   HY       */
OP(fd,bd) { CP(LY);                                           } /* CP   LY       */
OP(fd,be) { EAY(); CP(RM(EA));                                } /* CP   (IY+o)   */
OP_ILLEGAL1(fd,bf)                                              /* DB   FD       */

OP_ILLEGAL1(fd,c0)                                              /* DB   FD       */
OP_ILLEGAL1(fd,c1)                                              /* DB   FD       */
OP_ILLEGAL1(fd,c2)                                              /* DB   FD       */
OP_ILLEGAL1(fd,c3)                                              /* DB   FD       */
OP_ILLEGAL1(fd,c4)                                              /* DB   FD       */
OP_ILLEGAL1(fd,c5)                                              /* DB   FD       */
OP_ILLEGAL1(fd,c6)                                              /* DB   FD       */
OP_ILLEGAL1(fd,c7)                                              /* DB   FD       */

OP_ILLEGAL1(fd,c8)                                              /* DB   FD       */
OP_ILLEGAL1(fd,c9)                                              /* DB   FD       */
OP_ILLEGAL1(fd,ca)                                              /* DB   FD       */
OP(fd,cb) { EAY(); EXEC(xycb,ARG());                            } /* **** FD CB xx */
OP_ILLEGAL1(fd,cc)                                              /* DB   FD       */
OP_ILLEGAL1(fd,cd)                                              /* DB   FD       */
OP_ILLEGAL1(fd,ce)                                              /* DB   FD       */
OP_ILLEGAL1(fd,cf)                                              /* DB   FD       */

OP_ILLEGAL1(fd,d0)                                              /* DB   FD       */
OP_ILLEGAL1(fd,d1)                                              /* DB   FD       */
OP_ILLEGAL1(fd,d2)                                              /* DB   FD       */
OP_ILLEGAL1(fd,d3)                                              /* DB   FD       */
OP_ILLEGAL1(fd,d4)                                              /* DB   FD       */
OP_ILLEGAL1(fd,d5)                                              /* DB   FD       */
OP_ILLEGAL1(fd,d6)                                              /* DB   FD       */
OP_ILLEGAL1(fd,d7)                                              /* DB   FD       */

OP_ILLEGAL1(fd,d8)                                              /* DB   FD       */
OP_ILLEGAL1(fd,d9)                                              /* DB   FD       */
OP_ILLEGAL1(fd,da)                                              /* DB   FD       */
OP_ILLEGAL1(fd,db)                                              /* DB   FD       */
OP_ILLEGAL1(fd,dc)                                              /* DB   FD       */
OP(fd,dd) { EXEC(dd,ROP());                                   } /* **** FD DD xx */
OP_ILLEGAL1(fd,de)                                              /* DB   FD       */
OP_ILLEGAL1(fd,df)                                              /* DB   FD       */

OP_ILLEGAL1(fd,e0)                                              /* DB   FD       */
OP(fd,e1) { POP( iy );                                        } /* POP  IY       */
OP_ILLEGAL1(fd,e2)                                              /* DB   FD       */
OP(fd,e3) { EXSP( iy );                                       } /* EX   (SP),IY  */
OP_ILLEGAL1(fd,e4)                                              /* DB   FD       */
OP(fd,e5) { PUSH( iy );                                       } /* PUSH IY       */
OP_ILLEGAL1(fd,e6)                                              /* DB   FD       */
OP_ILLEGAL1(fd,e7)                                              /* DB   FD       */

OP_ILLEGAL1(fd,e8)                                              /* DB   FD       */
OP(fd,e9) { PC = IY;                                          } /* JP   (IY)     */
OP_ILLEGAL1(fd,ea)                                              /* DB   FD       */
OP_ILLEGAL1(fd,eb)                                              /* DB   FD       */
OP_ILLEGAL1(fd,ec)                                              /* DB   FD       */
OP_ILLEGAL1(fd,ed)                                              /* DB   FD       */
OP_ILLEGAL1(fd,ee)                                              /* DB   FD       */
OP_ILLEGAL1(fd,ef)                                              /* DB   FD       */

OP_ILLEGAL1(fd,f0)                                              /* DB   FD       */
OP_ILLEGAL1(fd,f1)                                              /* DB   FD       */
OP_ILLEGAL1(fd,f2)                                              /* DB   FD       */
OP_ILLEGAL1(fd,f3)                                              /* DB   FD       */
OP_ILLEGAL1(fd,f4)                                              /* DB   FD       */
OP_ILLEGAL1(fd,f5)                                              /* DB   FD       */
OP_ILLEGAL1(fd,f6)                                              /* DB   FD       */
OP_ILLEGAL1(fd,f7)                                              /* DB   FD       */

OP_ILLEGAL1(fd,f8)                                              /* DB   FD       */
OP(fd,f9) { SP = IY;                                          } /* LD   SP,IY    */
OP_ILLEGAL1(fd,fa)                                              /* DB   FD       */
OP_ILLEGAL1(fd,fb)                                              /* DB   FD       */
OP_ILLEGAL1(fd,fc)                                              /* DB   FD       */
OP(fd,fd) { EXEC(fd,ROP());                                   } /* **** FD FD xx */
OP_ILLEGAL1(fd,fe)                                              /* DB   FD       */
OP_ILLEGAL1(fd,ff)                                              /* DB   FD       */

/**********************************************************
 * special opcodes (ED prefix)
 **********************************************************/
OP_ILLEGAL2(ed,00)                                              /* DB   ED      */
OP_ILLEGAL2(ed,01)                                              /* DB   ED      */
OP_ILLEGAL2(ed,02)                                              /* DB   ED      */
OP_ILLEGAL2(ed,03)                                              /* DB   ED      */
OP_ILLEGAL2(ed,04)                                              /* DB   ED      */
OP_ILLEGAL2(ed,05)                                              /* DB   ED      */
OP_ILLEGAL2(ed,06)                                              /* DB   ED      */
OP_ILLEGAL2(ed,07)                                              /* DB   ED      */

OP_ILLEGAL2(ed,08)                                              /* DB   ED      */
OP_ILLEGAL2(ed,09)                                              /* DB   ED      */
OP_ILLEGAL2(ed,0a)                                              /* DB   ED      */
OP_ILLEGAL2(ed,0b)                                              /* DB   ED      */
OP_ILLEGAL2(ed,0c)                                              /* DB   ED      */
OP_ILLEGAL2(ed,0d)                                              /* DB   ED      */
OP_ILLEGAL2(ed,0e)                                              /* DB   ED      */
OP_ILLEGAL2(ed,0f)                                              /* DB   ED      */

OP_ILLEGAL2(ed,10)                                              /* DB   ED      */
OP_ILLEGAL2(ed,11)                                              /* DB   ED      */
OP_ILLEGAL2(ed,12)                                              /* DB   ED      */
OP_ILLEGAL2(ed,13)                                              /* DB   ED      */
OP_ILLEGAL2(ed,14)                                              /* DB   ED      */
OP_ILLEGAL2(ed,15)                                              /* DB   ED      */
OP_ILLEGAL2(ed,16)                                              /* DB   ED      */
OP_ILLEGAL2(ed,17)                                              /* DB   ED      */

OP_ILLEGAL2(ed,18)                                              /* DB   ED      */
OP_ILLEGAL2(ed,19)                                              /* DB   ED      */
OP_ILLEGAL2(ed,1a)                                              /* DB   ED      */
OP_ILLEGAL2(ed,1b)                                              /* DB   ED      */
OP_ILLEGAL2(ed,1c)                                              /* DB   ED      */
OP_ILLEGAL2(ed,1d)                                              /* DB   ED      */
OP_ILLEGAL2(ed,1e)                                              /* DB   ED      */
OP_ILLEGAL2(ed,1f)                                              /* DB   ED      */

OP_ILLEGAL2(ed,20)                                              /* DB   ED      */
OP_ILLEGAL2(ed,21)                                              /* DB   ED      */
OP_ILLEGAL2(ed,22)                                              /* DB   ED      */
OP_ILLEGAL2(ed,23)                                              /* DB   ED      */
OP_ILLEGAL2(ed,24)                                              /* DB   ED      */
OP_ILLEGAL2(ed,25)                                              /* DB   ED      */
OP_ILLEGAL2(ed,26)                                              /* DB   ED      */
OP_ILLEGAL2(ed,27)                                              /* DB   ED      */

OP_ILLEGAL2(ed,28)                                              /* DB   ED      */
OP_ILLEGAL2(ed,29)                                              /* DB   ED      */
OP_ILLEGAL2(ed,2a)                                              /* DB   ED      */
OP_ILLEGAL2(ed,2b)                                              /* DB   ED      */
OP_ILLEGAL2(ed,2c)                                              /* DB   ED      */
OP_ILLEGAL2(ed,2d)                                              /* DB   ED      */
OP_ILLEGAL2(ed,2e)                                              /* DB   ED      */
OP_ILLEGAL2(ed,2f)                                              /* DB   ED      */

OP_ILLEGAL2(ed,30)                                              /* DB   ED      */
OP_ILLEGAL2(ed,31)                                              /* DB   ED      */
OP_ILLEGAL2(ed,32)                                              /* DB   ED      */
OP_ILLEGAL2(ed,33)                                              /* DB   ED      */
OP_ILLEGAL2(ed,34)                                              /* DB   ED      */
OP_ILLEGAL2(ed,35)                                              /* DB   ED      */
OP_ILLEGAL2(ed,36)                                              /* DB   ED      */
OP_ILLEGAL2(ed,37)                                              /* DB   ED      */

OP_ILLEGAL2(ed,38)                                              /* DB   ED      */
OP_ILLEGAL2(ed,39)                                              /* DB   ED      */
OP_ILLEGAL2(ed,3a)                                              /* DB   ED      */
OP_ILLEGAL2(ed,3b)                                              /* DB   ED      */
OP_ILLEGAL2(ed,3c)                                              /* DB   ED      */
OP_ILLEGAL2(ed,3d)                                              /* DB   ED      */
OP_ILLEGAL2(ed,3e)                                              /* DB   ED      */
OP_ILLEGAL2(ed,3f)                                              /* DB   ED      */

OP(ed,40) { B = IN(BC); F = (F & CF) | SZP[B];                } /* IN   B,(C)   */
OP(ed,41) { OUT(BC, B);                                       } /* OUT  (C),B   */
OP(ed,42) { SBC16( bc );                                      } /* SBC  HL,BC   */
OP(ed,43) { EA = ARG16(); WM16( EA, &Z80.bc ); WZ = EA+1; } /* LD   (w),BC  */
OP(ed,44) { NEG;                                              } /* NEG          */
OP(ed,45) { RETN;                                             } /* RETN;        */
OP(ed,46) { IM = 0;                                           } /* IM   0       */
OP(ed,47) { LD_I_A;                                           } /* LD   I,A     */

OP(ed,48) { C = IN(BC); F = (F & CF) | SZP[C];                } /* IN   C,(C)   */
OP(ed,49) { OUT(BC, C);                                       } /* OUT  (C),C   */
OP(ed,4a) { ADC16( bc );                                      } /* ADC  HL,BC   */
OP(ed,4b) { EA = ARG16(); RM16( EA, &Z80.bc ); WZ = EA+1; } /* LD   BC,(w)  */
OP(ed,4c) { NEG;                                              } /* NEG          */
OP(ed,4d) { RETI;                                             } /* RETI         */
OP(ed,4e) { IM = 0;                                           } /* IM   0       */
OP(ed,4f) { LD_R_A;                                           } /* LD   R,A     */

OP(ed,50) { D = IN(BC); F = (F & CF) | SZP[D];                } /* IN   D,(C)   */
OP(ed,51) { OUT(BC, D);                                       } /* OUT  (C),D   */
OP(ed,52) { SBC16( de );                                      } /* SBC  HL,DE   */
OP(ed,53) { EA = ARG16(); WM16( EA, &Z80.de ); WZ = EA+1; } /* LD   (w),DE  */
OP(ed,54) { NEG;                                              } /* NEG          */
OP(ed,55) { RETN;                                             } /* RETN;        */
OP(ed,56) { IM = 1;                                           } /* IM   1       */
OP(ed,57) { LD_A_I;                                           } /* LD   A,I     */

OP(ed,58) { E = IN(BC); F = (F & CF) | SZP[E];                } /* IN   E,(C)   */
OP(ed,59) { OUT(BC, E);                                       } /* OUT  (C),E   */
OP(ed,5a) { ADC16( de );                                      } /* ADC  HL,DE   */
OP(ed,5b) { EA = ARG16(); RM16( EA, &Z80.de ); WZ = EA+1; } /* LD   DE,(w)  */
OP(ed,5c) { NEG;                                              } /* NEG          */
OP(ed,5d) { RETI;                                             } /* RETI         */
OP(ed,5e) { IM = 2;                                           } /* IM   2       */
OP(ed,5f) { LD_A_R;                                           } /* LD   A,R     */

OP(ed,60) { H = IN(BC); F = (F & CF) | SZP[H];                } /* IN   H,(C)   */
OP(ed,61) { OUT(BC, H);                                       } /* OUT  (C),H   */
OP(ed,62) { SBC16( hl );                                      } /* SBC  HL,HL   */
OP(ed,63) { EA = ARG16(); WM16( EA, &Z80.hl ); WZ = EA+1; } /* LD   (w),HL  */
OP(ed,64) { NEG;                                              } /* NEG          */
OP(ed,65) { RETN;                                             } /* RETN;        */
OP(ed,66) { IM = 0;                                           } /* IM   0       */
OP(ed,67) { RRD;                                              } /* RRD  (HL)    */

OP(ed,68) { L = IN(BC); F = (F & CF) | SZP[L];                } /* IN   L,(C)   */
OP(ed,69) { OUT(BC, L);                                       } /* OUT  (C),L   */
OP(ed,6a) { ADC16( hl );                                      } /* ADC  HL,HL   */
OP(ed,6b) { EA = ARG16(); RM16( EA, &Z80.hl ); WZ = EA+1; } /* LD   HL,(w)  */
OP(ed,6c) { NEG;                                              } /* NEG          */
OP(ed,6d) { RETI;                                             } /* RETI         */
OP(ed,6e) { IM = 0;                                           } /* IM   0       */
OP(ed,6f) { RLD;                                              } /* RLD  (HL)    */

OP(ed,70) { UINT8 res = IN(BC); F = (F & CF) | SZP[res];      } /* IN   0,(C)   */
OP(ed,71) { OUT(BC, 0);                                       } /* OUT  (C),0   */
OP(ed,72) { SBC16( sp );                                      } /* SBC  HL,SP   */
OP(ed,73) { EA = ARG16(); WM16( EA, &Z80.sp ); WZ = EA+1; } /* LD   (w),SP  */
OP(ed,74) { NEG;                                              } /* NEG          */
OP(ed,75) { RETN;                                             } /* RETN;        */
OP(ed,76) { IM = 1;                                           } /* IM   1       */
OP_ILLEGAL2(ed,77)                                              /* DB   ED,77   */

OP(ed,78) { A = IN(BC); F = (F & CF) | SZP[A]; WZ = BC+1;     } /* IN   E,(C)   */
OP(ed,79) { OUT(BC, A); WZ = BC + 1;                          } /* OUT  (C),A   */
OP(ed,7a) { ADC16( sp );                                      } /* ADC  HL,SP   */
OP(ed,7b) { EA = ARG16(); RM16( EA, &Z80.sp ); WZ = EA+1;     } /* LD   SP,(w)  */
OP(ed,7c) { NEG;                                              } /* NEG          */
OP(ed,7d) { RETI;                                             } /* RETI         */
OP(ed,7e) { IM = 2;                                           } /* IM   2       */
OP_ILLEGAL2(ed,7f)                                              /* DB   ED,7F   */

OP_ILLEGAL2(ed,80)                                              /* DB   ED      */
OP_ILLEGAL2(ed,81)                                              /* DB   ED      */
OP_ILLEGAL2(ed,82)                                              /* DB   ED      */
OP_ILLEGAL2(ed,83)                                              /* DB   ED      */
OP_ILLEGAL2(ed,84)                                              /* DB   ED      */
OP_ILLEGAL2(ed,85)                                              /* DB   ED      */
OP_ILLEGAL2(ed,86)                                              /* DB   ED      */
OP_ILLEGAL2(ed,87)                                              /* DB   ED      */

OP_ILLEGAL2(ed,88)                                              /* DB   ED      */
OP_ILLEGAL2(ed,89)                                              /* DB   ED      */
OP_ILLEGAL2(ed,8a)                                              /* DB   ED      */
OP_ILLEGAL2(ed,8b)                                              /* DB   ED      */
OP_ILLEGAL2(ed,8c)                                              /* DB   ED      */
OP_ILLEGAL2(ed,8d)                                              /* DB   ED      */
OP_ILLEGAL2(ed,8e)                                              /* DB   ED      */
OP_ILLEGAL2(ed,8f)                                              /* DB   ED      */

OP_ILLEGAL2(ed,90)                                              /* DB   ED      */
OP_ILLEGAL2(ed,91)                                              /* DB   ED      */
OP_ILLEGAL2(ed,92)                                              /* DB   ED      */
OP_ILLEGAL2(ed,93)                                              /* DB   ED      */
OP_ILLEGAL2(ed,94)                                              /* DB   ED      */
OP_ILLEGAL2(ed,95)                                              /* DB   ED      */
OP_ILLEGAL2(ed,96)                                              /* DB   ED      */
OP_ILLEGAL2(ed,97)                                              /* DB   ED      */

OP_ILLEGAL2(ed,98)                                              /* DB   ED      */
OP_ILLEGAL2(ed,99)                                              /* DB   ED      */
OP_ILLEGAL2(ed,9a)                                              /* DB   ED      */
OP_ILLEGAL2(ed,9b)                                              /* DB   ED      */
OP_ILLEGAL2(ed,9c)                                              /* DB   ED      */
OP_ILLEGAL2(ed,9d)                                              /* DB   ED      */
OP_ILLEGAL2(ed,9e)                                              /* DB   ED      */
OP_ILLEGAL2(ed,9f)                                              /* DB   ED      */

OP(ed,a0) { LDI;                                              } /* LDI          */
OP(ed,a1) { CPI;                                              } /* CPI          */
OP(ed,a2) { INI;                                              } /* INI          */
OP(ed,a3) { OUTI;                                             } /* OUTI         */
OP_ILLEGAL2(ed,a4)                                              /* DB   ED      */
OP_ILLEGAL2(ed,a5)                                              /* DB   ED      */
OP_ILLEGAL2(ed,a6)                                              /* DB   ED      */
OP_ILLEGAL2(ed,a7)                                              /* DB   ED      */

OP(ed,a8) { LDD;                                              } /* LDD          */
OP(ed,a9) { CPD;                                              } /* CPD          */
OP(ed,aa) { IND;                                              } /* IND          */
OP(ed,ab) { OUTD;                                             } /* OUTD         */
OP_ILLEGAL2(ed,ac)                                              /* DB   ED      */
OP_ILLEGAL2(ed,ad)                                              /* DB   ED      */
OP_ILLEGAL2(ed,ae)                                              /* DB   ED      */
OP_ILLEGAL2(ed,af)                                              /* DB   ED      */

OP(ed,b0) { LDIR;                                             } /* LDIR         */
OP(ed,b1) { CPIR;                                             } /* CPIR         */
OP(ed,b2) { INIR;                                             } /* INIR         */
OP(ed,b3) { OTIR;                                             } /* OTIR         */
OP_ILLEGAL2(ed,b4)                                              /* DB   ED      */
OP_ILLEGAL2(ed,b5)                                              /* DB   ED      */
OP_ILLEGAL2(ed,b6)                                              /* DB   ED      */
OP_ILLEGAL2(ed,b7)                                              /* DB   ED      */

OP(ed,b8) { LDDR;                                             } /* LDDR         */
OP(ed,b9) { CPDR;                                             } /* CPDR         */
OP(ed,ba) { INDR;                                             } /* INDR         */
OP(ed,bb) { OTDR;                                             } /* OTDR         */
OP_ILLEGAL2(ed,bc)                                              /* DB   ED      */
OP_ILLEGAL2(ed,bd)                                              /* DB   ED      */
OP_ILLEGAL2(ed,be)                                              /* DB   ED      */
OP_ILLEGAL2(ed,bf)                                              /* DB   ED      */

OP_ILLEGAL2(ed,c0)                                              /* DB   ED      */
OP_ILLEGAL2(ed,c1)                                              /* DB   ED      */
OP_ILLEGAL2(ed,c2)                                              /* DB   ED      */
OP_ILLEGAL2(ed,c3)                                              /* DB   ED      */
OP_ILLEGAL2(ed,c4)                                              /* DB   ED      */
OP_ILLEGAL2(ed,c5)                                              /* DB   ED      */
OP_ILLEGAL2(ed,c6)                                              /* DB   ED      */
OP_ILLEGAL2(ed,c7)                                              /* DB   ED      */

OP_ILLEGAL2(ed,c8)                                              /* DB   ED      */
OP_ILLEGAL2(ed,c9)                                              /* DB   ED      */
OP_ILLEGAL2(ed,ca)                                              /* DB   ED      */
OP_ILLEGAL2(ed,cb)                                              /* DB   ED      */
OP_ILLEGAL2(ed,cc)                                              /* DB   ED      */
OP_ILLEGAL2(ed,cd)                                              /* DB   ED      */
OP_ILLEGAL2(ed,ce)                                              /* DB   ED      */
OP_ILLEGAL2(ed,cf)                                              /* DB   ED      */

OP_ILLEGAL2(ed,d0)                                              /* DB   ED      */
OP_ILLEGAL2(ed,d1)                                              /* DB   ED      */
OP_ILLEGAL2(ed,d2)                                              /* DB   ED      */
OP_ILLEGAL2(ed,d3)                                              /* DB   ED      */
OP_ILLEGAL2(ed,d4)                                              /* DB   ED      */
OP_ILLEGAL2(ed,d5)                                              /* DB   ED      */
OP_ILLEGAL2(ed,d6)                                              /* DB   ED      */
OP_ILLEGAL2(ed,d7)                                              /* DB   ED      */

OP_ILLEGAL2(ed,d8)                                              /* DB   ED      */
OP_ILLEGAL2(ed,d9)                                              /* DB   ED      */
OP_ILLEGAL2(ed,da)                                              /* DB   ED      */
OP_ILLEGAL2(ed,db)                                              /* DB   ED      */
OP_ILLEGAL2(ed,dc)                                              /* DB   ED      */
OP_ILLEGAL2(ed,dd)                                              /* DB   ED      */
OP_ILLEGAL2(ed,de)                                              /* DB   ED      */
OP_ILLEGAL2(ed,df)                                              /* DB   ED      */

OP_ILLEGAL2(ed,e0)                                              /* DB   ED      */
OP_ILLEGAL2(ed,e1)                                              /* DB   ED      */
OP_ILLEGAL2(ed,e2)                                              /* DB   ED      */
OP_ILLEGAL2(ed,e3)                                              /* DB   ED      */
OP_ILLEGAL2(ed,e4)                                              /* DB   ED      */
OP_ILLEGAL2(ed,e5)                                              /* DB   ED      */
OP_ILLEGAL2(ed,e6)                                              /* DB   ED      */
OP_ILLEGAL2(ed,e7)                                              /* DB   ED      */

OP_ILLEGAL2(ed,e8)                                              /* DB   ED      */
OP_ILLEGAL2(ed,e9)                                              /* DB   ED      */
OP_ILLEGAL2(ed,ea)                                              /* DB   ED      */
OP_ILLEGAL2(ed,eb)                                              /* DB   ED      */
OP_ILLEGAL2(ed,ec)                                              /* DB   ED      */
OP_ILLEGAL2(ed,ed)                                              /* DB   ED      */
OP_ILLEGAL2(ed,ee)                                              /* DB   ED      */
OP_ILLEGAL2(ed,ef)                                              /* DB   ED      */

OP_ILLEGAL2(ed,f0)                                              /* DB   ED      */
OP_ILLEGAL2(ed,f1)                                              /* DB   ED      */
OP_ILLEGAL2(ed,f2)                                              /* DB   ED      */
OP_ILLEGAL2(ed,f3)                                              /* DB   ED      */
OP_ILLEGAL2(ed,f4)                                              /* DB   ED      */
OP_ILLEGAL2(ed,f5)                                              /* DB   ED      */
OP_ILLEGAL2(ed,f6)                                              /* DB   ED      */
OP_ILLEGAL2(ed,f7)                                              /* DB   ED      */

OP_ILLEGAL2(ed,f8)                                              /* DB   ED      */
OP_ILLEGAL2(ed,f9)                                              /* DB   ED      */
OP_ILLEGAL2(ed,fa)                                              /* DB   ED      */
OP_ILLEGAL2(ed,fb)                                              /* DB   ED      */
OP_ILLEGAL2(ed,fc)                                              /* DB   ED      */
OP_ILLEGAL2(ed,fd)                                              /* DB   ED      */
OP_ILLEGAL2(ed,fe)                                              /* DB   ED      */
OP_ILLEGAL2(ed,ff)                                              /* DB   ED      */

static inline void take_interrupt(void)
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
  if( IM == 2 )
  {
    irq_vector = (irq_vector & 0xff) | (I << 8);
    PUSH( pc );
    RM16( irq_vector, &Z80.pc );
    LOG(("Z80 #%d IM2 [$%04x] = $%04x\n",cpu_getactivecpu() , irq_vector, PCD));
      /* CALL $xxxx + 'interrupt latency' cycles */
    z80_ICount -= cc[Z80_TABLE_op][0xcd] + cc[Z80_TABLE_ex][0xff];
  }
  else
    /* Interrupt mode 1. RST 38h */
    if( IM == 1 )
    {
      LOG(("Z80 #%d IM1 $0038\n",cpu_getactivecpu() ));
      PUSH( pc );
      PCD = 0x0038;
        /* RST $38 + 'interrupt latency' cycles */
      z80_ICount -= cc[Z80_TABLE_op][0xff] + cc[Z80_TABLE_ex][0xff];
    }
    else
    {
      /* Interrupt mode 0. We check for CALL and JP instructions, */
      /* if neither of these were found we assume a 1 byte opcode */
      /* was placed on the databus                */
      LOG(("Z80 #%d IM0 $%04x\n",cpu_getactivecpu() , irq_vector));
      switch (irq_vector & 0xff0000)
      {
        case 0xcd0000:  /* call */
        PUSH( pc );
        PCD = irq_vector & 0xffff;
           /* CALL $xxxx + 'interrupt latency' cycles */
        z80_ICount -= cc[Z80_TABLE_op][0xcd] + cc[Z80_TABLE_ex][0xff];
          break;
        case 0xc30000:  /* jump */
        PCD = irq_vector & 0xffff;
          /* JP $xxxx + 2 cycles */
        z80_ICount -= cc[Z80_TABLE_op][0xc3] + cc[Z80_TABLE_ex][0xff];
          break;
        default:    /* rst (or other opcodes?) */
        PUSH( pc );
        PCD = irq_vector & 0x0038;
          /* RST $xx + 2 cycles */
        z80_ICount -= cc[Z80_TABLE_op][0xff] + cc[Z80_TABLE_ex][0xff];
          break;
      }
    }
  WZ=PCD;
}

/****************************************************************************
 * Processor initialization
 ****************************************************************************/
void z80_init(int index, int clock, const void *config, int (*irqcallback)(int))
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
    SZ[i] = i ? i & SF : ZF;
    SZ[i] |= (i & (YF | XF));    /* undocumented flag bits 5+3 */
    SZ_BIT[i] = i ? i & SF : ZF | PF;
    SZ_BIT[i] |= (i & (YF | XF));  /* undocumented flag bits 5+3 */
    SZP[i] = SZ[i] | ((p & 1) ? 0 : PF);
    SZHV_inc[i] = SZ[i];
    if( i == 0x80 ) SZHV_inc[i] |= VF;
    if( (i & 0x0f) == 0x00 ) SZHV_inc[i] |= HF;
    SZHV_dec[i] = SZ[i] | NF;
    if( i == 0x7f ) SZHV_dec[i] |= VF;
    if( (i & 0x0f) == 0x0f ) SZHV_dec[i] |= HF;
  }

  /* Reset registers to their initial values */
  memset(&Z80, 0, sizeof(Z80));
  IX = IY = 0xffff; /* IX and IY are FFFF after a reset! */
  F = ZF;      /* Zero flag is set */
  SP = 0xdff0; /* fix Shadow Dancer & Ace of Aces (normally set by BIOS) */
  Z80.daisy = config;
  Z80.irq_callback = irqcallback;
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
  Z80.after_ei = FALSE;
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
int z80_execute(int cycles)
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
    Z80.nmi_pending = FALSE;
  }

  while( z80_ICount > 0 )
  {
    /* check for IRQs before each instruction */
    if (Z80.irq_state != CLEAR_LINE && IFF1 && !Z80.after_ei)
      take_interrupt();
    Z80.after_ei = FALSE;

    if (z80_ICount > 0)
    {
      R++;
      EXEC_INLINE(op,ROP());
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
      Z80.nmi_pending = TRUE;
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
