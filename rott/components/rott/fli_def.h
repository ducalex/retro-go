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
/* Flic.h - header file containing structure of a flic file.
 *
 * Copyright (c) 1992 Jim Kent.  This file may be freely used, modified,
 * copied and distributed.  This file was first published as part of
 * an article for Dr. Dobb's Journal March 1993 issue.
 */

#ifndef FLIC_H		/* Keep this from being included twice */
#define FLIC_H

#pragma pack (1)
	/* Flic Header */

#warning fli_def structures need to be packed

typedef struct
	{
	Long	size;		/* Size of flic including this header. */
	Ushort 	type;		/* Either FLI_TYPE or FLC_TYPE below. */
	Ushort	frames;		/* Number of frames in flic. */
	Ushort	width;		/* Flic width in pixels. */
	Ushort	height;		/* Flic height in pixels. */
	Ushort	depth;		/* Bits per pixel.  (Always 8 now.) */
	Ushort	flags;		/* FLI_FINISHED | FLI_LOOPED ideally. */
	Long 	speed;		/* Delay between frames. */
	Short	reserved1;	/* Set to zero. */
	Ulong	created;	/* Date of flic creation. (FLC only.) */
	Ulong	creator;	/* Serial # of flic creator. (FLC only.) */
	Ulong	updated;	/* Date of flic update. (FLC only.) */
	Ulong	updater;	/* Serial # of flic updater. (FLC only.) */
	Ushort	aspect_dx;	/* Width of square rectangle. (FLC only.) */
	Ushort	aspect_dy;	/* Height of square rectangle. (FLC only.) */
	Char 	reserved2[38];/* Set to zero. */
	Long 	oframe1;	/* Offset to frame 1. (FLC only.) */
	Long 	oframe2;	/* Offset to frame 2. (FLC only.) */
	Char 	reserved3[40];/* Set to zero. */
	} FlicHead;
	/* Values for FlicHead.type */
#define FLI_TYPE 0xAF11u	/* 320x200 .FLI type ID */
#define FLC_TYPE 0xAF12u	/* Variable rez .FLC type ID */
	/* Values for FlicHead.flags */
#define FLI_FINISHED 0x0001
#define FLI_LOOPED	 0x0002

	/* Optional Prefix Header */
typedef struct
	{
	Long size;		/* Size of prefix including header. */
	Ushort type;	/* Always PREFIX_TYPE. */
	Short chunks;	/* Number of subchunks in prefix. */
	Char reserved[8];/* Always 0. */
	} PrefixHead;
	/* Value for PrefixHead.type */
#define PREFIX_TYPE  0xF100u

	/* Frame Header */
typedef struct
	{
	Long size;		/* Size of frame including header. */
	Ushort type;	/* Always FRAME_TYPE */
	Short chunks;	/* Number of chunks in frame. */
	Char reserved[8];/* Always 0. */
	} FrameHead;
	/* Value for FrameHead.type */
#define FRAME_TYPE 0xF1FAu

	/* Chunk Header */
typedef struct
	{
	Long size;		/* Size of chunk including header. */
	Ushort type;	/* Value from ChunkTypes below. */
	} ChunkHead;
enum ChunkTypes
	{
	COLOR_256 = 4,	/* 256 level color pallette info. (FLC only.) */
	DELTA_FLC = 7,	/* Word-oriented delta compression. (FLC only.) */
	COLOR_64 = 11,	/* 64 level color pallette info. */
	DELTA_FLI = 12,	/* Byte-oriented delta compression. */
	BLACK = 13,		/* whole frame is color 0 */
	BYTE_RUN = 15,	/* Byte run-length compression. */
	LITERAL = 16,	/* Uncompressed pixels. */
	PSTAMP = 18,	/* "Postage stamp" chunk. (FLC only.) */
	};

#endif /* FLIC_H */
