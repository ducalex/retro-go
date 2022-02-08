/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2001 by
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
 *      Handles WAD file header, directory, lump I/O.
 *
 *-----------------------------------------------------------------------------
 */

// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.h"
#include "d_net.h"
#include "doomtype.h"
#include "i_system.h"
#include "w_wad.h"
#include "lprintf.h"

//
// GLOBALS
//

// Opened WAD files
wadfile_info_t wadfiles[MAX_WAD_FILES];
size_t numwadfiles = 0;

// Location of each lump on disk.
lumpinfo_t *lumpinfo;
size_t      numlumps;

void ExtractFileBase (const char *path, char *dest)
{
  const char *src = path + strlen(path) - 1;
  int length;

  // back up until a \ or the start
  while (src != path && src[-1] != ':' // killough 3/22/98: allow c:filename
         && *(src-1) != '\\'
         && *(src-1) != '/')
  {
    src--;
  }

  // copy up to eight characters
  memset(dest,0,8);
  length = 0;

  while ((*src) && (*src != '.') && (++length<9))
  {
    *dest++ = toupper(*src);
    src++;
  }
  /* cph - length check removed, just truncate at 8 chars.
   * If there are 8 or more chars, we'll copy 8, and no zero termination
   */
}

//
// 1/18/98 killough: adds a default extension to a path
// Note: Backslashes are treated specially, for MS-DOS.
//

char *AddDefaultExtension(char *path, const char *ext)
{
  char *p = path;
  while (*p++);
  while (p-->path && *p!='/' && *p!='\\')
    if (*p=='.')
      return path;
  if (*ext!='.')
    strcat(path,".");
  return strcat(path,ext);
}

//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// Reload hack removed by Lee Killough
// CPhipps - source is an enum
//
// proff - changed using pointer to wadfile_info_t
static void W_AddFile(wadfile_info_t *wadfile)
{
  size_t startlump = numlumps;
  filelump_t *lumpindex, *fileinfo;
  wadinfo_t header;

  // If we do not have the whole thing in memory then we open it from disk
  if (!wadfile->data)
  {
    wadfile->handle = fopen(wadfile->name, "rb");
#ifdef HAVE_NET
    if (!wadfile->handle && D_NetGetWad(wadfile->name)) // CPhipps
      wadfile->handle = fopen(wadfile->name, "rb");
#endif
    if (wadfile->handle)
    {
      fseek(wadfile->handle, 0, SEEK_END);
      wadfile->size = ftell(wadfile->handle);
    }
  }

  if (!wadfile->handle && !wadfile->data)
    I_Error("W_AddFile: couldn't open %s", wadfile->name);

  W_Read(&header, sizeof(header), 0, wadfile);

  if (strncmp(header.identification, "IWAD", 4) && strncmp(header.identification, "PWAD", 4))
  {
    // Assume it's a single lump file
    lumpindex = fileinfo = malloc(sizeof(filelump_t));
    fileinfo->size = wadfile->size;
    fileinfo->filepos = 0;
    ExtractFileBase(wadfile->name, fileinfo->name);
    numlumps++;
  }
  else
  {
    // It's a proper WAD file
    size_t wadlumps = LONG(header.numlumps);
    size_t length = wadlumps * sizeof(filelump_t);
    lumpindex = malloc(length);
    W_Read(lumpindex, length, LONG(header.infotableofs), wadfile);
    numlumps += wadlumps;
  }

  // Append the new lumps to lumpinfo
  lumpinfo = realloc(lumpinfo, numlumps * sizeof(lumpinfo_t));
  fileinfo = lumpindex;

  for (size_t i = startlump; i < numlumps; i++, fileinfo++)
  {
    lumpinfo_t *lump_p = &lumpinfo[i];
    lump_p->wadfile = wadfile;                    //  killough 4/25/98
    lump_p->position = LONG(fileinfo->filepos);
    lump_p->size = LONG(fileinfo->size);
    lump_p->li_namespace = ns_global;              // killough 4/17/98
    lump_p->locks = 0;
    lump_p->ptr = NULL;
    memcpy(lump_p->name, fileinfo->name, 8);
  }

  free(lumpindex);

  lprintf(LO_INFO, " added %s:%s (%d lumps)\n",
    wadfile->data ? "data" : "file",
    wadfile->name,
    numlumps - startlump);
}

// jff 1/23/98 Create routines to reorder the master directory
// putting all flats into one marked block, and all sprites into another.
// This will allow loading of sprites and flats from a PWAD with no
// other changes to code, particularly fast hashes of the lumps.
//
// killough 1/24/98 modified routines to be a little faster and smaller

