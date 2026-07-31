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

#include "rt_def.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "rt_error.h"
#include "rt_util.h"

// Globals
boolean DivisionError = false;

// Stubs for functions that were in rt_error.c or handled DOS interrupts
void DivisionErrorClear (void) { DivisionError = false; }

/*
===============
=
= Error
=
= For fatal errors
=
===============
*/

void Error (char *error, ...)
{
	char	string[256];
	va_list	argptr;

	va_start (argptr,error);
	vsprintf (string,error,argptr);
	va_end (argptr);

    printf("FATAL ERROR: %s\n", string);
    rg_system_log(RG_LOG_ERROR, "ROTT Error: %s", string);
    rg_system_exit();
}

/*
===============
=
= SoftwareError
=
= For non-fatal errors
=
===============
*/

void SoftwareError (char *error, ...)
{
	char	string[256];
	va_list	argptr;

	va_start (argptr,error);
	vsprintf (string,error,argptr);
	va_end (argptr);

    printf("Soft Error: %s\n", string);
    rg_system_log(RG_LOG_WARN, "ROTT SoftError: %s", string);
}

void UL_GeneralError (const char *s, int error)
{
    Error("General Error %d: %s", error, (char *)s);
}

void UL_RetryError (const char *s, int error)
{
    SoftwareError("Retry Error %d: %s", error, (char *)s);
}

void UL_UserMessage (int x, int y, const char *str, ...)
{
    // Stub
}

void UL_ErrorStartup ( void ) {}
void UL_ErrorShutdown ( void ) {}
void UL_StartupDivisionByZero ( void ) {}
void UL_ShutdownDivisionByZero ( void ) {}

void DivisionErrorCheck (const char *string, int val)
{
   if (DivisionError)
      {
      SoftwareError("Division Error in %s with value %d\n", (char *)string, val);
      DivisionError = false;
      }
}
