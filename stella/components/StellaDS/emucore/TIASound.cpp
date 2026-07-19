#include <stdint.h>
extern "C" uint32_t wave_direct_samples;
/*****************************************************************************/
/*                                                                           */
/* Module:  TIA Chip Sound Simulator                                         */
/* Purpose: To emulate the sound generation hardware of the Atari TIA chip.  */
/* Author:  Ron Fries                                                        */
/*                                                                           */
/* Revision History:                                                         */
/*    10-Sep-96 - V1.0 - Initial Release                                     */
/*    14-Jan-97 - V1.1 - Cleaned up sound output by eliminating counter      */
/*                       reset.                                              */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*                 License Information and Copyright Notice                  */
/*                 ========================================                  */
/*                                                                           */
/* TiaSound is Copyright(c) 1996 by Ron Fries                                */
/*                                                                           */
/* This library is free software; you can redistribute it and/or modify it   */
/* under the terms of version 2 of the GNU Library General Public License    */
/* as published by the Free Software Foundation.                             */
/*                                                                           */
/* This library is distributed in the hope that it will be useful, but       */
/* WITHOUT ANY WARRANTY; without even the implied warranty of                */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library */
/* General Public License for more details.                                  */
/* To obtain a copy of the GNU Library General Public License, write to the  */
/* Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.   */
/*                                                                           */
/* Any permitted reproduction of these routines, in whole or in part, must   */
/* bear this legend.                                                         */
/*                                                                           */
/*****************************************************************************/
#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Cart.hxx"
#include "System.hxx"
#include "TIASound.hxx"

/* CONSTANT DEFINITIONS */

/* definitions for AUDCx (15, 16) */
#define VOL_ONLY     0x00      /* 0000 */
#define POLY4        0x01      /* 0001 */
#define DIV31_POLY4  0x02      /* 0010 */
#define POLY5_POLY4  0x03      /* 0011 */
#define PURE         0x04      /* 0100 */
#define PURE2        0x05      /* 0101 */
#define DIV31_PURE   0x06      /* 0110 */
#define POLY5_2      0x07      /* 0111 */
#define POLY9        0x08      /* 1000 */
#define POLY5        0x09      /* 1001 */
#define DIV31_POLY5  0x0a      /* 1010 */
#define POLY5_POLY5  0x0b      /* 1011 */
#define DIV3_PURE    0x0c      /* 1100 */
#define DIV3_PURE2   0x0d      /* 1101 */
#define DIV93_PURE   0x0e      /* 1110 */
#define DIV3_POLY5   0x0f      /* 1111 */
                 
#define DIV3_MASK    0x0c                 
                 
#define AUDC0        0x15
#define AUDC1        0x16
#define AUDF0        0x17
#define AUDF1        0x18
#define AUDV0        0x19
#define AUDV1        0x1a

/* channel definitions */
#define CHAN1       0
#define CHAN2       1

#define FALSE       0
#define TRUE        1

/* LOCAL GLOBAL VARIABLE DEFINITIONS */

// wave_direct_samples moved to StellaDS.cpp

/* structures to hold the 6 tia sound control bytes */
uInt8 AUDC[2] ;    /* AUDCx (15, 16) */
uInt8 AUDF[2] ;    /* AUDFx (17, 18) */
uInt8 AUDV[2] ;    /* AUDVx (19, 1A) */

uInt8 bProcessingSample  = 0;

uInt32 Outvol[2] ;  /* last output volume for each channel */

/* Initialze the bit patterns for the polynomials. */

/* The 4bit and 5bit patterns are the identical ones used in the tia chip. */
/* Though the patterns could be packed with 8 bits per byte, using only a */
/* single bit per byte keeps the math simple, which is important for */
/* efficient processing. */

static uInt8 Bit4[POLY4_SIZE]  =
      { 0xff,0xff,0,0xff,0xff,0xff,0,0,0,0,0xff,0,0xff,0,0 };

static uInt8 Bit5[POLY5_SIZE]  =
      { 0,0,1,0,1,1,0,0,1,1,1,1,1,0,0,0,1,1,0,1,1,1,0,1,0,1,0,0,0,0,1 };

