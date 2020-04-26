#ifndef INCLUDE_SHARED_MEMORY
#define INCLUDE_SHARED_MEMORY

#include "cleantypes.h"
#include "myadd.h"

#define VRAMSIZE           0x10000

#define PSG_DIRECT_ACCESS_BUFSIZE 1024

typedef union {
#if defined(WORDS_BIGENDIAN)
	struct {
		uchar h, l;
	} B;
#else
	struct {
		uchar l, h;
	} B;
#endif
	uint16 W;
} pair;

/* The structure containing all variables relatives to Input and Output */
typedef struct tagIO {
	/* VCE */
	pair VCE[0x200];			/* palette info */
	pair vce_reg;				/* currently selected color */
	uchar vce_ratch;			/* temporary value to keep track of the first byte
								 * when setting a 16 bits value with two byte access
								 */
	/* VDC */
	pair VDC[32];				/* value of each VDC register */
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
	uchar RAM[0x8000];
	uchar *PCM; //[0x10000]
	uchar WRAM[0x2000];
	uchar VRAM[VRAMSIZE];
	uchar *VRAM2; //[VRAMSIZE];
	uchar *VRAMS; //[VRAMSIZE];
	uchar vchange[VRAMSIZE / 32];
	uchar vchanges[VRAMSIZE / 128];

	uint32 s_scanline;

	uint16 SPRAM[64 * 4];
	uchar Pal[512];

	uint16 s_reg_pc;
	uchar s_reg_a;
	uchar s_reg_x;
	uchar s_reg_y;
	uchar s_reg_p;
	uchar s_reg_s;

	uint32 s_cyclecount;
	uint32 s_cyclecountold;

	uint32 s_cycles;

	int32 s_external_control_cpu;

	uchar mmr[8];

	IO s_io;

	uint32 rom_shared_memory_size;

} struct_hard_pce;

#endif
