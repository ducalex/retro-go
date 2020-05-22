/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** nes_state.h
**
** state saving header
** $Id: nes_state.h,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#ifndef _NESSTATE_H_
#define _NESSTATE_H_

#include <nes.h>

typedef struct
{
   uint8  type[4];
   uint32 blockVersion;
   uint32 blockLength;
} SnssBlockHeader;

extern void state_setslot(int slot);
extern int state_load(char* fn);
extern int state_save(char* fn);

#endif /* _NESSTATE_H_ */
