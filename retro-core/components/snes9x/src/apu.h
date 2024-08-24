/* This file is part of Snes9x. See LICENSE file. */

#ifndef USE_BLARGG_APU

#ifndef _apu_h_
#define _apu_h_

#include "port.h"
#include "spc700.h"


typedef struct
{
   uint8_t*      PC;
   SAPURegisters Registers;
   uint8_t*      RAM;
   uint8_t*      DirectPage;
   bool          APUExecuting;
   uint8_t       Bit;
   uint32_t      Address;
   uint8_t*      WaitAddress1;
   uint8_t*      WaitAddress2;
   uint32_t      WaitCounter;
   uint8_t       _Carry;
   uint8_t       _Zero;
   uint8_t       _Overflow;
   uint32_t      UNUSED1;
   uint32_t      UNUSED2;
   int32_t       OneCycle;
   int32_t       TwoCycles;
} SIAPU;

typedef struct
{
   int32_t  Cycles;
   bool     ShowROM;
   uint8_t  UNUSED1;
   uint8_t  KeyedChannels;
   uint8_t  OutPorts     [4];
   uint8_t  DSP          [0x80];
   uint8_t  ExtraRAM     [64];
   uint16_t Timer        [3];
   uint16_t TimerTarget  [3];
   bool     TimerEnabled [3];
   bool     UNUSED2 [3];
} SAPU;

extern SAPU APU;
extern SIAPU IAPU;

static INLINE void S9xAPUUnpackStatus(void)
{
   IAPU._Zero     = ((IAPU.Registers.P & Zero) == 0) | (IAPU.Registers.P & Negative);
   IAPU._Carry    = (IAPU.Registers.P & Carry);
   IAPU._Overflow = IAPU.Registers.P & Overflow;
}

static INLINE void S9xAPUPackStatus(void)
{
   IAPU.Registers.P &= ~(Zero | Negative | Carry | Overflow);
   if (IAPU._Carry)
      IAPU.Registers.P |= Carry;
   if (!IAPU._Zero)
      IAPU.Registers.P |= Zero;
   if (IAPU._Overflow)
      IAPU.Registers.P |= Overflow;
   if (IAPU._Zero & 0x80)
      IAPU.Registers.P |= Negative;
}

void S9xResetAPU(void);
bool S9xInitAPU(void);
void S9xDeinitAPU(void);
void S9xDecacheSamples(void);
void S9xSetAPUControl(uint8_t byte);
void S9xSetAPUDSP(uint8_t byte);
uint8_t S9xGetAPUDSP(void);
bool S9xInitSound(int32_t buffer_ms, int32_t lag_ms);
void S9xPrintAPUState(void);
extern uint8_t S9xAPUCycles [256];       /* Scaled cycle lengths */
extern const uint8_t S9xAPUCycleLengths [256]; /* Raw data. */
extern void (* const S9xApuOpcodes [256])(void);

#define APU_VOL_LEFT  0x00
#define APU_VOL_RIGHT 0x01
#define APU_P_LOW     0x02
#define APU_P_HIGH    0x03
#define APU_SRCN      0x04
#define APU_ADSR1     0x05
#define APU_ADSR2     0x06
#define APU_GAIN      0x07
#define APU_ENVX      0x08
#define APU_OUTX      0x09

#define APU_MVOL_LEFT  0x0c
#define APU_MVOL_RIGHT 0x1c
#define APU_EVOL_LEFT  0x2c
#define APU_EVOL_RIGHT 0x3c
#define APU_KON        0x4c
#define APU_KOFF       0x5c
#define APU_FLG        0x6c
#define APU_ENDX       0x7c

#define APU_EFB  0x0d
#define APU_PMON 0x2d
#define APU_NON  0x3d
#define APU_EON  0x4d
#define APU_DIR  0x5d
#define APU_ESA  0x6d
#define APU_EDL  0x7d

#define APU_C0 0x0f
#define APU_C1 0x1f
#define APU_C2 0x2f
#define APU_C3 0x3f
#define APU_C4 0x4f
#define APU_C5 0x5f
#define APU_C6 0x6f
#define APU_C7 0x7f

#define APU_SOFT_RESET    0x80
#define APU_MUTE          0x40
#define APU_ECHO_DISABLED 0x20

#define FREQUENCY_MASK 0x3fff
#endif

#else
#include "apu_blargg.h"
#define ONE_APU_CYCLE 21
#define APU_EXECUTE1() do {} while(0)
#define APU_EXECUTE()  do {} while(0)

#endif
