/***************************************************************************/
/*                                                                         */
/*                         HARDware PCEngine                               */
/*                                                                         */
/* This header deals with definition of structure and functions used to    */
/* handle the pc engine hardware in itself (RAM, IO ports, ...) which were */
/* previously found in pce.h                                               */
/*                                                                         */
/***************************************************************************/
#ifndef _INCLUDE_HARD_PCE_H
#define _INCLUDE_HARD_PCE_H

#include <stdio.h>

#include "config.h"
#include "cleantypes.h"

#define RAMSIZE            0x2000
#define VRAMSIZE           0x10000

#define PSG_DIRECT_ACCESS_BUFSIZE 1024

#define PSG_VOICE_REG           0	/* voice index */

#define PSG_VOLUME_REG          1	/* master volume */

#define PSG_FREQ_LSB_REG        2	/* lower 8 bits of 12 bit frequency */

#define PSG_FREQ_MSB_REG        3	/* actually most significant nibble */

#define PSG_DDA_REG             4
#define PSG_DDA_ENABLE          0x80	/* bit 7 */
#define PSG_DDA_DIRECT_ACCESS   0x40	/* bit 6 */
#define PSG_DDA_VOICE_VOLUME    0x1F	/* bits 0-4 */

#define PSG_BALANCE_REG         5
#define PSG_BALANCE_LEFT        0xF0	/* bits 4-7 */
#define PSG_BALANCE_RIGHT       0x0F	/* bits 0-3 */

#define PSG_DATA_INDEX_REG      6

#define PSG_NOISE_REG           7
#define PSG_NOISE_ENABLE        0x80	/* bit 7 */

typedef union {
	struct {
#if defined(WORDS_BIGENDIAN)
		uchar h, l;
#else
		uchar l, h;
#endif
	} B;
	uint16 W;
} Word;

/* The structure containing all variables relatives to Input and Output */
typedef struct {
	/* VCE */
	Word VCE[0x200];			/* palette info */
	Word vce_reg;				/* currently selected color */
	uchar vce_ratch;			/* temporary value to keep track of the first byte
								 * when setting a 16 bits value with two byte access
								 */
	/* VDC */
	Word VDC[32];				/* value of each VDC register */
	uint16 vdc_inc;				/* VRAM pointer increment once accessed */
	uint16 vdc_raster_count;	/* unused as far as I know */
	uchar vdc_reg;				/* currently selected VDC register */
	uchar vdc_status;			/* current VCD status (end of line, end of screen, ...) */
	uchar vdc_ratch;			/* temporary value to keep track of the first byte
								 * when setting a 16 bits value with two byte access
								 */
	uchar vdc_satb;				/* boolean which keeps track of the need to copy
								 * the SATB from VRAM to internal SATB
								 */
	uchar vdc_pendvsync;		/* unsure, set if a end of screen IRQ is waiting */
	int32 bg_h;					/* number of tiles vertically in virtual screen */
	int32 bg_w;					/* number of tiles horizontaly in virtual screen */
	int32 screen_w;				/* size of real screen in pixels */
	int32 screen_h;				/* size of real screen in pixels */
	int32 scroll_y;
	int32 minline;
	int32 maxline;

	uint16 vdc_min_display;		// First scanline of active display
	uint16 vdc_max_display;		// Last scanline of active display

	/* joypad */
	uchar JOY[16];				/* value of pressed button/direct for each pad
								 * (why 16 ? 5 should be enough for everyone :)
								 */
	uchar joy_select;			/* used to know what nibble we must return */
	uchar joy_counter;			/* current addressed joypad */

	/* PSG */
	uchar PSG[6][8], wave[6][32];
	// PSG STRUCTURE
	// 0 : dda_out
	// 2 : freq (lo byte)  | In reality it's a divisor
	// 3 : freq (hi byte)  | 3.7 Mhz / freq => true snd freq
	// 4 : dda_ctrl
	//     000XXXXX
	//     ^^  ^
	//     ||  ch. volume
	//     ||
	//     |direct access (everything at byte 0)
	//     |
	//    enable
	// 5 : pan (left vol = hi nibble, right vol = low nibble)
	// 6 : wave ringbuffer index
	// 7 : noise data for channels 5 and 6

	uchar psg_ch, psg_volume, psg_lfo_freq, psg_lfo_ctrl;

	uchar psg_da_data[6][PSG_DIRECT_ACCESS_BUFSIZE];
	uint16 psg_da_index[6], psg_da_count[6];
	int psg_channel_disabled[6];

	/* TIMER */
	uchar timer_reload, timer_start, timer_counter;

	/* IRQ */
	uchar irq_mask, irq_status;

	/* Remanence latch */
	uchar io_buffer;

} IO;

