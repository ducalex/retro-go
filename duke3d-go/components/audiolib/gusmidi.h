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
#ifndef __GUSMIDI_H
#define __GUSMIDI_H

extern struct gf1_dma_buff GUS_HoldBuffer;

enum GUS_Errors
   {
   GUS_Warning        = -2,
   GUS_Error          = -1,
   GUS_Ok             = 0,
   GUS_OutOfMemory,
   GUS_OutOfDosMemory,
   GUS_OutOfDRAM,
   GUS_GF1Error,
   GUS_InvalidIrq,
   GUS_ULTRADIRNotSet,
   GUS_MissingConfig,
   GUS_FileError
   };

char *GUS_ErrorString( int ErrorNumber );
int   GUS_GetPatchMap( char *name );
int   GUSMIDI_UnloadPatch( int prog );
int   GUSMIDI_LoadPatch( int prog );
void  GUSMIDI_ProgramChange( int channel, int prog );
void  GUSMIDI_NoteOn( int chan, int note, int velocity );
void  GUSMIDI_NoteOff( int chan, int note, int velocity );
void  GUSMIDI_ControlChange( int channel, int number, int value );
void  GUSMIDI_PitchBend( int channel, int lsb, int msb );
void  GUSMIDI_ReleasePatches( void );
void  GUSMIDI_SetVolume( int volume );
int   GUSMIDI_GetVolume( void );
int   GUS_Init( void );
void  GUS_Shutdown( void );
#pragma aux GUS_Shutdown frame;
int   GUSMIDI_Init( void );
void  GUSMIDI_Shutdown( void );
void *D32DosMemAlloc( unsigned size );

#endif
