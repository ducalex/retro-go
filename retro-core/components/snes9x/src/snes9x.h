/* This file is part of Snes9x. See LICENSE file. */

#ifndef _SNES9X_H_
#define _SNES9X_H_

#include <stdlib.h>
#include <stdint.h>

#include "port.h"
#include "65c816.h"

#define ROM_NAME_LEN 22

/* SNES screen width and height */
#define SNES_WIDTH            256
#define SNES_HEIGHT           224
#define SNES_HEIGHT_EXTENDED  239

#define SNES_SPRITE_TILE_PER_LINE 34

#define SNES_MAX_NTSC_VCOUNTER  262
#define SNES_MAX_PAL_VCOUNTER   312
#define SNES_HCOUNTER_MAX       341
#define SPC700_TO_65C816_RATIO  2
#define AUTO_FRAMERATE          200

/* NTSC master clock signal 21.47727MHz
 * PPU: master clock / 4
 * 1 / PPU clock * 342 -> 63.695us
 * 63.695us / (1 / 3.579545MHz) -> 228 cycles per scanline
 * From Earth Worm Jim: APU executes an average of 65.14285714 cycles per
 * scanline giving an APU clock speed of 1.022731096MHz                    */

/* PAL master clock signal 21.28137MHz
 * PPU: master clock / 4
 * 1 / PPU clock * 342 -> 64.281us
 * 64.281us / (1 / 3.546895MHz) -> 228 cycles per scanline.  */

#define SNES_SCANLINE_TIME (63.695e-6)
#define SNES_CLOCK_SPEED   (3579545u)

#define SNES_CLOCK_LEN (1.0 / SNES_CLOCK_SPEED)

#define SNES_CYCLES_PER_SCANLINE ((uint32_t) ((SNES_SCANLINE_TIME / SNES_CLOCK_LEN) * 6 + 0.5))

#ifdef SNES_OVERCLOCK_CYCLES
#define ONE_CYCLE        (overclock_cycles ? one_c : 6u)
#define SLOW_ONE_CYCLE   (overclock_cycles ? slow_one_c : 8u)
#define TWO_CYCLES       (overclock_cycles ? two_c : 12u)
#else
#define ONE_CYCLE        (6u)
#define SLOW_ONE_CYCLE   (8u)
#define TWO_CYCLES       (12u)
#endif

#define SNES_TR_MASK     (1u << 4)
#define SNES_TL_MASK     (1u << 5)
#define SNES_X_MASK      (1u << 6)
#define SNES_A_MASK      (1u << 7)
#define SNES_RIGHT_MASK  (1u << 8)
#define SNES_LEFT_MASK   (1u << 9)
#define SNES_DOWN_MASK   (1u << 10)
#define SNES_UP_MASK     (1u << 11)
#define SNES_START_MASK  (1u << 12)
#define SNES_SELECT_MASK (1u << 13)
#define SNES_Y_MASK      (1u << 14)
#define SNES_B_MASK      (1u << 15)

extern bool overclock_cycles;
extern int one_c, slow_one_c, two_c;

enum
{
   SNES_MULTIPLAYER5,
   SNES_JOYPAD,
   SNES_MOUSE,
   SNES_SUPERSCOPE,
   SNES_JUSTIFIER,
   SNES_JUSTIFIER_2,
   SNES_MAX_CONTROLLER_OPTIONS
};

#define DEBUG_MODE_FLAG    (1u << 0)
#define TRACE_FLAG         (1u << 1)
#define SINGLE_STEP_FLAG   (1u << 2)
#define BREAK_FLAG         (1u << 3)
#define SCAN_KEYS_FLAG     (1u << 4)
#define SAVE_SNAPSHOT_FLAG (1u << 5)
#define DELAYED_NMI_FLAG   (1u << 6)
#define NMI_FLAG           (1u << 7)
#define PROCESS_SOUND_FLAG (1u << 8)
#define FRAME_ADVANCE_FLAG (1u << 9)
#define DELAYED_NMI_FLAG2  (1u << 10)
#define IRQ_PENDING_FLAG   (1u << 11)