typedef struct {
	uchar RAM[RAMSIZE];

	// Extra RAM for SuperGraphx
	uchar *SuperRAM; // 0x6000

	// Extra RAM contained on the HuCard (Populous)
	uchar *ExtraRAM;

	// Backup RAM
	uchar SaveRAM[0x800];

	// Video mem
	// 0x10000 bytes on coregraphx, the double on supergraphx I think
	// contain information about the sprites position/status, information
	// about the pattern and palette to use for each tile, and patterns
	// for use in sprite/tile rendering
	uchar VRAM[VRAMSIZE];

	// These are array to keep in memory the result of the linearisation of
	// PCE sprites and tiles
	uchar *VRAM2; // [VRAMSIZE];
	uchar *VRAMS; // [VRAMSIZE];

	// These array are boolean array to know if we must update the
	// corresponding linear sprite representation in VRAM2 and VRAMS or not
	// if (sprite_converted[5] == 0) 6th pattern in VRAM2 must be updated
	bool plane_converted[VRAMSIZE / 32];
	bool sprite_converted[VRAMSIZE / 128];

	// SPRAM = sprite RAM
	// The pc engine got a function to transfert a piece VRAM toward the inner
	// gfx cpu sprite memory from where data will be grabbed to render sprites
	uint16 SPRAM[64 * 4];

	// PCE->PC Palette convetion array
	// Each of the 512 available PCE colors (333 RGB -> 512 colors)
	// got a correspondancy in the 256 fixed colors palette
	uchar Pal[512];

	// CPU Registers
	uint16 s_reg_pc;
	uchar s_reg_a;
	uchar s_reg_x;
	uchar s_reg_y;
	uchar s_reg_p;
	uchar s_reg_s;

	// The current rendered line on screen
	uint32 s_scanline;

	// Number of elapsed cycles
	uint32 s_cyclecount;

	// Previous number of elapsed cycles
	uint32 s_cyclecountold;

	// Number of pc engine cycles elapsed since the resetting of the emulated console
	uint32 s_cycles;

	// Value of each of the MMR registers
	uchar mmr[8];

	IO s_io;

	int32 s_external_control_cpu;

} struct_hard_pce;

/**
  * Exported functions to access hardware
  **/

void hard_init(void);
void hard_reset(void);
void hard_term(void);

void IO_write(uint16 A, uchar V);
uchar IO_read(uint16 A);
uchar TimerInt();
void bank_set(uchar P, uchar V);

void dump_pce_cpu_environment();

/**
  * Exported variables
  **/

extern struct_hard_pce hard_pce;
// The global structure for all hardware variables

#define zp_base RAM
// pointer to the beginning of the Zero Page area

#define sp_base (RAM + 0x100)
// pointer to the beginning of the Stack Area

extern uchar *IOAREA;
// physical address on emulator machine of the IO area (fake address as it has to be handled specially)

extern uchar *TRAPRAM;
// False "ram"s in which you can read/write (to homogeneize writes into RAM, BRAM, ... as well as in rom) but the result isn't coherent

extern uchar *PageR[8];
extern uchar *ROMMapR[256];

extern uchar *PageW[8];
extern uchar *ROMMapW[256];
// physical address on emulator machine of each of the 256 banks

#define RAM   hard_pce.RAM
#define SaveRAM  hard_pce.SaveRAM
#define SuperRAM hard_pce.SuperRAM
#define ExtraRAM hard_pce.ExtraRAM
#define SPRAM hard_pce.SPRAM
#define VRAM hard_pce.VRAM
#define VRAM2 hard_pce.VRAM2
#define VRAMS hard_pce.VRAMS
#define plane_converted hard_pce.plane_converted
#define sprite_converted hard_pce.sprite_converted
#define Palette hard_pce.Pal

#define mmr hard_pce.mmr
#define scanline hard_pce.s_scanline
#define io hard_pce.s_io
#define cyclecount hard_pce.s_cyclecount
#define cyclecountold hard_pce.s_cyclecountold

#define TimerPeriod 1097
// Base period for the timer

// registers:
#define reg_pc hard_pce.s_reg_pc
#define reg_a  hard_pce.s_reg_a
#define reg_x  hard_pce.s_reg_x
#define reg_y  hard_pce.s_reg_y
#define reg_p  hard_pce.s_reg_p
#define reg_s  hard_pce.s_reg_s
// These are the main h6280 register, reg_p is the flag register


/**
  * Definitions to ease writing
  **/

#define	VRR	2
enum _VDC_REG {
	MAWR,						/*  0 *//* Memory Address Write Register */
	MARR,						/*  1 *//* Memory Adress Read Register */
	VWR,						/*  2 *//* VRAM Read Register / VRAM Write Register */
	vdc3,						/*  3 */
	vdc4,						/*  4 */
	CR,							/*  5 *//* Control Register */
	RCR,						/*  6 *//* Raster Compare Register */
	BXR,						/*  7 *//* Horizontal scroll offset */
	BYR,						/*  8 *//* Vertical scroll offset */
	MWR,						/*  9 *//* Memory Width Register */
	HSR,						/*  A *//* Unknown, other horizontal definition */
	HDR,						/*  B *//* Horizontal Definition */
	VPR,						/*  C *//* Higher byte = VDS, lower byte = VSW */
	VDW,						/*  D *//* Vertical Definition */
	VCR,						/*  E *//* Vertical counter between restarting of display */
	DCR,						/*  F *//* DMA Control */
	SOUR,						/* 10 *//* Source Address of DMA transfert */
	DISTR,						/* 11 *//* Destination Address of DMA transfert */
	LENR,						/* 12 *//* Length of DMA transfert */
	SATB						/* 13 *//* Adress of SATB */
};

#define	NODATA	   0xff
#define	ENABLE	   1
#define	DISABLE	   0

static inline uchar
Read8(uint16 addr)
{
	register uchar *page = PageR[addr >> 13];

	if (page == IOAREA)
		return IO_read(addr);
	else
		return page[addr];
}

static inline void
Write8(uint16 addr, uchar byte)
{
	register uchar *page = PageW[addr >> 13];

	if (page == IOAREA)
		IO_write(addr, byte);
	else
		page[addr] = byte;
}

static inline uint16
Read16(uint16 addr)
{
	register unsigned int memreg = addr >> 13;

	uint16 ret_16bit = PageR[memreg][addr];
	memreg = (++addr) >> 13;
	ret_16bit += (uint16) (PageR[memreg][addr] << 8);

	return (ret_16bit);
}

static inline void
Write16(uint16 addr, uint16 word)
{

}

#endif
