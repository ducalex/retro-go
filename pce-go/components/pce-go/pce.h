#pragma once

#include "pce-go.h"

// System clocks (hz)
#define CLOCK_MASTER           (21477270)
#define CLOCK_TIMER            (CLOCK_MASTER / 3)
#define CLOCK_CPU              (CLOCK_MASTER / 3)
#define CLOCK_PSG              (CLOCK_MASTER / 6)

// Timings (we don't support CSH/CSL yet...)
#define CYCLES_PER_FRAME       (CLOCK_CPU / 60)
#define CYCLES_PER_LINE        (CYCLES_PER_FRAME / 263 + 1)
#define CYCLES_PER_TIMER_TICK  (1024) // 1097
//#define CYCLES_PER_TIMER_TICK  (1097)

typedef struct __attribute__((packed))
{
	uint16_t y; 	/* Vertical position */
	uint16_t x; 	/* Horizontal position */
	uint16_t no;   	/* Offset in VRAM */
	uint16_t attr; 	/* Attributes */
	/*
		* bit 0-4 : number of the palette to be used
		* bit 7 : background sprite
		*          0 -> must be drawn behind tiles
		*          1 -> must be drawn in front of tiles
		* bit 8 : width
		*          0 -> 16 pixels
		*          1 -> 32 pixels
		* bit 11 : horizontal flip
		*          0 -> normal shape
		*          1 -> must be draw horizontally flipped
		* bit 13-12 : height
		*          00 -> 16 pixels
		*          01 -> 32 pixels
		*          10 -> 48 pixels
		*          11 -> 64 pixels
		* bit 15 : vertical flip
		*          0 -> normal shape
		*          1 -> must be drawn vertically flipped
	*/
} sprite_t;

// VDC Status Flags (vdc_status bit)
typedef enum {
	VDC_STAT_CR  = 0,	/* Sprite Collision */
	VDC_STAT_OR  = 1,	/* Sprite Overflow */
	VDC_STAT_RR  = 2,	/* Scanline interrupt */
	VDC_STAT_DS  = 3,	/* End of VRAM to SATB DMA transfer */
	VDC_STAT_DV  = 4,	/* End of VRAM to VRAM DMA transfer */
	VDC_STAT_VD  = 5,	/* VBlank */
	VDC_STAT_BSY = 6,	/* DMA Transfer in progress */
} vdc_stat_t;

// VDC Registers
typedef enum {
	MAWR 	= 0,		/* Memory Address Write Register */
	MARR 	= 1,		/* Memory Address Read Register */
	VRR 	= 2,		/* VRAM Read Register */
	VWR 	= 2,		/* VRAM Write Register */
	vdc3 	= 3,		/* Unused */
	vdc4 	= 4,		/* Unused */
	CR 		= 5,		/* Control Register */
	RCR 	= 6,		/* Raster Compare Register */
	BXR 	= 7,		/* Horizontal scroll offset */
	BYR 	= 8,		/* Vertical scroll offset */
	MWR 	= 9,		/* Memory Width Register */
	HSR 	= 10,		/* Unknown, other horizontal definition */
	HDR 	= 11,		/* Horizontal Definition */
	VPR 	= 12,		/* Higher byte = VDS, lower byte = VSW */
	VDW 	= 13,		/* Vertical Definition */
	VCR 	= 14,		/* Vertical counter between restarting of display */
	DCR 	= 15,		/* DMA Control */
	SOUR 	= 16,		/* Source Address of DMA transfert */
	DISTR 	= 17,		/* Destination Address of DMA transfert */
	LENR 	= 18,		/* Length of DMA transfert */
	SATB 	= 19		/* Address of SATB */
} vdc_reg_t;

#define PSG_CHAN_ENABLE         0x80 // bit 7
#define PSG_DDA_ENABLE          0x40 // bit 6
#define PSG_CHAN_VOLUME         0x1F // bits 0-4
#define PSG_BALANCE_LEFT        0xF0 // bits 4-7
#define PSG_BALANCE_RIGHT       0x0F // bits 0-3
#define PSG_NOISE_ENABLE        0x80 // bit 7

#define PSG_CHANNELS            6


