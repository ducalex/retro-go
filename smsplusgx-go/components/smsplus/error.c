/******************************************************************************
 *  Sega Master System / GameGear Emulator
 *  Copyright (C) 1998-2007  Charles MacDonald
 *
 *  additionnal code by Eke-Eke (SMS Plus GX)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *   Error logging routines
 *
 ******************************************************************************/

#include "shared.h"

#ifdef DEBUG
static FILE *error_log;
#endif

struct {
    int enabled;
    int verbose;
    FILE *log;
} t_error;

void error_init(void)
{
#ifdef DEBUG
    error_log = fopen("error.log","w");
#endif
}

void error_shutdown(void)
{
#ifdef DEBUG
    if(error_log) fclose(error_log);
#endif
}

void error(char *format, ...)
{
#ifdef DEBUG
    va_list ap;
    va_start(ap, format);
    if(error_log) vfprintf(error_log, format, ap);
    va_end(ap);
#endif
}