static int IsMarker(const char *marker, const char *name)
{
  return !strncasecmp(name, marker, 8) ||
    (*name == *marker && !strncasecmp(name+1, marker, 7));
}

// killough 4/17/98: add namespace tags

static void W_CoalesceMarkedResource(const char *start_marker,
                                     const char *end_marker, int li_namespace)
{
  lumpinfo_t *marked = malloc(sizeof(*marked) * numlumps);
  size_t i, num_marked = 0, num_unmarked = 0;
  int is_marked = 0, mark_end = 0;
  lumpinfo_t *lump = lumpinfo;
  char start_marker_buf[10], end_marker_buf[10];

  // just to make sure the memcpy below doesn't read out of bounds
  start_marker = strncpy(start_marker_buf, start_marker, 8);
  end_marker = strncpy(end_marker_buf, end_marker, 8);

  for (i=numlumps; i--; lump++)
    if (IsMarker(start_marker, lump->name))       // start marker found
      { // If this is the first start marker, add start marker to marked lumps
        if (!num_marked)
          {
            memcpy(marked->name, start_marker, 8);
            marked->size = 0;  // killough 3/20/98: force size to be 0
            marked->li_namespace = ns_global;        // killough 4/17/98
            marked->wadfile = NULL;
            num_marked = 1;
          }
        is_marked = 1;                            // start marking lumps
      }
    else
      if (IsMarker(end_marker, lump->name))       // end marker found
        {
          mark_end = 1;                           // add end marker below
          is_marked = 0;                          // stop marking lumps
        }
      else
        if (is_marked)                            // if we are marking lumps,
          {                                       // move lump to marked list
            marked[num_marked] = *lump;
            marked[num_marked++].li_namespace = li_namespace;  // killough 4/17/98
          }
        else
          lumpinfo[num_unmarked++] = *lump;       // else move down THIS list

  // Append marked list to end of unmarked list
  memcpy(lumpinfo + num_unmarked, marked, num_marked * sizeof(*marked));

  free(marked);                                   // free marked list

  numlumps = num_unmarked + num_marked;           // new total number of lumps

  if (mark_end)                                   // add end marker
    {
      lumpinfo[numlumps].size = 0;  // killough 3/20/98: force size to be 0
      lumpinfo[numlumps].wadfile = NULL;
      lumpinfo[numlumps].li_namespace = ns_global;   // killough 4/17/98
      memcpy(lumpinfo[numlumps++].name, end_marker, 8);
    }
}

// Hash function used for lump names.
// Must be mod'ed with table size.
// Can be used for any 8-character names.
// by Lee Killough

unsigned W_LumpNameHash(const char *s)
{
  unsigned hash;
  (void) ((hash =        toupper(s[0]), s[1]) &&
          (hash = hash*3+toupper(s[1]), s[2]) &&
          (hash = hash*2+toupper(s[2]), s[3]) &&
          (hash = hash*2+toupper(s[3]), s[4]) &&
          (hash = hash*2+toupper(s[4]), s[5]) &&
          (hash = hash*2+toupper(s[5]), s[6]) &&
          (hash = hash*2+toupper(s[6]),
           hash = hash*2+toupper(s[7]))
         );
  return hash;
}

//
// W_CheckNumForName
// Returns -1 if name not found.
//
// Rewritten by Lee Killough to use hash table for performance. Significantly
// cuts down on time -- increases Doom performance over 300%. This is the
// single most important optimization of the original Doom sources, because
// lump name lookup is used so often, and the original Doom used a sequential
// search. For large wads with > 1000 lumps this meant an average of over
// 500 were probed during every search. Now the average is under 2 probes per
// search. There is no significant benefit to packing the names into longwords
// with this new hashing algorithm, because the work to do the packing is
// just as much work as simply doing the string comparisons with the new
// algorithm, which minimizes the expected number of comparisons to under 2.
//
// killough 4/17/98: add namespace parameter to prevent collisions
// between different resources such as flats, sprites, colormaps
//

