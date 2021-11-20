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

// Tunables

// Alignment of zone memory (benefit may be negated by HEADER_SIZE, CHUNK_SIZE)
#define CACHE_ALIGN 32

// Minimum chunk size at which blocks are allocated
#define CHUNK_SIZE 4

// signature for block header
#define ZONEID  0x931d4a11

// Number of mallocs & frees kept in history buffer (must be a power of 2)
#define ZONE_HISTORY 4

// End Tunables

typedef struct memblock
{
#ifdef ZONEIDCHECK
  unsigned id;
#endif

  unsigned tag: 10;
  unsigned size:22;

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

// 0 means unlimited, any other value is a hard limit
//static int memory_size = 8192*1024;
static int memory_size = 0;
static int free_memory = 0;

#ifdef INSTRUMENTED

// statistics for evaluating performance
static int active_memory = 0;
static int purgable_memory = 0;

static void Z_DrawStats(void)            // Print allocation statistics
{
  if (gamestate != GS_LEVEL)
    return;

  if (memory_size > 0) {
    unsigned long total_memory = free_memory + memory_size + active_memory + purgable_memory;
    double s = 100.0 / total_memory;

    doom_printf("%-5i\t%6.01f%%\tstatic\n"
            "%-5i\t%6.01f%%\tpurgable\n"
            "%-5i\t%6.01f%%\tfree\n"
            "%-5li\t\ttotal\n",
            active_memory,
            active_memory*s,
            purgable_memory,
            purgable_memory*s,
            (free_memory + memory_size),
            (free_memory + memory_size)*s,
            total_memory
            );
  } else {
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

#ifdef INSTRUMENTED

// killough 4/26/98: Add history information

enum {malloc_history, free_history, NUM_HISTORY_TYPES};

static const char *file_history[NUM_HISTORY_TYPES][ZONE_HISTORY];
static int line_history[NUM_HISTORY_TYPES][ZONE_HISTORY];
static int history_index[NUM_HISTORY_TYPES];
static const char *const desc[NUM_HISTORY_TYPES] = {"malloc()'s", "free()'s"};

void Z_DumpHistory(char *buf)
{
  int i,j;
  char s[1024];
  strcat(buf,"\n");
  for (i=0;i<NUM_HISTORY_TYPES;i++)
    {
      sprintf(s,"\nLast several %s:\n\n", desc[i]);
      strcat(buf,s);
      for (j=0; j<ZONE_HISTORY; j++)
        {
          int k = (history_index[i]-j-1) & (ZONE_HISTORY-1);
          if (file_history[i][k])
            {
              sprintf(s, "File: %s, Line: %d\n", file_history[i][k],
                      line_history[i][k]);
              strcat(buf,s);
            }
        }
    }
}
#else

void Z_DumpHistory(char *buf)
{
}

#endif

void Z_Close(void)
{
#if 0
  (free)(zonebase);
  zone = rover = zonebase = NULL;
#endif
}

void Z_Init(void)
{
#if 0
  size_t size = zone_size*1000;

#ifdef HAVE_MMAP
  return; /* cphipps - if we have mmap, we don't need our own heap */
#endif

#ifdef INSTRUMENTED
  if (!(HEADER_SIZE >= sizeof(memblock_t) && size > HEADER_SIZE))
    I_Error("Z_Init: Sanity check failed");
#endif

  size = (size+CHUNK_SIZE-1) & ~(CHUNK_SIZE-1);  // round to chunk size
  size += HEADER_SIZE + CACHE_ALIGN;

  // Allocate the memory

  zonebase=(malloc)(size);
  if (!zonebase)
    I_Error("Z_Init: Failed on allocation of %lu bytes", (unsigned long)size);

  lprintf(LO_INFO,"Z_Init : Allocated %lukb zone memory\n",
      (long unsigned)size / 1000);

  // Align on cache boundary

  zone = (memblock_t *) ((char *) zonebase + CACHE_ALIGN -
                         ((unsigned) zonebase & (CACHE_ALIGN-1)));

  rover = zone;                            // Rover points to base of zone mem
  zone->next = zone->prev = zone;          // Single node
  zone->size = size;                       // All memory in one block
  zone->tag = PU_FREE;                     // A free block
  zone->vm  = 0;

#ifdef ZONEIDCHECK
  zone->id  = 0;
#endif

#ifdef INSTRUMENTED
  free_memory = size;
  /* cph - remove unnecessary initialisations to 0 */
#endif
#ifdef HEAPDUMP
  atexit(Z_DumpMemory);
#endif
#endif
}

/* Z_Malloc
 * You can pass a NULL user if the tag is < PU_PURGELEVEL.
 *
 * cph - the algorithm here was a very simple first-fit round-robin
 *  one - just keep looping around, freeing everything we can until
 *  we get a large enough space
 *
 * This has been changed now; we still do the round-robin first-fit,
 * but we only free the blocks we actually end up using; we don't
 * free all the stuff we just pass on the way.
 */

void *(Z_Malloc)(size_t size, int tag, void **user
#ifdef INSTRUMENTED
     , const char *file, int line
#endif
     )
{
  memblock_t *block = NULL;

#ifdef INSTRUMENTED
#ifdef CHECKHEAP
  Z_CheckHeap();
#endif

  file_history[malloc_history][history_index[malloc_history]] = file;
  line_history[malloc_history][history_index[malloc_history]++] = line;
  history_index[malloc_history] &= ZONE_HISTORY-1;
#endif

#ifdef ZONEIDCHECK
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

  if (memory_size > 0 && ((free_memory + memory_size) < (int)(size + HEADER_SIZE)))
  {
    memblock_t *end_block;
    block = blockbytag[PU_CACHE];
    if (block)
    {
      end_block = block->prev;
      while (1)
      {
        memblock_t *next = block->next;
#ifdef INSTRUMENTED
        (Z_Free)((char *) block + HEADER_SIZE, file, line);
#else
        (Z_Free)((char *) block + HEADER_SIZE);
#endif
        if (((free_memory + memory_size) >= (int)(size + HEADER_SIZE)) || (block == end_block))
          break;
        block = next;               // Advance to next block
      }
    }
    block = NULL;
  }

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
  free_memory -= block->size;

#ifdef INSTRUMENTED
  block->file = file;
  block->line = line;
#endif

#ifdef ZONEIDCHECK
  block->id = ZONEID;         // signature required in block header
#endif
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

void (Z_Free)(void *p
#ifdef INSTRUMENTED
              , const char *file, int line
#endif
             )
{
  memblock_t *block = (memblock_t *)((char *) p - HEADER_SIZE);

#ifdef INSTRUMENTED
#ifdef CHECKHEAP
  Z_CheckHeap();
#endif
  file_history[free_history][history_index[free_history]] = file;
  line_history[free_history][history_index[free_history]++] = line;
  history_index[free_history] &= ZONE_HISTORY-1;
#endif

  if (!p)
    return;


#ifdef ZONEIDCHECK
  if (block->id != ZONEID)
    I_Error("Z_Free: freed a pointer without ZONEID"
#ifdef INSTRUMENTED
            "\nSource: %s:%d"
            "\nSource of malloc: %s:%d"
            , file, line, block->file, block->line
#endif
           );
  block->id = 0;              // Nullify id so another free fails
#endif

  if (block->user)            // Nullify user if one exists
    *block->user = NULL;

  if (block == block->next)
    blockbytag[block->tag] = NULL;
  else
    if (blockbytag[block->tag] == block)
      blockbytag[block->tag] = block->next;
  block->prev->next = block->next;
  block->next->prev = block->prev;

  free_memory += block->size;
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

void (Z_FreeTags)(int lowtag, int hightag, int max
#ifdef INSTRUMENTED
                  , const char *file, int line
#endif
                 )
{
#ifdef HEAPDUMP
  Z_DumpMemory();
#endif

  if (lowtag <= PU_FREE)
    lowtag = PU_FREE+1;

  if (hightag > PU_CACHE)
    hightag = PU_CACHE;

  for (;lowtag <= hightag; lowtag++)
  {
    memblock_t *block, *end_block;
    block = blockbytag[lowtag];
    if (!block)
      continue;
    end_block = block->prev;
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

void (Z_ChangeTag)(void *ptr, int tag
#ifdef INSTRUMENTED
       , const char *file, int line
#endif
       )
{
  memblock_t *block = (memblock_t *)((char *) ptr - HEADER_SIZE);

  // proff - added sanity check, this can happen when an empty lump is locked
  if (!ptr)
    return;

  // proff - do nothing if tag doesn't differ
  if (tag == block->tag)
    return;

#ifdef INSTRUMENTED
#ifdef CHECKHEAP
  Z_CheckHeap();
#endif
#endif

#ifdef ZONEIDCHECK
  if (block->id != ZONEID)
    I_Error ("Z_ChangeTag: freed a pointer without ZONEID"
#ifdef INSTRUMENTED
             "\nSource: %s:%d"
             "\nSource of malloc: %s:%d"
             , file, line, block->file, block->line
#endif
            );

  if (tag >= PU_PURGELEVEL && !block->user)
    I_Error ("Z_ChangeTag: an owner is required for purgable blocks\n"
#ifdef INSTRUMENTED
             "Source: %s:%d"
             "\nSource of malloc: %s:%d"
             , file, line, block->file, block->line
#endif
            );

#endif // ZONEIDCHECK

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

void *(Z_Realloc)(void *ptr, size_t n, int tag, void **user
#ifdef INSTRUMENTED
                  , const char *file, int line
#endif
                 )
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

void *(Z_Calloc)(size_t n1, size_t n2, int tag, void **user
#ifdef INSTRUMENTED
                 , const char *file, int line
#endif
                )
{
  return
    (n1*=n2) ? memset((Z_Malloc)(n1, tag, user DA(file, line)), 0, n1) : NULL;
}

char *(Z_Strdup)(const char *s, int tag, void **user
#ifdef INSTRUMENTED
                 , const char *file, int line
#endif
                )
{
  return strcpy((Z_Malloc)(strlen(s)+1, tag, user DA(file, line)), s);
}

void (Z_CheckHeap)(
#ifdef INSTRUMENTED
       const char *file, int line
#else
       void
#endif
       )
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
