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
 *  Provides a logical console output routine that allows what is
 *  output to console normally and when output is redirected to
 *  be controlled..
 *
 *-----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "doomtype.h"
#include "doomdef.h"
#include "lprintf.h"
#include "i_main.h"
#include "m_argv.h"

int cons_output_mask = -1;        /* all output enabled */


void lprintf(OutputLevels lvl, const char *s, ...)
{
  if (lvl & cons_output_mask)
  {
    va_list arg;
    va_start(arg, s);
#ifdef RETRO_GO
    rg_system_vlog(RG_LOG_USER, NULL, s, arg);
#else
    vprintf(s, arg);
#endif
    // vprintf(s, arg);
    va_end(arg);
  }
}


/*
 * I_Error
 *
 * cphipps - moved out of i_* headers, to minimise source files that depend on
 * the low-level headers. All this does is print the error, then call the
 * low-level safe exit function.
 * killough 3/20/98: add const
 */

void I_Error(const char *error, ...)
{
  va_list arg;
  va_start(arg, error);
#ifdef RETRO_GO
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), error, arg);
  RG_PANIC(buffer);
#else
  vprintf(error, arg);
  abort();
#endif
  va_end(arg);
}
