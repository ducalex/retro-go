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
#include "rt_sound.h"
#include "rt_door.h"
#include "rt_actor.h"
#include "rt_stat.h"
#include "_rt_door.h"
#include "z_zone.h"
#include "w_wad.h"
#include "rt_ted.h"
#include "rt_draw.h"
#include "rt_main.h"
#include "rt_playr.h"
#include "rt_util.h"
#include "rt_menu.h"
#include "rt_ted.h"
#include "rt_msg.h"
#include "rt_game.h"
#include "rt_vid.h"
#include "rt_net.h"
#include "isr.h"
#include "develop.h"
#include "rt_rand.h"
#include "engine.h"
#include <stdlib.h>
#include <string.h>
//MED
#include "memcheck.h"

/*=============================================================================

							DOORS

doorobjlist[] holds most of the information for the doors

Open doors conect two areas, so sounds will travel between them and sight
	will be checked when the player is in a connected area.

Areaconnect is incremented/decremented by each door. If >0 they connect

Every time a door opens or closes the areabyplayer matrix gets recalculated.
	An area is true if it connects with the player's current spor.

=============================================================================
*/


// Global Variables


#define ELEVATORMUSICTIME   560

#ifndef RETRO_GO
elevator_t     ELEVATOR[MAXELEVATORS];
#endif
int            _numelevators;
animmaskedwallobj_t* FIRSTANIMMASKEDWALL,*LASTANIMMASKEDWALL;
maskedwallobj_t* FIRSTMASKEDWALL,*LASTMASKEDWALL;
#ifndef RETRO_GO
word             (*touchindices)[MAPSIZE];
#endif
int              lasttouch;
#ifndef RETRO_GO
touchplatetype   *touchplate[MAXTOUCHPLATES],*lastaction[MAXTOUCHPLATES];

byte             numactions[MAXTOUCHPLATES];
#endif
int              totalactions;

#ifndef RETRO_GO
byte             TRIGGER[MAXTOUCHPLATES];
doorobj_t	   *doorobjlist[MAXDOORS];
#endif
int			   doornum;
#ifndef RETRO_GO
maskedwallobj_t *maskobjlist[MAXMASKED];
#endif
int            maskednum;

#ifndef RETRO_GO
pwallobj_t     *pwallobjlist[MAXPWALLS];
#endif
int            pwallnum;

#ifndef RETRO_GO
byte	         areaconnect[NUMAREAS][NUMAREAS];

boolean	      areabyplayer[NUMAREAS];
#endif


// Local Variables

static void (*touchactions[NUMTOUCHPLATEACTIONS])(long) =
						 {ActivatePushWall,
						  ActivateMoveWall,
						  LinkedOpenDoor,
						  LinkedCloseDoor,
						  EnableObject,
						  DisableObject,
						  ActivateLight,
						  DeactivateLight};

#if ((DEVELOPMENT == 1))
#if ((LOADSAVETEST == 1))
static char*touchstrings[NUMTOUCHPLATEACTIONS] =
						 {"ActivatePushWall",
						  "ActivateMoveWall",
						  "LinkedOpenDoor",
						  "LinkedCloseDoor",
						  "EnableObject",
						  "DisableObject",
						  "ActivateLight",
						  "DeactivateLight"
						 };
#endif
#endif
#if ((DEVELOPMENT == 1))
#if ((ELEVATORTEST == 1))

static char*elevstring[NUMELEVATORACTIONS] =
						{"ready at source",
						 "ready at destination",
						 "moving to source",
						 "moving to destination",
						 "doorclosing"
						};
#endif
#endif

void UtilizeDoor (int door,void (*action)(int));
void UseDoor (int door);
void Teleport(elevator_t*eptr,int destination);
void ConnectPushWall (int pwall);
void SetupPushWall (int pwall);
void WallMoving (int pwall);
int SetNextAction(elevator_t*eptr,int action);

/*
===============
=
= MakeMaskedWallActive
=
===============
*/

void MakeMaskedWallActive(maskedwallobj_t* tmwall)
 {if (!FIRSTMASKEDWALL)
	 FIRSTMASKEDWALL	= tmwall;
  else
	  {tmwall->prev = LASTMASKEDWALL;
		LASTMASKEDWALL->next = tmwall;
	  }
  LASTMASKEDWALL = tmwall;
 }

/*
===============
=
= MakeMaskedWallInactive
=
===============
*/


void MakeMaskedWallInactive(maskedwallobj_t* tmwall)
{
 if (tmwall == LASTMASKEDWALL)
	LASTMASKEDWALL = tmwall->prev;
 else
	tmwall->next->prev = tmwall->prev;

 if (tmwall == FIRSTMASKEDWALL)
	FIRSTMASKEDWALL = tmwall->next;
 else
	tmwall->prev->next = tmwall->next;

 tmwall->prev = NULL;
 tmwall->next = NULL;

}


/*
===============
=
= ActivateAnimMaskedWall
=
===============
*/

void ActivateAnimMaskedWall(animmaskedwallobj_t* amwall)
{
   if (!FIRSTANIMMASKEDWALL)
	   FIRSTANIMMASKEDWALL	= amwall;
   else
	   {
      amwall->prev = LASTANIMMASKEDWALL;
		LASTANIMMASKEDWALL->next = amwall;
	   }
   LASTANIMMASKEDWALL = amwall;
}

/*
===============
=
= DeactivateAnimMaskedWall
=
===============
*/


void DeactivateAnimMaskedWall(animmaskedwallobj_t* amwall)
{
 if (amwall == LASTANIMMASKEDWALL)
	LASTANIMMASKEDWALL = amwall->prev;
 else
	amwall->next->prev = amwall->prev;

 if (amwall == FIRSTANIMMASKEDWALL)
	FIRSTANIMMASKEDWALL = amwall->next;
 else
	amwall->prev->next = amwall->next;

 amwall->prev = NULL;
 amwall->next = NULL;

}


int PlatformHeight(int tilex,int tiley)
   {
   int platform;

   if (!IsPlatform(tilex,tiley))
      return nominalheight;

   platform = MAPSPOT(tilex,tiley,2);

   switch(platform)
      {
      case 1:
         return -10;
      case 4:
         return nominalheight;
      case 5:
      case 6:
         return nominalheight - 64;
      case 7:
         return nominalheight;
      case 8:
      case 9:
         return -10;
      }

   return -1000;
   }

void SpawnAnimatedMaskedWall ( int num )
{
   animmaskedwallobj_t * temp;

	temp = (animmaskedwallobj_t *)Z_LevelMalloc(sizeof(animmaskedwallobj_t),PU_LEVELSTRUCT,NULL);
   if (!temp)
      Error("SpawnAnimatedMaskedWall: Failed on allocation of animated masked wall");
   temp->num=num;
   temp->count=AMW_NUMFRAMES;
   temp->ticcount=AMW_TICCOUNT;
   temp->next=NULL;
   temp->prev=NULL;
   ActivateAnimMaskedWall(temp);
}

void KillAnimatedMaskedWall ( animmaskedwallobj_t * temp )
{
   DeactivateAnimMaskedWall(temp);
   Z_Free(temp);
}


void DoAnimatedMaskedWalls ( void )
{
   boolean done;
   animmaskedwallobj_t * temp;

   for(temp=FIRSTANIMMASKEDWALL;temp;)
      {
      done=false;
      temp->ticcount-=tics;
      while (temp->ticcount<0)
         {
         temp->ticcount+=AMW_TICCOUNT;
         temp->count--;
         maskobjlist[temp->num]->bottomtexture++;
         if (temp->count==0)
            {
            done=true;
            break;
            }
         }
      if (done==true)
         {
         animmaskedwallobj_t * temp2;

         temp2=temp->next;
         KillAnimatedMaskedWall(temp);
         temp=temp2;
         }
      else
         temp=temp->next;
      }
}


int GetIndexForAction(void (*action)(long))
{int i;

 for(i=0;i<NUMTOUCHPLATEACTIONS;i++)
  if (action == touchactions[i])
	 return i;

 Error("Touchplate Action Not Matched");
 return -1;
}


void SaveTouchPlates(byte ** buffer,int *size)
{int i,k;
 byte * tptr;
 touchplatetype *temp;
 saved_touch_type dummy;

 *size = MAXTOUCHPLATES;
 *size += MAXTOUCHPLATES;
 *size += sizeof(saved_touch_type)*totalactions;

 *buffer = (byte *)SafeMalloc(*size);
 tptr = *buffer;
 memcpy(tptr,&TRIGGER[0],MAXTOUCHPLATES);
 tptr+=MAXTOUCHPLATES;

 memcpy(tptr,&numactions[0],MAXTOUCHPLATES);
 tptr+=MAXTOUCHPLATES;

 #if ((DEVELOPMENT == 1))
 #if (LOADSAVETEST == 1)
	Debug("\n\nSAVE INFO\n");
	Debug("---------");

	Debug("\n\nTOUCHINDICES\n");
	Debug("------------\n\n");
	for(i=0;i<MAPSIZE;i++)
	 for(j=0;j< MAPSIZE;j++)
		if (touchindices[i][j])
		  Debug("\ntouchindices[%3d][%3d]: %d",i,j,touchindices[i][j]);

	Debug("\n\nTRIGGER: ");
	for(i=0;i<MAXTOUCHPLATES;i++)
	  if (TRIGGER[i])
		 Debug("%1d",TRIGGER[i]);
	Debug("\n\nNUMACTIONS PER TOUCHPLATE\n");
	Debug("-------------------------\n\n");
	for(i=0;i<MAXTOUCHPLATES;i++)
	  if (numactions[i])
	  Debug("\n %2d: %2d",i,numactions[i]);
 #endif
 #endif

 for(i=0;i<lasttouch;i++)
	 {
    #if ((DEVELOPMENT == 1))
    #if (LOADSAVETEST == 1)
	  Debug("\n\nTOUCHPLATE[%2d]\n",i);
	  Debug("--------------\n\n");
	 #endif
	 #endif

	 for(k=0,temp=touchplate[i];temp;k++,temp = temp->nextaction)
		 {
		 dummy.tictime = temp->tictime;
		 dummy.ticcount = temp->ticcount;
		 dummy.triggered = temp->triggered;
		 dummy.done = temp->done;
		 dummy.complete = temp->complete;

		 if (temp->action)
			dummy.actionindex = GetIndexForAction(temp->action);
		 else
			dummy.actionindex = -1;

		 if (temp->swapaction)
			dummy.swapactionindex = GetIndexForAction(temp->swapaction);
		 else
			dummy.swapactionindex = -1;
		 if ((dummy.actionindex > 5) || (dummy.swapactionindex > 5)) // means whichobj holds pointer to actor
			{
			statobj_t *tstat;

			tstat = (statobj_t*)(temp->whichobj);
			dummy.whichobj = (tstat->whichstat|FL_TSTAT);
			}

		 else if ((dummy.actionindex > 3) || (dummy.swapactionindex > 3))

			{
			objtype *tactor;

			tactor = (objtype*)(temp->whichobj);
			dummy.whichobj = (tactor->whichactor|FL_TACT);
			}

		 else
			dummy.whichobj = temp->whichobj;

       #if ((DEVELOPMENT == 1))
       #if (LOADSAVETEST == 1)
		  Debug("action node %d: tictime: %d, ticcount: %d ",k,dummy.tictime,dummy.ticcount);
		  if (dummy.actionindex == -1)
			 Debug("action: -1,");
		  else
			 Debug("action: %13s,",touchstrings[dummy.actionindex]);

		  if (dummy.swapactionindex == -1)
			 Debug("swapaction: -1,");
		  else
			 Debug("swapaction: %13s,",touchstrings[dummy.swapactionindex]);

		  if (dummy.whichobj & FL_TACT)
			 Debug("whichobj (actor): %4x\n",(dummy.whichobj & ~FL_TACT));
		  else
			 Debug("whichobj (nonactor): %4x\n",dummy.whichobj);
		 #endif
		 #endif


		 memcpy(tptr,&dummy,sizeof(saved_touch_type));
		 tptr+=sizeof(saved_touch_type);
		}
	 }
}

statobj_t* GetStatForIndex(int index)
{statobj_t *temp;

 for(temp = FIRSTSTAT;temp;temp=temp->statnext)
	if (temp->whichstat == index)
	  return temp;

 SoftError("\nstat not found in GetStatForIndex");
 return NULL;

}


