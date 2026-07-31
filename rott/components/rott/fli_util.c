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
/* pcclone.c - This file contains all the machine specific bits of the
 * flic reader.  It's job is to set up data structures and routines for
 * the Screen, Clock, and Key structures,  and the Machine structure
 * that contains them all.
 *
 * For optimum performance a flic-reader should be coded in assembler.
 * However you can get significantly greater performance merely by
 * recoding in assembler the three routines: screen_copy_seg(),
 * screen_repeat_one() and screen_repeat_two().
 *
 * Copyright (c) 1992 Jim Kent.  This file may be freely used, modified,
 * copied and distributed.  This file was first published as part of
 * an article for Dr. Dobb's Journal March 1993 issue.
 */

#include "rt_def.h"
#include "cin_glob.h"
#include <bios.h>
#include <dos.h>
#include <time.h>
#include <stdlib.h>
#include <mem.h>
#include <limits.h>
#include <fcntl.h>
#include <io.h>
#include <conio.h>
#include "fli_type.h"
#include "fli_util.h"
#include "fli_def.h"
#include "fli_main.h"
//MED
#include "memcheck.h"

static Ushort screenlookup[200];

/** Screen oriented stuff. **/

static Boolean set_vmode(Uchar mode)
	/* Ask bios to set video mode. */
{
union REGS regs;

regs.h.ah = 0;		/* Set Video Mode request. */
regs.h.al = mode;	/* For our specific mode. */
int86(0x10, &regs, &regs);
return TRUE;
// return (regs.x.cflag == 0);	/* Carry flag clear? */
}

static Uchar get_vmode()
	/* Ask bios for current video mode. */
{
union REGS regs;

regs.h.ah = 0xF;			/* Get Video Mode request. */
int86(0x10, &regs, &regs);
return regs.h.al;
}

ErrCode screen_open(Screen *s)
	/* Put machine into graphics mode and fill out screen structure. */
{
ClearStruct(s);			/* Start in a known state... */
s->old_mode = get_vmode();
if (set_vmode(0x13))
	{
	if (get_vmode() == 0x13)
		{
      int i;

		s->is_open = TRUE;	/* Now it's open. */
		s->width = 320;
		s->height = 200;
      for (i=0;i<200;i++)
         screenlookup[i]=320*i;
      s->pixels = (unsigned char *)0xA0000;  /* Base video screen address. */
		return Success;
		}
	}
/* If got to here have failed.  Restore old video mode and return
 * failure code. */
set_vmode(s->old_mode);
return ErrDisplay;
}

void screen_close(Screen *s)
	/* Close screen.  Restore original display mode. */
{
if (s->is_open)		/* Don't do this twice... */
	{
//   set_vmode(s->old_mode);
	ClearStruct(s);		/* Discourage use after it's closed... */
	}
}

int screen_width(Screen *s)
	/* Return width of screen. */
{
return s->width;
}

int screen_height(Screen *s)
	/* Return height of screen. */
{
return s->height;
}

void screen_put_dot(Screen *s, int x, int y, Pixel color)
	/* Set one dot. */
{
	/* First clip it. */
if (x < 0 || y < 0 || x >= s->width || y >= s->height)
	return;

	/* Then set it. */
s->pixels[screenlookup[y] + x] = color;
}

static Boolean line_clip(Screen *s, int *px, int *py, int *pwidth)
	/* Clip a horizontal line segment so that it fits on the screen.
	 * Return FALSE if clipped out entirely. */
{
int x = *px;
int y = *py;
int width = *pwidth;
int xend = x + width;

if (y < 0 || y >= s->height || xend < 0 || x >= s->width)
	return FALSE;	/* Clipped off screen. */
if (x < 0)
	{
	*pwidth = width = width + x;		/* and shortens width. */
	*px = 0;
	}
if (xend > s->width)
	{
	*pwidth = width = width - (xend - s->width);
	}
if (width < 0)
	return FALSE;
return TRUE;
}

void screen_copy_seg(Screen *s, int x, int y, Pixel  *pixels, int count)
	/* Copy pixels from memory into screen. */
{
Pixel  *pt;
int unclipped_x = x;

	/* First let's do some clipping. */
if (!line_clip(s, &x, &y, &count))
	return;

pixels += (x - unclipped_x);   /* Clipping change in start position. */

	/* Calculate start screen address. */
pt = s->pixels + (unsigned)screenlookup[y] + (unsigned)x;

	/* Copy pixels to display. */
memcpy (pt,pixels,count);
}

void screen_repeat_one(Screen *s, int x, int y, Pixel color, int count)
	/* Draw a horizontal line of a solid color */
{
Pixel  *pt;

	/* First let's do some clipping. */
if (!line_clip(s, &x, &y, &count))
	return;

	/* Calculate start screen address. */
pt = s->pixels + (unsigned)screenlookup[y] + (unsigned)x;

	/* Repeat pixel on display. */
   memset (pt,color,count);
}

void screen_repeat_two(Screen *s, int x, int y, Pixels2 pixels2, int count)
	/* Repeat 2 pixels count times on screen. */
{
Pixels2  *pt;
int is_odd;

	/* First let's do some clipping. */
count <<= 1;		/* Convert from word to pixel count. */
if (!line_clip(s, &x, &y, &count))
	return;
is_odd = (count&1);		/* Did it turn odd after clipping?  Ack! */
count >>= 1;			/* Convert back to word count. */

	/* Calculate start screen address. */
pt = (Pixels2  *)(s->pixels + (unsigned)screenlookup[y] + (unsigned)x);

while (--count >= 0)	/* Go set screen 2 pixels at a time. */
	*pt++ = pixels2;

if (is_odd)				/* Deal with pixel at end of screen if needed. */
	{
   Pixel  *end = (Pixel  *)pt;
	*end = pixels2.pixels[0];
	}
}


