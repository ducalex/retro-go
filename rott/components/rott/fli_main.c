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
/* readflic.c - This module contains the routines to read and decompress
 * a flic.  They assume Intel byte ordering,  but otherwise should be
 * fairly portable.  They call machine specific stuff in pcclone.c.
 * This file starts with the low level decompression routines - first
 * for colors and then for pixels.  Then it goes to the higher level
 * exported flic_xxxx routines as prototyped in readflic.h.
 *
 * Copyright (c) 1992 Jim Kent.  This file may be freely used, modified,
 * copied and distributed.  This file was first published as part of
 * an article for Dr. Dobb's Journal March 1993 issue.
 */

#include "rt_def.h"
#include "cin_glob.h"
#include <errno.h>
#include <string.h>
#include <io.h>
#include "fli_type.h"
#include "fli_util.h"
#include "fli_def.h"
#include "fli_main.h"
//MED
#include "memcheck.h"



typedef void ColorOut(Screen *s, int start, Color  *colors, int count);
	/* This is the type of output parameter to our decode_color below.
	 * Not coincedently screen_put_color is of this type. */

static void decode_color(Uchar  *data
, Flic *flic, Screen *s, ColorOut *output)
	/* Decode color map.  Put results into output.  The two color
	 * compressions are identical except for whether the RGB values
	 * are 0-63 or 0-255.  Passing in an output that does the appropriate
	 * shifting on the way to the real pallete lets us use the same
	 * code for both COLOR_64 and COLOR_256 compression.
	 */
{
int start = 0;
Uchar  *cbuf = (Uchar  *)data;
Short  *wp = (Short  *)cbuf;
Short ops;
int count;

ops = *wp;
cbuf += sizeof(*wp);
while (--ops >= 0)
	{
	start += *cbuf++;
	if ((count = *cbuf++) == 0)
		count = 256;
   (*output)(s, start, (Color  *)cbuf, count);
	cbuf += 3*count;
	start += count;
	}
}

static void decode_color_256(Uchar  *data, Flic *flic, Screen *s)
	/* Decode COLOR_256 chunk. */
{
decode_color(data, flic, s, screen_put_colors);
}

static void decode_color_64(Uchar  *data, Flic *flic, Screen *s)
	/* Decode COLOR_64 chunk. */
{
decode_color(data, flic, s, screen_put_colors_64);
}


static void decode_byte_run(Uchar  *data, Flic *flic, Screen *s)
	/* Byte-run-length decompression. */
{
int x,y;
int width = flic->head.width;
int height = flic->head.height;
Char psize;
Char  *cpt = data;
int end;

y = flic->yoff;
end = flic->xoff + width;
while (--height >= 0)
	{
	x = flic->xoff;
	cpt += 1;	/* skip over obsolete opcount byte */
	psize = 0;
	while ((x+=psize) < end)
		{
		psize = *cpt++;
		if (psize >= 0)
			{
			screen_repeat_one(s, x, y, *cpt++, psize);
			}
		else
			{
			psize = -psize;
         screen_copy_seg(s, x, y, (Pixel  *)cpt, psize);
			cpt += psize;
			}
		}
	y++;
	}
}

static void decode_delta_fli(Uchar  *data, Flic *flic, Screen *s)
	/* Fli style delta decompression. */
{
int xorg = flic->xoff;
int yorg = flic->yoff;
Short  *wpt = (Short  *)data;
Uchar  *cpt = (Uchar  *)(wpt + 2);
int x,y;
Short lines;
Uchar opcount;
Char psize;

y = yorg + *wpt++;
lines = *wpt;
while (--lines >= 0)
	{
	x = xorg;
	opcount = *cpt++;
	while (opcount > 0)
		{
		x += *cpt++;
		psize = *cpt++;
		if (psize < 0)
			{
			psize = -psize;
			screen_repeat_one(s, x, y, *cpt++, psize);
			x += psize;
			opcount-=1;
			}
		else
			{
         screen_copy_seg(s, x, y, (Pixel  *)cpt, psize);
			cpt += psize;
			x += psize;
			opcount -= 1;
			}
		}
	y++;
	}
}


