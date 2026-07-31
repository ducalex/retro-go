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
   module: ADLIBFX.H

   author: James R. Dose
   date:   April 1, 1994

   Public header for ADLIBFX.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __ADLIBFX_H
#define __ADLIBFX_H

enum ADLIBFX_Errors
   {
   ADLIBFX_Warning = -2,
   ADLIBFX_Error   = -1,
   ADLIBFX_Ok      = 0,
   ADLIBFX_NoVoices,
   ADLIBFX_VoiceNotFound,
   ADLIBFX_DPMI_Error
   };

typedef	struct
   {
   unsigned long  length;
   short int      priority;
   char           mChar, cChar;
   char           mScale, cScale;
   char           mAttack, cAttack;
   char           mSus, cSus;
   char           mWave, cWave;
   char           nConn;
   char           voice;
   char           mode;
   char           unused[ 3 ];
   char           block;
   char           data[];
   } ALSound;

#define ADLIBFX_MaxVolume      255
#define ADLIBFX_MinVoiceHandle 1

char *ADLIBFX_ErrorString( int ErrorNumber );
int   ADLIBFX_Stop( int handle );
int   ADLIBFX_SetVolume( int handle, int volume );
int   ADLIBFX_SetTotalVolume( int volume );
int   ADLIBFX_GetTotalVolume( void );
int   ADLIBFX_VoiceAvailable( int priority );
int   ADLIBFX_Play( ALSound *sound, int volume, int priority, unsigned long callbackval );
int   ADLIBFX_SoundPlaying( int handle );
void  ADLIBFX_SetCallBack( void ( *function )( unsigned long ) );
int   ADLIBFX_Init( void );
int   ADLIBFX_Shutdown( void );
   #pragma aux ADLIBFX_Shutdown frame;
void  PCFX_UnlockMemory( void );
   #pragma aux ADLIBFX_UnlockMemory frame;
int   PCFX_LockMemory( void );

#endif
