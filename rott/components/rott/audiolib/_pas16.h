/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/**********************************************************************
   module: _PAS16.H

   author: James R. Dose
   date:   March 27, 1994

   Private header for for PAS16.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef ___PAS16_H
#define ___PAS16_H

#define TRUE    ( 1 == 1 )
#define FALSE   ( !TRUE )

#define VALID   ( 1 == 1 )
#define INVALID ( !VALID )

#define lobyte( num )   ( ( int )*( ( char * )&( num ) ) )
#define hibyte( num )   ( ( int )*( ( ( char * )&( num ) ) + 1 ) )

#define STEREO      1
#define SIXTEEN_BIT 2

#define MONO_8BIT    0
#define STEREO_8BIT  ( STEREO )
#define MONO_16BIT   ( SIXTEEN_BIT )
#define STEREO_16BIT ( STEREO | SIXTEEN_BIT )

#define PAS_MaxMixMode        STEREO_16BIT

#define MONO_8BIT_SAMPLE_SIZE    1
#define MONO_16BIT_SAMPLE_SIZE   2
#define STEREO_8BIT_SAMPLE_SIZE  ( 2 * MONO_8BIT_SAMPLE_SIZE )
#define STEREO_16BIT_SAMPLE_SIZE ( 2 * MONO_16BIT_SAMPLE_SIZE )

#define PAS_RevisionBits        0xe0

#define AudioFilterControl      0xb8a
#define InterruptControl        0xb8b
#define InterruptStatus         0xb89
#define PCMDataRegister         0xf88
#define CrossChannelControl     0xf8a
#define SampleRateTimer         0x1388
#define SampleBufferCount       0x1389
#define LocalSpeakerTimerCount  0x138a
#define LocalTimerControl       0x138b
#define SampleSizeConfiguration 0x8389

#define AudioMuteFlag             0x20
#define SampleRateTimerGateFlag   0x40
#define SampleBufferCountGateFlag 0x80

#define SampleRateInterruptFlag   0x04
#define SampleBufferInterruptFlag 0x08

#define PAS_SampleSizeMask     0xf3
#define PAS_SignedSampleMask   0xe3
#define PAS_16BitSampleFlag    0x04
#define PAS_UnsignedSampleFlag 0x10
//bSC2msbinv   equ   00010000b   ;; invert MSB from standard method

#define PAS_OverSamplingMask 0xfc

#define PAS_1xOverSampling  0x00
#define PAS_2xOverSampling  0x01
#define PAS_4xOverSampling  0x03

#define PAS_StereoFlag      0x20

#define PAS_AudioMuteFlag  0x20

#define DEFAULT_BASE ( 0x0388 ^ 0x388 ) /* default base I/O address */
#define ALT_BASE_1   ( 0x0384 ^ 0x388 ) /* first alternate address  */
#define ALT_BASE_2   ( 0x038C ^ 0x388 ) /* second alternate address */
#define ALT_BASE_3   ( 0x0288 ^ 0x388 ) /* third alternate address  */

#define PAS_DMAEnable          0x80
#define PAS_ChannelConnectMask 0x0f
#define PAS_PCMStartDAC        0xD0
#define PAS_PCMStartADC        0xC0
#define PAS_PCMStopMask        0x3f

#define RECORD   0
#define PLAYBACK 1

#define SelectSampleRateTimer   0x36   // 00110110b
#define SelectSampleBufferCount 0x74   // 01110100b

#define CalcTimeInterval( rate ) \
   ( 1193180UL / ( rate ) )

#define CalcSamplingRate( interval ) \
   ( 1193180UL / ( interval ) )

#define MV_Signature                 0x4d56
#define MV_SoundInt                  0x2f
#define MV_CheckForDriver            0xbc00
#define MV_GetVersion                0xbc01
#define MV_GetPointerToStateTable    0xbc02
#define MV_GetPointerToFunctionTable 0xbc03
#define MV_GetDmaIrqInt              0xbc04
#define MV_SendCommandStructure      0xbc05
#define MV_GetDriverMessage          0xbc06
#define MV_SetHotkeyScanCodes        0xbc0a
#define MV_GetPathToDriver           0xbc0b

#define OUTPUTMIXER     0x00         /* output mixer H/W select */
#define INPUTMIXER      0x40         /* input mixer select      */
#define DEFMIXER        -1           /* use last mixer selected   */

/* left channel values */

#define L_FM            0x01
#define L_IMIXER        0x02
#define L_EXT           0x03
#define L_INT           0x04
#define L_MIC           0x05
#define L_PCM           0x06
#define L_SPEAKER       0x07
#define L_FREE          0x00
#define L_SBDAC         0x00

/* right channel values */

#define R_FM            0x08
#define R_IMIXER        0x09
#define R_EXT           0x0A
#define R_INT           0x0B
#define R_MIC           0x0C
#define R_PCM           0x0D
#define R_SPEAKER       0x0E
#define R_FREE          0x0F
#define R_SBDAC         0x0F

