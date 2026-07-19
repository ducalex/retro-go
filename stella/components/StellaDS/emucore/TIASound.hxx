/*****************************************************************************/
/*                                                                           */
/* Module:  TIA Chip Sound Simulator Includes, V1.1                          */
/* Purpose: Define global function prototypes and structures for the TIA     */
/*          Chip Sound Simulator.                                            */
/* Author:  Ron Fries                                                        */
/*                                                                           */
/* Revision History:                                                         */
/*    10-Sep-96 - V1.0 - Initial Release                                     */
/*    14-Jan-97 - V1.1 - Added compiler directives to facilitate compilation */
/*                       on a C++ compiler.                                  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*                 License Information and Copyright Notice                  */
/*                 ========================================                  */
/*                                                                           */
/* TiaSound is Copyright(c) 1997 by Ron Fries                                */
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

#ifndef _TIASOUND_H
#define _TIASOUND_H

/* the size (in entries) of the 4 polynomial tables */
#define POLY4_SIZE  0x000f
#define POLY5_SIZE  0x001f
#define POLY9_SIZE  0x01ff


void Tia_sound_init (unsigned short sample_freq, unsigned short playback_freq);
void Update_tia_sound_0(void);
void Update_tia_sound_1(void);
void Update_tia_sound (uInt8 chan);
void Tia_process (void);
void Tia_process_wave(void);

extern unsigned char AUDC[2];
extern unsigned char AUDF[2];
extern unsigned char AUDV[2];
extern uInt8  bProcessingSample;
extern uInt32 Outvol[2];
extern uInt16 tia_buf_idx;
extern uInt16 tia_out_idx;

extern uInt8  Bit9[POLY9_SIZE];
extern uInt8  P4[2];
extern uInt8  P5[2];
extern uInt16 P9[2];
extern uInt32 Div_n_cnt[2];
extern uInt32 Div_n_max[2];
extern uInt32 Samp_n_max;
extern uInt32 Samp_n_cnt;

extern uInt16 *tia_buf;

#endif