void LoadTouchPlates(byte * buffer, int size)
{touchplatetype *temp;
 int i,savedactions,loadedactions,index=0;
 saved_touch_type dummy;

 savedactions = (size-MAXTOUCHPLATES-MAXTOUCHPLATES)/sizeof(saved_touch_type);
 memset(touchplate,0,MAXTOUCHPLATES * sizeof(touchplatetype *));
 memset(lastaction,0,MAXTOUCHPLATES * sizeof(touchplatetype *));
 memset(numactions,0,MAXTOUCHPLATES);
 totalactions = 0;

 memcpy(&TRIGGER[0],buffer,MAXTOUCHPLATES);
 buffer += MAXTOUCHPLATES;

 memcpy(&numactions[0],buffer,MAXTOUCHPLATES);
 buffer += MAXTOUCHPLATES;

 for(loadedactions=0,index=0,i=0;i<savedactions;i++)
  {memcpy(&dummy,buffer,sizeof(saved_touch_type));
	temp = (touchplatetype*)Z_LevelMalloc(sizeof(touchplatetype),PU_LEVELSTRUCT,NULL);
	if (!temp)
		Error("LoadTouchplates: Failed on allocation of touchplates %d of %d",i,savedactions);
	memset(temp,0,sizeof(*temp));

	temp->tictime = dummy.tictime;
	temp->ticcount = dummy.ticcount;
	temp->triggered = dummy.triggered;
	temp->done = dummy.done;
	temp->complete = dummy.complete;

	if (dummy.whichobj & FL_TACT)
	  temp->whichobj = (long)(objlist[dummy.whichobj & ~FL_TACT]);

	else if (dummy.whichobj & FL_TSTAT)
	  temp->whichobj = (long)(GetStatForIndex(dummy.whichobj & ~FL_TSTAT));
	else
	  temp->whichobj = dummy.whichobj;
	if (dummy.actionindex != -1)
	 temp->action = touchactions[dummy.actionindex];
	else
	 temp->action = NULL;

	if (dummy.swapactionindex != -1)
	 temp->swapaction = touchactions[dummy.swapactionindex];
	else
	 temp->swapaction = NULL;

	buffer+=sizeof(saved_touch_type);

	while (!numactions[index])
	  index ++;

	AddTouchplateAction(temp,index);

	/*if (touchplate[index])
	  lastaction[index]->nextaction = temp;
	else
	  touchplate[index] = temp;
	lastaction[index] = temp;*/

	totalactions ++;

	loadedactions++;
	if (loadedactions == numactions[index]) // found end of a touchplate's actions, goto next touch.
	 {loadedactions = 0;
	  index++;
	 }
  }


 #if ((DEVELOPMENT == 1))
 #if (LOADSAVETEST == 1)
	Debug("\n\nLOAD INFO\n");
	Debug("---------");

	Debug("\n\nTOUCHINDICES\n");
	Debug("------------\n\n");
	for(i=0;i<MAPSIZE;i++)
	 for(j=0;j< MAPSIZE;j++)
		if (touchindices[i][j])
		  Debug("\ntouchindices[%3d][%3d]: %d",i,j,touchindices[i][j]);

	Debug("\n\nTRIGGER: ");
	for(i=0;i<MAXTOUCHPLATES;i++)
	  if (TRIGGER[i])
		  Debug("%1d",TRIGGER[i]);
	Debug("\n\nNUMACTIONS PER TOUCHPLATE\n");
	Debug("-------------------------\n\n");
	for(i=0;i<MAXTOUCHPLATES;i++)
	  if (numactions[i])
	  Debug("\n %2d: %2d",i,numactions[i]);

	for(i=0;i<lasttouch;i++)
	  {
	  Debug("\n\nTOUCHPLATE[%2d]\n",i);
	  Debug("--------------\n\n");

	  for(k=0,temp=touchplate[i];temp;k++,temp = temp->nextaction)
		  {
		  Debug("action node %d: tictime: %d, ticcount: %d ",k,temp->tictime,temp->ticcount);
		  if (!temp->action)
			 Debug("action: NULL,");
		  else
			 Debug("action: %13s,",touchstrings[GetIndexForAction(temp->action)]);

		  if (!temp->swapaction)
			 Debug("swapaction: NULL,");
		  else
			 Debug("swapaction: %13s,",touchstrings[GetIndexForAction(temp->swapaction)]);

		  Debug("whichobj: %4x\n",(int)temp->whichobj);
		  }
	  }
 #endif
 #endif

   SafeFree(objlist);

}




void AddTouchplateAction(touchplatetype *tplate,int index)
{
 if (touchplate[index])
	 {tplate->prevaction = lastaction[index];
	  lastaction[index]->nextaction = tplate;
	 }
  else
	 touchplate[index] = tplate;
  lastaction[index] = tplate;

}




void  RemoveTouchplateAction(touchplatetype *tplate,int index)
{
  if (tplate == lastaction[index])     // remove from master list
	 lastaction[index] = tplate->prevaction;
  else
	 tplate->nextaction->prevaction = tplate->prevaction;

  if (tplate == touchplate[index])
	 touchplate[index] = tplate->nextaction;
  else
	 tplate->prevaction->nextaction = tplate->nextaction;

  Z_Free(tplate);
  numactions[index]--;
  totalactions--;

}



void  Link_To_Touchplate(word touchlocx, word touchlocy, void (*maction)(long), void (*swapaction)(long), long wobj, int delaytime)
{touchplatetype *temp;
 int index;

 index = touchindices[touchlocx][touchlocy]-1;

 temp = (touchplatetype*)Z_LevelMalloc(sizeof(touchplatetype),PU_LEVELSTRUCT,NULL);
 if (!temp)
	 Error("Link_To_Touchplate: Failed on allocation of touchplate\n");
 memset(temp,0,sizeof(*temp));
 temp->action = maction;
 temp->swapaction = swapaction;
 temp->whichobj = wobj;
 temp->tictime = temp->ticcount = delaytime;

 AddTouchplateAction(temp,index);
 /*if(touchplate[index])
	lastaction[index]->nextaction=temp;
 else
	touchplate[index] = temp;
 lastaction[index] = temp;*/
 numactions[index]++;
 totalactions++;
}



void ClockLink (void (*saction)(long), void (*eaction)(long), long wobj,int whichclock)
{touchplatetype*temp;


 // adding two actions per clock
 temp = (touchplatetype*)Z_LevelMalloc(sizeof(touchplatetype),PU_LEVELSTRUCT,NULL);
 if (!temp)
	 Error("ClockLink: Failed on allocation of clock");
 memset(temp,0,sizeof(*temp));
 temp->action = saction;
 temp->swapaction = eaction;
 temp->whichobj = wobj;
 temp->clocktype = 1;

 AddTouchplateAction(temp,whichclock);
/*  if(touchplate[whichclock])
	lastaction[whichclock]->nextaction = temp;
  else
	touchplate[whichclock] = temp;
  lastaction[whichclock]=temp;*/

 numactions[whichclock]++;
 totalactions ++;
}


void DisplayMessageForAction(touchplatetype *temp, boolean *wallmessage,
                             boolean *doormessage, boolean*columnmessage)
   {

   if ((temp->action == ActivatePushWall) ||
      (temp->action == ActivateMoveWall)
      )
      {
      if (*wallmessage == false)
         {
         if (temp->clocktype)
            AddMessage("Time-delay wall moves.",MSG_GAME);
         else
            AddMessage("A wall moves.",MSG_GAME);
         *wallmessage = true;
         }
      }

   else if (temp->action == LinkedCloseDoor)
      {
      if (*doormessage == false)
         {
         if (temp->clocktype)
            AddMessage("Time-delay door closes.",MSG_GAME);
         else
            AddMessage("A door closes.",MSG_GAME);
         *doormessage = true;
         }
      }

   else if (temp->action == LinkedOpenDoor)
      {
      if (*doormessage == false)
         {
         if (temp->clocktype)
            AddMessage("Time-delay door opens.",MSG_GAME);
         else
            AddMessage("A door opens.",MSG_GAME);
         *doormessage = true;
         }
      }

   else if (temp->action == EnableObject)
      {
      objtype *tempactor = (objtype*)(temp->whichobj);

      if (M_ISACTOR(tempactor) && (tempactor->obclass == pillarobj))
         {
         if (*columnmessage == false)
            {
            if (temp->clocktype)
               AddMessage("Time-delay column moves.",MSG_GAME);
            else
               AddMessage("A column moves.",MSG_GAME);
            *columnmessage = true;
            }
         }
      }
   }

void TriggerStuff(void)
   {
   touchplatetype *temp;
   int i,touchcomplete,j;
   int playeron;
   void (*tempact)(long);
   boolean wallmessage,doormessage,columnmessage;

   for(i=0;i<lasttouch;i++)
      {
      playeron = false;
      for( j = 0; j < numplayers; j++ )
         {
         if ( i == touchindices[ PLAYER[ j ]->tilex ][ PLAYER[ j ]->tiley ] - 1 )
            {
            playeron = true;
            break;
            }
         }
#if (BNACRASHPREVENT == 1)
	  //SetTextMode (  ); qwert
	  //	CRASH IN SHAREWARE 'ride em cowboy' BNA FIX
      // DONT ALLOW BAD touchplate ( == 0 ) see rt_playr.c
	  if (touchplate[i] == 0) {continue;}
#endif

      if (!TRIGGER[i])
         continue;

      else if (touchplate[i]->complete)
         {
         if (!playeron)
            TRIGGER[i] = 0;
         continue;
         }

      if (touchplate[i]->done)
         {
         if (!playeron)
            {
            for(temp = touchplate[i];temp;temp = temp->nextaction)
               temp->triggered=false;
            TRIGGER[i] = 0;
            touchplate[i]->done = false;
            }
         }

      else
         {
         wallmessage = false;
         doormessage = false;
         columnmessage = false;

         for(temp = touchplate[i];temp;temp = temp->nextaction)
            {
            if (temp->action && (!temp->triggered))
               {
               if (!temp->ticcount)
                  {
                  temp->action(temp->whichobj);
                  if (temp->action == ActivateMoveWall)
                     {
                     int tilex,tiley;

                     tilex = pwallobjlist[temp->whichobj]->tilex;
                     tiley = pwallobjlist[temp->whichobj]->tiley;
                     tilemap[tilex][tiley] = 0;

                     }
                  if (gamestate.difficulty == gd_baby)
                     {
                     DisplayMessageForAction(temp,&wallmessage,&doormessage,&columnmessage);
                     }

                  tempact = temp->action;
                  temp->action = temp->swapaction;
                  temp->swapaction = tempact;
                  temp->ticcount = temp->tictime;
                  temp->triggered = true;
                  }

               else
                  temp->ticcount --;
               }
            }
   //done:

         // check to see if any actions will ever be triggered by this
         // touchplate again; if not, null touchplate out; else,
         // check status of other actions

         touchcomplete = 1;
         for(temp = touchplate[i];temp;temp = temp->nextaction)
            {
            if (temp->action)
               {
               touchcomplete = 0;
               break;
               }
            }

         if (touchcomplete)
            touchplate[i]->complete = 1; // this touchplate is out of commission
         else
            {
            touchplate[i]->done = true;
            for(temp = touchplate[i];temp;temp = temp->nextaction)
               {
               if (temp->action && (!temp->triggered))
                  {
                  touchplate[i]->done = false;
                  break;
                  }
               }
            }
         }
      }
   }


//==================== Tile stuff ====================================

boolean CheckTile(int x, int y)
{

	if ((x < 2) || (x > (MAPSIZE-1)) || (y < 2) || (y > (MAPSIZE - 1)))
	  return false;

	if (actorat[x][y])
	  {objtype *check = (objtype*)(actorat[x][y]);
		if (insetupgame)
		  return false;
		if (!(M_ISACTOR(check) && (check->obclass == playerobj)))
		  return false;
     }
   if (DiskAt(x,y))
      return false;
   if (sprites[x][y])
		return false;
	if ((tilemap[x][y]) && (IsPlatform(x,y)==false))
		return false;
	if ((AREANUMBER(x,y)<=0) || (AREANUMBER(x,y)>NUMAREAS))
		return false;
   if (IsWindow(x,y))
      return false;
   return true;
}


#define CountTile(x,y) \
{                       \
 if (oldarea == AREANUMBER(x,y))\
    {if (CheckTile(x,y))         \
        numemptytiles ++;                   \
                                             \
     areanumbercount++;                       \
     if (areanumbercount == numareatiles[oldarea])\
        return numemptytiles;                     \
    }                                              \
                                                    \
}                                                   \


int Number_of_Empty_Tiles_In_Area_Around(int x, int y)
{int roverx,rovery,areanumbercount=0,
     numemptytiles=0,oldarea,i,limit,j;

 oldarea = AREANUMBER(x,y);

   for (i=1;;i++)
      {roverx = x-i;
       rovery = y-i;

       CountTile(roverx,rovery);
       limit = i<<1;

       for(j=0;j<limit;j++)
         {roverx++;
          CountTile(roverx,rovery);
         }

       for(j=0;j<limit;j++)
         {rovery++;
          CountTile(roverx,rovery);
         }

       for(j=0;j<limit;j++)
         {roverx--;
          CountTile(roverx,rovery);
         }

       for(j=0;j<limit-1;j++)
         {rovery--;
          CountTile(roverx,rovery);
         }
      }
}



#define CheckSet(x,y)      \
{if (CheckTile(x,y) && (oldarea == AREANUMBER(x,y))) \
   {*stilex = x;    \
    *stiley = y;    \
    return;              \
   } \
}                     \



void FindEmptyTile(int *stilex, int *stiley)
{
   int i,j,x,y,oldarea,roverx,rovery,limit;

	oldarea = AREANUMBER(*stilex,*stiley);

   x = *stilex;
   y = *stiley;

   if (CheckTile(x,y) && (oldarea == AREANUMBER(x,y)))
     return;

   for (i=1;;i++)
      {roverx = x-i;
       rovery = y-i;

       CheckSet(roverx,rovery);
       limit = i<<1;

       for(j=0;j<limit;j++)
         {roverx++;
          CheckSet(roverx,rovery);
         }

       for(j=0;j<limit;j++)
         {rovery++;
          CheckSet(roverx,rovery);
         }

       for(j=0;j<limit;j++)
         {roverx--;
          CheckSet(roverx,rovery);
         }

       for(j=0;j<limit-1;j++)
         {rovery--;
          CheckSet(roverx,rovery);
         }
      }
}

//================================================================




void RecursiveConnect (int areanumber)
{
	int	i;

	for (i=0;i<NUMAREAS;i++)
	{
		if (areaconnect[areanumber][i] && !areabyplayer[i])
		{
			areabyplayer[i] = true;
			RecursiveConnect (i);
		}
	}
}


/*
==============
=
= ConnectAreas
=
= Scans outward from playerarea, marking all connected areas
=
==============
*/

void ConnectAreas (void)
{objtype*temp;
 statobj_t*tstat;
 int i;
#define MASTER_DISK(ob) ((ob->obclass == diskobj) && (ob->flags & FL_MASTER))

   memset (areabyplayer,0,NUMAREAS * sizeof(rott_boolean));
   for (i=0;i<numplayers;i++)
      {
      areabyplayer[PLAYER[i]->areanumber] = true;
      RecursiveConnect (PLAYER[i]->areanumber);
      }
	for(temp=FIRSTACTOR;temp;temp=temp->next)
		{
      if (MASTER_DISK(temp))
        continue;
		if (!areabyplayer[temp->areanumber])
			continue;
		if (!(temp->flags & FL_ABP))
			{temp->flags |= FL_ABP;
			 MakeActive(temp);
			}
		}

	for(tstat=FIRSTSTAT;tstat;tstat=tstat->statnext)
	  {if (areabyplayer[tstat->areanumber])
		  {if (!(tstat->flags & FL_ABP))
			  {tstat->flags |= FL_ABP;
				MakeStatActive(tstat);
			  }
		  }
		else if (tstat->flags & FL_ABP)
		 {MakeStatInactive(tstat);
		  tstat->flags &= ~FL_ABP;
		 }
	  }

	for(i=0;i<maskednum;i++)
	  {if (areabyplayer[maskobjlist[i]->areanumber])
		  {if (!(maskobjlist[i]->flags & MW_ABP))
			  {maskobjlist[i]->flags |= MW_ABP;
				MakeMaskedWallActive(maskobjlist[i]);
			  }
		  }
		else if (maskobjlist[i]->flags & MW_ABP)
		 {MakeMaskedWallInactive(maskobjlist[i]);
		  maskobjlist[i]->flags &= ~MW_ABP;
		 }
	  }
}