void screen_put_colors(Screen *s, int start, Color  *colors, int count)
	/* Set count colors in color map starting at start.  RGB values
	 * go from 0 to 255. */
{
int end = start + count;
int ix;

for (ix = start; ix < end; ++ix)
	{
	outportb(0x3C8, ix);
	outportb(0x3C9, colors->r>>2);
	outportb(0x3C9, colors->g>>2);
	outportb(0x3C9, colors->b>>2);
	++colors;
	}
}

void screen_put_colors_64(Screen *s, int start, Color  *colors, int count)
	/* Set count colors in color map starting at start.  RGB values
	 * go from 0 to 64. */
{
int end = start + count;
int ix;

for (ix = start; ix < end; ++ix)
	{
	outportb(0x3C8, ix);
	outportb(0x3C9, colors->r);
	outportb(0x3C9, colors->g);
	outportb(0x3C9, colors->b);
	++colors;
	}
}

#if 0
/** Clock oriented stuff. **/

#define CMODE	0x43
#define CDATA	0x40

ErrCode clock_open(Clock *c)
	/* Set up clock and store speed of clock.  */
{
c->speed = 4608;        /* Our peculiar speed.  */
outportb(CMODE, 0x34);  /* Change from divide by two to linear. */
outportb(CDATA, 0);     /* Set period to highest available. */
outportb(CDATA, 0);
return Success;
}

void clock_close(Clock *c)
	/* Return clock to normal. */
{
outportb(CMODE, 0x36);  /* Change from linear to divide by two. */
outportb(CDATA, 0);     /* Set period to highest available. */
outportb(CDATA, 0);
}

Ulong clock_ticks(Clock *c)
	/* Get current clock tick. */
{
/* This routine returns a clock with occassional spikes where time will
 * look like its running backwards 1/18th of a second.  The resolution
 * of the clock is 1/(18*256) = 1/4608 second.  The spikes are ok for
 * our purposes since the wait loop will just ignore them. */
union REGS regs;
Uchar chip_time;
Ulong time;

regs.h.ah = 0;				/* Go ask BIOS timer services */
int86(0x1A, &regs, &regs);	/* for time in 1/18ths second. */
outportb(CMODE,0);			/* Latch time at timer chip. */
inportb(CDATA);				/* Read in LSB of chip time and discard. */
chip_time = inportb(CDATA);	/* Read in MSB of chip time and save. */
chip_time = -(signed char)chip_time;
	/* We calculate the time using 3 bytes from the BIOS 18hz counter
	 * and one byte from the timer chip itself.  We discard the hi
	 * byte of the BIOS time,  shift the rest left by 8, and
	 * fill in the low byte with the MSB from the chip timer.
	 * This looks a little more complicated than this because
	 * the bios time is in various registers - cx for the hi word
	 * and dx for the low word. */
time = (Ulong)regs.h.cl << 24L;		/* Get MSB of our final time. */
time += (Ulong)regs.w.dx << 8L;     /* Fold in middle two bytes. */
time += (Ulong)chip_time;			/* Add in LSB from chip. */
return time;
}


/** Keyboard oriented stuff. **/

ErrCode key_open(Key *key)
	/* Set up keyboard. */
{
return Success;		/* Pretty easy on a PC. */
}

void key_close(Key *key)
	/* Close keyboard. */
{
return;				/* Also very easy under DOS. */
}

Boolean key_ready(Key *key)
	/* See if a key is ready. */
{
unsigned val;

if ((val = _bios_keybrd(_KEYBRD_READY)) == 0)
	return FALSE;
else
	{
	key->ascii = val;
	key->scancode = val;
	return TRUE;
	}
}

Uchar key_read(Key *key)
	/* Get next key. */
{
unsigned val;

val = _bios_keybrd(_KEYBRD_READ);
key->ascii = val;
key->scancode = val;
return key->ascii;
}

#endif

/** MemPtr stuff - to allocate and free blocks of memory > 64K. */

ErrCode big_alloc(MemPtr *bb, Ulong size)
	/* Allocate a big block. */
{
   (*bb) = SafeMalloc (size);
   return Success;
}

void big_free(MemPtr *bb)
	/* Free up a big block. */
{
   SafeFree(*bb);
}

/** Stuff for reading files - regular and over 64k blocks at a time. **/

ErrCode file_open_to_read(FileHandle *phandle, char *name)
	/* Open a binary file to read. */
{
   *phandle = SafeOpenRead(name);
	return Success;
}

ErrCode file_read_big_block(FileHandle handle, MemPtr bb, Ulong size)
	/* Read in a big block.  Could be bigger than 64K. */
{
   SafeRead (handle,bb,size);
   return Success;
}


/** Machine oriented stuff - open and close the whole banana. **/

ErrCode machine_open(Machine *machine)
	/* Open up machine: keyboard, clock, and screen. */
{
ErrCode err;

ClearStruct(machine);	/* Start it in a known state. */
//if ((err = key_open(&machine->key)) >= Success)
   {
//   if ((err = clock_open(&machine->clock)) >= Success)
		{
		if ((err = screen_open(&machine->screen)) >= Success)
			return Success;
//      clock_close(&machine->clock);
		}
//   key_close(&machine->key);
	}
return err;
}

void machine_close(Machine *machine)
	/* Close down machine. */
{
screen_close(&machine->screen);
//clock_close(&machine->clock);
//key_close(&machine->key);
}
