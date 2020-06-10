#ifndef _INCLUDE_HARD_PCE_H
#define _INCLUDE_HARD_PCE_H

#include "config.h"
#include "stddef.h"
#include "cleantypes.h"

#define RAMSIZE            0x2000
#define VRAMSIZE           0x10000
#define BRAMSIZE           0x800
#define PSG_DA_BUFSIZE     1024
#define PSG_CHANNELS       6

typedef union {
	struct {
#if defined(WORDS_BIGENDIAN)
		uchar h, l;
#else
		uchar l, h;
#endif
	} B;
	uint16 W;
} UWord;

/* The structure containing all variables relatives to Input and Output */
typedef struct {
	/* VCE */
	UWord VCE[0x200];			/* palette info */
	UWord vce_reg;				/* currently selected color */

	/* VDC */
	UWord VDC[32];				/* value of each VDC register */
	uchar vdc_inc;				/* VRAM pointer increment once accessed */
	uchar vdc_reg;				/* currently selected VDC register */
	uchar vdc_status;			/* current VCD status (end of line, end of screen, ...) */
	uchar vdc_ratch;			/* temporary value to keep track of the first byte
								 * when setting a 16 bits value with two byte access
								 */
	uchar vdc_satb;				/* boolean which keeps track of the need to copy
								 * the SATB from VRAM to internal SATB through DMA
								 */
	uchar vdc_satb_counter; 	/* DMA finished interrupt delay counter */
	uchar vdc_pendvsync;		/* unsure, set if a end of screen IRQ is waiting */
	uchar vdc_mode_chg;         /* Video mode change needed at next frame */
	uint16 bg_w;				/* number of tiles horizontaly in virtual screen */
	uint16 bg_h;				/* number of tiles vertically in virtual screen */
	uint16 screen_w;			/* size of real screen in pixels */
	uint16 screen_h;			/* size of real screen in pixels */
	uint16 vdc_minline;			/* First scanline of active display */
	uint16 vdc_maxline;			/* Last scanline of active display */

	/* joypad */
	uchar JOY[8];				/* value of pressed button/direct for each pad */
	uchar joy_select;			/* used to know what nibble we must return */
	uchar joy_counter;			/* current addressed joypad */

	/* PSG */
	uchar PSG[PSG_CHANNELS][8];
	uchar PSG_WAVE[PSG_CHANNELS][32];
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

	uchar psg_da_data[PSG_CHANNELS][PSG_DA_BUFSIZE];
	uint16 psg_da_index[PSG_CHANNELS];
	uint16 psg_da_count[PSG_CHANNELS];

	/* TIMER */
	uchar timer_reload, timer_start, timer_counter;

	/* IRQ */
	uchar irq_mask, irq_status;

	/* Remanence latch */
	uchar io_buffer;

} IO_t;

typedef struct {
	// Main memory
	uchar RAM[RAMSIZE];

	// Extra RAM contained on the HuCard (Populous)
	uchar *ExtraRAM;

	// Backup RAM
	uchar BackupRAM[BRAMSIZE];

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

	// ROM memory
	uchar *ROM;

	// ROM size in 0x2000 blocks
	uint16 ROMSIZE;

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
	uint16 Scanline;

	// Total number of elapsed cycles in the current scanline
	uint16 Cycles;

	// Total number of elapsed cycles
	uint32 TotalCycles;

	// Previous number of elapsed cycles
	uint32 PrevTotalCycles;

	// Value of each of the MMR registers
	uchar MMR[8];
	uchar SF2; // Street Fighter 2 Mapper

	// IO Registers
	IO_t IO;

} PCE_t;

/**
  * Exported functions to access hardware
  **/

int  hard_init(void);
void hard_reset(void);
void hard_term(void);

void  IO_write(uint16 A, uchar V);
uchar IO_read(uint16 A);
uchar TimerInt();
void  bank_set(uchar P, uchar V);

/**
  * Exported variables
  **/

