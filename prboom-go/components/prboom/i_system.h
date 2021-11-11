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
 *      System specific interface stuff.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#ifdef __GNUG__
#pragma interface
#endif

extern int ms_to_next_tick;
bool I_StartDisplay(void);
void I_EndDisplay(void);
int I_GetTime_RealTime(void);     /* killough */
#ifndef PRBOOM_SERVER
int I_GetTimeFrac (void);
#endif
void I_GetTime_SaveMS(void);

unsigned long I_GetRandomTimeSeed(void); /* cphipps */

void I_uSleep(unsigned long usecs);

/* cphipps - I_GetVersionString
 * Returns a version string in the given buffer
 */
const char* I_GetVersionString(char* buf, size_t sz);

/* cphipps - I_SigString
 * Returns a string describing a signal number
 */
const char* I_SigString(char* buf, size_t sz, int signum);

const char *I_DoomExeDir(void); // killough 2/16/98: path to executable's dir

char* I_FindFile(const char* wfname, const char* ext);

#endif