static uInt8 Bit5a[POLY5_SIZE]  =
      { 0,0,0xff,0,0xff,0xff,0,0,0xff,0xff,0xff,0xff,0xff,0,0,0,0xff,0xff,0,0xff,0xff,0xff,0,0xff,0,0xff,0,0,0,0,0xff };

/* I've treated the 'Div by 31' counter as another polynomial because of */
/* the way it operates.  It does not have a 50% duty cycle, but instead */
/* has a 13:18 ratio (of course, 13+18 = 31).  This could also be */
/* implemented by using counters. */

static uInt8 Div31[POLY5_SIZE]  =
      { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0 };

/* Rather than have a table with 511 entries, I use a random number generator. */
uInt8 Bit9[POLY9_SIZE];
    
uInt8  P4[2] ; /* Position pointer for the 4-bit POLY array */
uInt8  P5[2] ; /* Position pointer for the 5-bit POLY array */
uInt16 P9[2] ; /* Position pointer for the 9-bit POLY array */

uInt32 Div_n_cnt[2] ;  /* Divide by n counter. one for each channel */
uInt32 Div_n_max[2] ;  /* Divide by n maximum, one for each channel */

/* In my routines, I treat the sample output as another divide by N counter. */
/* For better accuracy, the Samp_n_cnt has a fixed binary decimal point */
/* which has 8 binary digits to the right of the decimal point. */

uInt32 Samp_n_max ; /* Sample max, multiplied by 256 */
uInt32 Samp_n_cnt ; /* Sample count */

uInt16 *sampleExtender = NULL;   // Will be allocated in Tia_sound_init

uInt16 tia_buf_idx   = 0;
uInt16 tia_out_idx   = 0;

uInt16 *tia_buf = NULL; // Will be allocated in Tia_sound_init
extern uInt16 *aptr;
extern uInt16 *bptr;

/*****************************************************************************/
/* Module:  Tia_sound_init()                                                 */
/* Purpose: to handle the power-up initialization functions                  */
/*          these functions should only be executed on a cold-restart        */
/*                                                                           */
/* Author:  Ron Fries                                                        */
/* Date:    September 10, 1996                                               */
/*                                                                           */
/* Inputs:  sample_freq - the value for the '30 Khz' Tia audio clock         */
/*          playback_freq - the playback frequency in samples per second     */
/*                                                                           */
/* Outputs: Adjusts local globals - no return value                          */
/*                                                                           */
/*****************************************************************************/

void Tia_sound_init (uInt16 sample_freq, uInt16 playback_freq)
{
   uInt8 chan;
   Int16 n;

   if (sampleExtender == NULL) sampleExtender = (uInt16 *)rg_alloc(512 * sizeof(uInt16), MEM_FAST);
   if (tia_buf == NULL) tia_buf = (uInt16 *)rg_alloc(SOUND_SIZE * sizeof(uInt16), MEM_FAST);
    
   /* fill the 9bit polynomial with random bits */
   for (n=0; n<POLY9_SIZE; n++)
   {
      Bit9[n] = ((rand() & 0x01) ? 0xFF:0x00);       /* fill poly9 with random bits */
   }

   /* calculate the sample 'divide by N' value based on the playback freq. */
   Samp_n_max = (uInt16)(((uInt32)sample_freq<<8)/playback_freq);
   Samp_n_cnt = 256;  /* initialize all bits of the sample counter */

   /* initialize the local globals */
   for (chan = CHAN1; chan <= CHAN2; chan++)
   {
      Outvol[chan] = 0;
      Div_n_cnt[chan] = 0;
      Div_n_max[chan] = 0;
      AUDC[chan] = 0;
      AUDF[chan] = 0;
      AUDV[chan] = 0;
      P4[chan] = 0;
      P5[chan] = 0;
      P9[chan] = 0;
   }
    
   memset(tia_buf, 0x00, SOUND_SIZE * (sizeof(uInt16)));
    
   bProcessingSample = false;
    
   tia_buf_idx = tia_out_idx = 0;

    // A bit of speed-up to have this pre-computed
    for (uInt16 i=0; i<256; i++)
    {
        sampleExtender[i] = (uInt16) ((i << 8) / 2);
        sampleExtender[256+i] = (uInt16) ((i << 8));
    }    
}