typedef struct
   {
   unsigned char  sysspkrtmr;   /*   42 System Speaker Timer Address */
   unsigned char  systmrctlr;   /*   43 System Timer Control         */
   unsigned char  sysspkrreg;   /*   61 System Speaker Register      */
   unsigned char  joystick;     /*  201 Joystick Register            */
   unsigned char  lfmaddr;      /*  388 Left  FM Synth Address       */
   unsigned char  lfmdata;      /*  389 Left  FM Synth Data          */
   unsigned char  rfmaddr;      /*  38A Right FM Synth Address       */
   unsigned char  rfmdata;      /*  38B Right FM Synth Data          */
   unsigned char  dfmaddr;      /*  788 Dual  FM Synth Address       */
   unsigned char  dfmdata;      /*  789 Dual  FM Synth Data          */
   unsigned char  RESRVD1[1];   /*      reserved                     */
   unsigned char  paudiomixr;   /*  78B Paralllel Audio Mixer Control*/
   unsigned char  audiomixr;    /*  B88 Audio Mixer Control          */
   unsigned char  intrctlrst;   /*  B89 Interrupt Status             */
   unsigned char  audiofilt;    /*  B8A Audio Filter Control         */
   unsigned char  intrctlr;     /*  B8B Interrupt Control            */
   unsigned char  pcmdata;      /*  F88 PCM Data I/O Register        */
   unsigned char  RESRVD2;      /*      reserved                     */
   unsigned char  crosschannel; /*  F8A Cross Channel                */
   unsigned char  RESRVD3;      /*      reserved                     */
   unsigned short samplerate;   /* 1388 Sample Rate Timer            */
   unsigned short samplecnt;    /* 1389 Sample Count Register        */
   unsigned short spkrtmr;      /* 138A Shadow Speaker Timer Count   */
   unsigned char  tmrctlr;      /* 138B Shadow Speaker Timer Control */
   unsigned char  mdirqvect;    /* 1788 MIDI IRQ Vector Register     */
   unsigned char  mdsysctlr;    /* 1789 MIDI System Control Register */
   unsigned char  mdsysstat;    /* 178A MIDI IRQ Status Register     */
   unsigned char  mdirqclr;     /* 178B MIDI IRQ Clear Register      */
   unsigned char  mdgroup1;     /* 1B88 MIDI Group #1 Register       */
   unsigned char  mdgroup2;     /* 1B89 MIDI Group #2 Register       */
   unsigned char  mdgroup3;     /* 1B8A MIDI Group #3 Register       */
   unsigned char  mdgroup4;     /* 1B8B MIDI Group #4 Register       */
   } MVState;

typedef struct
   {
   unsigned long SetMixer;
   unsigned long SetVolume;
   unsigned long SetFilter;
   unsigned long SetCrossChannel;
   unsigned long GetMixer;
   unsigned long GetVolume;
   unsigned long GetFilter;
   unsigned long GetCrossChannel;
   unsigned long ReadSound;
   unsigned long FMSplit;
   } MVFunc;

int     PAS_CheckForDriver( void );
MVState *PAS_GetStateTable( void );
MVFunc  *PAS_GetFunctionTable( void );
int     PAS_GetCardSettings( void );
void    PAS_EnableInterrupt( void );
void    PAS_DisableInterrupt( void );
void    interrupt far PAS_ServiceInterrupt( void );
//void    interrupt PAS_ServiceInterrupt( void );
void    PAS_Write( int Register, int Data );
int     PAS_Read( int Register );
void    PAS_SetSampleRateTimer( void );
void    PAS_SetSampleBufferCount( void );
int     PAS_SetupDMABuffer( char *BufferPtr, int BufferSize, int mode );
int     PAS_GetFilterSetting( int rate );
void    PAS_BeginTransfer( int mode );
int     PAS_TestAddress( int address );
int     PAS_FindCard( void );
int     PAS_CallMVFunction( unsigned long function, int ebx, int ecx, int edx );
void    PAS_SaveState( void );
void    PAS_RestoreState( void );


#pragma aux PAS_TestAddress = \
   "mov   dx, 0b8bh", \
   "xor    dx, ax", \
   "in    al, dx", \
   "cmp   al, 0ffh", \
   "je    TestExit", \
   "mov   ah, al", \
   "xor   al, 0e0h", \
   "out   dx, al", \
   "jmp   TestDelay1", \
   "TestDelay1:", \
   "jmp   TestDelay2", \
   "TestDelay2:", \
   "in    al, dx", \
   "xchg  al, ah", \
   "out   dx, al", \
   "sub   al, ah", \
   "TestExit:", \
   "and   eax, 0ffh" \
   parm [ eax ] modify exact [ eax dx ];

#endif
