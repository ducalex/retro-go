#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
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
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#ifndef TARGET_GNW
#include <file/file_path.h>
#endif

#include "fceu-types.h"
#include "fceu.h"

#include "general.h"
#include "fceu-state.h"

#include "driver.h"

#include "md5.h"

#ifndef TARGET_GNW
static char BaseDirectory[2048] = {0};

void FCEUI_SetBaseDirectory(const char *dir)
{
	strncpy(BaseDirectory, dir, 2047);
	BaseDirectory[2047] = 0;
}

char *FCEU_MakeFName(int type, int id1, char *cd1)
{
   char tmp[4096 + 512] = {0}; /* +512 for no reason :D */
   char *ret      = 0;

   switch (type)
   {
      case FCEUMKF_GGROM:
         fill_pathname_join(tmp, BaseDirectory,
               "gamegenie.nes", sizeof(tmp));
         break;
      case FCEUMKF_FDSROM:
         fill_pathname_join(tmp, BaseDirectory,
               "disksys.rom", sizeof(tmp));
         break;
      case FCEUMKF_PALETTE:
         fill_pathname_join(tmp, BaseDirectory,
               "nes.pal", sizeof(tmp));
         break;
      default:
         break;
   }

   FCEU_printf(" FCEU_MakeFName: %s\n", tmp);

   ret = (char*)malloc(strlen(tmp) * sizeof(char) + 1);
   strcpy(ret, tmp);

   return(ret);
}
#endif

uint32 uppow2(uint32 n)
{
   int x;

   for (x = 31; x >= 0; x--)
      if (n & (1 << x))
      {
         if ((1 << x) != n)
            return(1 << (x + 1));
         break;
      }
   return n;
}
#endif