/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** types.h
**
** Data type definitions
** $Id: noftypes.h,v 1.1 2001/04/27 14:37:11 neil Exp $
*/

#ifndef _TYPES_H_
#define _TYPES_H_

/* Define this if running on little-endian (x86) systems */
#define  HOST_LITTLE_ENDIAN

#define  INLINE      static inline __attribute__((__always_inline__))
#define  ZERO_LENGTH 0

/* quell stupid compiler warnings */
#define  UNUSED(x)   ((x) = (x))

typedef  signed char    int8;
typedef  signed short   int16;
typedef  signed int     int32;
typedef  unsigned char  uint8;
typedef  unsigned short uint16;
typedef  unsigned int   uint32;

#ifndef __cplusplus
#ifndef bool
typedef enum
{
   false = 0,
   true = 1
} bool;
#endif

#ifndef  NULL
#define  NULL     ((void *) 0)
#endif
#endif /* !__cplusplus */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#endif /* _TYPES_H_ */