int W_CheckNumForNameNs(register const char *name, register int li_namespace)
{
  // Hash function maps the name to one of possibly numlump chains.
  // It has been tuned so that the average chain length never exceeds 2.

  // proff 2001/09/07 - check numlumps==0, this happens when called before WAD loaded
  register int i = (numlumps==0)?(-1):(lumpinfo[W_LumpNameHash(name) % (unsigned) numlumps].index);

  // We search along the chain until end, looking for case-insensitive
  // matches which also match a namespace tag. Separate hash tables are
  // not used for each namespace, because the performance benefit is not
  // worth the overhead, considering namespace collisions are rare in
  // Doom wads.

  while (i >= 0 && (strncasecmp(lumpinfo[i].name, name, 8) ||
                    lumpinfo[i].li_namespace != li_namespace))
    i = lumpinfo[i].next;

  // Return the matching lump, or -1 if none found.

  return i;
}

//
// killough 1/31/98: Initialize lump hash table
//

void W_HashLumps(void)
{
  for (size_t i = 0; i < numlumps; i++)
    lumpinfo[i].index = -1;                     // mark slots empty

  // Insert nodes to the beginning of each chain, in first-to-last
  // lump order, so that the last lump of a given name appears first
  // in any chain, observing pwad ordering rules. killough

  for (size_t i = 0; i < numlumps; i++)
    {                                           // hash function:
      size_t j = W_LumpNameHash(lumpinfo[i].name) % numlumps;
      lumpinfo[i].next = lumpinfo[j].index;     // Prepend to list
      lumpinfo[j].index = i;
    }
}

// End of lump hashing -- killough 1/31/98


// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName (const char* name)     // killough -- const added
{
  int i = W_CheckNumForName(name);
  if (i == -1)
    I_Error("W_GetNumForName: %.8s not found", name);
  return i;
}


// W_Init
// Loads each of the files in the wadfiles array.
// All files are optional, but at least one file
//  must be found.
// Files with a .wad extension are idlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
// Lump names can appear multiple times.
// The name searcher looks backwards, so a later file
//  does override all earlier ones.
//

void W_Init(void)
{
  lumpinfo = NULL;
  numlumps = 0;

  for (size_t i = 0; i < numwadfiles; i++)
    W_AddFile(&wadfiles[i]);

  if (!numlumps)
    I_Error ("W_Init: No files found");

  W_CoalesceMarkedResource("S_START", "S_END", ns_sprites);
  W_CoalesceMarkedResource("F_START", "F_END", ns_flats);
  W_CoalesceMarkedResource("C_START", "C_END", ns_colormaps);

  W_HashLumps();
}

//
// W_Read
// Read arbitrary data from the WAD file
//
int W_Read(void *dest, size_t size, size_t offset, wadfile_info_t *wad)
{
  if (offset >= wad->size)
  {
    lprintf(LO_WARN, "W_Read: offset %d > %d\n", offset, wad->size);
  }
  else if (wad->data)
  {
    memcpy(dest, wad->data + offset, size);
    return size;
  }
  else if (wad->handle)
  {
    fseek(wad->handle, offset, SEEK_SET);
    fread(dest, size, 1, wad->handle);
    return size;
  }
  return -1;
}

//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength(int lump)
{
  if ((unsigned)lump >= numlumps)
    I_Error("W_LumpLength: index out of bounds");

  return lumpinfo[lump].size;
}

//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void W_ReadLump(void *dest, int lump)
{
  if ((unsigned)lump >= numlumps)
    I_Error("W_ReadLump: index out of bounds");

  lumpinfo_t *l = lumpinfo + lump;
  if (l->wadfile)
  {
    W_Read(dest, l->size, l->position, l->wadfile);
  }
}

//
// W_CacheLumpNum
//
const void *W_CacheLumpNum(int lump)
{
  if ((unsigned)lump >= numlumps)
    I_Error("W_CacheLumpNum: index out of bounds");

  lumpinfo_t *l = &lumpinfo[lump];

  if (!l->ptr)
  {
    // Bypass caching if we have the WAD mapped in memory
    if (l->wadfile && l->wadfile->data)
      return l->wadfile->data + l->position;
    W_ReadLump(Z_Malloc(W_LumpLength(lump), PU_STATIC, &l->ptr), lump);
    l->locks = 0;
  }

  if (++l->locks == 1)
  {
    Z_ChangeTag(l->ptr, PU_STATIC);
    l->locks = 1;
  }

  return l->ptr;
}

//
// W_UnlockLumpNum
//
void W_UnlockLumpNum(int lump)
{
  if ((unsigned)lump >= numlumps)
    I_Error("W_UnlockLumpNum: index out of bounds");

  lumpinfo_t *l = &lumpinfo[lump];
  if (l->ptr && l->locks && --l->locks == 0)
    Z_ChangeTag(l->ptr, PU_CACHE);
}
