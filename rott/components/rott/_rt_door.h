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
#ifndef _rt_door_private
#define _rt_door_private

#define OPENTICS	165

typedef struct tp
 {
  signed char actionindex;
  signed char swapactionindex;
  int         whichobj;
  byte        tictime;
  byte        ticcount;
  byte        triggered;
  byte        complete;
  byte        done;
 } saved_touch_type;


#define NUMTOUCHPLATEACTIONS 8

#define FL_TACT 0x4000
#define FL_TSTAT 0x8000

#define PUSHWALLSPEED 10

#define AMW_NUMFRAMES 9
#define AMW_TICCOUNT  3

#endif
