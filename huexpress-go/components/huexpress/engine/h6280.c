//  h6280.c - Execute CPU instructions
//

#include <stdio.h>
#include <stdlib.h>

#include "interupt.h"
#include "dis_runtime.h"
#include "pce.h"
#include "hard_pce.h"
#include "gfx.h"
#include "pce.h"
#include "utils.h"
#include "myadd.h"

#ifdef MY_INLINE
#define imm_operand(addr) ((uchar) (PageR[(unsigned short int)(addr >> 13)][addr]))
#define get_8bit_addr(addr) get_8bit_addr_(addr)

//#define put_8bit_addr(addr, byte) put_8bit_addr_(addr, byte)
#define put_8bit_addr(addr_, byte_) { \
    uchar byte = byte_; \
    uint16 addr = addr_; \
    register unsigned int memreg = addr >> 13; \
    if (PageW[memreg] == IOAREA) { \
        IO_write(addr, byte); \
    } else { \
        PageW[memreg][addr] = byte; \
    } }

#define get_16bit_addr(addr) get_16bit_addr_(addr)

#define get_8bit_zp(zp_addr) ((uchar) * (zp_base + zp_addr))
#define get_16bit_zp(zp_addr) get_16bit_zp_(zp_addr)
#define put_8bit_zp(zp_addr, byte) *(zp_base + zp_addr) = byte
#define push_8bit(byte)  *(sp_base + reg_s--) = byte;

#define pull_8bit() ((uchar) * (sp_base + ++reg_s))
#define push_16bit(addr) { \
   *(sp_base + reg_s--) = (uchar) (addr >> 8); \
   *(sp_base + reg_s--) = (uchar) (addr & 0xFF); }
#define pull_16bit() pull_16bit_()
#else
#define imm_operand(addr) imm_operand_(addr)
#define get_8bit_addr(addr) get_8bit_addr_(addr)
#define put_8bit_addr(addr, byte) put_8bit_addr_(addr, byte)
#define get_16bit_addr(addr) get_16bit_addr_(addr)

#define get_8bit_zp(zp_addr) get_8bit_zp_(zp_addr)
#define get_16bit_zp(zp_addr) get_16bit_zp_(zp_addr)
#define put_8bit_zp(zp_addr, byte) put_8bit_zp_(zp_addr, byte)
#define push_8bit(byte)  push_8bit_(byte)

#define pull_8bit() pull_8bit_()
#define push_16bit(addr) push_16bit_(addr)
#define pull_16bit() pull_16bit_()

#endif


#if defined(KERNEL_DEBUG)
static
	int
one_bit_set(uchar arg)
{
	return (arg != 0) && ((-arg & arg) == arg);
}
#endif

// flag-value table (for speed)
uchar flnz_list[256] = {
	FL_Z, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 00-0F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 40-4F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 70-7F
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// 80-87
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// 90-97
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// A0-A7
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// B0-B7
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// C0-C7
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// D0-D7
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// E0-E7
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N,	// F0-F7
	FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N, FL_N
};

// Elementary operations:
// - defined here for clarity, brevity,
//   and reduced typographical errors

// This code ignores hardware-segment accesses; it should only be used
// to access immediate data (hardware segment does not run code):
//
//  as a function:

inline uchar
imm_operand_(uint16 addr)
{
	register unsigned short int memreg = addr >> 13;
	return ((uchar) (PageR[memreg][addr]));
}

// This is the more generalized access routine:
inline uchar
get_8bit_addr_(uint16 addr)
{
	register unsigned short int memreg = addr >> 13;

	if (PageR[memreg] == IOAREA)
		return (IO_read(addr));
	else
		return ((uchar) (PageR[memreg][addr]));
}

inline void
put_8bit_addr_(uint16 addr, uchar byte)
{
	register unsigned int memreg = addr >> 13;

	if (PageW[memreg] == IOAREA) {
		IO_write(addr, byte);
	} else {
		PageW[memreg][addr] = byte;
		// printf("WRITE: %d: 0x%04X := 0x%02X\n", memreg, addr, byte);
	}
}

inline uint16
get_16bit_addr_(uint16 addr)
{
	register unsigned int memreg = addr >> 13;
	uint16 ret_16bit = (uchar) PageR[memreg][addr];
	memreg = (++addr) >> 13;
	ret_16bit += (uint16) ((uchar) PageR[memreg][addr] << 8);

	return (ret_16bit);
}


//Addressing modes:

