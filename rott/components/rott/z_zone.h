/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
//***************************************************************************
//
//    Z_Zone Memory management Constants
//
//***************************************************************************


#ifndef _z_zone_public
#define _z_zone_public


extern int lowmemory;

// tags < 100 are not overwritten until freed
#define PU_STATIC               1                       // static entire execution time
#define PU_GAME                 20                      // static until game completed
#define PU_LEVELSTRUCT          49                      // start of static until level exited
#define PU_LEVEL                50                      // start of static until level exited
#define PU_LEVELEND             51                      // end of static until level exited

// tags >= 100 are purgable whenever needed
#define PU_PURGELEVEL           100
#define PU_CACHE                101
#define PU_CACHEWALLS           155
#define PU_CACHESPRITES         154
#define PU_CACHEBJWEAP          153
#define PU_CACHEACTORS          152
#define PU_CACHESOUNDS          120
#define PU_FLAT                 102
#define PU_PATCH                103
#define PU_TEXTURE              104

#define URGENTLEVELSTART PU_LEVEL

//***************************************************************************
//
//    Z_ZONE.C - Carmack's Memory manager for protected mode
//
//***************************************************************************

extern int zonememorystarted;

void Z_Init (int size, int min);                // Starts up Memory manager (size is in bytes), (min is minimum requirement)
void Z_Free (void *ptr);                        // Free a pointer in Z_Zone's domain
void *Z_Malloc (int size, int tag, void *user); // Malloc You can pass a NULL user if the tag is < PU_PURGELEVEL
void *Z_LevelMalloc (int size, int tag, void *user); // Level Malloc for level structures
void Z_FreeTags (int lowtag, int hightag);      // Free a series of memory tags
void Z_DumpHeap (int lowtag, int hightag);      // Dump the heap (for debugging purposes)
void Z_CheckHeap (void);                        // Check the heap for corruption
void Z_ChangeTag (void *ptr, int tag);          // Change the tag of a memory item
int Z_HeapSize ( void );                        // Return the total heap size
int Z_UsedHeap ( void );                        // Return used portion of heap size
int Z_AvailHeap ( void );                       // Returns largest available contiguous block
int Z_UsedStaticHeap ( void );                  // Returns amount of heap which is static ( < PURGELEVEL )
void Z_ShutDown( void );
int Z_GetSize (void *ptr);
int Z_UsedLevelHeap ( void );
void Z_Realloc (void ** ptr, int newsize);

#endif
