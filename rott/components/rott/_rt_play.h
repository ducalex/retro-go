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
#ifndef _rt_playr_private
#define _rt_playr_private

#include "watcom.h"
//double dTopYZANGLELIMIT; in winrott.c
#define TopYZANGLELIMIT  (44*FINEANGLES/360)//bna added
#define YZANGLELIMIT  (80*FINEANGLES/360)//bna--(30*FINEANGLES/360)
/*
//bna++ had to limit this or the sky would fuck up 
#define TopYZANGLELIMIT  (6*FINEANGLES/360)//bna added
#define YZANGLELIMIT  (40*FINEANGLES/360)//bna--(30*FINEANGLES/360)
//partly fixed you can now se up but not down very much see rt_playr.c TopYZANGLELIMIT
*/


#define YZTILTSPEED   20*4//*2 = bna added
#define SNAPBACKSPEED 10*15//*2 = bna added
#define YZHORIZONSPEED 4*5//*2 = bna added
#define HORIZONYZOFFSET (FINEANGLES/4)
#define SetPlayerHorizon(ps,hlevel)    \
    {                                  \
    if ((hlevel)>YZANGLELIMIT)           \
       (ps)->horizon=HORIZONYZOFFSET+YZANGLELIMIT;\
    else if ((hlevel)<-YZANGLELIMIT)     \
       (ps)->horizon=HORIZONYZOFFSET-YZANGLELIMIT;\
    else                                  \
       (ps)->horizon=HORIZONYZOFFSET+(hlevel);\
	 }

#define StartWeaponChange \
{\
 SD_PlaySoundRTP(SD_SELECTWPNSND,ob->x,ob->y);\
 pstate->weapondowntics = WEAPONS[pstate->weapon].screenheight/GMOVE;\
 if ((ob==player) && SHOW_BOTTOM_STATUS_BAR() )\
	 DrawBarAmmo (false);            \
}


#define BULLETHOLEOFFSET 0x700

#define M_DIST(x1,x2,y1,y2)   (((x1-x2)*(x1-x2))+((y1-y2)*(y1-y2)))

#define W_CHANGE(x)   ((x->weapondowntics) || (x->weaponuptics))
#define NOMOM         ((!ob->momentumx) && (!ob->momentumy))
#define DISTOK(p1,p2,d)    (abs((p1)-(p2)) <= d)
#define Fix(a)        (a &= (FINEANGLES-1))
#define REMOTEPKTSIZE    (sizeof(MoveType))
#define BJRUNSPEED   2048
#define BJJUMPSPEED  680
#define PROJECTILESIZE 0xc000l

#define MOVESCALE       150l
#define BACKMOVESCALE   100l
#define EXTRAPOINTS     40000
#define JETPACKTHRUST (64)

#define RUNMOVE      0xa00
#define BASEMOVE     0x600
#define ANGLEMOVE    0x100000

#define SGN(x)                 (x>0 ? 1 : -1)
#define NETMOM    (BASEMOVE+0x10000)

#define DISTANCE(x1,x2,y1,y2)   (FixedMulShift((x1-x2),(x1-x2),16)+FixedMulShift((y1-y2),(y1-y2),16))

#define TILTLIMIT     95
#define PLAYERSTEPTIME 0xf000

#define HITWALLSPEED  0x2800
#define GODVIEWANGLE  FINEANGLES/150
#define GODOFFSET     10
#define DOGOFFSET     46
#define KESTICS       70
#define BBTIME        105

#define KEYBOARDNORMALTURNAMOUNT (0x160000)
//#define KEYBOARDPREAMBLETURNAMOUNT (0xe0000)
#define KEYBOARDPREAMBLETURNAMOUNT (0xa0000)
#define TURBOTURNTIME      (5)
#define TURBOTURNAMOUNT    (0x1e000)
#define TURNAROUNDSPEED   ((ANG180<<16)/15)

#define VR_INPUT_SERVICE (0x30)
#define VR_FEEDBACK_SERVICE (0x31)

#define VR_RUNBUTTON         (0)
#define VR_STRAFELEFTBUTTON  (1)
#define VR_STRAFERIGHTBUTTON (2)
#define VR_ATTACKBUTTON      (3)
#define VR_LOOKUPBUTTON      (4)
#define VR_LOOKDOWNBUTTON    (5)
#define VR_SWAPWEAPONBUTTON  (6)
#define VR_USEBUTTON         (7)
#define VR_HORIZONUPBUTTON   (8)
#define VR_HORIZONDOWNBUTTON (9)
#define VR_MAPBUTTON         (10)
#define VR_PISTOLBUTTON      (11)
#define VR_DUALPISTOLBUTTON  (12)
#define VR_MP40BUTTON        (13)
#define VR_MISSILEWEAPONBUTTON (14)
#define VR_RECORDBUTTON      (15)

#define STEPADJUST 3
void BatAttack(objtype*ob);
void  T_DogLick (objtype *ob);
#endif
