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
/* pcclone.h - machine specific parts of readflic.  Structures and
 * prototypes for polling the keyboard,  checking the time,
 * writing to the video screen, allocating large blocks of memory
 * and reading files.
 *
 * Copyright (c) 1992 Jim Kent.  This file may be freely used, modified,
 * copied and distributed.  This file was first published as part of
 * an article for Dr. Dobb's Journal March 1993 issue.
 */

#ifndef PCCLONE_H	/* Prevent file from being included twice. */
#define PCCLONE_H

typedef Uchar Pixel;			/* Pixel type. */

typedef struct
	{
	Uchar r,g,b;
	} Color;					/* One color map entry r,g,b 0-255. */

typedef struct
	{
	Pixel pixels[2];
	} Pixels2;					/* For word-oriented run length encoding */

typedef struct
	{
   Pixel  *pixels;   /* Set to AOOO:0000 for hardware. */
	int width, height;	/* Dimensions of screen. (320x200) */
	int old_mode;		/* Mode screen was in originally. */
	Boolean is_open;	/* Is screen open? */
	} Screen;					/* Device specific screen type. */


	/* Prototypes for routines that work on display screen. */

ErrCode screen_open(Screen *s);
	/* Put machine into graphics mode and fill out screen structure. */

void screen_close(Screen *s);
	/* Close screen.  Restore original display mode. */

int screen_width(Screen *s);
	/* Return width of screen. */

int screen_height(Screen *s);
	/* Return height of screen. */

void screen_put_dot(Screen *s, int x, int y, Pixel color);
	/* Set one dot. */

void screen_copy_seg(Screen *s, int x, int y, Pixel  *pixels, int count);
	/* Copy pixels from memory into screen. */

void screen_repeat_one(Screen *s, int x, int y, Pixel color, int count);
	/* Draw a horizontal line of a solid color */

void screen_repeat_two(Screen *s, int x,int y, Pixels2 pixels2, int count);
	/* Repeat 2 pixels count times on screen. */

void screen_put_colors(Screen *s, int start, Color  *colors, int count);
	/* Set count colors in color map starting at start.  RGB values
	 * go from 0 to 255. */

void screen_put_colors_64(Screen *s, int start, Color  *colors, int count);
	/* Set count colors in color map starting at start.  RGB values
	 * go from 0 to 64. */


/* Clock structure and routines. */

typedef struct
	{
	Ulong speed;	/* Number of clock ticks per second. */
	} Clock;

ErrCode clock_open(Clock *clock);
	/* Set up millisecond clock. */

void clock_close(Clock *clock);
	/* Return clock to normal. */

Ulong clock_ticks(Clock *clock);
	/* Get time in terms of clock->speed. */

/* Keyboard structure and routines. */

typedef struct
	{
	Uchar ascii;
	Ushort scancode;
	} Key;

ErrCode key_open(Key *key);
	/* Set up keyboard. */

void key_close(Key *key);
	/* Close keyboard. */

Boolean key_ready(Key *key);
	/* See if a key is ready. */

Uchar key_read(Key *key);
	/* Get next key. */


/** MemPtr - handles allocating big blocks of memory (>64K) on the
 ** PC.  On other machines may be much simpler. */

typedef Uchar * MemPtr;

ErrCode big_alloc(MemPtr *bb, Ulong size);
	/* Allocate a big block. */

void big_free(MemPtr *bb);
	/* Free up a big block. */

/** Stuff for reading files - regular and over 64k blocks at a time. **/

ErrCode file_open_to_read(FileHandle *phandle, char *name);
	/* Open a binary file to read. */

ErrCode file_read_block(FileHandle handle, void  *block, unsigned size);
	/* Read in a block.  If read less than size return error code. */

ErrCode file_read_big_block(FileHandle handle, MemPtr bb, Ulong size);
	/* Read in a big block.  Could be bigger than 64K. */



/** Machine structure - contains all the machine dependent stuff. **/

typedef struct
	{
	Screen screen;
	Clock clock;
	Key key;
	} Machine;

ErrCode machine_open(Machine *machine);
	/* Open up machine: keyboard, clock, screen. */

void machine_close(Machine *machine);
	/* Close down machine. */
#endif /* PCCLONE_H */
