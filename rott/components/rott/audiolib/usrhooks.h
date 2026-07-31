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
   module: USRHOOKS.H

   author: James R. Dose
   date:   July 26, 1994

   Public header file for USRHOOKS.C.

   This module contains cover functions for operations the library
   needs that may be restricted by the calling program.  The function
   prototypes in this header should not be modified.
**********************************************************************/

#ifndef __USRHOOKS_H
#define __USRHOOKS_H

/*---------------------------------------------------------------------
   Error definitions
---------------------------------------------------------------------*/

enum USRHOOKS_Errors
   {
   USRHOOKS_Warning = -2,
   USRHOOKS_Error   = -1,
   USRHOOKS_Ok      = 0
   };


/*---------------------------------------------------------------------
   Function Prototypes
---------------------------------------------------------------------*/

int USRHOOKS_GetMem( void **ptr, unsigned long size );
int USRHOOKS_FreeMem( void *ptr );

#endif