static void decode_delta_flc(Uchar  *data, Flic *flic, Screen *s)
	/* Flc-style delta decompression.  The data is word oriented though
    * a lot of the control info (how  to skip, how many words to
	 * copy) are byte oriented still to save space. */
{
int xorg = flic->xoff;
int yorg = flic->yoff;
int width = flic->head.width;
int x,y;
Short lp_count;
Short opcount;
int psize;
union {Short  *w; Uchar  *ub; Char  *b; Pixels2  *p2;} wpt;
int lastx;


	lastx = xorg + width - 1;
	wpt.ub = data;
	lp_count = *wpt.w++;
	y = yorg;
	goto LPACK;

SKIPLINES:	/* Advance over some lines. */
	y -= opcount;

LPACK:		/* do next line */
	if ((opcount = *wpt.w++) >= 0)
		goto DO_SS2OPS;
	if( ((Ushort)opcount) & 0x4000) /* skip lines */
		goto SKIPLINES;
	screen_put_dot(s,(Uchar)opcount,lastx,y); /* put dot at eol with low byte */
	if((opcount = *wpt.w++) == 0)
	{
		++y;
		if (--lp_count > 0)
			goto LPACK;
		goto OUT;
	}
DO_SS2OPS:
	x = xorg;

PPACK:				/* do next packet */
	x += *wpt.ub++;
	psize = *wpt.b++;
	if ((psize += psize) >= 0)
	{
      screen_copy_seg(s, x, y, (Pixel  *)wpt.ub, psize);
		x += psize;
		wpt.ub += psize;
		if (--opcount != 0)
			goto PPACK;
		++y;
		if (--lp_count > 0)
			goto LPACK;
	}
	else
	{
		psize = -psize;
		screen_repeat_two(s, x, y, *wpt.p2++, psize>>1);
		x += psize;
		if (--opcount != 0)
			goto PPACK;
		++y;
		if (--lp_count > 0)
			goto LPACK;
	}
OUT:
	return;
}

static void decode_black(Uchar  *data, Flic *flic, Screen *s)
	/* Decode a BLACK chunk.  Set frame to solid color 0 one
	 * line at a time. */
{
Pixels2 black;
int i;
int height = flic->head.height;
int width = flic->head.width;
int x = flic->xoff;
int y = flic->yoff;

black.pixels[0] = black.pixels[1] = 0;
for (i=0; i<height; ++i)
	{
	screen_repeat_two(s, x, y+i, black, width/2);
	if (width & 1)	/* if odd set last pixel */
		screen_put_dot(s, x+width-1, y+i, 0);
	}
}

static void decode_literal(Uchar  *data, Flic *flic, Screen *s)
	/* Decode a LITERAL chunk.  Just copy data to screen one line at
	 * a time. */
{
int i;
int height = flic->head.height;
int width = flic->head.width;
int x = flic->xoff;
int y = flic->yoff;

for (i=0; i<height; ++i)
	{
   screen_copy_seg(s, x, y+i, (Pixel  *)data, width);
	data += width;
	}
}

ErrCode SetupFlicAccess (Flic * flic)
{
if (flic->usefile==TRUE)
   {
   return file_open_to_read(&flic->handle, flic->name);
   }
else
   {
   flic->flicoffset=0;
   return Success;
   }
}

ErrCode CopyNextFlicBlock (Flic * flic, MemPtr buf, Ulong size)
{
ErrCode err;

if (flic->usefile==TRUE)
   {
   err = file_read_big_block(flic->handle, buf, size);
   return err;
   }
else
   {
   memcpy(buf, flic->flicbuffer+flic->flicoffset, size);
   flic->flicoffset+=size;
   return Success;
   }
}

void    SetFlicOffset (Flic * flic, Ulong offset )
{
if (flic->usefile==TRUE)
   {
   lseek(flic->handle,offset,SEEK_SET);
   }
else
   {
   flic->flicoffset = offset;
   }
}


