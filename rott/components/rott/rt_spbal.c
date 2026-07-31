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
#include "watcom.h"
#include "splib.h"
#include "rt_draw.h"
#include "rt_playr.h"
#include "isr.h"
#include <stdlib.h>
#include "rt_spbal.h"
#include "_rt_spba.h"
#include "sbconfig.h"
#include "rt_main.h"
#include "rt_map.h"

#include "rt_debug.h"
#include "rt_game.h"
#include "rt_str.h"
#include "rt_vid.h"
#include "rt_playr.h"
#include "rt_actor.h"
#include "rt_main.h"
#include "rt_util.h"
#include "rt_draw.h"
#include "rt_in.h"
#include "z_zone.h"
#include "rt_ted.h"
#include "rt_view.h"
#include "develop.h"
#include "version.h"
#include "scriplib.h"
#include <stdlib.h>

#ifdef DOS
#include <conio.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <mem.h>
#endif
//MED
#include "memcheck.h"

#ifdef DOS
/* This file and associated .h files and Spaceball libraries:
   Copyright 1995 Spacetec IMC Corporation
*/
//static char c[]="Copyright 1995 Spacetec IMC Corporation";


#define MSGN(x)    (((x)>0)?1:-1)
#define MABS(x)    ((x<0)?(-x):(x))

#define FF(n) FLOAT_TO_FIXED(n)

#define MAX_WARPS 1  // maximum number of ranges in all warps in defaultWarps
static WarpRange defaultWarps[][MAX_WARPS]={
    {{  0, 511, FF(0.00013)}},   // Tx
    {{  0, 511, FF(0.0075 )}},   // Ty
	 {{  0, 511, FF(0.0005 )}},   // Tz
    {{150, 511, FF(0.0075 )}},   // Rx
	 {{  0, 511, FF(0.15   )}}	   // Ry
};

#undef FF


static WarpRecord defaultRecords[]={
	 {"Tx",  defaultWarps[0], 1},
	 {"Ty",  defaultWarps[1], 1},
	 {"Tz",  defaultWarps[2], 1},
	 {"Rx",  defaultWarps[3], 1},
	 {"Ry",  defaultWarps[4], 1},
};

WarpRecord *WarpTx, *WarpTy, *WarpTz, *WarpRx, *WarpRy;

static char *SpaceBallConfigName = "spwrott.cfg";
static char *ApogeePath = "APOGEECD";

#define    TURBO_LIMIT          1000000
static int turbo_increment    = 500000,
           turbo_count        = 0,
           turboFire          = false;


// sbbuttons mask: 00FE DCBA

#define BUTTON_A    0x01
#define BUTTON_B    0x02
#define BUTTON_C    0x04
#define BUTTON_D    0x08
#define BUTTON_E    0x10
#define BUTTON_F    0x20

// All possible button assignment masks (with defaults)
static
short turboFireMask      = BUTTON_A,
      attackMask         = BUTTON_B,
      useMask            = BUTTON_C,
      mapMask            = 0,
      swapWeaponMask     = BUTTON_D,
      singleAxisMask     = 0,
      planarMask         = 0,
      aimMask            = BUTTON_E,
      pauseMask          = BUTTON_F;

// Array exists just to make sure two tasks are not assigned to same button
static short *masks[6]={&turboFireMask,  &attackMask, &useMask,
                        &swapWeaponMask, &aimMask,    &pauseMask};



//******************************************************************************
//
// ShiftTowardZero ()
//
//******************************************************************************

static short
ShiftTowardZero(short n, short amount)
{
   if (MABS(n) >= amount)
      return n-((n>0)?amount:-amount);
   else
      return 0;
}


//******************************************************************************
//
// SPW_SingleAxisFilter
//
//  Zeroes all but the largest (in absolute value) element in a RawPacket
//
//   returns: nothing
//
//******************************************************************************

static void
SPW_SingleAxisFilter(SpwRawData *p)
{
    short *array   = &(p->tx),  // hope the data are in contigous locations
          *largest = array,
           newabs, large, array_length = 6;

    large = MABS( *array );
    while (--array_length > 0) {
        ++array;
        newabs = MABS(*array);
        if (newabs > large) {
            *largest = 0;
            large = newabs;
            largest = array;
        } else
            *array = 0;
    }

} /* end of SPW_SingleAxisFilter */



//******************************************************************************
//
// HandleSpaceBallMotion ()
//
//******************************************************************************

