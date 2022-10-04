/*
    SN76489 emulation
    by Maxim in 2001 and 2002
    converted from my original Delphi implementation

    I'm a C newbie so I'm sure there are loads of stupid things
    in here which I'll come back to some day and redo

    Includes:
    - Super-high quality tone channel "oversampling" by calculating fractional positions on transitions
    - Noise output pattern reverse engineered from actual SMS output
    - Volume levels taken from actual SMS output

    07/08/04  Charles MacDonald
    Modified for use with SMS Plus:
    - Added support for multiple PSG chips.
    - Added reset/config/update routines.
    - Added context management routines.
    - Removed SN76489_GetValues().
    - Removed some unused variables.

    07/08/04  bzhxx few simplication for gwenesis to fit on MCU
*/


#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include "gwenesis_bus.h"
#include "gwenesis_sn76489.h"
#include "gwenesis_savestate.h"

#define NoiseInitialState   0x8000  /* Initial state of shift register */
#define PSG_CUTOFF          0x6     /* Value below which PSG does not output */

// #define PSG_MAX_VOLUME 2800
// static const uint16 chanVolume[16] = {
//   PSG_MAX_VOLUME,               /*  MAX  */
//   PSG_MAX_VOLUME * 0.794328234, /* -2dB  */
//   PSG_MAX_VOLUME * 0.630957344, /* -4dB  */
//   PSG_MAX_VOLUME * 0.501187233, /* -6dB  */
//   PSG_MAX_VOLUME * 0.398107170, /* -8dB  */
//   PSG_MAX_VOLUME * 0.316227766, /* -10dB */
//   PSG_MAX_VOLUME * 0.251188643, /* -12dB */
//   PSG_MAX_VOLUME * 0.199526231, /* -14dB */
//   PSG_MAX_VOLUME * 0.158489319, /* -16dB */
//   PSG_MAX_VOLUME * 0.125892541, /* -18dB */
//   PSG_MAX_VOLUME * 0.1,         /* -20dB */
//   PSG_MAX_VOLUME * 0.079432823, /* -22dB */
//   PSG_MAX_VOLUME * 0.063095734, /* -24dB */
//   PSG_MAX_VOLUME * 0.050118723, /* -26dB */
//   PSG_MAX_VOLUME * 0.039810717, /* -28dB */
//   0                             /*  OFF  */
// };

#define PSG_MAX_VOLUME_MAX 3100
#define PSG_MAX_VOLUME_2dB (int)(PSG_MAX_VOLUME_MAX*0.794328234)
#define PSG_MAX_VOLUME_4dB (int)(PSG_MAX_VOLUME_MAX*0.630957344)

static const int PSGVolumeValues[16] = {
	PSG_MAX_VOLUME_MAX   ,PSG_MAX_VOLUME_2dB   ,PSG_MAX_VOLUME_4dB,
    PSG_MAX_VOLUME_MAX/2 ,PSG_MAX_VOLUME_2dB/2 ,PSG_MAX_VOLUME_4dB/2,
    PSG_MAX_VOLUME_MAX/4 ,PSG_MAX_VOLUME_2dB/4 ,PSG_MAX_VOLUME_4dB/4,
    PSG_MAX_VOLUME_MAX/8 ,PSG_MAX_VOLUME_2dB/8 ,PSG_MAX_VOLUME_4dB/8,
    PSG_MAX_VOLUME_MAX/16,PSG_MAX_VOLUME_2dB/16,PSG_MAX_VOLUME_4dB/16,
    0};

static SN76489_Context gwenesis_SN76489;

void gwenesis_SN76489_Init( int PSGClockValue, int SamplingRate,int freq_divisor)
{
    gwenesis_SN76489.dClock=(float)PSGClockValue/16/SamplingRate;
    gwenesis_SN76489.divisor = freq_divisor;

    gwenesis_SN76489_Reset();
}

void gwenesis_SN76489_Reset()
{
    int i;

    for(i = 0; i <= 3; i++)
    {
        /* Initialise PSG state */
        gwenesis_SN76489.Registers[2*i] = 1;         /* tone freq=1 */
        gwenesis_SN76489.Registers[2*i+1] = 0xf;     /* vol=off */
        gwenesis_SN76489.NoiseFreq = 0x10;

        /* Set counters to 0 */
        gwenesis_SN76489.ToneFreqVals[i] = 0;

        /* Set flip-flops to 1 */
        gwenesis_SN76489.ToneFreqPos[i] = 1;

        /* Set intermediate positions to do-not-use value */
        gwenesis_SN76489.IntermediatePos[i] = LONG_MIN;
    }

    gwenesis_SN76489.LatchedRegister=0;

    /* Initialise noise generator */
    gwenesis_SN76489.NoiseShiftRegister=NoiseInitialState;

    /* Zero clock */
    gwenesis_SN76489.Clock=0;
    sn76489_index=0;
    sn76489_clock=0;

}

