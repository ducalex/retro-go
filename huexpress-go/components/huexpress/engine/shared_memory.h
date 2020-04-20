#ifndef INCLUDE_SHARED_MEMORY
#define INCLUDE_SHARED_MEMORY

#include "cleantypes.h"
#include "myadd.h"

#define VRAMSIZE           0x10000

#define PSG_DIRECT_ACCESS_BUFSIZE 1024

#define SHM_HANDLE       0x25679
#define SHM_ROM_HANDLE   0x25680

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

	uchar *psg_da_data[6];//[PSG_DIRECT_ACCESS_BUFSIZE];
	uint16 psg_da_index[6], psg_da_count[6];
	int psg_channel_disabled[6];

	/* TIMER */
	uchar timer_reload, timer_start, timer_counter;

	/* IRQ */
	uchar irq_mask, irq_status;

	/* CDROM extention */
	int32 backup, adpcm_firstread;
	uchar cd_port_1800;
	uchar cd_port_1801;
	uchar cd_port_1802;
	uchar cd_port_1804;

	/* Adpcm related variables */
	pair adpcm_ptr;
	uint16 adpcm_rptr, adpcm_wptr;
	uint16 adpcm_dmaptr;
	uchar adpcm_rate;
	uint32 adpcm_pptr;			/* to know where to begin playing adpcm (in nibbles) */
	uint32 adpcm_psize;			/* to know how many 4-bit samples to play */

	/* Arcade Card variables */
	uint32 ac_base[4];			/* base address for AC ram accessing */
	uint16 ac_offset[4];		/* offset address for AC ram accessing */
	uint16 ac_incr[4];			/* incrment value after read or write accordingly to the control bit */

	uchar ac_control[4];		/* bit 7: unused
								 * bit 6: only $1AX6 hits will add offset to base
								 * bit 5 + bit 6: either hit to $1AX6 or $1AXA will add offset to base
								 * bit 4: auto increment offset if 0, and auto
								 *        increment base if 1
								 * bit 3: unknown
								 * bit 2: unknown
								 * bit 1: use offset address in the effective address
								 *   computation
								 * bit 0: apply autoincrement if set
								 */
	uint32 ac_shift;
	uchar ac_shiftbits;			/* number of bits to shift by */

/*        uchar  ac_unknown3; */
	uchar ac_unknown4;

	/* Remanence latch */
	uchar io_buffer;

} IO;

typedef struct {
	uchar RAM[0x8000];
	uchar *PCM; //[0x10000]
	uchar WRAM[0x2000];
	uchar *VRAM; //[VRAMSIZE]
	uchar *VRAM2; //[VRAMSIZE];
	uchar *VRAMS; //[VRAMSIZE];
	uchar vchange[VRAMSIZE / 32];
	uchar vchanges[VRAMSIZE / 128];

	uchar *cd_extra_mem;//[0x10000];
	uchar *cd_extra_super_mem;//[0x30000];
	uchar *ac_extra_mem;//[0x200000];
	uchar *cd_sector_buffer;//[0x2000];

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