static void HandleSpaceBallMotion (SpwRawData * npacket, int controlbuf[])
{

   int    strafeAngle;
   short  tx, ty, tz, rx, ry;
   long   sbtx, sbty, sbtz, sbrx, sbry;
   static short ButtonState;

   // If they are really cranking on it, limit them to one axis at a time
   if ((MABS(npacket->tx) > 450) ||
       (MABS(npacket->ty) > 450) ||
       (MABS(npacket->tz) > 450) ||
       (MABS(npacket->rx) > 450) ||
       (MABS(npacket->ry) > 450)) SPW_SingleAxisFilter(npacket);

   // Don't want to lose the low end that the sb null_region calc is losing
   tx=ShiftTowardZero(npacket->tx,30);   // range now approximately +/- 0-480
   ty=ShiftTowardZero(npacket->ty,30);   // range now approximately +/- 0-480
   tz=ShiftTowardZero(npacket->tz,50);   // range now approximately +/- 0-460
   rx=ShiftTowardZero(npacket->rx,60);   // range now approximately +/- 0-450
   ry=ShiftTowardZero(npacket->ry,60);   // range now approximately +/- 0-450

   sbtx = SbFxConfigWarp( WarpTx, tx );
   sbty = SbFxConfigWarp( WarpTy, ty );
   sbtz = SbFxConfigWarp( WarpTz, tz );
   sbrx = SbFxConfigWarp( WarpRx, rx );
   sbry = SbFxConfigWarp( WarpRy, ry );

   strafeAngle = (player->angle - FINEANGLES/4)&(FINEANGLES-1);

   // check for turboFire
   if (turboFire)
      {
      if (MABS(turbo_count) > TURBO_LIMIT)
         turbo_increment *= -1;
      turbo_count += turbo_increment;
      sbry += turbo_increment;
      }

   controlbuf[0] += -(FixedMul (sbtz, viewcos))+
                     FixedMul (sbtx, costable[strafeAngle]);

   controlbuf[1] +=  FixedMul (sbtz, viewsin) -
                    FixedMul (sbtx, sintable[strafeAngle]);

   controlbuf[2] += sbry;



   // Handle looking up and down ^ flying

   ButtonState=npacket->buttons.cur;

   // should the user be running?
   buttonpoll[bt_run] = true; //((MABS(tx)+MABS(tz)+MABS(ry)) > 320) ? true : false ;


   // TY does flying if the user has acquired the ability to fly.
   // Currently this is ^'ed with looking up and down
   if (player->flags & FL_FLEET)
      {
      if (sbty != 0)
         {
         if (sbty > 0)
            buttonpoll[bt_lookup  ] = true;
         else
            buttonpoll[bt_lookdown] = true;
         }
      }
   else  // Lookup/down || aim up/down ?
      {
      if ( sbrx != 0 )
         {
         if (sbrx > 0)
            {
            if (ButtonState & aimMask)
               buttonpoll[bt_horizonup] = true;
            else
               buttonpoll[bt_lookup] = true;
            }
         else
            {
            if (ButtonState & aimMask)
               buttonpoll[bt_horizondown] = true;
            else
               buttonpoll[bt_lookdown] = true;
            }
         }
      }


}

//******************************************************************************
//
// PollSpaceBall ()
//
//******************************************************************************

void PollSpaceBall (void)
{
   SpwRawData rawpacket;
	short buttonsChanged;
   static short buttonState=0;
   static int reusePacketNTimes=0;
   static SpwRawData lastPacket;

   memset(&rawpacket,0,sizeof(rawpacket));
   if (SpwSimpleGet(0,&rawpacket))
      {
      lastPacket=rawpacket;
      reusePacketNTimes=6;
      }
   else if (reusePacketNTimes > 0)
      {
      rawpacket=lastPacket;
      --reusePacketNTimes;
      }
   else if (buttonState) // did not get a packet but a button is down
      rawpacket.buttons.cur=lastPacket.buttons.cur;
   else
      return;


   buttonsChanged=buttonState ^ rawpacket.buttons.cur;
   buttonState=rawpacket.buttons.cur;

   buttonpoll[bt_attack] = (turboFire = (buttonState & turboFireMask) ? true : false );

   if (buttonState & mapMask)          DoMap(player->tilex,player->tiley);
   if (buttonState & useMask)          buttonpoll[bt_use]        = true;
   if (buttonState & attackMask)       buttonpoll[bt_attack]     = true;
   if (buttonState & swapWeaponMask)   buttonpoll[bt_swapweapon] = true;
   if (buttonState & singleAxisMask)   SPW_SingleAxisFilter(&rawpacket);
   if (buttonState & planarMask)       rawpacket.ty=rawpacket.rz=rawpacket.rx=0;
   if (buttonState & pauseMask)        PausePressed=true;
   if (!PausePressed)                  HandleSpaceBallMotion(&rawpacket,controlbuf);
}


//******************************************************************************
//
// OpenSpaceBall ()
//
//******************************************************************************