#define abs_operand(x)     get_8bit_addr(get_16bit_addr(x))
#define absx_operand(x)    get_8bit_addr(get_16bit_addr(x)+reg_x)
#define absy_operand(x)    get_8bit_addr(get_16bit_addr(x)+reg_y)
#define zp_operand(x)      get_8bit_zp(imm_operand(x))
#define zpx_operand(x)     get_8bit_zp(imm_operand(x)+reg_x)
#define zpy_operand(x)     get_8bit_zp(imm_operand(x)+reg_y)
#define zpind_operand(x)   get_8bit_addr(get_16bit_zp(imm_operand(x)))
#define zpindx_operand(x)  get_8bit_addr(get_16bit_zp(imm_operand(x)+reg_x))
#define zpindy_operand(x)  get_8bit_addr(get_16bit_zp(imm_operand(x))+reg_y)

// Elementary flag check (flags 'N' and 'Z'):

#define chk_flnz_8bit(x) reg_p = ((reg_p & (~(FL_N|FL_T|FL_Z))) | flnz_list_get(x));

inline uchar
get_8bit_zp_(uchar zp_addr)
{
	return ((uchar) * (zp_base + zp_addr));
}

inline uint16
get_16bit_zp_(uchar zp_addr)
{
	uint16 n = *(zp_base + zp_addr);
	n += (*(zp_base + (uchar) (zp_addr + 1)) << 8);
	return (n);
}


inline void
put_8bit_zp_(uchar zp_addr, uchar byte)
{
	*(zp_base + zp_addr) = byte;
}

inline void
push_8bit_(uchar byte)
{
	*(sp_base + reg_s--) = byte;
}

inline uchar
pull_8bit_(void)
{
	return ((uchar) * (sp_base + ++reg_s));
}

inline void
push_16bit_(uint16 addr)
{
	*(sp_base + reg_s--) = (uchar) (addr >> 8);
	*(sp_base + reg_s--) = (uchar) (addr & 0xFF);
	return;
}

inline uint16
pull_16bit_(void)
{
	uint16 n = (uchar) * (sp_base + ++reg_s);
	n += (uint16) (((uchar) * (sp_base + ++reg_s)) << 8);
	return (n);
}

#include "h6280_opcodes_define.h"
#include "h6280_opcodes_func.h"

/*@ =type */
#if 0
// Execute a single instruction :
void
exe_instruct(void)
{
	(*optable_runtime[PageR[reg_pc >> 13][reg_pc]].func_exe) ();
}
#endif

#ifdef MY_INLINE
#define Int6502(Type)                                       \
{                                                           \
    uint16 J;                                               \
    if ((Type == INT_NMI) || (!(reg_p & FL_I))) {           \
        cycles += 7;                                        \
        push_16bit(reg_pc);                                 \
        push_8bit(reg_p);                                   \
        reg_p = (reg_p & ~FL_D);                            \
        if (Type == INT_NMI) {                              \
            J = VEC_NMI;                                    \
        } else {                                            \
            reg_p |= FL_I;                                  \
            switch (Type) {                                 \
            case INT_IRQ:                                   \
                J = VEC_IRQ;                                \
                break;                                      \
            case INT_IRQ2:                                  \
                J = VEC_IRQ2;                               \
                break;                                      \
            case INT_TIMER:                                 \
                J = VEC_TIMER;                              \
                break;                                      \
            }                                               \
        }                                                   \
        reg_pc = get_16bit_addr((uint16) J);                \
    }                                                       \
}
#else
inline void
Int6502(uchar Type)
{
	uint16 J;

	if ((Type == INT_NMI) || (!(reg_p & FL_I))) {
		cycles += 7;
		push_16bit(reg_pc);
		push_8bit(reg_p);
		reg_p = (reg_p & ~FL_D);
/*
      WrRAM (SP + R->S,
	     ((R->P & ~(B_FLAG | T_FLAG)) & ~(N_FLAG | V_FLAG | Z_FLAG)) |
	     (R->NF & N_FLAG) | (R->VF & V_FLAG) | (R->ZF ? 0 : Z_FLAG));
*/

		if (Type == INT_NMI) {
			J = VEC_NMI;
		} else {
			reg_p |= FL_I;

			switch (Type) {

			case INT_IRQ:
				J = VEC_IRQ;
				break;

			case INT_IRQ2:
				J = VEC_IRQ2;
				break;

			case INT_TIMER:
				J = VEC_TIMER;
				break;

			}

		}
#ifdef KERNEL_DEBUG
		Log("Interruption : %04X\n", J);
#endif
		reg_pc = get_16bit_addr((uint16) J);
	} else {
#ifdef KERNEL_DEBUG
		Log("Dropped interruption %02X\n", Type);
#endif
	}
}
#endif

