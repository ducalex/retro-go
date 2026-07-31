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
/* Readflic.h - file containing prototypes and other
 * structural info for readflic program.
 *
 * Copyright (c) 1992 Jim Kent.  This file may be freely used, modified,
 * copied and distributed.  This file was first published as part of
 * an article for Dr. Dobb's Journal March 1993 issue.
 */

/* Some handy macros I use in lots of programs: */

#define ArrayEls(a) (sizeof(a)/sizeof((a)[0]))
	/* Count up number of elements in an array */

#define ClearMem(buf,size)	memset(buf, 0, size)
	/* Clear a block of memory. */

#define ClearStruct(pt)	ClearMem(pt, sizeof(*(pt)))
	/* Clear a structure (pass in pointer) */


/* Data structures peculiar to readflic program: */

typedef struct
	{
	FlicHead head;	/* Flic file header. */
	int handle;		/* File handle. */
	int frame;		/* Current frame in flic. */
	char *name;		/* Name from flic_open.  Helps error reporting. */
	int xoff,yoff;	/* Offset to display flic at. */
   MemPtr flicbuffer; // address of flicbuffer in memory
   int flicoffset;
   Boolean usefile;
	} Flic;


/* Prototypes peculiar to readflic program: */

ErrCode flic_open(Flic *flic, char *name, MemPtr buf, Boolean usefile);
	/* Open flic file.  Read header and verify it's a flic. */

void flic_close(Flic *flic);
	/* Close flic file and scrub flic. */

ErrCode flic_play_once(Flic *flic, Machine *machine);
	/* Play a flic through once. */

ErrCode flic_play_loop(Flic *flic, Machine *machine);
	/* Play a flic until key is pressed. */

ErrCode flic_next_frame(Flic *flic, Screen *screen);
	/* Advance to next frame of flic. */

ErrCode SetupFlicAccess (Flic * flic);

ErrCode CopyNextFlicBlock (Flic * flic, MemPtr buf, Ulong size);

void    SetFlicOffset (Flic * flic, Ulong offset );


char *flic_err_string(ErrCode err);

/* Various error codes flic reader can get. */
#define ErrNoMemory	-2		/* Not enough memory. */
#define ErrBadFlic	-3		/* File isn't a flic. */
#define ErrBadFrame	-4		/* Bad frame in flic. */
#define ErrOpen		-5		/* Couldn't open file.  Check errno. */
#define ErrRead		-6		/* Couldn't read file.  Check errno. */
#define ErrDisplay	-7		/* Couldn't open display. */
#define ErrClock	-8		/* Couldn't open clock. */
#define ErrKey		-9		/* Couldn't open keyboard. */
#define ErrCancel	-10		/* User cancelled. */