void InitAreas (void)
{
	memset (areabyplayer,0,NUMAREAS * sizeof(rott_boolean));
	memset (areaconnect,0,NUMAREAS * NUMAREAS);
}


/*
===============
=
= InitDoorList
=
===============
*/

void InitDoorList (void)
{
   doornum=0;
   pwallnum=0;
	maskednum=0;
	lasttouch = 0;
   numclocks=0;

	memset(touchindices,0,MAPSIZE * MAPSIZE * sizeof(word));
	memset(touchplate,0,MAXTOUCHPLATES * sizeof(touchplatetype *));
	memset(lastaction,0,MAXTOUCHPLATES * sizeof(touchplatetype *));
	memset(numactions,0,MAXTOUCHPLATES);
	totalactions = 0;
	memset(TRIGGER,0,MAXTOUCHPLATES);
	memset(Clocks,0,sizeof(Clocks));
   FIRSTMASKEDWALL=NULL;
   LASTMASKEDWALL=NULL;
   FIRSTANIMMASKEDWALL=NULL;
   LASTANIMMASKEDWALL=NULL;
}

/*
===============
=
= IsWall
=
===============
*/

int IsWall (int tilex, int tiley)
{
	int map;

	map=MAPSPOT(tilex,tiley,0);

	if ((map>=1) && (map<=89))
		return 1;

	else if ((map>=106) && (map<=107))
		return 1;

	else if ((map>=224) && (map<=233))
		return 1;

	else if ((map>=242) && (map<=244))
		return 1;

	return 0;
}



/*
===============
=
= InitElevators
=
===============
*/

void InitElevators(void)
{_numelevators = 0;
 memset(ELEVATOR,0,MAXELEVATORS * sizeof(elevator_t));

}



/*
===============
=
= IsDoor
=
===============
*/

int IsDoor (int tilex, int tiley)
{
	int map;

	map=MAPSPOT(tilex,tiley,0);

	if ((map>=33) && (map<=35))
		return 1;

	if ((map>=90) && (map<=104))
		return 1;

	if ((map>=154) && (map<=156))
		return 1;

	if (M_ISDOOR(tilex,tiley))
      return 1;

	return 0;
}


/*
===============
=
= SpawnDoor
=
===============
*/

void SpawnDoor (int tilex, int tiley, int lock, int texture)
{
   int i;
   doorobj_t * lastdoorobj;
   int up,dn,lt,rt;
   int abovewallstart;
   int swallstart;
   int basetexture;

   abovewallstart=W_GetNumForName("ABVWSTRT")+1;
   swallstart=W_GetNumForName("SIDESTRT")+1;

   doorobjlist[doornum]=(doorobj_t*)Z_LevelMalloc(sizeof(doorobj_t),PU_LEVELSTRUCT,NULL);
   if (!doorobjlist[doornum])
      Error("SpawnDoor: Failed on allocation of door %d ",doornum);
   memset(doorobjlist[doornum],0,sizeof(doorobj_t));
   lastdoorobj=doorobjlist[doornum];

   if (
        ( MAPSPOT(tilex,tiley,1) >= 29 ) &&
        ( MAPSPOT(tilex,tiley,1) <= 32 )
      )
      {
      lock = MAPSPOT(tilex,tiley,1) - 28;
      }

   lastdoorobj->position = 0;
	lastdoorobj->tilex = tilex;
	lastdoorobj->tiley = tiley;
	lastdoorobj->lock = lock;
	lastdoorobj->action = dr_closed;
	lastdoorobj->which = DOOR;
   lastdoorobj->flags = 0;
	lastdoorobj->eindex = -1;

   //
   // make the door space solid
   //

   if (loadedgame==false)
      actorat[tilex][tiley] = lastdoorobj;

   if (IsDoor(tilex,tiley-1)) up=2;
   else if (IsWall(tilex,tiley-1)) up=1;
   else up=0;

   if (IsDoor(tilex,tiley+1)) dn=2;
   else if (IsWall(tilex,tiley+1)) dn=1;
   else dn=0;

   if (IsDoor(tilex-1,tiley)) lt=2;
   else if (IsWall(tilex-1,tiley)) lt=1;
   else lt=0;

   if (IsDoor(tilex+1,tiley)) rt=2;
   else if (IsWall(tilex+1,tiley)) rt=1;
   else rt=0;

	if ((up==1) && (dn==1))
		lastdoorobj->vertical = true;
	else if ((lt==1) && (rt==1))
		lastdoorobj->vertical = false;
	else if ((up>0) && (dn>0))
		lastdoorobj->vertical = true;
	else if ((lt>0) && (rt>0))
		lastdoorobj->vertical = false;
	else if (up>0)
		lastdoorobj->vertical = true;
	else if (dn>0)
		lastdoorobj->vertical = true;
	else if (lt>0)
		lastdoorobj->vertical = false;
	else if (rt>0)
		lastdoorobj->vertical = false;

   switch (texture)
      {

      case 0:
      case 8:
         basetexture = W_GetNumForName("RAMDOOR1\0");
         break;

      case 1:
      case 9:
         basetexture = W_GetNumForName("DOOR2\0");
         break;


      case 2:
      case 3:
      case 13:
         basetexture = W_GetNumForName("TRIDOOR1\0");
         break;

      case 10:
      case 11:
      case 14:
         basetexture = W_GetNumForName("SDOOR4\0");
         break;



      case 12:
         basetexture = W_GetNumForName("EDOOR\0");
         break;
      case 15:
         basetexture = W_GetNumForName("SNDOOR\0");
         break;
      case 16:
         basetexture = W_GetNumForName("SNADOOR\0");
         break;
      case 17:
         basetexture = W_GetNumForName("SNKDOOR\0");
         break;

      case 18:
         basetexture = W_GetNumForName("TNDOOR\0");
         break;
      case 19:
         basetexture = W_GetNumForName("TNADOOR\0");
         break;
      case 20:
         basetexture = W_GetNumForName("TNKDOOR\0");
         break;
      default:
         Error("Illegal door value encountered\n");
         break;
      }
   lastdoorobj->basetexture = basetexture;
	lastdoorobj->texture = lastdoorobj->basetexture;

   SD_PreCacheSoundGroup(SD_OPENDOORSND,SD_CLOSEDOORSND);

//
// make the door tile a special tile, and mark the adjacent tiles
// for door sides
//
   tilemap[tilex][tiley] = doornum | 0x8000;

   switch (texture)
      {
      case 0:
      case 1:
      case 2:
      case 3:
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
         lastdoorobj->sidepic    = W_GetNumForName("SIDE8");
         lastdoorobj->alttexture = W_GetNumForName("ABOVEW3");
         break;

      case 15:
      case 16:
      case 17:
      #if (SHAREWARE == 1)
         lastdoorobj->sidepic = W_GetNumForName("SIDE8");

      #else
         lastdoorobj->sidepic    = W_GetNumForName("SIDE16");
      #endif

         lastdoorobj->alttexture = W_GetNumForName("ABOVEW16");
         break;

      case 18:
      case 19:
      case 20:
      #if (SHAREWARE == 1)
         lastdoorobj->sidepic = W_GetNumForName("SIDE8");
         lastdoorobj->alttexture = W_GetNumForName("ABOVEW3");

      #else
         lastdoorobj->sidepic    = W_GetNumForName("SIDE17");
         lastdoorobj->alttexture = W_GetNumForName("ABOVEW17");

      #endif

         break;
      default:
         Error("Illegal door value encountered\n");
         break;
      }



   if ((lock>0) && (lock<5))
      lastdoorobj->sidepic    = W_GetNumForName("lock1")+lock-1;

   PreCacheLump(lastdoorobj->sidepic,PU_CACHEWALLS,cache_pic_t);
   PreCacheLump(lastdoorobj->alttexture,PU_CACHEWALLS,cache_pic_t);

	if (lastdoorobj->vertical==true)
	{
      if (up==1)
		   tilemap[tilex][tiley-1] |= 0x4000;
      else if (up==2)
         lastdoorobj->flags|=DF_MULTI;
      if (dn==1)
		   tilemap[tilex][tiley+1] |= 0x4000;
      else if (dn==2)
         lastdoorobj->flags|=DF_MULTI;
	}
	else
	{
      if (lt==1)
		   tilemap[tilex-1][tiley] |= 0x4000;
      else if (lt==2)
         lastdoorobj->flags|=DF_MULTI;
      if (rt==1)
		   tilemap[tilex+1][tiley] |= 0x4000;
      else if (rt==2)
         lastdoorobj->flags|=DF_MULTI;
	}

   PreCacheLump(lastdoorobj->texture,PU_CACHEWALLS,cache_pic_t);
   for (i=1;i<9;i++) // only first texture is pic_t!
      PreCacheLump(lastdoorobj->texture+i,PU_CACHEWALLS,cache_patch_t);
	doornum++;
	lastdoorobj++;
	if (doornum==MAXDOORS)
		Error ("Too many doors on level!");

}

/*
===============
=
= MakeWideDoorVisible
=
===============
*/
void MakeWideDoorVisible ( int doornum )
{
   int dx,dy;
   doorobj_t * dr2;
   doorobj_t * dr;
   int tx,ty;

   dr=doorobjlist[doornum];

   dx=0;dy=0;
   if (dr->vertical==true)
      dy=1;
   else
      dx=1;
   spotvis[dr->tilex][dr->tiley]=1;
   tx=dr->tilex+dx;
   ty=dr->tiley+dy;
	while (M_ISDOOR(tx,ty))
      {
      int num;

      num=tilemap[tx][ty]&0x3ff;
      dr2=doorobjlist[num];
      if (!(dr2->flags&DF_MULTI))
         break;
      spotvis[tx][ty]=1;

      tx+=dx;
      ty+=dy;
      }
   tx=dr->tilex-dx;
   ty=dr->tiley-dy;
	while (M_ISDOOR(tx,ty))
      {
      int num;

      num=tilemap[tx][ty]&0x3ff;
      dr2=doorobjlist[num];
      if (!(dr2->flags&DF_MULTI))
         break;
      spotvis[tx][ty]=1;

      tx-=dx;
      ty-=dy;
      }
}

/*
=====================
=
= LockLinkedDoor
=
=====================
*/

void LockLinkedDoor (int door)
{
	doorobj_t*dptr;

	dptr = doorobjlist[door];
   if (!dptr->lock)
      dptr->lock=5;
}

/*
=====================
=
= IsDoorLinked
=
=====================
*/

boolean IsDoorLinked (int door)
{
	doorobj_t*dptr;

	dptr = doorobjlist[door];
   if (dptr->lock==5)
      return true;
   return false;
}


/*
===============
=
= FixDoorAreaNumbers
=
===============
*/
void FixDoorAreaNumbers ( void )
{
   int i;
   int up,dn,lt,rt;
   int tilex,tiley;

   for (i=0;i<doornum;i++)
      {
      tilex=doorobjlist[i]->tilex;
      tiley=doorobjlist[i]->tiley;
      up=MAPSPOT(tilex,tiley-1,0)-AREATILE;
		dn=MAPSPOT(tilex,tiley+1,0)-AREATILE;
		lt=MAPSPOT(tilex-1,tiley,0)-AREATILE;
		rt=MAPSPOT(tilex+1,tiley,0)-AREATILE;

      up = ((up>0) && (up<=NUMAREAS));
		dn = ((dn>0) && (dn<=NUMAREAS));
		lt = ((lt>0) && (lt<=NUMAREAS));
		rt = ((rt>0) && (rt<=NUMAREAS));


		if (doorobjlist[i]->vertical==true)
			{
         if (rt)
				MAPSPOT(tilex,tiley,0) = MAPSPOT(tilex+1,tiley,0);
         else if (lt)
				MAPSPOT(tilex,tiley,0) = MAPSPOT(tilex-1,tiley,0);
			else
				Error("FixDoors: Couldn't fix up area at x=%d y=%d\n",tilex,tiley);
			}
		else
			{
         if (dn)
				MAPSPOT(tilex,tiley,0) = MAPSPOT(tilex,tiley+1,0);
         else if (up)
				MAPSPOT(tilex,tiley,0) = MAPSPOT(tilex,tiley-1,0);
			else
				Error("FixDoors: Couldn't fix up area at x=%d y=%d\n",tilex,tiley);
			}
      if (IsDoorLinked(i))
         UtilizeDoor(i,LockLinkedDoor);
		}
}


//===========================================================================

/*
=====================
=
= OpenDoor
=
=====================
*/

void OpenDoor (int door)
{
	if (doorobjlist[door]->action == dr_open)
		doorobjlist[door]->ticcount = 0;			// reset open time
	else
      {
		doorobjlist[door]->action = dr_opening;	// start it opening
      }
}

/*
=====================
=
= DoorUnBlocked
=
=====================
*/

boolean DoorUnBlocked (int door)
{
	int	tilex,tiley;
	objtype *check;
	doorobj_t*dptr;

	dptr = doorobjlist[door];

//
// don't close on anything solid
//

	tilex = dptr->tilex;
	tiley = dptr->tiley;
	check = (objtype*)actorat[tilex][tiley];

	if (check && (check->which == ACTOR))
		return false;

	if (dptr->vertical==true)
	{
		check = (objtype*)actorat[tilex-1][tiley];
		if (check && (check->which==ACTOR) && ((check->x+MINDIST) >> TILESHIFT) == tilex )
			return false;
		check = (objtype*)actorat[tilex+1][tiley];
		if (check && (check->which==ACTOR) && ((check->x-MINDIST) >> TILESHIFT) == tilex )
			return false;
	}
	else if (dptr->vertical==false)
	{
		check = (objtype*)actorat[tilex][tiley-1];
		if (check && (check->which==ACTOR) && ((check->y+MINDIST) >> TILESHIFT) == tiley )
			return false;
		check = (objtype*)actorat[tilex][tiley+1];
		if (check && (check->which==ACTOR) && ((check->y-MINDIST) >> TILESHIFT) == tiley )
			return false;
	}
   return true;
}