typedef union {
	struct {
		uint8_t l, h;
	} B;
	uint16_t W;
} UWord;

typedef struct {
	uint8_t freq_lsb;   // 2
	uint8_t freq_msb;   // 3
	uint8_t control;    // 4
	uint8_t balance;    // 5
	uint8_t wave_index; // 6
	uint8_t noise_ctrl; // 7
	uint8_t pad0, pad1;

	uint8_t wave_data[32];
	uint8_t dda_data[256];

	uint32_t dda_count;
	uint32_t dda_index;

	uint32_t wave_accum;

	int32_t noise_accum;
	int32_t noise_level;
	int32_t noise_rand;
} psg_chan_t;

typedef struct {
	// Main memory
	uint8_t RAM[0x2000];

	// Video RAM
	uint16_t VRAM[0x8000];

	// Sprite RAM
	sprite_t SPRAM[64];

	// Extra RAM contained on the HuCard (Populous)
	uint8_t *ExRAM;

	// ROM memory
	uint8_t *ROM, *ROM_DATA;

	// ROM size in 0x2000 blocks
	uint16_t ROM_SIZE;

	// ROM crc
	uint32_t ROM_CRC;

	// For performance reasons we trap read/writes to unmapped areas:
	uint8_t *IOAREA;
	uint8_t *NULLRAM;

	// PCE->PC Palette convetion array
	// Each of the 512 available PCE colors (333 RGB -> 512 colors)
	// got a correspondance in the 256 fixed colors palette
	uint8_t Palette[512];

	// The current rendered line on screen
	int32_t Scanline;

	//
	int ScrollYDiff;

	// Number of executed CPU cycles
	int32_t Cycles;

	// Run CPU until Cycles >= MaxCycles
	int32_t MaxCycles;

	// Value of each of the MMR registers
	uint8_t MMR[8];

	// Effective memory map
	uint8_t *MemoryMapR[256];
	uint8_t *MemoryMapW[256];

	// Street Fighter 2 Mapper
	uint8_t SF2;

	// Remanence latch
	uint8_t io_buffer;

	// Timer
	struct {
		int32_t cycles_per_line;
		int32_t cycles_counter;
		uint32_t counter;
		uint32_t reload;
		uint32_t running;
	} Timer;

	// Joypad
	struct {
		uint8_t regs[8];		/* value of pressed button/direct for each pad */
		uint8_t nibble;			/* used to know what nibble we must return */
		uint8_t counter;		/* current addressed joypad */
	} Joypad;

	// Video Color Encoder
	struct {
		UWord regs[0x200];		/* palette info */
		size_t reg;				/* currently selected color */
	} VCE;

	// Video Display Controller
	struct {
		UWord regs[32];			/* value of each VDC register */
		size_t reg;				/* currently selected VDC register */
		uint8_t status;			/* current VCD status (end of line, end of screen, ...) */
		uint8_t vram;			/* VRAM DMA transfer status to happen in vblank */
		uint8_t satb;			/* DMA transfer status to happen in vblank */
		uint8_t mode_chg;       /* Video mode change needed at next frame */
		uint32_t pending_irqs;	/* Pending VDC IRQs (we use it as a stack of 4bit events) */
		uint32_t screen_width;	/* Effective resolution updated by mode_chg */
		uint32_t screen_height;	/* Effective resolution updated by mode_chg */
	} VDC;

	// Programmable Sound Generator
	struct {
		uint8_t ch;             // reg 0
		uint8_t volume;         // reg 1
		uint8_t lfo_freq;       // reg 8
		uint8_t lfo_ctrl;       // reg 9
		psg_chan_t chan[PSG_CHANNELS]; // regs 2-7
		uint8_t padding[16];
	} PSG;

} PCE_t;


#include "h6280.h"


// The global structure for all hardware variables
extern PCE_t PCE;

// physical address on emulator machine of each of the 256 banks
extern uint8_t *PageR[8];
extern uint8_t *PageW[8];