ErrCode flic_open(Flic *flic, char *name, MemPtr buf, Boolean usefile)
	/* Open flic file.  Read header and verify it's a flic.
	 * Seek to first frame. */
{
ErrCode err;

ClearStruct(flic);		/* Start at a known state. */
flic->usefile=usefile;  /* use file or buffer */
flic->name = name;      /* Save name for future use. */
flic->flicbuffer=buf;   /* save address of flicbuffer */

if ((err = SetupFlicAccess (flic)) >= Success)
   {
   if ((err = CopyNextFlicBlock (flic, (Uchar *)&flic->head, sizeof(flic->head)))
      >= Success)
      {
      if (flic->head.type == FLC_TYPE)
         {
         /* Seek frame 1. */
         SetFlicOffset (flic, flic->head.oframe1 );
         return Success;
         }
      if (flic->head.type == FLI_TYPE)
         {
         /* Do some conversion work here. */
         flic->head.oframe1 = sizeof(flic->head);
         flic->head.speed = flic->head.speed * 1000L / 70L;
         return Success;
         }
      else
         {
         err = ErrBadFlic;
         }
      }
   }
flic_close(flic);    /* Close down and scrub partially opened flic. */
return err;
}


void flic_close(Flic *flic)
	/* Close flic file and scrub flic. */
{
if (flic->usefile==TRUE)
   {
   close(flic->handle);
   }
ClearStruct(flic);		/* Discourage use after close. */
}

static ErrCode decode_frame(Flic *flic
, FrameHead *frame, Uchar  *data, Screen *s)
	/* Decode a frame that is in memory already into screen.
	 * Here we just loop through each chunk calling appropriate
	 * chunk decoder.
	 */
{
int i;
ChunkHead  *chunk;

for (i=0; i<frame->chunks; ++i)
	{
   chunk = (ChunkHead  *)data;
	data += chunk->size;
	switch (chunk->type)
		{
		case COLOR_256:
         decode_color_256((Uchar  *)(chunk+1), flic, s);
			break;
		case DELTA_FLC:
         decode_delta_flc((Uchar  *)(chunk+1), flic, s);
			break;
		case COLOR_64:
         decode_color_64((Uchar  *)(chunk+1), flic, s);
			break;
		case DELTA_FLI:
         decode_delta_fli((Uchar  *)(chunk+1), flic, s);
			break;
		case BLACK:
         decode_black((Uchar  *)(chunk+1), flic, s);
			break;
		case BYTE_RUN:
         decode_byte_run((Uchar  *)(chunk+1), flic, s);
			break;
		case LITERAL:
         decode_literal((Uchar  *)(chunk+1), flic, s);
			break;
		default:
			break;
		}
	}
return Success;
}

ErrCode flic_next_frame(Flic *flic, Screen *screen)
	/* Advance to next frame of flic. */
{
FrameHead head;
ErrCode err;
MemPtr bb;
long size;

if ((err = CopyNextFlicBlock (flic, (Uchar *)&head, sizeof(head))) >= Success)
	{
	if (head.type == FRAME_TYPE)
		{
		size = head.size - sizeof(head);	/* Don't include head. */
		if (size > 0)
			{
         if ((err = big_alloc(&bb, size)) >= Success)
				{
            if ((err = CopyNextFlicBlock (flic, bb, size)) >= Success)
					{
               err = decode_frame(flic, &head, bb, screen);
					}
            big_free(&bb);
				}
			}
		}
	else
		{
		err = ErrBadFrame;
		}
	}
return err;
}


static Ulong calc_end_time(Ulong millis)
	/* Little helper subroutine to find out when to start on next
	 * frame. */
{
return (GetCinematicTime() + ( (millis * CLOCKSPEED) / 4000l) );
}

