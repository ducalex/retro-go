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
//***************************************************************************
//
//                  RT_STAT.C (static object functions)
//
//***************************************************************************
#ifndef _rt_stat_public
#define _rt_stat_public

#include "rt_ted.h"

#define MAXSWITCHES     64
#define MAXANIMWALLS    17
#define NUMSTATS        91

typedef enum {
		  stat_ylight,
		  stat_rlight,
		  stat_glight,
		  stat_blight,
		  stat_chandelier,
		  stat_lamp,
		  stat_pedgoldkey,
		  stat_pedsilverkey,
		  stat_pedironkey,
		  stat_pedcrystalkey,
		  stat_gibs1,
		  stat_gibs2,
		  stat_gibs3,
		  stat_monkmeal,
		  stat_priestporridge,
		  stat_monkcrystal1,
		  stat_monkcrystal2,
		  stat_oneup,
		  stat_threeup,
		  stat_altbrazier1,
		  stat_altbrazier2,
		  stat_healingbasin,
		  stat_emptybasin,
        stat_bat,
        stat_knifestatue,
		  stat_twopistol,
		  stat_mp40,
		  stat_bazooka,
        stat_firebomb,
        stat_heatseeker,
		  stat_drunkmissile,
		  stat_firewall,
        stat_splitmissile,
		  stat_kes,
        stat_lifeitem1,
		  stat_lifeitem2,
		  stat_lifeitem3,
		  stat_lifeitem4,
		  stat_tntcrate,
		  stat_bonusbarrel,
		  stat_torch,
		  stat_floorfire,
		  stat_dipball1,
		  stat_dipball2,
		  stat_dipball3,
		  stat_touch1,
		  stat_touch2,
		  stat_touch3,
		  stat_touch4,
		  stat_dariantouch,
		  stat_scotthead,
		  stat_garb1,
		  stat_garb2,
		  stat_garb3,
		  stat_shit,
		  stat_grate,
		  stat_metalshards,
		  stat_emptypedestal,
		  stat_emptytable,
		  stat_stool,
		  stat_bcolumn,
		  stat_gcolumn,
		  stat_icolumn,
		  stat_tree,
		  stat_plant,
		  stat_urn,
		  stat_haystack,
		  stat_ironbarrel,
		  stat_heatgrate,
		  stat_standardpole,
        stat_pit,
        stat_godmode,
        stat_dogmode,
        stat_fleetfeet,
		  stat_elastic,
		  stat_mushroom,
		  stat_gasmask,
		  stat_bulletproof,
		  stat_asbesto,
		  stat_random,
		  stat_rubble,
		  stat_woodfrag,
		  stat_metalfrag,
		  stat_emptystatue,
		  stat_tomlarva,
		  stat_bullethole,
		  stat_collector,
		  stat_mine,
		  stat_missmoke,
		  stat_disk,
		  stat_badstatic
} stat_t;

typedef struct awall
{
  byte active;
  byte count;
  signed char ticcount;
  int  texture;
  int  basetexture;
} animwall_t;

typedef struct awallinfo_s
{
    int  tictime,
         numanims;
    char firstlump[9];
} awallinfo_t;

typedef struct statstruct
{
		  thingtype         which;
		  byte              tilex,tiley;
		  fixed             x,y,z;
		  int               shapenum;
		  unsigned          flags;
		  signed char       ticcount;
		  signed char       ammo;
		  byte              *visspot;
		  signed char       count;
		  byte              numanims;
		  stat_t            itemnumber;
		  short int         hitpoints;
		  short int         whichstat;
		  short int         areanumber;

		  long              linked_to;
		  struct statstruct *statnext;
		  struct statstruct *statprev;
		  struct statstruct *nextactive;
		  struct statstruct *prevactive;

} statobj_t;

typedef struct respstruct
{
		  byte              tilex,tiley;
		  int               ticcount;
		  stat_t            itemnumber;
		  int               spawnz;
		  struct respstruct *next;
		  struct respstruct *prev;
                  long              linked_to;
} respawn_t;


typedef struct
{
		  short      heightoffset;
		  int        picnum;
		  stat_t     type;
		  unsigned   flags;
		  byte       tictime;
		  byte       numanims;
		  byte       hitpoints;
		  byte       damage;
		  signed char  ammo;
} statinfo;

extern  statobj_t       *lastactivestat,*firstactivestat;
extern  statobj_t       *firstemptystat,*lastemptystat;
extern  int             spritestart;
extern  wall_t          *switches,*lastswitch;

extern  respawn_t       *firstrespawn,*lastrespawn;
extern  statobj_t       *FIRSTSTAT,*LASTSTAT,*(*sprites)[MAPSIZE];
extern  statinfo        stats[NUMSTATS];
extern  dirtype         (*diagonal)[9];
extern  dirtype         *opposite;

extern  int             statcount;

extern   int            animwallstart;
extern   animwall_t     *animwalls;
extern   awallinfo_t    *animwallsinfo;


void Set_NewZ_to_MapValue(fixed*,int,const char*,int,int);
void RemoveFromFreeStaticList(statobj_t*);
void CheckCriticalStatics(void);
void ActivateLight(long);
void DeactivateLight(long);
void TurnOnLight(int,int);
void TurnOffLight(int,int);
void MakeStatActive(statobj_t*);
void MakeStatInactive(statobj_t*);
void AddStatic(statobj_t *);
void RemoveStatic(statobj_t *);


void SpawnSwitchThingy(int,int);
void InitStaticList (void);
void InitAnimatedWallList(void);
void SetupAnimatedWall(int which);
void SpawnStatic (int tilex, int tiley, int mtype, int zoffset);
void SpawnSolidStatic (statobj_t * temp);
void AnimateWalls(void);
void DoSprites(void);

void SaveAnimWalls(byte ** buf, int * size);
void SaveStatics(byte ** buf, int * size);

void LoadAnimWalls(byte * buf, int size);
void LoadStatics(byte * buf, int size);

void SaveSwitches(byte ** buf, int * size);
void LoadSwitches(byte * buf, int size);

void SpawnInertStatic (int x, int y, int z, int mtype);
void SpawnStaticDamage(statobj_t * stat, int angle);

#endif
