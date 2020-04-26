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
  * Exported functions to access hardware
  **/

void hard_init(void);
void hard_reset_io(void);
void hard_term(void);

void IO_write(uint16 A, uchar V);
uchar IO_read(uint16 A);
void bank_set(uchar P, uchar V);

uchar read_memory_simple(uint16);
void write_memory_simple(uint16, uchar);

#define Wr6502(A,V) (write_memory_simple((A),(V)))
#define Rd6502(A) (read_memory_simple(A))

void dump_pce_cpu_environment();

/**
  * Global structure for all hardware variables
  **/

#include "shared_memory.h"

/**
  * Exported variables
  **/

extern struct_hard_pce hard_pce;
// The global structure for all hardware variables

#define io (*p_io)

extern IO *p_io;
// the global I/O status

extern uchar *RAM;
// mem where variables are stocked (well, RAM... )
// in reality, only 0x2000 bytes are used in a coregraphx and 0x8000 only
// in a supergraphx

extern uchar *WRAM;
// extra backup memory
// This memory lies in Interface Unit or eventually in RGB adaptator

extern uchar *VRAM;
// Video mem
// 0x10000 bytes on coregraphx, the double on supergraphx I think
// contain information about the sprites position/status, information
// about the pattern and palette to use for each tile, and patterns
// for use in sprite/tile rendering

extern uint16 *SPRAM;
// SPRAM = sprite RAM
// The pc engine got a function to transfert a piece VRAM toward the inner
// gfx cpu sprite memory from where data will be grabbed to render sprites

extern uchar *Pal;
// PCE->PC Palette convetion array
// Each of the 512 available PCE colors (333 RGB -> 512 colors)
// got a correspondancy in the 256 fixed colors palette

extern uchar *VRAM2, *VRAMS;
// These are array to keep in memory the result of the linearisation of
// PCE sprites and tiles

extern uchar *vchange, *vchanges;
// These array are boolean array to know if we must update the
// corresponding linear sprite representation in VRAM2 and VRAMS or not
// if (vchanges[5] != 0) 6th pattern in VRAM2 must be updated

#define scanline (*p_scanline)

extern uint32 *p_scanline;
// The current rendered line on screen

extern uchar *PCM;
// The ADPCM array (0x10000 bytes)

extern uchar *zp_base;
// pointer to the beginning of the Zero Page area

extern uchar *sp_base;
// pointer to the beginning of the Stack Area

extern uchar *mmr;
// Value of each of the MMR registers

extern uchar *IOAREA;
// physical address on emulator machine of the IO area (fake address as it has to be handled specially)

extern uchar *TRAPRAM;
// False "ram"s in which you can read/write (to homogeneize writes into RAM, BRAM, ... as well as in rom) but the result isn't coherent

extern uchar *PageR[8];
extern uchar *ROMMapR[256];

extern uchar *PageW[8];
extern uchar *ROMMapW[256];
// physical address on emulator machine of each of the 256 banks

#define cyclecount (*p_cyclecount)

extern uint32 *p_cyclecount;
// Number of elapsed cycles

#define cyclecountold (*p_cyclecountold)

extern uint32 *p_cyclecountold;
// Previous number of elapsed cycles

#define external_control_cpu (*p_external_control_cpu)

extern int32 *p_external_control_cpu;

#define TimerPeriod 1097
// extern const uint32 TimerPeriod;
// Base period for the timer

// registers:

#define reg_pc reg_pc_
#define reg_a reg_a_
#define reg_x reg_x_
#define reg_y reg_y_
#define reg_p reg_p_
#define reg_s reg_s_

extern uint32 reg_pc;
extern uchar reg_a;
extern uchar reg_x;
extern uchar reg_y;
extern uchar reg_p;
extern uchar reg_s;

// These are the main h6280 register, reg_p is the flag register

#define cycles (*p_cycles)
extern uint32 *p_cycles;

// Number of pc engine cycles elapsed since the resetting of the emulated console

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

#endif