extern PCE_t PCE;
// The global structure for all hardware variables

extern uchar *IOAREA, *TRAPRAM;
// Regions of the memory map that we need to trap. Normally games do not read or write to these areas.

extern uchar *PageR[8];
extern uchar *PageW[8];
extern uchar *MemoryMapR[256];
extern uchar *MemoryMapW[256];
// physical address on emulator machine of each of the 256 banks

#define RAM PCE.RAM
#define BackupRAM PCE.BackupRAM
#define ExtraRAM PCE.ExtraRAM
#define SPRAM PCE.SPRAM
#define VRAM PCE.VRAM
#define ROM PCE.ROM
#define ROMSIZE PCE.ROMSIZE
#define Scanline PCE.Scanline
#define Palette PCE.Palette
#define Cycles PCE.Cycles
#define TotalCycles PCE.TotalCycles
#define PrevTotalCycles PCE.PrevTotalCycles
#define MMR PCE.MMR
#define SF2 PCE.SF2
#define io PCE.IO

#define IO_VDC_REG        io.VDC
#define IO_VDC_REG_ACTIVE io.VDC[io.vdc_reg]

// H6280 CPU registers
#define reg_pc PCE.reg_pc
#define reg_a  PCE.reg_a
#define reg_x  PCE.reg_x
#define reg_y  PCE.reg_y
#define reg_p  PCE.reg_p
#define reg_s  PCE.reg_s

#define SATBIntON  (IO_VDC_REG[DCR].W&0x01)
#define DMAIntON   (IO_VDC_REG[DCR].W&0x02)

#define SpHitON    (IO_VDC_REG[CR].W&0x01)
#define OverON     (IO_VDC_REG[CR].W&0x02)
#define RasHitON   (IO_VDC_REG[CR].W&0x04)
#define VBlankON   (IO_VDC_REG[CR].W&0x08)
#define SpriteON   (IO_VDC_REG[CR].W&0x40)
#define ScreenON   (IO_VDC_REG[CR].W&0x80)

#define	ScrollX	(IO_VDC_REG[BXR].W)
#define	ScrollY	(IO_VDC_REG[BYR].W)
#define	Control (IO_VDC_REG[CR].W)

#define VDC_CR     0x01
#define VDC_OR     0x02
#define VDC_RR     0x04
#define VDC_DS     0x08
#define VDC_DV     0x10
#define VDC_VD     0x20
#define VDC_BSY    0x40
#define VDC_SpHit       VDC_CR
#define VDC_Over        VDC_OR
#define VDC_RasHit      VDC_RR
#define VDC_InVBlank    VDC_VD
#define VDC_DMAfinish   VDC_DV
#define VDC_SATBfinish  VDC_DS

#define TimerPeriod 1097 // Base period for the timer

#define IRQ2    1
#define IRQ1    2
#define TIRQ    4

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


static inline uchar
Read8(uint16 addr)
{
	uchar *page = PageR[addr >> 13];

	if (page == IOAREA)
		return IO_read(addr);
	else
		return page[addr];
}

static inline void
Write8(uint16 addr, uchar byte)
{
	uchar *page = PageW[addr >> 13];

	if (page == IOAREA)
		IO_write(addr, byte);
	else
		page[addr] = byte;
}

static inline uint16
Read16(uint16 addr)
{
#ifdef WORDS_BIGENDIAN
	uchar memreg = addr >> 13;

	uint16 ret_16bit = PageR[memreg][addr];
	memreg = (++addr) >> 13;
	ret_16bit += (uint16) (PageR[memreg][addr] << 8);

	return (ret_16bit);
#else
	return (*((uint16*)(PageR[addr >> 13] + (addr))));
#endif
}

static inline void
Write16(uint16 addr, uint16 word)
{
#ifdef WORDS_BIGENDIAN
#else
	*((uint16*)(PageR[addr >> 13] + (addr))) = word;
#endif
}

#endif
