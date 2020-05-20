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
** bitmap.c
**
** Bitmap object manipulation routines
** $Id: bitmap.c,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#include <string.h>
#include <nofrendo.h>
#include <bitmap.h>
#include <nes.h>

bitmap_t *bmp_create(short width, short height, short overdraw)
{
   MESSAGE_INFO("bmp_create: width=%d, height=%d, overdraw=%d\n", width, height, overdraw);

   bitmap_t *bitmap = malloc(sizeof(bitmap_t) + (sizeof(uint8 *) * height));
   if (bitmap == NULL)
      return NULL;

   uint8 *data = malloc((width + (overdraw * 2)) * height);
   if (data == NULL)
      return NULL;

   bitmap->height = height;
   bitmap->width = width;
   bitmap->data = data;
   bitmap->pitch = width + (overdraw * 2);

   for (int i = 0; i < height; i++)
      bitmap->line[i] = bitmap->data + (bitmap->pitch * i) + overdraw;

   return bitmap;
}

void bmp_clear(bitmap_t *bitmap, uint8 color)
{
   memset(bitmap->data, color, bitmap->pitch * bitmap->height);
}

/* Deallocate space for a bitmap structure */
void bmp_free(bitmap_t *bitmap)
{
   if (bitmap)
   {
      if (bitmap->data)
         free(bitmap->data);
      free(bitmap);
   }
}