void OpenSpaceBall (void)
{
   int  btn;
   char filename[ 128 ];

   if (SpwSimpleOpen(0))
      {
      SpwRawData sp;

      SpaceBallPresent = true;
      printf("Test the Spaceball by moving the ball and/or pressing buttons.\n");
      printf("Exit test by pressing any keyboard key...\n");
      while(!kbhit())
         {

         if (SpwSimpleGet(0,&sp))
            printf("\r# tx: %4d ty: %4d tz: %4d # rx: %4d ry: %4d rz: %4d # b:  %1c-%1c-%1c-%1c-%1c-%1c",
                  sp.tx, sp.ty, sp.tz,
                  sp.rx,  sp.ry,  sp.rz,
                  sp.buttons.cur &  1 ? 'A':' ', sp.buttons.cur &  2 ? 'B':' ',
                  sp.buttons.cur &  4 ? 'C':' ', sp.buttons.cur &  8 ? 'D':' ',
                  sp.buttons.cur & 16 ? 'E':' ', sp.buttons.cur & 32 ? 'F':' ');
         }

      // Check for configuration file, set defaults if none

      GetPathFromEnvironment( filename, ApogeePath, SpaceBallConfigName );

	   SbConfigParse(filename);

      // Check for warp records
   	WarpTx = SbConfigGetWarpRange("Tx");
   	WarpTy = SbConfigGetWarpRange("Ty");
   	WarpTz = SbConfigGetWarpRange("Tz");
   	WarpRx = SbConfigGetWarpRange("Rx");
   	WarpRy = SbConfigGetWarpRange("Ry");

	   if(!WarpTx) WarpTx = &defaultRecords[0];
	   if(!WarpTy) WarpTy = &defaultRecords[1];
   	if(!WarpTz) WarpTz = &defaultRecords[2];
   	if(!WarpRx) WarpRx = &defaultRecords[3];
   	if(!WarpRy) WarpRy = &defaultRecords[4];

      // Check for button assignments
	   if((btn=SbConfigGetButtonNumber("TURBO_FIRE"))!=-1)
         {
         if (masks[btn])
            *(masks[btn]) = 0; // zero out mask previously assigned to button
         masks[btn]= &turboFireMask;
		   turboFireMask=(short)(1<<btn);
         }

	   if((btn=SbConfigGetButtonNumber("ATTACK"))!=-1)
         {
         if (masks[btn])
            *masks[btn] = 0; // zero out mask previously assigned to button
         masks[btn]= &attackMask;
		   attackMask=(short)(1<<btn);
         }

	   if((btn=SbConfigGetButtonNumber("USE"))!=-1)
         {
         if (masks[btn])
            *masks[btn] = 0; // zero out mask previously assigned to button
         masks[btn]= &useMask;
		   useMask=(short)(1<<btn);
         }

	   if((btn=SbConfigGetButtonNumber("MAP"))!=-1)
         {
         if (masks[btn])
            *masks[btn] = 0; // zero out mask previously assigned to button
         masks[btn]= &mapMask;
		   mapMask=(short)(1<<btn);
         }

	   if((btn=SbConfigGetButtonNumber("SWAP_WEAPON"))!=-1)
         {
         if (masks[btn])
            *masks[btn] = 0; // zero out mask previously assigned to button
         masks[btn]= &swapWeaponMask;
		   swapWeaponMask=(short)(1<<btn);
         }

	   if((btn=SbConfigGetButtonNumber("SINGLE_AXIS_FILTER"))!=-1)
         {
         if (masks[btn])
            *masks[btn] = 0; // zero out mask previously assigned to button
         masks[btn]= &singleAxisMask;
		   singleAxisMask=(short)(1<<btn);
         }

	   if((btn=SbConfigGetButtonNumber("PLANAR_FILTER"))!=-1)
         {
         if (masks[btn])
            *masks[btn] = 0; // zero out mask previously assigned to button
         masks[btn]= &planarMask;
		   planarMask=(short)(1<<btn);
         }

	   if((btn=SbConfigGetButtonNumber("AIM"))!=-1)
         {
         if (masks[btn])
            *masks[btn] = 0; // zero out mask previously assigned to button
         masks[btn]= &aimMask;
		   aimMask=(short)(1<<btn);
         }

	   if((btn=SbConfigGetButtonNumber("PAUSE"))!=-1)
         {
         if (masks[btn])
            *masks[btn] = 0; // zero out mask previously assigned to button
         masks[btn]= &pauseMask;
		   pauseMask=(short)(1<<btn);
         }
      }
   else
      {
      SpaceBallPresent = false;
      }
}

//******************************************************************************
//
// CloseSpaceBall ()
//
//******************************************************************************

void CloseSpaceBall (void)
{
   if (SpaceBallPresent)
      {
      SpwSimpleClose(0);
      }
}

//******************************************************************************
//
// GetSpaceBallButtons ()
//
//******************************************************************************
 
unsigned GetSpaceBallButtons (void)
{
   SpwRawData sp;

   return ((SpwSimpleGet(0,&sp) & SPW_BUTTON_DOWN));
}

#else

/* This isn't of much use in Linux. */

void PollSpaceBall (void)
{
	STUB_FUNCTION;
}

void OpenSpaceBall (void)
{
	STUB_FUNCTION;
}

void CloseSpaceBall (void)
{
}

unsigned GetSpaceBallButtons (void)
{
	STUB_FUNCTION;
	
	return 0;
}

#endif