/*
=====================
=
= DoorReadyToClose
=
= Alter the door's state
=
=====================
*/

boolean DoorReadyToClose(int door)
{
	doorobj_t*dptr;
	int dx,dy;
	doorobj_t * dr2;
	int tx,ty;


	dptr = doorobjlist[door];

	if (dptr->action==dr_closed)
		return true;

	if (DoorUnBlocked(door)==false)
		return false;

	dx=0;dy=0;
	if (dptr->vertical==true)
		dy=1;
	else
		dx=1;
	tx=dptr->tilex+dx;
	ty=dptr->tiley+dy;
	while (M_ISDOOR(tx,ty))
		{
		int num;

		num=tilemap[tx][ty]&0x3ff;
		dr2=doorobjlist[num];
		if (!(dr2->flags&DF_MULTI))
			break;
		if (DoorUnBlocked(num)==false)
			return false;
		tx+=dx;
		ty+=dy;
		}
	tx=dptr->tilex-dx;
	ty=dptr->tiley-dy;
	while (M_ISDOOR(tx,ty))
		{
		int num;

		num=tilemap[tx][ty]&0x3ff;
		dr2=doorobjlist[num];
		if (!(dr2->flags&DF_MULTI))
			break;
		if (DoorUnBlocked(num)==false)
			return false;
		tx-=dx;
		ty-=dy;
		}
	return true;
}


/*
=====================
=
= CloseDoor
=
=====================
*/

void CloseDoor (int door)
{
	int	tilex,tiley,area;
	doorobj_t*dptr;

	dptr = doorobjlist[door];
   if (dptr->action == dr_closed)
      return;
	tilex = dptr->tilex;
	tiley = dptr->tiley;

//
// play door sound
//
   area = MAPSPOT(tilex,tiley,0)-AREATILE;
   if (areabyplayer[area])
      {
      dptr->soundhandle=SD_PlaySoundRTP ( SD_CLOSEDOORSND, dptr->tilex<<16, dptr->tiley<<16 );
      }

	dptr->action = dr_closing;
//
// make the door space solid
//
	actorat[tilex][tiley] = dptr;
}



/*
=====================
=
= OperateDoor
=
= The player wants to change the door's direction
=
=====================
*/

void OperateDoor (int keys, int door, boolean localplayer )
{
	int	lock;
	doorobj_t*dptr;

	dptr = doorobjlist[door];
	if ( ( dptr->flags & DF_ELEVLOCKED ) ||
      ( MISCVARS->GASON && ( MAPSPOT( dptr->tilex,
      dptr->tiley, 1 ) == GASVALUE ) ) )
      {
      if ( localplayer )
         {
         // locked
         SD_Play ( SD_NOITEMSND );
         }
      return;
      }

   lock = dptr->lock;
	if ( lock && !( keys & ( 1 << ( lock - 1 ) ) ) )
      {
      if ( localplayer )
         {
         // locked
         switch (lock)
            {
            case 1:
               AddMessage("You need the \\EGOLD key",MSG_DOOR);
               break;

            case 2:
               AddMessage("You need the \\FSILVER key",MSG_DOOR);
               break;

            case 3:
               AddMessage("You need the \\8IRON key",MSG_DOOR);
               break;

            case 4:
               AddMessage("You need the \\AOSCURO key",MSG_DOOR);
               break;

            default:
               AddMessage("This door appears to be locked",MSG_DOOR);
               break;
            }

         SD_Play( SD_NOITEMSND );
         }
      return;
      }
   UseDoor(door);
   }

/*
=====================
=
= LinkedOpenDoor
=
=====================
*/

void LinkedOpenDoor (long door)
{
   UtilizeDoor(door,OpenDoor);
}

/*
=====================
=
= LinkedCloseDoor
=
=====================
*/

void LinkedCloseDoor (long door)
{
   if (DoorReadyToClose(door)==true)
      UtilizeDoor(door,CloseDoor);
}

/*
=====================
=
= UtilizeDoor
=
= Alter the door's state
=
=====================
*/

void UtilizeDoor (int door,void (*action)(int))
{
	doorobj_t*dptr;
   int dx,dy;
   doorobj_t * dr2;
   int tx,ty;

	dptr = doorobjlist[door];

   action(door);

   dx=0;dy=0;
   if (dptr->vertical==true)
      dy=1;
   else
      dx=1;
   tx=dptr->tilex+dx;
   ty=dptr->tiley+dy;
	while (M_ISDOOR(tx,ty))
      {
      int num;

      num=tilemap[tx][ty]&0x3ff;
      dr2=doorobjlist[num];
      if (!(dr2->flags&DF_MULTI))
         break;
      action(num);
      tx+=dx;
      ty+=dy;
      }
   tx=dptr->tilex-dx;
   ty=dptr->tiley-dy;
	while (M_ISDOOR(tx,ty))
      {
      int num;

      num=tilemap[tx][ty]&0x3ff;
      dr2=doorobjlist[num];
      if (!(dr2->flags&DF_MULTI))
         break;
      action(num);
      tx-=dx;
      ty-=dy;
      }
}

/*
=====================
=
= UseDoor
=
= Alter the door's state
=
=====================
*/

void UseDoor (int door)
{
	switch (doorobjlist[door]->action)
	{
	case dr_closing:
		SD_StopSound(doorobjlist[door]->soundhandle);
	case dr_closed:
      UtilizeDoor(door,OpenDoor);
		break;
	case dr_opening:
		SD_StopSound(doorobjlist[door]->soundhandle);
	case dr_open:
      if (DoorReadyToClose(door)==true)
         UtilizeDoor(door,CloseDoor);
		break;
	}
}

//===========================================================================

/*
===============
=
= DoorOpen
=
= Close the door after three seconds
=
===============
*/

void DoorOpen (int door)
{doorobj_t* dptr;

 dptr = doorobjlist[door];
 dptr->ticcount += 1;
 if ((dptr->ticcount >= OPENTICS) &&
     (!(dptr->flags & DF_TIMED)) &&
     (DoorReadyToClose(door)==true))
    UtilizeDoor(door,CloseDoor);
}



/*
===============
=
= DoorOpening
=
===============
*/

void DoorOpening (int door)
{
	int		area1,area2;
	word  	*map;
	long	   position;
	int      tilex,tiley;

	position = doorobjlist[door]->position;
	tilex = doorobjlist[door]->tilex;
	tiley = doorobjlist[door]->tiley;
	if (!position)
	{
	//
	// door is just starting to open, so connect the areas
	//
		map = &MAPSPOT(tilex,tiley,0);

		if (doorobjlist[door]->vertical==true)
		{
			area1 =	*(map+1);
			area2 =	*(map-1);
		}
		else
		{
		 area1 =	*(map-mapwidth);
		 area2 =	*(map+mapwidth);
		}
		area1 -= AREATILE;
		area2 -= AREATILE;
		areaconnect[area1][area2]++;
		areaconnect[area2][area1]++;
		if ((insetupgame==false) && (loadedgame==false))
			ConnectAreas ();
	   if (areabyplayer[area1])
    	   {
         doorobjlist[door]->soundhandle=SD_PlaySoundRTP ( SD_OPENDOORSND, doorobjlist[door]->tilex<<16, doorobjlist[door]->tiley<<16 );
         }
	}

//
// slide the door by an adaptive amount
//
	position += 1<<12;
	if (position >= 0xffff)
	{
	//
	// door is all the way open
	//
		position = 0xffff;
		doorobjlist[door]->ticcount = 0;
		doorobjlist[door]->action = dr_open;
		if (doorobjlist[door] == actorat[tilex][tiley])
			actorat[tilex][tiley] = 0;
	}

	doorobjlist[door]->position = position;
	doorobjlist[door]->texture=doorobjlist[door]->basetexture+((position+1)>>13);
}


/*
===============
=
= DoorClosing
=
===============
*/

void DoorClosing (int door)
{
	int		area1,area2;
	word	   *map;
	long	   position;
	int		tilex,tiley;
	doorobj_t *dptr;

	dptr = doorobjlist[door];

	tilex = dptr->tilex;
	tiley = dptr->tiley;

	position = dptr->position;

//
// slide the door by an adaptive amount
//
	position -= 1<<12;
	if (position < (0xffff >> 1))
	   ResolveDoorSpace(tilex,tiley);


	if (position <= 0)
	{
	//
	// door is closed all the way, so disconnect the areas
	//
		position = 0;

		dptr->action = dr_closed;



		map = &MAPSPOT(tilex,tiley,0);

      if (areabyplayer[(*map-AREATILE)])
         {
         dptr->soundhandle=SD_PlaySoundRTP ( SD_DOORHITSND, dptr->tilex<<16, dptr->tiley<<16 );
         }

      if (dptr->vertical==true)
		{
			area1 =	*(map+1);
			area2 =	*(map-1);
		}
		else
		{
			area1 =	*(map-mapwidth);
			area2 =	*(map+mapwidth);
		}
		area1 -= AREATILE;
		area2 -= AREATILE;
		areaconnect[area1][area2]--;
		areaconnect[area2][area1]--;

		ConnectAreas ();
   }

	dptr->position = position;
	dptr->texture=dptr->basetexture+((position+1)>>13);
}

/*
===============
=
= IsMaskedWall
=
===============
*/

int IsMaskedWall (int tilex, int tiley)
{
	int map;

	if (IsPlatform(tilex,tiley))
		return 1;

	map=MAPSPOT(tilex,tiley,0);

	if ((map>=157) && (map<=160))
		return 1;

	if ((map>=162) && (map<=179))
		return 1;

	if (M_ISMWALL(tilex,tiley))
      return 1;

	return 0;
}

/*
===============
=
= SpawnMaskedWall
=
===============
*/


