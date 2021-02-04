/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _SNES9X_H_
#define _SNES9X_H_

#ifndef VERSION
#define VERSION	"1.60"
#endif

#include "port.h"

#include <rg_system.h>

/* Experimental retro-go flags */

// RETRO_LESS_ACCURATE runs pending events only once per execution
// loop. This is much less accurate, but also much faster. to be
// seen how badly it affects game
#define RETRO_LESS_ACCURATE_CPU 1
#define RETRO_LESS_ACCURATE_MEM 1
#define RETRO_LESS_ACCURATE_APU 0


#define STREAM					FILE *
#define GETS_STREAM(p, l, s)	fgets(p, l, s)
#define GETC_STREAM(s)			fgetc(s)
#define READ_STREAM(p, l, s)	fread(p, 1, l, s)
#define WRITE_STREAM(p, l, s)	fwrite(p, 1, l, s)
#define OPEN_STREAM(f, m)		fopen(f, m)
#define FIND_STREAM(s)			ftell(s)
#define REVERT_STREAM(s, o, p)	fseek(s, o, p)
#define CLOSE_STREAM(s)			fclose(s)


#define SNES_WIDTH					256
#define SNES_HEIGHT					224
#define SNES_HEIGHT_EXTENDED		239

#define	NTSC_MASTER_CLOCK			21477272.727272 // 21477272 + 8/11 exact
#define	PAL_MASTER_CLOCK			21281370.0

#define SNES_MAX_NTSC_VCOUNTER		262
#define SNES_MAX_PAL_VCOUNTER		312
#define SNES_MAX_VCOUNTER			(Settings.PAL ? SNES_MAX_PAL_VCOUNTER : SNES_MAX_NTSC_VCOUNTER)
#define SNES_MAX_HCOUNTER			341

#define ONE_CYCLE					6
#define SLOW_ONE_CYCLE				8
#define TWO_CYCLES					12
#define	ONE_DOT_CYCLE				4

#define SNES_CYCLES_PER_SCANLINE	(SNES_MAX_HCOUNTER * ONE_DOT_CYCLE)
#define SNES_SCANLINE_TIME			(SNES_CYCLES_PER_SCANLINE / NTSC_MASTER_CLOCK)

#define SNES_SPRITE_TILE_PER_LINE	34

#define SNES_WRAM_REFRESH_HC		538
#define SNES_WRAM_REFRESH_CYCLES	40

#define SNES_HBLANK_START_HC		1096					// H=274
#define	SNES_HDMA_START_HC			1106					// FIXME: not true
#define	SNES_HBLANK_END_HC			4						// H=1
#define	SNES_HDMA_INIT_HC			20						// FIXME: not true
#define	SNES_RENDER_START_HC		(128 * ONE_DOT_CYCLE)	// FIXME: Snes9x renders a line at a time.

#define SNES_IRQ_TRIGGER_CYCLES		14

/* From byuu: The total delay time for both the initial (H)DMA sync (to the DMA clock),
	and the end (H)DMA sync (back to the last CPU cycle's mcycle rate (6, 8, or 12)) always takes between 12-24 mcycles.
	Possible delays: { 12, 14, 16, 18, 20, 22, 24 }
	XXX: Snes9x can't emulate this timing :( so let's use the average value... */
#define SNES_DMA_CPU_SYNC_CYCLES	18

/* If the CPU is halted (i.e. for DMA) while /NMI goes low, the NMI will trigger
	after the DMA completes (even if /NMI goes high again before the DMA
	completes). In this case, there is a 24-30 cycle delay between the end of DMA
	and the NMI handler, time enough for an instruction or two. */
#define SNES_NMI_DMA_DELAY		24

#define SNES_TR_MASK		(1 <<  4)
#define SNES_TL_MASK		(1 <<  5)
#define SNES_X_MASK			(1 <<  6)
#define SNES_A_MASK			(1 <<  7)
#define SNES_RIGHT_MASK		(1 <<  8)
#define SNES_LEFT_MASK		(1 <<  9)
#define SNES_DOWN_MASK		(1 << 10)
#define SNES_UP_MASK		(1 << 11)
#define SNES_START_MASK		(1 << 12)
#define SNES_SELECT_MASK	(1 << 13)
#define SNES_Y_MASK			(1 << 14)
#define SNES_B_MASK			(1 << 15)

#define DEBUG_MODE_FLAG		(1 <<  0)	// debugger
#define TRACE_FLAG			(1 <<  1)	// debugger
#define SINGLE_STEP_FLAG	(1 <<  2)	// debugger
#define BREAK_FLAG			(1 <<  3)	// debugger
#define SCAN_KEYS_FLAG		(1 <<  4)	// CPU
#define HALTED_FLAG			(1 << 12)	// APU
#define FRAME_ADVANCE_FLAG	(1 <<  9)

