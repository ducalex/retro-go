/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Zone Memory Allocation. Neat.
 *
 * Neat enough to be rewritten by Lee Killough...
 *
 * Must not have been real neat :)
 *
 * Made faster and more general, and added wrappers for all of Doom's
 * memory allocation functions, including malloc() and similar functions.
 * Added line and file numbers, in case of error. Added performance
 * statistics and tunables.
 *-----------------------------------------------------------------------------
 */


// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "doomstat.h"
#include "lprintf.h"
#include "z_zone.h"

#define CHUNK_SIZE 4        // Minimum chunk size at which blocks are allocated
#define ZONEID  0x931d4a11  // signature for block header

typedef struct memblock
{
  uint32_t zoneid;
  uint32_t tag: 10;
  uint32_t size:22;

  struct memblock *next,*prev;
  void **user;

#ifdef INSTRUMENTED
  const char *file;
  int line;
#endif

} memblock_t;

/* size of block header
 * cph - base on sizeof(memblock_t), which can be larger than CHUNK_SIZE on
 * 64bit architectures */
static const size_t HEADER_SIZE = (sizeof(memblock_t)+CHUNK_SIZE-1) & ~(CHUNK_SIZE-1);

static memblock_t *blockbytag[PU_MAX];

#ifdef INSTRUMENTED

// statistics for evaluating performance
static int active_memory = 0;
static int purgable_memory = 0;

static void Z_DrawStats(void)            // Print allocation statistics
{
  if (gamestate != GS_LEVEL)
    return;

    unsigned long total_memory = active_memory + purgable_memory;
    double s = 100.0 / total_memory;

    doom_printf("%-5i\t%6.01f%%\tstatic\n"
            "%-5i\t%6.01f%%\tpurgable\n"
            "%-5li\t\ttotal\n",
            active_memory,
            active_memory*s,
            purgable_memory,
            purgable_memory*s,
            total_memory
            );
}

#ifdef HEAPDUMP

#ifndef HEAPDUMP_DIR
#define HEAPDUMP_DIR "."
#endif

void W_PrintLump(FILE* fp, void* p);

void Z_DumpMemory(void)
{
  static int dump;
  char buf[PATH_MAX + 1];
  FILE* fp;
  size_t total_cache = 0, total_free = 0, total_malloc = 0;
  int tag;

  sprintf(buf, "%s/memdump.%d", HEAPDUMP_DIR, dump++);
  fp = fopen(buf, "w");
  for (tag = PU_FREE; tag < PU_MAX; tag++)
  {
    memblock_t* end_block, *block;
    block = blockbytag[tag];
    if (!block)
      continue;
    end_block = block->prev;
    while (1)
    {
      switch (block->tag) {
      case PU_FREE:
        fprintf(fp, "free %d\n", block->size);
        total_free += block->size;
        break;
      case PU_CACHE:
        fprintf(fp, "cache %s:%d:%d\n", block->file, block->line, block->size);
        total_cache += block->size;
        break;
      case PU_LEVEL:
        fprintf(fp, "level %s:%d:%d\n", block->file, block->line, block->size);
        total_malloc += block->size;
        break;
      default:
        fprintf(fp, "malloc %s:%d:%d", block->file, block->line, block->size);
        total_malloc += block->size;
        if (block->file)
          if (strstr(block->file,"w_memcache.c"))
            W_PrintLump(fp, (char*)block + HEADER_SIZE);
        fputc('\n', fp);
        break;
      }
      if (block == end_block)
        break;
      block=block->next;
    }
  }
  fprintf(fp, "malloc %d, cache %d, free %d, total %d\n",
    total_malloc, total_cache, total_free,
    total_malloc + total_cache + total_free);
  fclose(fp);
}
#endif
#endif

void Z_Close(void)
{
#ifdef INSTRUMENTED
  Z_DumpMemory();
#endif
  // Release everything
  Z_FreeTags(PU_FREE, PU_MAX);
}

void Z_Init(void)
{
  // Nothing to do
}

