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
 *      WAD I/O functions.
 *
 *-----------------------------------------------------------------------------*/


#ifndef __W_WAD__
#define __W_WAD__

#ifdef __GNUG__
#pragma interface
#endif

//
// WADFILE I/O related stuff.
//

typedef struct
{
  char identification[4];                  // Should be "IWAD" or "PWAD".
  int  numlumps;
  int  infotableofs;
} wadinfo_t;

typedef struct
{
  int  filepos;
  int  size;
  char name[8];
} filelump_t;

typedef enum
{
  source_iwad=0,    // iwad file load
  source_pre,       // predefined lump
  source_auto_load, // lump auto-loaded by config file
  source_pwad,      // pwad file load
  source_lmp,       // lmp file load
  source_net        // CPhipps
} wad_source_t;

typedef enum {
  ns_global=0,
  ns_sprites,
  ns_flats,
  ns_colormaps,
  ns_prboom
} lump_ns_t;

typedef struct
{
  const char* name;
  const void *data;
  size_t size;
  void *handle;
} wadfile_info_t;

typedef struct
{
  char   name[8];         // lump name, uppercased
  short  li_namespace;    // lump namespace
  short  index, next;     // Index in lumpinfo[]
  size_t size;            // lump size
  size_t position;        // position in wadfile
  wadfile_info_t *wadfile;// source file
} lumpinfo_t;

#define MAX_WAD_FILES 8

extern wadfile_info_t wadfiles[MAX_WAD_FILES];
extern size_t numwadfiles;
extern lumpinfo_t *lumpinfo;
extern size_t      numlumps;

void W_Init(void); // CPhipps - uses the above array
void W_InitCache(void);
void W_DoneCache(void);

// killough 4/17/98: if W_CheckNumForName() called with only
// one argument, pass ns_global as the default namespace

#define W_CheckNumForName(name) W_CheckNumForNameNs(name, ns_global)
int     W_CheckNumForNameNs(const char* name, int);   // killough 4/17/98
int     W_GetNumForName(const char* name);
int     W_LumpLength(int lump);
int     W_Read(void *dest, size_t size, size_t offset, wadfile_info_t *wad);
void    W_ReadLump(void *dest, int lump);
const void* W_CacheLumpNum(int lump);
const void* W_LockLumpNum(int lump);
void    W_UnlockLumpNum(int lump);

// CPhipps - convenience macros
#define W_CacheLumpName(name) W_CacheLumpNum (W_GetNumForName(name))
#define W_UnlockLumpName(name) W_UnlockLumpNum (W_GetNumForName(name))

char *AddDefaultExtension(char *, const char *);  // killough 1/18/98
void ExtractFileBase(const char *, char *);       // killough
unsigned W_LumpNameHash(const char *s);           // killough 1/31/98
void W_HashLumps(void);                           // cph 2001/07/07 - made public

#endif
