/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef _UTILS_C
#define _UTILS_C


#include "cleantypes.h"


void Log(const char *, ...);

//! Wait for the current 60th of sec to be elapsed
void wait_next_vsync();

//! CRC predefined array
extern const unsigned long TAB_CONST[256];

#if defined(WIN32)
#include <stdio.h>
#include <windows.h>

// Dummy declarations to fool our implementation of shared memory
typedef int key_t;
#define IPC_CREAT 0
#define IPC_EXCL  0
#define IPC_RMID  0

int shmget(key_t, int, int);
char* shmat(int, int, int);
int shmctl(int, int, int);

#if !defined(unix)
#define htons(arg) (((arg) >> 8) | (((arg) & 0xFF) << 8))
#endif

#endif

void patch_rom(char* filename, int offset, uchar value);

char *strupr(char *s);
#if !defined(FREEBSD) && !defined(__cplusplus)
char *strcasestr (const char *s1, const char *s2);
#endif

#if !defined(WIN32)
int stricmp (char *s1, char *s2);
#endif

void get_directory_from_filename(char*);
void wipe_directory(char*);
int file_exists(char*);
long file_size(char* file_name);


#endif