void *(Z_Malloc)(size_t size, int tag, void **user DA(const char *file, int line))
{
  memblock_t *block = NULL;

#ifdef INSTRUMENTED
#ifdef CHECKHEAP
  Z_CheckHeap();
#endif

  if (tag >= PU_PURGELEVEL && !user)
    I_Error ("Z_Malloc: An owner is required for purgable blocks"
#ifdef INSTRUMENTED
             "Source: %s:%d", file, line
#endif
       );
#endif

  if (!size)
    return user ? *user = NULL : NULL;           // malloc(0) returns NULL

  size = (size+CHUNK_SIZE-1) & ~(CHUNK_SIZE-1);  // round to chunk size

  while (!(block = (malloc)(size + HEADER_SIZE))) {
    if (!blockbytag[PU_CACHE])
      I_Error ("Z_Malloc: Failure trying to allocate %lu bytes"
#ifdef INSTRUMENTED
               "\nSource: %s:%d"
#endif
               ,(unsigned long) size
#ifdef INSTRUMENTED
               , file, line
#endif
      );
    // RG: Don't nuke the whole cache at once!
    (Z_FreeTags)(PU_CACHE, PU_CACHE, 2);
  }

  if (!blockbytag[tag])
  {
    blockbytag[tag] = block;
    block->next = block->prev = block;
  }
  else
  {
    blockbytag[tag]->prev->next = block;
    block->prev = blockbytag[tag]->prev;
    block->next = blockbytag[tag];
    blockbytag[tag]->prev = block;
  }

  block->size = size;

#ifdef INSTRUMENTED
  if (tag >= PU_PURGELEVEL)
    purgable_memory += block->size;
  else
    active_memory += block->size;
#endif

#ifdef INSTRUMENTED
  block->file = file;
  block->line = line;
#endif

  block->zoneid = ZONEID;     // signature required in block header
  block->tag = tag;           // tag
  block->user = user;         // user
  block = (memblock_t *)((char *) block + HEADER_SIZE);
  if (user)                   // if there is a user
    *user = block;            // set user to point to new block

#ifdef INSTRUMENTED
  Z_DrawStats();           // print memory allocation stats
  // scramble memory -- weed out any bugs
  memset(block, gametic & 0xff, size);
#endif

  return block;
}

void (Z_Free)(void *p DA(const char *file, int line))
{
  memblock_t *block = (memblock_t *)((char *) p - HEADER_SIZE);

#ifdef INSTRUMENTED
#ifdef CHECKHEAP
  Z_CheckHeap();
#endif
#endif

  if (!p)
    return;

  if (block->zoneid != ZONEID)
    I_Error("Z_Free: freed a pointer without ZONEID"
#ifdef INSTRUMENTED
            "\nSource: %s:%d"
            "\nSource of malloc: %s:%d"
            , file, line, block->file, block->line
#endif
           );
  block->zoneid = 0;          // Nullify id so another free fails

  if (block->user)            // Nullify user if one exists
    *block->user = NULL;

  if (block == block->next)
    blockbytag[block->tag] = NULL;
  else
    if (blockbytag[block->tag] == block)
      blockbytag[block->tag] = block->next;
  block->prev->next = block->next;
  block->next->prev = block->prev;

#ifdef INSTRUMENTED
  if (block->tag >= PU_PURGELEVEL)
    purgable_memory -= block->size;
  else
    active_memory -= block->size;

  /* scramble memory -- weed out any bugs */
  memset(block, gametic & 0xff, block->size + HEADER_SIZE);
#endif

  (free)(block);

#ifdef INSTRUMENTED
      Z_DrawStats();           // print memory allocation stats
#endif
}

void (Z_FreeTags)(int lowtag, int hightag, int max DA(const char *file, int line))
{
#ifdef HEAPDUMP
  Z_DumpMemory();
#endif

  lowtag = MAX(lowtag, PU_FREE+1);
  hightag = MIN(hightag, PU_MAX-1);

  for (;lowtag <= hightag; hightag--)
  {
    if (!blockbytag[hightag])
      continue;
    memblock_t *block = blockbytag[hightag];
    memblock_t *end_block = block->prev;
    while (max--)
    {
      memblock_t *next = block->next;
#ifdef INSTRUMENTED
      (Z_Free)((char *) block + HEADER_SIZE, file, line);
#else
      (Z_Free)((char *) block + HEADER_SIZE);
#endif
      if (block == end_block)
        break;
      block = next;               // Advance to next block
    }
  }
}

