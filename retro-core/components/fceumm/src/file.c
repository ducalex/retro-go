/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <string/stdstring.h>
#include <file/file_path.h>
#include <streams/file_stream.h>

#include "fceu-types.h"
#include "file.h"
#include "fceu-endian.h"
#include "fceu-memory.h"
#include "driver.h"
#include "general.h"

static MEMWRAP *MakeMemWrap(RFILE *tz)
{
   MEMWRAP *tmp = NULL;

   if (!(tmp = (MEMWRAP*)FCEU_malloc(sizeof(MEMWRAP))))
      goto doret;
   tmp->location = 0;

   filestream_seek(tz, 0, RETRO_VFS_SEEK_POSITION_END);
   tmp->size = filestream_tell(tz);
   filestream_seek(tz, 0, RETRO_VFS_SEEK_POSITION_START);

   if (!(tmp->data_int = (uint8*)FCEU_malloc(tmp->size)))
   {
      free(tmp);
      tmp = NULL;
      goto doret;
   }

   filestream_read(tz, tmp->data_int, tmp->size);
   tmp->data = tmp->data_int;

doret:
   return tmp;
}

static MEMWRAP *MakeMemWrapBuffer(const uint8 *buffer, size_t bufsize)
{
   MEMWRAP *tmp = (MEMWRAP*)FCEU_malloc(sizeof(MEMWRAP));

   if (!tmp)
      return NULL;

   tmp->location = 0;
   tmp->size     = bufsize;
   tmp->data_int = NULL;
   tmp->data     = buffer;

   return tmp;
}

FCEUFILE * FCEU_fopen(const char *path, const uint8 *buffer, size_t bufsize)
{
   FCEUFILE *fceufp = (FCEUFILE*)malloc(sizeof(FCEUFILE));

   if (buffer)
      fceufp->fp = MakeMemWrapBuffer(buffer, bufsize);
   else
   {
      RFILE *t = NULL;

      if (!string_is_empty(path) && path_is_valid(path))
         t = filestream_open(path,
               RETRO_VFS_FILE_ACCESS_READ,
               RETRO_VFS_FILE_ACCESS_HINT_NONE);

      if (!t)
      {
         free(fceufp);
         return NULL;
      }

      fceufp->fp = MakeMemWrap(t);
      filestream_close(t);
   }
   return fceufp;
}

int FCEU_fclose(FCEUFILE *fp)
{
   if (!fp)
      return 0;

   if (fp->fp)
   {
      if (fp->fp->data_int)
         free(fp->fp->data_int);
      fp->fp->data_int = NULL;

      free(fp->fp);
   }
   fp->fp = NULL;

	free(fp);
	fp = NULL;

	return 1;
}

uint64 FCEU_fread(void *ptr, size_t element_size, size_t nmemb, FCEUFILE *fp)
{
   uint32_t total = nmemb * element_size;

   if (fp->fp->location >= fp->fp->size)
      return 0;

   if((fp->fp->location + total) > fp->fp->size)
   {
      int64_t ak = fp->fp->size - fp->fp->location;

      memcpy((uint8_t*)ptr, fp->fp->data + fp->fp->location, ak);

      fp->fp->location = fp->fp->size;

      return (ak / element_size);
   }

   memcpy((uint8_t*)ptr, fp->fp->data + fp->fp->location, total);

   fp->fp->location += total;

   return nmemb;
}

int FCEU_fseek(FCEUFILE *fp, long offset, int whence)
{
   switch (whence)
   {
      case SEEK_SET:
         if (offset >= fp->fp->size)
            return -1;

         fp->fp->location = offset;
         break;
      case SEEK_CUR:
         if ((offset + fp->fp->location) > fp->fp->size)
            return -1;

         fp->fp->location += offset;
         break;
   }

   return 0;
}

int FCEU_read32le(uint32 *Bufo, FCEUFILE *fp)
{
   if ((fp->fp->location + 4) > fp->fp->size)
      return 0;

   *Bufo = FCEU_de32lsb(fp->fp->data + fp->fp->location);

   fp->fp->location += 4;

   return 1;
}

int FCEU_fgetc(FCEUFILE *fp)
{
   if (fp->fp->location < fp->fp->size)
      return fp->fp->data[fp->fp->location++];

   return EOF;
}

uint64 FCEU_ftell(FCEUFILE *fp)
{
   return fp->fp->location;
}

uint64 FCEU_fgetsize(FCEUFILE *fp)
{
   return fp->fp->size;
}