/*****************************************************************************/
/* Module:  Update_tia_sound()                                               */
/* Purpose: To process the latest control values stored in the AUDF, AUDC,   */
/*          and AUDV registers.  It pre-calculates as much information as    */
/*          possible for better performance.  This routine has not been      */
/*          optimized.                                                       */
/*                                                                           */
/* Author:  Ron Fries                                                        */
/* Date:    January 14, 1997                                                 */
/*                                                                           */
/* Inputs:  addr - the address of the parameter to be changed                */
/*          val - the new value to be placed in the specified address        */
/*                                                                           */
/* Outputs: Adjusts local globals - no return value                          */
/*                                                                           */
/*****************************************************************************/
 void Update_tia_sound_0 (void)
{
    uInt16 new_val;
    
   /* an AUDC value of 0 is a special case */
   if (AUDC[0] == VOL_ONLY)
   {
      /* indicate the clock is zero so no processing will occur */
      new_val = 0;

      /* and set the output to the selected volume */
      Outvol[0] = AUDV[0];
   }
   else
   {
      /* otherwise calculate the 'divide by N' value */
      new_val = AUDF[0] + 1;

      /* if bits 2 & 3 are set, then multiply the 'div by n' count by 3 */
      if ((AUDC[0] & DIV3_MASK) == DIV3_MASK)
      {
         new_val *= 3;
      }
   }

   /* only reset those channels that have changed */
   if (new_val != Div_n_max[0])
   {
      /* reset the divide by n counters */
      Div_n_max[0] = new_val;

      /* if the channel is now volume only or was volume only */
      if ((Div_n_cnt[0] == 0) || (new_val == 0))
      {
         /* reset the counter (otherwise let it complete the previous) */
         Div_n_cnt[0] = new_val;
      }
   }}

 void Update_tia_sound_1 (void)
{
    uInt16 new_val;
    
   /* an AUDC value of 0 is a special case */
   if (AUDC[1] == VOL_ONLY)
   {
      /* indicate the clock is zero so no processing will occur */
      new_val = 0;

      /* and set the output to the selected volume */
      Outvol[1] = AUDV[1];
   }
   else
   {
      /* otherwise calculate the 'divide by N' value */
      new_val = AUDF[1] + 1;

      /* if bits 2 & 3 are set, then multiply the 'div by n' count by 3 */
      if ((AUDC[1] & DIV3_MASK) == DIV3_MASK)
      {
         new_val *= 3;
      }
   }

   /* only reset those channels that have changed */
   if (new_val != Div_n_max[1])
   {
      /* reset the divide by n counters */
      Div_n_max[1] = new_val;

      /* if the channel is now volume only or was volume only */
      if ((Div_n_cnt[1] == 0) || (new_val == 0))
      {
         /* reset the counter (otherwise let it complete the previous) */
         Div_n_cnt[1] = new_val;
      }
   }
}

