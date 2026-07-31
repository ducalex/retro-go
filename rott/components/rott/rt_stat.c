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
#ifdef DOS
#include <malloc.h>
#include <dos.h>
#endif

#include <string.h>
#include "sprites.h"
#include <stdio.h>
#include <stdlib.h>
#include "rt_stat.h"
#include "z_zone.h"
#include "lumpy.h"
#include "rt_util.h"
#include "rt_draw.h"
#include "rt_ted.h"
#include "rt_door.h"
#include "rt_main.h"
#include "w_wad.h"
#include "rt_main.h"
#include "rt_rand.h"
#include "rt_menu.h"
#include "rt_sound.h"
#include "_rt_stat.h"
#include "rt_net.h"
#include "rt_view.h"
#include "isr.h"
//MED
#include "memcheck.h"



/*
=============================================================================

Global Variables                                                                                                                                 GLOBAL VARIABLES

=============================================================================
*/

statobj_t   *firstactivestat,*lastactivestat;
statobj_t   *firstemptystat,*lastemptystat;

#ifndef RETRO_GO
wall_t      switches[MAXSWITCHES],*lastswitch;
#else
wall_t      *lastswitch;
#endif
respawn_t   *firstrespawn,*lastrespawn;
#ifndef RETRO_GO
statobj_t       *FIRSTSTAT,*LASTSTAT;
#endif

#ifndef RETRO_GO
statobj_t	*sprites[MAPSIZE][MAPSIZE];
#endif
#ifndef RETRO_GO
animwall_t  animwalls[MAXANIMWALLS];

dirtype opposite[9] =
	{west,southwest,south,southeast,east,northeast,north,northwest,nodir};
#endif