#define ROM_NAME_LEN	23
#define AUTO_FRAMERATE	200

typedef enum
{
	HC_HBLANK_START_EVENT = 1,
	HC_HDMA_START_EVENT   = 2,
	HC_HCOUNTER_MAX_EVENT = 3,
	HC_HDMA_INIT_EVENT    = 4,
	HC_RENDER_EVENT       = 5,
	HC_WRAM_REFRESH_EVENT = 6
} hevent_t;

enum
{
	IRQ_NONE        = 0x0,
	IRQ_SET_FLAG    = 0x1,
	IRQ_CLEAR_FLAG  = 0x2,
	IRQ_TRIGGER_NMI = 0x4
};

struct SCPUState
{
	uint32	Flags;
	int32	Cycles;
	int32	V_Counter;
	uint8	*PCBase;
	bool8	NMIPending;
	bool8	IRQLine;
	bool8	IRQLastState;
	int32	IRQPending;
	int32	MemSpeed;
	int32	MemSpeedx2;
	int32	FastROMSpeed;
	bool8	InDMA;
	bool8	InHDMA;
	bool8	InDMAorHDMA;
	bool8	InWRAMDMAorHDMA;
	uint8	HDMARanInDMA;
	int32	CurrentDMAorHDMAChannel;
	hevent_t	WhichEvent;
	int32	NextEvent;
	bool8	WaitingForInterrupt;
	uint32	AutoSaveTimer;
	bool8	SRAMModified;

	int32	NMITriggerPos;
	int32	NextIRQTimer;
	int32	IRQFlagChanging;	// This value is just a hack.
};

struct SSettings
{
	bool8	TraceDMA;
	bool8	TraceHDMA;
	bool8	TraceVRAM;
	bool8	TraceUnknownRegisters;
	bool8	TraceDSP;
	bool8	TraceHCEvent;
	bool8	TraceSMP;

	uint8	DSP;

	bool8	ForcePAL;
	bool8	ForceNTSC;
	bool8	PAL;
	uint32	FrameTime;
	uint32	FrameRate;

	bool8	SoundSync;
	uint32	SoundPlaybackRate;
	uint32	SoundInputRate;
	bool8	Stereo;
	bool8	Mute;
	bool8	DynamicRateControl;
	int32	DynamicRateLimit; /* Multiplied by 1000 */
	int32	InterpolationMethod;

	bool8	Transparency;
	uint8	BG_Forced;
	bool8	DisableGraphicWindows;

	uint32	InitialInfoStringTimeout;
	uint16	DisplayColor;

	bool8	Paused;

	uint32	SkipFrames;
	uint32	TurboSkipFrames;
	bool8	TurboMode;

	int32	AutoSaveDelay;

	bool8	DisableGameSpecificHacks;
	bool8	UniracersHack;
	bool8	DMACPUSyncHack;
};

void S9xExit(void);
void S9xMessage(int, int, const char *);
void S9xInitSettings(void);

extern struct SSettings			Settings;
extern struct SCPUState			CPU;
extern char						String[513];

// Types of message sent to S9xMessage()
enum
{
	S9X_TRACE,
	S9X_DEBUG,
	S9X_WARNING,
	S9X_INFO,
	S9X_ERROR,
	S9X_FATAL_ERROR
};

// Individual message numbers
enum
{
	S9X_NO_INFO,
	S9X_ROM_INFO,
	S9X_HEADERS_INFO,
	S9X_CONFIG_INFO,
	S9X_ROM_CONFUSING_FORMAT_INFO,
	S9X_ROM_INTERLEAVED_INFO,
	S9X_SOUND_DEVICE_OPEN_FAILED,
	S9X_APU_STOPPED,
	S9X_USAGE,
	S9X_GAME_GENIE_CODE_ERROR,
	S9X_ACTION_REPLY_CODE_ERROR,
	S9X_GOLD_FINGER_CODE_ERROR,
	S9X_DEBUG_OUTPUT,
	S9X_DMA_TRACE,
	S9X_HDMA_TRACE,
	S9X_WRONG_FORMAT,
	S9X_WRONG_VERSION,
	S9X_ROM_NOT_FOUND,
	S9X_FREEZE_FILE_NOT_FOUND,
	S9X_PPU_TRACE,
	S9X_TRACE_DSP1,
	S9X_FREEZE_ROM_NAME,
	S9X_HEADER_WARNING,
	S9X_NETPLAY_NOT_SERVER,
	S9X_FREEZE_FILE_INFO,
	S9X_TURBO_MODE,
	S9X_SOUND_NOT_BUILT,
	S9X_SNAPSHOT_INCONSISTENT,
	S9X_PRESSED_KEYS_INFO
};

#endif