void SpawnMaskedWall (int tilex, int tiley, int which, int flags)
{word *map;
 int area1, area2;
 int up,dn,lt,rt;
 int himask;
 boolean sidepic;
 int side, middle, above, bottom;
 maskedwallobj_t * lastmaskobj;
 boolean metal;
 int maskedstart;
 int abovemaskedwallstart;
 int swallstart;

	himask=W_GetNumForName("HMSKSTRT")+1;
   maskedstart=W_GetNumForName("MASKSTRT");
   abovemaskedwallstart=W_GetNumForName("ABVMSTRT");
   swallstart=W_GetNumForName("SIDESTRT");

	maskobjlist[maskednum]=(maskedwallobj_t*)Z_LevelMalloc(sizeof(maskedwallobj_t),PU_LEVELSTRUCT,NULL);
	memset(maskobjlist[maskednum],0,sizeof(maskedwallobj_t));
	lastmaskobj=maskobjlist[maskednum];

   sidepic=true;

	lastmaskobj->tilex = tilex;
	lastmaskobj->tiley = tiley;
	lastmaskobj->which = MWALL;
	up=MAPSPOT(tilex,tiley-1,0)-AREATILE;
	dn=MAPSPOT(tilex,tiley+1,0)-AREATILE;
	lt=MAPSPOT(tilex-1,tiley,0)-AREATILE;
	rt=MAPSPOT(tilex+1,tiley,0)-AREATILE;

   if (IsMaskedWall(tilex,tiley-1)) up=2;
   else if (IsWall(tilex,tiley-1)) up=1;
   else up=0;

   if (IsMaskedWall(tilex,tiley+1)) dn=2;
   else if (IsWall(tilex,tiley+1)) dn=1;
   else dn=0;

   if (IsMaskedWall(tilex-1,tiley)) lt=2;
   else if (IsWall(tilex-1,tiley)) lt=1;
   else lt=0;

   if (IsMaskedWall(tilex+1,tiley)) rt=2;
   else if (IsWall(tilex+1,tiley)) rt=1;
   else rt=0;

	if ((up==1) && (dn==1))
		lastmaskobj->vertical = true;
	else if ((lt==1) && (rt==1))
		lastmaskobj->vertical = false;
	else if ((up>0) && (dn>0))
		lastmaskobj->vertical = true;
	else if ((lt>0) && (rt>0))
		lastmaskobj->vertical = false;
	else if (up>0)
		lastmaskobj->vertical = true;
	else if (dn>0)
		lastmaskobj->vertical = true;
	else if (lt>0)
		lastmaskobj->vertical = false;
	else if (rt>0)
		lastmaskobj->vertical = false;

	tilemap[tilex][tiley] = maskednum | 0xc000;
	map = &MAPSPOT(tilex,tiley,0);

	if (lastmaskobj->vertical==true)
		{
		area1 =	*(map+1);
		area2 =	*(map-1);
      area1 -= AREATILE;
		area2 -= AREATILE;
		if (lt==0 && rt==0)
			{
			areaconnect[area1][area2]++;
			areaconnect[area2][area1]++;
			}
		}
	else
		{
		area1 =	*(map-mapwidth);
		area2 =	*(map+mapwidth);
      area1 -= AREATILE;
		area2 -= AREATILE;
		if (up==0 && dn==0)
			{
			areaconnect[area1][area2]++;
			areaconnect[area2][area1]++;
			}
		}
	lastmaskobj->flags=flags;

   if (IsPlatform(tilex,tiley))
      {
      if (MAPSPOT(tilex,tiley,0)==21)
         {
         metal=true;
			actorat[tilex][tiley]=0;
         }
      else
         metal=false;
      }



   switch (which)
      {
      case mw_peephole:

      //#if (SHAREWARE == 1)
         side = W_GetNumForName("SIDE21");
         middle = W_GetNumForName("ABOVEM4A") ;
         above  = W_GetNumForName("ABOVEM4") ;
      /*
      #else
         side   = W_GetNumForName("SIDE16");
         middle = W_GetNumForName("ABOVEM3A") ;
         above  = W_GetNumForName("ABOVEM2A") ;

      #endif
      */
         bottom = W_GetNumForName("PEEPMASK");
         break;


      case mw_dogwall:

         side = W_GetNumForName("SIDE21");
         above  = W_GetNumForName("ABOVEM4") ;

   #if (SHAREWARE == 1)
         middle = W_GetNumForName("ABOVEM4A") ;
   #else
         middle = W_GetNumForName("ABOVEM9") ;

   #endif
         bottom = W_GetNumForName("DOGMASK");
         break;

      case mw_multi1:

  /*
      #if (SHAREWARE == 1)
         side = W_GetNumForName("SIDE21");
         middle = W_GetNumForName("ABOVEM4A") ;
         above  = W_GetNumForName("ABOVEM4") ;

      #else
  */
         //side   = W_GetNumForName("SIDE23") ;
         side   = W_GetNumForName("SIDE21") ;
         middle = W_GetNumForName("ABOVEM5A") ;
         above  = W_GetNumForName("ABOVEM5") ;

     // #endif

         bottom = W_GetNumForName("MULTI1");
         break;

      case mw_multi2:
      /*
      #if (SHAREWARE == 1)
         side = W_GetNumForName("SIDE21");
         middle = W_GetNumForName("ABOVEM4A");
         above  = W_GetNumForName("ABOVEM4") ;

      #else
      */
         //side   = W_GetNumForName("SIDE23") ;
         side   = W_GetNumForName("SIDE21") ;
         middle = W_GetNumForName("ABOVEM5B");
         above  = W_GetNumForName("ABOVEM5") ;


      //#endif

         bottom = W_GetNumForName("MULTI2");
         break;

      case mw_multi3:

      /*
      #if (SHAREWARE == 1)
         side = W_GetNumForName("SIDE21");
         middle = W_GetNumForName("ABOVEM4A") ;
         above  = W_GetNumForName("ABOVEM4")  ;

      #else
      */
         //side   = W_GetNumForName("SIDE23") ;
         side   = W_GetNumForName("SIDE21") ;
         middle = W_GetNumForName("ABOVEM5C") ;
         above  = W_GetNumForName("ABOVEM5")  ;


      //#endif

         bottom = W_GetNumForName("MULTI3");
         break;

      case mw_singlepane:

   //   #if (SHAREWARE == 1)
         side = W_GetNumForName("SIDE21");

   //   #else
   //      side   = W_GetNumForName("SIDE22") ;
   //   #endif

         middle = W_GetNumForName("ABOVEM4A") ;
         above  = W_GetNumForName("ABOVEM4")  ;
         bottom = W_GetNumForName("MASKED4");
         break;

      case mw_normal1:
         side = W_GetNumForName("SIDE21");


     // #if (SHAREWARE == 1)
         middle = W_GetNumForName("ABOVEM4A") ;
         above  = W_GetNumForName("ABOVEM4")  ;

      //#else
       //  middle = W_GetNumForName("ABOVEM1A") ;
       //  above  = W_GetNumForName("ABOVEM1")  ;


      //#endif

         bottom = W_GetNumForName("MASKED1");
         break;

      case mw_normal2:
         side = W_GetNumForName("SIDE21");

      //#if (SHAREWARE == 1)
         middle = W_GetNumForName("ABOVEM4A") ;
         above  = W_GetNumForName("ABOVEM4")  ;

      //#else
      //   middle = W_GetNumForName("ABOVEM2A") ;
      //   above  = W_GetNumForName("ABOVEM2")  ;

      //#endif

         bottom = W_GetNumForName("MASKED2");
         break;

      case mw_normal3:
         side = W_GetNumForName("SIDE21");

      //#if (SHAREWARE == 1)
         middle = W_GetNumForName("ABOVEM4A") ;
         above  = W_GetNumForName("ABOVEM4")  ;

      //#else
      //   middle = W_GetNumForName("ABOVEM3A") ;
      //   above  = W_GetNumForName("ABOVEM3")  ;

      //#endif

         bottom = W_GetNumForName("MASKED3");
         break;

      case mw_exitarch:

         side = W_GetNumForName("SIDE21");

      //#if (SHAREWARE == 1)
         middle = W_GetNumForName("ABOVEM4A") ;
         above  = W_GetNumForName("ABOVEM4")  ;

      //#else
      //   middle = W_GetNumForName("ABOVEM6A") ;
      //   above  = W_GetNumForName("ABOVEM6")  ;

      //#endif

         bottom = W_GetNumForName("EXITARCH");
         break;

      case mw_secretexitarch:

         side = W_GetNumForName("SIDE21");

      //#if (SHAREWARE == 1)
         middle = W_GetNumForName("ABOVEM4A") ;
         above  = W_GetNumForName("ABOVEM4")  ;

      //#else
      //   middle = W_GetNumForName("ABOVEM8A") ;
      //   above  = W_GetNumForName("ABOVEM8")  ;

      //#endif

         bottom = W_GetNumForName("EXITARCA");
         break;

      case mw_railing:
         sidepic = false;
         middle = -1;
         above  = -1;
         bottom = W_GetNumForName("RAILING");
         break;

      case mw_hiswitchon:
         sidepic = false;
         middle = himask+1;
         above = himask+3;
         bottom = himask;
         break;

      case mw_hiswitchoff:
         sidepic = false;
         middle = himask+1;
         above = himask+2;
         bottom = himask;
         break;

      case mw_entrygate:
         side = W_GetNumForName("SIDE21");

      //#if (SHAREWARE == 1)
         //side = W_GetNumForName("SIDE21");
         middle = W_GetNumForName("ABOVEM4A") ;
         above  = W_GetNumForName("ABOVEM4")  ;

      //#else
         //side   = W_GetNumForName("SIDE20") ;
      //   middle = W_GetNumForName("ABOVEM7A") ;
      //   above  = W_GetNumForName("ABOVEM7")  ;


      //#endif

         bottom = W_GetNumForName("ENTRARCH");
         break;

      case mw_platform1:
         sidepic = false;
         bottom = -1;
         middle = -1;
         above  = himask+10;
         if (metal==true)
            {
            bottom = -1;
            middle = -1;
            above  = himask+15;
            }
         break;
      case mw_platform2:
         sidepic = false;
         bottom = himask+8;
         middle = -1;
         above = -1;
         if (metal==true)
            {
            bottom = himask+14;
            middle = -1;
            above = -1;
            }
         else
            lastmaskobj->flags|=MW_BOTTOMFLIPPING;
         break;
      case mw_platform3:
         sidepic = false;
         bottom = himask+8;
         middle = -1;
         above = himask+10;
         if (metal==true)
            {
            bottom = himask+14;
            middle = -1;
            above = himask+15;
            }
         else
            lastmaskobj->flags|=MW_BOTTOMFLIPPING;
         break;
      case mw_platform4:
         sidepic = false;
         bottom = himask+12;
         middle = himask+7;
         above = himask+7;
         if (metal==true)
            {
            bottom = -1;
            middle = himask+15;
            above = himask+15;
            }
         break;
      case mw_platform5:
         sidepic = false;
         bottom = himask+12;
         middle = himask+7;
         above = himask+5;
         if (metal==true)
            {
            bottom = -1;
            middle = himask+15;
            above = -1;
            }
         else
            lastmaskobj->flags|=MW_TOPFLIPPING;
         break;
      case mw_platform6:
         sidepic = false;
         bottom = himask+4;
         middle = himask+7;
         above = himask+5;
         if (metal==true)
            {
            bottom = himask+14;
            middle = himask+15;
            above = -1;
            }
         else
            lastmaskobj->flags|=MW_TOPFLIPPING;
         break;
      case mw_platform7:
         sidepic = false;
         bottom = himask+4;
         middle = himask+7;
         above = himask+5;
         if ((up==1) || (dn==1))
            lastmaskobj->vertical=true;
         else if ((lt==1) || (rt==1))
            lastmaskobj->vertical=false;
         else
            Error("Perpendicular platform used with no wall near it\n");
         if (metal==true)
            {
            bottom = himask+14;
            middle = himask+15;
            above = -1;
            }
         else
            lastmaskobj->flags|=MW_TOPFLIPPING;
         break;
      }

   switch (which)
      {
      case mw_multi1:
      case mw_multi2:
      case mw_multi3:
      case mw_singlepane:
      case mw_normal1:
      case mw_normal2:
      case mw_normal3:
         if (!(flags & MW_SHOOTABLE))
            bottom+=9;
         break;
      }
   lastmaskobj->midtexture=middle;
   lastmaskobj->toptexture=above;
   lastmaskobj->bottomtexture=bottom;

   if (sidepic == true)
      {
      lastmaskobj->sidepic=side;
      if (lastmaskobj->vertical==true)
         {
         if (up==1)
            tilemap[tilex][tiley-1] |= 0x4000;
         if (dn==1)
            tilemap[tilex][tiley+1] |= 0x4000;
         }
      else
         {
         if (lt==1)
            tilemap[tilex-1][tiley] |= 0x4000;
         if (rt==1)
            tilemap[tilex+1][tiley] |= 0x4000;
         }
      }

	// Cache in the broken version

	if (lastmaskobj->flags & MW_SHOOTABLE)
      {
      int i;

      for (i=1;i<AMW_NUMFRAMES;i++)
         {
         PreCacheLump(lastmaskobj->bottomtexture+i,PU_CACHEWALLS,cache_transpatch_t);
         }
      SD_PreCacheSound(SD_GLASSBREAKSND);
		}
   if (sidepic==true)
      {
      PreCacheLump(lastmaskobj->sidepic,PU_CACHEWALLS,cache_pic_t);
      }
   if (lastmaskobj->bottomtexture>=0)
      PreCacheLump(lastmaskobj->bottomtexture,PU_CACHEWALLS,cache_transpatch_t);
   if (lastmaskobj->toptexture>=0)
      PreCacheLump(lastmaskobj->toptexture,PU_CACHEWALLS,cache_patch_t);
   if (lastmaskobj->midtexture>=0)
      PreCacheLump(lastmaskobj->midtexture,PU_CACHEWALLS,cache_patch_t);
	maskednum++;
	lastmaskobj++;
	if (maskednum==MAXMASKED)
		Error ("Too many masked walls\n");
}

/*
===============
=
= FixMaskedWallAreaNumbers
=
===============
*/
void FixMaskedWallAreaNumbers ( void )
{
	int i;
	int up,dn,lt,rt;
	int tilex,tiley;

	for (i=0;i<maskednum;i++)
		{
		int tile;
		tilex=maskobjlist[i]->tilex;
		tiley=maskobjlist[i]->tiley;
      tile=MAPSPOT(tilex,tiley,0)-AREATILE;
		if ((tile<=NUMAREAS) && (tile>0))
		   {
         maskobjlist[i]->areanumber = tile;
			continue;
		   }
		up=MAPSPOT(tilex,tiley-1,0)-AREATILE;
		dn=MAPSPOT(tilex,tiley+1,0)-AREATILE;
		lt=MAPSPOT(tilex-1,tiley,0)-AREATILE;
		rt=MAPSPOT(tilex+1,tiley,0)-AREATILE;



		up = ((up>0) && (up<=NUMAREAS));
		dn = ((dn>0) && (dn<=NUMAREAS));
		lt = ((lt>0) && (lt<=NUMAREAS));
		rt = ((rt>0) && (rt<=NUMAREAS));

		if (maskobjlist[i]->vertical==true)
			{
			if (rt)
				MAPSPOT(tilex,tiley,0) = MAPSPOT(tilex+1,tiley,0);
			else if (lt)
				MAPSPOT(tilex,tiley,0) = MAPSPOT(tilex-1,tiley,0);
			else if (up)
				MAPSPOT(tilex,tiley,0) = MAPSPOT(tilex,tiley-1,0);
			else if (dn)
				MAPSPOT(tilex,tiley,0) = MAPSPOT(tilex,tiley+1,0);
			else
				Error("FixMaskedWalls: Couldn't fix up area at x=%d y=%d\n",tilex,tiley);
			}
		else
			{
			if (dn)
				MAPSPOT(tilex,tiley,0) = MAPSPOT(tilex,tiley+1,0);
			else if (up)
				MAPSPOT(tilex,tiley,0) = MAPSPOT(tilex,tiley-1,0);
			else if (rt)
				MAPSPOT(tilex,tiley,0) = MAPSPOT(tilex+1,tiley,0);
			else if (lt)
				MAPSPOT(tilex,tiley,0) = MAPSPOT(tilex-1,tiley,0);
			else
				Error("FixMaskedWalls: Couldn't fix up area at x=%d y=%d\n",tilex,tiley);
			}
		maskobjlist[i]->areanumber = MAPSPOT(tilex,tiley,0)-AREATILE;
		if ((maskobjlist[i]->areanumber <0) || (maskobjlist[i]->areanumber > NUMAREAS))
		  Error("Bad masked wall areanumber of %d",maskobjlist[i]->areanumber);
		}
}


/*
===============
=
= CheckMaskedWall
=
===============
*/

int CheckMaskedWall( maskedwallobj_t * mw )
{
	int result;

	result=0;
	if (mw->flags & MW_SHOOTABLE)
		{
		if (mw->flags & MW_BLOCKINGCHANGES)
         {
         mw->flags&=~MW_BLOCKINGCHANGES;
         mw->flags&=~MW_BLOCKING;
         mw->flags|=MW_BOTTOMPASSABLE;
         }
      mw->flags&=~MW_SHOOTABLE;
//      mw->bottomtexture++;
      result=1;
      }
   return result;
}


/*
===============
=
= UpdateMaskedWall
=
===============
*/

