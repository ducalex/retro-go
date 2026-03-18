#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) ||                                                    \
    (defined(TARGET_GNW) && defined(ENABLE_EMULATOR_NES) &&                    \
     FORCE_NOFRENDO == 0)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdlib.h>
#include <string.h>

#include "fceu-memory.h"
#include "fceu-types.h"
#include "fceu.h"
#include "general.h"

#ifdef TARGET_GNW
#include "gw_malloc.h"
#endif

void *FCEU_gmalloc(uint32 size) {
#ifndef TARGET_GNW
  void *ret = malloc(size);
  if (!ret) {
    FCEU_PrintError("Error allocating memory!  Doing a hard exit.");
    exit(1);
  }
  memset(ret, 0, size);
  return ret;
#else
  FCEU_printf("FCEU_gmalloc %ld\n", size);
  void *ret = itc_calloc(1, size);
  return ret;
#endif
}

void *FCEU_malloc(uint32 size) {
#ifndef TARGET_GNW
  void *ret = (void *)malloc(size);

  if (!ret) {
    FCEU_PrintError("Error allocating memory!");
    ret = 0;
  }
  memset(ret, 0, size);
  return ret;
#else
  FCEU_printf("FCEU_malloc %ld\n", size);
  void *ret = itc_calloc(1, size);
  return ret;
#endif
}

void FCEU_free(void *ptr) { free(ptr); }

void FCEU_gfree(void *ptr) { free(ptr); }
#endif