static ErrCode wait_til(Ulong end_time, Machine *machine)
	/* This waits until key is hit or end_time arrives.
	 * Return Success if timed out,  ErrCancel if key hit.
	 * Insures keyboard will be polled at least once.
	 */
{
   do
     {
     if (CinematicAbort()!=0)
        return ErrCancel;
     }
   while (GetCinematicTime() < end_time);
	return Success;
}

ErrCode flic_play_once(Flic *flic, Machine *machine)
	/* Play a flic through once. */
{
ErrCode err;
int i;
Ulong end_time;

for (i=0; i<flic->head.frames; ++i)
	{
   end_time = calc_end_time(flic->head.speed);
	if ((err = flic_next_frame(flic, &machine->screen)) < Success)
		break;
	if ((err = wait_til(end_time, machine)) < Success)
		break;
	}
return err;
}

static ErrCode fill_in_frame2(Flic *flic)
	/* This figures out where the second frame of the flic is
	 * (useful for playing in a loop).  */
{
FrameHead head;
ErrCode err;

SetFlicOffset (flic, flic->head.oframe1 );
if ((err = CopyNextFlicBlock (flic, (MemPtr)&head, sizeof(head))) < Success)
	return err;
flic->head.oframe2 = flic->head.oframe1 + head.size;
return Success;
}

ErrCode flic_play_loop(Flic *flic, Machine *machine)
	/* Play a flic until key is pressed. */
{
int i;
Ulong end_time;
ErrCode err;

if (flic->head.oframe2 == 0)
	{
	fill_in_frame2(flic);
	}
	/* Seek to first frame. */
SetFlicOffset (flic, flic->head.oframe1 );
	/* Save time to move on. */
end_time = calc_end_time(flic->head.speed);
	/* Display first frame. */
if ((err = flic_next_frame(flic, &machine->screen)) < Success)
	return err;
for (;;)
	{
		/* Seek to second frame */
   SetFlicOffset (flic, flic->head.oframe2 );
		/* Loop from 2nd frame thru ring frame*/
	for (i=0; i<flic->head.frames; ++i)
		{
		if (wait_til(end_time, machine) < Success)
			return Success;		/* Time out is a success here. */
		if ((err = flic_next_frame(flic, &machine->screen)) < Success)
			return err;
      end_time = calc_end_time(flic->head.speed);
		}
	}
}


static char *err_strings[] =
	{
	"Unspecified error",
	"Not enough memory",
	"Not a flic file",
	"Bad frame in flic",
	NULL,
	NULL,
	"Couldn't open display",
	"Couldn't open keyboard",
	"User canceled action",
	};

char *flic_err_string(ErrCode err)
	/* Return a string that describes an error. */
{
	if (err >= Success)
		return "Success";		/* Shouldn't happen really... */
	if (err == ErrOpen || err == ErrRead)
		return strerror(errno);	/* Get Disk IO error from DOS. */
	err = -err;
	err -= 1;
	if (err > ArrayEls(err_strings))
		return "Unknown error";
	return err_strings[err];
}


static void center_flic(Flic *flic, Screen *s)
	/* Set flic.xoff and flic.yoff so flic plays centered rather
	 * than in upper left corner of display. */
{
	flic->xoff = (screen_width(s) - (signed)flic->head.width)/2;
	flic->yoff = (screen_height(s) - (signed)flic->head.height)/2;
}

void PlayFlic ( char * name, unsigned char * buffer, int usefile, int loop)
{
ErrCode err;
Flic flic;
Machine machine;

if ((err = machine_open(&machine)) >= Success)
	{
   if ((err = flic_open(&flic, name, buffer, usefile)) >= Success)
      {
      center_flic(&flic, &machine.screen);
      if (loop==0)
         {
         err = flic_play_once(&flic, &machine);
         }
      else
         {
         err = flic_play_loop(&flic, &machine);
         }
      flic_close(&flic);
      }
	machine_close(&machine);
	}
if (err < Success && err != ErrCancel)
   {
   Error("Play Flic had troubles with %s.\n%s.\n",name, flic_err_string(err));
	}
}