int UpdateMaskedWall (int num)
{
   maskedwallobj_t * mw;
   int result;


   mw=maskobjlist[num];
   result=CheckMaskedWall(mw);
   if (result==1)
      {
      SpawnAnimatedMaskedWall(num);
      if (loadedgame==false)
	      SD_PlaySoundRTP(SD_GLASSBREAKSND,mw->tilex<<16,mw->tiley<<16);
      if (mw->flags&MW_MULTI)
         {
         int i;
         int dx,dy;
         int r;
         maskedwallobj_t * mw2;

         dx=0;dy=0;
         if (mw->vertical==true)
            dy=1;
         else
            dx=1;
         i=1;
			while (M_ISMWALL(mw->tilex+(dx*i),mw->tiley+(dy*i)))
            {
            int num;

            num=tilemap[mw->tilex+(dx*i)][mw->tiley+(dy*i)]&0x3ff;
            mw2=maskobjlist[num];
            if (!(mw2->flags&MW_MULTI))
               break;
            r=CheckMaskedWall(mw2);
            if (r==1)
               {
               SpawnAnimatedMaskedWall(num);
               if (loadedgame==false)
	               SD_PlaySoundRTP(SD_GLASSBREAKSND,mw2->tilex<<16,mw2->tiley<<16);
               }
            i++;
            }
         i=1;
			while (M_ISMWALL(mw->tilex-(dx*i),mw->tiley-(dy*i)))
            {
            int num;

            num=tilemap[mw->tilex-(dx*i)][mw->tiley-(dy*i)]&0x3ff;
            mw2=maskobjlist[num];
            if (!(mw2->flags&MW_MULTI))
               break;
            r=CheckMaskedWall(mw2);
            if (r==1)
               {
               SpawnAnimatedMaskedWall(num);
               if (loadedgame==false)
	               SD_PlaySoundRTP(SD_GLASSBREAKSND,mw2->tilex<<16,mw2->tiley<<16);
               }
            i++;
            }
         }
      }
   return result;
}



/*
============================
=
= ExecuteElevatorStopActions
=
============================
*/


void ExecuteElevatorStopActions(elevator_t *eptr, int teleport_location,
                                int desttilex,int desttiley)
   {
   eptr->state = ev_doorclosing;
   eptr->doorclosing = eptr->doortoopen;
   doorobjlist[eptr->doortoopen]->flags &= ~DF_ELEVLOCKED;
   OpenDoor(eptr->doortoopen);
   SD_PlaySoundRTP(SD_ELEVATORENDSND,desttilex,desttiley);
   Teleport(eptr,teleport_location);
   eptr->ticcount = OPENTICS;
   eptr->doortoopen = -1;
   if (MISCVARS->elevatormusicon == true)
      {
      MU_StartSong(song_level);
      MU_RestoreSongPosition();
      MISCVARS->elevatormusicon = false;

      }
   }


boolean PlayerInElevator(elevator_t *eptr)
   {
   if (eptr->state == ev_mts)
      {
      if ((eptr->dx == player->tilex) && (eptr->dy == player->tiley))
         return true;
      }
   else if (eptr->state == ev_mtd)
      {
      if ((eptr->sx == player->tilex) && (eptr->sy == player->tiley))
         return true;
      }

   return false;

   }

#define SHOULD_START_ELEVATOR_MUSIC(eptr)                              \
        ((demoplayback == false) && (demorecord == false) &&           \
         (MusicStarted() == true) &&                                   \
         (!BATTLEMODE) && \
         (!(player->flags & FL_GODMODE)) &&\
         (GameRandomNumber("elevator music",0) < 25) && \
         (PlayerInElevator(eptr))                                      \
        )                                                              \


/*
==========================
=
= SetElevatorOperationTime
=
==========================
*/


void SetElevatorOperationTime(elevator_t*eptr)
   {
   if (SHOULD_START_ELEVATOR_MUSIC(eptr))
      {
      MU_StoreSongPosition();
      MU_StartSong(song_elevator);
      MISCVARS->elevatormusicon = true;
      eptr->ticcount = ELEVATORMUSICTIME;
      }

   else if (AREANUMBER(eptr->sx,eptr->sy) == AREANUMBER(eptr->dx,eptr->dy))
      eptr->ticcount = 70;
   else
      eptr->ticcount = 170;

   }


/*
=====================
=
= CheckElevatorStart
=
=====================
*/

void CheckElevatorStart (elevator_t*eptr)
   {
   doorobj_t *dptr = doorobjlist[eptr->doorclosing];

   if (dptr->action == dr_closed)
      {

      if (eptr->nextaction!=-1)
         {
         eptr->state = eptr->nextaction;
         eptr->nextaction = -1;
         switch (eptr->state)
            {
            case ev_mtd:
               eptr->doortoopen = eptr->door2;
               SD_PlaySoundRTP(SD_ELEVATORONSND,eptr->sx<<16,eptr->sy<<16);
               //eptr->doorclosing = eptr->door1;

               SetElevatorOperationTime(eptr);
               break;

            case ev_mts:
               eptr->doortoopen = eptr->door1;

               SD_PlaySoundRTP(SD_ELEVATORONSND,eptr->dx<<16,eptr->dy<<16);

               SetElevatorOperationTime(eptr);
               break;
            }
         }

      else if (eptr->doorclosing == eptr->door1)
         eptr->state = ev_ras;


      else if (eptr->doorclosing == eptr->door2)
         eptr->state = ev_rad;


      eptr->doorclosing = -1;
      }
   }


/*
=====================
=
= ProcessElevators
=
= Called from PlayLoop
=
=====================
*/

void ProcessElevators (void)
   {
	int		  ectr;
	elevator_t *eptr;

	for (ectr = 0 ; ectr < _numelevators ; ectr++)
      {
      eptr = &ELEVATOR[ectr];
		if (eptr->ticcount)
			eptr->ticcount --;
		else
         {
         switch (eptr->state)
            {
            /*
            case ev_ras:
               break;

            case ev_rad:
               break;
            */
            case ev_mts:
               ExecuteElevatorStopActions(eptr,0,(eptr->sx << 16),(eptr->sy << 16));
               break;

            case ev_mtd:
               ExecuteElevatorStopActions(eptr,1,(eptr->dx << 16),(eptr->dy << 16));
               break;

            case ev_doorclosing:
               CheckElevatorStart(eptr);
               break;
            }
         }
      }
   }



void Teleport(elevator_t*eptr,int destination)
{statobj_t*tstat;
 objtype*temp;
 int startx,starty,destx,desty;

 if (destination) // move to dest
	{startx = eptr->sx;
	 starty = eptr->sy;
	 destx = eptr->dx;
	 desty = eptr->dy;
    tilemap[eptr->esx][eptr->esy] = (elevatorstart + 5) | 0x2000;

	}
 else
	{startx = eptr->dx;
	 starty = eptr->dy;
	 destx = eptr->sx;
	 desty = eptr->sy;
	 tilemap[eptr->edx][eptr->edy] = (elevatorstart + 5) | 0x2000;
	}

 for(tstat=firstactivestat;tstat;tstat=tstat->nextactive)
	{if ((tstat->tilex == startx) && (tstat->tiley == starty))
		{
		 tstat->x += ((destx - tstat->tilex) << TILESHIFT);
		 tstat->y += ((desty - tstat->tiley) << TILESHIFT);
		 tstat->tilex = tstat->x >> TILESHIFT;
		 tstat->tiley = tstat->y >> TILESHIFT;
		 tstat->visspot = &spotvis[tstat->tilex][tstat->tiley];
		 if (sprites[startx][starty] == tstat)
			{sprites[startx][starty] = NULL;
			 sprites[destx][desty] = tstat;

			}

		}
	}

 for(temp=firstactive;temp;temp=temp->nextactive)
	{if ((temp->tilex == startx) && (temp->tiley == starty))
		{temp->x += ((destx - temp->tilex) << TILESHIFT);
		 temp->y += ((desty - temp->tiley) << TILESHIFT);
		 temp->tilex = temp->x >> TILESHIFT;
		 temp->tiley = temp->y >> TILESHIFT;
		 if (temp->obclass!=inertobj)
			 {
			 RemoveFromArea (temp);
			 temp->areanumber = AREANUMBER(temp->tilex,temp->tiley);
			 MakeLastInArea (temp);
			 }
		 if (temp == player)
			SHAKETICS = 10;
		}
	}



}



void OperateElevatorDoor(int dnum)
{
 elevator_t*eptr;
 doorobj_t *dptr,*door1,*door2;

 dptr = doorobjlist[dnum];
 eptr = &ELEVATOR[dptr->eindex];
 door1 = doorobjlist[eptr->door1];
 door2 = doorobjlist[eptr->door2];

 switch(eptr->state)
	{/*
	 case ev_mtd:               // if already on the way to request,
										 // ignore; else, put request in
		if (dnum == eptr->door1)
		  {eptr->nextaction = ev_mts;
			//eptr->doortoopen = eptr->door1;
         #if (DEVELOPMENT == 1)
         #if (ELEVATORTEST == 1)
			Debug("\nplayer at source requesting elev %d mtd",dptr->eindex);
			#endif
			#endif
		  }
		break;

	 case ev_mts:
		if (dnum == eptr->door2)
		  {eptr->nextaction = ev_mtd;
			//eptr->doortoopen = eptr->door2;
         #if (DEVELOPMENT == 1)
         #if (ELEVATORTEST == 1)
			Debug("\nplayer at dest requesting elev %d mts",dptr->eindex);
			#endif
			#endif
		  }
		break;
	 */
	 case ev_rad:                          // if ready at other place,
		if ((dnum == eptr->door1) && (eptr->nextaction != ev_mts))  // process request, lock doors,

			{
         #if (DEVELOPMENT == 1)
         #if (ELEVATORTEST == 1)
			Debug("\nplayer at source requesting elev %d rad",dptr->eindex);
			#endif
			#endif
												// start moving to current loc;
			 SetNextAction(eptr,0);		// if already there, do nothing

			}
		break;

	 case ev_ras:
		if ((dnum == eptr->door2) && (eptr->nextaction != ev_mtd))
			{
         #if (DEVELOPMENT == 1)
         #if (ELEVATORTEST == 1)
			 Debug("\nplayer at dest requesting elev %d ras",dptr->eindex);
			#endif
			#endif
			 SetNextAction(eptr,1);

			}
		break;

	 case ev_doorclosing:
		if (eptr->doorclosing == dnum)      // if opening door at current loc,
														// reset elev state to ready
		  {//if (eptr->door1 == dnum)
			// eptr->nextaction = ev_ras;
			//else
			 //eptr->nextaction = ev_rad;
		  }
		else                                //else prepare for movement
		  {if ((eptr->door1 == dnum) && (eptr->nextaction != ev_mts))
			  {
            #if ((DEVELOPMENT == 1))
            #if ((ELEVATORTEST == 1))
				Debug("\nplayer at source requesting elev %d dc",dptr->eindex);
				#endif
				#endif
				SetNextAction(eptr,0);

			  }
			else if ((eptr->door2 == dnum) && (eptr->nextaction != ev_mtd))
			  {
            #if ((DEVELOPMENT == 1))
            #if ((ELEVATORTEST == 1))
				Debug("\nplayer at dest requesting elev %d dc",dptr->eindex);
				#endif
				#endif
				SetNextAction(eptr,1);

			  }
		  }
	  break;


	 }

}


int SetNextAction(elevator_t*eptr,int action)
{int dn;

 if (action)
	{if (!DoorReadyToClose(eptr->door1))
	  return false;

	 eptr->nextaction = ev_mtd;
	 dn = eptr->door1;
	}
 else
	{if (!DoorReadyToClose(eptr->door2))
	  return false;

	 eptr->nextaction = ev_mts;
	 dn = eptr->door2;
	}
 eptr->state = ev_doorclosing;

 eptr->doorclosing = dn;
 #if (DEVELOPMENT == 1)
 #if (ELEVATORTEST == 1)
	Debug("\nCloseDoor %d",dn);
 #endif
 #endif
 if (doorobjlist[dn]->action != dr_closed)
  CloseDoor(dn);
 doorobjlist[dn]->flags |= DF_ELEVLOCKED;

 return true;
}


void OperateElevatorSwitch(objtype*ob,int elevnum,int checkx,int checky)
{elevator_t*eptr;
 doorobj_t *door1,*door2;

 eptr = &ELEVATOR[elevnum];

 if ((eptr->state == ev_mts) ||
	  (eptr->state == ev_mtd))
	{
    #if (DEVELOPMENT == 1)
    #if (ELEVATORTEST == 1)
	 Debug("\nobj %d tried to use elevator %d switch while in use",ob->obclass,elevnum);
	 #endif
	 #endif
	 return;
	}

 door1 = doorobjlist[eptr->door1];
 door2 = doorobjlist[eptr->door2];

 if ((abs(ob->tilex-door1->tilex)<=1) && //switch at source
	  (abs(ob->tiley-door1->tiley)<=1))
	{if (!SetNextAction(eptr,1)) // set next to dest
		return;
    #if (DEVELOPMENT == 1)
    #if (ELEVATORTEST == 1)
	 Debug("\nswitch at src %d flipped",elevnum);
	 #endif
	 #endif
	 eptr->ticcount = 0;
	}
 else //switch at dest
	{if (!SetNextAction(eptr,0)) // set next to src
		return;
    #if (DEVELOPMENT == 1)
    #if (ELEVATORTEST == 1)
	 Debug("\nswitch at dest %d flipped",elevnum);
	 #endif
	 #endif
	 eptr->ticcount = 0;
	}

 tilemap[checkx][checky] = (elevatorstart + 6) | 0x2000;
 SD_PlaySoundRTP(SD_TOUCHPLATESND,ob->x,ob->y);

}




/*
=====================
=
= MoveDoors
=
= Called from PlayLoop
=
=====================
*/

void MoveDoors (void)
{
	int		door;

	for (door = 0 ; door < doornum ; door++)
		switch (doorobjlist[door]->action)
		{
		case dr_open:
			DoorOpen (door);
			break;

		case dr_opening:
			DoorOpening(door);
         SD_PanRTP ( doorobjlist[door]->soundhandle, doorobjlist[door]->tilex<<16, doorobjlist[door]->tiley<<16 );
			break;

		case dr_closing:
			DoorClosing(door);
         SD_PanRTP ( doorobjlist[door]->soundhandle, doorobjlist[door]->tilex<<16, doorobjlist[door]->tiley<<16 );
			break;
		default:
		    ;
		}
}


//===========================================================
//
//                   PUSHWALLS
//
//===========================================================


