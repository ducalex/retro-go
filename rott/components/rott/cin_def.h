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
#ifndef _cin_def_public
#define _cin_def_public

#define MAXCINEMATICEVENTS 100
#define MAXCINEMATICACTORS 30

#define FRACTIONBITS (16)
#define FRACTIONUNIT (1<<FRACTIONBITS)

typedef enum {
        background_noscrolling,
        background_scrolling,
        background_multi,
        sprite_background,
        sprite_foreground,
        backdrop_scrolling,
        backdrop_noscrolling,
        palette,
        flic,
        fadeout,
        cinematicend,
        blankscreen,
        clearbuffer
} enum_eventtype;

typedef struct eventtype
   {
   int time;
   enum_eventtype effecttype;
   void * effect;
   struct eventtype * next;
   struct eventtype * prev;
   } eventtype;

typedef struct actortype
   {
   enum_eventtype effecttype;
   void * effect;
   struct actortype * next;
   struct actortype * prev;
   } actortype;

typedef struct
   {
   char name[10];
   boolean loop;
   boolean usefile;
   } flicevent;

typedef struct
   {
   char name[10];
   int duration;
   int frame;
   int frametime;
   int numframes;
   int framedelay;
   int x;
   int y;
   int scale;
   int dx;
   int dy;
   int dscale;
   } spriteevent;

typedef struct
   {
   char name[10];
   int duration;
   int backdropwidth;
   int currentoffset;
   int dx;
   int yoffset;
   int height;
   byte * data;
   } backevent;

typedef struct
   {
   char name[10];
   } paletteevent;


#endif

