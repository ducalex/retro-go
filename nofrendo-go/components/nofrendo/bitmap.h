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
** bitmap.h
**
** Bitmap object defines / prototypes
** $Id: bitmap.h,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#ifndef _BITMAP_H_
#define _BITMAP_H_

typedef struct
{
    uint8_t r, g, b;
} rgb_t;

typedef struct
{
    size_t height;
    size_t width;
    size_t pitch;
    uint8_t *data;    /* protected */
    uint8_t *line[0]; /* will hold line pointers */
} bitmap_t;

extern bitmap_t *bmp_create(size_t width, size_t height, size_t overdraw);
extern void bmp_clear(bitmap_t *bitmap, uint8_t color);
extern void bmp_free(bitmap_t *bitmap);

#endif /* _BITMAP_H_ */
