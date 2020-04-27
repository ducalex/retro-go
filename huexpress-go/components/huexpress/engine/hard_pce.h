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

	// SPRAM = sprite RAM
	// The pc engine got a function to transfert a piece VRAM toward the inner
	// gfx cpu sprite memory from where data will be grabbed to render sprites
	uint16 SPRAM[64 * 4];

	// PCE->PC Palette convetion array
	// Each of the 512 available PCE colors (333 RGB -> 512 colors)
	// got a correspondancy in the 256 fixed colors palette
	uchar Palette[512];

	// CPU Registers
	uint16 reg_pc;
	uchar reg_a;
	uchar reg_x;
	uchar reg_y;
	uchar reg_p;
	uchar reg_s;

	// The current rendered line on screen
	uint32 Scanline;

	// Total number of elapsed cycles in the current scanline
	uint32 Cycles;

	// Total number of elapsed cycles
	uint32 TotalCycles;

	// Previous number of elapsed cycles
	uint32 PrevTotalCycles;

	// Value of each of the MMR registers
	uchar MMR[8];

	// IO Registers
	IO io;

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

extern struct_hard_pce PCE;
// The global structure for all hardware variables

extern uchar *IOAREA;
// physical address on emulator machine of the IO area (fake address as it has to be handled specially)

extern uchar TRAPRAM[0x2004];
// False "ram"s in which you can read/write (to homogeneize writes into RAM, BRAM, ... as well as in rom) but the result isn't coherent

extern uchar *PageR[8];
extern uchar *ROMMapR[256];

extern uchar *PageW[8];
extern uchar *ROMMapW[256];
// physical address on emulator machine of each of the 256 banks

#define RAM PCE.RAM
#define SaveRAM PCE.SaveRAM
#define SuperRAM PCE.SuperRAM
#define ExtraRAM PCE.ExtraRAM
#define SPRAM PCE.SPRAM
#define VRAM PCE.VRAM
#define Scanline PCE.Scanline
#define Palette PCE.Palette
#define Cycles PCE.Cycles
#define TotalCycles PCE.TotalCycles
#define PrevTotalCycles PCE.PrevTotalCycles
#define MMR PCE.MMR
#define io PCE.io

#define TimerPeriod 1097
// Base period for the timer

#define reg_pc PCE.reg_pc
#define reg_a  PCE.reg_a
#define reg_x  PCE.reg_x
#define reg_y  PCE.reg_y
#define reg_p  PCE.reg_p
#define reg_s  PCE.reg_s
// H6280 CPU registers

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