statinfo stats[NUMSTATS] =
{
{0,SPR0_YLIGHT, stat_ylight,FL_LIGHT|FL_SHOOTABLE,0,0,2,0,0},
{0,SPR1_RLIGHT, stat_rlight,FL_LIGHT|FL_SHOOTABLE,0,0,2,0,0},
{0,SPR2_GLIGHT, stat_glight,FL_LIGHT|FL_SHOOTABLE,0,0,2,0,0},
{0,SPR3_BLIGHT, stat_blight,FL_LIGHT|FL_SHOOTABLE,0,0,2,0,0},
{0,SPR4_CHAND, stat_chandelier,FL_LIGHT|FL_SHOOTABLE,0,0,2,0,0},
{0,SPR5_LAMPOFF, stat_lamp,FL_LIGHT|FL_BLOCK|FL_SHOOTABLE,0,0,2,0,0},
{0,SPR73_GKEY1, stat_pedgoldkey,FL_COLORED|FL_BONUS|FL_CHANGES|FL_BLOCK|FL_ACTIVE,2,16,pc_orange,0,0},
{0,SPR73_GKEY1,stat_pedsilverkey,FL_COLORED|FL_BONUS|FL_CHANGES|FL_BLOCK|FL_ACTIVE,2,16,pc_gray,0,0},
{0,SPR73_GKEY1, stat_pedironkey,FL_COLORED|FL_BONUS|FL_CHANGES|FL_BLOCK|FL_ACTIVE,2,16,pc_olive,0,0},
{0,SPR73_GKEY1,stat_pedcrystalkey,FL_COLORED|FL_BONUS|FL_CHANGES|FL_BLOCK|FL_ACTIVE,2,16,pc_red,0,0},
{0,SPR6_GIBS1,  stat_gibs1,0,0,0,0,0,0},
{0,SPR7_GIBS2,  stat_gibs2,0,0,0,0,0,0},
{0,SPR8_GIBS3,  stat_gibs3,0,0,0,0,0,0},
{0,SPR9_MONKMEAL, stat_monkmeal,FL_BONUS|FL_RESPAWN,0,0,0,0,0},
{0,PORRIDGE1, stat_priestporridge,FL_BONUS|FL_RESPAWN,2,6,0,0,0},
{0,MONKCRYSTAL11,stat_monkcrystal1,FL_BONUS|FL_ACTIVE|FL_RESPAWN,2,6,0,0,0},
{0,MONKCRYSTAL21,stat_monkcrystal2,FL_BONUS|FL_ACTIVE|FL_RESPAWN,2,7,0,0,0},
{0,ONEUP01,stat_oneup,FL_BONUS|FL_ACTIVE|FL_FULLLIGHT,2,8,0,0,0},
{0,THREEUP01,stat_threeup,FL_BONUS|FL_ACTIVE|FL_FULLLIGHT,2,8,0,0,0},
{0,TORCH1,stat_altbrazier1,FL_HEAT|FL_BLOCK|FL_ACTIVE|FL_LIGHT|FL_SHOOTABLE,2,15,2,0,0},
{0,SPR_ABRAZIER2,stat_altbrazier2,FL_HEAT|FL_BLOCK|FL_LIGHT|FL_SHOOTABLE,0,0,2,0,0},
{0,FBASIN1,stat_healingbasin,FL_BONUS|FL_CHANGES|FL_ACTIVE|FL_BLOCK,2,3,0,0,0},
{20,EBASIN,stat_emptybasin,FL_BLOCK|FL_SHOOTABLE,0,0,0,0,0},
{0,BAT1,stat_bat,FL_BONUS|FL_ACTIVE|FL_RESPAWN|FL_WEAPON,1,16,0,0,10},
{0,KNIFE_STATUE1,stat_knifestatue,FL_BONUS|FL_CHANGES|FL_BLOCK,0,0,0,0,0},
{0,SPR_TWOPIST,stat_twopistol,FL_BONUS|FL_RESPAWN|FL_WEAPON,0,0,0,0,5},
{0,SPR_MP40,stat_mp40,FL_BONUS|FL_RESPAWN|FL_WEAPON,0,0,0,0,5},
{0,SPR_BAZOOKA,stat_bazooka,FL_BONUS|FL_RESPAWN|FL_WEAPON,0,0,0,0,10},
{0,SPR_FIREBOMB,stat_firebomb,FL_BONUS|FL_RESPAWN|FL_WEAPON,0,0,0,0,5},
{0,SPR_HEATSEEK,stat_heatseeker,FL_BONUS|FL_RESPAWN|FL_WEAPON,0,0,0,0,7},
{0,SPR_DRUNK,stat_drunkmissile,FL_BONUS|FL_RESPAWN|FL_WEAPON,0,0,0,0,7},
{0,SPR_FIREWALL,stat_firewall,FL_BONUS|FL_RESPAWN|FL_WEAPON,0,0,0,0,5},
{0,SPR_SPLIT,stat_splitmissile,FL_BONUS|FL_RESPAWN|FL_WEAPON,0,0,0,0,7},
{0,SPR_KES,stat_kes,FL_BONUS|FL_RESPAWN|FL_WEAPON,0,0,0,0,7},
{0,LIFEITEMA01,stat_lifeitem1,FL_BONUS|FL_ACTIVE|FL_SHOOTABLE|FL_FULLLIGHT,2,8, 2,0,0},
{0,LIFEITEMB01,stat_lifeitem2,FL_BONUS|FL_ACTIVE|FL_SHOOTABLE|FL_FULLLIGHT,2,8, 2,0,0},
{0,LIFEITEMD01,stat_lifeitem3,FL_BONUS|FL_ACTIVE|FL_SHOOTABLE|FL_FULLLIGHT,2,8, 2,0,0},
{0,LIFEITEMC01,stat_lifeitem4,FL_BONUS|FL_ACTIVE|FL_SHOOTABLE|FL_FULLLIGHT,2,15, 2,0,0},
{24,SPR32_EXPLOS,stat_tntcrate,FL_SHOOTABLE|FL_BLOCK|FL_WOODEN,0,0,3,10,0},
{12,SPR33_CBARREL,stat_bonusbarrel,FL_METALLIC|FL_SHOOTABLE|FL_BLOCK,0,0,3,10,0},
{0,TORCH1,stat_torch,FL_BLOCK|FL_LIGHT|FL_ACTIVE|FL_HEAT|FL_SHOOTABLE,2,15,2,0,0},
{30,FFLAME1,stat_floorfire,FL_HEAT|FL_BLOCK|FL_LIGHT|FL_ACTIVE|FL_SHOOTABLE,2,7,30,0,0},
{0,DIP11,stat_dipball1,FL_BONUS|FL_FULLLIGHT,0,0,0,0,0},
{0,DIP21,stat_dipball2,FL_BONUS|FL_FULLLIGHT,0,0,0,0,0},
{0,DIP31,stat_dipball3,FL_BONUS|FL_FULLLIGHT,0,0,0,0,0},
{0,SPR34_TOUCH1,stat_touch1,0|FL_TRANSLUCENT|FL_FADING,0,0,0,0,0},
{0,SPR35_TOUCH2,stat_touch2,0|FL_TRANSLUCENT|FL_FADING,0,0,0,0,0},
{0,SPR36_TOUCH3,stat_touch3,0|FL_TRANSLUCENT|FL_FADING,0,0,0,0,0},
{0,SPR37_TOUCH4,stat_touch4,0|FL_TRANSLUCENT|FL_FADING,0,0,0,0,0},
{20,SPR62_ETOUCH1,stat_dariantouch,FL_METALLIC|FL_BANDF|FL_SHOOTABLE|FL_BLOCK,10,3,50,0,0},
{0,SCOTHEAD1, stat_scotthead,FL_BONUS|FL_ACTIVE|FL_FULLLIGHT,4,7,0,0,0},
{0,SPR38_GARBAGE1,stat_garb1,0,0,0,0,0,0},
{0,SPR39_GARBAGE2,stat_garb2,0,0,0,0,0,0},
{0,SPR40_GARBAGE3,stat_garb3,0,0,0,0,0,0},
{0,SPR41_SHIT,stat_shit,0,0,0,0,0,0},
{0,SPR42_GRATE,stat_grate,0,0,0,0,0,0},
{0,SPR43_MSHARDS,stat_metalshards,0,0,0,0,0,0},
{20,SPR44_PEDESTAL,stat_emptypedestal,FL_BLOCK|FL_SHOOTABLE|FL_WOODEN,0,0,60,0,0},
{20,SPR45_ETABLE,stat_emptytable,FL_BLOCK|FL_SHOOTABLE|FL_WOODEN,0,0,100,0,0},
{16,SPR46_STOOL,stat_stool,FL_BLOCK|FL_SHOOTABLE|FL_WOODEN,0,0,25,0,0},
{0,SPR_PUSHCOLUMN1,stat_bcolumn,FL_BLOCK|FL_HEIGHTFLIPPABLE,0,0,0,0,0},
{0,SPR_PUSHCOLUMN1,stat_gcolumn,FL_BLOCK|FL_HEIGHTFLIPPABLE,0,0,0,0,0},
{0,SPR_PUSHCOLUMN1,stat_icolumn,FL_BLOCK|FL_HEIGHTFLIPPABLE,0,0,0,0,0},
{20,SPR50_TREE,stat_tree,FL_SHOOTABLE|FL_BLOCK,0,0,0,0,0},
{20,SPR51_PLANT,stat_plant,FL_SHOOTABLE|FL_BLOCK,0,0,0,0,0},
{20,BLUEVASE,stat_urn,FL_SHOOTABLE|FL_BLOCK,0,0,0,0,0},
{0,SPR54_HAY,stat_haystack,FL_SHOOTABLE|FL_BLOCK,0,0,20,0,0},
{12,SPR55_IBARREL,stat_ironbarrel,FL_METALLIC|FL_BLOCK|FL_SHOOTABLE,0,0,50,0,0},
{0,HGRATE1,stat_heatgrate,FL_LIGHT|FL_ACTIVE,2,4,0,5,0},
{-10,STNPOLE1,stat_standardpole,FL_SHOOTABLE|FL_BLOCK,0,0,25,0,0},
{0,PREPIT,stat_pit,FL_CHANGES,0,0,0,0,0},
{0,GODPOWERUP1,stat_godmode,FL_WEAPON|FL_BONUS|FL_ACTIVE|FL_RESPAWN|FL_FULLLIGHT,2,8,0,0,0},
{0,DOGPOWERUP1,stat_dogmode,FL_WEAPON|FL_BONUS|FL_ACTIVE|FL_RESPAWN|FL_FULLLIGHT,2,8,0,0,0},
{0,FLEETFEETPOWERUP1,stat_fleetfeet,FL_BONUS|FL_ACTIVE|FL_RESPAWN|FL_FULLLIGHT,2,8,0,0,0},
{0,ELASTICPOWERUP1, stat_elastic, FL_BONUS|FL_ACTIVE|FL_RESPAWN|FL_FULLLIGHT,2,8,0,0,0},
{0,MUSHROOMPOWERUP1, stat_mushroom, FL_BONUS|FL_ACTIVE|FL_RESPAWN|FL_FULLLIGHT,2,8,0,0,0},
{0,GASMASKPOWERUP, stat_gasmask, FL_BONUS|FL_RESPAWN,0,0,0,0,0},
{0,BULLETPROOFPOWERUP, stat_bulletproof, FL_BONUS|FL_RESPAWN,0,0,0,0,0},
{0,ASBESTOSPOWERUP, stat_asbesto, FL_BONUS|FL_RESPAWN,0,0,0,0,0},
{0,RANDOMPOWERUP1, stat_random, FL_BONUS|FL_ACTIVE|FL_RESPAWN,2,8,0,0,0},
{0,RUBBLE1, stat_rubble, FL_ACTIVE,2,10,0,0,0},
{0,WOODFRAG1, stat_woodfrag, FL_ACTIVE,2,14,0,0,0},
{0,ROBOGRDDIE1, stat_metalfrag, FL_ACTIVE,2,10,0,0,0},
{0,EMPTY_STATUE1,stat_emptystatue,FL_BLOCK|FL_SHOOTABLE,0,0,50,0,0},
{16,TOMLARVA1,stat_tomlarva,FL_ACTIVE|FL_SHOOTABLE|FL_BLOCK,2,4,150,0,0},
{0,BULLETHOLE,stat_bullethole,FL_TRANSLUCENT,0,0,0,0,0},
//MED
#if (SHAREWARE == 1)
{0,COLLECTOR1,stat_collector,FL_ACTIVE|FL_BONUS,2,8,-1,0,0},
#else
{0,DOPE1,stat_collector,FL_ACTIVE|FL_BONUS,2,8,-1,0,0},
#endif
{0,SPR_MINE1,stat_mine,FL_BONUS|FL_SHOOTABLE|FL_RESPAWN,0,0,10,0,0},
{0,MISSMOKE1,stat_missmoke,FL_ACTIVE,6,4,0,0,0},
{0,PLATFORM1,stat_disk,FL_BLOCK|FL_HEIGHTFLIPPABLE,0,0,0,0,0},
{-1,0,0,0,0,0,0,0,0}
};


