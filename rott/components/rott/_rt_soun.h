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
#ifndef _rt_sound_private
#define _rt_sound_private

#define SOUNDTYPES 2

typedef struct
{
   int     snds[SOUNDTYPES];
   byte    flags;
   byte    priority;
   byte    count;
   int     prevhandle;
   int     prevdistance;
} sound_t;

#define SD_OVERWRITE  0x01
#define SD_WRITE      0x02
#define SD_LOOP       0x04
#define SD_PITCHSHIFTOFF 0x08
#define SD_PLAYONCE      0x10

enum {
   sd_prio0=17,
   sd_prio1=16,
   sd_prio2=15,
   sd_prio3=14,
   sd_prio4=13,
   sd_prio5=12,
   sd_prio6=11,
   sd_prio7=10,
   sd_prio8=9,
   sd_prio9=8,
   sd_prio10=7,
   sd_prio11=6,
   sd_prio12=5,
   sd_prio13=4,
   sd_prio14=3,
   sd_prio15=2,
   sd_prio16=1
} ;

#define SD_PRIOSECRET sd_prio1
#define SD_PRIOREMOTE sd_prio1
#define SD_PRIOPHURT  sd_prio1
#define SD_PRIOGAME   sd_prio2
#define SD_PRIOBOSS   sd_prio2
#define SD_PRIOGODDOG sd_prio2
#define SD_PRIOEXPL   sd_prio3
#define SD_PRIOASNEAK sd_prio3
#define SD_PRIOADEATH sd_prio3
#define SD_PRIOPMISS  sd_prio4
#define SD_PRIOPGUNS  sd_prio5
#define SD_PRIOAFIRE  sd_prio6
#define SD_PRIOPMP40  sd_prio7
#define SD_PRIOAHURT  sd_prio8
#define SD_PRIOGLASS  sd_prio9
#define SD_PRIOPSNDS  sd_prio11
#define SD_PRIOPCAUSD sd_prio12
#define SD_PRIOAGREET sd_prio13
#define SD_PRIOENVRON sd_prio14
#define SD_PRIOQUIT   sd_prio15
#define SD_PRIOMENU   sd_prio16

#define SD_DISTANCESHIFT 12

#define USEADLIB 255

#define SD_RANDOMSHIFT 1

#define PitchOffset()  ((RandomNumber("Pitch Offset",0)-128)>>SD_RANDOMSHIFT)

#define SoundOffset(x) (sounds[x].snds[soundtype])

#define GUSMIDIINIFILE ("gusmidi.ini")


typedef enum {
   loop_yes,
   loop_no
} looptypes;


typedef struct
{
   byte    loopflag;
   byte    songtype;
   char    lumpname[9];
   char    songname[40];
} song_t;

int SD_PlayIt ( int sndnum, int angle, int distance, int pitch );
boolean SD_SoundOkay ( int sndnum );

#endif

