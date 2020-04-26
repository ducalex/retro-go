//  h6280.c - Execute CPU instructions
//

#include <stdio.h>
#include <stdlib.h>

#include "interupt.h"
#include "hard_pce.h"
#include "gfx.h"
#include "pce.h"
#include "utils.h"

#define MY_INLINE_h6280_opcodes

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

// Addressing modes:
#define imm_operand(addr) ((uchar) (PageR[(addr) >> 13][(addr)]))
#define abs_operand(x)     get_8bit_addr(get_16bit_addr(x))
#define absx_operand(x)    get_8bit_addr(get_16bit_addr(x)+reg_x)
#define absy_operand(x)    get_8bit_addr(get_16bit_addr(x)+reg_y)
#define zp_operand(x)      get_8bit_zp(imm_operand(x))
#define zpx_operand(x)     get_8bit_zp(imm_operand(x)+reg_x)
#define zpy_operand(x)     get_8bit_zp(imm_operand(x)+reg_y)
#define zpind_operand(x)   get_8bit_addr(get_16bit_zp(imm_operand(x)))
#define zpindx_operand(x)  get_8bit_addr(get_16bit_zp(imm_operand(x)+reg_x))
#define zpindy_operand(x)  get_8bit_addr(get_16bit_zp(imm_operand(x))+reg_y)

// Flag check (flags 'N' and 'Z'):
#define chk_flnz_8bit(x) reg_p = ((reg_p & (~(FL_N|FL_T|FL_Z))) | flnz_list_get(x));

// Memory access
#define get_8bit_zp(zp_addr) (*(zp_base + (zp_addr)))
#define put_8bit_zp(zp_addr, byte) (*(zp_base + (zp_addr)) = (byte))
#define push_8bit(byte) (*(sp_base + reg_s--) = (byte))
#define pull_8bit() (*(sp_base + ++reg_s))

static inline uint16
get_16bit_zp(uchar zp_addr)
{
	uint16 n = *(zp_base + zp_addr);
	n += (*(zp_base + (uchar) (zp_addr + 1)) << 8);
	return (n);
}

static inline void
push_16bit(uint16 addr)
{
	*(sp_base + reg_s--) = (uchar) (addr >> 8);
	*(sp_base + reg_s--) = (uchar) (addr & 0xFF);
}

static inline uint16
pull_16bit(void)
{
	uint16 n = (uchar) *(sp_base + ++reg_s);
	n += (uint16) (((uchar) *(sp_base + ++reg_s)) << 8);
	return (n);
}

static inline uint16
get_16bit_addr(uint16 addr)
{
	register unsigned int memreg = addr >> 13;

	uint16 ret_16bit = (uchar) PageR[memreg][addr];
	memreg = (++addr) >> 13;
	ret_16bit += (uint16) ((uchar) PageR[memreg][addr] << 8);

	return (ret_16bit);
}


#include "h6280_opcodes.h"


#ifdef MY_INLINE_h6280_opcodes
#include "h6280_opcodes_define.h"
#endif



/*@ =type */
#if 0
// Execute a single instruction :
void
exe_instruct(void)
{
	(*optable_runtime[PageR[reg_pc >> 13][reg_pc]].func_exe) ();
}
#endif

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

    uint32 cycles = 0;
	cyclecount = 0;

// Slower!
//#undef reg_pc
  //  WORD_ALIGNED_ATTR uint16 reg_pc = reg_pc_;

//#undef reg_a
 //   uchar reg_a = 0;

    while (true) {
		while (cycles <= 455)
		{
			// (*optable_runtime[PageR[reg_pc >> 13][reg_pc]].func_exe) ();
			#include "h6280_instr_switch.h"
		}

        // HSYNC stuff - count cycles:
        /*if (cycles > 455) */ {
            cyclecount += cycles;
            // cycles -= 455;
            // scanline++;

            // Log("Calling periodic handler\n");

            I = Loop6502();     /* Call the periodic handler */

            // _ICount += _IPeriod;
            /* Reset the cycle counter */
            cycles = 0;

            // Log("Requested interrupt is %d\n",I);

            /* if (I == INT_QUIT) return; */

            if (I)
                Int6502(I);     /* Interrupt if needed  */

            if ((uint) (cyclecount - cyclecountold) > (uint) TimerPeriod * 2)
				cyclecountold = cyclecount;

        } /*else*/ {
            if (cyclecount - cyclecountold >= TimerPeriod) {
                cyclecountold += TimerPeriod;
                if ((I = IO_TimerInt()))
                    Int6502(I);
            }
        }
    }
    MESSAGE_ERROR("Abnormal exit from the cpu loop\n");
}
