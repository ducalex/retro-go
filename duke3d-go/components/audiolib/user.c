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
/**********************************************************************
   module: USER.C

   author: James R. Dose
   date:   April 26, 1994

   Routines to parse command line options.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <string.h>
#include "user.h"

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

#ifdef PLAT_DOS
extern int   _argc;
extern char **_argv;
#endif

/*---------------------------------------------------------------------
   Function: USER_CheckParameter

   Checks if the specified string is present in the command line.
---------------------------------------------------------------------*/

int USER_CheckParameter
   (
   const char *parameter
   )

   {
#ifdef PLAT_DOS
   int i;
   int found;
   char *ptr;

   found = FALSE;
   i = 1;
   while( i < _argc )
      {
      ptr = _argv[ i ];

      // Only check parameters preceded by - or /
      if ( ( *ptr == '-' ) || ( *ptr == '/' ) )
         {
         ptr++;
         if ( stricmp( parameter, ptr ) == 0 )
            {
            found = TRUE;
            break;
            }
         }

      i++;
      }

   return( found );
#else
   return FALSE;
#endif
   }


/*---------------------------------------------------------------------
   Function: USER_GetText

   Checks if the specified string is present in the command line
   and returns a pointer to the text following it.
---------------------------------------------------------------------*/

char *USER_GetText
   (
   const char *parameter
   )

   {
   return NULL;
   }