/*
===============
=
= GetAreaNumber
=
===============
*/
int GetAreaNumber ( int tilex, int tiley, int dir )
{
   int up,dn,lt,rt;

   up=MAPSPOT(tilex,tiley-1,0)-AREATILE;
   dn=MAPSPOT(tilex,tiley+1,0)-AREATILE;
   lt=MAPSPOT(tilex-1,tiley,0)-AREATILE;
   rt=MAPSPOT(tilex+1,tiley,0)-AREATILE;
   if ((up<=0) || (up>NUMAREAS)) up=0;
   if ((dn<=0) || (dn>NUMAREAS)) dn=0;
   if ((lt<=0) || (lt>NUMAREAS)) lt=0;
   if ((rt<=0) || (rt>NUMAREAS)) rt=0;
	switch (dir)
		{
		case north:
			if (up)
				return up;
			else if (dn)
				return dn;
			break;
		case south:
			if (dn)
				return dn;
			else if (up)
				return up;
			break;
		case east:
			if (rt)
				return rt;
			else if (lt)
				return lt;
			break;
		case west:
			if (lt)
				return lt;
			else if (rt)
				return rt;
			break;
		}
	if (up)
		return up;
	else if (dn)
		return dn;
	else if (lt)
		return lt;
	else if (rt)
		return rt;
	else
		Error("Cannot find an area number for tile at x=%d y=%d\n",tilex,tiley);
	return -1;
}

/*
===============
=
= SpawnPushWall
=
===============
*/

void SpawnPushWall (int tilex, int tiley, int lock, int texture, int dir, int type)
{
   pwallobj_t * lastpwallobj;

	if (pwallnum==MAXPWALLS)
		Error ("MAXPWALLS on level!");

   pwallobjlist[pwallnum]=(pwallobj_t*)Z_LevelMalloc(sizeof(pwallobj_t),PU_LEVELSTRUCT,NULL);
   memset(pwallobjlist[pwallnum],0,sizeof(pwallobj_t));
   lastpwallobj=pwallobjlist[pwallnum];

	lastpwallobj->x = (tilex<<16)+0x8000;
	lastpwallobj->y = (tiley<<16)+0x8000;
   lastpwallobj->momentumx=0;
   lastpwallobj->momentumy=0;
   lastpwallobj->tilex = tilex;
   lastpwallobj->tiley = tiley;
	lastpwallobj->lock = lock;
	lastpwallobj->action = pw_npushed;
	lastpwallobj->which = PWALL;
	lastpwallobj->dir = dir;
   lastpwallobj->num = pwallnum;
	actorat[tilex][tiley] = (pwallobj_t*)(lastpwallobj);	// consider it a solid wall

   if ( (MAPSPOT(tilex,tiley,0)==44) ||
        (MAPSPOT(tilex,tiley,0)==233)
      )
      lastpwallobj->flags=PW_DAMAGE;

	lastpwallobj->texture = texture;
   if (!texture&0x1000)
	   PreCacheLump(texture,PU_CACHEWALLS,cache_pic_t);
   lastpwallobj->areanumber = GetAreaNumber(tilex,tiley,lastpwallobj->dir);

   MAPSPOT (tilex, tiley, 0)=(word)(lastpwallobj->areanumber+AREATILE);

   switch(type)
      {
      case 0:
      case 1:
		case 3:
	      lastpwallobj->speed = 2;
         break;
      case 2:
      case 4:
	      lastpwallobj->speed = 4;
         break;
      default:
         Error("Illegal PushWall type passed into SpawnPushWall\n");
         break;
      }
   if (type>2)
      {
      tilemap[tilex][tiley] = 0;
	   ActivateMoveWall(pwallnum);
      }
   else
      {
      tilemap[tilex][tiley] = texture|0x800;
      if ((loadedgame==false) && (type==0))
         gamestate.secrettotal++;
      }

	pwallnum++;
	lastpwallobj++;

	SD_PreCacheSoundGroup(SD_PUSHWALLSND,SD_TURBOWALLSND);
}



/*
=====================
=
= OperatePushWall
=
= The player wants to change the pushwall's direction
=
=====================
*/
void OperatePushWall (int pwall, int dir, boolean localplayer )
{
   pwallobj_t * pw;

   pw=pwallobjlist[pwall];

   if (pw->lock)
		{
      if ( localplayer )
         {
         // Can't push
         AddMessage("This push wall appears to be locked...",MSG_DOOR);
         PlayNoWaySound();
         }
		return;
		}
	switch (pw->action)
	   {
	   case pw_npushed:
         if ((dir!=pw->dir) && (pw->dir!=nodir))
            {
            // Can't push
            if ( localplayer )
               {
               PlayNoWaySound();
               }
            return;
            }
         else if (localplayer && (gamestate.difficulty == gd_baby))
            AddMessage("Push Wall Activated.",MSG_GAME);

         pw->action=pw_pushing;
         pw->dir=dir;
         SD_PlaySoundRTP ( SD_TOUCHPLATESND, pw->x, pw->y );
         ConnectPushWall(pwall);
         SetupPushWall(pwall);
   	   gamestate.secretcount++;
		   break;
	   default:
         // Can't push
         if ( localplayer )
            {
            PlayNoWaySound();
            }
		   break;
	   }
}

/*
=====================
=
= ActivateAllPushWalls
=
= A Push wall has beeen activated by a touch plate
=
=====================
*/

void ActivateAllPushWalls(void)
   {
   int i;

   for(i=0;i<pwallnum;i++)
      {
      if (pwallobjlist[i]->dir != nodir)
         {
         ActivatePushWall(i);
         }
      }
   }

/*
=====================
=
= ActivatePushWall
=
= A Push wall has beeen activated by a touch plate
=
=====================
*/

void ActivatePushWall (long pwall)
{
   pwallobj_t * pw;

   pw=pwallobjlist[pwall];

	switch (pw->action)
	{
	case pw_npushed:
      pw->action=pw_pushing;
      ConnectPushWall(pwall);
      SetupPushWall(pwall);
   	gamestate.secretcount++;
		break;
	default:
      // Can't push
      SD_Play( SD_BADTOUCHSND );
		break;
	}
}

/*
=====================
=
= ActivateMoveWall
=
= A Push wall has beeen activated by a touch plate
=
=====================
*/

void ActivateMoveWall (long pwall)
{
   pwallobj_t * pw;

   pw=pwallobjlist[pwall];

	switch (pw->action)
	{
	case pw_npushed:
      pw->action=pw_moving;
      SetupPushWall(pwall);
      break;
	default:
      SD_Play( SD_BADTOUCHSND );
		break;
	}
}



/*
===============
=
= ConnectPushWall
=
===============
*/

void ConnectPushWall (int pwall)
{
	int      checkx;
	int      checky;
	int		area1,area2;
	int		area3,area4;
	word  	*map;
	pwallobj_t * pw;

	pw=pwallobjlist[pwall];

	checkx=pw->tilex;
	checky=pw->tiley;
	tilemap[checkx][checky] = 0;
	map = &MAPSPOT (checkx, checky, 0);

	area1 =	*(map-mapwidth);
	area2 =	*(map+mapwidth);
	area3 =  *(map+1);
	area4 =  *(map-1);

	area1 -= AREATILE;
	area2 -= AREATILE;
	area3 -= AREATILE;
	area4 -= AREATILE;
	if (((area1>0) && (area1<NUMAREAS)) &&
		 ((area2>0) && (area2<NUMAREAS)))
		{
		areaconnect[area1][area2]++;
		areaconnect[area2][area1]++;

		if ((insetupgame==false) && (loadedgame==false))
			ConnectAreas ();
		}
	if (((area3>0) && (area3<NUMAREAS)) &&
		 ((area4>0) && (area4<NUMAREAS)))
		{
		areaconnect[area3][area4]++;
		areaconnect[area4][area3]++;

		if ((insetupgame==false) && (loadedgame==false))
			ConnectAreas ();
		}
}

/*
===============
=
= SetupPushWall
=
===============
*/

void SetupPushWall (int pwall)
{
	pwallobj_t * pw;
	int speed;

	pw=pwallobjlist[pwall];
	speed=pw->speed<<PUSHWALLSPEED;
	switch (pw->dir)
	{
	case north:
		pw->momentumx=0;
		pw->momentumy=-speed;
		break;

	case east:
		pw->momentumx=speed;
		pw->momentumy=0;
		break;

	case northeast:
		pw->momentumx=speed;
		pw->momentumy=-speed;
		break;

	case southeast:
		pw->momentumx=speed;
		pw->momentumy=speed;
		break;

	case south:
		pw->momentumx=0;
		pw->momentumy=speed;
		break;

	case west:
		pw->momentumx=-speed;
		pw->momentumy=0;
		break;

	case northwest:
		pw->momentumx=-speed;
		pw->momentumy=-speed;
		break;

	case southwest:
		pw->momentumx=-speed;
		pw->momentumy=speed;
		break;
	}
	if (pw->action==pw_pushing)
		{
      if (loadedgame==false)
         pw->soundhandle=SD_PlaySoundRTP ( SD_PUSHWALLSND, pw->x, pw->y );
      pw->state=(0x20000L/speed);
		}
	if (pw->action==pw_moving)
      pw->state=(0x10000L/speed);
}


/*
=====================
=
= MovePWalls
=
= Called from PlayLoop
=
=====================
*/

void MovePWalls (void)
{
	int		pwall;

	for (pwall = 0 ; pwall < pwallnum ; pwall++)
		{
		if (pwallobjlist[pwall]->action==pw_pushing)
			{
			WallPushing (pwall);
			SD_PanRTP (pwallobjlist[pwall]->soundhandle, pwallobjlist[pwall]->x, pwallobjlist[pwall]->y );
			}
		if (pwallobjlist[pwall]->action==pw_moving)
			{
			WallMoving (pwall);
			SD_PanRTP (pwallobjlist[pwall]->soundhandle, pwallobjlist[pwall]->x, pwallobjlist[pwall]->y );
			}
		}
}


void ClearActorat(pwallobj_t*pw)
{int txhigh,txlow,tyhigh,tylow;
 int tryx,tryy,x,y;
 int pwrad = 0x6fff;


 tryx = pw->x;
 tryy = pw->y;
 txlow = (tryx - pwrad) >> 16;
 txhigh = (tryx + pwrad) >> 16;
 tylow = (tryy - pwrad) >> 16;
 tyhigh = (tryy + pwrad) >> 16;
 for(y=tylow;y<=tyhigh;y++)
	 for(x=txlow;x<=txhigh;x++)
		{if (actorat[x][y] == pw)
			actorat[x][y] = NULL;
		}
}

void SetActorat(pwallobj_t*pw)
{int txhigh,txlow,tyhigh,tylow;
 int tryx,tryy,x,y;
 int pwrad = 0x6fff;

 tryx = pw->x;
 tryy = pw->y;
 txlow = (tryx - pwrad) >> 16;
 txhigh = (tryx + pwrad) >> 16;
 tylow = (tryy - pwrad) >> 16;
 tyhigh = (tryy + pwrad) >> 16;

 for(y=tylow;y<=tyhigh;y++)
	 for(x=txlow;x<=txhigh;x++)
		actorat[x][y] = pw;
}

/*
=================
=
= FinishPushWall
=
=================
*/
void FinishPushWall (pwallobj_t * pw)
{
   pw->action = pw_pushed;
   actorat[pw->tilex][pw->tiley] = (wall_t*)&walls[GetWallIndex(pw->texture)];
   tilemap[pw->tilex][pw->tiley] = pw->texture;
}

/*
=================
=
= ResetPushWall
=
=================
*/
void ResetPushWall (pwallobj_t * pw)
{
   SetActorat(pw);
   tilemap[pw->tilex][pw->tiley] = pw->texture|0x800;
}

/*
=================
=
= WallPushing
=
=================
*/
void WallPushing (int pwall)
{
	int      checkx,checky;
	int      spot;
	pwallobj_t * pw;

	pw=pwallobjlist[pwall];

   ClearActorat(pw);

	PushWallMove(pwall);
	pw->x+=pw->momentumx;
	pw->y+=pw->momentumy;

	pw->state--;

	checkx=pw->tilex;
	checky=pw->tiley;

	pw->tilex=pw->x>>16;
	pw->tiley=pw->y>>16;

	if ((pw->tilex!=checkx) || (pw->tiley!=checky))
		{
		int x,y;
		int area = MAPSPOT(pw->tilex,pw->tiley,0)-AREATILE;

		if ((area<=0) || (area>NUMAREAS))
			{
			area=pw->areanumber;
			MAPSPOT (pw->tilex, pw->tiley, 0)=(word)(pw->areanumber+AREATILE);
			}
		// block crossed into a new block
		//
		// the tile can now be walked into
		//
		mapseen[checkx][checky] = 0;
		pw->areanumber = area;
		if (pw->momentumx>0)
			x=1;
		else if (pw->momentumx<0)
			x=-1;
		else
			x=0;
		if (pw->momentumy>0)
			y=1;
		else if (pw->momentumy<0)
			y=-1;
		else
			y=0;
		if (tilemap[pw->tilex+x][pw->tiley+y])
			{
			pw->state=(0x8000L/(pw->speed<<PUSHWALLSPEED));
			}
		if (actorat[pw->tilex+x][pw->tiley+y])
		  ResolveDoorSpace(pw->tilex+x,pw->tiley+y);
		}


	if (pw->state==0)
		{
		pw->x=(pw->tilex<<16)+0x8000;
		pw->y=(pw->tiley<<16)+0x8000;
		spot = MAPSPOT(pw->tilex,pw->tiley,1)-ICONARROWS;
		if ((spot >= 0) && (spot <= 7))
			{
         pw->action = pw_npushed;
			pw->dir = spot;
         ResetPushWall (pw);
         if (pw->lock)
				{
            pw->action=pw_pushing;
				ConnectPushWall(pwall);
				SetupPushWall(pwall);
				}
         else
            {
            gamestate.secrettotal++;
            }
         }
		else
			{
         FinishPushWall (pw);
			}
		}
   else
      SetActorat(pw);
}


