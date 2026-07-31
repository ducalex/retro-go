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
/* Types.h - integer types defined for portability between compilers and
 * notational convenience.
 *
 * Copyright (c) 1992 Jim Kent.  This file may be freely used, modified,
 * copied and distributed.  This file was first published as part of
 * an article for Dr. Dobb's Journal March 1993 issue.
 */

#ifndef TYPES_H		/* Prevent file from being included twice. */
#define TYPES_H

typedef signed char Char;		/* Signed 8 bits. */
typedef unsigned char Uchar;	/* Unsigned 8 bits. */
typedef short Short;			/* Signed 16 bits please. */
typedef unsigned short Ushort;	/* Unsigned 16 bits please. */
typedef int Long;				/* Signed 32 bits. */
typedef unsigned int Ulong;	/* Unsigned 32 bits. */

typedef int Boolean;			/* TRUE or FALSE value. */
typedef int ErrCode;			/* ErrXXX or Success. */
typedef int FileHandle;			/* OS file handle. */

	/* Values for Boolean types */
#define FALSE 0
#define TRUE (!FALSE)

	/* Values for ErrCodes */
#define Success		0		/* Things are fine. */
#define AError     -1    /* Unclassified error. */

#define int86 int386
#define inportb inp
#define outportb outp
#endif /* TYPES_H */