void (Z_ChangeTag)(void *ptr, int tag DA(const char *file, int line))
{
  memblock_t *block = (memblock_t *)((char *) ptr - HEADER_SIZE);

  if (!ptr || tag == block->tag)
    return;

#ifdef INSTRUMENTED
#ifdef CHECKHEAP
  Z_CheckHeap();
#endif

  if (tag >= PU_PURGELEVEL && !block->user)
    I_Error ("Z_ChangeTag: an owner is required for purgable blocks\n"
#ifdef INSTRUMENTED
             "Source: %s:%d"
             "\nSource of malloc: %s:%d"
             , file, line, block->file, block->line
#endif
            );
#endif

  if (block->zoneid != ZONEID)
    I_Error ("Z_ChangeTag: freed a pointer without ZONEID"
#ifdef INSTRUMENTED
             "\nSource: %s:%d"
             "\nSource of malloc: %s:%d"
             , file, line, block->file, block->line
#endif
            );

  if (block == block->next)
    blockbytag[block->tag] = NULL;
  else
    if (blockbytag[block->tag] == block)
      blockbytag[block->tag] = block->next;
  block->prev->next = block->next;
  block->next->prev = block->prev;

  if (!blockbytag[tag])
  {
    blockbytag[tag] = block;
    block->next = block->prev = block;
  }
  else
  {
    blockbytag[tag]->prev->next = block;
    block->prev = blockbytag[tag]->prev;
    block->next = blockbytag[tag];
    blockbytag[tag]->prev = block;
  }

#ifdef INSTRUMENTED
  if (block->tag < PU_PURGELEVEL && tag >= PU_PURGELEVEL)
  {
    active_memory -= block->size;
    purgable_memory += block->size;
  }
  else
    if (block->tag >= PU_PURGELEVEL && tag < PU_PURGELEVEL)
    {
      active_memory += block->size;
      purgable_memory -= block->size;
    }
#endif

  block->tag = tag;
}

void *(Z_Realloc)(void *ptr, size_t n, int tag, void **user DA(const char *file, int line))
{
  void *p = (Z_Malloc)(n, tag, user DA(file, line));
  if (ptr)
    {
      memblock_t *block = (memblock_t *)((char *) ptr - HEADER_SIZE);
      memcpy(p, ptr, n <= block->size ? n : block->size);
      (Z_Free)(ptr DA(file, line));
      if (user) // in case Z_Free nullified same user
        *user=p;
    }
  return p;
}

void *(Z_Calloc)(size_t n1, size_t n2, int tag, void **user DA(const char *file, int line))
{
  return (n1*=n2) ? memset((Z_Malloc)(n1, tag, user DA(file, line)), 0, n1) : NULL;
}

char *(Z_Strdup)(const char *s, int tag, void **user DA(const char *file, int line))
{
  size_t len = strlen(s) + 1;
  return memcpy((Z_Malloc)(len, tag, user DA(file, line)), s, len);
}

void (Z_CheckHeap)(DAC(const char *file, int line))
{
#if 0
  memblock_t *block;   // Start at base of zone mem
  if (block)
  do {                        // Consistency check (last node treated special)
    if ((block->next != zone &&
         (memblock_t *)((char *) block+HEADER_SIZE+block->size) != block->next)
        || block->next->prev != block || block->prev->next != block)
      I_Error("Z_CheckHeap: Block size does not touch the next block\n"
#ifdef INSTRUMENTED
              "Source: %s:%d"
              "\nSource of offending block: %s:%d"
              , file, line, block->file, block->line
#endif
              );
//#ifdef INSTRUMENTED
// shouldn't be needed anymore, was just for testing
#if 0
    if (((int)block->file < 0x00001000) && (block->file != NULL) && (block->tag != 0)) {
      block->file = NULL;
    }
#endif
  } while ((block=block->next) != zone);
#endif
}