//! Log all needed info to guess what went wrong in the cpu
void
dump_pce_core()
{

	int i;

	fprintf(stderr, "Dumping PCE core\n");

	Log("PC = 0x%04x\n", reg_pc);
	Log("A = 0x%02x\n", reg_a);
	Log("X = 0x%02x\n", reg_x);
	Log("Y = 0x%02x\n", reg_y);
	Log("P = 0x%02x\n", reg_p);
	Log("S = 0x%02x\n", reg_s);

	for (i = 0; i < 8; i++) {
		Log("MMR[%d] = 0x%02x\n", i, mmr[i]);
	}

	for (i = 0x2000; i < 0xFFFF; i++) {

		if ((i & 0xF) == 0) {
			Log("%04X: ", i);
		}

		Log("%02x ", get_8bit_addr((uint16) i));
		if ((i & 0xF) == 0xF) {
			Log("\n");
		}
		if ((i & 0x1FFF) == 0x1FFF) {
			Log("\n-------------------------------------------------------------\n");
		}
	}

}


// Execute instructions as a machine would, including all
// important (known) interrupts, hardware functions, and
// actual video display on the hardware
//
// Until the following happens:
// (1) An unknown instruction is to be executed
// (2) An unknown hardware access is performed
// (3) <ESC> key is hit

void
exe_go(void)
{
    gfx_init();
    uchar I;

    uint32 CycleNew = 0;
    uint32 cycles = 0;

// Slower!
//#undef reg_pc
  //  WORD_ALIGNED_ATTR uint16 reg_pc = reg_pc_;

//#undef reg_a
 //   uchar reg_a = 0;

    while (true) {
      ODROID_DEBUG_PERF_START2(debug_perf_total)

      ODROID_DEBUG_PERF_START2(debug_perf_part1)
      while (cycles<=455)
      {
#ifdef USE_INSTR_SWITCH
        #include "h6280_instr_switch.h"
#else
        /*err =*/ (*optable_runtime[PageR[reg_pc >> 13][reg_pc]].func_exe) ();
#endif
      }

      ODROID_DEBUG_PERF_INCR2(debug_perf_part1, ODROID_DEBUG_PERF_CPU)

        // HSYNC stuff - count cycles:
        /*if (cycles > 455) */ {
            CycleNew += cycles;
            // cycles -= 455;
            // scanline++;

            // Log("Calling periodic handler\n");

            I = Loop6502();     /* Call the periodic handler */

            ODROID_DEBUG_PERF_START2(debug_perf_int)
            // _ICount += _IPeriod;
            /* Reset the cycle counter */
            cycles = 0;

            // Log("Requested interrupt is %d\n",I);

            /* if (I == INT_QUIT) return; */

            if (I)
                Int6502(I);     /* Interrupt if needed  */

            if ((unsigned int) (CycleNew - cyclecountold) >
                (unsigned int) TimerPeriod * 2) cyclecountold = CycleNew;

            ODROID_DEBUG_PERF_INCR2(debug_perf_int, ODROID_DEBUG_PERF_INT)
        } /*else*/ {
            ODROID_DEBUG_PERF_START2(debug_perf_int2)
            if (CycleNew - cyclecountold >= TimerPeriod) {
                cyclecountold += TimerPeriod;
#ifdef MY_INLINE
                if (io.timer_start) {
                    io.timer_counter--;
                    if (io.timer_counter > 128) {
                        io.timer_counter = io.timer_reload;
                        if (!(io.irq_mask & TIRQ)) {
                            io.irq_status |= TIRQ;
                            I = INT_TIMER;
                        } else I = INT_NONE;
                    } else I = INT_NONE;
                } else I = INT_NONE;
#else
                I = TimerInt();
#endif
                if (I)
                    Int6502(I);
            }
            ODROID_DEBUG_PERF_INCR2(debug_perf_int2, ODROID_DEBUG_PERF_INT2)
        }
        ODROID_DEBUG_PERF_INCR2(debug_perf_total, ODROID_DEBUG_PERF_TOTAL)
    }
    MESSAGE_ERROR("Abnormal exit from the cpu loop\n");
}