/*****************************************************************************/
/* Module:  Tia_process()                                                    */
/* Purpose: To fill the output buffer with the sound output based on the     */
/*          tia chip parameters.  This routine has been optimized.           */
/*                                                                           */
/* Author:  Ron Fries                                                        */
/* Date:    September 10, 1996                                               */
/*                                                                           */
/* Inputs:  *buffer - pointer to the buffer where the audio output will      */
/*                    be placed                                              */
/*          n - size of the playback buffer                                  */
/*                                                                           */
/* Outputs: the buffer will be filled with n bytes of audio - no return val  */
/*                                                                           */
/*****************************************************************************/
 void Tia_process(void)
{
    int samplesGenerated = 0;
    bProcessingSample = true;
    /* loop until the buffer is filled */
    while (1)
    {
       /* Process channel 0 */
       if (Div_n_cnt[0] > 1)
       {
          Div_n_cnt[0]--;
       }
       else if (Div_n_cnt[0] == 1)
       {
          Div_n_cnt[0] = Div_n_max[0];

          if (++P5[0] == POLY5_SIZE) P5[0] = 0;
           
          /* check clock modifier for clock tick */
          if  ( ((AUDC[0] & 0x02) == 0) ||
                (((AUDC[0] & 0x01) == 0) && Div31[P5[0]]) ||
                (((AUDC[0] & Bit5[P5[0]]))) )
          {
             if (AUDC[0] & 0x04)       /* pure modified clock selected */
             {
                 Outvol[0] = (Outvol[0] ? 0:AUDV[0]);  // Toggle outvol
             }
             else if (AUDC[0] & 0x08)
             {
                 if (AUDC[0] == POLY9)    /* check for poly9 */
                 {
                    if (++P9[0] == POLY9_SIZE) P9[0]=0;
                    Outvol[0] = Bit9[P9[0]] & AUDV[0];
                 }
                 else // Must be poly5
                 {
                    Outvol[0] = (Bit5a[P5[0]] & AUDV[0]);
                 }
             }
             else  /* poly4 is the only remaining option */
             {
                if (++P4[0] == POLY4_SIZE) P4[0] = 0;
                Outvol[0] = (Bit4[P4[0]] & AUDV[0]);
             }
          }
       }


       /* Process channel 1 */
       if (Div_n_cnt[1] > 1)
       {
          Div_n_cnt[1]--;
       }
       else if (Div_n_cnt[1] == 1)
       {
          Div_n_cnt[1] = Div_n_max[1];

          if (++P5[1] == POLY5_SIZE) P5[1] = 0;
           
          /* check clock modifier for clock tick */
          if  ( ((AUDC[1] & 0x02) == 0) ||
                (((AUDC[1] & 0x01) == 0) && Div31[P5[1]]) ||
                (((AUDC[1] & Bit5[P5[1]]))) )
          {
             if (AUDC[1] & 0x04)       /* pure modified clock selected */
             {
                 Outvol[1] = (Outvol[1] ? 0:AUDV[1]);  // Toggle outvol
             }
             else if (AUDC[1] & 0x08)
             {
                 if (AUDC[1] == POLY9)    /* check for poly9 */
                 {
                    if (++P9[1] == POLY9_SIZE) P9[1]=0;
                    Outvol[1] = Bit9[P9[1]] & AUDV[1];
                 }
                 else // Must be poly5
                 {
                    Outvol[1] = (Bit5a[P5[1]] & AUDV[1]);
                 }
             }
             else  /* poly4 is the only remaining option */
             {
                if (++P4[1] == POLY4_SIZE) P4[1] = 0;
                Outvol[1] = (Bit4[P4[1]] & AUDV[1]);
             }
          }
       }

       /* decrement the sample counter - value is 256 since the lower
          byte contains the fractional part */
       Samp_n_cnt -= 256;

       /* if the count down has reached zero */
       if (Samp_n_cnt & 0xFF00) continue;
        
      /* adjust the sample counter */
      Samp_n_cnt += Samp_n_max;

      /* calculate the latest output value and place in buffer
         scale the volume by 128, since this is the default silence value
         when using unsigned 8-bit samples in SDL */
        if (myCartInfo.soundQuality == SOUND_WAVE)
        {
            uInt16 next_idx = ((tia_buf_idx+1) & (SOUND_SIZE-1));
            if (next_idx == tia_out_idx) break; // Buffer full
            
            tia_buf[tia_buf_idx] = sampleExtender[Outvol[0] + Outvol[1]];
            tia_buf_idx = next_idx;
            samplesGenerated++;
        }
        else
        {
            *aptr = *bptr = sampleExtender[Outvol[0] + Outvol[1]];
            samplesGenerated++;
            break;
        }
    }
    bProcessingSample = false;
}

// Called from main loop
 void Tia_process_wave (void)
{
    wave_direct_samples++;
    if (tia_out_idx == tia_buf_idx) 
    {
        if (!bProcessingSample) Tia_process();
        if (tia_out_idx == tia_buf_idx) return;
    }
    *aptr = *bptr = tia_buf[tia_out_idx++];
    tia_out_idx &= (SOUND_SIZE-1);
}
