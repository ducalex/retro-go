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
   module: INTERRUP.H

   author: James R. Dose
   date:   March 31, 1994

   Inline functions for disabling and restoring the interrupt flag.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __INTERRUPT_H
#define __INTERRUPT_H

unsigned long DisableInterrupts( void );
void          RestoreInterrupts( unsigned long flags );

#ifdef PLAT_DOS
#pragma aux DisableInterrupts = \
   "pushfd",                    \
   "pop    eax",                \
   "cli"                        \
   modify [ eax ];

#pragma aux RestoreInterrupts = \
   "push   eax",                \
   "popfd"                      \
   parm [ eax ];
#endif

#endif