/*
=================
=
= WallMoving
=
=================
*/
void WallMoving (int pwall)
{
	int      checkx,checky;
	int      spot;
	pwallobj_t * pw;

	pw=pwallobjlist[pwall];

   ClearActorat(pw);

	PushWallMove(pwall);
	pw->x+=pw->momentumx;
	pw->y+=pw->momentumy;

	pw->state--;

	checkx=pw->tilex;
	checky=pw->tiley;

	pw->tilex=pw->x>>16;
	pw->tiley=pw->y>>16;

	if ((pw->tilex!=checkx) || (pw->tiley!=checky))
		{
		int area = MAPSPOT(pw->tilex,pw->tiley,0)-AREATILE;

		if ((area<=0) || (area>NUMAREAS))
			{
			area=pw->areanumber;
			MAPSPOT (pw->tilex, pw->tiley, 0)=(word)(pw->areanumber+AREATILE);
			}
		// block crossed into a new block
		//
		// the tile can now be walked into
		//
		if (areabyplayer[area])
			{
			if (pw->speed==2)
				pw->soundhandle=SD_PlaySoundRTP ( SD_GOWALLSND, pw->x, pw->y );
			else
				pw->soundhandle=SD_PlaySoundRTP ( SD_TURBOWALLSND, pw->x, pw->y );
			}

		if (actorat[pw->tilex][pw->tilex])
		  ResolveDoorSpace(pw->tilex,pw->tiley);
		mapseen[checkx][checky] = 0;
		pw->areanumber = MAPSPOT (pw->tilex, pw->tiley, 0)-AREATILE;
		//actorat[pw->tilex][pw->tiley]=pw;
		if ( (pw->tilex==0) || (pw->tilex==127) ||
			  (pw->tiley==0) || (pw->tiley==127) )
			{
         if (W_CheckNumForName("imfree")>=0)
            {
            lbm_t *LBM;

            LBM = (lbm_t *) W_CacheLumpNum (W_GetNumForName ("imfree"), PU_CACHE, Cvt_lbm_t, 1);
            VL_DecompressLBM (LBM,true);
            VW_UpdateScreen ();
            I_Delay (2000);
            }
			Error ("PushWall Attempting to escape off the edge of the map\nIt is located at x=%d y=%d\nI'm Free!!!!\n",
					  pw->tilex, pw->tiley);
			}
		}
	if (pw->state==0)
		{
		pw->x=(pw->tilex<<16)+0x8000;
		pw->y=(pw->tiley<<16)+0x8000;
		spot = MAPSPOT(pw->tilex,pw->tiley,1)-ICONARROWS;
		if ((spot >= 0) && (spot <= 7))
			{
			int area = MAPSPOT(pw->tilex,pw->tiley,0)-AREATILE;

			if ((area<=0) || (area>NUMAREAS))
				{
				area=pw->areanumber;
				MAPSPOT (pw->tilex, pw->tiley, 0)=(word)(pw->areanumber+AREATILE);
				}

			if (areabyplayer[area] && (abs(spot-pw->dir)==4))
				SD_PlaySoundRTP ( SD_PUSHWALLHITSND, pw->x, pw->y );
			pw->dir = spot;
			}
		SetupPushWall(pwall);
		}
   else
      SetActorat(pw);
}



/*
=================
=
= SavePushWalls
=
=================
*/
void SavePushWalls(byte ** buf, int * sz)
{
  int unitsize;
  pwallobj_t * pw;
  byte * bufptr;
  int i;
  int size;

  if (pwallnum==0)
	  {
	  *sz=0;
	  *buf=SafeMalloc(16);
	  return;
	  }
  pw=pwallobjlist[0];
  unitsize=0;
  unitsize+=sizeof(pw->state);
  unitsize+=sizeof(pw->x);
  unitsize+=sizeof(pw->y);
  unitsize+=sizeof(pw->dir);
  unitsize+=sizeof(pw->speed);
  unitsize+=sizeof(pw->action);

  *sz=pwallnum*unitsize;

  *buf=SafeMalloc(*sz);
  bufptr=*buf;

  for (i=0;i<pwallnum;i++)
     {
     pw=pwallobjlist[i];
     size=sizeof(pw->state);
     memcpy(bufptr,&(pw->state),size);
     bufptr+=size;

     size=sizeof(pw->x);
     memcpy(bufptr,&(pw->x),size);
     bufptr+=size;

     size=sizeof(pw->y);
     memcpy(bufptr,&(pw->y),size);
     bufptr+=size;

     size=sizeof(pw->dir);
     memcpy(bufptr,&(pw->dir),size);
     bufptr+=size;

     size=sizeof(pw->speed);
     memcpy(bufptr,&(pw->speed),size);
     bufptr+=size;

     size=sizeof(pw->action);
     memcpy(bufptr,&(pw->action),size);
     bufptr+=size;
     }
}

/*
=================
=
= LoadPushWalls
=
=================
*/
void LoadPushWalls(byte * bufptr, int sz)
{
  int unitsize;
  pwallobj_t * pw;
  pwallobj_t new;
  int i;
  int num;
  int size;
  int area;

  if (sz==0)
     return;
  SetupPushWalls();
  pw=pwallobjlist[0];
  unitsize=0;
  unitsize+=sizeof(pw->state);
  unitsize+=sizeof(pw->x);
  unitsize+=sizeof(pw->y);
  unitsize+=sizeof(pw->dir);
  unitsize+=sizeof(pw->speed);
  unitsize+=sizeof(pw->action);

  num=sz/unitsize;
  if (pwallnum!=num)
     Error("Different number of Push Walls when trying to load a game\npwallnum=%d num=%d",pwallnum,num);

  for (i=0;i<pwallnum;i++)
     {
     pw=pwallobjlist[i];

     size=sizeof(new.state);
     memcpy(&(new.state),bufptr,size);
     bufptr+=size;

     size=sizeof(new.x);
     memcpy(&(new.x),bufptr,size);
     bufptr+=size;

     size=sizeof(new.y);
     memcpy(&(new.y),bufptr,size);
     bufptr+=size;

     size=sizeof(new.dir);
     memcpy(&(new.dir),bufptr,size);
     bufptr+=size;

     size=sizeof(new.speed);
     memcpy(&(new.speed),bufptr,size);
     bufptr+=size;

     size=sizeof(new.action);
     memcpy(&(new.action),bufptr,size);
     bufptr+=size;

	  actorat[pw->tilex][pw->tiley] = 0;
     mapseen[pw->tilex][pw->tiley] = 0;

     new.tilex=new.x>>16;
     new.tiley=new.y>>16;

     if ((new.tilex!=pw->tilex) || (new.tiley!=pw->tiley))
        {
        ClearActorat(pw);
        tilemap[pw->tilex][pw->tiley] = 0;
        if (pw->state!=pw_moving)
           {
#if 0
           if (pw->dir==nodir)
              {
              if (tilemap[pw->tilex+1][pw->tiley]==0)
                 pw->dir=east;
              else if (tilemap[pw->tilex-1][pw->tiley]==0)
                 pw->dir=west;
              else if (tilemap[pw->tilex][pw->tiley+1]==0)
                 pw->dir=south;
              else
                 pw->dir=north;
              }
#endif
           ConnectPushWall(i);
           }
        }

//   fixup area if needed

     area = MAPSPOT(new.tilex,new.tiley,0)-AREATILE;
     if ((area<=0) || (area>NUMAREAS))
        {
        MAPSPOT (new.tilex, new.tiley, 0)=(word)(pw->areanumber+AREATILE);
        }

     pw->tilex=new.tilex;
     pw->tiley=new.tiley;
     pw->x=new.x;
     pw->y=new.y;
     pw->action=new.action;
     pw->dir=new.dir;
     pw->speed=new.speed;
     SetupPushWall(i);
     pw->state=new.state;

     pw->areanumber = MAPSPOT (pw->tilex, pw->tiley, 0)-AREATILE;

     if (pw->action==pw_pushed)
        {
        FinishPushWall (pw);
        }
     else if (pw->action==pw_npushed)
        {
        ResetPushWall (pw);
        }
     else
        {
        SetActorat(pw);
        }
     }
}





/*
=================
=
= SaveMaskedWalls
=
=================
*/
void SaveMaskedWalls(byte ** buf, int * size)
{
  int unitsize;
  maskedwallobj_t * mw;
  byte * bufptr;
  int i;
  int sz;

  if (maskednum==0)
     {
     *size=0;
     *buf=SafeMalloc(16);
     return;
     }
  mw=maskobjlist[0];
  unitsize=0;
  unitsize+=sizeof(mw->flags);

  *size=maskednum*unitsize;

  *buf=SafeMalloc(*size);
  bufptr=*buf;

  for (i=0;i<maskednum;i++)
     {
     mw=maskobjlist[i];
     sz=sizeof(mw->flags);
     memcpy(bufptr,&(mw->flags),sz);
     bufptr+=sz;
     }
}

/*
=================
=
= LoadMaskedWalls
=
=================
*/
void LoadMaskedWalls(byte * bufptr, int sz)
{
  int unitsize;
  maskedwallobj_t * mw;
  int i;
  int size;
  int num;

  if (sz==0)
     return;

  SetupMaskedWalls();
  FixMaskedWallAreaNumbers();

  mw=maskobjlist[0];
  unitsize=0;
  unitsize+=sizeof(mw->flags);

  num=sz/unitsize;
  if (maskednum!=num)
     Error("Different number of Masked Walls when trying to load a game\nmaskednum=%d num=%d",maskednum,num);

  for (i=0;i<maskednum;i++)
     {
     word flags;	// Endianness fix thanks to DrLex - DDOI

     mw=maskobjlist[i];
     size=sizeof(mw->flags);
     memcpy(&flags,bufptr,size);
     bufptr+=size;
     if ((flags&0xff)!=mw->flags)	// Preserves original behavior
        UpdateMaskedWall(i);
     if (mw->flags&MW_SWITCHON)
        mw->toptexture--;
     }
}


/*
=================
=
= SaveDoors
=
=================
*/

void SaveDoors (byte ** buf, int * size)
{
   int door;
   int doorsave;
   byte doorflag;
   byte doorlocked;
   signed char dooreindex;
   short int doortime;
   int unitsize;
   byte *ptr;

   if (doornum==0)
      {
      *size=0;
      *buf=SafeMalloc(16);
      return;
      }

   //
   // Size = (int + byte + byte) * numdoors
   //

   unitsize=0;
   unitsize+=sizeof(doorsave);
   unitsize+=sizeof(doorflag);
   unitsize+=sizeof(doorlocked);
   unitsize+=sizeof(doortime);
   unitsize+=sizeof(dooreindex);

   *size = unitsize*doornum;
   *buf = (byte *) SafeMalloc (*size);

   ptr = *buf;

	for (door = 0; door < doornum ; door++)
   {
      doorsave   = doorobjlist[door]->position & ~3;
      doorsave  |= doorobjlist[door]->action;
      doorflag   = doorobjlist[door]->flags;
      doorlocked = doorobjlist[door]->lock;
      doortime   = doorobjlist[door]->ticcount;
      dooreindex = doorobjlist[door]->eindex;

      memcpy (ptr, &doorsave, sizeof (doorsave));
      ptr += sizeof (doorsave);
      memcpy (ptr, &doorflag, sizeof (doorflag));
      ptr += sizeof (doorflag);
      memcpy (ptr, &doorlocked, sizeof (doorlocked));
      ptr += sizeof (doorlocked);
      memcpy (ptr, &doortime, sizeof (doortime));
      ptr += sizeof (doortime);
      memcpy (ptr, &dooreindex, sizeof (dooreindex));
      ptr += sizeof (dooreindex);
   }
}


/*
=================
=
= LoadDoors
=
=================
*/

void LoadDoors (byte * buf, int size)
{
   int door;
   int doorsave;
   byte doorflag;
   byte doorlocked;
   signed char dooreindex;
   short int doortime;
   byte *ptr;
   int unitsize;
   int num;

   SetupDoors ();
   FixDoorAreaNumbers();
   ptr  = buf;

   unitsize=0;
   unitsize+=sizeof(doorsave);
   unitsize+=sizeof(doorflag);
   unitsize+=sizeof(doorlocked);
   unitsize+=sizeof(doortime);
   unitsize+=sizeof(dooreindex);

   num=size/unitsize;
   if (doornum!=num)
      Error("Different number of Doors when trying to load a game\ndoornum=%d num=%d",doornum,num);

   for (door = 0; door < doornum; door++)
   {
      memcpy (&doorsave, ptr, sizeof (doorsave));
      ptr += sizeof (doorsave);
      memcpy (&doorflag, ptr, sizeof (doorflag));
      ptr += sizeof (doorflag);
      memcpy (&doorlocked, ptr, sizeof (doorlocked));
      ptr += sizeof (doorlocked);
      memcpy (&doortime, ptr, sizeof (doortime));
      ptr += sizeof (doortime);
      memcpy (&dooreindex, ptr, sizeof (dooreindex));
      ptr += sizeof (dooreindex);

      doorobjlist[door]->action    = doorsave & 3;

      // Update Areas

      if (doorobjlist[door]->action != dr_closed)
         DoorOpening(door);

      doorobjlist[door]->action    = doorsave & 3;
      doorobjlist[door]->position = doorsave;
      doorobjlist[door]->flags     = doorflag;
      doorobjlist[door]->lock      = doorlocked;
      doorobjlist[door]->ticcount  = doortime;
      doorobjlist[door]->eindex    = dooreindex;

      if (doorobjlist[door]->action == dr_open)
         doorobjlist[door]->position = 0xFFFF;

      else if (doorobjlist[door]->action == dr_closed)
         doorobjlist[door]->position = 0;

      if (
           (doorobjlist[door]->action == dr_closing) ||
           (doorobjlist[door]->action == dr_closed)
         )
         {
         actorat[doorobjlist[door]->tilex][doorobjlist[door]->tiley] = doorobjlist[door];
         }
      doorobjlist[door]->texture = doorobjlist[door]->basetexture +
                                  ((doorobjlist[door]->position+1)>>13);
   }
}


/*
=====================
=
= SaveElevators
=
=
=====================
*/

void SaveElevators(byte ** buffer,int *size)
{int i;
 byte * tptr;

 *size = _numelevators*sizeof(elevator_t);

 *buffer = (byte *)SafeMalloc(*size);
 tptr = *buffer;

 for(i=0;i<_numelevators;i++)
  {memcpy(tptr,&ELEVATOR[i],sizeof(elevator_t));
	tptr += sizeof(elevator_t);
  }
}


/*
=====================
=
= LoadElevators
=
=
=====================
*/

void LoadElevators(byte * buffer,int size)
{int i;

 _numelevators = size/sizeof(elevator_t);


 for(i=0;i<_numelevators;i++)
  {memcpy(&ELEVATOR[i],buffer,sizeof(elevator_t));
	buffer += sizeof(elevator_t);
  }
}