void gwenesis_SN76489_SetContext(uint8 *data)
{
    memcpy(&gwenesis_SN76489, data, sizeof(SN76489_Context));
}

void gwenesis_SN76489_GetContext(uint8 *data)
{
    memcpy(data, &gwenesis_SN76489, sizeof(SN76489_Context));
}

uint8 *gwenesis_SN76489_GetContextPtr()
{
    return (uint8 *)&gwenesis_SN76489;
}

int gwenesis_SN76489_GetContextSize(void)
{
    return sizeof(SN76489_Context);
}
static inline void gwenesis_SN76489_Update(INT16 *buffer, int length)
{
    int i, j;

    for(j = 0; j < length; j++)
    {
        for (i=0;i<=2;++i)
            if (gwenesis_SN76489.IntermediatePos[i]!=LONG_MIN)
                gwenesis_SN76489.Channels[i]=PSGVolumeValues[gwenesis_SN76489.Registers[2*i+1]]*gwenesis_SN76489.IntermediatePos[i]/65536;
            else
                gwenesis_SN76489.Channels[i]=PSGVolumeValues[gwenesis_SN76489.Registers[2*i+1]]*gwenesis_SN76489.ToneFreqPos[i];

        gwenesis_SN76489.Channels[3]=(short)(PSGVolumeValues[gwenesis_SN76489.Registers[7]]*(gwenesis_SN76489.NoiseShiftRegister & 0x1));

        gwenesis_SN76489.Channels[3]<<=1; /* Double noise volume to make some people happy */

        buffer[j] = (gwenesis_SN76489.Channels[0]);
        buffer[j] += (gwenesis_SN76489.Channels[1]);
        buffer[j] += (gwenesis_SN76489.Channels[2]);
        buffer[j] += (gwenesis_SN76489.Channels[3]);

        gwenesis_SN76489.Clock+=gwenesis_SN76489.dClock;
        gwenesis_SN76489.NumClocksForSample=(int)gwenesis_SN76489.Clock;  /* truncates */
        gwenesis_SN76489.Clock-=gwenesis_SN76489.NumClocksForSample;  /* remove integer part */

        /* Decrement tone channel counters */
        for (i=0;i<=2;++i)
            gwenesis_SN76489.ToneFreqVals[i]-=gwenesis_SN76489.NumClocksForSample;

        /* Noise channel: match to tone2 or decrement its counter */
        if (gwenesis_SN76489.NoiseFreq==0x80) gwenesis_SN76489.ToneFreqVals[3]=gwenesis_SN76489.ToneFreqVals[2];
        else gwenesis_SN76489.ToneFreqVals[3]-=gwenesis_SN76489.NumClocksForSample;

        /* Tone channels: */
        for (i=0;i<=2;++i) {
            if (gwenesis_SN76489.ToneFreqVals[i]<=0) {   /* If it gets below 0... */
                if (gwenesis_SN76489.Registers[i*2]>PSG_CUTOFF) {
                    /* Calculate how much of the sample is + and how much is - */
                    /* Go to floating point and include the clock fraction for extreme accuracy :D */
                    /* Store as long int, maybe it's faster? I'm not very good at this */
                    gwenesis_SN76489.IntermediatePos[i]=(long)((gwenesis_SN76489.NumClocksForSample-gwenesis_SN76489.Clock+2*gwenesis_SN76489.ToneFreqVals[i])*gwenesis_SN76489.ToneFreqPos[i]/(gwenesis_SN76489.NumClocksForSample+gwenesis_SN76489.Clock)*65536);
                    gwenesis_SN76489.ToneFreqPos[i]=-gwenesis_SN76489.ToneFreqPos[i]; /* Flip the flip-flop */
                } else {
                    gwenesis_SN76489.ToneFreqPos[i]=1;   /* stuck value */
                    gwenesis_SN76489.IntermediatePos[i]=LONG_MIN;
                }
                gwenesis_SN76489.ToneFreqVals[i]+=gwenesis_SN76489.Registers[i*2]*(gwenesis_SN76489.NumClocksForSample/gwenesis_SN76489.Registers[i*2]+1);
            } else gwenesis_SN76489.IntermediatePos[i]=LONG_MIN;
        }

        /* Noise channel */
        if (gwenesis_SN76489.ToneFreqVals[3]<=0) {   /* If it gets below 0... */
            gwenesis_SN76489.ToneFreqPos[3]=-gwenesis_SN76489.ToneFreqPos[3]; /* Flip the flip-flop */
            if (gwenesis_SN76489.NoiseFreq!=0x80)            /* If not matching tone2, decrement counter */
                gwenesis_SN76489.ToneFreqVals[3]+=gwenesis_SN76489.NoiseFreq*(gwenesis_SN76489.NumClocksForSample/gwenesis_SN76489.NoiseFreq+1);
            if (gwenesis_SN76489.ToneFreqPos[3]==1) {    /* Only once per cycle... */
                int Feedback;
                if (gwenesis_SN76489.Registers[6]&0x4) { /* White noise */
                    /* Calculate parity of fed-back bits for feedback */

                        /* If two bits fed back, I can do Feedback=(nsr & fb) && (nsr & fb ^ fb) */
                        /* since that's (one or more bits set) && (not all bits set) */
                        Feedback=((gwenesis_SN76489.NoiseShiftRegister&gwenesis_SN76489.WhiteNoiseFeedback) && ((gwenesis_SN76489.NoiseShiftRegister&gwenesis_SN76489.WhiteNoiseFeedback)^gwenesis_SN76489.WhiteNoiseFeedback));

                } else      /* Periodic noise */
                    Feedback=gwenesis_SN76489.NoiseShiftRegister&1;

                gwenesis_SN76489.NoiseShiftRegister=(gwenesis_SN76489.NoiseShiftRegister>>1) | (Feedback<<15);
            }
        }
    }
}
/* SN76589 execution */
extern int scan_line;
void gwenesis_SN76489_run(int target) {
 
if ( sn76489_clock >= target) return;

  int sn76489_prev_index = sn76489_index;
  sn76489_index += (target-sn76489_clock) / gwenesis_SN76489.divisor;
  if (sn76489_index > sn76489_prev_index) {
    gwenesis_SN76489_Update(gwenesis_sn76489_buffer + sn76489_prev_index, sn76489_index-sn76489_prev_index);
    sn76489_clock = sn76489_index*gwenesis_SN76489.divisor;
  } else {
    sn76489_index = sn76489_prev_index;
  }
}
void gwenesis_SN76489_Write(int data, int target)
{
  if (GWENESIS_AUDIO_ACCURATE == 1)
    gwenesis_SN76489_run(target);

  if (data & 0x80) {
    /* Latch/data byte  %1 cc t dddd */
    gwenesis_SN76489.LatchedRegister = ((data >> 4) & 0x07);
    gwenesis_SN76489.Registers[gwenesis_SN76489.LatchedRegister] =
        (gwenesis_SN76489.Registers[gwenesis_SN76489.LatchedRegister] &
         0x3f0)         /* zero low 4 bits */
        | (data & 0xf); /* and replace with data */
	} else {
        /* Data byte        %0 - dddddd */
        if (!(gwenesis_SN76489.LatchedRegister%2)&&(gwenesis_SN76489.LatchedRegister<5))
            /* Tone register */
            gwenesis_SN76489.Registers[gwenesis_SN76489.LatchedRegister]=
                (gwenesis_SN76489.Registers[gwenesis_SN76489.LatchedRegister] & 0x00f)    /* zero high 6 bits */
                | ((data&0x3f)<<4);                     /* and replace with data */
		else
            /* Other register */
            gwenesis_SN76489.Registers[gwenesis_SN76489.LatchedRegister]=data&0x0f;       /* Replace with data */
    }
    switch (gwenesis_SN76489.LatchedRegister) {
	case 0:
	case 2:
    case 4: /* Tone channels */
        if (gwenesis_SN76489.Registers[gwenesis_SN76489.LatchedRegister]==0) gwenesis_SN76489.Registers[gwenesis_SN76489.LatchedRegister]=1;    /* Zero frequency changed to 1 to avoid div/0 */
		break;
    case 6: /* Noise */
        gwenesis_SN76489.NoiseShiftRegister=NoiseInitialState;   /* reset shift register */
        gwenesis_SN76489.NoiseFreq=0x10<<(gwenesis_SN76489.Registers[6]&0x3);     /* set noise signal generator frequency */
		break;
    }
}

void gwenesis_sn76489_save_state() {
  SaveState* state;
  state = saveGwenesisStateOpenForWrite("sn76489");
  saveGwenesisStateSetBuffer(state, "gwenesis_SN76489", &gwenesis_SN76489, sizeof(gwenesis_SN76489));

}

void gwenesis_sn76489_load_state() {
  SaveState* state = saveGwenesisStateOpenForRead("sn76489");
  saveGwenesisStateGetBuffer(state, "gwenesis_SN76489", &gwenesis_SN76489, sizeof(gwenesis_SN76489));

}