#ifndef RETRO_GO
dirtype diagonal[9][9] =
{
/* east */  {nodir,nodir,northeast,nodir,nodir,nodir,southeast,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
/* north */ {northeast,nodir,nodir,nodir,northwest,nodir,nodir,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
/* west */  {nodir,nodir,northwest,nodir,nodir,nodir,southwest,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
/* south */ {southeast,nodir,nodir,nodir,southwest,nodir,nodir,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir}
};
#endif

/*
=============================================================================

Local Variables                                                                                                                                 GLOBAL VARIABLES

=============================================================================
*/
#ifndef RETRO_GO
static awallinfo_t animwallsinfo[MAXANIMWALLS] =
  {{3,4,"FPLACE1\0"}, //lava wall
   {3,6,"ANIMY1\0"}, //anim red
   {3,6,"ANIMR1\0"}, //anim yellow
   {40,4,"ANIMFAC1\0"}, //anim face
   {3,4,"ANIMONE1\0"}, //anim one
   {3,4,"ANIMTWO1\0"}, //anim two
   {3,4,"ANIMTHR1\0"}, //anim three
   {3,4,"ANIMFOR1\0"}, //anim four
   {3,6,"ANIMGW1\0"}, //anim grey water
   {3,6,"ANIMYOU1\0"}, //anim you do not belong
   {3,6,"ANIMBW1\0"}, //anim brown water
   {3,6,"ANIMBP1\0"}, //anim brown piston
   {3,6,"ANIMCHN1\0"}, //anim chain
   {3,6,"ANIMFW1\0"}, //anim firewall
   {3,6,"ANIMLAT1\0"}, //anim little blips
   {3,6,"ANIMST1\0"}, //anim light streams left
   {3,6,"ANIMRP1\0"}};//anim light streams right
#endif

int statcount;

void AddRespawnStatic(respawn_t*stat);
void DoLights (int tilex, int tiley);

void AddToFreeStaticList(statobj_t*stat)
{ if (!firstemptystat)
	 firstemptystat = stat;
  else
	 {stat->statprev = lastemptystat;
	  lastemptystat->statnext = stat;
	 }
  lastemptystat = stat;

}


void RemoveFromFreeStaticList(statobj_t*stat)
{
  if (stat == lastemptystat)
		lastemptystat = stat->statprev;
  else
		stat->statnext->statprev = stat->statprev;

  if (stat == firstemptystat)
		firstemptystat = stat->statnext;
  else
		stat->statprev->statnext = stat->statnext;

  stat->statprev = NULL;
  stat->statnext = NULL;

}




/*
===============
=
= MakeStatActive
=
===============
*/

void MakeStatActive(statobj_t*x)
 {if (!firstactivestat)
	 firstactivestat	= x;
  else
	  {x->prevactive = lastactivestat;
		lastactivestat->nextactive = x;
	  }
  lastactivestat = x;
 }


/*
===============
=
= MakeStatInactive
=
===============
*/

void MakeStatInactive(statobj_t*stat)
{
 if (stat == lastactivestat)
	lastactivestat = stat->prevactive;
 else
	stat->nextactive->prevactive = stat->prevactive;

 if (stat == firstactivestat)
	firstactivestat = stat->nextactive;
 else
	stat->prevactive->nextactive = stat->nextactive;

 stat->prevactive = NULL;
 stat->nextactive = NULL;

}
/*
===============
=
= AddStatic
=
===============
*/

void AddStatic(statobj_t *stat)
 {if (FIRSTSTAT)
	 {stat->statprev = LASTSTAT;
	  LASTSTAT->statnext = stat;
	 }
  else
	 FIRSTSTAT = stat;
  LASTSTAT = stat;
 }



void RemoveStatic(statobj_t*stat)
{
   if (stat->flags & FL_ABP) //remove from active list
      MakeStatInactive(stat);

   if (stat == LASTSTAT)     // remove from master list
      LASTSTAT = stat->statprev;
   else
      stat->statnext->statprev = stat->statprev;

   if (stat == FIRSTSTAT)
      FIRSTSTAT = stat->statnext;
   else
      stat->statprev->statnext = stat->statnext;

   stat->statprev = NULL;
   stat->statnext = NULL;

   if (stat->flags & FL_WEAPON)
      MISCVARS->NUMWEAPONS --;

   if ((stat->flags & FL_RESPAWN) &&
       gamestate.BattleOptions.RespawnItems &&
       (!((stat->flags & FL_WEAPON ) &&
          (gamestate.BattleOptions.WeaponPersistence)
         )
       )
      )
      {
      respawn_t*temp;
   //    if ( !( (stat->flags & FL_WEAPON) &&
   //          (MISCVARS->NUMWEAPONS >= (numplayers+10))
      //      )
      // )
         {
         temp = (respawn_t*)Z_LevelMalloc(sizeof(respawn_t),PU_LEVELSTRUCT,NULL);

         memset (temp,0,sizeof(*temp));
         temp->ticcount = GetRespawnTimeForItem(stat->itemnumber);
         temp->tilex = stat->tilex;
         temp->tiley = stat->tiley;
         temp->itemnumber = stat->itemnumber;
         temp->spawnz = stat->z;
         temp->linked_to = stat->linked_to;
         //SoftError("\nrespawn obj created for stattype %d with z = %d",stat->itemnumber,temp->spawnz);
         AddRespawnStatic(temp);
         }
      }
   //Z_Free(stat);
   AddToFreeStaticList(stat);

   //Add_To_Delete_Array(stat);
   statcount --;
   }


void RemoveRespawnStatic(respawn_t*stat)
{
	if (stat == lastrespawn)
		lastrespawn = stat->prev;
	else
		stat->next->prev = stat->prev;

	if (stat == firstrespawn)
		firstrespawn = stat->next;
	else
		stat->prev->next = stat->next;

	stat->prev = NULL;
	stat->next = NULL;
	Z_Free(stat);
}


void TurnOffLight(int tilex,int tiley)
{
  DoLights(tilex,tiley);
  LightsInArea[MAPSPOT(tilex,tiley,0)-AREATILE]--;

}


void ActivateLight(long light)
{statobj_t*tstat;

 tstat = (statobj_t*)light;

 tstat->shapenum ++;
 tstat->flags |= FL_LIGHTON;
 TurnOnLight(tstat->tilex,tstat->tiley);

}


void DeactivateLight(long light)
{statobj_t*tstat;

 tstat = (statobj_t*)light;

 tstat->shapenum --;
 tstat->flags &= ~(FL_LIGHTON);
 TurnOffLight(tstat->tilex,tstat->tiley);

}

void TurnOnLight(int i,int j)
{

 LightsInArea[MAPSPOT(i,j,0)-AREATILE]++;

 if (lightsource==0)
    return;

 if ((!(tilemap[i+1][j])) && (!(tilemap[i-1][j])))
	{
	SetLight(i,j,0xcdeffedc);
	SetLight(i+1,j,0x135789ab);
	SetLight(i-1,j,0xba987351);
	SetLight(i,j+1,0xcdeffedc);
	SetLight(i,j-1,0xcdeffedc);
	SetLight(i-1,j-1,0xba987351);
	SetLight(i+1,j+1,0xba987351);
	SetLight(i+1,j-1,0x135789ab);
	SetLight(i-1,j+1,0x135789ab);
	}
 else if ((!(tilemap[i][j+1])) && (!(tilemap[i][j-1])))
	{
	SetLight(i,j,0xcdeffedc);
	SetLight(i+1,j,0xcdeffedc);
	SetLight(i-1,j,0xcdeffedc);
	SetLight(i,j+1,0x135789ab);
	SetLight(i,j-1,0xba987531);
	SetLight(i-1,j-1,0x135789ab);
	SetLight(i+1,j-1,0xba987531);
	SetLight(i+1,j+1,0x135789ab);
	SetLight(i-1,j+1,0xba987531);
	}
					//  |
					//__|
 else if ((tilemap[i][j+1]) && (tilemap[i+1][j]))
	{
	SetLight(i,j,0xcdeffedc);
	SetLight(i+1,j,0xcdeffedc);
	SetLight(i-1,j,0xcdeffedc);
	SetLight(i,j+1,0xcdeffedc);
	SetLight(i,j-1,0xcdeffedc);
	SetLight(i-1,j-1,0xba987351);
	SetLight(i+1,j-1,0xba987351);
	SetLight(i+1,j+1,0xba987351);
	SetLight(i-1,j+1,0x135789ab);
	}
					//|
					//|_
 else if ((tilemap[i][j+1]) && (tilemap[i-1][j]))
	{
	SetLight(i,j,0xcdeffedc);
	SetLight(i+1,j,0xcdeffedc);
	SetLight(i-1,j,0xcdeffedc);
	SetLight(i,j+1,0xcdeffedc);
	SetLight(i,j-1,0xcdeffedc);
	SetLight(i-1,j-1,0x135789ab);
	SetLight(i+1,j-1,0xba987531);
	SetLight(i+1,j+1,0xba987531);
	SetLight(i-1,j+1,0xba987531);
	}
					//_
					// |
 else if ((tilemap[i][j-1]) && (tilemap[i+1][j]))
	{
	SetLight(i,j,0xcdeffedc);
	SetLight(i+1,j,0xcdeffedc);
	SetLight(i-1,j,0xcdeffedc);
	SetLight(i,j+1,0xcdeffedc);
	SetLight(i,j-1,0xcdeffedc);
	SetLight(i-1,j-1,0xba987531);
	SetLight(i+1,j-1,0x135789ab);
	SetLight(i+1,j+1,0x135789ab);
	SetLight(i-1,j+1,0xba987531);
	}
					//__
					//|
 else if ((tilemap[i][j-1]) && (tilemap[i-1][j]))
	{
	SetLight(i,j,0xcdeffedc);
	SetLight(i+1,j,0xcdeffedc);
	SetLight(i-1,j,0xcdeffedc);
	SetLight(i,j+1,0xcdeffedc);
	SetLight(i,j-1,0xcdeffedc);
	SetLight(i-1,j-1,0x135789ab);
	SetLight(i+1,j-1,0x135789ab);
	SetLight(i+1,j+1,0xba987531);
	SetLight(i-1,j+1,0xba987531);
	}
 else if (tilemap[i][j])
	{
	SetLight(i,j,0x58bffb85);
	}
 else
	{
	SetLight(i,j,0xcdeffedc);
	SetLight(i+1,j,0xba987654);
	SetLight(i-1,j,0x456789ab);
	SetLight(i,j+1,0xba987654);
	SetLight(i,j-1,0x456789ab);
	SetLight(i-1,j+1,0x33322211);
	SetLight(i+1,j+1,0x33322211);
	SetLight(i+1,j-1,0x11222333);
	SetLight(i-1,j-1,0x11222333);
	}


}




/*===============
=
= AddRespawnStatic
=
===============
*/

void AddRespawnStatic(respawn_t*stat)
 {
  if (firstrespawn)
	{stat->prev = lastrespawn;
	 lastrespawn->next = stat;
	}
  else
	firstrespawn = stat;
  lastrespawn = stat;
 }


/*
===============
=
= InitStaticList
=
===============
*/

void InitStaticList (void)
{
	FIRSTSTAT = NULL;
	LASTSTAT = NULL;
	lastactivestat = NULL;
	firstactivestat = NULL;
	firstrespawn = NULL;
	lastrespawn = NULL;
	lastemptystat = NULL;
	firstemptystat = NULL;

	memset(BulletHoles, 0, MAXBULLETS * sizeof(statobj_t *));

   MISCVARS->BulletHoleNum = 0;


	memset(sprites, 0, MAPSIZE * MAPSIZE * sizeof(statobj_t *));
	if (loadedgame==false)
		{
		memset(switches, 0, MAXSWITCHES * sizeof(wall_t));
		lastswitch = &switches[0];
		}
	statcount = 0;

}


/*
===============
=
= InitAnimatedWallList
=
===============
*/

void InitAnimatedWallList(void)
{
 int i;

 for (i=0;i<MAXANIMWALLS;i++)
	 animwalls[i].active=0;
}

/*
===============
=
= SetupAnimatedWall
=
===============
*/

void SetupAnimatedWall(int which)
{
   animwall_t * aw;
   int i;
   int texture;

   aw = &animwalls[which];

   aw->active=1;
   aw->ticcount=animwallsinfo[which].tictime;
   aw->count = 1;

   texture=W_GetNumForName(animwallsinfo[which].firstlump);

   aw->basetexture=texture;
   aw->texture=texture;

   if (DoPanicMapping()==true)
      {
      PreCacheLump(aw->basetexture,PU_CACHEWALLS,cache_pic_t);
      }
   else
      {
      for (i=aw->basetexture;i<aw->basetexture+animwallsinfo[which].numanims;i++)
         PreCacheLump(i,PU_CACHEWALLS,cache_pic_t);
      }
}


/*
===============
=
= SaveStatics
=
===============
*/

void SaveStatics (byte **buffer, int * size)
{statobj_t * temp;
 saved_stat_type dummy;
 byte * tptr;
 int count;

 if (statcount==0)
	 {
	 *size=0;
	 *buffer=SafeMalloc(16);
	 return;
	 }
 *size = statcount*sizeof(saved_stat_type);
 *buffer = (byte *)SafeMalloc(*size);
 tptr = *buffer;

 for(count=0,temp=FIRSTSTAT;temp;temp=temp->statnext)
  {dummy.x = temp->x;
	dummy.y = temp->y;
	dummy.z = temp->z;
	dummy.flags = temp->flags;
	dummy.ticcount = temp->ticcount;
	dummy.hitpoints = temp->hitpoints;
	dummy.shapenum = temp->shapenum;
	dummy.ammo = temp->ammo;
	dummy.count = temp->count;
	dummy.numanims = temp->numanims;
	dummy.itemnumber = temp->itemnumber;
	dummy.areanumber = temp->areanumber;
	temp->whichstat = count;
	dummy.whichstat = count++;
	dummy.linked_to = temp->linked_to;

	memcpy(tptr,&(dummy.x),sizeof(saved_stat_type));
	tptr += sizeof(saved_stat_type);

  }

}

/*
===============
=
= DoLights
=
===============
*/

void DoLights (int tilex, int tiley)
{
    if (lightsource==0)
       return;
	 if (TurnOffLight0 (tilex, tiley))
       LightSourceAt(tilex,tiley) = 0;

	 if (TurnOffLight1 (tilex, tiley, -1, -1))
       LightSourceAt(tilex-1,tiley-1) = 0;

	 if (TurnOffLight2 (tilex, tiley, -1))
       LightSourceAt(tilex,tiley-1) = 0;

	 if (TurnOffLight1 (tilex, tiley, 1, -1))
       LightSourceAt(tilex+1,tiley-1) = 0;

	 if (TurnOffLight3 (tilex, tiley, 1))
       LightSourceAt(tilex+1,tiley) = 0;

	 if (TurnOffLight1 (tilex, tiley, 1, 1))
       LightSourceAt(tilex+1,tiley+1) = 0;

	 if (TurnOffLight2 (tilex, tiley, 1))
       LightSourceAt(tilex,tiley+1) = 0;

	 if (TurnOffLight1 (tilex, tiley, -1, 1))
       LightSourceAt(tilex-1,tiley+1) = 0;

	 if (TurnOffLight3 (tilex, tiley, -1))
       LightSourceAt(tilex-1,tiley) = 0;
}


/*
===============
=
= TurnOffLight0
=
===============
*/

boolean TurnOffLight0 (int tilex, int tiley)
{
   if ( IsLight(tilex-1,tiley  ) ||
        IsLight(tilex-1,tiley-1) ||
        IsLight(tilex  ,tiley-1) ||
        IsLight(tilex+1,tiley-1) ||
        IsLight(tilex+1,tiley  ) ||
        IsLight(tilex+1,tiley+1) ||
        IsLight(tilex  ,tiley+1) ||
		  IsLight(tilex-1,tiley+1) )
		return (false);
	else
		return (true);
}

/*
===============
=
= TurnOffLight1
=
===============
*/

boolean TurnOffLight1 (int tilex, int tiley, int i, int j)
{
	int tempi = 2*i;
	int tempy = 2*j;

   if ( IsLight(tilex+i    ,tiley  ) ||
        IsLight(tilex+i    ,tiley+j) ||
        IsLight(tilex      ,tiley+j) ||
        IsLight(tilex      ,tiley+tempy) ||
        IsLight(tilex+i    ,tiley+tempy) ||
        IsLight(tilex+tempi,tiley+tempy) ||
        IsLight(tilex+tempi,tiley+j) ||
	     IsLight(tilex+tempi,tiley))
		return (false);
	else
		return (true);
}


/*
===============
=
= TurnOffLight2
=
===============
*/

boolean TurnOffLight2 (int tilex, int tiley, int j)
{
	int tempy = 2*j;

   if ( IsLight(tilex-1    ,tiley  ) ||
        IsLight(tilex-1    ,tiley+j) ||
        IsLight(tilex-1    ,tiley+tempy) ||
        IsLight(tilex      ,tiley+j) ||
        IsLight(tilex      ,tiley+tempy) ||
        IsLight(tilex+1    ,tiley) ||
        IsLight(tilex+1    ,tiley+j) ||
	     IsLight(tilex+1    ,tiley+tempy))
		return (false);
	else
		return (true);
}


/*
===============
=
= TurnOffLight3
=
===============
*/

boolean TurnOffLight3 (int tilex, int tiley, int i)
{
	int tempx = 2*i;

   if ( IsLight(tilex      ,tiley-1) ||
        IsLight(tilex+1    ,tiley-1) ||
        IsLight(tilex+tempx,tiley-1) ||
        IsLight(tilex+i    ,tiley) ||
        IsLight(tilex+tempx,tiley) ||
        IsLight(tilex      ,tiley+1) ||
        IsLight(tilex+i    ,tiley+1) ||
	     IsLight(tilex+tempx,tiley+1))
		return (false);
	else
		return (true);
}




/*
======================
=
= PreCacheStaticFrames
=
======================
*/




void PreCacheStaticFrames(statobj_t*temp)
   {
   int z,start,stop;
   int female=0,black=0;

   if (temp->itemnumber != stat_bullethole &&
       ((temp->itemnumber < stat_touch1) || (temp->itemnumber > stat_touch4)))
      PreCacheLump(temp->shapenum+shapestart,PU_CACHESPRITES,cache_patch_t);
   else
      PreCacheLump(temp->shapenum+shapestart,PU_CACHESPRITES,cache_transpatch_t);
   for (z=0;z<temp->numanims;z++)
      PreCacheLump(temp->shapenum+shapestart+z,PU_CACHESPRITES,cache_patch_t);

   if (temp->flags & FL_ROTATING)
      {
      for (z=1;z<8;z++)
         PreCacheLump(temp->shapenum+shapestart+z,PU_CACHESPRITES,cache_patch_t);
      }

   if (temp->flags & FL_WOODEN)
      {
      start = W_GetNumForName("WFRAG1");
      stop = W_GetNumForName("WFRAG14");
      PreCacheGroup(start,stop,cache_patch_t);
      }

   if (temp->flags & FL_METALLIC)
      {
      PreCacheLump(W_GetNumForName("MSHARDS"),PU_CACHESPRITES,cache_patch_t);
      start = W_GetNumForName("ROBODIE1");
      stop = W_GetNumForName("ROBODEAD");
      PreCacheGroup(start,stop,cache_patch_t);
      }

   female = ((locplayerstate->player == 1) || (locplayerstate->player == 3));
   black = (locplayerstate->player == 2);

   if (female)
      {
      start = W_GetNumForName("FPIST11");
      stop = W_GetNumForName("FPIST13");
      }
   else if (black)
      {
      start = W_GetNumForName("BMPIST1");
      stop = W_GetNumForName("BMPIST3");
      }
   else
      {
      start = W_GetNumForName("MPIST11");
      stop = W_GetNumForName("MPIST13");
		}

   PreCacheGroup(start,stop,cache_patch_t);

   switch (temp->itemnumber)
      {

      case stat_pedgoldkey:
      case stat_pedsilverkey:
      case stat_pedironkey:
      case stat_pedcrystalkey:
         PreCacheLump(W_GetNumForName("PEDESTA"),PU_CACHESPRITES,cache_patch_t);
         break;

      case stat_bat:
         start = W_GetNumForName("EXBAT1");
         stop = W_GetNumForName("EXBAT7");
         PreCacheGroup(start,stop,cache_patch_t);
         break;
      case stat_knifestatue:
         start = W_GetNumForName("KNIFE1");
         stop = W_GetNumForName("KNIFE10");
         PreCacheGroup(start,stop,cache_patch_t);
         break;
      case stat_twopistol:
         if (female)
            {
            start = W_GetNumForName("RFPIST1");
            stop = W_GetNumForName("LFPIST3");
            }
         else if (black)
            {
            start = W_GetNumForName("RBMPIST1");
            stop = W_GetNumForName("LBMPIST3");
            }
         else
            {
            start = W_GetNumForName("RMPIST1");
            stop = W_GetNumForName("LMPIST3");
            }
         PreCacheGroup(start,stop,cache_patch_t);
         break;
      case stat_mp40:
         start = W_GetNumForName("MP401");
         stop = W_GetNumForName("MP403");
         PreCacheGroup(start,stop,cache_patch_t);
         break;
      case stat_bazooka:
         start = W_GetNumForName("BAZOOKA1");
         stop = W_GetNumForName("BAZOOKA4");
         PreCacheGroup(start,stop,cache_patch_t);
         break;
      case stat_firebomb:
         start = W_GetNumForName("FBOMB1");
         stop = W_GetNumForName("FBOMB4");
         PreCacheGroup(start,stop,cache_patch_t);
         break;
      case stat_heatseeker:
         start = W_GetNumForName("HSEEK1");
         stop = W_GetNumForName("HSEEK4");
         PreCacheGroup(start,stop,cache_patch_t);
         break;
      case stat_drunkmissile:
         start = W_GetNumForName("DRUNK1");
         stop = W_GetNumForName("DRUNK4");
         PreCacheGroup(start,stop,cache_patch_t);
         break;
      case stat_firewall:
         start = W_GetNumForName("FIREW1");
         stop = W_GetNumForName("FIREW3");
         PreCacheGroup(start,stop,cache_patch_t);
         break;
      case stat_splitmissile:
         start = W_GetNumForName("SPLIT1");
         stop = W_GetNumForName("SPLIT4");
         PreCacheGroup(start,stop,cache_patch_t);
         break;
      case stat_kes:
         start = W_GetNumForName("KES1");
         stop = W_GetNumForName("KES6");
         PreCacheGroup(start,stop,cache_patch_t);
         break;
      case stat_godmode:
         start = W_GetNumForName("GODHAND1");
         stop = W_GetNumForName("GODHAND8");
         PreCacheGroup(start,stop,cache_patch_t);

         PreCacheGroup(W_GetNumForName("VAPO1"),
                       W_GetNumForName("LITSOUL"),
                       cache_patch_t);

         PreCacheGroup(W_GetNumForName("GODFIRE1"),
                       W_GetNumForName("GODFIRE4"),
                       cache_patch_t);

         break;
      case stat_dogmode:
         start = W_GetNumForName("DOGNOSE1");
         stop = W_GetNumForName("DOGPAW4");
         PreCacheGroup(start,stop,cache_patch_t);
         break;

      default:
	  ;
      }



   }



/*
===============
=
= LoadStatics
=
===============
*/




void LoadStatics( byte * buffer, int size)
{saved_stat_type dummy;
 int stcount,i;
 statobj_t*temp;

 stcount = size/sizeof(saved_stat_type);
 InitStaticList();

 for(i=0;i<stcount;i++)
	{
	temp = (statobj_t*)Z_LevelMalloc(sizeof(statobj_t),PU_LEVELSTRUCT,NULL);
	if (!temp)
		Error("LoadStatics: Failed on allocation of static %d of %d",i,stcount);
	memset(temp,0,sizeof(*temp));
	memcpy(&(dummy.x),buffer,sizeof(saved_stat_type));
	temp->whichstat = statcount++;
	temp->x = dummy.x;
	temp->y = dummy.y;
	temp->z = dummy.z;
	temp->flags = dummy.flags;
	temp->ticcount = dummy.ticcount;
	temp->hitpoints = dummy.hitpoints;
	temp->shapenum = dummy.shapenum;
	temp->ammo = dummy.ammo;
	temp->count = dummy.count;
	temp->numanims = dummy.numanims;
	temp->itemnumber = dummy.itemnumber;
	temp->areanumber = dummy.areanumber;
	temp->linked_to = dummy.linked_to;

	temp->which = SPRITE;
	temp->tilex = temp->x >> TILESHIFT;
	temp->tiley = temp->y >> TILESHIFT;
	temp->flags &= ~FL_ABP;
	temp->visspot = &spotvis[temp->tilex][temp->tiley];

	if ((temp->itemnumber >= stat_touch1) &&
		 (temp->itemnumber <= stat_touch4))
	  {touchindices[temp->tilex][temp->tiley] = lasttouch + 1;
		lasttouch ++;
		SD_PreCacheSoundGroup(SD_TOUCHPLATESND,SD_BADTOUCHSND);
	  }

	AddStatic(temp);
	if (temp->shapenum != -1)
	  {
     if (temp->itemnumber == stat_bullethole)
		  {
		  SetupBulletHoleLink(temp->linked_to,temp);
		  }

	  else if (temp->flags & FL_DEADBODY)
		  {
        if ( actorat[temp->tilex][temp->tiley] == NULL )
           actorat[temp->tilex][temp->tiley] = temp;
		  }

	  else if (!(temp->flags & FL_NONMARK))
		  {
		  sprites[temp->tilex][temp->tiley] = temp;
		  }

     PreCacheStaticFrames(temp);


	  }
	PreCacheStaticSounds(temp->itemnumber);

	buffer += sizeof(saved_stat_type);
  }
}


void Set_NewZ_to_MapValue(fixed *newz,int zoffset,const char*errorstr,int tilex,int tiley)
   {
   int zf,z;

   zoffset&=0xff;
   z=zoffset>>4;
   zf=zoffset&0xf;
   if (z==0xf)
      *newz = nominalheight+64-(zf<<2);
   else
      {
      if (z>levelheight)
         Error ("You specified a height of %x for the %s at tilex=%d tiley=%d when\n the level is only %d high\n",
                 zoffset,errorstr,tilex,tiley,levelheight);
      else
         *newz = nominalheight-(z<<6)-(zf<<2);
      }

   }



/*
===============
=
= SpawnStatic
=
===============
*/


int BaseMarkerZ;//bna++			

void SpawnStatic (int tilex, int tiley, int mtype, int zoffset)
{statobj_t * temp;
 boolean onetimer;



#if (SHAREWARE == 1)
   switch(mtype)
      {
      case stat_rlight:
      case stat_glight:
      case stat_ylight:
      case stat_chandelier:
         mtype = stat_blight;
         break;

      case stat_garb1:
      case stat_garb2:
      case stat_garb3:
      case stat_shit:
         mtype = stat_metalshards;
         break;

      case stat_lamp:
         mtype = stat_altbrazier2;
         break;

      }
#endif



   if ( BATTLEMODE )
      {
      if ( !gamestate.BattleOptions.SpawnWeapons )
         {
         if ( stats[ mtype ].flags & FL_WEAPON )
            {
            return;
            }
         }

      if (mtype == stat_pit)
         return;

      // Change lifeitems and extra lives to health
      switch( mtype )
         {
         case stat_lifeitem1 :
         case stat_lifeitem2 :
            mtype = stat_monkcrystal1;
            break;

         case stat_lifeitem3 :
         case stat_lifeitem4 :
         case stat_oneup :
         case stat_threeup :
            mtype = stat_monkcrystal2;
            break;
         }

      switch( mtype )
         {
         case stat_monkmeal :
         case stat_priestporridge :
         case stat_monkcrystal1 :
         case stat_monkcrystal2 :
         case stat_healingbasin :
            if ( ( gamestate.Product != ROTT_SHAREWARE ) &&
               ( gamestate.BattleOptions.SpawnMines ) &&
               ( !IsPlatform( tilex, tiley ) &&
               ( ( zoffset & 0xff00 ) != 0xb000 ) ) &&
               ( zoffset == -1 ) )
               {
               mtype = stat_mine;
               }
            else if ( !gamestate.BattleOptions.SpawnHealth )
               {
               return;
               }
            break;
         }
      }

 if (!firstemptystat)
	{temp = (statobj_t*)Z_LevelMalloc(sizeof(statobj_t),PU_LEVELSTRUCT,NULL);
	 //SoftError("\nMalloc-ing actor");
	 //if (insetupgame)
	  //	SoftError("in setup");
	}

 else
	{temp = lastemptystat;
	 //SoftError("\nfree actor available");
	 RemoveFromFreeStaticList(lastemptystat);
	}

 // Standard pole hack

 if ((zoffset>=14) && (zoffset<=17))
	 zoffset=-1;

 if (temp)
  {  memset(temp,0,sizeof(*temp));
	  temp->shapenum = stats[mtype].picnum;
	  temp->whichstat = statcount ++;
	  temp->tilex = tilex;
	  temp->tiley = tiley;
	  temp->x = ((long)tilex << TILESHIFT) + 0x8000;
	  temp->y = ((long)tiley << TILESHIFT) + 0x8000;
	  temp->areanumber = MAPSPOT(tilex,tiley,0)-AREATILE;
	  temp->linked_to = -1;
	  if ((temp->areanumber<=0) || (temp->areanumber>NUMAREAS))
		  Error ("Sprite at x=%d y=%d type=%d has an illegal areanumber\n",tilex,tiley,mtype);
     if ( mtype == stat_mine )
        {
        temp->z = nominalheight;
        }
     else if (zoffset!=-1)
		  {
		  if ((zoffset&0xff00)==0xb000)
           Set_NewZ_to_MapValue(&(temp->z),zoffset,"static",tilex,tiley);
		  else if (IsPlatform(tilex,tiley))
			  temp->z = PlatformHeight(tilex,tiley);
		  else if (zoffset==11)
			  temp->z=-65;
		  else if (zoffset==12)
			  temp->z=-66;
		  else
			  temp->z = nominalheight;
			 // Error ("You didn't specify a valid height over the sprite at tilex=%ld tiley=%ld\n",tilex,tiley);
		  }
	  else if (mtype>stat_chandelier)
		  temp->z = nominalheight;

	  temp->visspot = &spotvis[tilex][tiley];
	  temp->which = SPRITE;
	  temp->ticcount = stats[mtype].tictime;
	  temp->hitpoints = stats[mtype].hitpoints;
	  temp->itemnumber = stats[mtype].type;
     temp->flags = stats[mtype].flags;
	  temp->ammo = stats[mtype].ammo;
	  temp->numanims = stats[mtype].numanims;




     if (temp->flags & FL_BONUS)
		switch  (stats[mtype].type)
		  {case stat_lifeitem1:
			case stat_lifeitem2:
			case stat_lifeitem3:
			case stat_lifeitem4:
			 gamestate.treasuretotal++;
			 break;
		  default:
		      ;
		  }




      AddStatic(temp);

     onetimer = ((mtype == stat_rubble) || (mtype == stat_woodfrag) ||
                 (mtype == stat_metalfrag) || (mtype == stat_missmoke)
                );

     if (DoPanicMapping())
        {
        if (temp->numanims && (!onetimer))
           {
           temp->flags &= ~FL_ACTIVE;
           temp->numanims = 0;
           GameRandomNumber("SpawnStatic",mtype);
           }
        }

     else
        {
        if (temp->numanims)
           {
           if (!onetimer)
              temp->count = (int)(((int)GameRandomNumber("SpawnStatic",mtype) % stats[mtype].numanims) + 1);
           else
              temp->count = 0;
           }
        else if (temp->itemnumber == stat_standardpole)
           {
           if (MAPSPOT(temp->tilex,temp->tiley,2))
              temp->count = 2*(MAPSPOT(temp->tilex,temp->tiley,2)-14);
           else
              temp->count = 0;
           }

        if ((temp->itemnumber == stat_knifestatue) ||
            (temp->itemnumber == stat_emptystatue) ||
            (temp->itemnumber == stat_standardpole))
           temp->flags|=FL_ROTATING;
        }




     if (mtype != stat_missmoke)
		  sprites[tilex][tiley] = temp;
	  else
		  temp->flags |= FL_NONMARK;


//================ check special junk ==================================//

	  if (temp->itemnumber == stat_dariantouch)
		 {_2Dpoint *tdptr;

		  tdptr = &(MISCVARS->ETOUCH[MISCVARS->nexttouch]);

		  tdptr->x = tilex;
		  tdptr->y = tiley;
		  sprites[tilex][tiley]->linked_to = MISCVARS->nexttouch;
		  MISCVARS->nexttouch ++;
		 }
	  else if ((temp->itemnumber >= stat_touch1) &&
				  (temp->itemnumber <= stat_touch4))
		 {
        if (lasttouch < MAXTOUCHPLATES) {
            touchindices[tilex][tiley] = lasttouch + 1;
            SD_PreCacheSoundGroup(SD_TOUCHPLATESND,SD_BADTOUCHSND);
            lasttouch ++;
        } else {
            printf("WARNING: Max touchplates reached at %d,%d\n", tilex, tiley);
        }
		 }

//=====================================================================//
	//bna added
	// BaseMarkerZ used to adjust height in s_basemarker1
	// in SpawnNewObj(i,j,&s_basemarker1,inertobj); 
	BaseMarkerZ=temp->z;//bna++	BaseMarkerZ = spawnz;

     PreCacheStaticFrames(temp);

     PreCacheStaticSounds(temp->itemnumber);

	  if (temp->flags & FL_WEAPON)
		 MISCVARS->NUMWEAPONS ++;
  }
 else
	Error("Z_LevelMalloc failed in SpawnStatic!");

}

/*
===============
=
= SpawnInertStatic
=
===============
*/

void SpawnInertStatic (int x, int y, int z, int mtype)
{
   (void)mtype;
   statobj_t * temp;




 if (!firstemptystat)
	{temp = (statobj_t*)Z_LevelMalloc(sizeof(statobj_t),PU_LEVELSTRUCT,NULL);
	 //SoftError("\nMalloc-ing actor");
	 //if (insetupgame)
	  //	SoftError("in setup");
	}

 else
	{temp = lastemptystat;
	 //SoftError("\nfree actor available");
	 RemoveFromFreeStaticList(lastemptystat);
	}

 if (temp)
  {  memset(temp,0,sizeof(*temp));
	  temp->shapenum = stats[mtype].picnum;

	  temp->whichstat = statcount ++;
	  temp->tilex = x>>16;
	  temp->tiley = y>>16;
	  temp->x = x;
	  temp->y = y;
	  temp->areanumber = MAPSPOT(temp->tilex,temp->tiley,0)-AREATILE;
	  temp->linked_to = -1;
	  if ((temp->areanumber<=0) || (temp->areanumber>NUMAREAS))
		  {
		  int tilex=temp->tilex;
		  int tiley=temp->tiley;

		  FindEmptyTile(&tilex,&tiley);
		  temp->areanumber = MAPSPOT(tilex,tiley,0)-AREATILE;
		  }
	  temp->z=z;
	  temp->visspot = &spotvis[temp->tilex][temp->tiley];
	  temp->which = SPRITE;
	  temp->ticcount = stats[mtype].tictime;
	  temp->hitpoints = stats[mtype].hitpoints;
	  temp->itemnumber = stats[mtype].type;
	  temp->flags = (stats[mtype].flags|FL_ABP|FL_NONMARK);
     if (fog)
       {temp->shapenum++;
        temp->flags &= ~FL_TRANSLUCENT;
       }

     temp->ammo = stats[mtype].ammo;
	  temp->numanims = stats[mtype].numanims;
	  AddStatic(temp);
	  MakeStatActive(temp);
  }
 else
	Error("Z_LevelMalloc failed in SpawnStatic!");

}

/*
===============
=
= SpawnSolidStatic
=
===============
*/

void SpawnSolidStatic (statobj_t * temp)
{
	if (temp->flags & FL_ACTIVE)
	  temp->flags &= ~FL_ACTIVE;
	temp->hitpoints = INITIALFIRECOLOR;
   temp->flags = FL_SOLIDCOLOR|FL_SEEN|FL_ABP;
   if ((gamestate.BattleOptions.RespawnItems) &&
       (temp->itemnumber == stat_tntcrate)
      )
      temp->flags |= FL_RESPAWN;

   temp->ticcount = SOLIDCOLORTICTIME;
}


/*
======================
=
= PreCacheStaticSounds
=
======================
*/



void PreCacheStaticSounds (int itemnumber)
{

	if (stats[itemnumber].flags & FL_SHOOTABLE)
	  SD_PreCacheSound(SD_ITEMBLOWSND);


   switch(itemnumber)

      {
      case    stat_pit:
        SD_PreCacheSound(SD_PITTRAPSND);
        break;

      case    stat_bonusbarrel:
        SD_PreCacheSound(SD_BONUSBARRELSND);
        break;

      case    stat_knifestatue:
        SD_PreCacheSound(SD_GETKNIFESND);
        break;
      case    stat_gibs1:
      case    stat_gibs2:
      case    stat_gibs3:
         SD_PreCacheSound (SD_ACTORSQUISHSND);
         break;
      case    stat_pedgoldkey:
      case    stat_pedsilverkey:
      case    stat_pedironkey:
      case    stat_pedcrystalkey:
         SD_PreCacheSound (SD_GETKEYSND);

         break;
      case    stat_monkmeal:
         SD_PreCacheSound (SD_GETHEALTH1SND);
         break;
      case    stat_monkcrystal1:
         SD_PreCacheSound (SD_GETHEALTH2SND);
         break;
      case   stat_monkcrystal2:
         SD_PreCacheSound (SD_GETHEALTH2SND);
         break;
      case    stat_priestporridge:
         SD_PreCacheSound (SD_GETHEALTH1SND);
         SD_PreCacheSound(SD_COOKHEALTHSND);

         break;

      case   stat_healingbasin:
         SD_PreCacheSound (SD_GETHEALTH2SND);
         break;

      case stat_oneup:
         SD_PreCacheSound(SD_GET1UPSND);
         break;
      case stat_threeup:
         SD_PreCacheSound(SD_GET3UPSND);
         break;

      case stat_scotthead:
         SD_PreCacheSound(SD_GETHEADSND);
         break;

      case stat_twopistol:
      case stat_mp40:
      case stat_bazooka:
      case stat_firebomb:
      case stat_heatseeker:
      case stat_drunkmissile:
      case stat_firewall:
      case stat_splitmissile:
      case stat_kes:
         SD_PreCacheSound(SD_GETWEAPONSND);
         break;

      case stat_bat:
         SD_PreCacheSound(SD_GETBATSND);
         break;

      case stat_lifeitem1:
      case stat_lifeitem2:
      case stat_lifeitem3:
      case stat_lifeitem4:
         SD_PreCacheSound(SD_GETBONUSSND);
         break;


      case stat_random:
         SD_PreCacheSound(SD_GETGODSND);
         SD_PreCacheSound(SD_GETDOGSND);
         SD_PreCacheSound(SD_GETELASTSND);
         SD_PreCacheSound(SD_GETSHROOMSSND);
         SD_PreCacheSound(SD_LOSEMODESND);

         break;
      case stat_bulletproof:
         SD_PreCacheSound(SD_GETBVESTSND);
         SD_PreCacheSound(SD_LOSEMODESND);
         break;

      case stat_gasmask:
         SD_PreCacheSound(SD_GETMASKSND);
         SD_PreCacheSound(SD_LOSEMODESND);
         break;

      case stat_asbesto:
         SD_PreCacheSound(SD_GETAVESTSND);
         SD_PreCacheSound(SD_LOSEMODESND);
         break;

      case stat_elastic:
         SD_PreCacheSound(SD_GETELASTSND);
         SD_PreCacheSound(SD_LOSEMODESND);
         break;

      case stat_fleetfeet:
         SD_PreCacheSound(SD_GETFLEETSND);
         SD_PreCacheSound(SD_LOSEMODESND);
         break;

      case stat_godmode:
         SD_PreCacheSound(SD_GETGODSND);
         SD_PreCacheSound(SD_GODMODE1SND);

         break;

      case stat_dogmode:
         SD_PreCacheSound(SD_GETDOGSND);
         break;

      case stat_mushroom:
         SD_PreCacheSound(SD_GETSHROOMSSND);
         SD_PreCacheSound(SD_LOSEMODESND);

         break;

      case stat_dipball1:
      case stat_dipball2:
      case stat_dipball3:
         SD_PreCacheSound(SD_GETBONUSSND);
         break;

      default:
         SD_PreCacheSound(SD_GETBONUSSND);
         break;
      }

}





/*
===============
=
= SaveSwitches
=
===============
*/

void SaveSwitches(byte ** buffer, int * size)
{int numswitches,i;
 byte * tptr;

 numswitches = lastswitch-&switches[0];
 if (numswitches==0)
	 {
	 *size=0;
	 *buffer=SafeMalloc(16);
	 return;
	 }
 *size = numswitches*sizeof(wall_t);

 *buffer = (byte *)SafeMalloc(*size);
 tptr = *buffer;

 for(i=0;i<numswitches;i++)
  {memcpy(tptr,&switches[i],sizeof(wall_t));
	tptr += sizeof(wall_t);
  }

}


/*
===============
=
= LoadSwitches
=
===============
*/

void LoadSwitches (byte * buffer, int size)
{int numswitches,i,tilex,tiley;

 numswitches = size/sizeof(wall_t);

 for(i=0;i<numswitches;i++)
  {memcpy(&switches[i],buffer,sizeof(wall_t));
	tilex = switches[i].tilex;
	tiley = switches[i].tiley;
	actorat[tilex][tiley]=&switches[i];
	if (MAPSPOT(tilex,tiley,0) == 79) // On by default
      {
	   if (!(switches[i].flags & FL_ON))
         tilemap[tilex][tiley]--;
      }
   else if (switches[i].flags & FL_W_INVERTED) // Hi masked wall
      {
      maskedwallobj_t * lastmaskobj;

      lastmaskobj=maskobjlist[tilemap[tilex][tiley]&0x3ff];
    	if (switches[i].flags & FL_ON)
         lastmaskobj->toptexture++;
      }
	else if (switches[i].flags & FL_ON)
	   tilemap[tilex][tiley]++;
	buffer += sizeof(wall_t);
   touchindices[tilex][tiley] = lasttouch + 1;
	lasttouch ++;
  }
 lastswitch=&switches[numswitches];

}



/*
===============
=
= SpawnSwitchThingy
=
===============
*/

void SpawnSwitchThingy(int tilex, int tiley)
{
 lastswitch->flags |= FL_SWITCH;
 lastswitch->which = WALL;
 lastswitch->tilex = tilex;
 lastswitch->tiley = tiley;
 touchindices[tilex][tiley] = lasttouch + 1;
 lasttouch ++;
 actorat[tilex][tiley] = lastswitch;
 lastswitch ++;

}

/*
===============
=
= AnimateWalls
=
===============
*/

void AnimateWalls(void)
{
	int i;
	animwall_t * aw;

   if (DoPanicMapping()==true)
      return;

   for(i=0;i<MAXANIMWALLS;i++)
		{
		aw=&animwalls[i];
		if (aw->active==0)
			continue;
		if (aw->ticcount <= 0)
			{
			if (aw->count < animwallsinfo[i].numanims)
				{
				aw->texture = aw->basetexture + aw->count;
				aw->count++;
				}
			else
				{
				aw->texture = aw->basetexture;
				aw->count = 1;
				}

			while (aw->ticcount<=0)
				aw->ticcount+=animwallsinfo[i].tictime;
			}
		else
			aw->ticcount -= tics;
		}
}


#define M_ResetSprites(x)  \
  { if ((index == stat_dariantouch) && (!x->count))\
		 {x->shapenum = stats[index].picnum;               \
		  x->count = 1;                                     \
		  x->ticcount = stats[index].tictime;         \
		  x->flags &= ~FL_BACKWARDS;                   \
		  x->flags &= ~FL_ACTIVE;                       \
		 }                                               \
	}




void CheckCriticalStatics(void)
{respawn_t *rtemp,*ddt;
 int i,stilex,stiley;
 statobj_t*temp,*stat;

 for(rtemp = firstrespawn;rtemp;)
	{rtemp->ticcount --;
	 ddt = rtemp->next;

	 if (rtemp->ticcount <=0)
		 {int stype;

		  stilex = rtemp->tilex;
		  stiley = rtemp->tiley;

		  // if another weapon is there, nuke it
		  if (sprites[stilex][stiley])
			 {RemoveStatic(sprites[stilex][stiley]);
			  sprites[stilex][stiley] = NULL;
			 }

        if (rtemp->itemnumber == stat_tntcrate)
           {
           RemoveStatic((statobj_t*)(rtemp->linked_to));
           stype = stat_tntcrate;
           }
        else
           {
           for(i=0;i<NUMSTATS;i++)
              {
              if (rtemp->itemnumber == stats[i].type)
                 {
                 stype = i;
                 break;
                 }
              }

           }

		  SpawnStatic(stilex,stiley,stype,-1);
		  LASTSTAT->z = rtemp->spawnz;
		  LASTSTAT->flags |= FL_ABP;
		  MakeStatActive(LASTSTAT);
        SpawnNewObj(stilex,stiley,&s_itemspawn1,inertobj);
        SD_PlaySoundRTP(SD_RESPAWNSND,new->x,new->y);
		  new->flags |= FL_ABP;
		  MakeActive(new);
		  new->z = LASTSTAT->z;
		  RemoveRespawnStatic(rtemp);
		 }

	 rtemp = ddt;
	}

 for (temp = firstactivestat ; temp; )
	 {stat = temp->nextactive;

	  if (temp->flags & FL_SOLIDCOLOR)
		  {temp->ticcount--;
			if (temp->ticcount<=0)
				{temp->hitpoints+=SOLIDCOLORINCREMENT;
				 if (temp->hitpoints>MAXFIRECOLOR)
					 RemoveStatic(temp);
				 else
					 temp->ticcount = SOLIDCOLORTICTIME;
				}
		  }
	  temp = stat;
	 }


}



/*
===============
=
= DoSprites
=
===============
*/

void DoSprites(void)
{int index,i;
 statobj_t *temp,*tempnext;


#if (0)
	 Debug("\n");
#endif
i=0;
for(temp = firstactivestat;temp;)
  {tempnext = temp->nextactive;

	#if (0)
	 Debug("\nid: %d, shapenum: %d, numanims: %d",i++,temp->shapenum,temp->numanims);
	#endif

	if ((temp->shapenum != NOTHING) && (temp->flags & FL_ACTIVE))
		{index = temp->itemnumber;
		 temp->ticcount -= tics;
		 while (temp->ticcount <= 0)
			  {if (temp->count < temp->numanims)
					 {temp->shapenum = stats[index].picnum + temp->count;
					  if (index == stat_missmoke)
						  {RemoveStatic(temp);
							break;
						  }

					  if (((index == stat_rubble) && (temp->count == 9)) ||
							((index == stat_woodfrag) && (temp->count == 13)) ||
							((index == stat_metalfrag) && (temp->count == 9)))
						 {temp->flags &= ~FL_ACTIVE;
						  break;
						 }

					  if (temp->flags & FL_BACKWARDS)
						 {temp->count--;
						  M_ResetSprites(temp);
						 }
					  else
						 temp->count ++;
					 }
				else if (!(temp->flags & FL_BANDF))
					{temp->shapenum = stats[index].picnum;
					 temp->count=1;
					}
				else
					{temp->flags |= FL_BACKWARDS;
					 temp->count --;
					}
				temp->ticcount += stats[index].tictime;
			  }
		}
  temp = tempnext;

  }

}

void  SpawnStaticDamage(statobj_t * stat, int angle)
{
	GetNewActor ();
	MakeActive(new);
	NewState(new,&s_gunsmoke1);

	new->obclass = inertobj;
	new->which = ACTOR;
	new->x = (stat->x)-(costable[angle]>>4);
	new->y = (stat->y)+(sintable[angle]>>4);
	new->z = (stat->z)+stats[stat->itemnumber].heightoffset;
	new->drawx = new->x;
	new->drawy = new->y;
	new->tilex = (new->x >> TILESHIFT);
	new->tiley = (new->y >> TILESHIFT);
	new->dir = 0;
	new->speed = 0;
	new->flags = (FL_NEVERMARK|FL_ABP);
	if ((new->x<=0) || (new->y<=0))
	   Error("SpawnStaticDamage: bad x,y itemnumber=%d\n",stat->itemnumber);
}
