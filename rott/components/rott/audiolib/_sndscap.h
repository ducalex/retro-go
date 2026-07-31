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
   module: _SNDSCAP.H

   author: James R. Dose
   date:   October 26, 1994

   Private header for SNDSCAPE.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef ___SNDSCAP_H
#define ___SNDSCAP_H

#define VALID   ( 1 == 1 )
#define INVALID ( !VALID )

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

#define lobyte( num )   ( ( int )*( ( char * )&( num ) ) )
#define hibyte( num )   ( ( int )*( ( ( char * )&( num ) ) + 1 ) )

#define STEREO      1
#define SIXTEEN_BIT 2

#define MONO_8BIT    0
#define STEREO_8BIT  ( STEREO )
#define MONO_16BIT   ( SIXTEEN_BIT )
#define STEREO_16BIT ( STEREO | SIXTEEN_BIT )

#define SOUNDSCAPE_MaxMixMode        STEREO_16BIT

#define MONO_8BIT_SAMPLE_SIZE    1
#define MONO_16BIT_SAMPLE_SIZE   2
#define STEREO_8BIT_SAMPLE_SIZE  ( 2 * MONO_8BIT_SAMPLE_SIZE )
#define STEREO_16BIT_SAMPLE_SIZE ( 2 * MONO_16BIT_SAMPLE_SIZE )

#define SOUNDSCAPE_DefaultSampleRate 11000
#define SOUNDSCAPE_DefaultMixMode    MONO_8BIT
#define SOUNDSCAPE_MaxIrq            15

/* Ensoniq gate-array chip defines ... */
#define ODIE            0       /* ODIE gate array */
#define OPUS            1       /* OPUS gate array */
#define MMIC            2       /* MiMIC gate array */

/* relevant direct register defines - offsets from base address */
#define GA_HOSTCSTAT    2       /* host port ctrl/stat reg */
#define GA_HOSTDATA     3       /* host port data reg */
#define GA_REGADDR      4       /* indirect address reg */
#define GA_REGDATA      5       /* indirect data reg */
#define SB_IACK      0x22e /* SoundBlaster IACK register */

/* relevant indirect register defines */
#define GA_DMACHB       3       /* DMA chan B assign reg */
#define GA_INTCFG       4       /* interrupt configuration reg */
#define GA_DMACFG       5       /* DMA config reg */
#define GA_CDCFG        6       /* CD-ROM (AD-1848) config reg */
#define GA_HMCTL  9  /* host master control reg */
#define GA_CHIPSEL      10      /* programmable external chip select */

/* AD-1848 chip defines ... */
/* relevant direct register defines */
#define AD_REGADDR      0       /* indirect address reg */
#define AD_REGDATA      1       /* indirect data reg */
#define AD_STATUS       2       /* status register */
#define AD_OFFSET       8       /* for some boards, a fixed BasePort offset */

/* relevant indirect register defines */
#define AD_LEFTOUT      6       /* left DAC output control reg */
#define AD_RIGHTOUT     7       /* right DAC output control reg */
#define AD_FORMAT       8       /* clock and data format reg */
#define AD_CONFIG       9       /* interface config register */
#define AD_PINCTRL      10      /* external pin control reg */
#define AD_UCOUNT       14      /* upper count reg */
#define AD_LCOUNT       15      /* lower count reg */

/* some firmware command and communication defines */
#define SET_CTL      0x88  /* set a control value */
#define GET_CTL      0x89  /* get a control value */
#define SET_REV      0xb0  /* set synth reverb */
#define SYNTH_VOL 0x04  /* Synth Vol control number */
#define RXRDY     0x01  /* Receive-Ready bit mask */
#define TXRDY     0x02  /* Transmit-Ready bit mask */

/* some miscellaneous defines ... soundscape reg values, sytem int regs, ... */
#define INTCONT1        0x20    /* Interrupt Controller 1 control register */
#define INTCONT2        0xa0    /* Interrupt Controller 2 control register */
#define INTMASK1        0x21    /* Interrupt Controller 1 mask register */
#define INTMASK2        0xa1    /* Interrupt Controller 2 mask register */
#define VECTBASE1       0x08    /* vector base for XT interrupts */
#define VECTBASE2       0x70    /* vector base for AT extended interrupts */
#define EOI             0x20    /* End Of Interrupt command */
#define AUTO_OUT        0x58    /* DMA controller mode */

static void SOUNDSCAPE_EnableInterrupt( void );
static void SOUNDSCAPE_DisableInterrupt( void );
static void __interrupt __far SOUNDSCAPE_ServiceInterrupt( void );
static int  ga_read( int rnum );
static void ga_write( int rnum, int value );
static int  ad_read( int rnum );
static void ad_write( int rnum, int value );
static void tdelay( void );
static void pcm_format( void );
static int  SOUNDSCAPE_SetupDMABuffer( char *BufferPtr, int BufferSize, int mode );
static int  SOUNDSCAPE_BeginPlayback( int length );
static void SOUNDSCAPE_LockEnd( void );
static void SOUNDSCAPE_UnlockMemory( void );
static int  SOUNDSCAPE_LockMemory( void );
static unsigned short allocateTimerStack( unsigned short size );
static void deallocateTimerStack( unsigned short selector );
static int  parse( char *val, char *str, FILE *p1 );
static int  SOUNDSCAPE_FindCard( void );
static int  SOUNDSCAPE_Setup( void );

#endif