#define IO_VDC_REG           PCE.VDC.regs
#define IO_VDC_REG_ACTIVE    PCE.VDC.regs[PCE.VDC.reg]
#define IO_VDC_REG_INC(reg)  {unsigned _i[] = {1,32,64,128}; PCE.VDC.regs[(reg)].W += _i[(PCE.VDC.regs[CR].W >> 11) & 3];}
#define IO_VDC_STATUS(bit)   ((PCE.VDC.status >> bit) & 1)
#define IO_VDC_MINLINE       (IO_VDC_REG[VPR].B.h + IO_VDC_REG[VPR].B.l)
#define IO_VDC_MAXLINE       (IO_VDC_MINLINE + IO_VDC_REG[VDW].W)
#define IO_VDC_SCREEN_WIDTH  ((IO_VDC_REG[HDR].B.l + 1) * 8)
#define IO_VDC_SCREEN_HEIGHT (IO_VDC_REG[VDW].W + 1)

// Interrupt enabled
#define SATBIntON  (IO_VDC_REG[DCR].W & 0x01)
#define DMAIntON   (IO_VDC_REG[DCR].W & 0x02)
#define AutoSATBON (IO_VDC_REG[DCR].W & 0x10)
#define SpHitON    (IO_VDC_REG[CR].W & 0x01)
#define OverON     (IO_VDC_REG[CR].W & 0x02)
#define RasHitON   (IO_VDC_REG[CR].W & 0x04)
#define VBlankON   (IO_VDC_REG[CR].W & 0x08)

#define SpriteON   (IO_VDC_REG[CR].W & 0x40)
#define ScreenON   (IO_VDC_REG[CR].W & 0x80)
#define BurstMode  (IO_VDC_REG[CR].W & 0x0C)

#define DMA_TRANSFER_COUNTER 0x80
#define DMA_TRANSFER_PENDING 0x40

/**
 * Exported Functions
 */

int  pce_init(void);
void pce_reset(bool hard);
void pce_term(void);
void pce_run(void);
void pce_pause(void);
void pce_writeIO(uint16_t A, uint8_t V);
uint8_t pce_readIO(uint16_t A);


/**
 * Inlined Functions
 */

#if USE_MEM_MACROS

#define pce_read8(addr) ({							\
	uint16_t a = (addr); 							\
	uint8_t *page = PageR[a >> 13]; 				\
	(page == PCE.IOAREA) ? pce_readIO(a) : page[a]; \
})

#define pce_write8(addr, byte) {					\
	uint16_t a = (addr), b = (byte); 				\
	uint8_t *page = PageW[a >> 13]; 				\
	if (page == PCE.IOAREA) pce_writeIO(a, b); 		\
	else page[a] = b;								\
}

#define pce_read16(addr) ({							\
	uint16_t a = (addr); 							\
	*((uint16_t*)(PageR[a >> 13] + a));				\
})

#define pce_write16(addr, word) {					\
	uint16_t a = (addr), w = (word); 				\
	*((uint16_t*)(PageW[a >> 13] + a)) = w;			\
}

#else

static inline uint8_t
pce_read8(uint16_t addr)
{
	uint8_t *page = PageR[addr >> 13];

	if (page == PCE.IOAREA)
		return pce_readIO(addr);
	else
		return page[addr];
}

static inline void
pce_write8(uint16_t addr, uint8_t byte)
{
	uint8_t *page = PageW[addr >> 13];

	if (page == PCE.IOAREA)
		pce_writeIO(addr, byte);
	else
		page[addr] = byte;
}

static inline uint16_t
pce_read16(uint16_t addr)
{
	return (*((uint16_t*)(PageR[addr >> 13] + (addr))));
}

static inline void
pce_write16(uint16_t addr, uint16_t word)
{
	*((uint16_t*)(PageW[addr >> 13] + (addr))) = word;
}

#endif


static inline void
pce_bank_set(uint8_t P, uint8_t V)
{
	//TRACE_IO("Bank switching (MMR[%d] = %d)\n", P, V);

	PCE.MMR[P] = V;
	PageR[P] = (PCE.MemoryMapR[V] == PCE.IOAREA) ? (PCE.IOAREA) : (PCE.MemoryMapR[V] - P * 0x2000);
	PageW[P] = (PCE.MemoryMapW[V] == PCE.IOAREA) ? (PCE.IOAREA) : (PCE.MemoryMapW[V] - P * 0x2000);
}