typedef struct
{
   uint32_t Flags;
   bool     BranchSkip;
   bool     NMIActive;
   uint8_t  IRQActive;
   bool     WaitingForInterrupt;
   bool     InDMA;
   uint8_t  WhichEvent;
   uint8_t* PC;
   uint8_t* PCBase;
   uint8_t* PCAtOpcodeStart;
   uint8_t* WaitAddress;
   uint32_t WaitCounter;
   long     Cycles;       /* For savestate compatibility can't change to int32_t */
   long     NextEvent;    /* For savestate compatibility can't change to int32_t */
   long     V_Counter;    /* For savestate compatibility can't change to int32_t */
   long     MemSpeed;     /* For savestate compatibility can't change to int32_t */
   long     MemSpeedx2;   /* For savestate compatibility can't change to int32_t */
   long     FastROMSpeed; /* For savestate compatibility can't change to int32_t */
   uint32_t SaveStateVersion;
   bool     SRAMModified;
   uint32_t NMITriggerPoint;
   bool     UNUSED2;
   bool     TriedInterleavedMode2;
   uint32_t NMICycleCount;
   uint32_t IRQCycleCount;
} SCPUState;

#define HBLANK_START_EVENT  0u
#define HBLANK_END_EVENT    1u
#define HTIMER_BEFORE_EVENT 2u
#define HTIMER_AFTER_EVENT  3u
#define NO_EVENT            4u

typedef struct
{
   /* CPU options */
   bool     APUEnabled;
   bool     Shutdown;
   int32_t  H_Max;
   int32_t  HBlankStart;
   int32_t  CyclesPercentage;
   bool     DisableIRQ;

   /* Joystick options */
   bool     JoystickEnabled;

   /* ROM timing options (see also H_Max above) */
   bool     ForcePAL;
   bool     ForceNTSC;
   bool     PAL;
   uint32_t FrameTimePAL;
   uint32_t FrameTimeNTSC;
   uint32_t FrameTime;

   /* ROM image options */
   bool     ForceLoROM;
   bool     ForceHiROM;
   bool     ForceHeader;
   bool     ForceNoHeader;
   bool     ForceInterleaved;
   bool     ForceInterleaved2;
   bool     ForceNotInterleaved;

   /* Peripheral options */
   bool     ForceSuperFX;
   bool     ForceNoSuperFX;
   bool     ForceDSP1;
   bool     ForceNoDSP1;
   bool     ForceSA1;
   bool     ForceNoSA1;
   bool     ForceC4;
   bool     ForceNoC4;
   bool     ForceSDD1;
   bool     ForceNoSDD1;
   bool     MultiPlayer5;
   bool     Mouse;
   bool     SuperScope;
   bool     SRTC;
   uint32_t ControllerOption;
   bool     MultiPlayer5Master;
   bool     SuperScopeMaster;
   bool     MouseMaster;

   bool     SuperFX;
   bool     DSP1Master;
   bool     SA1;
   bool     C4;
   bool     SDD1;
   bool     SPC7110;
   bool     SPC7110RTC;
   bool     OBC1;
   uint8_t  DSP;

   /* Sound options */
   uint32_t SoundPlaybackRate;
#ifdef USE_BLARGG_APU
   uint32_t SoundInputRate;
#endif
   bool     TraceSoundDSP;
   bool     EightBitConsoleSound; /* due to caching, this needs S9xSetEightBitConsoleSound() */
   int32_t  SoundBufferSize;
   int32_t  SoundMixInterval;
   bool     SoundEnvelopeHeightReading;
   bool     DisableSoundEcho;
   bool     DisableMasterVolume;
   bool     SoundSync;
   bool     InterpolatedSound;
   bool     ThreadSound;
   bool     Mute;
   bool     NextAPUEnabled;

   /* Others */
   bool     ApplyCheats;

   /* Fixes for individual games */
   bool     StarfoxHack;
   bool     WinterGold;
   bool     BS; /* Japanese Satellite System games. */
   bool     JustifierMaster;
   bool     Justifier;
   bool     SecondJustifier;
   int8_t   SETA;
   bool     HardDisableAudio;
} SSettings;

extern SSettings Settings;
extern SCPUState CPU;
extern char String [513];

void S9xSetPause(uint32_t mask);
void S9xClearPause(uint32_t mask);
#endif
