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
#include <string.h>
#include <stdlib.h>
#include "rt_sound.h"
#include "rt_door.h"
#include "rt_ted.h"
#include "rt_draw.h"
#include "watcom.h"
#include "z_zone.h"
#include "w_wad.h"
#include "lumpy.h"
#include "gmove.h"
#include "states.h"
#include "rt_sqrt.h"
#include "rt_stat.h"
#include "sprites.h"
#include "rt_actor.h"
#include "rt_game.h"
#include "rt_main.h"
#include "rt_playr.h"
#include "rt_util.h"
#include "rt_rand.h"
#include "rt_menu.h"
#include "rt_swift.h"
#include "_rt_acto.h"
#include "rt_cfg.h"
#include "rt_floor.h"
#include "engine.h"
#include "develop.h"
#include "rt_view.h"
#include "isr.h"
#include "rt_com.h"
#include "rt_scale.h"
#include "modexlib.h"
#include "rt_net.h"
#include "rt_msg.h"
#include "fx_man.h"
//MED
#include "memcheck.h"




#define SGN(x)  (((x) > 0)?(1):(-1))



#define WILEYBLITZCHANCE  20
#define GIBSOUND       SD_GIBSPLASHSND
#define ACTORTHUDSND   SD_BODYLANDSND
#define ACTORLANDSND   SD_PLAYERLANDSND

//========================== Global Variables ===================================================

#define SHP(difficulty,ob)  (starthitpoints[difficulty][ob->obclass])

#define CAP_OSCUROS_HITPOINTS(ob)                         \
   {                                                      \
   if (ob->hitpoints > (SHP(gamestate.difficulty,ob)<<1)) \
      ob->hitpoints = (SHP(gamestate.difficulty,ob)<<1);  \
   }


boolean           ludicrousgibs=false;

short             colheight[15];

#ifndef RETRO_GO
byte              deathshapeoffset[8] = {0,7,7,8,8,9,8,7};
#endif

unsigned long     MAXFUNCTION,MINFUNCTION,MAXSTATE,MINSTATE;

objtype           *PLAYER0MISSILE;
objtype           *SCREENEYE;
objtype           *FIRSTACTOR,*LASTACTOR;

objtype           *FIRSTFREE,*LASTFREE;
objtype           *lastactive,*firstactive,**objlist;
#ifndef RETRO_GO
void              *actorat[MAPSIZE][MAPSIZE];
#if (SHAREWARE == 0)
#ifndef RETRO_GO
_2Dpoint          SNAKEPATH[512];
#endif
#endif
objtype           *firstareaactor[NUMAREAS+1],*lastareaactor[NUMAREAS+1];
#endif
#if (DEVELOPMENT == 1)
FILE *            williamdebug;
#endif
exit_t            playstate;

void              T_SlideDownScreen(objtype*);

#ifndef RETRO_GO
basic_actor_sounds  BAS[NUMCLASSES+3] =
		  {{0,0,0,0,0},
			{0,0,0,0,0},
         {0,SD_LOWGUARD1SEESND,SD_LOWGUARDFIRESND,SD_LOWGUARDOUCHSND,SD_LOWGUARD1DIESND},
			{0,SD_HIGHGUARD1SEESND,SD_HIGHGUARDFIRESND,SD_HIGHGUARDOUCHSND,SD_HIGHGUARDDIESND},
			{0,SD_OVERP1SEESND,SD_OVERPFIRESND,SD_OVERPOUCHSND,SD_OVERPDIESND},
			{0,SD_STRIKE1SEESND,SD_STRIKEFIRESND,SD_STRIKEOUCHSND,SD_STRIKEDIESND},
			{0,SD_BLITZ1SEESND,SD_BLITZFIRESND,SD_BLITZOUCHSND,SD_BLITZDIESND},
			{0,SD_ENFORCERSEESND,SD_ENFORCERFIRESND,SD_ENFORCEROUCHSND,SD_ENFORCERDIESND} ,
			{0,SD_MONKSEESND,SD_MONKGRABSND,SD_MONKOUCHSND,SD_MONKDIESND},
			{0,SD_FIREMONKSEESND,SD_FIREMONKFIRESND,SD_FIREMONKOUCHSND,SD_FIREMONKDIESND},
         {0,SD_ROBOTSEESND,SD_ROBOTFIRESND,0,SD_ROBOTDIESND},

			//bosses
         {SD_DARIANSAY1,SD_DARIANSEESND,SD_DARIANFIRESND,0,SD_DARIANDIESND},
         {SD_KRISTSAY1,SD_KRISTSEESND,SD_KRISTFIRESND,0,SD_KRISTDIESND},
         {0,SD_NMESEESND,SD_NMEFIRE1SND,0,SD_NMEDIESND},
         {SD_DARKMONKSAY1,SD_DARKMONKSEESND,SD_DARKMONKFIRE1SND,0,SD_DARKMONKDIESND},
         {SD_SNAKESAY1,SD_SNAKESEESND,SD_SNAKESPITSND,0,SD_SNAKEDIESND},

			//specials
			{0,SD_EMPLACEMENTSEESND,SD_EMPLACEMENTFIRESND,0,0},
         {0,SD_ROBOTSEESND,SD_ROBOTFIRESND,0,SD_ROBOTDIESND}, //wallop
			{0,0,0,0,0}, //pillar
			{SD_FIREJETSND,0,0,0,0}, //firejet
			{SD_BLADESPINSND,0,0,0,0}, //blade
			{SD_CYLINDERMOVESND,0,0,0,0}, //crushcol
			{SD_BOULDERROLLSND,0,0,SD_BOULDERHITSND,0}, //boulder
			{SD_SPEARSTABSND,0,0,0,0}, //spear
			{0,0,0,0,0}, //gasgrate
			{SD_SPRINGBOARDSND,0,0,0,0}, //spring
			{0,0,0,0,0}, //shuriken
			{SD_FIREBALLSND,0,0,SD_FIREBALLHITSND,0}, //wallfire
			{0,0,0,0,0}, //net
			{SD_KRISTMINEBEEPSND,0,0,0,0}, //h_mine
			{0,0,0,0,0}, //grenade
			{0,0,0,0,0}, //fireball
			{0,0,0,0,0}, //dmfball
			{0,0,0,0,0}, //bigshuriken
			{0,0,0,0,0}, //missile
			{0,0,0,0,0}, //NMEsaucer
			{0,0,0,0,0}, //dm_weapon
			{0,0,0,0,0}, //dm_heatseek
			{0,0,0,0,0}, //dm_spit
         {SD_MISSILEFLYSND,0,SD_BAZOOKAFIRESND,SD_MISSILEHITSND,0},
			{SD_MISSILEFLYSND,0,SD_FIREBOMBFIRESND,SD_MISSILEHITSND,0},
			{SD_MISSILEFLYSND,0,SD_HEATSEEKFIRESND,SD_MISSILEHITSND,0},
			{SD_MISSILEFLYSND,0,SD_DRUNKFIRESND,SD_MISSILEHITSND,0},
			{SD_FLAMEWALLSND,0,SD_FLAMEWALLFIRESND,SD_FIREHITSND,0},
			{SD_MISSILEFLYSND,0,SD_SPLITFIRESND,SD_MISSILEHITSND,0},
			{SD_GRAVSND,0,SD_GRAVFIRESND,SD_GRAVHITSND,0},
			{SD_GRAVSND,0,SD_GODMODEFIRESND,SD_GRAVHITSND,0}
		  };
#endif



//========================== Local Variables ==================================================

extern boolean dopefish;


boolean Masterdisk;

static objtype    *SNAKEHEAD,*SNAKEEND,*PARTICLE_GENERATOR,*EXPLOSIONS;
#if (SHAREWARE == 0)
  static int        OLDTILEX,OLDTILEY;
#endif


static char *debugstr[] = {

		  "inerttype",
		  "player",
		  "lowguard",
		  "highguard",
		  "overpatrol",
		  "strikeguard",
		  "blitzguard",
		  "triadenforcer",
		  "deathmonk",
		  "dfiremonk",
		  "roboguard",
		  "b_darian",
		  "b_heinrich",
		  "b_darkmonk",
		  "b_roboboss",
		  "b_darksnake",
		  "patrolgun",
		  "wallop",
		  "pillar",
		  "firejet",
		  "blade",
		  "crushcol",
		  "boulder",
		  "spear",
		  "gasgrate",
		  "spring",
		  "shuriken",
		  "wallfire",
		  "net",
		  "h_mine",
		  "grenade",
		  "fireball",
		  "dmfball",
		  "bigshuriken",
		  "missile",
		  "NMEsaucer",
		  "dm_weapon",
		  "dm_heatseek",
		  "dm_spit",
		  "p_bazooka",
		  "p_firebomb",
		  "p_heatseek",
		  "p_drunkmissile",
		  "p_firewall",
		  "p_splitmissile",
		  "p_kes",
		  "p_godball",
		  "collectorobj"
		  };







#ifndef RETRO_GO
static int        starthitpoints[4][100] =
                  {{0,0,30,35,50,40,45,425,200,200,100,1500,2500,3000,3000,-1,200,2},
                   {0,0,40,50,55,50,50,475,250,250,125,2300,3400,4500,3600,-1,250,2},
                   {0,0,50,65,60,60,60,525,275,300,150,2400,3600,5000,4500,-1,300,2},
                   {0,0,60,80,70,70,75,525,300,350,175,2800,3800,5900,4800,-1,350,2}};
#else
extern int (*starthitpoints)[100];
#endif


static statobj_t  *touchsprite;


static const byte dirdiff[8][8] = {{0,1,2,3,4,3,2,1},{1,0,1,2,3,4,3,2},
				{2,1,0,1,2,3,4,3},{3,2,1,0,1,2,3,4},
				{4,3,2,1,0,1,2,3},{3,4,3,2,1,0,1,2},
				{2,3,4,3,2,1,0,1},{1,2,3,4,3,2,1,0}};

static const byte dirorder[8][2] = {{southeast,northeast},{east,north},
				 {northeast,northwest},{north,west},
				 {northwest,southwest},{west,south},
				 {southwest,southeast},{south,east}};

#if (SHAREWARE == 0)

static const byte dirdiff16[16][16] = {
				{0,1,2,3,4,5,6,7,8,7,6,5,4,3,2,1},
				{1,0,1,2,3,4,5,6,7,8,7,6,5,4,3,2},
				{2,1,0,1,2,3,4,5,6,7,8,7,6,5,4,3},
				{3,2,1,0,1,2,3,4,5,6,7,8,7,6,5,4},
				{4,3,2,1,0,1,2,3,4,5,6,7,8,7,6,5},
				{5,4,3,2,1,0,1,2,3,4,5,6,7,8,7,6},
				{6,5,4,3,2,1,0,1,2,3,4,5,6,7,8,7},
				{7,6,5,4,3,2,1,0,1,2,3,4,5,6,7,8},
				{8,7,6,5,4,3,2,1,0,1,2,3,4,5,6,7},
				{7,8,7,6,5,4,3,2,1,0,1,2,3,4,5,6},
				{6,7,8,7,6,5,4,3,2,1,0,1,2,3,4,5},
				{5,6,7,8,7,6,5,4,3,2,1,0,1,2,3,4},
				{4,5,6,7,8,7,6,5,4,3,2,1,0,1,2,3},
				{3,4,5,6,7,8,7,6,5,4,3,2,1,0,1,2},
				{2,3,4,5,6,7,8,7,6,5,4,3,2,1,0,1},
				{1,2,3,4,5,6,7,8,7,6,5,4,3,2,1,0}};
#endif

static const byte dirorder16[16][2] = {
						 {15,1}  ,   {0,2},   {1,3},   {2,4},
						  {3,5}  ,   {4,6},   {5,7},   {6,8},
						  {7,9}  ,  {8,10},  {9,11}, {10,12},
						  {11,13}, {12,14}, {13,15},  {14,0}};

//static byte opposite16[16] = {8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7};

#if (SHAREWARE == 0)

static statetype * UPDATE_STATES[NUMSTATES][NUMENEMIES] =

 { {&s_lowgrdstand,&s_highgrdstand,&s_opstand,&s_strikestand,
	 &s_blitzstand,&s_enforcerstand,&s_dmonkstand,&s_firemonkstand,
	 &s_robogrdstand,&s_darianstand,&s_heinrichstand,NULL,
	 &s_darkmonkstand,NULL,&s_gunstand,&s_wallstand},

	{&s_lowgrdpath1,&s_highgrdpath1,&s_oppath1,&s_strikepath1,
	 &s_blitzpath1,&s_enforcerpath1,&s_dmonkpath1,&s_firemonkpath1,
	 &s_robogrdpath1,NULL,NULL,NULL,
	 NULL,NULL,NULL,&s_wallpath},

	{&s_lowgrdcollide,&s_highgrdcollide,&s_opcollide,&s_strikecollide,
	 &s_blitzcollide,&s_enforcercollide,&s_dmonkcollide,&s_firemonkcollide,
	 &s_robogrdcollide,&s_dariancollide,NULL,NULL,
	 NULL,NULL,NULL,&s_wallcollide},

	{&s_lowgrdcollide2,&s_highgrdcollide2,&s_opcollide2,&s_strikecollide2,
	 &s_blitzcollide2,&s_enforcercollide2,&s_dmonkcollide2,&s_firemonkcollide2,
	 &s_robogrdcollide2,&s_dariancollide2,NULL,NULL,
	 NULL,NULL,NULL,&s_wallcollide},

	{&s_lowgrdchase1,&s_highgrdchase1,&s_opchase1,&s_strikechase1,
	 &s_blitzchase1,&s_enforcerchase1,&s_dmonkchase1,&s_firemonkchase1,
	 NULL/*se1*/,&s_darianchase1,&s_heinrichchase,&s_NMEchase,
	 &s_darkmonkchase1,NULL,&s_gunstand,&s_wallpath},

	/*
	{&s_lowgrduse1,&s_highgrduse1,&s_opuse1,&s_strikeuse1,
	 &s_blitzuse,&s_enforceruse1,NULL,NULL,
	 NULL,&s_darianuse1,NULL,NULL,
	 NULL,NULL,NULL,NULL},*/
	 {0},

	{&s_lowgrdshoot1,&s_highgrdshoot1,&s_opshoot1,&s_strikeshoot1,
	 &s_blitzshoot1,&s_enforcershoot1,NULL,&s_firemonkcast1,
    &s_robogrdshoot1,&s_darianshoot1,&s_heinrichshoot1,NULL,
	 NULL,NULL,&s_gunfire1,&s_wallshoot},

	{&s_lowgrddie1,&s_highgrddie1,&s_opdie1,&s_strikedie1,
	 &s_blitzdie1,&s_enforcerdie1,&s_dmonkdie1,&s_firemonkdie1,
	 &s_robogrddie1,&s_dariandie1,&s_heinrichdie1,&s_NMEdie,
	 &s_darkmonkdie1,NULL,&s_gundie1,NULL},

	{0},

	{NULL,NULL,NULL,&s_strikewait,
	 &s_blitzstand,&s_enforcerdie1,&s_dmonkdie1,&s_firemonkdie1,
	 &s_robogrddie1,&s_dariandie1,&s_heinrichdie1,NULL,
	 &s_darkmonkdie1,NULL,NULL,NULL},

	{&s_lowgrdcrushed1,&s_highgrdcrushed1,&s_opcrushed1,&s_strikecrushed1,
	 &s_blitzcrushed1,&s_enforcercrushed1,&s_dmonkcrushed1,&s_firemonkcrushed1,
	 &s_robogrddie1,NULL,NULL,NULL,
	 NULL,NULL,NULL,NULL}

	};

#else

static statetype * UPDATE_STATES[NUMSTATES][NUMENEMIES] =

 { {&s_lowgrdstand,&s_highgrdstand,NULL,&s_strikestand,
    &s_blitzstand,&s_enforcerstand,NULL,NULL,
    &s_robogrdstand,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL},

   {&s_lowgrdpath1,&s_highgrdpath1,NULL,&s_strikepath1,
    &s_blitzpath1,&s_enforcerpath1,NULL,NULL,
	 &s_robogrdpath1,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL},

   {&s_lowgrdcollide,&s_highgrdcollide,NULL,&s_strikecollide,
    &s_blitzcollide,&s_enforcercollide,NULL,NULL,
    NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL},

   {&s_lowgrdcollide2,&s_highgrdcollide2,NULL,&s_strikecollide2,
    &s_blitzcollide2,&s_enforcercollide2,NULL,NULL,
    &s_robogrdcollide2,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL},

   {&s_lowgrdchase1,&s_highgrdchase1,NULL,&s_strikechase1,
    &s_blitzchase1,&s_enforcerchase1,NULL,NULL,
    NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL},

	/*
	{&s_lowgrduse1,&s_highgrduse1,&s_opuse1,&s_strikeuse1,
	 &s_blitzuse,&s_enforceruse1,NULL,NULL,
	 NULL,&s_darianuse1,NULL,NULL,
	 NULL,NULL,NULL,NULL},*/
	 {0},

   {&s_lowgrdshoot1,&s_highgrdshoot1,NULL,&s_strikeshoot1,
    &s_blitzshoot1,&s_enforcershoot1,NULL,NULL,
    &s_robogrdshoot1,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL},

   {&s_lowgrddie1,&s_highgrddie1,NULL,&s_strikedie1,
    &s_blitzdie1,&s_enforcerdie1,NULL,NULL,
    &s_robogrddie1,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL},

	{0},

	{NULL,NULL,NULL,&s_strikewait,
    &s_blitzstand,&s_enforcerdie1,NULL,NULL,
    &s_robogrddie1,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL},

   {&s_lowgrdcrushed1,&s_highgrdcrushed1,NULL,&s_strikecrushed1,
    &s_blitzcrushed1,&s_enforcercrushed1,NULL,NULL,
	 &s_robogrddie1,NULL,NULL,NULL,
	 NULL,NULL,NULL,NULL}

	};

#endif



#define TABLE_ACTOR(ob)  ((ob->obclass >= lowguardobj) && (ob->obclass <= wallopobj))


void T_Reset(objtype*ob);
void ApplyGravity(objtype *ob);
void BeginEnemyHurt(objtype *ob);
void T_PlayDead(objtype *ob);
void SpawnFirewall(objtype*ob,int which,int newz);
void SelectKristChaseDir(objtype*ob);
void ExplodeStatic(statobj_t*tempstat);
void AvoidPlayerMissile(objtype*ob);
int EnvironmentDamage(objtype *ob);

static int     STOPSPEED         =    0x200;
static int     PLAYERFRICTION    =    0xe000;
static int     ACTORFRICTION     =    0xf000;
static int     DIAGADJUST        =    0xb504;
static boolean MissileSound      =    true;




boolean FirstExplosionState(statetype *state)
   {
   if (DoPanicMapping())
      {
      if (state == &s_altexplosion1)
         return true;
      else
         return false;
      }
   else
      {
      if ((state == &s_explosion1) ||
          (state == &s_grexplosion1) ||
          (state == &s_staticexplosion1)
         )
         return true;
      else
         return false;
      }

   }





void SetGibSpeed(int speed)
   {
   MISCVARS->gibspeed = speed;
   }

void ResetGibSpeed(void)
   {
   MISCVARS->gibspeed = NORMALGIBSPEED;
   }

int ValidAreanumber (int areanumber)
{ if ((areanumber >=0) && (areanumber <= NUMAREAS))
	return 1;
  return 0;
}

int GetIndexForState (statetype * state)
{
	int i;

   if (state == NULL)
      return -1;

   for (i=0;i<MAXSTATES;i++)
		{
		if (statetable[i]==state)
			return i;
		}
   Error("Cannot find the state in 'GetIndexForState', state->shapenum = %d\n",state->shapenum);
	return -1;
}



statetype * GetStateForIndex (int index)
{
   if (index == -1)
      return NULL;

   return statetable[index];
}


statobj_t* GetStaticForIndex(int index)
{statobj_t* temp;

 for(temp=FIRSTSTAT;temp;temp=temp->statnext)
  if (index == temp->whichstat)
	 return temp;

 Error("Cannot find the static in 'GetStaticForIndex', statindex %d\n",index);
 return NULL;

}



void SaveActors(byte **buffer,int*size)
{objtype*temp,*tact;
 saved_actor_type dummy;
 byte*tptr;
 int actorcount;


 for(actorcount=0,temp=FIRSTACTOR;temp;temp=temp->next)
    temp->whichactor = actorcount++;



 *size = sizeof(int) + sizeof(numplayers) + sizeof(misc_stuff) + objcount*sizeof(saved_actor_type);
 *buffer = (byte*)SafeMalloc(*size);
 tptr = *buffer;

 memcpy(tptr,MISCVARS,sizeof(misc_stuff));
 tptr += sizeof(misc_stuff);

 memcpy(tptr,&numplayers,sizeof(numplayers));
 tptr += sizeof(numplayers);

 memcpy(tptr,&consoleplayer,sizeof(consoleplayer));
 tptr += sizeof(consoleplayer);

 for(temp=FIRSTACTOR;temp;temp=temp->next)
  {dummy.x = temp->x;
	dummy.y = temp->y;
	dummy.z = temp->z;
	dummy.flags = temp->flags;
	dummy.areanumber = temp->areanumber;
	//dummy.whichactor = temp->whichactor;
	dummy.hitpoints = temp->hitpoints;
	dummy.ticcount = temp->ticcount;
	dummy.obclass = (byte)(temp->obclass);
	dummy.stateindex = GetIndexForState(temp->state);
	dummy.shapeoffset = temp->shapeoffset;
	dummy.dirchoosetime = temp->dirchoosetime;
	dummy.door_to_open = temp->door_to_open;
	dummy.targetx = temp->targettilex;
	dummy.targety = temp->targettiley;
	dummy.dir = (signed char)temp->dir;
	dummy.angle = temp->angle;
	dummy.yzangle = temp->yzangle;
	dummy.speed = temp->speed;
	dummy.momentumx = temp->momentumx;
	dummy.momentumy = temp->momentumy;
	dummy.momentumz = temp->momentumz;

	dummy.temp1 = temp->temp1;
	dummy.temp2 = temp->temp2;
	dummy.temp3 = temp->temp3;
	if (temp->whatever)
	  {/*if ((temp->flags & FL_USE) && (temp!=player))
		  {dummy.whateverindex = (GetIndexForState((statetype*)(temp->whatever))|SG_PSTATE);
			if ((dummy.whateverindex < 0) && (dummy.whateverindex != -1))
			  Error("Bad actor whatever save value of %d\n",dummy.whateverindex);
		  }
		else*/
		  {tact = (objtype*)(temp->whatever);
			if (tact->which == ACTOR)
			  dummy.whateverindex = tact->whichactor;
			else
			  {statobj_t *tstat;

				tstat = (statobj_t*)(temp->whatever);
				dummy.whateverindex = (tstat->whichstat|SG_PSTAT);

			  }
		  }
	  }
	else
	 dummy.whateverindex = -1;


	if (temp->target)
	  {tact = (objtype*)(temp->target);
		if (tact->which == ACTOR)
		  {dummy.targetindex = tact->whichactor;
			Debug("\nsave actor %d, type %d has target %d",temp->whichactor,temp->obclass,tact->whichactor);
		  }
		else if (tact->which == SPRITE)
		  {statobj_t *tstat;

			tstat = (statobj_t*)(temp->target);
			dummy.targetindex = (tstat->whichstat|SG_PSTAT);
		  }
      else // It must be a push wall, and we don't save that
        dummy.targetindex=-1;
	  }
	else
	  dummy.targetindex = -1;


	memcpy(tptr,&(dummy.x),sizeof(saved_actor_type));
	tptr += sizeof(saved_actor_type);

  }

}



void LoadActors(byte *buffer,int size)
   {
   int numactors,i,playerindex;
   saved_actor_type dummy;
   objtype *temp;
   short *targetindices,*whateverindices;

   InitActorList();

   memcpy(MISCVARS,buffer,sizeof(misc_stuff));
   buffer += sizeof(misc_stuff);

   memcpy(&numplayers,buffer,sizeof(numplayers));
   buffer += sizeof(numplayers);

   memcpy(&playerindex,buffer,sizeof(playerindex));
   buffer += sizeof(playerindex);

   size -= (sizeof(misc_stuff)+sizeof(numplayers)+sizeof(playerindex));
   numactors = size/sizeof(saved_actor_type);


   objlist = (objtype**)SafeMalloc(numactors*sizeof(objtype*));
   targetindices = (short*)SafeMalloc(numactors*sizeof(short));
   whateverindices = (short*)SafeMalloc(numactors*sizeof(short));

   for(i=0;i<numactors;i++)
      {
      targetindices[i] = 0;
      whateverindices[i] = 0;
      objlist[i] = NULL;
      }


   for(i=0;i<numactors;i++)
      {
      GetNewActor();
      objlist[i] = new;
      if (i < numplayers)
         {
         PLAYER[i]=new;
         if (i==playerindex)
            player=new;
         }

      memcpy(&(dummy.x),buffer,sizeof(saved_actor_type));

      //new->x = dummy.x;
      //new->y = dummy.y;
      SetFinePosition(new,dummy.x,dummy.y);
      SetVisiblePosition(new,dummy.x,dummy.y);
      new->z = dummy.z;
      new->flags = dummy.flags;
      new->hitpoints = dummy.hitpoints;
      new->ticcount = dummy.ticcount;
      new->shapeoffset = dummy.shapeoffset;
      new->obclass = (classtype)(dummy.obclass);


      new->state = GetStateForIndex(dummy.stateindex);
      if (new->state == &s_superparticles)
         PARTICLE_GENERATOR = new;
      else if
         (new->state->think == T_SlideDownScreen)
         SCREENEYE = new;
      new->dirchoosetime = dummy.dirchoosetime;
      new->door_to_open = dummy.door_to_open;
      new->targettilex = dummy.targetx;
      new->targettiley = dummy.targety;
      new->dir = (dirtype)(dummy.dir);
      new->angle = dummy.angle;
      new->yzangle = dummy.yzangle;
      new->speed = dummy.speed;
      new->momentumx = dummy.momentumx;
      new->momentumy = dummy.momentumy;
      new->momentumz = dummy.momentumz;
      new->temp1 = dummy.temp1;
      new->temp2 = dummy.temp2;
      new->temp3 = dummy.temp3;
      if (dummy.whateverindex == -1)
         new->whatever = NULL;

      else if (dummy.whateverindex & SG_PSTAT)
         new->whatever = GetStaticForIndex(dummy.whateverindex & ~SG_PSTAT);
      else
         whateverindices[i] = dummy.whateverindex+1;


      if (dummy.targetindex == -1)
         new->target = NULL;
      else if (dummy.targetindex & SG_PSTAT)
         new->target = GetStaticForIndex(dummy.targetindex & ~SG_PSTAT);
      else
         {
         targetindices[i] = dummy.targetindex+1;
         Debug("\nload actor %d, type %d has target %d",i,new->obclass,dummy.targetindex);
         }


      new->areanumber = dummy.areanumber;
      new->shapenum = new->state->shapenum + new->shapeoffset;
      new->which = ACTOR;
      if (new->flags & FL_ABP)
         MakeActive(new);
      if (new->obclass != inertobj)
         MakeLastInArea(new);

      if (!(new->flags & (FL_NEVERMARK|FL_NONMARK)))
         actorat[new->tilex][new->tiley] = new;

      PreCacheActor(new->obclass,-1);
      buffer += sizeof(saved_actor_type);
      }


   // find unique links between actors,
   // searching list AFTER all have been spawned

   for(i=0;i<numactors;i++)
      {temp=objlist[i];
      if (whateverindices[i])
         temp->whatever = objlist[whateverindices[i]-1];
      if (targetindices[i])
         temp->target = objlist[targetindices[i]-1];
      }


   for(temp=FIRSTACTOR;temp;temp=temp->next)
      {if (temp->obclass == b_darksnakeobj)
         {if (!SNAKEHEAD)
            SNAKEHEAD = temp;
         else if (!temp->whatever)
            SNAKEEND = temp;
         }

      }

   if (SNAKEHEAD)
      for(temp=FIRSTACTOR;temp;temp=temp->next)
         {if (temp->state == &s_megaexplosions)
            EXPLOSIONS = temp;
         }

   //SafeFree(objlist);
   SafeFree(targetindices);
   SafeFree(whateverindices);

   }



int RandomSign(void)
   {
   if (GameRandomNumber("random sign",0) < 128)
      return -1;
   return 1;


   }


void AddToFreeList(objtype*ob)
{ if (!FIRSTFREE)
	 FIRSTFREE = ob;
  else
	 {ob->prev = LASTFREE;
	  LASTFREE->next = ob;
	 }
  LASTFREE = ob;

}

void RemoveFromFreeList(objtype*ob)
{
  if (ob == LASTFREE)
		LASTFREE = ob->prev;
  else
		ob->next->prev = ob->prev;

  if (ob == FIRSTFREE)
		FIRSTFREE = ob->next;
  else
		ob->prev->next = ob->next;

  ob->prev = NULL;
  ob->next = NULL;

}


void MakeActive(objtype *ob)
 {if ((ob == firstactive) || (ob->prevactive) || (ob->nextactive))
	 {
	 SoftError("\ndouble make active try");
	 //AddEndGameCommand ();
	 return;
	 }

  if (!firstactive)
	 firstactive = ob;
  else
	 {ob->prevactive = lastactive;
	  lastactive->nextactive = ob;
	 }
  lastactive = ob;

  #if ((DEVELOPMENT == 1))
  #if ((LOADSAVETEST == 1))
  if (!lastactive)
	Debug("\nlastactive = NULL !");
  else
	Debug("\nlastactive = %8x",lastactive);

  #endif
  #endif
 }



void MakeLastInArea(objtype *ob)
 {
  if (!ValidAreanumber(ob->areanumber))
     Error("\n ob type %s at %d,%d has illegal areanumber of %d",
           debugstr[ob->obclass],ob->tilex,ob->tiley,ob->areanumber);


  if ((ob == firstareaactor[ob->areanumber]) || (ob->previnarea) || (ob->nextinarea))
	 {
	 SoftError("\ndouble make last in area try");
	 //AddEndGameCommand ();
	 return;
	 }
  if (!firstareaactor[ob->areanumber])
	 firstareaactor[ob->areanumber]	= ob;
  else
	  {ob->previnarea = lastareaactor[ob->areanumber];
		lastareaactor[ob->areanumber]->nextinarea = ob;
	  }
  lastareaactor[ob->areanumber] = ob;
 }



void RemoveFromArea(objtype*ob)
 {
  if (!ValidAreanumber(ob->areanumber)) return;

  if (!((ob == firstareaactor[ob->areanumber]) || (ob->previnarea) || (ob->nextinarea)))
	 {
	 SoftError("\ndouble remove from area try");
	 //AddEndGameCommand ();
	 return;
	 }

  if (ob == lastareaactor[ob->areanumber])     // remove from master list
		lastareaactor[ob->areanumber] = ob->previnarea;
  else
		ob->nextinarea->previnarea = ob->previnarea;

  if (ob == firstareaactor[ob->areanumber])
		firstareaactor[ob->areanumber] = ob->nextinarea;
  else
		ob->previnarea->nextinarea = ob->nextinarea;

  ob->previnarea = NULL;
  ob->nextinarea = NULL;
 }


void MakeInactive(objtype*ob)
{
  if (!ACTIVE(ob))
//  if (!((ob == firstactive) || (ob->prevactive) || (ob->nextactive)))
	 {
	 SoftError("\n trying to remove inactive object");
	 //AddEndGameCommand ();
	 return;
	 }

  //if (ob->flags & FL_ABP)
	  {

		if (ob == lastactive)     // remove from master list
			lastactive = ob->prevactive;
		else
			ob->nextactive->prevactive = ob->prevactive;

		if (ob == firstactive)
			firstactive = ob->nextactive;
		else
			ob->prevactive->nextactive = ob->nextactive;


		ob->prevactive = NULL;
		ob->nextactive = NULL;
	  }

}



void A_Steal(objtype*ob)
   {
   int dx,dy,dz;

   ActorMovement(ob);

   dx = abs(ob->x - PLAYER[0]->x);
   dy = abs(ob->y - PLAYER[0]->y);
   dz = abs(ob->z - PLAYER[0]->z);

   if ((dx > TOUCHDIST) || (dy > TOUCHDIST) || (dz > (TOUCHDIST >> 10)))
      {
      NewState(ob,&s_blitzchase1);
      return;
      }

   if (ob->ticcount)
      return;

   SD_PlaySoundRTP(SD_BLITZSTEALSND,ob->x,ob->y);
   if (PLAYER[0]->flags & FL_GASMASK)
      {
      PLAYER[0]->flags &= ~FL_GASMASK;
      PLAYERSTATE[0].protectiontime = 1;
      ob->temp3 = stat_gasmask;
      GM_UpdateBonus (PLAYERSTATE[0].poweruptime, true);

      }
   else if(PLAYER[0]->flags & FL_BPV)
      {
      PLAYER[0]->flags &= ~FL_BPV;
      PLAYERSTATE[0].protectiontime = 1;
      ob->temp3 = stat_bulletproof;
      GM_UpdateBonus (PLAYERSTATE[0].poweruptime, true);

      }
   else if(PLAYER[0]->flags & FL_AV)
      {
      PLAYER[0]->flags &= ~FL_AV;
      PLAYERSTATE[0].protectiontime = 1;
      ob->temp3 = stat_asbesto;
      GM_UpdateBonus (PLAYERSTATE[0].poweruptime, true);

      }
   else if (PLAYERSTATE[0].missileweapon != -1)
      {
      NewState(PLAYER[0],&s_player);
      PLAYERSTATE[0].attackframe = PLAYERSTATE[0].weaponframe = 0;
      PLAYERSTATE[0].new_weapon = PLAYERSTATE[0].bulletweapon;
      ob->temp3 = GetItemForWeapon(PLAYERSTATE[0].missileweapon);
      ob->temp2 = PLAYERSTATE[0].ammo;
      //ob->temp1 = oldpolltime;
      PLAYERSTATE[0].ammo = -1;

      if (PLAYERSTATE[0].weapon == PLAYERSTATE[0].missileweapon)
         PLAYERSTATE[0].weapondowntics = WEAPONS[PLAYERSTATE[0].weapon].screenheight/GMOVE;
      PLAYERSTATE[0].missileweapon = -1;

      if ( SHOW_BOTTOM_STATUS_BAR() )
         DrawBarAmmo (false);

      }
   }


void FindAddresses(void)
{
 int i;
 unsigned long tstate,tfunct;

 MINFUNCTION = -1l;
 MAXFUNCTION = 0x00000000;
 MINSTATE = -1l;
 MAXSTATE = 0x00000000;

 for(i=0;i<MAXSTATES;i++)
   {
   tstate = (unsigned long)(statetable[i]);
   if (tstate < MINSTATE)
      MINSTATE = tstate;

   if (tstate > MAXSTATE)
      MAXSTATE = tstate;
   if (statetable[i]!=NULL)
      {
      tfunct = (unsigned long)(statetable[i]->think);
      if (tfunct < MINFUNCTION)
         MINFUNCTION = tfunct;

      if (tfunct > MAXFUNCTION)
         MAXFUNCTION = tfunct;
      }
   }
}

void CheckBounds(objtype*ob)
{
 unsigned long tstate,tfunct;

  tstate = (unsigned long)(ob->state);
  tfunct = (unsigned long)(ob->state->think);

 if ((tfunct < MINFUNCTION) || (tfunct > MAXFUNCTION) ||
	  (tstate < MINSTATE) || (tstate > MAXSTATE))
	{
	 if (tfunct < MINFUNCTION)
		Error("%s has thinking function less than MINFUNCTION",debugstr[ob->obclass]);

	 else if (tfunct > MAXFUNCTION)
		Error("%s has thinking function greater than MAXFUNCTION",debugstr[ob->obclass]);

	 if (tstate < MINSTATE)
		Error("%s has state less than MINSTATE",debugstr[ob->obclass]);

	 else if (tstate > MAXSTATE)
		Error("%s has state greater than MAXSTATE",debugstr[ob->obclass]);

	}


}

/*************************************************************/


/*
=====================
=
= DoActor
=
=====================
*/

void DoActor (objtype *ob)
   {
	void (*think)(objtype *);
	int door;


//  for(i=0;i<tics;i++)
//	{

#if (BNACRASHPREVENT == 1)//
		if (ob->state == 0){return;}
#endif
    ApplyGravity(ob);
	 M_CheckDoor(ob);
	 M_CheckBossSounds(ob);
    if ((ob->obclass >= b_darianobj) &&
        (ob->obclass < b_darksnakeobj) &&
        MISCVARS->REDTIME
       )
       {
       MISCVARS->REDTIME --;
       MISCVARS->redindex = (MISCVARS->REDTIME  & 15);
       }

	 if (ob->obclass == playerobj)
		 ControlPlayerObj(ob);

	 think = ob->state->think;
	 if (think)
		 {
		 //CheckBounds(ob);
       think (ob);

		 if (!ob->state)
          {
          RemoveObj (ob);
			 return;
          }
       }

	 if (ob->ticcount)
       ob->ticcount --;

	 else
       {
       if (!(ob->state->next))
          {
          RemoveObj (ob);
			 return;
          }
       else
          NewState(ob,ob->state->next);
       }


    if (ob->flags&FL_NEVERMARK)
       return;

    if ((ob->flags&FL_NONMARK) && actorat[ob->tilex][ob->tiley])
       return;

    actorat[ob->tilex][ob->tiley] = ob;
    }





void ApplyGravity(objtype *ob)
{
   int oldmomentumz;

   if (((ob->momentumz) || (ob->z != nominalheight)) &&
       (ob->obclass > playerobj) &&
       ((ob->obclass <= roboguardobj) || (ob->obclass == collectorobj) ||
        (ob->obclass == b_heinrichobj)) &&
       (ob->state->think != T_Stand)
      )
      {
      ob->z += (ob->momentumz>>16);
      ob->momentumz += GRAVITY;
      if (ob->z >= nominalheight)
         {
         ob->z = nominalheight;
         oldmomentumz = ob->momentumz;
         ob->momentumz = 0;
         if (oldmomentumz > 2*GRAVITY)
            {
            if (ob->flags & FL_DYING)
               SD_PlaySoundRTP(ACTORTHUDSND,ob->x,ob->y);
            else
               {
               int oldviolence = gamestate.violence;

               SD_PlaySoundRTP(ACTORLANDSND,ob->x,ob->y);
               gamestate.violence = vl_low;
               BeginEnemyHurt(ob);
               gamestate.violence = oldviolence;
               }
            }
         if (ob->flags&FL_FALLINGOBJECT)
            {
            RemoveObj(ob);
            return;
            }
         }
      }
}

/*
===================
=
= NewState
=
= Changes ob to a new state, setting ticcount to the max for that state
=
===================
*/

void NewState (objtype *ob, statetype *newstate)
{
   if (DoPanicMapping() &&
       ((newstate == &s_explosion1) ||
        (newstate == &s_grexplosion1) ||
        (newstate == &s_staticexplosion1)
       )
      )
      ob->state = &s_altexplosion1;
   else{
#if (BNACRASHPREVENT == 1)//crashed here when oscuro and larves were all killed
		if (ob == 0){return;}
#endif
      ob->state = newstate;
   }
   SetVisiblePosition(ob,ob->x,ob->y);
#if (BNACRASHPREVENT == 1)
		if (ob->state == 0){return;}
#endif

	ob->ticcount = (ob->state->tictime>>1);
	ob->shapenum = ob->state->shapenum + ob->shapeoffset;

}


/*
=========================
=
= InitActorList
=
= Call to clear out the actor object lists returning them all to the free
= list.  Allocates a special spot for the player.
=
=========================
*/



void InitActorList (void)
{
	//====== NETWORK STUFF =======================================
	memset(&DEADPLAYER[0],0,MAXDEAD * sizeof(statobj_t *));
	NUMDEAD = 0;



	//======= NULLIFY GLOBAL POINTERS ============================

	LASTACTOR=FIRSTACTOR=NULL;
	FIRSTFREE = LASTFREE = NULL;
	firstactive = lastactive = NULL;
	memset(firstareaactor,0,(NUMAREAS + 1) * sizeof(objtype *));
	memset(lastareaactor,0,(NUMAREAS + 1) * sizeof(objtype *));
   NUMSPAWNLOCATIONS = 0;


	PARTICLE_GENERATOR = NULL;
	EXPLOSIONS = NULL;
	SNAKEEND=SNAKEHEAD=NULL;
   SCREENEYE = NULL;
   PLAYER0MISSILE = NULL;

	//============================================================

	objcount = 0;
	memset(MISCVARS,0,sizeof(misc_stuff));
   MISCVARS->gibgravity = -1;
   MISCVARS->gibspeed = NORMALGIBSPEED;

   memset(&RANDOMACTORTYPE[0],0,sizeof(RANDOMACTORTYPE));
   FindAddresses();
   MissileSound = true;
   Masterdisk = false;

}

//===========================================================================

/*
=========================
=
= GetNewActor
=
= Sets the global variable new to point to a free spot in objlist.
= The free spot is inserted at the end of the liked list
=
= When the object list is full, the caller can either have it bomb out ot
= return a dummy object pointer that will never get used
=
=========================
*/

void GetNewActor (void)
   {
   objtype *temp;

   if (!FIRSTFREE)
      {
      temp = (objtype*)Z_LevelMalloc(sizeof(objtype),PU_LEVELSTRUCT,NULL);
      //SoftError("\nMalloc-ing actor");
      //if (insetupgame)
      //  SoftError("in setup");
      }

   else
      {
      temp = LASTFREE;
      //SoftError("\nfree actor available");
      RemoveFromFreeList(LASTFREE);
      }

   if (temp)
      {
      new = temp;
      memset(new,0,sizeof(*new));

      if (FIRSTACTOR)
         {
         new->prev = LASTACTOR;
         LASTACTOR->next = new;
         }
      else
         FIRSTACTOR = new;
      LASTACTOR = new;

      new->door_to_open = -1;
      new->soundhandle = -1;
      objcount ++;
      }
   else
      Error("Z_LevelMalloc failed in GetNewActor");
   }




//===========================================================================




/*
=========================
=
= RemoveObj
=
= Add the given object back into the free list, and unlink it from it's
= neighbors
=
=========================
*/


void RemoveObj (objtype *gone)
{
	if (gone == PLAYER[0])
		Error ("RemoveObj: Tried to remove the player!");

	gone->state=NULL;

	MakeInactive(gone);

	if (gone->obclass!=inertobj) {
	    if (ValidAreanumber(gone->areanumber))
		 RemoveFromArea(gone);
	  else
		 Error("tried to remove an instance of %s with invalid areanumber %d",debugstr[gone->obclass],gone->areanumber);
	}

	if (gone == LASTACTOR)
	  LASTACTOR = gone->prev;
	else
	  gone->next->prev = gone->prev;

	if (gone == FIRSTACTOR)
	  FIRSTACTOR = gone->next;
	else
	  gone->prev->next = gone->next;

	if (gone == EXPLOSIONS)
	 EXPLOSIONS = NULL;
	gone->next = NULL;
	gone->prev = NULL;
//	SoftError("\nremoving instance of %s",debugstr[gone->obclass]);
	if (actorat[gone->tilex][gone->tiley] == (void*)gone)
	 actorat[gone->tilex][gone->tiley] = NULL;

	gone->flags |= FL_NEVERMARK;

   if (gone->flags & FL_TARGET)
      UnTargetActor(gone);

	//Add_To_Delete_Array(gone);
	//Z_Free(gone);
	AddToFreeList(gone);

	objcount--;
}


//============== World Physics Model Functions =========================


void ParseMomentum(objtype *ob,int angle)
   {
   ob->momentumx += FixedMul(ob->speed,costable[angle]);
   ob->momentumy -= FixedMul(ob->speed,sintable[angle]);
   }

void Set_3D_Momenta(objtype *ob, int speed, int theta, int phi)
   {
   int _2Ddiag;

   ob->momentumz = -FixedMul(speed,sintable[phi]);
   _2Ddiag = FixedMul(speed,costable[phi]);
   ob->momentumx = FixedMul(_2Ddiag,costable[theta]);
   ob->momentumy = -FixedMul(_2Ddiag,sintable[theta]);


   }


int AngleBetween(objtype *source,objtype*target)
   {
   int dx,dy;

   dx = target->x - source->x;
   dy = source->y - target->y;
   return (atan2_appx(dx,dy));
   }


void GetMomenta(objtype *target, objtype *source, int *newmomx,
                int *newmomy, int *newmomz, int magnitude
               )
   {
   int angle,dx,dy,dz,yzangle,xydist,_2Ddiag;

   dx = target->x - source->x;
   dy = source->y - target->y;
   dz = source->z - target->z;
   xydist = FindDistance(dx,dy);
   angle = atan2_appx(dx,dy);
   yzangle = atan2_appx(xydist,(dz<<10));
   _2Ddiag = FixedMul(magnitude,costable[yzangle]);

   *newmomz = -FixedMul(magnitude,sintable[yzangle]);
   *newmomx = FixedMul(_2Ddiag,costable[angle]);
   *newmomy = -FixedMul(_2Ddiag,sintable[angle]);
   }

//=======================================================================





void SpawnNewObj (unsigned tilex, unsigned tiley, statetype *state, classtype which)
   {
   int newarea;

   GetNewActor ();
	new->obclass = which;
   SetTilePosition(new,tilex,tiley);
   SetVisiblePosition(new,new->x,new->y);
	new->dir = nodir;
	new->which = ACTOR;
   if (FirstExplosionState(state))
      new->flags |= (FL_NEVERMARK|FL_NOFRICTION);

   if ((which != inertobj) && (which != diskobj))
      actorat[tilex][tiley] = new;

   newarea = AREANUMBER(tilex,tiley);
   if ((which <= springobj) && (which != inertobj))
      {
      if (ValidAreanumber(newarea))
         new->areanumber = newarea;
      else
         Error("illegal initial areanumber of %d for actor type %s"
               "trying to spawn at %d, %d",newarea,debugstr[which],tilex,tiley);
      }
   else
      new->areanumber = newarea;

   if ((which != inertobj) && (!Masterdisk))
      MakeLastInArea(new);
	NewState(new,state);
	new->z = nominalheight;
	if (which==springobj)
		new->z+=2;

   }


//====================================================================

void ConsiderAlternateActor(objtype *ob,classtype which)
   {
   if (((which >= lowguardobj) && (which <= blitzguardobj)) ||
		 (which == dfiremonkobj))
		{if (GameRandomNumber("SpawnStand",which) < 128)
			{switch(which)
				{case lowguardobj:
               ob->shapeoffset =  W_GetNumForName("MARSHOO1") -
                                  W_GetNumForName("LWGSHOO1");
					break;
				 case highguardobj:
               ob->shapeoffset =  W_GetNumForName("HIGSHOO1") -
                                  W_GetNumForName("HG2SHOO1");
					break;
				 case overpatrolobj:
               ob->shapeoffset =  W_GetNumForName("PATSHOO1") -
                                  W_GetNumForName("OBPSHOO1");
					break;
				 case strikeguardobj:
               ob->shapeoffset =  W_GetNumForName("XYGSHOO1") -
                                  W_GetNumForName("ANGSHOO1");
					break;
				 /*case blitzguardobj:
					altstartlabel = "WIGSHOO1";
					new->shapeoffset = 80;
					break;*/
				 case dfiremonkobj:
               ob->shapeoffset =  W_GetNumForName("MRKKSH1") -
                                  W_GetNumForName("ALLKSH1");
					break;
				default:
				    ;
				}
			}

		}

   //if (new->shapeoffset)
   //  {if (W_CheckNumForName(altstartlabel) == -1)
   //     new->shapeoffset = 0;
   //  }

   }





/*
===================
=
= StandardEnemyInit
=
===================
*/

void StandardEnemyInit(objtype *ob,int dir)
   {
   int zoffset;

   if ((ob->obclass == deathmonkobj) || (ob->obclass == dfiremonkobj))
      ob->temp2 = DRAINTIME;

   else if ((ob->obclass == highguardobj) || (ob->obclass == triadenforcerobj))
      ob->flags |= FL_HASAUTO;

   ob->hitpoints = starthitpoints[gamestate.difficulty][ob->obclass];
   ob->dir = dir*2;
   ob->flags |= (FL_SHOOTABLE|FL_BLOCK);
   ob->speed = ENEMYRUNSPEED;
   ob->dirchoosetime = 0;
   ob->door_to_open = -1;

   zoffset = MAPSPOT(ob->tilex,ob->tiley,2);
   if ((zoffset&0xff00)==0xb000)
      Set_NewZ_to_MapValue(&(ob->z),zoffset,"standard enemy",ob->tilex,ob->tiley);
   else
      ob->z = PlatformHeight(ob->tilex,ob->tiley);

   }




void ConsiderOutfittingBlitzguard(objtype *ob)
   {
   if ((GameRandomNumber("wiley blitzguard",0) < WILEYBLITZCHANCE) &&
       (gamestate.difficulty >= gd_medium)
      )
      {
      ob->temp3 = stat_bazooka;
      ob->temp2 = 3;
      }
   }



/*
===============
=
= SpawnStand
=
===============
*/


void SpawnStand (classtype which, int tilex, int tiley, int dir, int ambush)
{statetype *temp;

 #if (SHAREWARE == 1)
   switch(which)
      {
       case overpatrolobj:
       case wallopobj:
       case deathmonkobj:
       case dfiremonkobj:
       case b_darianobj:
       case b_heinrichobj:
       case b_darkmonkobj:
         Error("\n%s actor at %d,%d not allowed in shareware !",debugstr[which],tilex,tiley);
         break;
      default:
	  ;
      }



 #endif

 if ((which == lowguardobj) && (GameRandomNumber("SpawnStand",which) < 128))
    which = blitzguardobj;


 if ((temp = UPDATE_STATES[STAND][which-lowguardobj]) != NULL)
   {
   SpawnNewObj(tilex,tiley,temp,which);
   if (!loadedgame)
      gamestate.killtotal++;


   if (ambush)
      new->flags |= FL_AMBUSH;


 #if 0
 #if (SUPERROTT == 1)

   ConsiderAlternateActor(new,which);

 #endif
 #endif

   StandardEnemyInit(new,dir);

   if (which == b_darkmonkobj)
      {
      new->flags |= (FL_NOFRICTION);//|FL_INVULNERABLE);
      new->speed = ENEMYRUNSPEED*2;
      }

   if (which == blitzguardobj)
      ConsiderOutfittingBlitzguard(new);


   if ((new->obclass >= lowguardobj) && (new->obclass <= dfiremonkobj))
      RANDOMACTORTYPE[new->obclass]++;

   if (MAPSPOT(tilex,tiley,2) == 0xdead)
      {
      new->flags |= FL_KEYACTOR;
      MISCVARS->KEYACTORSLEFT++;
      }

   PreCacheActor(which,0);
   }
 //else
   //Error("NULL initialization error");
}





/*
===============
=
= SpawnPatrol
=
===============
*/

void SpawnPatrol (classtype which, int tilex, int tiley, int dir)
{statetype *temp;
 int path=PATH;
#if 0
#if (SUPERROTT == 1)
 char *altstartlabel;
#endif
#endif



 #if (SHAREWARE==1)
   switch(which)
      {
       case overpatrolobj:
       case wallopobj:
       case deathmonkobj:
       case dfiremonkobj:
       case b_darianobj:
       case b_heinrichobj:
       case b_darkmonkobj:
         Error("\n%s actor at %d,%d not allowed in shareware !",debugstr[which],tilex,tiley);
         break;
      default:
	  ;
      }

 #endif

 if ((which == lowguardobj) && (GameRandomNumber("SpawnStand",which) < 128))
    which = blitzguardobj;




 if ((temp= UPDATE_STATES[path][(int)(which-lowguardobj)]) != NULL)
   {
   SpawnNewObj(tilex,tiley,temp,which);

   if (!loadedgame)
      gamestate.killtotal++;


   #if 0
   #if (SUPERROTT == 1)
   ConsiderAlternateActor(new,which);
   #endif
   #endif

   StandardEnemyInit(new,dir);

   if ((which == wallopobj) || (which == roboguardobj))
      {new->flags |= FL_NOFRICTION;
      //new->flags &= ~FL_SHOOTABLE;
      new->dir <<= 1;
      ParseMomentum(new,dirangle16[new->dir]);
      }
   else
      ParseMomentum(new,dirangle8[new->dir]);


   if (which == blitzguardobj)
      ConsiderOutfittingBlitzguard(new);


   if (MAPSPOT(tilex,tiley,2) == 0xdead)
      {new->flags |= FL_KEYACTOR;
      MISCVARS->KEYACTORSLEFT++;
      }

   PreCacheActor(which,0);
   }

}




//==========================================================================



void SpawnDisk(int tilex, int tiley, int type, boolean master)
{int zoffset;



 if (master == true)
   {
   Masterdisk = true;
   SpawnNewObj(tilex,tiley,&s_diskmaster,diskobj);
   Masterdisk = false;
   new->flags |= FL_MASTER;
   new->momentumz = -(DISKMOMZ << 16);
   new->flags |= FL_SYNCED;
   new->flags |= FL_NEVERMARK;
   new->temp1 = 1;
   //RemoveFromArea(new);

   }

 else
   {
   if (!type)
      {
      SpawnNewObj(tilex,tiley,&s_elevdisk,diskobj);
      new->momentumz = -(DISKMOMZ << 16);
      //new->flags |= FL_SYNCED;
      zoffset = MAPSPOT(tilex,tiley,2);
      if ((zoffset&0xff00)==0xb000)
         Set_NewZ_to_MapValue((fixed*)(&(new->temp2)),zoffset,"elev disk",tilex,tiley);
      else
         new->temp2 = 32;
      new->temp1 = 1;
      }
   else
      {
      SpawnNewObj(tilex,tiley,&s_pathdisk,diskobj);
      zoffset = MAPSPOT(tilex,tiley,2);
      if ((zoffset&0xff00)==0xb000)
         Set_NewZ_to_MapValue((fixed*)(&(new->z)),zoffset,"path disk",tilex,tiley);

      new->dir = (type-1) << 1;
      new->speed = 0x1000;

      //ParseMomentum(new,dirangle8[new->dir]);
      }
   actorat[tilex][tiley] = NULL;
   new->flags |= FL_BLOCK;
   new->flags |= (FL_NOFRICTION|FL_ACTIVE|FL_NEVERMARK);
   }
}




objtype* DiskAt(int tilex,int tiley)
{int area;
 objtype *temp;
 statobj_t *tstat;

 area = AREANUMBER(tilex,tiley);
 if (area >= 0 && area < NUMAREAS + 1) {
     for(temp = firstareaactor[area];temp;temp = temp->nextinarea)
       {if ((temp->tilex != tilex) || (temp->tiley != tiley) ||
            (temp->obclass != diskobj))
          continue;
        return temp;
       }
 }


 for(tstat = firstactivestat;tstat;tstat = tstat->nextactive)
    {
    if ((tstat->tilex != tilex) || (tstat->tiley != tiley) ||
       (tstat->itemnumber != stat_disk))
       continue;
    return (objtype*)tstat;
    }


 return NULL;

}


void SetElevatorDiskVariables(objtype *ob,int newz, int newmomentumz,
                              int newtemp1,int newtemp3,int newdirchoose)
   {
   ob->z = newz;
   ob->momentumz = newmomentumz;
   ob->temp1 = newtemp1;
   ob->temp3 = newtemp3;
   ob->dirchoosetime = newdirchoose;
   }


void T_ElevDisk(objtype*ob)
   {
   objtype *temp = (objtype*)(actorat[ob->tilex][ob->tiley]);
   objtype *master;

   if (ob->flags & FL_MASTER)
      goto masterlabel;

   master = (objtype*)(ob->target);
   if (!master)
   Error("disk without master !");


   //SoftError("\n ob->z:%d %s, master z:%d",ob->z,
   //        (ob->flags & FL_SYNCED)?("SYNCED"):("UNSYNCED"),master->z);


   if (M_ISACTOR(temp) && (temp != ob) && (!(temp->flags & FL_DYING)))
      {
      int dz = abs(ob->z - temp->z),
          dx = abs(ob->x - temp->x),
          dy = abs(ob->y - temp->y);

      if ((dx < 0x7000) && (dy < 0x7000) && (dz < 68) && (temp->z > ob->z))
         {
         ob->flags &= ~FL_SYNCED;
         return;
         }
      }

   if (master && (!(ob->flags & FL_SYNCED)))
      {
      int dz;

      dz = abs(master->z - ob->z);
      if ((dz > 0) && (dz < 8))
         {
         SetElevatorDiskVariables(ob,master->z,master->momentumz,master->temp1,
                                    master->temp3,master->dirchoosetime);
         ob->flags |= FL_SYNCED;
         //return;
         }
      return;
      }


 masterlabel:

   if (ob->dirchoosetime)
      {
      ob->dirchoosetime --;
      return;
      }


   if (ob->temp1) // moving
      {
      ob->z += (ob->momentumz >> 16);
      if (ob->momentumz > 0) // down
         {
         if (ob->z >= nominalheight + 40 + DISKMOMZ)
            SetElevatorDiskVariables(ob,ob->z - (ob->momentumz>>16),0,0,0,35);
         }
      else
         {
         if (ob->z < ob->temp2) // temp2 has max height
            SetElevatorDiskVariables(ob,ob->z - (ob->momentumz>>16),0,0,1,35);
         }
      }
   else
      {
      if (ob->temp3)
         ob->momentumz = (DISKMOMZ << 16);
      else
         ob->momentumz = -(DISKMOMZ << 16);
      ob->temp1 = 1;
      }
   }




void SpawnInertActor(int newx,int newy, int newz)
   {
	GetNewActor ();
   MakeActive(new);

   new->obclass = inertobj;
   new->which = ACTOR;
   SetFinePosition(new,newx,newy);
   SetVisiblePosition(new,new->x,new->y);
   new->z = newz;
   new->dir = 0;
   new->speed = 0;
   new->flags = (FL_NEVERMARK|FL_ABP);

   }




#if (SHAREWARE == 0)
void SpawnGroundExplosion(int x, int y, int z)
{
   SpawnInertActor(x,y,z);
   NewState(new,&s_grexplosion1);
   new->temp2 = GameRandomNumber("SpawnGroundExplosion",0)>>2;

}
#endif

void SpawnSlowParticles(int which, int numgibs, int x,int y,int z)
{objtype *prevlast,*temp;
 int tilex,tiley;

 tilex = x>>16;
 tiley = y>>16;

 SpawnNewObj(tilex,tiley,&s_gibs1,inertobj);
 SetFinePosition(new,x,y);
 SetVisiblePosition(new,x,y);
 prevlast = new;
 prevlast->flags |= FL_ABP;
 MakeActive(prevlast);
 SpawnParticles(new,which,numgibs);

 for(temp = prevlast->next;temp;temp=temp->next)
    {temp->z = z;
     temp->momentumx >>= 1;
     temp->momentumy >>= 1;
     temp->momentumz >>= 1;
    }
 RemoveObj(prevlast);

}


void ResolveDoorSpace(int tilex,int tiley)
   {
   statobj_t* tstat,*temp;

   for(tstat = firstactivestat;tstat;)
      {
      temp = tstat->nextactive;

      if (tstat->flags & FL_DEADBODY)
         {
         if ((tstat->tilex == tilex) && (tstat->tiley == tiley))
            {
            if ((tstat->flags & FL_DEADBODY) && (tstat->linked_to != -1))
                DEADPLAYER[tstat->linked_to] = NULL;
            RemoveStatic(tstat);
            if (tstat->flags & FL_DEADBODY)
               SpawnSlowParticles(GUTS,8,tstat->x,tstat->y,tstat->z);
            else
               SpawnSlowParticles(gt_sparks,8,tstat->x,tstat->y,tstat->z);
            SD_PlaySoundRTP(SD_ACTORSQUISHSND,tstat->x,tstat->y);
            }
         }
      tstat = temp;
      }
   }


void SpawnSpear(int tilex,int tiley,int up)
   {
   int count,i;
   statetype *tstate;


   if (BATTLEMODE && (!gamestate.BattleOptions.SpawnDangers))
      return;

   if (!up)
      {
#if (SHAREWARE == 1)
      Error("\ndownspear at %d,%d in shareware!",tilex,tiley);
#else
      SpawnNewObj(tilex,tiley,&s_speardown1,spearobj);
      new->z = 0;
#endif
      }
   else

      SpawnNewObj(tilex,tiley,&s_spearup1,spearobj);

   count = (int)(GameRandomNumber("Spawn Spear",0) % 16);
   for(i=0,tstate = new->state;i<count;i++,tstate=tstate->next);
   NewState(new,tstate);

   PreCacheActor(spearobj,up);
   new->flags |= (FL_ABP);//|FL_INVULNERABLE);
   MakeActive(new);
   }



void SpawnSpring(int tilex,int tiley)
   {
   int iconvalue;

   iconvalue = MAPSPOT(tilex,tiley,2);
   if (iconvalue == 3)
      {
      SpawnNewObj(tilex,tiley,&s_autospring1,springobj);
      new->ticcount = (GameRandomNumber("Spawn Spring",0) % new->ticcount)+1;
      new->temp1 = iconvalue;
      }
   else
      {
      SpawnNewObj(tilex,tiley,&s_spring1,springobj);
      if (iconvalue == 2)
         new->temp1 = iconvalue;
      }

   PreCacheActor(springobj,0);
   new->flags &= ~(FL_SHOOTABLE|FL_BLOCK);
   }




void T_Spring(objtype*ob)
   {
   objtype *temp;
   int op,dx,dy,dz;


   if ((ob->state->condition & SF_DOWN) && (ob->temp1))
      {
      if (ob->ticcount)
         return;
      ob->shapenum++;
      TurnActorIntoSprite(ob);
      return;
      }

   if (ValidAreanumber(ob->areanumber)) {
       for(temp=firstareaactor[ob->areanumber];temp;temp=temp->nextinarea)
      {
      if (temp == ob)
         continue;

      if (temp->obclass >= roboguardobj)
         continue;

      dx = abs(ob->x-temp->x);
      dy = abs(ob->y-temp->y);
      dz = abs(ob->z-temp->z);
      if ((dx > ACTORSIZE+0x2800) || (dy > ACTORSIZE+0x2800) || (dz > 40))
         continue;
      if (!temp->momentumz)
         {
         op = FixedMul(GRAVITY,(temp->z-5)<<16) << 1;
         temp->momentumz = -FixedSqrtHP(op);
         SD_PlaySoundRTP(SD_SPRINGBOARDSND,ob->x,ob->y);
         }

      }
   }
}


void T_Count(objtype*ob)
   {
   int index;
   wall_t* tswitch;
   touchplatetype *temp;
   objtype* tempactor;

   if (ob->dirchoosetime)
      {
      ob->dirchoosetime --;
      if (ob->dirchoosetime>980)
         MISCVARS->gasindex=((1050-ob->dirchoosetime)<<4)/70;
      else if (ob->dirchoosetime<35)
         MISCVARS->gasindex=(ob->dirchoosetime<<4)/35;
      if (ob->temp3)
         {
         ob->temp3 --;
         if (ob->temp3 & 1)
            SD_PlaySoundRTP(SD_GASHISSSND,ob->x,ob->y);
         }
      else
         {
         ob->temp3 = 105;
         if (ValidAreanumber(ob->areanumber)) {
             for(tempactor=firstareaactor[ob->areanumber];tempactor;tempactor=tempactor->nextinarea)
            {
            if (tempactor == ob)
               continue;
            if (!(tempactor->flags & FL_SHOOTABLE))
               continue;
            if (tempactor->obclass != playerobj)
               {
               if ((tempactor->obclass >= lowguardobj) &&
                   (tempactor->obclass <= dfiremonkobj))
                  {
                  int oldviolence = gamestate.violence;

                  gamestate.violence = vl_low;
                  DamageThing(tempactor,EnvironmentDamage(ob));
                  Collision(tempactor,ob,-(tempactor->momentumx),-(tempactor->momentumy));
                  gamestate.violence = oldviolence;

                  }
               }
            else if (!(tempactor->flags & FL_GASMASK))
               {
               DamageThing(tempactor,EnvironmentDamage(ob));
               Collision(tempactor,ob,0,0);
               M_CheckPlayerKilled(tempactor);
               }
            }
          }
      }
   }

   else
      {
      int i;
      playertype *pstate;

      for(i=0;i<numplayers;i++)
         {
         M_LINKSTATE(PLAYER[i],pstate);
         PLAYER[i]->flags &= ~FL_GASMASK;
         pstate->protectiontime = 1;
         }

      NewState(ob,&s_gas1);
      SD_PlaySoundRTP(SD_GASENDSND,ob->x,ob->y);
      ob->flags &= ~FL_ACTIVE;
      MISCVARS->gasindex=0;
      MU_StartSong(song_level);
      MU_RestoreSongPosition();
      MISCVARS->GASON = 0;

      index = touchindices[ob->temp1][ob->temp2]-1;
      TRIGGER[index] = 0;
      for(temp = touchplate[index];temp;temp = temp->nextaction)
         if (temp->action == EnableObject)
            {
            tempactor = (objtype*)(temp->whichobj);
            tempactor->flags &= ~FL_ACTIVE;
            }

      tswitch = (wall_t*)actorat[ob->temp1][ob->temp2];
      /*
      if (tswitch && (tswitch->which != ACTOR))
         {
         tilemap[ob->temp1][ob->temp2]--;
         tswitch->flags &= ~FL_ON;
         }
      */

      }
   }




void SpawnBlade(int tilex, int tiley,int dir,int upordown,int moving)
{int count,i;
 statetype *nstate;

 #if (SHAREWARE == 1)
   if (!upordown)
     Error("\ndown spinblade at %d,%d not allowed in shareware !",tilex,tiley);
   if (moving)
     Error("\nupdown spinblade at %d,%d not allowed in shareware !",tilex,tiley);

 #endif


 if (BATTLEMODE && (!gamestate.BattleOptions.SpawnDangers))
	return;

 if (moving)

  {
#if (SHAREWARE == 0)

  if (upordown)
	  SpawnNewObj(tilex,tiley,&s_spinupblade1,bladeobj);
   else
	  {SpawnNewObj(tilex,tiley,&s_spindownblade1,bladeobj);
		new->z = 0;
	  }
#endif
  }
 else
  {if (upordown)
	  SpawnNewObj(tilex,tiley,&s_upblade1,bladeobj);

#if (SHAREWARE == 0)

   else
	  {SpawnNewObj(tilex,tiley,&s_downblade1,bladeobj);
		new->z = 0;
	  }
#endif
  }


 count = (int)(GameRandomNumber("SpawnBlade",0) % 16);
 for(nstate=new->state,i=0;i<count;nstate = nstate->next,i++);
 NewState(new,nstate);

 new->flags |= (FL_BLOCK);
 new->flags &= ~FL_SHOOTABLE;
 new->dir = dir;
 if (dir != nodir)
  {new->flags |= FL_NOFRICTION;
	new->speed = ENEMYRUNSPEED;

  }
 if (!MAPSPOT(tilex,tiley,2))
	{new->flags |= FL_ACTIVE;
	 ParseMomentum(new,dirangle8[new->dir]);
	}
 PreCacheActor(bladeobj,(moving<<1)+upordown);
}


void SpawnCrushingColumn(int tilex, int tiley, int upordown)
{int i,count;
 statetype * nstate;


 #if (SHAREWARE == 1)
  if (!upordown)
    Error("\ncrush-up column at %d,%d not allowed in shareware!",tilex,tiley);
 #endif


 if (BATTLEMODE && (!gamestate.BattleOptions.SpawnDangers))
	return;
#if (SHAREWARE == 0)
 if (!upordown)
  SpawnNewObj(tilex,tiley,&s_columnupup1,crushcolobj);
 else
#endif
  {SpawnNewObj(tilex,tiley,&s_columndowndown1,crushcolobj);
	new->z = 0;
  }

 count = (int)(GameRandomNumber("SpawnCrushingColumn",0) % 8);
 for(nstate=new->state,i=0;i<count;nstate = nstate->next,i++)
	{if ((!upordown) && (nstate->condition & SF_UP))
		new->temp1 += (((nstate->tictime>>1) + 1)<<2);

	}
 NewState(new,nstate);
 new->flags |= (FL_BLOCK);

 new->flags &= ~FL_SHOOTABLE;
 PreCacheActor(crushcolobj,upordown);
}



void SpawnFirejet(int tilex, int tiley, int dir, int upordown)
   {
   int statecount,i;
   statetype *tstate;


   statecount = (int)(GameRandomNumber("SpawnFirejet",0) % 22);

   if (upordown)
      {
      for(i=0,tstate=&s_firejetup1;i<statecount;i++,tstate=tstate->next);
      SpawnNewObj(tilex,tiley,tstate,firejetobj);
      }
   else
      {
#if (SHAREWARE == 1)
      Error("\ndown firejet at %d,%d not allowed in shareware",tilex,tiley);
#else
      for(i=0,tstate=&s_firejetdown1;i<statecount;i++,tstate=tstate->next);
      SpawnNewObj(tilex,tiley,tstate,firejetobj);
      new->z = 0;
#endif
      }

   PreCacheActor(firejetobj,upordown);

   new->flags &= ~FL_SHOOTABLE;

   if (dir != nodir)
      {
      new->dir = dir*2;
      new->flags |= FL_NOFRICTION;
      new->speed = ENEMYRUNSPEED;
      ParseMomentum(new,dirangle8[new->dir]);
      }
   else
      new->dir = dir;
   }


void SpawnFirebomb(objtype*ob,int damage,int which)
   {
   int i,low,high,doorat;
   wall_t *tempwall;
   doorobj_t*tempdoor;

   if (which == 0)
      {
      low = (ob->dir>>1);
      high = low;
      }
   else
      {
      low = 0;
      high = which-1;

      if ((FindDistance((ob->x-player->x), (ob->y-player->y))<0x120000) &&
          (player->z==nominalheight)
         )
         SHAKETICS = 35;
      }

   for (i=low;i<=high;i++)
      {
      MissileSound = false;
      /*
      if (((which == 0) && ((low == 5) || (low == 6))) ||
          ((which == 6) && ((i==4) || (i==5)))
         )
         {

         if (((which == 0) && (low == 5)) ||
             ((which == 6) && (i == 4))
            )
            {
            newz = ob->z + 64;
            if (newz > maxheight)
               continue;
            SpawnMissile(ob,p_firebombobj,0,0,&s_grexplosion1,0);
            new->z = newz;
            new->dir = 10;


            }
         else
            {
            newz = ob->z - 64;
            if ((sky == 0) && (newz < 0))
               continue;
            SpawnMissile(ob,p_firebombobj,0,0,&s_grexplosion1,0);
            new->z = newz;
            new->dir = 12;

            }


         }
      else */
         {
         SpawnMissile(ob,p_firebombobj,0,dirangle8[2*i],&s_grexplosion1,0x10000);
         new->z = ob->z;
         new->dir = (i<<1);

         }

      MissileSound = true;


      SD_PlaySoundRTP(SD_EXPLODEFLOORSND,ob->x,ob->y);
      new->temp2 = FixedMul(damage,DIAGADJUST);


      tempwall = (wall_t*)actorat[new->tilex][new->tiley];
      doorat= 0;
      if (M_ISDOOR(new->tilex,new->tiley))
         {
         tempdoor = doorobjlist[tilemap[new->tilex][new->tiley]&0x3ff];
         if (tempdoor->position<0x8000)
            doorat = 1;
         }

      if ((tempwall && M_ISWALL(tempwall)) || doorat ||
          (new->tilex <=0) || (new->tilex > MAPSIZE-1) ||
          (new->tiley <=0) || (new->tiley > MAPSIZE-1)
         )
         {
         new->z = ob->z;
         SetFinePosition(new,ob->x,ob->y);
         SetVisiblePosition(new,ob->x,ob->y);
         }
      new->whatever = ob->whatever;
      new->temp3 = ob->temp3 - 1;
      }
   }






void MissileHitActor(objtype *owner, objtype *missile, objtype *victim,
                     int damage, int hitmomx, int hitmomy
                    )
   {
   int tcl = victim->obclass;
   int ocl = missile->obclass;

   if (
         (victim->flags & FL_DYING) || // hey, they're dying already;
         (victim->flags & FL_HEAD)  || // don't hurt overrobot's head, wheels
         (tcl == wallopobj)            || // bcraft is invulnerable
         (tcl == b_darkmonkobj)        || // darkmonk is invulnerable
         (!(victim->flags & FL_SHOOTABLE)) || // don't hurt environment dangers, dead guys
         ((tcl == b_darksnakeobj) &&
         ((SNAKELEVEL != 3) || (!victim->temp3))// return for non-red snake
         )
      )
      return;


   if ((tcl == playerobj) || (tcl == b_heinrichobj))
      victim->target = owner;


   if (tcl == NMEsaucerobj)  // can shoot over's saucer
      {
      NewState(victim,&s_explosion1);
      victim->flags &= ~FL_SHOOTABLE;
      victim->temp2 = damage;
      return;
      }

   else if (tcl == roboguardobj)       // check roboguard
      {
      DamageThing(victim,damage);
      Collision(victim,owner,0,0);
      }

   else if (tcl == collectorobj)
      {
      if (gamestate.SpawnEluder)
         return;

      DamageThing(victim,damage);
      Collision(victim,owner,0,0);
      }

   else if (tcl == patrolgunobj)
      {
      DamageThing(victim,damage);
      if (victim->hitpoints <= 0)
         {
         victim->momentumx = victim->momentumy = victim->momentumz = 0;
         victim->flags |= FL_DYING;
         if (victim->temp1 == -1)  // this is 4-way gun
            NewState(victim,&s_robogrddie1);
#if (SHAREWARE == 0)
         else                         // this is normal
            NewState(victim,&s_gundie1);
#endif
         }
      }

// bosses are "special" ==========================

   else if ((tcl >= b_darianobj) && (tcl < b_darkmonkobj))

      {
      DamageThing(victim,damage);
      if (!(victim->flags & FL_ATTACKMODE))
         FirstSighting (victim);     // put into combat mode
      if (victim->hitpoints <= 0)
         {
         victim->momentumx = victim->momentumy = victim->momentumz = 0;
         victim->flags |= FL_DYING;
         NewState(victim,UPDATE_STATES[DIE][victim->obclass-lowguardobj]);
         switch (victim->obclass)
            {
            case b_darianobj:
               AddMessage("Darian defeated!",MSG_CHEAT);
               break;

            case b_heinrichobj:
               AddMessage("Krist defeated!",MSG_CHEAT);
               break;

            case b_robobossobj:
               AddMessage("NME defeated!",MSG_CHEAT);
               break;
	    default:
		;
            }
         MU_StartSong(song_bossdie);
         }
#if (SHAREWARE == 0)
      else
         {
         MISCVARS->REDTIME = (damage >> 1);
         if (victim->obclass == b_heinrichobj)
            {
            NewState(victim,&s_heinrichdefend);
            if (Near(victim,PLAYER[0],3))
               {
               MISCVARS->HRAMMING = 1;
               MISCVARS->HMINING = 0;
               victim->dirchoosetime = 0;
               }
            else
               {
               MISCVARS->HMINING = 1;
               MISCVARS->HRAMMING = 0;
               victim->dirchoosetime = 5;//10;
               }
            victim->targettilex = victim->targettiley = 0;
            victim->target = NULL;
            }
         }
#endif
      }

#if (SHAREWARE == 0)
   else if ((tcl == b_darksnakeobj) && (victim->temp3)) // red snake
      {
      DamageThing(SNAKEEND,damage);
      if (victim->state->think == T_DarkSnakeChase)
         NewState(victim,&s_redheadhit);
      else
         NewState(victim,&s_redlinkhit);
      victim->temp3 = 0;
      }
#endif

//===============================================
   else // all other actors
      {

      if ((tcl == playerobj) &&
          (victim->flags & FL_AV) &&
          (ocl != p_godballobj)
         )
         {
         playertype *pstate;

         M_LINKSTATE(victim,pstate);
         pstate->protectiontime -= ((damage<<1) + damage);
         if (pstate->protectiontime < 1)
            pstate->protectiontime = 1;
         if (victim==player)
            GM_UpdateBonus (pstate->protectiontime, false);

         return;  // asbestos vest prevents victim damage
         }

      DamageThing(victim,damage);



      if ((tcl < roboguardobj) && (victim->hitpoints <= 0))
         {
         if (ocl != p_godballobj)
            victim->flags |= FL_HBM;
         else
            victim->flags |= (FL_GODSTRUCK | FL_FULLLIGHT);
         }

      if (tcl == playerobj)
         {
         playertype *pstate;

         M_LINKSTATE(victim,pstate);
         if (pstate->health <= 0)
            {
            if (ocl != p_godballobj)
               victim->flags |= FL_HBM;
            else
               victim->flags |= (FL_GODSTRUCK | FL_FULLLIGHT);

            if (M_ISACTOR(owner))
               {
               if (owner->obclass == playerobj)
                  {
                  if (!victim->momentumz)
                     BATTLE_PlayerKilledPlayer(battle_kill_with_missile,owner->dirchoosetime,victim->dirchoosetime);
                  else
                     BATTLE_PlayerKilledPlayer(battle_kill_with_missile_in_air,owner->dirchoosetime,victim->dirchoosetime);
                  }
               else
                  BATTLE_CheckGameStatus(battle_player_killed,missile->dirchoosetime);
               }
            else
                  BATTLE_CheckGameStatus(battle_player_killed,missile->dirchoosetime);
            }
         }



      if ((owner->obclass == playerobj) && (victim->flags & FL_HBM))
         {
            MISCVARS->supergibflag = true;
         //GivePoints(starthitpoints[gamestate.difficulty][victim->obclass]*5);
         }

      Collision(victim,owner,hitmomx,hitmomy);
      MISCVARS->supergibflag = false;
      if ((tcl == blitzguardobj) && (owner->obclass == playerobj))
         victim->flags |= FL_TARGET;

      }

   }



void MissileHit (objtype *ob,void *hitwhat)
   {
   int damage=0, random,tcl=0,ocl,fireweapon=0,sound,hitmomx,hitmomy;
   objtype* tempactor=NULL,*owner;



   if (ob==missobj)
       missobj=NULL;

   if (ob == PLAYER0MISSILE)
      PLAYER0MISSILE = NULL;


   ob->momentumz = 0;
   hitmomx = ob->momentumx;
   hitmomy = ob->momentumy;
   if (ob->soundhandle != -1)
      SD_StopSound(ob->soundhandle);

   ob->flags &= ~FL_SHOOTABLE;
   if (FirstExplosionState(ob->state))
      return;

   /*
   if ((ob->z < -28) || (IsWindow(ob->tilex,ob->tiley)))
      {
      NewState(ob,&s_megaremove);
      return;
      }
   */

   tempactor = (objtype*)hitwhat;
   owner = (objtype*)(ob->whatever);

   random = GameRandomNumber("MissileHit",0);
   ocl = ob->obclass;
   if (tempactor)
      {
      if (tempactor->which == ACTOR)
         tcl = tempactor->obclass;
      else if (tempactor->which == SPRITE)
         tcl = -1;
      }

   if ((!tcl) && (ob->z < -30))
      {
      if (ob->soundhandle != -1)
         SD_StopSound(ob->soundhandle);

      NewState(ob,&s_megaremove);
      return;

      }


   if (((ocl != p_kesobj) && (ocl != p_godballobj)) || (!tcl))
      ZEROMOM;

   if (tcl == b_darianobj)
      MISCVARS->ESAU_SHOOTING  = false;

   switch(ocl)
      {


      case p_bazookaobj:
         NewState(ob,&s_explosion1);
         if (M_ISACTOR(owner) && (owner->obclass == blitzguardobj))
            damage = 30 + (random >> 4);
         else
            damage = 2*((random>>3)+80);
         break;

      case p_heatseekobj:
         NewState(ob,&s_explosion1);
         damage = 2*((random>>3)+50);
         break;


      case p_drunkmissileobj:
         NewState(ob,&s_explosion1);
         damage = ((random>>3)+25);
         break;

      case p_firebombobj:
         NewState(ob,&s_explosion1);
         damage = 2*((random>>3)+90);
         ob->temp3 = 4;
         SpawnFirebomb(ob,damage,4);
         break;

      case p_firewallobj:

         if (tcl == playerobj)
            gamestate.DOGROUNDZEROBONUS = true;
         NewState(ob,&s_explosion1);
         damage = 2*((random>>3)+50);
         break;


      case p_godballobj:
         if ((tcl >= pillarobj) || (!tcl) || ((tcl == -1) && (!(tempactor->flags & FL_SHOOTABLE))))
            NewState(ob,&s_explosion1);
         ob->target = NULL;
         damage = 500;
         break;


      case shurikenobj:
         NewState(ob,&s_explosion1);
         damage = ((random >>3) + 30)>>2;
         break;


      case grenadeobj:
         NewState(ob,&s_explosion1);
         damage = (random >>3) + 20;
         break;

      case fireballobj:
         NewState(ob,&s_explosion1);
         damage = (random >> 3) + 10;
         fireweapon = 1;
         break;

      case missileobj:
         NewState(ob,&s_explosion1);
         if (M_ISACTOR(owner) && (owner->obclass == wallopobj))
            damage = (random >> 5);
         else
            damage = (random >>3) + 30;
         if (tcl && (tcl != b_heinrichobj))
            damage = 3*damage>>3;
         break;

      case wallfireobj:
         if ((!tempactor) ||
             (tempactor->which == ACTOR) ||
             (tempactor->which == SPRITE)
            )
            NewState(ob,&s_explosion1);
         else if (M_ISWALL(tempactor) || (tempactor->which == DOOR))
            NewState(ob,&s_crossdone1);
         damage = EnvironmentDamage(ob);
         fireweapon = 1;
         break;



      case inertobj:
         ob->state = NULL;
         return;
         break;


#if (SHAREWARE == 0)


      case p_splitmissileobj:
         NewState(ob,&s_explosion1);
         damage = 2*((random>>3)+50);
         break;


      case p_kesobj:
         if ((tcl >= pillarobj) ||
             (!tcl) ||
             ((tcl == -1) && (!(tempactor->flags & FL_SHOOTABLE)))
            )
            NewState(ob,&s_explosion1);
         damage = 2*((random>>3)+140);
         break;


      case netobj:
         ob->state=NULL;
         MISCVARS->NET_IN_FLIGHT = false;
         if (tempactor == PLAYER[0])
            {
            if ((tempactor->flags & FL_GODMODE) ||
                (tempactor->flags & FL_DOGMODE) ||
                godmode
               )
               damage = 0;
            else
               {
               damage = (random >>4) + 5;
               PLAYERSTATE[0].NETCAPTURED = -1;
               PLAYERSTATE[0].weapondowntics = WEAPONS[PLAYERSTATE[0].weapon].screenheight/GMOVE;
               NewState(PLAYER[0],&s_player);
               PLAYERSTATE[0].attackframe = PLAYERSTATE[0].weaponframe = 0;
               PLAYERSTATE[0].batblast = 0;
               if (PLAYERSTATE[0].HASKNIFE == false)
                  AddMessage("Wiggle left and right to get out of net!",
                             MSG_GAME);
               }
            }
         break;


      case bigshurikenobj:
         NewState(ob,&s_oshurikenhit1);
         if (owner->obclass == wallopobj)
            damage = (random >> 5);
         else
            damage = 4*((random >>3) + 30)/10;

         break;


      case dm_spitobj:
         NewState(ob,&s_spithit1);
         damage = 30;
         if (gamestate.difficulty == gd_hard)
            damage += 15;
         break;

      case dm_weaponobj:
         damage = 20;
         NewState(ob,&s_explosion1);
         break;

      case dm_heatseekobj:
         damage = 20;
         NewState(ob,&s_explosion1);
         break;

      case dmfballobj:
         NewState(ob,&s_explosion1);
         damage = (random >>3) + 20;
         fireweapon = 1;
         break;

      case h_mineobj:
         NewState(ob,&s_explosion1);
         damage = (random >>3) + 20;
         break;

      case NMEsaucerobj:
         NewState(ob,&s_explosion1);
         damage = 2*((random>>3)+30);
         break;


#endif


    //default:
       //Error("Unknown ob %d called MissileHit",ob->obclass);
      }

  //if (!ob->state)
	 //return;
   if ((sound = BAS[ob->obclass].hit)!=0)
      SD_PlaySoundRTP(sound,ob->x,ob->y);


   if (FirstExplosionState(ob->state))
      SD_PlaySoundRTP(SD_EXPLODESND,ob->x,ob->y);


   if (tcl>0) // actors
      {
      MissileHitActor(owner,ob,tempactor,damage,hitmomx,hitmomy);
      if ((ocl == p_kesobj) && (tcl < roboguardobj))
         {
         tempactor->momentumx = hitmomx; // kes gives wus targets its momentum
         tempactor->momentumy = hitmomy;
         //missile->flags |= FL_NOFRICTION;
         }
      }

   else if (tcl < 0)  // static
      {
      DamageThing(hitwhat,damage);
      if (FirstExplosionState(new->state))
         new->whatever = ob->whatever;
      }


   }




void T_Spears(objtype*ob)
{int dx,dy,dz,i;


 for(i=0;i<numplayers;i++)
   {if (PLAYER[i]->flags & FL_DYING)
     continue;

    dx = abs(PLAYER[i]->x - ob->x);
	 dy = abs(PLAYER[i]->y - ob->y);
	 dz = abs(PLAYER[i]->z - ob->z);


	 if ((!ob->ticcount)&&(ob->state->condition&SF_SOUND) &&
		  areabyplayer[ob->areanumber])
		  SD_PlaySoundRTP(BAS[ob->obclass].operate,ob->x,ob->y);

	 if ((dx < STANDDIST) && (dy < STANDDIST) && (dz < 20))
		{ob->flags &= ~FL_BLOCK;
		 if ((!ob->ticcount) && (ob->state->condition & SF_CRUSH))
			{DamageThing(PLAYER[i],EnvironmentDamage(ob));
          Collision(PLAYER[i],ob,0,0);
			 M_CheckPlayerKilled(PLAYER[i]);
			 return;
			}
		}
	 else
		{if (ob->state->condition & SF_DOWN)
			ob->flags &= ~FL_BLOCK;
		 else
			ob->flags |= FL_BLOCK;
		}
	}
}



void T_CrushUp(objtype*ob)
{int dx, dy,dist,dz,i,playeron;


  if ((!ob->ticcount) && (ob->state->condition & SF_SOUND) &&
		areabyplayer[ob->areanumber])
	SD_PlaySoundRTP(BAS[ob->obclass].operate,ob->x,ob->y);
  dist = ACTORSIZE+0x2000;
  if (ob->state->condition & SF_UP)
	 {ob->temp1 += 4;
 //	  Debug("\ncol momz = 4");
	 }
  else if (ob->state->condition & SF_DOWN)
	 {ob->temp1 -= 4;
 //      Debug("\ncol mom z = -4");
	 }
  else
	 {//ob->momentumz = 0;
  //	  Debug("\ncol mom z = 0");
	 }

  ob->temp2 = maxheight - ob->temp1 + 32;


  playeron = 0;
  for(i=0;i<numplayers;i++)
	 {dx = abs(PLAYER[i]->x - ob->x);
	  dy = abs(PLAYER[i]->y - ob->y);
	  dz = abs(ob->temp2-PLAYER[i]->z);

	  if ((dx < dist) && (dy < dist) && (dz < 65))
		  {ob->flags &= ~FL_BLOCK;
		  //player->temp2 = 0;
			playeron  = 1;
			if ((!ob->ticcount) && (ob->state->condition & SF_CRUSH) &&
             (levelheight<2) && (!(ob->flags & FL_DYING)))
			  {DamageThing(PLAYER[i],EnvironmentDamage(ob));
            if (PLAYER[i]->hitpoints <= 0)
               PLAYER[i]->flags |= FL_HBM;
            Collision(PLAYER[i],ob,0,0);
				M_CheckPlayerKilled(PLAYER[i]);
            //NewState(ob,ob->state); //reset ticcount
				return;
			  }
			if (ob->state->condition & SF_UP)
			  {
				PLAYER[i]->momentumz = -(4<<16);
				if (PLAYER[i]->z < -30)
               PLAYER[i]->z = -30;
			  }
			else if (ob->state->condition & SF_DOWN)
				{PLAYER[i]->momentumz = (4<<16);
				 if (PLAYER[i]->z >= nominalheight)
					 PLAYER[i]->z = nominalheight;
				}
			else
				PLAYER[i]->momentumz = 0;
			PLAYER[i]->whatever = ob;
			ob->whatever = PLAYER[i];
         //PLAYER[i]->flags |= FL_RIDING;

		  }

	}

  //if (!playeron)
	 {if (ob->state->condition & SF_BLOCK)
		 ob->flags |= FL_BLOCK;
	  else
		 ob->flags &= ~FL_BLOCK;

	 }


}


void T_CrushDown(objtype*ob)
{int dx,dy,dz,i,playeron;


  if ((!ob->ticcount) && (ob->state->condition & SF_SOUND)&&
		areabyplayer[ob->areanumber])
	SD_PlaySoundRTP(BAS[ob->obclass].operate,ob->x,ob->y);

  ob->temp2 = ob->z;
  playeron = 0;
  for(i=0;i<numplayers;i++)
	  {dx = abs(PLAYER[i]->x - ob->x);
		dy = abs(PLAYER[i]->y - ob->y);
		dz = abs(PLAYER[i]->z - ob->z);

		 if ((dx < STANDDIST) && (dy < STANDDIST) && (dz < 20))
			 {//PLAYER[i]->temp2 = 0;
			  playeron = 1;
			  ob->flags &= ~FL_BLOCK;
           if ((!ob->ticcount) && (ob->state->condition & SF_CRUSH) &&
               (!(ob->flags & FL_DYING)))
				 {DamageThing(PLAYER[i],EnvironmentDamage(ob));
              if (PLAYER[i]->hitpoints <= 0)
                 PLAYER[i]->flags |= FL_HBM;
              Collision(PLAYER[i],ob,0,0);
				  M_CheckPlayerKilled(PLAYER[i]);
              //NewState(ob,ob->state); //reset ticcount
				  return;
				 }
			  if ((ob->state->condition & SF_DOWN) &&
					((ob->state != &s_columndowndown1) &&
					(ob->state != s_columndowndown1.next)))
				{PLAYER[i]->temp2 = COLUMNCRUSH;
				 PLAYER[i]->whatever = ob;
				}
			 }

	  }
  //if (!playeron)
	 {if (ob->state->condition & SF_BLOCK)
			ob->flags |= FL_BLOCK;
	  else
			ob->flags &= ~FL_BLOCK;
	 }


}


void T_Explosion(objtype* ob)
   {
   int momx,momy,momz;
   int dx,dy,dz;
   int pdamage,dist,blastradius=0x20000,
       fatalradius=0x9000,impulse,damage,
       scalefactor;



   statobj_t* checkstat;
   objtype* check,*owner;

   if (ob->ticcount)
      return;

   damage = EXPLOSION_DAMAGE;
   owner = (objtype*)(ob->whatever);
   if ((ob->temp3) && (ob->obclass == p_firebombobj))
      {
      SpawnFirebomb(ob,damage,0);
      ob->temp3 = 0;
      }
 //================== check surrounding actors ============================//


   for(check = firstactive;check;check=check->nextactive)
      {
      if (check == ob)
         continue;

//	  if (check == owner)
//		continue;

      dx = abs(check->x - ob->x);
      if (dx > blastradius)
         continue;

      dy = abs(ob->y - check->y);
      if (dy > blastradius)
         continue;

      dz = ((abs(ob->z - check->z))<<10);
      if (dz > blastradius)
         continue;


      if (check->flags & FL_HEAD)
         continue;

      if (check->flags & FL_DYING)
         continue;

      if (!(check->flags & FL_SHOOTABLE))
         continue;



      if ((check->obclass >= roboguardobj) && (check->obclass != b_darkmonkobj))
			//(check->obclass <= wallopobj))
         continue;


      if (!CheckLine(ob,check,SIGHT))
         continue;


      if (check->obclass == NMEsaucerobj)
         {
         NewState(check,&s_explosion1);
         check->flags &= ~FL_SHOOTABLE;
         return;
         }
#if 0
      dist = FindDistance(dx,dy);
      scalefactor = (blastradius-dist)>>4;
      if (scalefactor > 0xffff)
         scalefactor = 0xffff;
      pdamage = FixedMul(damage,scalefactor);
#endif
//#if 0
     //magdx = abs(dx);
     //magdy = abs(dy);

      dist = Find_3D_Distance(dx,dy,dz);
      SoftError("\ndist: %x\n",dist);


      //if (dist < 0x10000)
        // dist = 0x10000;

      scalefactor = FixedDiv2(1<<16,FixedMul(dist,dist));
     //scalefactor = FixedDiv2(1<<16,dist);

      if (scalefactor > 0x12000)
         scalefactor = 0x12000;
      pdamage = FixedMul(damage,scalefactor);
      SoftError("\ndamage: %d, scalefactor: %x\n",pdamage,scalefactor);
//#endif
      impulse = FixedMul(EXPLOSION_IMPULSE,scalefactor);
      if (check->obclass == playerobj)
         {
         check->target = owner;
         if (check->flags & FL_AV)
            pdamage = 0;
         }

      if (check->obclass < roboguardobj)
         {
         SoftError("\nhitpoints before: %d",check->hitpoints);
         DamageThing(check,pdamage);
         SoftError("\nhitpoints after: %d",check->hitpoints);
         if ((check->hitpoints <= 0) && (gamestate.violence == vl_excessive) &&
             ((ob->obclass == p_firebombobj) ||
             ((dx < fatalradius) && (dy < fatalradius) && (dz < 20)))
            )
            check->flags |= FL_HBM;

         GetMomenta(check,ob,&momx,&momy,&momz,impulse);
            //Debug("\nhitmomx = %d, hitmomy = %d",momx,momy);

        /*if (M_ISACTOR(owner) &&
            (owner->obclass == playerobj) &&
            (check->hitpoints <=0)
           )
           GivePoints(starthitpoints[gamestate.difficulty][check->obclass]*5);
        */
         Collision(check,owner,momx,momy);
         check->momentumz += (momz<<6);
         if ((check->obclass == playerobj) && (check->flags & FL_DYING) &&
             M_ISACTOR(owner))
            {
            if (owner->obclass == playerobj)
               {
               if (check->z != nominalheight)
                  BATTLE_PlayerKilledPlayer(battle_kill_with_missile_in_air,owner->dirchoosetime,check->dirchoosetime);
               else
                  BATTLE_PlayerKilledPlayer(battle_kill_with_missile,owner->dirchoosetime,check->dirchoosetime);
               }
            else
               BATTLE_CheckGameStatus(battle_player_killed,check->dirchoosetime);
            }

         }
      else
         {
	     if (check->obclass != b_darkmonkobj) {
		 SoftError("non-darkmonk actor %d being helped by explosion",check->obclass);
	     }
         check->hitpoints += pdamage;
         }
      }

 //======================== check surrounding statics ================


	for(checkstat = firstactivestat;checkstat;checkstat=checkstat->nextactive)
      {

      if ((!(checkstat->flags & FL_SHOOTABLE)) && (checkstat->itemnumber != stat_priestporridge))
         continue;

      if ((checkstat->itemnumber >= stat_lifeitem1) &&
          (checkstat->itemnumber <= stat_lifeitem4))
         continue;

      dx = abs(checkstat->x - ob->x);
      dy = abs(checkstat->y - ob->y);
      dz = ((abs(checkstat->z - ob->z))<<10);

      if ((dx < blastradius) && (dy < blastradius) && (dz < blastradius))
         {
         dist = Find_3D_Distance(dx,dy,dz)+0xc00;

         if (dist < 0x10000)
            dist = 0x10000;

         scalefactor = FixedDiv2(1<<16,FixedMul(dist,dist));
         pdamage = FixedMul(damage,scalefactor);

         if (checkstat->itemnumber != stat_priestporridge)
            DamageThing(checkstat,pdamage);
         else if (!(checkstat->flags & FL_ACTIVE))
            {
            checkstat->flags |= FL_ACTIVE;
            checkstat->count = 1;
            //checkstat->numanims = 6;
            SD_PlaySoundRTP(SD_COOKHEALTHSND,ob->x,ob->y);
            }
         }
      }

 //======================== check surrounding walls ================
	{
	int tilexlow,tilexhigh;
	int tileylow,tileyhigh;
	int radius =0x10000;
	int x,y;

	tilexlow = (int)((ob->x-radius) >>TILESHIFT);
	tileylow = (int)((ob->y-radius) >>TILESHIFT);

	tilexhigh = (int)((ob->x+radius) >>TILESHIFT);
	tileyhigh = (int)((ob->y+radius) >>TILESHIFT);

	for (y=tileylow;y<=tileyhigh;y++)
		{
		for (x=tilexlow;x<=tilexhigh;x++)
			{
			if ((tilemap[x][y]&0x8000) && (tilemap[x][y]&0x4000) && (abs(ob->z - nominalheight) < 32))
				{
				maskedwallobj_t * mw;

				mw=maskobjlist[tilemap[x][y]&0x3ff];
				if (mw->flags&MW_SHOOTABLE)
					UpdateMaskedWall(tilemap[x][y]&0x3ff);
				}
			}
		}
	}

}



void SpawnScreenEye(objtype *ob)
   {
   SpawnNewObj(ob->tilex,ob->tiley,&s_eye1,inertobj);
   new->targettiley = 0;
   new->targettilex = GameRandomNumber("eye position",0) + 20;
   SCREENEYE = new;
   //RemoveFromArea(new);
   new->flags |= FL_ABP;
   MakeActive(new);

   }






void SpawnSuperFatalityGibs(objtype *ob,objtype *attacker)
   {
   int crazygibs = (GameRandomNumber("crazy gibs",0) % 6) + 4;
   int i;


   if ((MISCVARS->supergibflag == true) &&
       ((crazygibs == 9) || (ludicrousgibs == true))
      )

      {
      int olddirect = MISCVARS->directgibs;

      MISCVARS->directgibs = false;
      if (ludicrousgibs == false)
         {
         if (attacker == player)
            {
            AddMessage("Ludicrous Gibs!",MSG_GAME);
            if (!(attacker->flags&FL_DOGMODE))
               SD_Play(PlayerSnds[locplayerstate->player]);
            }
         }
      else
         {
         MISCVARS->randgibspeed = true;

#ifdef MEDGIBS
         SpawnParticles(ob,GUTS,150);
#else
         SpawnParticles(ob,GUTS,75);
#endif
         
         MISCVARS->randgibspeed = false;
         }
      SpawnParticles(ob,GUTS,40);
      MISCVARS->directgibs = olddirect;

      }

   for (i=gt_head;i<=crazygibs;i++)
      {

      if (((ob->obclass == dfiremonkobj) || (ob->obclass == deathmonkobj)) &&
          (i == gt_leg)
         )
         SpawnParticles(ob,gt_arm,1);
      else
         SpawnParticles(ob,i,1);
      }


   }





boolean Vicious_Annihilation(objtype *ob, objtype *attacker)
   {
   if ((ob->flags & FL_HBM) && (gamestate.violence >= vl_high))
      {
      ob->shapeoffset = 0;
      ob->flags &= ~FL_FULLLIGHT;
      NewState(ob,(ob->obclass == playerobj)?(&s_remoteguts1):(&s_guts1));
      SD_PlaySoundRTP(SD_ACTORSQUISHSND,ob->x,ob->y);
      if (gamestate.violence == vl_excessive)
         {
         int numgibs;
         objtype *prevlast;

         numgibs = (GameRandomNumber("excessive guts",0) & 7) + 4;
         //SoftError("\nnumgibs = %d,gamestate.difficulty = %d",numgibs,gamestate.difficulty);
         prevlast = LASTACTOR;
         MISCVARS->fulllightgibs = true;
         SpawnParticles(ob,GUTS,numgibs);
         MISCVARS->fulllightgibs = false;
         for(prevlast = prevlast->next;prevlast;prevlast = prevlast->next)
            prevlast->momentumz += (prevlast->momentumz >> 1);

         if ((GameRandomNumber("super gib chance",0) < 100) ||
             (ludicrousgibs == true)
            )
            {
            MISCVARS->directgibs = true;
//MED
            MISCVARS->gibgravity = GRAVITY/2;
//            MISCVARS->gibgravity = GRAVITY*2;           
            MISCVARS->fulllightgibs = true;
            SpawnSuperFatalityGibs(ob,attacker);
            MISCVARS->fulllightgibs = false;
            MISCVARS->gibgravity = -1;
            MISCVARS->directgibs = false;
            }
         }
      return true;

      }


   if (ob->flags & FL_GODSTRUCK)
      {
      ob->shapeoffset = 0;
      ob->flags |= (FL_FULLLIGHT);
      ob->flags &= ~FL_COLORED;
      ob->momentumx = ob->momentumy = ob->momentumz = 0;
      KillActor(ob);
      NewState(ob,&s_vaporized1);
      return true;
      }

   if (ob->flags & FL_SKELETON)
      {
      KillActor(ob);
      ob->shapeoffset = 0;
      ob->flags &= ~FL_COLORED;
      ob->momentumx = ob->momentumy = ob->momentumz = 0;
      NewState(ob,&s_skeleton1);
      SD_PlaySoundRTP(SD_ACTORBURNEDSND,ob->x,ob->y);
      return true;
      }

   return false;

   }


/*
========================
=
= BeginEnemyFatality
=
========================
*/



void BeginEnemyFatality(objtype *ob,objtype *attacker)
   {


   if ((attacker == player) && (ob->obclass < (NUMENEMIES + 2)))
      {
      GivePoints(starthitpoints[gamestate.difficulty][ob->obclass]*5);
      if (timelimitenabled)
         timelimit+=VBLCOUNTER;
      }

   ob->flags |= FL_DYING;
   ob->soundhandle = -1;

#if 0
   if ((ob->obclass == blitzguardobj) &&
       (ob->state->condition & SF_DOWN)
      )

      SD_Play(PlayerSnds[locplayerstate->player]);
#endif

   if (Vicious_Annihilation(ob,attacker))
      return;

   if ((ob->obclass == patrolgunobj) && (ob->temp1 == -1))
      NewState(ob,&s_robogrddie1);
   else if (ob->obclass == collectorobj)
      {
      if ((!M_ISACTOR(attacker)) || (attacker->obclass != playerobj))
         RespawnEluder();
      else
         BATTLE_CheckGameStatus(battle_shot_deluder,attacker->dirchoosetime);

      NewState(ob,&s_explosion1);
      MISCVARS->gibgravity = GRAVITY/2;
      MISCVARS->fulllightgibs = true;
      SpawnParticles(ob,gt_sparks,100);
      MISCVARS->fulllightgibs = false;
      MISCVARS->gibgravity = -1;
      }
   else
      {
      statetype *temp;

      if ((ob->obclass == blitzguardobj) &&
            (ob->state->condition & SF_FAKING)
         )
         {
         NewState(ob,&s_blitzstruggledie1);
         ob->flags &= ~FL_FULLLIGHT;
         }
      else if
         ((ob->obclass == blitzguardobj) &&
            (ob->state->condition & SF_DOWN)
         )
         {
         NewState(ob,&s_blitzplead7);
         ob->flags &= ~FL_FULLLIGHT;
         }

      else if
         ((temp= M_S(DIE)) != NULL)
         {
         if (LOW_VIOLENCE_DEATH_SHOULD_BE_SET(ob))
            SET_DEATH_SHAPEOFFSET(ob);
         NewState(ob,temp);
         ob->flags &= ~FL_FULLLIGHT;
         }
      else
         Error("Null dead state called in Collision, obclass %d",ob->obclass);
      }


   }



/*
========================
=
= BeginPlayerFatality
=
========================
*/




void BeginPlayerFatality(objtype *ob,objtype *attacker)
   {
   playertype *pstate;
   M_LINKSTATE(ob,pstate);


   ob->flags &= ~(FL_ELASTO|FL_GODMODE|FL_DOGMODE|FL_NOFRICTION|FL_RIDING);

   ob->flags |= FL_DYING;
   pstate->weapon = -1;


   if (BATTLEMODE)
      SD_PlaySoundRTP (SD_PLAYERTCDEATHSND+(pstate->player),ob->x,ob->y);

   if (Vicious_Annihilation(ob,attacker) == false)
      {
      if (LOW_VIOLENCE_DEATH_SHOULD_BE_SET(ob))
         SET_DEATH_SHAPEOFFSET(ob);

      NewState(ob,&s_remotedie1);
      ob->flags &= ~FL_FULLLIGHT;
      }



   }



/*
========================
=
= BeginEnemyHurt
=
========================
*/


void BeginEnemyHurt(objtype *ob)
   {
   statetype *temp;

   if ((temp= M_S(COLLIDE1)) != NULL)
      {


      if ((ob->obclass == blitzguardobj) &&
            (ob->state->condition & SF_FAKING)
         )
         {
         ob->temp1 = 1;
         ob->dirchoosetime = 0;
         T_PlayDead(ob);

         }

      else
         {
         if ((ob->obclass == triadenforcerobj) &&
             (GameRandomNumber("george pain chance",0) <
              (50 + (gamestate.difficulty<<6))
             )
            )
            {
            ob->flags &= ~FL_FULLLIGHT;
            return;

            }


         if (LOW_VIOLENCE_PAIN_SHOULD_BE_SET(ob))
            SET_PAIN_SHAPEOFFSET(ob);

         if (GameRandomNumber("Collision",0) < 128)
            NewState(ob,temp);
         else
            NewState(ob,M_S(COLLIDE2));
         }

      ob->flags &= ~FL_FULLLIGHT;
      ob->ticcount = PAINTIME;
      if (ob->obclass == strikeguardobj)
         ob->ticcount >>= 1;
      }
   }


void  Collision(objtype*ob,objtype *attacker,int hitmomentumx,int hitmomentumy)
   {
   int ocl;

   if ((!(ob->flags & FL_SHOOTABLE)) || (ob->flags & FL_DYING))
   return;

   ocl = ob->obclass;

   ob->momentumx += hitmomentumx;
   ob->momentumy += hitmomentumy;

   if ((ocl == playerobj) && (gamestate.battlemode == battle_Eluder))
      return;

   //insertion 5

   if (ocl != playerobj)
      {
      if ((!(ob->flags & FL_ATTACKMODE)) && (TABLE_ACTOR(ob)))
         ActivateEnemy(ob);


      if (ob->hitpoints <= 0)
         BeginEnemyFatality(ob,attacker);

      else if (ocl != roboguardobj)// && (ob->state->think != T_Collide))
         BeginEnemyHurt(ob);
      }
   else
      {
      playertype *pstate;

      if ((ob->flags & FL_GODMODE) || (ob->flags & FL_DOGMODE) || godmode)
         return;

      M_LINKSTATE(ob,pstate);
      if (pstate->health<=0)
         BeginPlayerFatality(ob,attacker);
      else
         ob->flags |= FL_PAIN;

      }

}




void T_BossExplosions(objtype*ob)
{

 if (ob->temp1)
	 {if (ob->dirchoosetime)
		 ob->dirchoosetime --;
	  else
		 {int randtime,randangle,randdist,sound;
		  statetype *nstate;

		  ob->temp1 --;
		  randtime = GameRandomNumber("Boss Explosion Time",0);
		  ob->dirchoosetime = 10;
		  if (randtime < 128)
			ob->dirchoosetime >>= 1;
		  randangle = (GameRandomNumber("Boss Explosion Angle",0) << 3);
		  randdist = (GameRandomNumber("Boss Explosion Distance",0) << 7)+0x4000;
		  sound = SD_EXPLODEFLOORSND;
		  if (randtime < 128)
			 {nstate = &s_explosion1;
			  sound++;
			 }
#if (SHAREWARE == 0)
		  else
			 nstate = &s_grexplosion1;
#endif


		  SpawnMissile(ob,inertobj,0,randangle,nstate,randdist);
		  SD_PlaySoundRTP(sound,new->x,new->y);
		 }
	 }
}




gib_t RandomGutsType(void)
   {
   int rand = GameRandomNumber("gut random",0);


   if (rand < 128)
      return gt_organ;

   //if (rand < 160)
      return gt_rib;

   //return gt_pinkorgan;


   }


//MED
void SpawnParticles(objtype*ob,int which,int numparticles)
   {
   int randphi,randtheta,i,nspeed;
   boolean eyespawned = false;
   int gibtype;
   int randadj;


   if ((ob->z <= -64) && (sky == 0)) //shouldn't happen
      return;


   if (((which == GUTS) || (which == RANDOM)) && (gamestate.violence < vl_high))
      which = gt_sparks;

   gibtype = which;


   for(i=0;i<numparticles;i++)
      {
      int ordertemp;	/* DDOI - Watcom evaluates the mult order diff */
      randphi = (GameRandomNumber("particle generate phi",0) << 3);
      // randadj = RandomSign() * (GameRandomNumber("rand gib adjust",0) >> 4);
      ordertemp = (GameRandomNumber("rand gib adjust",0) >> 4);
      randadj = RandomSign() * ordertemp;



      if (ob->z > (nominalheight - 32))
         randphi &= ((ANGLES/2) - 1);

      randtheta = (GameRandomNumber("particle generate theta",0) << 3);
      nspeed = MISCVARS->gibspeed;


      if (which == RANDOM)
         {
         if (GameRandomNumber("random gib",0) < 128)
            gibtype = RandomGutsType();
         else
            gibtype = gt_sparks;

         }

      if ((which == GUTS) || (which == DISEMBOWEL))
         {

         gibtype = RandomGutsType();
         if (which == DISEMBOWEL)
            {
            randphi>>=2;
            randtheta=ob->temp1+(randtheta>>3)-(randtheta>>4);
            }

         }

      if (lowmemory && (gibtype >= gt_rib) && (gibtype <= gt_limb))
         gibtype = gt_organ;

      if
         (
        // (gibtype >= gt_organ) && (gibtype <= gt_limb) &&
         (MISCVARS->numgibs >= MAXGIBS)
         )
         return;



      if (gibtype == gt_lsoul)
         {
         SpawnNewObj(ob->tilex,ob->tiley,&s_littlesoul,inertobj);
         randphi = 0;
         }


   #if (SHAREWARE == 0)
      else if (gibtype == gt_spit)

         SpawnNewObj(ob->tilex,ob->tiley,&s_slop1,inertobj);
   #endif
      else
         {
         SpawnNewObj(ob->tilex,ob->tiley,&s_gibs1,inertobj);
         new->shapeoffset = gibtype*12;
         NewState(new,new->state);
         }

      if (MISCVARS->directgibs == true)
         {
         int dx,dy,dz;

         dx = PLAYER[0]->x - ob->x;
         dy = ob->y - PLAYER[0]->y;
         randtheta = AngleBetween(ob,PLAYER[0]) +
                     (randadj<<4);
         dz = 100 + (randadj<<3);

#ifdef MEDGIBS
         nspeed = 0x2800;
#else
         nspeed = 0x2800 + (randadj<<7);
#endif

         randphi = atan2_appx(FindDistance(dx,dy),dz<<10);
         }


      if ((eyespawned == false) && (which == GUTS) &&
            (ob->obclass != playerobj)
         )
         {
         eyespawned = true;
         new->flags |= FL_EYEBALL;
         }

      if ((gibtype >= gt_organ) && (gibtype <= gt_limb))
         {
         new->dirchoosetime = GIBVALUE;
         MISCVARS->numgibs ++;
         }
      new->temp2 = gibtype;
      new->temp3 = (MISCVARS->gibgravity == -1)?(GRAVITY):(MISCVARS->gibgravity);

      new->speed = nspeed>>1;

#ifndef MEDGIBS
      if (MISCVARS->randgibspeed == true)
         new->speed += (randadj << 11);
#endif

//      if (ob->state == &s_snakefireworks)
         new->z = ob->z;

      Fix(randphi);
      Fix(randtheta);

      Set_3D_Momenta(new,new->speed,randtheta,randphi);
      new->momentumz <<= 6;
      new->flags |= (FL_NOFRICTION|FL_CRAZY|FL_ABP|FL_NEVERMARK);
      if (MISCVARS->fulllightgibs == true)
         new->flags |= FL_FULLLIGHT;
      new->dir = west;
      new->whatever = ob;
      MakeActive(new);
      }
   }



void T_SlideDownScreen(objtype *ob)
{

 ob->targettiley += 12;
 if (ob->targettiley > 300)
    {
    NewState(ob,&s_megaremove);
    SCREENEYE = NULL;
    }

}

void T_SpawnSoul(objtype*ob)
{
 if (ob->ticcount)
  return;

 SpawnNewObj(ob->tilex,ob->tiley,&s_bigsoul,inertobj);
 new->momentumz = -4000;
 new->flags |= (FL_NOFRICTION|FL_CRAZY|FL_ABP|FL_NEVERMARK);
 new->z = ob->z;
 new->dir = west;
 MakeActive(new);

 SpawnParticles(ob,gt_lsoul,6);


}


void BloodDrip(objtype *ob,int tilex,int tiley)
{int dx,dy,x,y,scale;

 dx = ob->tilex - tilex;
 dy = ob->tiley - tiley;

 if (!dy)
   {
   scale = (ob->momentumx)?(FixedDiv2(ob->momentumy,ob->momentumx)):(0);
   x = (dx < 0)?(tilex << 16):((tilex+1) << 16);
   y = FixedMul(x - ob->x,scale) + ob->y;
   }

 else if (!dx)
   {
   scale = (ob->momentumy)?(FixedDiv2(ob->momentumx,ob->momentumy)):(0);
   y = (dy < 0)?(tiley << 16):((tiley+1) << 16);
   x = FixedMul(y - ob->y,scale) + ob->x;
   }

 ob->temp2 = (GameRandomNumber("BloodDrip",0) << 9) + 0xc000;
 ob->temp1 = (ob->z<<16);
 SetFinePosition(ob,x,y);
 ob->shapeoffset = 0;
 NewState(ob,&s_blooddrip1);

}



void T_BloodFall(objtype*ob)
{
 ob->temp1 += ob->temp2;
 ob->z = (ob->temp1 >> 16);
 if (ob->z >= maxheight)
    {
    ob->shapeoffset = 12;
    MISCVARS->numgibs--;
    NewState(ob,&s_gibsdone1);
    ob->z = nominalheight;
    }

}




void T_Xylophone(objtype*ob)
   {
   if (!ob->ticcount)
      SD_PlaySoundRTP(SD_ACTORSKELETONSND,ob->x,ob->y);
   }



void T_ParticleGenerate(objtype*ob)
{

 if (ob->dirchoosetime)
	ob->dirchoosetime--;
 else
	{
    SetGibSpeed(0x3000);
    SpawnParticles(ob,gt_sparks,(GameRandomNumber("particle count",0) % 10) + 7);
    ResetGibSpeed();
	 ob->dirchoosetime = 10;
	 if (GameRandomNumber("particle generator choose time",0) < 128)
		ob->dirchoosetime >>= 1;
	}

}



void T_Particle(objtype*ob)
{int dx,dy,dz;

 ob->z += (ob->momentumz>>16);

 if ((ob->z >= nominalheight) || (!ob->momentumz))

    {
    if (ob->z >= nominalheight)
       ob->z = nominalheight;
//done:
    if (ob->temp2 == gt_spit)
       ob->state = NULL;
    else
       {
       if (ob->dirchoosetime == GIBVALUE)
          {
          MISCVARS->numgibs--;
          SD_PlaySoundRTP(GIBSOUND,ob->x,ob->y);
          }
       NewState(ob,&s_gibsdone1);
       }
    return;
    }
 else if ((ob->z < -64) && (sky == 0))
    {
    ob->momentumz = 1; //any positive value will do
    ob->z = -64;

    }


 ob->momentumz += ob->temp3;
 ActorMovement(ob);


 if (!BATTLEMODE)
    {
    dx = abs(ob->x - PLAYER[0]->x);
    dy = abs(ob->y - PLAYER[0]->y);
    dz = abs(ob->z - PLAYER[0]->z);

#if (SHAREWARE==0)
    if ((ob->flags & FL_EYEBALL) && (dx < 0x20000) && (dy < 0x20000) &&
        (dz < 64) && (GameRandomNumber("eye chance",0) < 15) &&
        (SCREENEYE == NULL) && (locplayerstate->weapon != wp_dog)
       )
#else
    if ((ob->flags & FL_EYEBALL) && (dx < 0x20000) && (dy < 0x20000) &&
        (dz < 64) && (GameRandomNumber("eye chance",0) < 15) &&
        (SCREENEYE == NULL)
       )
#endif
       SpawnScreenEye(ob);
    }

 //MoveActor(ob);
 //if ((!ob->momentumx) && (!ob->momentumy))
	//goto done;


}



void DropItemInEmptyTile(int item,int tilex,int tiley)
   {
   int stilex = tilex;
   int stiley = tiley;

   FindEmptyTile(&stilex,&stiley);
   SpawnStatic(stilex,stiley,item,9);
   LASTSTAT->flags |= FL_ABP;
   MakeStatActive(LASTSTAT);

   }


void KillActor(objtype*ob)
{int ocl;

 ocl = ob->obclass;


 //GivePoints(starthitpoints[gamestate.difficulty][ob->obclass]*5);
 if ((ocl == highguardobj) &&
	  (GameRandomNumber("Drop mp40 chance",0) < 25))
    {
    DropItemInEmptyTile(stat_mp40,ob->tilex,ob->tiley);
    }

 else if ((ocl == blitzguardobj) && (ob->temp3))
    {
    DropItemInEmptyTile(ob->temp3,ob->tilex,ob->tiley);
    LASTSTAT->ammo = ob->temp2;
    }

  if (actorat[ob->tilex][ob->tiley] == (void*)ob)
	 actorat[ob->tilex][ob->tiley] = NULL;

  gamestate.killcount++;
  ob->flags &= ~FL_SHOOTABLE;
  ob->flags &= ~FL_BLOCK;
  ob->flags |= FL_NEVERMARK;
#if (SHAREWARE == 0)
  if (ocl == b_darksnakeobj)
	 {if (ob == SNAKEHEAD)
		 {SpawnNewObj(ob->tilex,ob->tiley,&s_megaexplosions,inertobj);
		  new->temp1 = 7000;
		  new->flags |= FL_ABP;
		  EXPLOSIONS = new;
		  NewState(ob,&s_darkmonkheaddie1);
		  MakeActive(new);
		  ob->dirchoosetime = 0;

		 }
	  else
		{objtype *temp;

		 SNAKEEND = (objtype*)(ob->target);
		 SNAKEEND->whatever = NULL;
		 NewState(ob,&s_explosion1);
		 for(temp=SNAKEHEAD;temp;temp = (objtype*)(temp->whatever))
			temp->speed += 0x500;
		}
	 }
  else
#endif
	 {ob->whatever = NULL;
     if (ob->obclass!=playerobj)
		  ob->target   = NULL;
	 }

  if ((ob->flags & FL_KEYACTOR) && (ocl!=playerobj) && (ocl != blitzguardobj))
	 {MISCVARS->KEYACTORSLEFT --;
	  if (!MISCVARS->KEYACTORSLEFT)
		 {SpawnNewObj(ob->tilex,ob->tiley,&s_timekeeper,inertobj);
		  new->flags |= FL_ABP;
		  MakeActive(new);
		 }
	 }

}


void T_End(objtype *ob)
{
 if (ob->ticcount)
	return;

 if (MAPSPOT(0,5,2) == LASTLEVELVALUE)
	playstate = ex_gameover;
 else
	playstate = ex_bossdied;

}



void T_Convert(objtype*ob)
   {
   if (ob->ticcount)
      return;

   if (ob->obclass == playerobj)
      {
      if (ob->state == &s_vaporized8)
         {
         T_SpawnSoul(ob);
         NewState(ob,&s_voidwait);
         }
      else if (ob->state == &s_skeleton48)
         {
         playertype *pstate;

         M_LINKSTATE(ob,pstate);
         if ((pstate->falling == true) ||
             (!ob->momentumz)
            )
            NewState(ob,&s_ashwait);
         else
            CheckPlayerSpecials(ob);


         }
      }
   else
      {
      if (ob->state == &s_vaporized8)
         T_SpawnSoul(ob);
      else if (ob->state == &s_skeleton48)
         TurnActorIntoSprite(ob);
      }
   }

void TurnActorIntoSprite(objtype *ob)
{statobj_t*temp;
 objtype *tactor;


 if (!firstemptystat)
   temp = (statobj_t*)Z_LevelMalloc(sizeof(statobj_t),PU_LEVELSTRUCT,NULL);

 else
	{temp = lastemptystat;
	 //SoftError("\nfree actor available");
	 RemoveFromFreeStaticList(lastemptystat);
	}


 if (temp)
  {
   if ((ob->obclass == blitzguardobj) &&
       ((ob->flags & FL_PLEADING) || (ob->flags & FL_UNDEAD))
      )
      MISCVARS->NUMBEGGINGKEVINS = 0;


   if (ob->obclass == roboguardobj)
   {
	  if (ValidAreanumber(ob->areanumber)) {
	      for(tactor=firstareaactor[ob->areanumber];tactor;tactor=tactor->nextinarea)

		  {if (tactor == ob)
			  continue;
			if (tactor->obclass != ob->obclass)
			  continue;

			if (tactor->flags & FL_DYING)
			  continue;
			if (!tactor->state->think)
			  NewState(tactor,UPDATE_STATES[PATH][tactor->obclass-lowguardobj]);

		  }
	  }
   }

   memset(temp,0,sizeof(*temp));
	temp->shapenum = ob->shapenum;
	temp->linked_to = -1;
	temp->whichstat = statcount ++;
   SetFinePosition(temp,ob->x,ob->y);
   temp->areanumber = MAPSPOT(temp->tilex,temp->tiley,0)-AREATILE;
//	if ((temp->areanumbers<=0) || (temp->areanumber>NUMAREAS))
  //		  Error ("Sprite at x=%ld y=%ld type=%ld has an illegal areanumber\n",tilex,tiley,mtype);

	temp->visspot = &spotvis[temp->tilex][temp->tiley];
	temp->which = SPRITE;
	temp->itemnumber = -1;
	temp->flags = FL_DEADBODY;
	if (ob->flags & FL_COLORED)
	  {playertype *pstate;

		M_LINKSTATE(ob,pstate);
		temp->flags |= FL_COLORED;
		temp->hitpoints = pstate->uniformcolor;
	  }
	temp->z = ob->z;
	AddStatic(temp);
//	sprites[temp->tilex][temp->tiley] = temp;

	if (areabyplayer[temp->areanumber])
	 {temp->flags |= FL_ABP;
	  MakeStatActive(temp);
	 }
	if (ob->state != &s_guts12)
	 actorat[ob->tilex][ob->tiley] = temp;
	ob->state = NULL; // say goodbye actor
  }
 else
	Error("Z_LevelMalloc failed in TurnActorIntoSprite!");

}


void T_Blood(objtype*ob)
{
  if (ob->dirchoosetime)
	 {ob->dirchoosetime --;
	  return;
	 }

  ob->dirchoosetime = 35 + (GameRandomNumber("blood time",0) % 20);
  NewState(ob,&s_deadblood1);

}




void ActorDeath(objtype*ob)
   {
#if (SHAREWARE == 0)
   if (ob->obclass == b_heinrichobj)
      {
      KillActor(ob);
      ob->temp1 = ob->dirchoosetime = 5;//10; // init. spin counter for heinrich
      ob->temp3 = 7; //number of times to stay at fast spin
      ob->temp2 = ob->dir; //temp2 holds orig. dir.
      }

   else if (ob->obclass == b_robobossobj)
      {
      objtype *wheels,*head;


      head = (objtype*)(ob->whatever);
      wheels = (objtype*)(ob->target);
      head->flags &= ~(FL_HEAD|FL_SHOOTABLE|FL_BLOCK);
      head->temp2 = 5;
      head->flags |= (FL_NOFRICTION|FL_CRAZY);
//      head->obclass = inertobj;
      //RemoveObj(wheels);   // remove wheels
      KillActor(ob);
      ob->whatever = head;
      ob->target = wheels;
      //ob->temp1 = 25;
      //ob->shapeoffset = 0;
      SpawnNewObj(ob->tilex,ob->tiley,&s_megaexplosions,inertobj);
      new->temp1 = 18;
      new->flags |= FL_ABP;
      MakeActive(new);
      //ob->state = NULL;
      NewState(ob,&s_NMEdeathbuildup);
      }
   else
#endif
      if ((ob->state == ob->state->next) &&
          (ob->flags & FL_DYING)
         )
      {
      KillActor(ob);
      TurnActorIntoSprite(ob);
      if (LASTSTAT->z < nominalheight)
         {
         if ((!IsPlatform(LASTSTAT->tilex,LASTSTAT->tiley)) &&
            (DiskAt(LASTSTAT->tilex,LASTSTAT->tiley) == NULL)
            )
            {
            SpawnParticles(ob,GUTS,10 + gamestate.difficulty);
            RemoveStatic(LASTSTAT);
            }
         }
      /*
      else if ((GameRandomNumber("blood spray",0) < 300) && areabyplayer[ob->areanumber])
         {ob->shapeoffset = 0;
         ob->temp2 = ob->temp3 = 0;
         ob->temp1 = 10;
         NewState(ob,&s_deadblood1);
         return;
         }
         */
      }
   }




void BeginPostPainAction(objtype *ob)
   {

   if ((ob->obclass == strikeguardobj) &&
       (ob->target == (void*)PLAYER[0])
      )
      {//ob->target = NULL;
      if (LOW_VIOLENCE_DEATH_IS_SET(ob))
         RESET_DEATH_SHAPEOFFSET(ob);

      if (GameRandomNumber("T_Collide",0) < 128)
         NewState(ob,&s_strikerollright1);
      else
         NewState(ob,&s_strikerollleft1);

      SelectRollDir(ob);

      if (ob->dirchoosetime)
         {
         SD_PlaySoundRTP(SD_STRIKEROLLSND,ob->x,ob->y);
         return;
         }

      }

   if (LOW_VIOLENCE_PAIN_IS_SET(ob))
      RESET_PAIN_SHAPEOFFSET(ob);

   if (ob->obclass < roboguardobj)
      ob->flags &= ~FL_NOFRICTION;

   if (
      (ob->obclass == blitzguardobj) &&
      (gamestate.violence == vl_excessive) &&
      (GameRandomNumber("blitzplead",0) < 128) &&
      (MISCVARS->NUMBEGGINGKEVINS == 0) &&
      (ob->flags & FL_TARGET) &&

      (ob->hitpoints < (starthitpoints[gamestate.difficulty][ob->obclass] >> 1)) &&
      (ob->momentumz == 0) &&
      (!(ob->flags & FL_UNDEAD))
      )

      {
      NewState(ob,&s_blitzplead1);
      MISCVARS->NUMBEGGINGKEVINS = 1;
      ob->momentumx = ob->momentumy = 0;
      ob->flags |= FL_PLEADING;
      ob->flags &= ~FL_TARGET;
      ob->dirchoosetime = 165;
      ob->hitpoints = 1;
      }

   else
      {
      NewState(ob,M_S(CHASE));
      ob->targettilex = ob->targettiley = 0;
      ob->dirchoosetime = 0;
      }

   }



void T_Collide(objtype*ob)
   {
	if (!(ob->flags & FL_SHOOTABLE))
	 return;

	ActorMovement(ob);

	if (ob->state == NULL)
	  return;

	if (ob->ticcount)
	  return;

	if (ob->hitpoints <= 0)
      {

      if ((ob->soundhandle == -1) &&
          (!ob->ticcount) &&
          (ob->state->next->tictime == 0)
         )
         {
         ob->soundhandle = SD_PlaySoundRTP(ACTORTHUDSND,ob->x,ob->y);

         }

      if (ob->momentumx || ob->momentumy || ob->momentumz)
         return;

      ActorDeath(ob);
      return;
      }

   BeginPostPainAction(ob);

   }



/*
=========================================================================
=
=                Special Blitzguard Functions
=
=========================================================================
*/


/*
=================
=
=   T_Plead
=
=================
*/



void T_Plead(objtype*ob)
{
 int handle;

 ActorMovement(ob);
 if (ob->dirchoosetime)
   {
   if (!(ob->dirchoosetime & 31))
      {
      int random = GameRandomNumber("blitz plead sound",0);
      if (random < 80)
         SD_PlaySoundRTP(SD_BLITZPLEADSND,ob->x,ob->y);
      else if (random < 160)
         SD_PlaySoundRTP(SD_BLITZPLEAD1SND,ob->x,ob->y);
      else
         SD_PlaySoundRTP(SD_BLITZPLEAD2SND,ob->x,ob->y);
      }
   ob->dirchoosetime --;
   return;
	}
 ob->hitpoints = (starthitpoints[gamestate.difficulty][blitzguardobj]>>1);
 //ob->flags |= FL_DYING;
 ob->flags |= FL_UNDEAD;
 SET_DEATH_SHAPEOFFSET(ob);
 NewState(ob,&s_blitzfakedie1);
 ob->flags &= ~FL_PLEADING;
 ob->dirchoosetime = (GameRandomNumber("blitz fake time",0) >> 2) + 70;
 handle = SD_PlaySoundRTP(SD_BLITZOUCHSND,ob->x,ob->y);
 SD_SetSoundPitch (handle,-500);
 ob->temp1 = 0;
 ob->temp1 = (GameRandomNumber("blitz visible rise",0) < 60);

}



/*
=================
=
=   T_ReallyDead
=
=================
*/


void T_ReallyDead(objtype *ob)
   {
   ActorMovement(ob);
   if ((!ob->ticcount) && (LOW_VIOLENCE_DEATH_SHOULD_BE_SET(ob)))
      SET_DEATH_SHAPEOFFSET(ob);


   }



/*
=================
=
=   T_PlayDead
=
=================
*/



void T_PlayDead(objtype *ob)
   {
   int dangle;

   ob->flags &= ~FL_DYING;

   ActorMovement(ob);
   if (ob->dirchoosetime)
      {
      ob->dirchoosetime--;
      return;
      }

   dangle = abs(player->angle - AngleBetween(player,ob));
   if (dangle > ANGLES/2)
      dangle = ANGLES - dangle;

   if (ob->temp1 || (dangle > ANGLES/4))
      {
      if (LOW_VIOLENCE_DEATH_IS_SET(ob))
         RESET_DEATH_SHAPEOFFSET(ob);
      ob->temp1 = 0;

      NewState(ob,&s_blitzrise2);

      }

   }


void AdjustAngle(int maxadjust, short int *currangle,int targetangle)
   {
   int dangle,i,magangle;

   for(i=0;i<maxadjust;i++)
      {
      dangle = *currangle - targetangle;

      if (dangle)
         {
         magangle = abs(dangle);
         if (magangle > (ANGLES/2))
            {
            if (dangle > 0)
               (*currangle) ++;
            else
               (*currangle) --;
            }
         else
            {
            if (dangle > 0)
               (*currangle) --;
            else
               (*currangle) ++;
            }
         Fix(*currangle);
         }
      }
   }


void ResolveMinimumDistance(objtype *heatseeker, objtype *potential_target,
                            int *currmin)
   {
   int currdist,angle,magangle;


   currdist = FindDistance((heatseeker->x-potential_target->x),
                           (heatseeker->y-potential_target->y));
   angle = AngleBetween(heatseeker,potential_target);


   if (heatseeker->obclass != p_godballobj)
      {
      magangle = abs(heatseeker->angle - angle);
      if (magangle > VANG180)
         magangle = ANGLES - magangle;
      if (magangle > ANGLESDIV8)
         return;
      }

   if (currdist > LOOKAHEAD)
      return;

   if (currdist < (*currmin))
      {
      *currmin = currdist;
      heatseeker->target = potential_target;
      }
   }




void HeatSeek(objtype*ob)
{int        xydist;

 int        mindist;
 objtype*   tactor;
 objtype*   owner;
 statobj_t* tstat;
 int        angle,dz,yzangle,adjust;
 int        dx,dy;


 owner=(objtype *)ob->whatever;

 if (ob->dirchoosetime)
    ob->dirchoosetime --;

 else
    {
    if (!ob->target)
        {mindist = 0x7fffffff;

         for (tactor=firstactive;tactor;tactor=tactor->nextactive)
            {
            if (tactor == owner)
               continue;

            if (tactor->flags & FL_HEAD)
               continue;

            if ((tactor == ob) ||
                (!(tactor->flags & FL_SHOOTABLE)) ||
                (tactor->flags & FL_DYING))
               continue;

            if (!CheckLine(ob,tactor,SIGHT))
               continue;

            if ((tactor->obclass == bladeobj) || (tactor->obclass == NMEsaucerobj))
               continue;


            ResolveMinimumDistance(ob,tactor,&mindist);

				}

			if (ob->obclass != p_godballobj)
            {
            for(tstat=firstactivestat;tstat;tstat=tstat->nextactive)
               {
               if (!(tstat->flags & FL_HEAT))
                  continue;

               if (!CheckLine(ob,tstat,SHOOT))
                  continue;

               ResolveMinimumDistance(ob,(objtype*)tstat,&mindist);
               }
            }


			if (!ob->target)
				ob->dirchoosetime = 5;
		  }

	 else //if (ob->target != owner)
       {
       tactor = (objtype*)ob->target;

       if ((!tactor->nextactive) && (!tactor->prevactive))
          {
          ob->target = NULL;
          return;
			 }
       dx = tactor->x - ob->x;
       dy = ob->y - tactor->y;
       dz = ob->z - tactor->z;
       //xydist = FixedSqrtHP((FixedMul(dx,dx) + FixedMul(dy,dy))>>16);
       xydist = FindDistance(dx,dy);
       angle = atan2_appx(dx,dy);
       adjust = (ob->obclass == p_godballobj)?(GODHAPT):(HAAPT);
       AdjustAngle(adjust,&(ob->angle),angle);
       ob->dir = angletodir[ob->angle];
       ob->momentumx = FixedMul(ob->speed,costable[ob->angle]);
       ob->momentumy = -FixedMul(ob->speed,sintable[ob->angle]);

       yzangle = atan2_appx(xydist,(dz<<10));
       adjust = (ob->obclass == p_godballobj)?(GODVAPT):(VAAPT);
       AdjustAngle(adjust,&(ob->yzangle),yzangle);
       ob->momentumz = -(FixedMul(ob->speed,sintable[ob->yzangle]));

		 }
    }

}


void Stagger(objtype*ob)
   {
   int randadj;


   randadj = (int)(GameRandomNumber("Stagger",1) >> 3);

   if (!ob->dirchoosetime)
      {
      ob->momentumz = ob->temp1 + (RandomSign() << 12);
      ob->dirchoosetime = 6;
      }
   else
      ob->dirchoosetime --;

   if ((ob->z + (ob->momentumz >> 10)) > (maxheight-12))
      ob->momentumz = -ob->momentumz;
   else if ((ob->z < 5) && (!sky))
      ob->z = 5;

   ob->angle += (RandomSign()*randadj);
   Fix(ob->angle);
   ob->momentumx = FixedMul(ob->speed,costable[ob->angle]);
   ob->momentumy = -FixedMul(ob->speed,sintable[ob->angle]);
   ob->dir = angletodir[ob->angle];

   }




void SpawnSplit(objtype *ob,int angle)
   {
   Fix(angle);
   SpawnMissile(ob,p_heatseekobj,ob->speed,angle,&s_p_bazooka1,0x4000);
   new->momentumz = ob->momentumz;
   new->whatever = ob->whatever;

   }



void SplitMissile(objtype*ob)
   {

   SD_PlaySoundRTP(SD_SPLITSND,ob->x,ob->y);
   if (ob->soundhandle != -1)
      SD_StopSound(ob->soundhandle);

   SpawnSplit(ob,ob->angle + ANGLES/12);
   SpawnSplit(ob,ob->angle - ANGLES/12);

   if (missobj == ob)
      {
      if (GameRandomNumber("split misscam",0)<128)
         missobj = LASTACTOR;
      else
         missobj = LASTACTOR->prev;
      }

   ob->state=NULL; // get rid of current missile

   }




void SpawnMissileSmoke(objtype *ob)
   {
   if (!ValidAreanumber(AREANUMBER(ob->tilex,ob->tiley)))
      return;

   SpawnStatic(ob->tilex,ob->tiley,stat_missmoke,-1);
   LASTSTAT->flags |= FL_ABP;
   MakeStatActive(LASTSTAT);
   SetFinePosition(LASTSTAT,ob->x,ob->y);
   LASTSTAT->z = ob->z+3;

   }


void T_Projectile (objtype *ob)
   {
   objtype *owner;
   playertype * pstate;

   owner = (objtype*)(ob->whatever);

   if (owner->obclass == playerobj)
      M_LINKSTATE(owner,pstate);

   if ((ob->soundhandle != -1) &&
       (!(oldpolltime & 7))
      )
      SD_PanRTP(ob->soundhandle,ob->x,ob->y);
#if (SHAREWARE == 0)
   if (ob->obclass == h_mineobj)
      {
      if (!ob->dirchoosetime)
         {
         NewState(ob,&s_grexplosion1);
         SD_PlaySoundRTP(SD_KRISTMINEHITSND,ob->x,ob->y);
         }
		else
         ob->dirchoosetime --;
      if (
         (ob->state == &s_mine1) &&
         (!ob->ticcount)
         )
         SD_PlaySoundRTP(BAS[ob->obclass].operate,ob->x,ob->y);

      }
#endif


	if (!ob->ticcount)
      {
      if (ob->state == &s_p_grenade)
         ob->momentumz += (GRAVITY>>6);
      else if
         (ob->state == &s_grenade_fall6)
         {
         NewState(ob,&s_explosion1);
         return;
         }
      }




   if (ob->obclass == p_drunkmissileobj)
      {
      if (ob->temp3 > 0)
         {
         ob->temp3 --;
         Stagger(ob);
         }
      else
         {
         if (ob->target == NULL)
            Stagger(ob);
         HeatSeek(ob);
         }

      }

   else if (ob->temp1 == NME_DRUNKTYPE)

      Stagger(ob);


   else if ((ob->obclass == p_heatseekobj) ||
       (ob->obclass == dm_heatseekobj) ||
       (ob->temp1 == NME_HEATSEEKINGTYPE) ||
       (ob->obclass == p_godballobj)
      )
      HeatSeek(ob);


   else if
      ((ob->obclass == p_splitmissileobj) &&
       (!pstate->buttonstate[bt_attack])
      )
      {
      SplitMissile(ob);
      return;
      }




   if ((!BATTLEMODE) &&
       (!(ob->ticcount & 7)) &&
       (ob->obclass != p_firewallobj) &&
       (ob->obclass != p_kesobj) &&
       (ob->obclass != p_godballobj) &&
       ((ob->obclass >= p_bazookaobj) ||  (ob->obclass == missileobj))
      )// &&

      SpawnMissileSmoke(ob);

	MissileMovement(ob);

	if (ob->obclass == grenadeobj)

      {
      if (ob->temp1 > 0)
         ob->temp1 -= ob->speed;
      else if (!(ob->flags & FL_DONE))
         {
         NewState(ob,&s_grenade_fall1);
         ob->flags |= FL_DONE;
         }
      }
   }



void StartFirewall(objtype*ob, int newz)
   {
   objtype *owner = (objtype*)(ob->whatever);

   MISCVARS->firespawned = 0;
   owner->temp1 = 0;
   SpawnFirewall(ob,2,newz);
   if (missobj == ob)
      missobj = LASTACTOR;
   NewState(ob,&s_megaremove);

   }



void MissileMovement(objtype*ob)
{int tryx, tryy,tryz;

 tryx = ob->x + ob->momentumx;
 tryy = ob->y + ob->momentumy;
 tryz = ob->z + (ob->momentumz >> 10);
 if (!MissileTryMove (ob, tryx, tryy, tryz))
    return;
 ob->z += (ob->momentumz >> 10);
 MoveActor(ob);
}



#define DetonateMissile(x,y) \
{MissileHit(x,y);             \
 return false;                 \
}                                \

#define QuietDetonate(ob)            \
   {                                 \
   if (ob->soundhandle != -1)        \
      SD_StopSound(ob->soundhandle); \
   if (ob == missobj)                \
      missobj = NULL;                \
   NewState(ob,&s_megaremove);       \
   return false;                     \
   }

boolean MissileTryMove(objtype*ob,int tryx,int tryy,int tryz)
{
 int             tilexlow,tileylow,tilexhigh,tileyhigh,x,y,
                 trytilex,trytiley,dx,dy,dzt,dztp1,radius,
					  sprrad,actrad,tcl,ocl,oldsrad,area,zdist,
                 wall;

 objtype         *temp;
 wall_t          *tempwall;
 doorobj_t       *tempdoor;
 int             doorn;
 statobj_t       *tempstat;
 boolean         areatried[NUMAREAS] = {0};

 sprrad = 0x4500;
 actrad = ACTORSIZE+0x2800;
 ocl = ob->obclass;
 radius = PROJSIZE-0x2200;
 if (ocl==wallfireobj)
	 radius-=0x3000;
 trytilex = (tryx >> TILESHIFT);
 trytiley = (tryy >> TILESHIFT);


 if (IsWindow(trytilex,trytiley) || (!InMapBounds((tryx>>16),(tryy>>16))))
    {
    QuietDetonate(ob);
    }



 /*
 */
 //==== ceiling/floor clipping =================//

 if (tryz < -30)
    {if ((sky==0) || (ocl == inertobj))
        {
        DetonateMissile(ob,NULL);
        }
     /*
     else
        return true;
        */
     /*
     else
        {
        NewState(ob,&s_megaremove);
        if (missobj == ob)
           missobj = NULL;

        return false;
        }
      */
    }


 if (tryz > (maxheight-10))
    {
    if ((ocl == p_firewallobj) && (!(ob->flags & FL_ISFIRE)))
       StartFirewall(ob,nominalheight);
    else
       MissileHit(ob,NULL);
    return false;
    }

 //=============================================//

 sprrad = PROJSIZE+0x1000;


 tilexlow = (int)((tryx-radius) >>TILESHIFT);
 tileylow = (int)((tryy-radius) >>TILESHIFT);

 tilexhigh = (int)((tryx+radius) >>TILESHIFT);
 tileyhigh = (int)((tryy+radius) >>TILESHIFT);

 oldsrad = sprrad;


 if (ocl == inertobj)
  goto walls;

 area = ob->areanumber;
 areatried[area] = true;
actors:
  if (ValidAreanumber(area)) {
      for(temp=firstareaactor[area];temp;temp=temp->nextinarea)
         {
         if (temp == ob)
            continue;
      dx = abs(tryx - temp->x);
      dy = abs(tryy - temp->y);
      if ((dx > actrad) || (dy > actrad))
         continue;

      if (temp->flags & FL_HEAD)
         continue;

      if ((!(temp->flags & FL_BLOCK)) || (temp->flags & FL_DYING))
         continue;

      tcl = temp->obclass;


      zdist = 32;
      dzt = abs(tryz - temp->z);

      if ((tcl == playerobj) && (temp->flags & FL_DOGMODE))
         {
         dzt = abs(tryz - (temp->z + 30));
         zdist = 10;
         }
      else if (tcl == diskobj)
         {
         zdist = 50;
         }

      if (dzt > zdist)
         continue;



      //if ((ocl==wallfireobj) && (tcl==playerobj) && (temp->flags&FL_DOGMODE) && (dz>15))
         // continue;

      //if ((ocl==playerobj) &&
         //  (ob->whatever == (void*)temp))
         //continue;

      if (ob->whatever && (ob->whatever == temp->whatever))// &&
   //      (ocl == tcl))// missiles with same owner
                                                         // go through each other
         continue;

      if (!(ob->flags & FL_ISFIRE))
            {

            int random;

            if (tcl != b_darkmonkobj)
               {
               MissileHit(ob,temp);
               ob->target = NULL;
               if (tcl == wallfireobj)
                  MissileHit(temp,NULL);
               if (((ocl == p_kesobj) || (ocl == p_godballobj)) && (tcl < pillarobj))
                  continue;
               else
                  return false;
               }
            random = GameRandomNumber("empower darkmonk",0);
#if (SHAREWARE == 0)

            if (ocl == p_kesobj)
               {
               NewState(ob,&s_megaremove);
               //ob->state = NULL;
               temp->hitpoints += (((random>>3)+140)<<1);
               CAP_OSCUROS_HITPOINTS(temp);
               }
            else if (ocl == p_firebombobj)
               {
               NewState(ob,&s_explosion1);
               temp->hitpoints += (((random>>3)+70)<<1);
               CAP_OSCUROS_HITPOINTS(temp);

               ob->target = NULL;
               }
            else
               {
               NewState(ob,&s_explosion1);
               temp->hitpoints += (((random>>3)+50)<<1);
               CAP_OSCUROS_HITPOINTS(temp);

               ob->target = NULL;
               }
            temp->temp3 = ocl;
            temp->speed = 5*SPDPATROL;
            NewState(temp,&s_darkmonkreact);
#endif
            return false;
            }

      else if (tcl < roboguardobj)
         {if ((temp->z == nominalheight) &&
            (!((tcl == playerobj) && ((temp->flags & FL_GODMODE) || (temp->flags & FL_DOGMODE) || godmode))))
            {
            if (tcl == playerobj)
               {
               playertype *pstate;
               objtype *owner = (objtype*)(ob->whatever);

               M_LINKSTATE(temp,pstate);
               if (temp->flags & FL_AV)
                  {pstate->protectiontime = 1;
                  if (temp==player)
                     GM_UpdateBonus (pstate->protectiontime, false);
                  continue;
                  }


               //temp->flags &= ~FL_COLORED;
               pstate->health = 0;
               pstate->weapon = -1;
               if (owner->obclass == playerobj)
                  BATTLE_PlayerKilledPlayer(battle_kill_with_missile,owner->dirchoosetime,temp->dirchoosetime);


               }


            temp->flags |= FL_SKELETON;
            temp->hitpoints = 0;
            Collision(temp,ob->whatever,-temp->momentumx,-temp->momentumy);
            }
         continue;
         }



      else
         {
         NewState(ob,&s_megaremove);
         ob->target = NULL;
#if (SHAREWARE == 0)
         if (tcl == b_darkmonkobj)
            NewState(temp,&s_darkmonkfspark1);
#endif
         }

      return false;

      }
   }


   for (y=tileylow;y<=tileyhigh;y++)
      for (x=tilexlow;x<=tilexhigh;x++)
         {
         area = AREANUMBER(x,y);
         if (ValidAreanumber(area) && (areatried[area]==false))
            {
            areatried[area] = true;
            goto actors;
            }
         }


/******************* WALLS/PWALLS *****************************************/

walls:

 for (y=tileylow;y<=tileyhigh;y++)
  for (x=tilexlow;x<=tilexhigh;x++)
	 {


	  tempwall = (wall_t*)actorat[x][y];
	  wall=tilemap[x][y];

     if (tempwall && M_ISWALL(tempwall) && (tempwall->which!=MWALL))
		  {if (ocl == h_mineobj)
				{
				if (WallCheck(ob->x-ob->momentumx, tryy))
					{ob->momentumx = -ob->momentumx;
					  continue;
					 }
				else if (WallCheck(tryx, ob->y-ob->momentumy))
					 {ob->momentumy = -ob->momentumy;
					  continue;
					 }
				}

         DetonateMissile(ob,tempwall);
         //MissileHit(ob,tempwall);
         //return false;
			}


     tempstat = sprites[x][y];
     sprrad = oldsrad;

     if (tempstat &&
			((tempstat->flags & FL_SHOOTABLE) || (tempstat->flags & FL_BLOCK)))
        {

		  if ((tempstat->itemnumber >= stat_bcolumn) &&
				(tempstat->itemnumber <= stat_icolumn))
			 sprrad += 0x5000;

		  dx = tryx - tempstat->x;
		  if ((dx < -sprrad) || (dx > sprrad))
           continue;

        dy = tryy - tempstat->y;
		  if ((dy < -sprrad) || (dy > sprrad))
           continue;

//#define MINSTATZDIFF 60

        dzt = abs(ob->z - tempstat->z);
        dztp1 = abs(tryz - tempstat->z);
      /*
        if (ocl == p_firewallobj)// && (dztp1 <= MINSTATZDIFF))
           {
           if (ob->flags & FL_ISFIRE)
              {
              int cz = (ob->z - tempstat->z + MINSTATZDIFF);

              if ((cz >= -MAXSTEPHEIGHT) && (cz <= 0))
                 {
                 ob->z = tempstat->z - MINSTATZDIFF;
                 tryz = ob->z + (ob->momentumz >> 16);
                 dzt = MINSTATZDIFF;
                 }
              }

           if ((dztp1 >= MINSTATZDIFF) || (dzt >= MINSTATZDIFF))
              continue;

           if (!(ob->flags & FL_ISFIRE))
              {
              StartFirewall(ob,tempstat->z - MINSTATZDIFF);
              return false;
              }
           }


        else*/
           {
           if (dztp1 > 50)
              continue;

           DetonateMissile(ob,tempstat);
           }
        //MissileHit(ob,tempstat);
        //return false;
        }
	 }


//mwalls:



 if (M_ISMWALL(trytilex,trytiley))
      {
      maskedwallobj_t * mw;

      wall=tilemap[trytilex][trytiley];
      //tempwall = (wall_t*)actorat[trytilex][trytiley];


      mw=maskobjlist[wall&0x3ff];

      if (!(mw->flags&MW_BLOCKING))
         {

         if ((levelheight > 1) &&
            (((!(mw->flags & MW_ABOVEPASSABLE)) && (tryz <= 32)) ||
             ((!(mw->flags & MW_MIDDLEPASSABLE)) && (tryz > 25) && (tryz < nominalheight-32)) ||
             ((!(mw->flags & MW_BOTTOMPASSABLE)) && (tryz > maxheight - 74))
            )
            )
            DetonateMissile(ob,NULL);

         }

      else if (mw->flags&MW_SHOOTABLE)
         {
         if (ob->z >= maxheight-64)
            {
            UpdateMaskedWall(tilemap[trytilex][trytiley]&0x3ff);
            }
         else
            DetonateMissile(ob,NULL);

         }

      else
         DetonateMissile(ob,NULL);
      //MissileHit(ob,tempwall);
      //return false;
      }



/******************* DOOR STUFF ******************************************/


  else if (M_ISDOOR(trytilex,trytiley))
     {
     doorn = tilemap[trytilex][trytiley];
     tempdoor = doorobjlist[doorn&0x3ff];
     if (tempdoor->position>=0x8000)
			 {
			 if (ob->z>maxheight-64)
				 return true;
			 }
     DetonateMissile(ob,tempdoor);
     }

  return true;


}



void SpawnFirewall(objtype*ob,int which,int newz)
{int i,j,count,newx,newy;
 objtype* owner;
 wall_t*tempwall;
 statetype* frame;
 int offset;

 owner = (objtype*)(ob->whatever);

 if ((owner->temp1 < 2) && (MISCVARS->firespawned < 14))
    {for(i=0;i<=which;i++)
        {
        GetNewActor ();
        MakeActive(new);
        MISCVARS->firespawned ++;
        new->obclass = p_firewallobj;
        new->which = ACTOR;
        new->areanumber = ob->areanumber;
        MakeLastInArea(new);
        offset = 0x6000;
        if (!which)
           new->temp1 = ob->temp1;
        else if (i==1)
           new->temp1 = ob->angle + ANGLES/4;
        else if (i==2)
           new->temp1 = ob->angle - ANGLES/4;
        else
           {
           new->temp1 = 0;
           offset = 0;
           new->flags |= FL_DONE;
           }

        Fix(new->temp1);

        new->speed = 0x8000;
        new->angle = ob->angle;
        ParseMomentum(new,new->angle);
        newx = ob->x + FixedMul(offset,costable[new->temp1]);
        newy = ob->y - FixedMul(offset,sintable[new->temp1]);
        SetFinePosition(new,newx,newy);
        SetVisiblePosition(new,newx,newy);
        new->whatever = ob->whatever;
        new->dirchoosetime = 2;
        new->flags |= (FL_NEVERMARK|FL_ABP|FL_NOFRICTION);
        count = (int)(GameRandomNumber("SpawFireWall",0) & 15);

        for(frame = &s_fireunit1,j=0;j<count;frame = frame->next,j++);

        NewState(new,frame);
        new->flags |= FL_ISFIRE;
         //SD_Play(SD_EXPL);
        tempwall = (wall_t*)actorat[new->tilex][new->tiley];
        new->z = newz;
        if (tempwall && M_ISWALL(tempwall))
            {
            SetFinePosition(new,ob->x,ob->y);
            SetVisiblePosition(new,ob->x,ob->y);
            owner->temp1++;
            }
        }
    }

}


void T_Firethink(objtype*ob)
  {

  if (ob->dirchoosetime)
     ob->dirchoosetime--;
  else if (!(ob->flags & FL_DONE))
     {
     SpawnFirewall(ob,0,ob->z);
	  ob->flags |= FL_DONE;
     }

  MissileMovement(ob);

  }



void ResolveRide(objtype *ob)
   {
   objtype *ride = (objtype*)(ob->whatever);

   if (M_ISACTOR(ride) && (ob->obclass != playerobj))
      {
      if (ob->flags & FL_RIDING)
         {
         int dx,dy;

         dx = ob->x - ride->x;
         dy = ob->y - ride->y;
         if ((dx < -MINACTORDIST) || (dx > MINACTORDIST) ||
             (dy < -MINACTORDIST) || (dy > MINACTORDIST) )
            {
            ride->whatever = NULL;
            ob->whatever = NULL;
            ob->flags &= ~FL_RIDING;
            }
			}
      }

   }




void MoveActor(objtype*ob)
   {
   int linked,oldarea,newarea,
       tilex,tiley,oldtilex,oldtiley;

	linked = 0;

   ResolveRide(ob);

      oldtilex = ob->tilex;
      oldtiley = ob->tiley;

   SetFinePosition(ob,ob->x+ob->momentumx,ob->y+ob->momentumy);
   /*
      if (ob->state == &s_explosion1)	 Error("moving explosion"); */

   if ((ob->obclass == playerobj) || (ob->flags & FL_NOFRICTION) || (ob->state->think == T_Collide) ||
       (ob->obclass == b_heinrichobj) || (ob->obclass == h_mineobj))

     SetVisiblePosition(ob,ob->x,ob->y);



	if (ob->obclass == inertobj)
	 return;

   if ((ob->obclass == b_darksnakeobj) && (ob != SNAKEHEAD))
      {
      oldarea = ob->areanumber;
      newarea = SNAKEHEAD->areanumber;
      if (oldarea != newarea)
         {
         RemoveFromArea(ob);
         ob->areanumber = newarea;
         MakeLastInArea(ob);
         }
      return;

      }


   oldarea = ob->areanumber;
   newarea = AREANUMBER(ob->tilex,ob->tiley);

//======================  swap in linked lists =====================
	if (oldarea != newarea)
      {
      RemoveFromArea(ob);
      ob->areanumber = newarea;
      MakeLastInArea(ob);
      }
  return;
}



void SpawnPushColumn(int tilex,int tiley,int which,int dir, int linked)
{
  if (which==0)
	  {
	  SpawnNewObj(tilex,tiley,&s_pushcolumn1,pillarobj);
//     for (i=0;i<(levelheight-1);i++)
//        SpawnStatic(tilex,tiley,stat_bcolumn,-(i<<6));
	  }
  else if (which==1)
	  {
	  SpawnNewObj(tilex,tiley,&s_pushcolumn2,pillarobj);
//     for (i=0;i<(levelheight-1);i++)
//        SpawnStatic(tilex,tiley,stat_gcolumn,-(i<<6));
	  }
  else
	  {
	  SpawnNewObj(tilex,tiley,&s_pushcolumn3,pillarobj);
//     for (i=0;i<(levelheight-1);i++)
//        SpawnStatic(tilex,tiley,stat_icolumn,-(i<<6));
	  }
  PreCacheActor(pillarobj,which);

  gamestate.secrettotal++;
  new->speed = PILLARMOM;
  new->temp1 = 0x20000;
  new->temp2 = linked;
  new->flags |= (FL_BLOCK|FL_NOFRICTION);
  new->flags &= ~FL_SHOOTABLE;
  new->flags |= FL_HEIGHTFLIPPABLE;
  new->dir = dir;
  if (dir != nodir)
	ParseMomentum(new,dirangle8[new->dir]);


}



void SpawnWallfire(int tilex, int tiley, int dir)
{int offx,offy;


  GetNewActor();
  new->speed = 0x2000;
  SetTilePosition(new,tilex,tiley);
  SetVisiblePosition(new,new->x,new->y);
  new->obclass = wallfireobj;
  new->dir = dir*2;
  new->flags |= (FL_BLOCK|FL_NOFRICTION|FL_NEVERMARK);
  new->flags &= ~FL_SHOOTABLE;
  new->which = ACTOR;
  new->angle = dirangle8[new->dir];
  offx = FixedMul(0x10000,costable[new->angle])>>TILESHIFT;
  offy = -(FixedMul(0x10000,sintable[new->angle])>>TILESHIFT);

  new->areanumber = MAPSPOT(new->tilex+offx,new->tiley+offy,0)-AREATILE;
  MakeLastInArea(new);

  NewState(new,&s_wallfireball);

}



void SpawnSneaky(int tilex,int tiley)
   {

   SpawnNewObj(tilex,tiley,&s_sneakydown,lowguardobj);
   new->temp3 = SNEAKY;
   if (!loadedgame)
      gamestate.killtotal++;
   StandardEnemyInit(new,north>>1);

   PreCacheActor(lowguardobj,0);
   }


void RespawnEluder(void)
   {
   int rand,count;
   int nx,ny;

   rand = (GameRandomNumber("eluder respawn",0) % NUMSPAWNLOCATIONS);

   for(count=0;count < NUMSPAWNLOCATIONS;count++)
      {
      if (!actorat[SPAWNLOC[rand].x][SPAWNLOC[rand].y])
         {
         SpawnCollector(SPAWNLOC[rand].x,SPAWNLOC[rand].y);
         return;
         }
      rand= ((rand + 1) % NUMSPAWNLOCATIONS);

      }

//MED
   nx = SPAWNLOC[rand].x;
   ny = SPAWNLOC[rand].y;
   FindEmptyTile(&nx,&ny);
   SpawnCollector(nx,ny);
   }

//****************************************************************************
//
//
//
//****************************************************************************

void SpawnCollector(int tilex, int tiley)
{
 #if (SHAREWARE == 0)
 if ( dopefish==true )
    {
    SpawnNewObj(tilex,tiley,&s_scottwander1,collectorobj);
    }
 else
 #endif
    {
    SpawnNewObj(tilex,tiley,&s_collectorwander1,collectorobj);
    }

 new->flags |= (FL_SHOOTABLE|FL_BLOCK|FL_NOFRICTION|FL_FULLLIGHT);
 new->hitpoints = 500;
 new->speed = 0x3000;
 new->dir = north;
 new->dirchoosetime = 175;
 new->z = PlatformHeight(tilex,tiley);
 if (new->z == -10)
   new->z = 0;
 if (areabyplayer[new->areanumber])
	{new->flags |= FL_ABP;
	 MakeActive(new);
	}

}



void SelectDoorDir(objtype*ob)
{int dx,dy,actrad;
 dirtype dtry1,dtry2,tdir,olddir;

 dx= ob->targettilex - ob->x;
 dy= ob->y - ob->targettiley;
 olddir = ob->dir;
 if ((abs(dx) < 0x4000) && (abs(dy) < 0x4000))
	 {ZEROMOM;
     SetFinePosition(ob,ob->targettilex,ob->targettiley);
     SetVisiblePosition(ob,ob->x,ob->y);
     ParseMomentum(ob,dirangle8[ob->temp2]);
	  ActorMovement(ob);
	  ob->temp2 = 0;
	  ob->temp1 = 20;
    #if (SHAREWARE == 0)
    if ( dopefish==true )
       {
       NewState(ob,&s_scottwander1);
       }
    else
    #endif
       {
       NewState(ob,&s_collectorwander1);
       }
	  ob->targettilex = ob->targettiley = 0;
	  ob->dirchoosetime = 165;
	  return;
	 }


 ZEROMOM;

 ParseMomentum(ob,atan2_appx(dx,dy));
 ActorMovement(ob);
 if (ob->momentumx || ob->momentumy)
	return;
 actrad = ACTORSIZE;
 dtry1=nodir;
 dtry2=nodir;

 if (dx> actrad)
	dtry1= east;
 else if (dx< -actrad)
	dtry1= west;
 if (dy> actrad)
	dtry2= north;
 else if (dy < -actrad)
	dtry2= south;


 if (abs(dy)>abs(dx))
	 {tdir=dtry1;
	  dtry1=dtry2;
	  dtry2=tdir;
	 }


 if (dtry1 != nodir)
	M_CHECKDIR(ob,dtry1);

 if (dtry2 != nodir)
	M_CHECKDIR(ob,dtry2);

 if (dtry1 != nodir)
	M_CHECKDIR(ob,dirorder[dtry1][NEXT]);

 if (dtry2 != nodir)
	M_CHECKDIR(ob,dirorder[dtry2][NEXT]);

 for(tdir = dirorder[olddir][NEXT];tdir != olddir;tdir = dirorder[tdir][NEXT])
	M_CHECKDIR(ob,tdir);
 ob->dir = olddir;

}


boolean EluderCaught(objtype*ob)
{
 objtype *temp;
 int dx,dy,dz;
 playertype *pstate;
 int dist = 0xc000;

 for(temp = PLAYER[0];temp != PLAYER[numplayers-1]->next;temp=temp->next)
	{
#if (SHAREWARE == 0)
     if (temp->state != &s_doguse)
		 continue;
#endif

	  dx = M_ABS(temp->x - ob->x);
	  if (dx > dist)
		 continue;

	  dy = M_ABS(temp->y - ob->y);
	  if (dy > dist)
		 continue;

	  dz = M_ABS(temp->z - ob->z);
	  if (dz > (dist>>10))
		 continue;

	  M_LINKSTATE(temp,pstate);
	  //if (DOGSCRATCH.attackinfo[pstate->attackframe].attack == at_pulltrigger)
		 {BATTLE_CheckGameStatus(battle_caught_eluder,temp->dirchoosetime);
		  SpawnNewObj(ob->tilex,ob->tiley,&s_itemspawn1,inertobj);
		  new->flags |= FL_ABP;
        SetFinePosition(new,ob->x,ob->y);
        SetVisiblePosition(new,ob->x,ob->y);
		  new->z = ob->z;
		  SD_PlaySoundRTP(SD_GETBONUSSND,ob->x,ob->y);
		  MakeActive(new);
		  NewState(ob,&s_megaremove);
		  return true;
		 }

	}

 return false;
}


void T_CollectorFindDoor(objtype*ob)
{

 if (EluderCaught(ob))
	return;

 if (!(gamestate.TimeCount % 17))
	SD_PlaySoundRTP(SD_MONKGRABSND,ob->x,ob->y);


 if ((ob->z != nominalheight) && (!IsPlatform(ob->tilex,ob->tiley)))
	ZEROMOM;



 if (ob->dirchoosetime)
    ob->dirchoosetime --;
 else
    {
    #if (SHAREWARE == 0)
    if ( dopefish==true )
       {
       NewState(ob,&s_scottwander1);
       }
    else
    #endif
       {
       NewState(ob,&s_collectorwander1);
       }
	  ob->dirchoosetime = 165;
	  ob->targettilex = ob->targettiley = 0;
	  return;
	 }

 if (ob->temp1)
	{int dx,dy;

	 ob->temp1 --;
	 ActorMovement(ob);
	 dx = ob->targettilex-ob->x;
	 dy = ob->targettiley-ob->y;
	 if ((abs(dx) < 0x4000) && (abs(dy) < 0x4000))
		 {ZEROMOM;
        SetFinePosition(ob,ob->targettilex,ob->targettiley);
        SetVisiblePosition(ob,ob->x,ob->y);

		  ParseMomentum(ob,dirangle8[ob->temp2]);
		  ActorMovement(ob);
		  ob->temp2 = 0;
		  ob->temp1 = 35;
        #if (SHAREWARE == 0)
        if ( dopefish==true )
           {
           NewState(ob,&s_scottwander1);
           }
        else
        #endif
           {
           NewState(ob,&s_collectorwander1);
           }
		  ob->targettilex = ob->targettiley = 0;
		  ob->dirchoosetime = 165;
		  return;
		 }
	 if (NOMOM)
		ob->temp1 = 0;
	 return;
	}



 ob->temp1 = 5;

 if (ob->targettilex || ob->targettiley)
	SelectDoorDir(ob);

 else
	{int i;
	 doorobj_t* dptr;

//==========================================================================
#define SetCollectorTarget(xoffset,yoffset,newdir)                         \
   {                                                                       \
   ob->targettilex = ((dptr->tilex + (xoffset)) << TILESHIFT) + HALFGLOBAL1; \
   ob->targettiley = ((dptr->tiley + (yoffset)) << TILESHIFT) + HALFGLOBAL1; \
   ob->temp2 = newdir;                                                     \
   if (GameRandomNumber("collector door search",0) < 100)                  \
      return;                                                              \
   }
//==========================================================================

   for(i=0;i<doornum;i++)
	  {dptr = doorobjlist[i];
		if (dptr->vertical)
        {
        int area1 = AREANUMBER(dptr->tilex-1,dptr->tiley),
            area2 = AREANUMBER(dptr->tilex+1,dptr->tiley);

        if (area1 == ob->areanumber)
           SetCollectorTarget(-1,0,east)

        else if (area2 == ob->areanumber)
           SetCollectorTarget(1,0,west);

		  }
		else
        {
        int area1 = AREANUMBER(dptr->tilex,dptr->tiley-1),
            area2 = AREANUMBER(dptr->tilex,dptr->tiley+1);

        if (area1 == ob->areanumber)
           SetCollectorTarget(0,-1,south)

        else if (area2 == ob->areanumber)
           SetCollectorTarget(0,1,north);

		  }
	  }
  }

}


void T_CollectorWander(objtype*ob)
{

   int newtilex,newtiley;

   if (EluderCaught(ob))
      return;

   if ((ob->z != nominalheight) && (!IsPlatform(ob->tilex,ob->tiley)))
      ZEROMOM;

   if (!(gamestate.TimeCount & 15))//%17
      SD_PlaySoundRTP(SD_MONKGRABSND,ob->x,ob->y);

   if (ob->dirchoosetime)
      {
      if (doornum > 0)
         ob->dirchoosetime --;
      }

   else
      {
      #if (SHAREWARE == 0)
      if ( dopefish==true )
         {
         NewState(ob,&s_scottwanderdoor1);
         }
      else
      #endif
         {
         NewState(ob,&s_collectorfdoor1);
         }
      ob->temp1 = 0;
      ob->dirchoosetime = 165;
      ob->targettilex = ob->targettiley = 0;
      return;
      }

   if (ob->temp1) // temp1 holds direction time
      ob->temp1 --;
   else
      {
      dirtype bestdir,tempdir;

      bestdir = angletodir[GameRandomNumber("collector theta",0) << 3];

      for(tempdir = bestdir;tempdir != dirorder[bestdir][PREV];tempdir = dirorder[tempdir][NEXT])
         {
         ParseMomentum(ob,dirangle8[tempdir]);
         newtilex = ((ob->x + ob->momentumx)>>16);
         newtiley = ((ob->y + ob->momentumy)>>16);
         if (IsWindow(newtilex,newtiley))
            continue;
         ActorMovement(ob);
         if (ob->momentumx || ob->momentumy)
            {
            ob->temp1 = (GameRandomNumber("collector choose time",0) >> 2);
            return;
            }
         }

      }

   newtilex = ((ob->x + ob->momentumx)>>16);
   newtiley = ((ob->y + ob->momentumy)>>16);


   if (IsWindow(newtilex,newtiley))
      {
      ob->temp1 = 0;
      return;
      }


   ActorMovement(ob);

   if (NOMOM)
      ob->temp1 = 0;
   }





boolean CheckDoor(objtype *ob,doorobj_t * door,int trytilex,int trytiley)
{boolean doorok=false;


 switch(ob->dir)
	{case north:
	  if ((ob->tiley == (door->tiley + 1)) && (trytilex == ob->tilex))
		 doorok = true;
	  break;

	 case east:
	  if ((ob->tilex == (door->tilex - 1)) &&	(trytiley == ob->tiley))
		doorok = true;
	  break;

	 case south:
	  if ((ob->tiley == (door->tiley - 1)) &&	(trytilex == ob->tilex))
		doorok = true;
	  break;

	 case west:
	  if ((ob->tilex == (door->tilex + 1)) &&	(trytiley == ob->tiley))
		doorok = true;
	  break;
	default:
	    ;
	}


  if (doorok)
   {SetTilePosition(ob,ob->tilex,ob->tiley);
    SetVisiblePosition(ob,ob->x,ob->y);
	 return true;
	}
  return false;
}


boolean WallCheck(int tryx,int tryy)
{int tilexlow,tilexhigh,tileylow,tileyhigh,y,x;

 tilexlow = (int)((tryx -PLAYERSIZE) >>TILESHIFT);
 tileylow = (int)((tryy -PLAYERSIZE) >>TILESHIFT);

 tilexhigh = (int)((tryx + PLAYERSIZE) >>TILESHIFT);
 tileyhigh = (int)((tryy + PLAYERSIZE) >>TILESHIFT);


 for (y=tileylow;y<=tileyhigh;y++)
  for (x=tilexlow;x<=tilexhigh;x++)
	{//tempwall = (wall_t*)actorat[x][y];
	 //if (tempwall && M_ISWALL(tempwall))
	 if (tilemap[x][y])
	  return false;
	}

 return true;

}


boolean QuickSpaceCheck(objtype*ob,int tryx, int tryy)
{int xlow,xhigh,ylow,yhigh,x,y,dx,dy;
 objtype* temp;

 xlow = (int)((tryx-ACTORSIZE) >>TILESHIFT);
 ylow = (int)((tryy-ACTORSIZE) >>TILESHIFT);

 xhigh = (int)((tryx+ACTORSIZE) >>TILESHIFT);
 yhigh = (int)((tryy+ACTORSIZE) >>TILESHIFT);
/******************* WALLS/PWALLS *****************************************/

 for (y=ylow;y<=yhigh;y++)
  for (x=xlow;x<=xhigh;x++)
	{temp = (objtype*)actorat[x][y];
	 if ((temp && (temp->which != ACTOR)) ||
		  (sprites[x][y] && (sprites[x][y]->flags & FL_BLOCK))

			|| tilemap[x][y])
		return false;
	}

   if (ValidAreanumber(ob->areanumber)) {
       for(temp=firstareaactor[ob->areanumber];temp;temp=temp->nextinarea)
	{if (temp == ob)
		continue;
	 if ((temp->flags & FL_NONMARK) || (temp->flags & FL_NEVERMARK))
	  continue;
	 dx = tryx - temp->x;
	 if ((dx < -MINACTORDIST) || (dx > MINACTORDIST))
		continue;
	 dy = tryy - temp->y;
	 if ((dy < -MINACTORDIST) || (dy > MINACTORDIST))
		continue;
	 if (ob->whatever == (void*)temp)
		continue;
	 if (temp->whatever == ob->whatever)
		continue;
	 return false;
	}
   }
 return true;
}


//=========================================================================
//
//                    ACTOR TRY MOVE MADNESS
//
//=========================================================================

typedef enum
{
 NO_MOVEMENT,
 Z_MOVEMENT_ONLY,
 OK_TO_CONTINUE

}movement_status;



//==================== Some ActorTryMove macros ==============================

#define CheckProximitySpecials(ob,temp)                             \
{                                                                   \
   if (ocl == b_heinrichobj)                                       \
      {                                                            \
      if (tcl == playerobj)                                        \
         {                                                         \
         playertype *pstate;                                      \
                                                                     \
         M_LINKSTATE(temp,pstate);                                \
         DamageThing(temp,5);                                     \
         temp->whatever = ob;                                     \
         temp->temp2 = COLUMNCRUSH;                               \
         pstate->heightoffset += 4;                               \
         if (pstate->heightoffset >= 30)                          \
            pstate->heightoffset = 30;                           \
         pstate->oldheightoffset = pstate->heightoffset;          \
         }                                                         \
      else                                                         \
         {                                                         \
         temp->momentumx = temp->momentumy = temp->momentumz = 0; \
         temp->hitpoints = 0;                                     \
         }                                                         \
      if (temp->hitpoints <= 0)                                    \
         temp->flags |= FL_HBM;                                     \
      Collision(temp,ob,0,0);                                      \
      continue;                                                    \
      }                                                             \
                                                                    \
   else if ((ocl == b_darksnakeobj) && (tcl == playerobj)) \
      {                                                     \
      DamageThing(temp,1);                                      \
      Collision(temp,ob,0,0);     \
      M_CheckPlayerKilled(temp);                                   \
      } \
        \
   if ((ocl == boulderobj) && (tcl >= lowguardobj) && (tcl < roboguardobj))\
      {temp->momentumx = temp->momentumy = temp->momentumz = 0;     \
       temp->hitpoints = 0;                                         \
       temp->flags |= FL_HBM;                                       \
       Collision(temp,ob,0,0);                                      \
       SD_PlaySoundRTP(SD_ACTORSQUISHSND,temp->x,temp->y);          \
       continue;                                                    \
      }                                                             \
                                                                    \
   if (pusher && (ocl != tcl) && (!(temp->flags & FL_DYING))  &&    \
       (tcl < roboguardobj)                                         \
      )                                                             \
      {if ((!ob->ticcount) && (ocl != collectorobj) && (ocl != diskobj))\
          DamageThing(temp,5);                                      \
                                                                    \
       if (tcl == playerobj)                                        \
         temp->flags |= FL_PUSHED;                                  \
       Collision(temp,ob,ob->momentumx-temp->momentumx,ob->momentumy-temp->momentumy);\
       M_CheckPlayerKilled(temp);                                   \
       continue;                                                    \
      }                                                             \
                                                                    \
   if (bouncer)                                                    \
      {ob->momentumx = -ob->momentumx;                              \
       continue;                                                    \
      }                                                             \
   }



#define CheckStepping(ob,step,minzdiff)                         \
{                                                               \
 int cz = (ob->z - step->z + minzdiff);                         \
                                                                \
 if ((cz >= -MAXSTEPHEIGHT) && (cz <= MAXSTEPHEIGHT))           \
      {if ((ob->obclass == playerobj) && (ob->temp2 == 0) &&    \
           (ob->z != (step->z - minzdiff))                      \
          )                                                     \
         {                                                      \
         playertype *pstate;                                    \
                                                                \
         M_LINKSTATE(ob,pstate);                                \
                                                                \
         pstate->heightoffset = pstate->oldheightoffset + cz;   \
         ob->temp2 = (cz >= 0)?(STEPUP):(STEPDOWN);             \
         }                                                      \
      ob->z = step->z - minzdiff;                               \
      tryz = ob->z + (ob->momentumz >> 16);                     \
      dzt = minzdiff;                                           \
      }                                                         \
}                                                               \




//============ Players crushing other players =====================

void BattleCrushCheck(objtype *ob,objtype *listrover)                              \
   {
   if ((ob->obclass == playerobj) && (listrover->obclass == playerobj))
      {
      playertype * pstate;

      M_LINKSTATE(listrover,pstate);
      if (pstate->health <= 0)
         BATTLE_PlayerKilledPlayer(battle_kill_by_crushing,ob->dirchoosetime,
                                    listrover->dirchoosetime);
      }
   }


//=================================================================





movement_status CheckOtherActors(objtype*ob,int tryx,int tryy,int tryz)
   {
   objtype *listrover;
   int area;
   int op;
   int areatried[NUMAREAS]={0};
   int tilexlow,tilexhigh,tileylow,tileyhigh;
   int radius,actrad,oldrad;
   boolean bouncer,pusher,thinkingactor,zstoppable,ACTORSTOP;
   int dzt,dztp1,checkz;
   int x,y,dx,dy;
   int ocl,tcl;
   int ISPLAYER = 0;
   int hoffset;

   ocl = ob->obclass;

   actrad = MINACTORDIST;//ACTORSIZE+0x2800;
   pusher =  ((ocl == wallopobj) || (ocl == pillarobj) ||
              (ocl == roboguardobj) || (ocl == collectorobj) ||
              (ocl == boulderobj) || (ocl == diskobj)
             );

   thinkingactor = ((ocl != playerobj) && (ob->state->think != T_Collide) &&
                    (ocl < roboguardobj)
                   );

   zstoppable = (!(ob->flags & FL_DYING));
   bouncer = ((ocl == playerobj) && (ob->flags & FL_ELASTO));
   radius = ACTORSIZE;


   if (ocl != playerobj)
         {
         //actrad = MINACTORDIST;
         //if ((ob->dir == nodir) && (ocl != b_robobossobj) &&
          //   (ocl != wallopobj) && (ocl != roboguardobj) && (ocl != diskobj)
           // )
           // Error("ob called with nodir");
         if (ocl == boulderobj)
            radius += (ACTORSIZE/4);
         else if (ocl == b_darksnakeobj)
            radius -= 6000;
         else if (ocl == inertobj)
            radius -= 0x2000;
         }

   else
         {
         ISPLAYER = 1;
         if (ob->flags & FL_DOGMODE)
            hoffset = 10;
         }



   tilexlow = (int)((tryx-radius) >>TILESHIFT);
   tileylow = (int)((tryy-radius) >>TILESHIFT);

   tilexhigh = (int)((tryx+radius) >>TILESHIFT);
   tileyhigh = (int)((tryy+radius) >>TILESHIFT);

   area = ob->areanumber;
   if (ValidAreanumber(area)) areatried[area] = 1;
   ACTORSTOP = false;
   oldrad = actrad;

 actors:
   if (ValidAreanumber(area)) {
       for(listrover=firstareaactor[area];listrover;listrover=listrover->nextinarea)
         {
         actrad = oldrad;

         if (listrover == ob)
            continue;


         tcl = listrover->obclass;

         if ((tcl == b_darksnakeobj) && (listrover != SNAKEHEAD))
            continue;

         if (((tcl == bladeobj) || (tcl == firejetobj)) && thinkingactor)

            actrad += 0x3000;


         dx = tryx - listrover->x;
         if ((dx < -actrad) || (dx > actrad))
            continue;

         dy = tryy - listrover->y;
         if ((dy < -actrad) || (dy > actrad))
            continue;


         if ((ocl == b_darksnakeobj) && (tcl == ocl))
            continue;

         if ((tcl == springobj) && (listrover->state->condition & SF_UP) &&
             (listrover->temp1!=3) && (levelheight > 1) &&
             (abs(listrover->z - ob->z) < 32) && (!ob->momentumz)
            )
            {
               {
               op = (FixedMul((int)GRAVITY,(int)((ob->z-10)<<16))<<1);
               ob->momentumz = -FixedSqrtHP(op);
               }

               SD_PlaySoundRTP(SD_SPRINGBOARDSND,listrover->x,listrover->y);
               NewState(listrover,&s_spring2);

            }



         if ((tcl == firejetobj) && (ob->z < listrover->z))
            continue;

         if ((!(listrover->flags & FL_BLOCK)) && (actrad == oldrad)) // if not blocking
                                                   // and actor not avoiding
                                                   // env. danger
            continue;



         if (tcl == crushcolobj)
            checkz = listrover->temp2;
         else
            checkz = listrover->z;

      #define  MINACTORZDIFF 58

         dzt = abs(checkz - ob->z);
         dztp1 = abs(checkz - tryz);

         if ((tcl == diskobj) && (dztp1 <= MINACTORZDIFF) && zstoppable &&
             (ocl != b_heinrichobj)
            )
            CheckStepping(ob,listrover,MINACTORZDIFF);

         dztp1 = abs(checkz - tryz);



         if ((dzt > (MINACTORZDIFF - 25)) && (dzt < MINACTORZDIFF) &&
             (dztp1 < MINACTORZDIFF) && (tcl < roboguardobj) &&
             (ocl < roboguardobj)
            )
            {
            int rdx,rdy;

            rdx = abs(ob->x - listrover->x);
            rdy = abs(ob->y - listrover->y);
            if ((rdx < actrad) && (rdy < actrad))
               {
               if (ob->z > listrover->z)
                  listrover->z = ob->z - MINACTORZDIFF;
               else
                  ob->z = listrover->z - MINACTORZDIFF;

               dzt = dztp1 = MINACTORZDIFF;
               }

            }


         if ((dztp1 >= MINACTORZDIFF) || (dzt >= MINACTORZDIFF))
            {
            if ((dzt >= MINACTORZDIFF) && (dztp1 <= MINACTORZDIFF) &&
                zstoppable
               )
               {//ob->momentumz = 0;
               if (ob->z < listrover->z)
                  {
                  ob->z = listrover->z - MINACTORZDIFF;
                  ob->momentumz = 0;
                  }
               else
                  ob->momentumz = 2*GRAVITY;
               if ((listrover->z > ob->z) && (tcl < roboguardobj) && (ocl < roboguardobj) &&
                     (!(listrover->flags & FL_DYING))
                  )

                  {
                  DamageThing(listrover,5);
                  BattleCrushCheck(ob,listrover);
                  Collision(listrover,ob,listrover->momentumx,listrover->momentumy);
                  /*
                  if ((ocl == playerobj) && (listrover->flags & FL_DYING))
                     GivePoints(starthitpoints[gamestate.difficulty][tcl]);
                  */
                  }

               if (((tcl == bladeobj) || (tcl == diskobj)) && (ob->z < listrover->z))
                  {
                  ob->whatever = listrover;
                  if (listrover->flags & FL_ACTIVE)
                     ob->flags |= FL_RIDING;
                  listrover->whatever = ob;
                  }

               //Debug("\nplayerz %d, tryz %d momz zeroed at %d, clearances %d and %d",
                  //   ob->z,tryz,listrover->z-64 + (listrover->momentumz >> 16),dzt,dztp1);

               }

            continue;
            }


         CheckProximitySpecials(ob,listrover);


         ACTORSTOP = true;
         if (!ob->momentumz)
            return NO_MOVEMENT;

         }
      }


   for (y=tileylow;y<=tileyhigh;y++)
      for (x=tilexlow;x<=tilexhigh;x++)
         {
         area = AREANUMBER(x,y);
         if (ValidAreanumber(area) && (areatried[area]==0))
            {
            areatried[area] = 1;
            goto actors;
            }
         }



   if (ACTORSTOP==true)
     return Z_MOVEMENT_ONLY;

   return OK_TO_CONTINUE;
}



movement_status CheckRegularWalls(objtype *ob,int tryx,int tryy,int tryz)
   {
   int tilexlow,tilexhigh,tileylow,tileyhigh,x,y,radius;
   classtype ocl;
   boolean WALLSTOP,ISPLAYER=false;

   ocl = ob->obclass;
   tryz=tryz;


   if (ocl != playerobj)
         {
         radius = ACTORSIZE - 0x1000;
         //actrad = MINACTORDIST;
         //if ((ob->dir == nodir) && (ocl != b_robobossobj) &&
          //   (ocl != wallopobj) && (ocl != roboguardobj) && (ocl != diskobj)
          //  )
           // Error("ob called with nodir");
         if (ocl == boulderobj)
            radius += (ACTORSIZE/4);
         else if (ocl == b_darksnakeobj)
            radius -= 6000;
         else if (ocl == inertobj)
            radius -= 0x2000;
         }

   else
         {
         radius = PLAYERSIZE;
         ISPLAYER = true;
         }


   tilexlow = (int)((tryx-radius) >>TILESHIFT);
   tileylow = (int)((tryy-radius) >>TILESHIFT);

   tilexhigh = (int)((tryx+radius) >>TILESHIFT);
   tileyhigh = (int)((tryy+radius) >>TILESHIFT);


   WALLSTOP = false;

   for (y=tileylow;y<=tileyhigh;y++)
      for (x=tilexlow;x<=tilexhigh;x++)
         {
         wall_t          *tempwall;
         int             wall;

         tempwall = (wall_t*)actorat[x][y];
         wall=tilemap[x][y];
         if (tempwall)
            {
            if (tempwall->which==WALL)// && IsWindow(x,y)==false)
               {
               if (ocl == boulderobj)
                  {
#if (SHAREWARE == 0)
                  NewState(ob,&s_bouldersink1);
#endif
                  SD_PlaySoundRTP(SD_BOULDERHITSND,ob->x,ob->y);
                  }
               else if (ISPLAYER && (!(ob->flags & FL_DYING)) &&
                     (!(ob->flags & FL_AV)) &&
                     (tempwall->flags & FL_W_DAMAGE))
                  {
                  DamageThing(ob,5);
                  Collision(ob,(objtype*)tempwall,0,0);
                  M_CheckPlayerKilled(ob);
                  SD_PlaySoundRTP(SD_PLAYERBURNEDSND,ob->x,ob->y);
                  }

               //return false;
               WALLSTOP = true;
               if ((ocl == inertobj) &&
                   (ob->dirchoosetime == GIBVALUE) &&

                   (((ob->tilex - x) == 0) != ((ob->tiley - y) == 0)) &&
                   (ob->z > -28)

                  )
                  {
   //                SoftError ("Blood Dripping oldpolltime=%ld\n",oldpolltime);
                  BloodDrip(ob,x,y);
                  return NO_MOVEMENT;
                  }

               if (!ob->momentumz)
                  return NO_MOVEMENT;
               else// if (ocl != inertobj)
                  return Z_MOVEMENT_ONLY;
               //else
               //goto doors;
               }

            else if (tempwall->which==PWALL)
                  {
                  pwallobj_t*pw;
                  int dx,dy;

                  pw=(pwallobj_t *)tempwall;
                  dx = abs(pw->x - tryx);
                  if (dx > PWALLRAD+0x5000)
                     continue;

                  dy = abs(pw->y - tryy);
                  if (dy > PWALLRAD+0x5000)
                     continue;

                  return NO_MOVEMENT;
                  }

            }
         }

   return OK_TO_CONTINUE;

   }



movement_status CheckStaticObjects(objtype *ob,int tryx,int tryy,int tryz)
  {
  int dx,dy,dzt,dztp1,x,y;
  statobj_t*tempstat;
  int sprrad,oldsrad,sprtrad;
  boolean specialstat=false,widestat=false,zstoppable;
  int sprxlow,sprxhigh,sprylow,spryhigh;
  boolean SPRSTOP;
  classtype ocl;

  ocl = ob->obclass;

  if (ocl != playerobj)
     sprtrad = ACTORSIZE - 0x1000;
  else
     sprtrad = ACTORSIZE - 0x1000 + 0x10000;



  sprxlow = (int)((tryx-sprtrad) >>TILESHIFT);
  sprylow = (int)((tryy-sprtrad) >>TILESHIFT);

  sprxhigh = (int)((tryx+sprtrad) >>TILESHIFT);
  spryhigh = (int)((tryy+sprtrad) >>TILESHIFT);

  if (sprxlow < 0)
     sprxlow = 0;

  if (sprxhigh > (MAPSIZE-1))
     sprxhigh = MAPSIZE-1;

  if (sprylow < 0)
     sprylow = 0;

  if (spryhigh > (MAPSIZE-1))
     spryhigh = MAPSIZE-1;

  SPRSTOP = false;
  sprrad = 0x4500;
  zstoppable = (!(ob->flags & FL_DYING));
  oldsrad = sprrad;

  for (y=sprylow;y<=spryhigh;y++)
      for (x=sprxlow;x<=sprxhigh;x++)
         {
         tempstat = sprites[x][y];
         sprrad = oldsrad;

         if (tempstat)
            {
            specialstat = ((tempstat->itemnumber == stat_heatgrate) ||
                           (tempstat->itemnumber == stat_pit)
                          );

            widestat = (((tempstat->itemnumber >= stat_bcolumn) &&
                        (tempstat->itemnumber <= stat_icolumn)) ||
                        (tempstat->itemnumber == stat_disk)
                       );

            if ((tempstat->flags & FL_BLOCK) || (specialstat==true))
               {
               if ((specialstat==true) && (ocl !=playerobj) &&
                   (ob->state->think != T_Collide)
                  )
                  sprrad += 0x5000;

               if (widestat==true)
                  sprrad += 0x3b00;


               if ((tempstat->itemnumber == stat_ironbarrel) ||
                     (tempstat->itemnumber == stat_bonusbarrel))
                  sprrad += 0x5000;

               dx = abs(tryx - tempstat->x);
               if (dx > sprrad)
                  continue;
               dy = abs(tryy - tempstat->y);
               if (dy > sprrad)
                  continue;

      #define MINSTATZDIFF 58

               dzt = abs(ob->z - tempstat->z);
               dztp1 = abs(tryz - tempstat->z);

               if (widestat && (dztp1 <= MINSTATZDIFF) && zstoppable &&
                   (ocl != b_heinrichobj)
                  )
                  CheckStepping(ob,tempstat,MINSTATZDIFF);


               dztp1 = abs(tryz - tempstat->z);

#if (SHAREWARE == 0)
               if ((ocl == b_darksnakeobj) && (tempstat->itemnumber == stat_heatgrate))
                  {
                  if (ob->state->think == T_DarkSnakeChase)
                     NewState(ob,&s_darkmonkredhead);
                  else
                     NewState(ob,&s_darkmonkredlink);
                  ob->temp3 ++; // make shootable
                  }
#endif

               if (specialstat==true)
                  continue;


               if ((dztp1 >= MINSTATZDIFF) || (dzt >= MINSTATZDIFF))
                  {if ((dzt >= MINSTATZDIFF) && (dztp1 <= MINSTATZDIFF) && zstoppable)
                     {//ob->momentumz = 0;
                     if (ob->z <= tempstat->z)
                        {
                        ob->z = tempstat->z - MINSTATZDIFF;
                        ob->momentumz = 0;
                        }
                     else
                        ob->momentumz = 2*GRAVITY; // ((2*GRAVITY + GRAVITY) >> 16) = 1
                     }
                  continue;
                  }


               if (ocl == boulderobj)
                  {if ((tempstat->itemnumber < stat_bcolumn) ||
                       (tempstat->itemnumber > stat_icolumn)
                      )
                     {
                     tempstat->flags |= FL_SHOOTABLE;
                     DamageThing(tempstat,tempstat->hitpoints);
                     continue;
                     }
#if (SHAREWARE == 0)
                  else
                     NewState(ob,&s_bouldersink1);
#endif
                  }
               //ob->momentumz=0;
               //return false;
               SPRSTOP=true;
               if (!ob->momentumz)
                  return NO_MOVEMENT;
               }
            }
         }
  if (SPRSTOP == true)
     return Z_MOVEMENT_ONLY;

  return OK_TO_CONTINUE;

  }





//============== Platform craziness ======================================


#define ClipHeight(ob,clipz)                                \
{  ob->momentumz = 0;                                       \
                                                            \
   if (ISPLAYER && (ob->z != clipz) && (ob->temp2 == 0))    \
      {playertype *pstate;                                  \
       int dz = ob->z - clipz;                              \
                                                            \
       M_LINKSTATE(ob,pstate);                              \
                                                            \
       pstate->heightoffset = pstate->oldheightoffset + dz; \
       ob->temp2 = (dz >= 0)?(STEPUP):(STEPDOWN);           \
      }                                                     \
                                                            \
   ob->z = clipz;                                           \
}

//======================

#define CheckSpecialGibMovement(blocker)            \
   {                                                \
   int centerx = ((trytilex<<16) + 0x8000);         \
   int centery = ((trytiley<<16) + 0x8000);         \
                                                    \
   if (blocker->vertical==false)                    \
      {                                             \
      int dyt = centery - ob->y;                    \
      int dytp1 = centery - tryy;                   \
                                                    \
      if ((abs(dytp1) > abs(dyt)) &&                \
         (SGN(dyt) == SGN(dytp1))                   \
         )                                          \
         return OK_TO_CONTINUE;                     \
                                                    \
      }                                             \
   else                                             \
      {                                             \
      int dxt = centerx - ob->x;                    \
      int dxtp1 = centerx - tryx;                   \
                                                    \
      if ((abs(dxtp1) > abs(dxt)) &&                \
         (SGN(dxt) == SGN(dxtp1))                   \
         )                                          \
         return OK_TO_CONTINUE;                     \
                                                    \
      }                                             \
   }


movement_status CheckMaskedWalls(objtype *ob,int tryx,int tryy,int tryz)
  {
  int trytilex,trytiley;
  boolean MWALLSTOP;
  int ISPLAYER = (ob->obclass == playerobj);
  classtype ocl = ob->obclass;

  trytilex = (tryx >> TILESHIFT);
  trytiley = (tryy >> TILESHIFT);
  MWALLSTOP = false;
 //for (y=tileylow;y<=tileyhigh;y++)
 // for (x=tilexlow;x<=tilexhigh;x++)

   if (M_ISMWALL(trytilex,trytiley))
       {
       int wall = tilemap[trytilex][trytiley];
       maskedwallobj_t * mw;

       mw=maskobjlist[wall&0x3ff];

       if (ocl == inertobj)
          CheckSpecialGibMovement(mw);

       if (!(mw->flags&MW_BLOCKING))
			  {
			  if (mw->flags&MW_NONDOGBLOCKING)
				  {
				  if ((ocl==playerobj)&&(ob->flags&FL_DOGMODE))
					  {
					  if (ob->z < nominalheight)
                    {
                    MWALLSTOP = true;
                    if (!ob->momentumz)
                       return NO_MOVEMENT;
						  }
					  }
				  else
                 {
                 MWALLSTOP = true;
                 if (!ob->momentumz)
                    return NO_MOVEMENT;
					  }
				  }


           else
				  {
					if (mw->flags & MW_ABOVEPASSABLE)
						{if (mw->flags & MW_MIDDLEPASSABLE) // ==> not bottom
                      {if (ob->z > LOWFALLCLIPZ+MAXSTEPHEIGHT)
                         MWALLSTOP = true;
                       else if (tryz >= LOWFALLCLIPZ)
                         ClipHeight(ob,LOWFALLCLIPZ);
							 }
						 else if (mw->flags & MW_BOTTOMPASSABLE)
                      {if ((ob->z > HIGHFALLCLIPZ+MAXSTEPHEIGHT) && (ob->z < LOWRISECLIPZ))
                         MWALLSTOP = true;
                       else if (ob->z <= HIGHFALLCLIPZ+MAXSTEPHEIGHT)
                         {if (tryz >= HIGHFALLCLIPZ)
                            ClipHeight(ob,HIGHFALLCLIPZ);
                         }
                       else if (tryz <= LOWRISECLIPZ)
								 ob->momentumz = 0;

							 }
						 else // ==> above only
                      {if (ob->z > HIGHFALLCLIPZ+MAXSTEPHEIGHT)
                         MWALLSTOP = true;
                       else if (tryz >= HIGHFALLCLIPZ)
                         ClipHeight(ob,HIGHFALLCLIPZ);
                      }

						}
					else if (mw->flags & MW_MIDDLEPASSABLE)
						{if (mw->flags & MW_BOTTOMPASSABLE) //==> not above passable
                     {if (ob->z >= HIGHRISECLIPZ)
                        {if (tryz <= HIGHRISECLIPZ)
									ob->momentumz = 0;
								}
                      else if (tryz <= HIGHRISECLIPZ)
                        MWALLSTOP = true;
							}

						 else  //==> middle only
                     {if (ob->z > LOWFALLCLIPZ+MAXSTEPHEIGHT)
                         MWALLSTOP = true;
                      else if (tryz >= LOWFALLCLIPZ)
                         ClipHeight(ob,LOWFALLCLIPZ)
                      else
                        {if (ob->z >= HIGHRISECLIPZ)
                          {if (tryz <= HIGHRISECLIPZ)
									  ob->momentumz = 0;
								  }
                         else if (tryz <= HIGHRISECLIPZ)
                           MWALLSTOP = true;
								}
							}

						}
					else // ==> bottompassable only
                  {if (ob->z < LOWRISECLIPZ)
                         MWALLSTOP = true;
                   else if (tryz < LOWRISECLIPZ)
							 ob->momentumz = 0;
						}

				  }
			  }
       else
			 {
          if ( (mw->flags&MW_SHOOTABLE) &&
                (mw->flags&MW_BLOCKINGCHANGES) &&
                (ob->z >= nominalheight)

              )
              {
              int speed=FindDistance(ob->momentumx,ob->momentumy);
              if ((speed>0x2800) && (!(ob->flags & FL_DYING)))
                 {
                 if (ob->obclass == playerobj)
                    {
                    DamageThing(ob,10);
                    Collision(ob,(objtype*)mw,0,0);
                    }
                 UpdateMaskedWall(wall&0x3ff);
                 if (tryz < nominalheight)
                    ob->momentumz = 0;
                 }
              else
                 {
                 MWALLSTOP = true;
                 if (!ob->momentumz)
                    return NO_MOVEMENT;
                 }
              }
          else
              {
              MWALLSTOP = true;
              if (!ob->momentumz)
                 return NO_MOVEMENT;
              }
			 }
       }

  if (MWALLSTOP == true)
    return Z_MOVEMENT_ONLY;

  return OK_TO_CONTINUE;

  }



movement_status CheckDoors(objtype *ob,int tryx,int tryy,int tryz)
  {
  int trytilex,trytiley;
  int ocl;


  trytilex = (tryx >> TILESHIFT);
  trytiley = (tryy >> TILESHIFT);
  ocl = ob->obclass;


  if (M_ISDOOR(trytilex,trytiley))
       {
       doorobj_t*tempdoor;
       int doorn;

       doorn = tilemap[trytilex][trytiley];

       tempdoor = doorobjlist[doorn&0x3ff];
		 if (tempdoor->action == dr_open)
         {

         if (ob->z >= nominalheight)
            {
            if (tryz < nominalheight)
					ob->momentumz = 0;
            return OK_TO_CONTINUE;
				}

         }
       if (ocl == inertobj)
          {
          CheckSpecialGibMovement(tempdoor);
          }
       else if ((ocl == playerobj) || (ocl > b_darksnakeobj))
          return NO_MOVEMENT;
		 else if (ob->state->think != T_Collide)
			{

#define DOOR_LOCKED(door)                                      \
           (((door->flags & DF_ELEVLOCKED) || (door->lock)) && \
             (ob->obclass != b_darianobj)                       \
           )
#define GAS_DOOR(x,y) (MISCVARS->GASON && (MAPSPOT(x,y,1) == GASVALUE))

         if ((!DOOR_LOCKED(tempdoor)) &&
             (!GAS_DOOR(trytilex,trytiley))
            )


            //)
            {
            ob->door_to_open = doorn&0x3ff;
            LinkedOpenDoor(ob->door_to_open);
            if (tempdoor->eindex != -1)
					OperateElevatorDoor(doorn&0x3ff);
				}

			 //if ((nstate = M_S(USE)) != NULL)
			  //{ob->whatever = ob->state;
			 //	NewState(ob,nstate);
			 //	ob->flags |= FL_USE;
			 // }
         return NO_MOVEMENT;
			}
		 else
          return NO_MOVEMENT;
      }
  return OK_TO_CONTINUE;
}

boolean ActorTryMove(objtype*ob,int tryx, int tryy, int tryz)
   {

   movement_status (*reduced_movement_check[3])(objtype*,int,int,int)=
                   {
                   CheckRegularWalls,
                   CheckMaskedWalls,
                   CheckDoors,
                   };


   movement_status (*complete_movement_check[5])(objtype*,int,int,int)=
                   {
                   CheckOtherActors,
                   CheckRegularWalls,
                   CheckStaticObjects,
                   CheckMaskedWalls,
                   CheckDoors,
                   };

   movement_status (**movement_function)(objtype*,int,int,int);
   movement_status movement_check_result;
   int             numcheckfunctions;
   int             i;
   boolean         xyblocked;




   if ((tryz < -30) && (sky==0) && (ob->obclass != inertobj))
      {
      ob->z = -28;
      ob->momentumz = 0;
      return false;
      }

   if ((!InMapBounds(tryx>>16,tryy>>16)) ||
       ((ob->obclass != playerobj) && (IsWindow((tryx>>16),(tryy>>16))))
      )
      return false;

   switch(ob->obclass)
      {
      case inertobj:
      case bladeobj:
      case firejetobj:
         movement_function = &reduced_movement_check[0];
         numcheckfunctions = 3;
         break;

      default:
         movement_function = &complete_movement_check[0];
         numcheckfunctions = 5;
         break;
      }


   for(xyblocked=false,i=0;i<numcheckfunctions;i++)
      {
      movement_check_result = movement_function[i](ob,tryx,tryy,tryz);
      if (movement_check_result == Z_MOVEMENT_ONLY)
         xyblocked = true;

      else if (movement_check_result == NO_MOVEMENT)
         return false;
      }

   if (xyblocked)
      {
      return false;
      }

   return true;
   }

void PushWallMove(int num)
{
 int             tcl;
 pwallobj_t      *pwall;
 int             dx,dy;
 int             actrad;
 objtype         *temp;
 boolean         pushem;
 int             tryx,tryy,areanumber,trytilex,trytiley;


 pwall=pwallobjlist[num];

 actrad = PWALLRAD + 0x5000;
 tryx = (pwall->x + pwall->momentumx);
 tryy = (pwall->y + pwall->momentumy);
 trytilex = (tryx >> 16);
 trytiley = (tryy >> 16);

 areanumber = AREANUMBER(trytilex,trytiley);
 if (areanumber >= 0 && areanumber < NUMAREAS + 1) {
     for(temp=firstareaactor[areanumber];temp;temp=temp->nextinarea)
 {
 tcl = temp->obclass;
	 if (temp->flags & FL_HEAD)  //ignore NME's head and wheels
		continue;

	 if ((temp->flags & FL_DYING) || (!(temp->flags & FL_SHOOTABLE)))
		continue;

    if (tcl > b_darianobj)
       continue;


    dx = abs(tryx - temp->x);
	 if (dx > actrad)
		 continue;

    dy = abs(tryy - temp->y);
	 if (dy > actrad)
		 continue;

	 if (pwall->flags&PW_DAMAGE)
		 {if (!((tcl == playerobj) && (temp->flags & FL_AV)))
			 DamageThing(temp,5);

        Collision(temp,(objtype*)pwall,0,0);
		  M_CheckPlayerKilled(temp);
		  if (temp->flags & FL_DYING)
			 return;

		 }

	 pushem=false;
	 switch (pwall->dir)
			{

#define PWALLTOL (0xc000)

			case north:
            if ((temp->y<pwall->y) && (dx<PWALLTOL))
					pushem=true;
				break;
			case east:
            if ((temp->x>pwall->x) && (dy<PWALLTOL))
					pushem=true;
				break;
			case northeast:
            if ((temp->y<pwall->y) && (dx<PWALLTOL))
					pushem=true;
            else if ((temp->x>pwall->x) && (dy<PWALLTOL))
					pushem=true;
				break;
			case northwest:
            if ((temp->y<pwall->y) && (dx<PWALLTOL))
					pushem=true;
            else if ((temp->x<pwall->x) && (dy<PWALLTOL))
					pushem=true;
				break;
			case south:
            if ((temp->y>pwall->y) && (dx<PWALLTOL))
					pushem=true;
				break;
			case west:
            if ((temp->x<pwall->x) && (dy<PWALLTOL))
					pushem=true;
				break;
			case southeast:
            if ((temp->y>pwall->y) && (dx<PWALLTOL))
					pushem=true;
            else if ((temp->x>pwall->x) && (dy<PWALLTOL))
					pushem=true;
				break;
			case southwest:
            if ((temp->y>pwall->y) && (dx<PWALLTOL))
					pushem=true;
            else if ((temp->x<pwall->x) && (dy<PWALLTOL))
					pushem=true;
				break;
			default:
            //Error ("Pushwall #%d has an illegal direction %d \n",num,pwall->dir);
				break;
			}


	 //if (!pushem)
		//continue;

	 //temp->momentumx = temp->momentumy = 0;
	 if (temp->obclass==playerobj)
			 temp->flags|=FL_PUSHED;

	 if (!pushem)
      {
       Collision(temp,(objtype*)pwall,-temp->momentumx,-temp->momentumy);
		 continue;
		}

	 if ((temp->obclass >= lowguardobj) && (temp->obclass < roboguardobj))
       {

		  temp->momentumx = temp->momentumy = temp->momentumz = 0;
		  temp->hitpoints = 0;

		  if (gamestate.violence >= vl_high)
			 temp->flags |= FL_HBM;
        Collision(temp,(objtype*)pwall,0,0);
		  /*
				 if (gamestate.violence < vl_high)
					{if ((tstate = UPDATE_STATES[CRUSH][temp->obclass - lowguardobj])!=NULL)
						 NewState(temp,tstate);

					 else
						 Error("\n\Null low-violence crush state in push wall crush, instance of %s",debugstr[temp->obclass]);
					}
				 else
					{temp->shapeoffset = 0;
					 //tactor->flags|=FL_HBM;
					 NewState(temp,&s_guts1);
					 //KillActor(temp);
					}*/

		  SD_PlaySoundRTP(SD_ACTORSQUISHSND,temp->x,temp->y);
		 }
	 else
       {
       if (!ActorTryMove(temp,temp->x + temp->momentumx,temp->y + temp->momentumy,
                         temp->z + (temp->momentumz >> 16))
          )
          {
          DamageThing(temp,30);
          if ((temp->obclass==playerobj) && (temp->hitpoints <= 0))
             temp->target = (objtype *)pwall;
          }

       Collision(temp,(objtype*)pwall,pwall->momentumx-temp->momentumx,pwall->momentumy-temp->momentumy);
       M_CheckPlayerKilled(temp);
		 }
	 }
  }
}

void ActorMovement (objtype *ob)
{int tryx,tryy,tryz,limitok,max,friction,ocl;



 if ((ob->obclass == strikeguardobj) && (!(ob->flags & FL_DYING)) &&
     (gamestate.difficulty > gd_easy)
    )
    {
    AvoidPlayerMissile(ob);
    ob->flags &= ~FL_FULLLIGHT;
    }


 if ((!ob->momentumx) && (!ob->momentumy) && (!ob->momentumz))
	{if (ob->flags & FL_RIDING)
		goto ride;
	 else
		return;
	}


 limitok = 1;

 friction = ACTORFRICTION;
 if (!(ob->flags & FL_DYING))
    friction >>= 1;
 ocl = ob->obclass;
 if (ocl == playerobj)
    {
    playertype *pstate;

    M_LINKSTATE(ob,pstate);
    max = pstate->topspeed;
	 friction = PLAYERFRICTION;
	 if ((ob->temp2 == PITFALL) || (ob->temp2 == PITRISE))
       friction >>= 4;
    }


 else if (/*(ob->state->think != T_Collide) &&*/ (ocl != b_robobossobj) &&
		  (ocl != boulderobj) && (ocl !=b_darkmonkobj) && (ocl != b_darksnakeobj) &&
		  (ocl != inertobj) && (ocl != collectorobj))
	 max = MAXMOVE;

 else
  limitok = 0;

 if (limitok)
  {if (ocl == playerobj)
     {int dist,scale;

      dist = FindDistance(ob->momentumx,ob->momentumy);
      if (dist > max)
        {scale = FixedDiv2(max,dist);
         ob->momentumx = FixedMul(ob->momentumx,scale);
         ob->momentumy = FixedMul(ob->momentumy,scale);
        }
     }
   else
     {
      if (ob->momentumx > max)
        ob->momentumx = max;
      else if (ob->momentumx < -max)
        ob->momentumx = -max;
      if (ob->momentumy > max)
        ob->momentumy = max;
      else if (ob->momentumy < -max)
        ob->momentumy = -max;
     }

  }

  tryx = ob->x + ob->momentumx;
  tryy = ob->y + ob->momentumy;
  tryz = ob->z + (ob->momentumz >> 16);

 if (ocl != playerobj)
	 ob->flags &= ~FL_STUCK;


 if (!ActorTryMove (ob, tryx, tryy, tryz))
	{if (ocl == playerobj)
	  {if (!(ob->flags & FL_ELASTO))
			PlayerSlideMove (ob);
		else
        {if (ActorTryMove(ob,tryx, ob->y-ob->momentumy,tryz))
				ob->momentumy = -(ob->momentumy);
         else if (ActorTryMove(ob,ob->x-ob->momentumx,tryy,tryz))
				ob->momentumx = -(ob->momentumx);
			else
				ZEROMOM;
		  }
	  }

	 else
	  {ZEROMOM;
		ob->flags |= FL_STUCK;
		return;
	  }
	}

 MoveActor(ob);


ride:

 if (ob->flags & FL_RIDING)
    {
    objtype *ride = (objtype*)(ob->whatever);

    ob->z += (ride->momentumz >> 16);

    if ((ride->momentumx || ride->momentumy) &&
        ActorTryMove(ob,ob->x+ride->momentumx,ob->y+ride->momentumy,tryz)
       )
       {
          // ActorTryMove already moved us
       }
	 }


#define SLIDER(ob)  ((ob->flags & FL_NOFRICTION) && (ob->state->think != T_Collide))
#define AIRBORNE(ob) ((ob->obclass != playerobj) && (ob->z != nominalheight) &&\
                      (!IsPlatform(ob->tilex,ob->tiley)) && \
                      (DiskAt(ob->tilex,ob->tiley) == NULL) \
                     )

  if (SLIDER(ob) || AIRBORNE(ob))
     return;

  if ( (abs(ob->momentumx) < STOPSPEED) &&
		 (abs(ob->momentumy) < STOPSPEED)
	  )
	  {
	  ZEROMOM;
	  }

  else if ((ob->flags & FL_DYING) && (ob->state == ob->state->next))
	{ob->momentumx = FixedMul (ob->momentumx, DEADFRICTION);
	 ob->momentumy = FixedMul (ob->momentumy, DEADFRICTION);

	}

  else
	{ob->momentumx = FixedMul (ob->momentumx, friction);
	 ob->momentumy = FixedMul (ob->momentumy, friction);

	}



}




void T_Guts(objtype*ob)
{if (ob->ticcount)
  return;
 SpawnParticles(ob,GUTS,50);

}


void T_Special(objtype*ob)
   {
   if (ob->ticcount)
      return;

#if (SHAREWARE == 0)
   if (ob->state == &s_NMEheadexplosion)
      {
      ob->z -= 42;
      SetGibSpeed(0x4000);
      SpawnParticles(ob,gt_sparks,100);
      ResetGibSpeed();
      SD_PlaySoundRTP(SD_EXPLODESND,ob->x,ob->y);
      return;
      }
#endif
   if (ob->obclass != b_robobossobj)
      return;

   NewState(ob,&s_bossdeath);
   }




void SpawnBoulder(int tilex,int tiley,int dir)
{
  #if (SHAREWARE == 1)
    tilex = tilex;
    tiley = tiley;
    dir = dir;
    Error("Boulders aren't allowed in shareware!");
  #endif
#if (SHAREWARE == 0)
  SpawnNewObj(tilex,tiley,&s_boulderspawn,inertobj);
  new->z = 0;
  PreCacheActor(boulderobj,0);
  new->dir = 2*dir;
#endif

}



#define InitSprayPart(newflags)                                          \
   {                                                                     \
   new->hitpoints = starthitpoints[gamestate.difficulty][b_robobossobj]; \
   new->dir = dir*4;                                                     \
   new->speed = 7*SPDPATROL;                                             \
   new->door_to_open = -1;                                               \
   new->flags |= (newflags);                                             \
   }                                                                     \


void SpawnMultiSpriteActor(classtype actorclass, int tilex,int tiley,int dir)
{



#if (SHAREWARE==1)

      actorclass = actorclass;
      tilex = tilex;
      tiley = tiley;
      dir  = dir;
      Error("\nSPRAY not allowed in shareware !");

#else

   {
   objtype *temp;

   gamestate.killtotal++;

   SpawnNewObj(tilex,tiley,&s_NMEstand,actorclass);
   InitSprayPart(FL_BLOCK|FL_NOFRICTION|FL_SHOOTABLE);

   new->temp1 = -1; // temp1 used as one-event queue for directions when chasing
						  // -1 when isn't waiting to try new dir, dirnumber when waiting
	temp = new;

   SpawnNewObj(tilex,tiley,&s_NMEhead1,actorclass);
   InitSprayPart(FL_NOFRICTION|FL_SHOOTABLE|FL_HEAD|FL_NEVERMARK);

	//new->whatever = temp;  // head points to body

	temp->whatever = new;  // body points to head

   SpawnNewObj(tilex,tiley,&s_NMEwheels2,actorclass);
   InitSprayPart(FL_NOFRICTION|FL_SHOOTABLE|FL_HEAD|FL_NEVERMARK);


	//new->whatever = temp;  // head points to body
	temp->target = new;     // body also points to wheels
	actorat[tilex][tiley] = NULL;
   PreCacheActor(b_robobossobj,0);
   }
#endif
}


void SpawnSnake(int tilex,int tiley)
{

#if (SHAREWARE == 1)
   tilex = tilex;
   tiley = tiley;
   Error("snake not allowed in shareware!");
#else

   GetNewActor();
	MakeActive(new);
	new->flags |= (FL_DONE|FL_ABP|FL_NEVERMARK);
   SetTilePosition(new,tilex,tiley);
   SetVisiblePosition(new,new->x,new->y);
	new->obclass = b_darkmonkobj;
	new->which = ACTOR;
	new->z = nominalheight;
	if (SNAKELEVEL == 2)
	 NewState(new,&s_darkmonkfastspawn);
	else
	 NewState(new,&s_darkmonkhspawn);
#endif

}

void SpawnGunThingy(classtype which, int tilex, int tiley, int dir)
   {
#if (SHAREWARE == 1)
   which = which;
   tilex = tilex;
   tiley = tiley;
   dir  = dir;

   Error("no emplacements allowed in shareware!");
#else
   SpawnNewObj(tilex,tiley,&s_gunstand,which);


   if (!loadedgame)
      gamestate.killtotal++;

   PreCacheActor(patrolgunobj,0);

   new->hitpoints = starthitpoints[gamestate.difficulty][which];
   new->dir = dir*2;
//  new->speed = 0x500;
//  ParseMomentum(new,dirangle8[new->dir]);
   new->flags |= (FL_BLOCK|FL_SHOOTABLE);
#endif
}


void SpawnFourWayGun(int tilex, int tiley)
{
#if (SHAREWARE == 1)
   tilex = tilex;
   tiley = tiley;
   Error("no 4-way emplacements allowed in shareware!");
#else


  SpawnNewObj(tilex,tiley,&s_4waygun,patrolgunobj);
  if (!loadedgame)
	  gamestate.killtotal++;

  PreCacheActor(patrolgunobj,0);
  new->temp1 = -1;
  new->hitpoints = starthitpoints[gamestate.difficulty][patrolgunobj]*3;
  new->flags |= (FL_BLOCK|FL_SHOOTABLE);
#endif
}




/*
=======================================================================
=
=                          NON-SHAREWARE CODE
=
=======================================================================
*/

#if (SHAREWARE == 0)

void T_BoulderSpawn(objtype*ob)
{objtype *tactor;
 int dx,dy,cl;

 if (!(ob->flags & FL_ACTIVE))
  return;

 else if (!ob->ticcount)
 {
  if (ValidAreanumber(ob->areanumber)) {
      for(tactor=firstareaactor[ob->areanumber];tactor;tactor=tactor->nextinarea)
	 {cl = tactor->obclass;
	  if (tactor == ob)
		continue;

	  if (!(tactor->flags & FL_SHOOTABLE))
		continue;
	  dx = abs(tactor->x - ob->x);
	  if (dx > MINACTORDIST)
		continue;
	  dy = abs(tactor->y - ob->y);
	  if (dy > MINACTORDIST)
		continue;
	  if ((cl == b_heinrichobj) || (cl== b_darkmonkobj) ||
			(cl == b_darianobj) || (cl == b_robobossobj) ||
			(cl == pillarobj) || (cl == wallopobj) ||
			(cl == boulderobj))
		 return;
	  else break;
	 }
  }


	SpawnNewObj(ob->tilex,ob->tiley,&s_boulderdrop1,boulderobj);
	new->z = 0;
	new->dir = ob->dir;
	//new->angle = dirangle8[new->dir];
	new->speed = 0x4000;
	ParseMomentum(new,dirangle8[new->dir]);
	new->flags |= (FL_BLOCK|FL_NOFRICTION);
	new->flags &= ~FL_SHOOTABLE;
	new->whatever = ob;
	if (tactor)
	 new->target = tactor;
	MakeActive(new);
	new->flags |= FL_ABP;

  }

}

void T_BoulderDrop(objtype*ob)
{int dx,dy,dz;
 objtype * tactor;
 statetype *tstate;


 if (ob->state == &s_boulderdrop12)
  {

	if (ob->z == nominalheight)
	 NewState(ob,&s_boulderroll1);
	else if (ob->momentumz)
	 {ob->z += (ob->momentumz>>16);
	  ob->momentumz += (GRAVITY<<1);
	  if (ob->z > nominalheight)
		 {ob->z = nominalheight;
		  ob->momentumz = 0;
		  //ob->flags &= ~FL_NOFRICTION;
		 }
	 }
	else if (!ob->temp1)
	 {ob->momentumz = (GRAVITY<<6);
	  ob->temp1  = 1;
	 }

  }

 if (ob->ticcount)
  return;
 if (ob->state->condition & SF_SOUND)

  SD_PlaySoundRTP(SD_BOULDERFALLSND,ob->x,ob->y);
 tactor = (objtype*)(ob->target);
 if (tactor && (!(tactor->flags & FL_DYING)))
  {dx = tactor->x - ob->x;
	dy = tactor->y - ob->y;
	dz = tactor->z - ob->z;
	if ((abs(dx) < MINACTORDIST) && (abs(dy) < MINACTORDIST) &&
		 (abs(dz) < 50))
	  {if (tactor->obclass != playerobj)
		  {tactor->momentumx = tactor->momentumy = tactor->momentumz = 0;
			tactor->flags |= FL_DYING;
			tactor->hitpoints = 0;
			if (gamestate.violence < vl_high)
			  {if ((tstate = UPDATE_STATES[CRUSH][tactor->obclass - lowguardobj])!=NULL)
				  NewState(tactor,tstate);

            //else
              //Error("\n\Null low-violence crush state in boulder drop, instance of %s",debugstr[tactor->obclass]);
			  }
			else
			 {tactor->shapeoffset = 0;
			  //tactor->flags|=FL_HBM;
			  NewState(tactor,&s_guts1);
			 }
		  }
		else
		  {DamageThing(tactor,200);
         Collision(tactor,ob,0,0);
			M_CheckPlayerKilled(tactor);
		  }
		SD_PlaySoundRTP(SD_ACTORSQUISHSND,tactor->x,tactor->y);
		ob->target = NULL;
	  }

  }
}


void CheckCrush(objtype*ob)
{
 objtype *temp;
 int dx,dy,dz;

 for(temp = PLAYER[0];temp != PLAYER[numplayers-1]->next;temp=temp->next)
	{
     if (ob->flags & FL_DYING)
       continue;

     dx = abs(temp->x - ob->x);
	  if (dx > MINACTORDIST)
		 continue;

     dy = abs(temp->y - ob->y);
	  if (dy > MINACTORDIST)
		 continue;

     dz = abs(temp->z - ob->z);
	  if (dz > (MINACTORDIST>>10))
		 continue;

	  if (!ob->ticcount)
			DamageThing(temp,EnvironmentDamage(ob));
     Collision(temp,ob,ob->momentumx-temp->momentumx,ob->momentumy-temp->momentumy);
	  M_CheckPlayerKilled(temp);
	}
}


void T_BoulderMove(objtype*ob)
{


 if (MAPSPOT(ob->tilex,ob->tiley,1) == 395)
  {NewState(ob,&s_bouldersink1);
	return;
  }
 if (NOMOM)
  ParseMomentum(ob,dirangle8[ob->dir]);
 if ((!ob->ticcount) && (ob->state->condition & SF_SOUND) &&
	  areabyplayer[ob->areanumber])
  SD_PlaySoundRTP(BAS[ob->obclass].operate,ob->x,ob->y);
 SelectPathDir(ob);

}


/*
=========================================================================
=
=                           Boss Functions
=
=========================================================================
*/

//***************************** Esau ************************************


enum {
  ESAU_USING_HOLES=1,
  ESAU_LEAVING_CONTROL_ROOM,
  ESAU_USING_TOUCH_PEDASTALS,
  ESAU_CHASING_PLAYER
  };




void T_EsauWait(objtype*ob)
   {
   int dist;

   dist = FindDistance(ob->tilex-PLAYER[0]->tilex,ob->tiley-PLAYER[0]->tiley);
   MISCVARS->ESAU_SHOOTING = false;

   if (ob->dirchoosetime)
      ob->dirchoosetime --;

   if ((dist>81) || (dist<36))
      {
      if (CheckLine(ob,PLAYER[0],MISSILE))
         {
         NewState(ob,&s_darianshoot1);
         ob->momentumx = ob->momentumy = 0;
         }
      return;
      }
   else if ((!ob->dirchoosetime) && (CheckLine(ob,PLAYER[0],SHOOT)))
      {
      NewState(ob,&s_dariandefend1);
      ob->dirchoosetime = (GameRandomNumber("T_EsauWait",0) % 35) + 17;//35;
      return;
      }
   }


void T_EsauRise(objtype*ob)
   {
   int newarea,oldarea;

   // if (gamestate.victoryflag)
   // return;

   if (!ob->ticcount)
      {//Debug("\n tx before: %d, ty before: %d",
         //         ob->targettilex,ob->targettiley);

      SelectTouchDir(ob);
      if (ob->targettilex || ob->targettiley)
         {//Debug("\n ob->tilex: %d, ob->tiley: %d, targettilex: %d, targettiley: %d",
            //         ob->tilex, ob->tiley, ob->targettilex, ob->targettiley);

         SetTilePosition(ob,ob->targettilex,ob->targettiley);
         SetVisiblePosition(ob,ob->x,ob->y);
         oldarea = ob->areanumber;
         newarea = AREANUMBER(ob->tilex,ob->tiley);
         if (oldarea != newarea)
            {
            RemoveFromArea(ob);
            ob->areanumber = newarea;
            MakeLastInArea(ob);
            }
         }
      else
         MISCVARS->EPOP[ob->temp3].x = MISCVARS->EPOP[ob->temp3].y = 0;

      ob->dirchoosetime= (GameRandomNumber("T_EsauRise",0) % 35) + 17;
      MISCVARS->ESAU_HIDING = false;
      MISCVARS->ESAU_SHOOTING = true;
      ob->flags |= FL_SHOOTABLE;
      }
   }



void T_EsauChase(objtype*ob)
   {
   int dx,dy,chance,dist;
   statetype *temp;



   if ((ob->tilex == ob->targettilex) && (ob->tiley == ob->targettiley))
      {
      if (MISCVARS->DSTATE == ESAU_USING_HOLES)
         {
         MISCVARS->ESAU_HIDING = true;
         MISCVARS->ESAU_SHOOTING = false;
         SD_PlaySoundRTP(SD_DARIANHIDESND,ob->x,ob->y);
         NewState(ob,&s_dariansink1);
         ob->flags &= ~FL_SHOOTABLE;
         return;
         }
      else if (MISCVARS->DSTATE == ESAU_LEAVING_CONTROL_ROOM)
         {
         if (!MISCVARS->doorcount)
            {
            SetTilePosition(ob,ob->tilex,ob->tiley);
            SetVisiblePosition(ob,ob->x,ob->y);
            }
         MISCVARS->doorcount ++;
         if (MISCVARS->doorcount == 4)
            MISCVARS->DSTATE = ESAU_USING_HOLES;
         else // hack to FORCE esau to walk through door
            {
            switch (ob->temp1)
               {
               case east:
                  ob->targettilex ++;
                  break;
               case west:
                  ob->targettilex --;
                  break;
               case north:
                  ob->targettiley --;
                  break;
               case south:
                  ob->targettiley ++;
                  break;
               }
            }
         SelectTouchDir(ob);
         return;
         }
      }

   if (touchsprite && (touchsprite->itemnumber == stats[stat_dariantouch].type))
      {
      dx = touchsprite->x - ob->x;
      dy = touchsprite->y - ob->y;

      if (((dx > -0x5000) && (dx < 0x5000)) &&
          ((dy > -0x5000) && (dy < 0x5000)))
         {
         SD_PlaySoundRTP(SD_DARIANGONNAUSESND,ob->x,ob->y);
         NewState(ob,&s_darianuse1);
         return;
         }
      }

   if (ob->dirchoosetime)
      ob->dirchoosetime --;

   if (NOMOM || (!ob->dirchoosetime))
      {
      SelectTouchDir(ob);
      ob->dirchoosetime = M_CHOOSETIME(ob);
      }
   else
      ActorMovement(ob);


   if (!ob->ticcount)
      {
      if (CheckLine(ob,PLAYER[0],MISSILE))   // got a shot at player?
         {
         if (Near(ob,PLAYER[0],1))
            chance = 300;
         else
            {
            dx = abs(PLAYER[0]->tilex-ob->tilex);
            dy = abs(PLAYER[0]->tiley-ob->tiley);
            dist = (dx>dy)?dx:dy;
            chance = 400/dist;
            }
         if (GameRandomNumber("T_EsauChase",0) <chance)
            {
            if ((temp=M_S(AIM)) != NULL)
               {
               NewState(ob,temp);
               ob->dirchoosetime = 0;
               ob->momentumx = ob->momentumy = 0;
               SetVisiblePosition(ob,ob->x,ob->y);
               return;
               }
            }
         if (MISCVARS->ESAU_SHOOTING)
            {
            SetVisiblePosition(ob,ob->x,ob->y);
            return;
            }
         }
      }
   }


void T_EsauSpears(objtype*ob)
   {

   if (ob->ticcount == (ob->state->tictime>>1)-1)
      {
      OLDTILEX = PLAYER[0]->tilex;
      OLDTILEY = PLAYER[0]->tiley;
      }

   else if (!ob->ticcount)
      {
      SpawnNewObj(OLDTILEX,OLDTILEY,&s_speardown1,spearobj);
      new->flags |= FL_ABP;
      MakeActive(new);
      }
   }



void FindDoor(objtype*ob)
   {
   int i,area1,area2,min,curr,
       dest1x,dest1y,dest2x,dest2y,
       d1,d2;

   dirtype tdir1,tdir2;
   doorobj_t*dr;

   min = 0x7fffffff;
   for(i=0;i<doornum;i++)
      {
      dr = doorobjlist[i];
      if (dr->vertical)
         {
         area1 = MAPSPOT(dr->tilex-1,dr->tiley,0)-AREATILE;
         dest1x = dr->tilex-1;
         dest1y = dr->tiley;
         tdir1 = east;
         area2 = MAPSPOT(dr->tilex+1,dr->tiley,0)-AREATILE;
         dest2x = dr->tilex+1;
         dest2y = dr->tiley;
         tdir2 = west;
         }
      else
         {
         area1 = MAPSPOT(dr->tilex,dr->tiley-1,0)-AREATILE;
         dest1x = dr->tilex;
         dest1y = dr->tiley-1;
         tdir1 = south;
         area2 = MAPSPOT(dr->tilex,dr->tiley+1,0)-AREATILE;
         dest2x = dr->tilex;
         dest2y = dr->tiley+1;
         tdir2 = north;
         }

//============================================================
#define CheckMinDist(destx,desty,dir)                    \
   {                                                     \
   curr = FindDistance(destx-ob->tilex,desty-ob->tiley); \
   if (curr < min)                                       \
      {                                                  \
      min = curr;                                        \
      ob->targettilex = destx;                           \
      ob->targettiley = desty;                           \
      ob->temp1 = dir;                                   \
      }                                                  \
   }
//============================================================

      if (area1 == ob->areanumber)
         {
         if (area1 == area2)
            {
            d1 = FindDistance(dest1x-ob->tilex,dest1y-ob->tiley);
            d2 = FindDistance(dest2x-ob->tilex,dest2y-ob->tiley);
            if (d2 < d1) //swap areas
               {
               CheckMinDist(dest2x,dest2y,tdir2);
               continue;
               }
            }

         CheckMinDist(dest1x,dest1y,tdir1);

         }
      else if (area2 == ob->areanumber)
         CheckMinDist(dest2x,dest2y,tdir2);

      }
   }


int FindTouch(objtype *ob)
   {
   int i,curr,min,tx,ty,noneleft;
   statobj_t* tempstat;


   min = 0x7fffffff;
   noneleft = 1;
   for(i=0;i<MISCVARS->nexttouch;i++)
      {
      if (MISCVARS->ETOUCH[i].x || MISCVARS->ETOUCH[i].y)
         {
         noneleft = 0;
         tx = MISCVARS->ETOUCH[i].x;
         ty = MISCVARS->ETOUCH[i].y;
         tempstat = sprites[tx][ty];
         curr = FindDistance(tx-ob->tilex,ty-ob->tiley);

         if (curr < min)
            {
            min = curr;
            ob->targettilex = tx;
            ob->targettiley = ty;
            touchsprite = tempstat;
            }
         }
      }
   return (!noneleft);
   }




typedef enum
   {
   down_in_a_hole=-1,
   no_holes_available=0,
   holes_unreachable=1,
   hole_targetted=2

   }hiding_status;



hiding_status HoleStatus(objtype*ob)
   {
   int i,tx,ty,dist,noneleft,invisible,curr,min;
   tpoint dummy,*dptr = &dummy;
   objtype *tactor;
   _2Dpoint *tdptr;

   min = 0x7fffffff;
   noneleft = 1;



   for(i=0;i<MISCVARS->nextpop;i++)
      {
      tdptr = &(MISCVARS->EPOP[i]);

      if (tdptr->x || tdptr->y)
         {
         tactor = (objtype*)actorat[tdptr->x][tdptr->y];
         if (tactor && (tactor->obclass == pillarobj))
            {
            tdptr->x = 0;
            tdptr->y = 0;
            MISCVARS->popsleft --;
            }
         }
      }


   if (MISCVARS->popsleft > 1)
      {
      for(i=0;i<MISCVARS->nextpop;i++)
         {
         tdptr = &(MISCVARS->EPOP[i]);

         if (tdptr->x || tdptr->y)
            {
            tx = tdptr->x;
            ty = tdptr->y;

            if ((PLAYER[0]->tilex == tx) || (PLAYER[0]->tiley == ty))
               continue;

            if (MISCVARS->ESAU_HIDING)
               {
               dist = FindDistance(PLAYER[0]->tilex-tx,PLAYER[0]->tiley-ty);
               if ((ob->tilex == tx) && (ob->tiley == ty) && (MISCVARS->popsleft != 1))
                  continue;
               noneleft = 0;
               if ((MAPSPOT(tx,ty,0)-AREATILE) == ob->areanumber)
                  {
                  ob->targettilex = tx;
                  ob->targettiley = ty;
                  ob->temp3 = i;
                  if ((dist < 81) && (dist > 36))
                     return down_in_a_hole;
                  }
               }

            else if (!MISCVARS->ESAU_SHOOTING)
               {
               curr = FindDistance(tx-ob->tilex,ty-ob->tiley);
               if (curr < min)
                  {
                  min = curr;
                  noneleft = 0;
                  dptr->which = ACTOR;
                  SetTilePosition(dptr,tx,ty);
                  //dptr->x = (tx << TILESHIFT) + TILEGLOBAL/2;
                  //dptr->y = (ty << TILESHIFT) + TILEGLOBAL/2;
                  dptr->z = ob->z;
                  invisible = 0;
                  if ((!CheckLine(ob,dptr,SHOOT)) && (MISCVARS->DSTATE != ESAU_USING_HOLES))
                     {
                     invisible = 1;
                     MISCVARS->DSTATE = ESAU_LEAVING_CONTROL_ROOM;
                     }
                  else
                     MISCVARS->DSTATE = ESAU_USING_HOLES;
                  ob->targettilex = tx;
                  ob->targettiley = ty;
                  }
               }
            }
         }
      }

   if (MISCVARS->ESAU_HIDING)
      return down_in_a_hole;

   if (noneleft)
      {
      MISCVARS->DSTATE = ESAU_CHASING_PLAYER;
      return  no_holes_available;
      }

   if (invisible) //leave present room
      return holes_unreachable;

   return hole_targetted;
   }


void SelectTouchDir (objtype *ob)
   {
   int dx,dy,noneleft,invisible;
   hiding_status hole;


   dirtype d[3];
   dirtype tdir, olddir, turnaround;


   olddir=ob->dir;
   turnaround= opposite[olddir];


   invisible = 0;
   noneleft = 1;

   if (!MISCVARS->notouch)
      {
      if (!FindTouch(ob))
         MISCVARS->notouch = 1;
      else
         MISCVARS->DSTATE = ESAU_USING_TOUCH_PEDASTALS;
      }

   else if ((!MISCVARS->noholes) && (MISCVARS->DSTATE != ESAU_LEAVING_CONTROL_ROOM))
      {
      hole = HoleStatus(ob);

      switch(hole)
         {
         case down_in_a_hole:
            return;

         case no_holes_available:
            MISCVARS->noholes = 1;
            break;

         case holes_unreachable:
            FindDoor(ob);
            break;
            
         default:
            break;
         }
      }

   else if (MISCVARS->DSTATE == ESAU_CHASING_PLAYER)

   // only gets here if all gimmicks (touch tables,
                  // holes) are inoperative
      {
      ob->flags |= FL_SHOOTABLE;
      ob->targettilex = PLAYER[0]->tilex;
      ob->targettiley = PLAYER[0]->tiley;
      }
   /*
   if (DSTATE == SDOOR)
   {dx = ((ob->targettilex<<16)+TILEGLOBAL/2) - ob->x;
      dy = ob->y - ((ob->targettiley<<16)+TILEGLOBAL/2);
      angle = atan2_appx(dx,dy);
      ZEROMOM;
      ParseMomentum(ob,angle);
      ActorMovement(ob);
      if (ob->momentumx || ob->momentumy)
      {ob->angle = angle;
      ob->dir = angletodir[ob->angle];
      return;
      }
   }
   else */
   dx = ob->targettilex - ob->tilex;
   dy = ob->tiley - ob->targettiley;




   d[1]=nodir;
   d[2]=nodir;


   if (dx>0)
      d[1]= east;
   else if (dx<0)
      d[1]= west;
   if (dy>0)
      d[2]=north;
   else if (dy<0)
      d[2]=south;


   if (GameRandomNumber("SelectTouchDir",0)<128)
      {
      tdir=d[1];
      d[1]=d[2];
      d[2]=tdir;
      }

   ZEROMOM;


   if (d[1]!=nodir)
      M_CHECKDIR(ob,d[1]);


   if (d[2]!=nodir)
      M_CHECKDIR(ob,d[2]);



   if (GameRandomNumber("SelectTouchDir",ob->obclass)>128)   //randomly determine direction of search
      {
      for (tdir=north;tdir<=west;tdir++)
         {
         if (tdir!=turnaround)
            M_CHECKDIR(ob,tdir);
         }
      }
   else
      {
      for (tdir=west;tdir>=north;tdir--)
         {
         if (tdir!=turnaround)
            M_CHECKDIR(ob,tdir);
         }
      }

   if (turnaround !=  nodir)
      M_CHECKDIR(ob,turnaround);


   if (olddir!=nodir)
      M_CHECKDIR(ob,olddir);

   }




//************** Krist ****************************************************



void CheckRunover(objtype*ob)
{int dx,dy,dz;

 dx = abs(PLAYER[0]->x - ob->x);
 if (dx > MINACTORDIST)
	return;

 dy = abs(PLAYER[0]->y - ob->y);
 if (dy > MINACTORDIST)
	return;

 dz = abs(PLAYER[0]->z - ob->z);
 if (dz > 10)
	return;

 locplayerstate->heightoffset = 18 + locplayerstate->playerheight;
 locplayerstate->oldheightoffset = locplayerstate->heightoffset;
 PLAYER[0]->temp2 = RENORMALIZE;
 DamageThing(PLAYER[0],30);
 Collision(PLAYER[0],ob,0,0);
 M_CheckPlayerKilled(PLAYER[0]);

}


void T_HeinrichChase(objtype*ob)
{
 int   dx,dy,dist,chance,perpangle;
// statetype *temp;
 boolean doorok;

 CheckRunover(ob);

  //	ob->flags &= ~FL_DODGE;
	if (CheckLine(ob,PLAYER[0],SIGHT))
		{ob->targettilex = PLAYER[0]->x;
		 ob->targettiley = PLAYER[0]->y;
		}

	if (!ob->ticcount)
	 {

//	  if (gamestate.victoryflag)
//		return;


	  if (CheckLine(ob,PLAYER[0],SHOOT))   // got a shot at PLAYER[0]?
		 {dx = abs(ob->tilex - PLAYER[0]->tilex);
		  dy = abs(ob->tiley - PLAYER[0]->tiley);
		  dist = dx>dy ? dx : dy;
		  if (!dist || dist==1)
				chance = 300;
		  else
           chance = 2400/dist;

		  if (GameRandomNumber("T_HeinrichChase",0) <chance)
          {tpoint dummy,*dptr=&dummy;

           if (Near(ob,PLAYER[0],2))
				 goto cdoor;


			  perpangle = AngleBetween(ob,PLAYER[0]) + ANGLES/4;
			  Fix(perpangle);
           dptr->which = ACTOR;
           dptr->x = ob->x + FixedMul(0x10000l,costable[perpangle]);
           dptr->y = ob->y - FixedMul(0x10000l,sintable[perpangle]);

           dptr->z = ob->z;
           if (!CheckLine(dptr,PLAYER[0],SHOOT))
				  goto cdoor;

			  ob->target = PLAYER[0];
			  NewState(ob,M_S(AIM));
			  ob->dirchoosetime = 0;
			  return;


			 }
		 }

	 }

cdoor:
	doorok = NextToDoor(ob);


	if (ob->dirchoosetime)
	 ob->dirchoosetime--;

	if ((ob->flags & FL_STUCK) || (!ob->dirchoosetime) || doorok)
	  {/*if ((ob->flags & FL_DODGE) && (!doorok))
		 SelectKristDodgeDir (ob);
		else */
		 SD_PlaySoundRTP(SD_KRISTMOTORSND,ob->x,ob->y);
		 SelectKristChaseDir(ob);

      ob->dirchoosetime = 4*M_CHOOSETIME(ob);

	  }

	else
	 {if (NOMOM)
		ParseMomentum(ob,dirangle8[ob->dir]);
	  ActorMovement(ob);

	 }

}

void T_Heinrich_Defend (objtype*ob)
{
  CheckRunover(ob);

  if (ob->dirchoosetime)
	ob->dirchoosetime--;


  if (MISCVARS->HRAMMING)
	ParseMomentum(ob,dirangle8[ob->dir]);


  if ((ob->flags & FL_STUCK) || (!ob->dirchoosetime))
	 {if (MISCVARS->HRAMMING)
		 {if (!Near(ob,PLAYER[0],3))
			{NewState(ob,M_S(CHASE));
			 ob->dirchoosetime = 0;
			 return;
			}
		  SelectKristChaseDir(ob);
		 }
	  else if (MISCVARS->HMINING)
		 {SelectMineDir(ob);
		  if (!MISCVARS->HMINING)
			 goto hchase;
		  ob->dirchoosetime = 5;//10;
		  return;
		 }
	  else
  hchase:
		 NewState(ob,M_S(CHASE));
		 ob->dirchoosetime = 0;
	 }
  else
	{if (NOMOM)
	  ParseMomentum(ob,dirangle8[ob->dir]);
	 ActorMovement(ob);
	}
}


void T_Heinrich_Out_of_Control(objtype*ob)
{
   if (ob->dirchoosetime)
      ob->dirchoosetime --;
   else
      {
      if (!ob->temp1)
         {
         SetGibSpeed(0x4000);
         SpawnParticles(ob,RANDOM,120);
         ResetGibSpeed();

         NewState(ob,&s_dexplosion1);
         SD_PlaySoundRTP(SD_EXPLODESND,ob->x,ob->y);
         }
      else
         {
         ob->dir = dirorder[ob->dir][PREV];
         ob->angle = dirangle8[ob->dir];
         if (ob->dir == (unsigned)ob->temp2)
            {
            if (ob->temp1 > 1)
               ob->temp1--;
            else
               {
               if (ob->temp3 == 7)
                  {
                  SpawnNewObj(ob->tilex,ob->tiley,&s_megaexplosions,inertobj);
                  new->temp1 = 25;
                  new->flags |= FL_ABP;
                  MakeActive(new);
                  SpawnNewObj(ob->tilex,ob->tiley,&s_superparticles,inertobj);
                  new->flags |= FL_ABP;
                  PARTICLE_GENERATOR = new;
                  MakeActive(new);
                  }
               if (ob->temp3)
                  ob->temp3 --;
               else
                  ob->temp1 --;
               }
            }

         if (ob->temp1)
            ob->dirchoosetime = ob->temp1;
         else
            {
            ob->dirchoosetime = 70; // end of spin wait for megaexplosion
            if (PARTICLE_GENERATOR)
               {
               NewState(PARTICLE_GENERATOR,&s_megaremove);
               PARTICLE_GENERATOR = NULL;
               }
            }
         }
      }
   }




void SelectKristChaseDir(objtype*ob)
{int dx,dy,tx,ty,angle;
 dirtype dtry1,dtry2,tdir,olddir,next,prev,straight;
 //tpoint dummy,*dptr=&dummy;

 olddir=ob->dir;


 //dptr->which = ACTOR;
 //dptr->z = ob->z;
 if (ob->targettilex || ob->targettiley)
	{tx = ob->targettilex;
	 ty = ob->targettiley;
	 dx= tx - ob->x;
	 dy= ob->y - ty;
   // SetFinePosition(dptr,tx,ty);
	 if ( ((dx < 0x20000) && (dx > -0x20000)) &&
			((dy < 0x20000) && (dy > -0x20000)))
      {
      dx= PLAYER[0]->x-ob->x;
      dy= ob->y-PLAYER[0]->y;
     // SetFinePosition(dptr,PLAYER[0]->x,PLAYER[0]->y);
      }
	}
 else
    {
    dx= PLAYER[0]->x-ob->x;
    dy= ob->y-PLAYER[0]->y;
    //SetFinePosition(dptr,PLAYER[0]->x,PLAYER[0]->y);

	 }

 angle = atan2_appx(dx,dy);
 straight = angletodir[angle];
 /*
 if (ob->areanumber == PLAYER[0]->areanumber)
	  {//tpoint newpos1,newpos2;
		//dirtype leftdir;
		//int leftangle1,leftangle2;

		if (CheckLine(ob,&dummy,DIRCHECK))
		  {//Debug("\ntrying straight dir %d",straight);
			M_CHECKTURN(ob,straight);
			//Debug("\nstraight dir %d failed",straight);
		  }
		//leftdir = dirorder[straight][PREV];
		//leftangle1 = dirangle8[leftdir];
		//newpos1.which = ACTOR;
		//rightangle = dirangle[dirorder[straight][NEXT]];
		//newpos1.x = ob->x + FixedMul(0x10000,costable[leftangle1]);
		//newpos1.y = ob->y - FixedMul(0x10000,sintable[leftangle1]);
		//newpos1.z = ob->z;

		//leftangle2 = dirangle8[dirorder[leftdir][PREV]];
		//newpos2.which = ACTOR;
		//rightangle = dirangle[dirorder[straight][NEXT]];
		//newpos2.x = ob->x + FixedMul(0x10000,costable[leftangle2]);
		//newpos2.y = ob->y - FixedMul(0x10000,sintable[leftangle2]);
		//newpos2.z = ob->z;
		//if (CheckLine(&newpos1,&dummy,SHOOT))// || CheckLine(&newpos2,&dummy,SHOOT))
			{for(tdir = dirorder[straight][PREV];tdir != dirorder[straight][NEXT];tdir = dirorder[tdir][PREV])
				{//Debug("\ntried left-hand rule dir %d",tdir);
				 M_CHECKTURN(ob,tdir);
				}
			}
		//else
		  //{for(tdir = dirorder[straight][NEXT];tdir != dirorder[straight][PREV];tdir = dirorder[tdir][NEXT])
			 // {//Debug("\ntrying right-hand rule dir %d",tdir);
			 //	M_CHECKTURN(ob,tdir);
				//Debug("\nright-hand rule dir %d failed\n",tdir);
		  //	  }
		 // }
	  }
 else*/
	 {dtry1=nodir;
	  dtry2=nodir;

	  if (dx> ACTORSIZE)
		  dtry1= east;
	  else if (dx< -ACTORSIZE)
		  dtry1= west;
	  if (dy> ACTORSIZE)
		  dtry2=north;
	  else if (dy < -ACTORSIZE)
		  dtry2= south;


	  if (abs(dy)>abs(dx))
		 {tdir=dtry1;
		  dtry1=dtry2;
		  dtry2=tdir;
		 }

	  //	ZEROMOM;
	  ob->momentumx = FixedMul (ob->momentumx, DEADFRICTION>>gamestate.difficulty);
	  ob->momentumy = FixedMul (ob->momentumy, DEADFRICTION>>gamestate.difficulty);


	  M_CHECKTURN(ob,straight);

	  if (dtry1 != nodir)
		  M_CHECKTURN(ob,dtry1);

	  if (dtry2 != nodir)
		  M_CHECKTURN(ob,dtry2);

	  if (dtry1 != nodir)
		  {M_CHECKTURN(ob,dirorder[dtry1][NEXT]);
			M_CHECKTURN(ob,dirorder[dtry1][PREV]);
		  }

	  for(tdir = dirorder[olddir][NEXT];tdir != olddir;tdir = dirorder[tdir][NEXT])
		  M_CHECKTURN(ob,tdir);

	  ob->dir = olddir;
	 }



}



void T_KristLeft(objtype*ob)
{CheckRunover(ob);
 ActorMovement(ob);
 if (!ob->ticcount)
  {SD_PlaySoundRTP(SD_KRISTTURNSND,ob->x,ob->y);
	if (ob->dir != (unsigned)ob->temp1)
	 ob->dir = dirorder[ob->dir][NEXT];
	else
	 {ob->temp1 = 0;
	  NewState(ob,&s_heinrichchase);
	 }
  }

}

void T_KristRight(objtype*ob)
{CheckRunover(ob);
 ActorMovement(ob);
 if (!ob->ticcount)
  {SD_PlaySoundRTP(SD_KRISTTURNSND,ob->x,ob->y);
	if (ob->dir != (unsigned)ob->temp1)
	 ob->dir = dirorder[ob->dir][PREV];
	else
	 {ob->temp1 = 0;
	  NewState(ob,&s_heinrichchase);
	 }
  }
}


void T_KristCheckFire(objtype*ob)
{int perpangle,angle;
 tpoint dummy;

 if (!ob->ticcount)
  {angle = AngleBetween(ob,PLAYER[0]);

	if (ob->state == &s_heinrichshoot1)
	 perpangle = angle + ANGLES/4;
	else
	 perpangle = angle - ANGLES/4;

	Fix(perpangle);


	dummy.which = ACTOR;
	dummy.x = ob->x + FixedMul(0x4000,costable[angle]) + FixedMul(0x4000l,costable[perpangle]) +
				 FixedMul(PROJSIZE,costable[perpangle]); // offset ahead plus
				 // offset for left/right missile plus offset for missile
				 // radius (will missile reach player without hitting wall,etc.)

	dummy.y = ob->y - FixedMul(0x4000,sintable[angle]) - FixedMul(0x4000l,sintable[perpangle]) -
				 FixedMul(PROJSIZE,sintable[perpangle]);

	dummy.x -= (FixedMul(PROJSIZE,costable[perpangle])<<1);

	dummy.y += (FixedMul(PROJSIZE,sintable[perpangle])<<1);
	dummy.z = ob->z;

	if (!CheckLine(&dummy,PLAYER[0],SHOOT))
	 {NewState(ob,&s_heinrichchase);
	  return;
	 }


  }
}



void SelectMineDir(objtype*ob)
{int angle,missangle;
 dirtype olddir,tdir,next,prev,destdir;
 static int nummines=0;

 if (!CheckLine(ob,PLAYER[0],SIGHT))
  {NewState(ob,M_S(CHASE));
	MISCVARS->HMINING = 0;
	return;
  }

 olddir = ob->dir;

 angle = AngleBetween(ob,PLAYER[0]);
 tdir = angletodir[angle];
 destdir = opposite[tdir];

 if (destdir != olddir)
  {next = dirorder[olddir][NEXT];
	prev = dirorder[olddir][PREV];
	if (dirdiff[destdir][next] < dirdiff[destdir][prev])
	 ob->dir = next;
	else
	 ob->dir = prev;
	return;
  }

 nummines ++;
 missangle  = angle;
 if (nummines == 2)
  missangle -= (ANGLES/36);
 else if (nummines == 3)
  missangle += (ANGLES/36);

 Fix(missangle);
// if (missangle > (ANGLES - 1))
//  missangle -= ANGLES;
// else if (missangle < 0)
//  missangle += ANGLES;


 SpawnMissile(ob,h_mineobj,0x2000,missangle,&s_mine1,0xa000);
 new->dirchoosetime = 140;
 SD_PlaySoundRTP(SD_KRISTDROPSND,ob->x,ob->y);

 if (nummines == 3)
  {MISCVARS->HMINING = 0;
	nummines = 0;
  }
}



void  A_HeinrichShoot(objtype* ob)
{int angle,perpangle;

 if (!ob->ticcount)
  {angle = AngleBetween(ob,PLAYER[0]);
	if (ob->state == &s_heinrichshoot4)
	 perpangle = angle + ANGLES/4;
	else
	 perpangle = angle - ANGLES/4;

	Fix(perpangle);

	SpawnMissile(ob,missileobj,0x4000,angle,&s_missile1,0x8000);
	SD_PlaySoundRTP(BAS[ob->obclass].fire,ob->x,ob->y);

   SetFinePosition(new,new->x + FixedMul(0x4000l,costable[perpangle]),
                       new->y - FixedMul(0x4000l,sintable[perpangle]));
   SetVisiblePosition(new,new->x,new->y);
  }


}


//***************************///////**************************************
//***************************/ NME /**************************************
//***************************///////**************************************





void UpdateNMELinkedActors(objtype*ob)
   {
   objtype *head,*wheels;
   int oldarea;


   head = (objtype*)(ob->whatever);
   wheels = (objtype*)(ob->target);

   oldarea = head->areanumber;

   SetFinePosition(head,ob->x,ob->y);
   SetFinePosition(wheels,ob->x,ob->y);
   SetVisiblePosition(head,ob->x,ob->y);
   SetVisiblePosition(wheels,ob->x,ob->y);

   if (oldarea != ob->areanumber)
      {
      RemoveFromArea(head);
      head->areanumber = ob->areanumber;
      MakeLastInArea(head);
      RemoveFromArea(wheels);
      wheels->areanumber = ob->areanumber;
      MakeLastInArea(wheels);
      }

   }


void T_OrobotChase(objtype*ob)
   {
   int dx,dy;


   if (CheckLine(ob,PLAYER[0],SIGHT))
      {

      ob->targettilex = PLAYER[0]->tilex;
      ob->targettiley = PLAYER[0]->tiley;
      }



   if (!ob->ticcount)
      {
      if (NMEspincheck(ob))
         return;

      dx = PLAYER[0]->x - ob->x;
      dy = ob->y - PLAYER[0]->y;
      /*
      if ((dx > -0x18000) && (dx < 0x18000) && (dy > -0x18000) && (dy < 0x18000))
         {NewState(ob,&s_NMEavoid);
         return;
         }
      */

      if (CheckLine(ob,PLAYER[0],SIGHT))
         {
         int inrange;

         switch(gamestate.difficulty)
            {
            case gd_baby: inrange = Near(ob,PLAYER[0],6);break;
            case gd_easy: inrange = Near(ob,PLAYER[0],9);break;
            case gd_medium: inrange = Near(ob,PLAYER[0],12);break;
            case gd_hard: inrange = 1;break;
            }

         if ((!Near(ob,PLAYER[0],3)) && inrange)
            {
            SD_PlaySoundRTP(SD_NMEREADYSND,ob->x,ob->y);
            if ((ob->hitpoints < 2000) && (GameRandomNumber("NME special attack",0) < 120))
               {
               int next,prev;

               next = dirorder16[ob->dir][NEXT];
               prev = dirorder16[ob->dir][PREV];
               ob->targettilex = (angletodir[atan2_appx(dx,dy)]<<1);

               if (dirdiff16[prev][ob->targettilex] < dirdiff16[next][ob->targettiley])
                  ob->temp3 = PREV;
               else
                  ob->temp3 = NEXT;
               NewState(ob,&s_NMEspinfire);
               }
            else
               {
               NewState(ob,&s_NMEwindup);
               ob->temp3 = 0;
               }
         //NewState((objtype*)(ob->target),&s_NMEwheelspin);

            NewState((objtype*)(ob->target),&s_NMEwheels120);
            return;
            }
         }
      }

   if (ob->dirchoosetime)
      ob->dirchoosetime --;

   if ((ob->flags & FL_STUCK) || (!ob->dirchoosetime))
      {
      SelectOrobotChaseDir(ob);
      ob->dirchoosetime = 4;//8;
      }

   else
      {
      ActorMovement(ob);
      UpdateNMELinkedActors(ob);
      }
   }



void T_Saucer(objtype*ob)
{int angle,dangle;

 if (!ob->ticcount)  // if on track at end of each state, accelerate
							// towards PLAYER[0]
  {if (ob->state->condition & SF_SOUND)
	  SD_PlaySoundRTP(SD_NMEREADYSND,ob->x,ob->y);
	angle = AngleBetween(ob,PLAYER[0]);
	dangle = ob->angle - angle;
	if ((dangle > -(ANGLES/72)) && (dangle < (ANGLES/72)))
	 {if (ob->speed < 0x10000)
		{ob->speed += 0x200;
		 ZEROMOM;
		 ParseMomentum(ob,ob->angle);
		}
	 }
	else // off track; zero mom. and select new dir.
	 {ob->speed = 0x1000;
	  ZEROMOM;
	  ob->angle = angle;
	  ParseMomentum(ob,ob->angle);
	 }
  }
 MissileMovement(ob);


}


void T_NME_WindUp(objtype*ob)
{objtype *head,*wheels;

 head = (objtype*)(ob->whatever);
 wheels = (objtype*)(ob->target);

 if (ob->dirchoosetime)
  {ob->dirchoosetime--;
	return;
  }

 ob->dirchoosetime = 0;//3;

 if (MISCVARS->NMErotate < 3)
  {head->dir = dirorder16[head->dir][NEXT];
	MISCVARS->NMErotate ++;
  }
 else if (MISCVARS->NMErotate < 6)
  {head->dir = dirorder16[head->dir][PREV];
	MISCVARS->NMErotate ++;
  }
 else if (MISCVARS->NMErotate < 9)
  {ob->dir = dirorder16[ob->dir][NEXT];
	wheels->dir = ob->dir;
	MISCVARS->NMErotate++;
  }
 else if (MISCVARS->NMErotate < 12)
  {ob->dir = dirorder16[ob->dir][PREV];
	wheels->dir = ob->dir;
	MISCVARS->NMErotate ++;
  }
 else
  {MISCVARS->NMErotate = 0;

	NewState(ob,&s_NMEattack);
	ob->dirchoosetime = 0;
	//ob->dirchoosetime = 50 - (ob->shapeoffset >> 2) - (gamestate.difficulty << 2);//70;
	if (!ob->temp2)
	  NewState((objtype*)(ob->whatever),&s_NMEhead1rl);
	else
	  NewState((objtype*)(ob->whatever),&s_NMEhead2rl);
	NewState(wheels,&s_NMEwheels2);
  }

}

#define SPRAYDIST 0x12000

void SelectOrobotChaseDir(objtype*ob)     // this code is for head
   {
   int dx,dy,angle,tx,ty;
   int tdir,olddir,nextdir,prevdir;
   objtype* head,*wheels;



   head = (objtype*)(ob->whatever);
   wheels = (objtype*)(ob->target);
   olddir=head->dir;

findplayer:
   if (ob->temp1 == -1)
      {
      if (ob->targettilex || ob->targettiley)
         {
         tx = (int)((ob->targettilex << TILESHIFT) + HALFGLOBAL1);
         ty = (int)((ob->targettiley << TILESHIFT) + HALFGLOBAL1);
         dx= tx - ob->x;
         dy= ob->y - ty;
         if (((dx <SPRAYDIST ) && (dx > -SPRAYDIST)) &&
             ((dy <SPRAYDIST ) && (dy > -SPRAYDIST)))
            {
            dx= PLAYER[0]->x-ob->x;
            dy= ob->y - PLAYER[0]->y;
            }
         }
      else
         {
         dx= PLAYER[0]->x - ob->x;
         dy= ob->y - PLAYER[0]->y;
         }

      angle = atan2_appx(dx,dy);

      tdir = (((angletodir[angle])<<1) & 0xf);
      }
   else
      {
      tdir = (ob->temp1 & 0xf);

      if ((head->dir == (unsigned)tdir) && (ob->dir == (unsigned)tdir)) // increment
      // tried dir if robot will attempt to move at tdir =>
      // head and body are at move try dir
         {//Debug("\ntrying next queue dir %d",tdir);
         MISCVARS->NMEdirstried ++;
         if (MISCVARS->NMEdirstried == MISCVARS->NMEqueuesize) //gone through all queue entries
            {//Debug("\nqueue exhausted");
            ob->temp1 = -1;
            MISCVARS->NMEdirstried = 0;
            goto findplayer;
            }
         }
      }


   if (tdir != olddir) //rotate head to new chase direction
      {
      nextdir = dirorder16[olddir][NEXT];
      prevdir = dirorder16[olddir][PREV];
      if (dirdiff16[tdir][nextdir] < dirdiff16[tdir][prevdir])
         head->dir = nextdir;
      else
         head->dir = prevdir;
      return;
      }
   //Debug("\nhead aligned to dir %d",tdir);

   //oddir = ob->dir;
   if (ob->dir != head->dir)   // align body and wheels with head
      {
      ZEROMOM;
      NewState(wheels,&s_NMEwheels120);  //rotate wheels for spinning
      nextdir = dirorder16[ob->dir][NEXT];
      prevdir = dirorder16[ob->dir][PREV];
      if (dirdiff16[head->dir][nextdir] < dirdiff16[head->dir][prevdir])
         ob->dir = nextdir;
      else
         ob->dir = prevdir;
      wheels->dir = ob->dir;
      return;
      }

   // Debug("\nbody aligned to head at dir %d",ob->dir);

   ZEROMOM;
   ParseMomentum(ob,dirangle16[head->dir]);
   // Debug("\ntrying to move at dir %d",head->dir);
   ActorMovement(ob);
   UpdateNMELinkedActors(ob);

   if (ob->momentumx || ob->momentumy)
      {
      NewState(wheels,&s_NMEwheels2); // align wheels for movement
      //Debug("\nmove at dir %d succesful, resetting queue",head->dir);
      ob->temp1 = -1; //clear direction queue
      return;
      }
   else if (ob->temp1 == -1) // if queue is empty
                           //make a queue of directions (byte packed)
      {
      //Debug("\nmove at dir %d failed and queue empty",head->dir);
      ob->temp1 = 0;
      MISCVARS->NMEdirstried = 0;
      MISCVARS->NMEqueuesize = 0;

      nextdir = ((tdir + 6) & 0xf);
      prevdir = ((tdir - 6) & 0xf);

      for(; MISCVARS->NMEqueuesize < 6;MISCVARS->NMEqueuesize += 2)
         {
         ob->temp1 <<= 4;
         ob->temp1 += nextdir;
         ob->temp1 <<= 4;
         ob->temp1 += prevdir;
         nextdir = ((nextdir-2) & 0xf);
         prevdir = ((prevdir+2) & 0xf);

         }
   #if 0
      SoftError("\n straight dir: %d\n queue dirs ",tdir);
      for(count = 0;count < MISCVARS->NMEqueuesize;count++)
         {
         SoftError("\n dir %d: %d",MISCVARS->NMEqueuesize-count,
                   ((ob->temp1 >> (4*count)) &0xf)
                  );

         }
   #endif
      }
   else             // else goto next queue dir;
      {
      ob->temp1 >>= 4;
      }

   }



void T_NME_Explode(objtype*ob)
{

 if (ob->ticcount == 35)
	 {objtype*head;
	  int op;

	  head = (objtype*)(ob->whatever);

	  op = FixedMul(GRAVITY,(head->z-25)<<16) << 1;
	  head->momentumz = -FixedSqrtHP(op);
	  head->momentumx = (GameRandomNumber("NME head momx",0) << 2);
	  head->momentumy = (GameRandomNumber("NME head momy",0) << 2);
     head->hitpoints = 0;
     head->flags |= FL_DYING;
	  NewState(head,&s_shootinghead);

			 //RemoveObj((objtype*)(ob->whatever)); // remove head
	 }
 else if (!ob->ticcount)
	 {ob->shapeoffset = 0;
	  NewState(ob,&s_explosion1);
     SetGibSpeed(0x4000);
     SpawnParticles(ob,gt_sparks,200);
     ResetGibSpeed();
	  RemoveObj((objtype*)(ob->target));
	 }

}

void T_NME_HeadShoot(objtype*ob)
{//int randtheta,i,offx,offy;

 ob->z += (ob->momentumz>>16);

 /*if (ob->momentumz < 0)
  {for(i=0;i<3;i++)
	 {randtheta = (GameRandomNumber("NME spark drop",0) << 3);
	  SpawnNewObj(ob->tilex,ob->tiley,&s_particle1,inertobj);
	  new->temp2 = 1;
	  offx = FixedMul(0x400,costable[randtheta]);
	  offy = -FixedMul(0x400,sintable[randtheta]);
	  new->x = new->drawx = ob->x + offx;
	  new->y = new->drawy = ob->y + offy;
	  new->z = ob->z-15;
	  new->flags |= (FL_NOFRICTION|FL_CRAZY|FL_ABP);
	  new->dir = west;
	  MakeActive(new);
	 }
  }*/

 ob->momentumz += GRAVITY;
 if (ob->z >= (nominalheight+45))
  {ob->z = nominalheight+45;
	if (ob->temp2)
	 {ob->momentumz = -30000*ob->temp2;
	  ob->temp2--;
	 }
	else
	 {ob->momentumx = ob->momentumy = ob->momentumz = 0;
	  ob->shapeoffset = 0;
	  NewState(ob,&s_NMEheadexplosion);

	  return;
	 }
  }
 ActorMovement(ob);

}


boolean NMEspincheck(objtype*ob)
   {
   int dx,dy,dz;

   dx = abs(PLAYER[0]->x - ob->x);
   dy = abs(PLAYER[0]->y - ob->y);
   dz = abs(PLAYER[0]->z - ob->z);
   if ((dx < 0x10000) && (dy < 0x10000) && (dz < 32))
      {
      NewState(ob,&s_NMEspinattack);
      NewState((objtype*)(ob->target),&s_NMEwheelspin);
      if (!ob->temp2)
         NewState((objtype*)(ob->whatever),&s_NMEhead1);
      else
         NewState((objtype*)(ob->whatever),&s_NMEhead2);
      ob->dirchoosetime = 1;
      return true;
      }
   return false;
   }





void T_NME_SpinAttack(objtype* ob)
{int mx,my,mz;
 objtype*head,*wheels;



 if (ob->ticcount == 30)  // knock player back
  {GetMomenta(PLAYER[0],ob,&mx,&my,&mz,0x4000);
	DamageThing(PLAYER[0],20);
   Collision(PLAYER[0],ob,mx,my);
	M_CheckPlayerKilled(PLAYER[0]);
  }
 if (ob->dirchoosetime)
  ob->dirchoosetime --;
 else
  {head = (objtype*)(ob->whatever);
	wheels = (objtype*)(ob->target);
	wheels->dir = head->dir = ob->dir = dirorder16[dirorder16[ob->dir][NEXT]][NEXT];

	ob->dirchoosetime = 1;
  }


}


void T_NME_SpinFire(objtype*ob)
{
 int randtheta,oldyzangle,dx,dy,xydist,dz;
 objtype *head,*wheels;


 head = (objtype*)(ob->whatever);
 wheels = (objtype*)(ob->target);

 if (ob->dir != (unsigned)ob->targettilex)
  {ob->dir = head->dir = wheels->dir = dirorder16[ob->dir][ob->temp3];
	return;
  }

 if (ob->dirchoosetime)
  {ob->dirchoosetime --;
	return;
  }

 if (ob->temp3 < 20)
	{//randphi = (GameRandomNumber("NME generate phi",0) << 3) & ((ANGLES/2) -1);
	 if (GameRandomNumber("NME generate theta",0) < 128)
		 randtheta = (GameRandomNumber("NME generate theta",0)>>4);
	 else
		 randtheta = -(GameRandomNumber("NME generate theta",0)>>4);
	 dx = PLAYER[0]->x-ob->x;
	 dy = ob->y-PLAYER[0]->y;
	 if (GameRandomNumber("bcraft shoot up/down",0) < 128)
	  dz = 5;
	 else
	  dz = -5;
	 xydist = FindDistance(dx,dy);
	 randtheta += atan2_appx(dx,dy);
	 Fix(randtheta);
	 oldyzangle = ob->yzangle;
	 ob->yzangle = atan2_appx(xydist,dz<<10);
	 //ob->yzangle = randphi;
	 SD_PlaySoundRTP(BAS[ob->obclass].fire+1,ob->x,ob->y);
	 //wheels->dir = head->dir = ob->dir = dirorder16[dirorder16[ob->dir][NEXT]][NEXT];
	 SpawnMissile(ob,fireballobj,0x6000,randtheta,&s_NMEminiball1,0x10000);
	 ob->dirchoosetime = 1;
	 ob->yzangle = oldyzangle;
	 ob->temp3 ++;
	}
 else
  {ob->temp3 = 0;
	NewState(ob,&s_NMEchase);
	NewState((objtype*)(ob->target),&s_NMEwheels2);
	if (!ob->temp2)
		NewState((objtype*)(ob->whatever),&s_NMEhead1);
	else
		NewState((objtype*)(ob->whatever),&s_NMEhead2);


  }


}

void T_NME_Attack(objtype*ob)
{int angle,perpangle,i;



  if (NMEspincheck(ob))
	{//ob->temp3 = 0;
	 return;
	}
  if (ob->dirchoosetime)
	  {ob->dirchoosetime --;
		return;
	  }


  if (!CheckLine(ob,PLAYER[0],SIGHT))
	{//ob->temp3 = 0;
    //#if ((DEVELOPMENT == 1))
	 //Debug("\nCheckLine failed in NME Attack");
	 //#endif
	 NewState(ob,&s_NMEchase);
	 NewState((objtype*)(ob->target),&s_NMEwheels2);
	 if (!ob->temp2)
		NewState((objtype*)(ob->whatever),&s_NMEhead1);
	 else
		NewState((objtype*)(ob->whatever),&s_NMEhead2);
	 return;
	}
  //sound = BAS[ob->obclass].fire;
  angle = AngleBetween(ob,PLAYER[0]);



  if ((ob->temp3 == 0) || (ob->temp3 == 1)) //heatseek

		{SD_PlaySoundRTP(BAS[ob->obclass].fire+2,ob->x,ob->y);
		 angle = AngleBetween(ob,PLAYER[0]);
       SpawnMissile(ob,missileobj,0x6000,angle,&s_missile1,0x8000);
		 if (ob->temp3 == 3)
			perpangle = angle + ANGLES/4;
		 else
			perpangle = angle - ANGLES/4;
		 Fix(perpangle);

		 new->temp1 = NME_HEATSEEKINGTYPE;
       SetFinePosition(new,new->x + FixedMul(0x8000l,costable[perpangle]),
                           new->y - FixedMul(0x8000l,sintable[perpangle]));
       SetVisiblePosition(new,new->x,new->y);
		 if (!ob->temp3)
			ob->dirchoosetime = 20;
		 else
			{ob->dirchoosetime = 35 - (ob->shapeoffset >> 2) - (gamestate.difficulty << 2);//70;
			 if (!ob->temp2)
				 NewState((objtype*)(ob->whatever),&s_NMEhead1);
			 else
				 NewState((objtype*)(ob->whatever),&s_NMEhead2);
			}
		 ob->temp3 ++;

		}

  else if (ob->temp3 == 2)          // saucer
	  { SpawnMissile(ob,NMEsaucerobj,0x1000,angle,&s_NMEsaucer1,0xc000);
		 new->flags |= FL_SHOOTABLE;
		 ob->temp3++;
		 ob->dirchoosetime = 35 - (ob->shapeoffset >> 2) - (gamestate.difficulty << 2);//70;
		 if (!ob->temp2)
			NewState((objtype*)(ob->whatever),&s_NMEhead1rl);
		 else
			NewState((objtype*)(ob->whatever),&s_NMEhead2rl);
	  }

  else if ((ob->temp3 == 3) || (ob->temp3 == 4))    // drunk
		{SD_PlaySoundRTP(BAS[ob->obclass].fire+2,ob->x,ob->y);
		 if (!ob->temp3)
		  perpangle = angle + ANGLES/4;
		 else
		  perpangle = angle - ANGLES/4;
		 Fix(perpangle);
		 for(i=0;i<(2+gamestate.difficulty);i++)
          {
          SpawnMissile(ob,missileobj,0x6000,angle,&s_missile1,0x8000);
			 new->temp1 = NME_DRUNKTYPE;
          SetFinePosition(new,new->x + FixedMul(0x8000l,costable[perpangle]),
                              new->y - FixedMul(0x8000l,sintable[perpangle]));
          SetVisiblePosition(new,new->x,new->y);
          }

		 if (ob->temp3 == 3)
			ob->dirchoosetime = 20;
		 else
			{ob->temp3 = 0;
			 NewState(ob,&s_NMEchase);
			 if (!ob->temp2)
				NewState((objtype*)(ob->whatever),&s_NMEhead1);
			 else
				NewState((objtype*)(ob->whatever),&s_NMEhead2);
			}

		 ob->temp3 ++;

		}



}



//================== Tom/Snake ============================================





void T_DarkSnakeSpawn(objtype*ob)
   {
   objtype * linkinfront;

   if (((ob->state == &s_darkmonkhspawn) && (!(ob->ticcount%8))) ||
       ((ob->state == &s_darkmonkfastspawn) && (!(ob->ticcount%4))))
      {
      GetNewActor();
      MakeActive(new);
      SetFinePosition(new,ob->x,ob->y);
      SetVisiblePosition(new,ob->x,ob->y);
      new->z = nominalheight;
      new->areanumber = MAPSPOT(new->tilex,new->tiley,0)-AREATILE;
      MakeLastInArea(new);
      new->obclass = b_darksnakeobj;
      new->which = ACTOR;
      new->angle = AngleBetween(ob,PLAYER[0]);
      new->dir = angletodir[new->angle];
      if (SNAKELEVEL == 1)
         new->speed = 0x5000;
      else if (SNAKELEVEL == 2)
         new->speed = 0x5800;
      else
         new->speed = 0x2000;


      new->hitpoints = 1000;
      new->dirchoosetime = 0;
      new->door_to_open = -1;

      new->flags |= (FL_ABP|FL_NOFRICTION|FL_SHOOTABLE|FL_BLOCK);

      if (ob->whatever)
         {
         linkinfront = (objtype*)(ob->whatever);
         linkinfront->whatever = new;
         new->target = linkinfront;
         new->targettilex = linkinfront->x;
         new->targettiley = linkinfront->y;
         new->angle = AngleBetween(new,linkinfront);
         new->dir = angletodir[new->angle];
         new->flags |= FL_NEVERMARK;
         ParseMomentum(new,new->angle);
         NewState(new,&s_darkmonksnakelink);
         }

      else
         {
         SNAKEHEAD = new;
         if (SNAKELEVEL == 3)
            NewState(new,&s_darkmonkhead);
         else if (SNAKELEVEL == 1)
            {
            NewState(new,&s_snakefindpath);
            new->flags |= FL_ATTACKMODE;
            }
         else if (SNAKELEVEL == 2)
            {
            NewState(new,&s_snakepath);
            new->angle = 3*ANGLES/4;
            new->dir = angletodir[new->angle];
            new->flags |= FL_ATTACKMODE;

            }
         ob->targettilex = ob->targettiley = 0;
         ParseMomentum(new,new->angle);
         }

      if (!ob->ticcount)
         SNAKEEND = new;

      ob->whatever = new;

      }
   }


void T_GenericMove(objtype*ob)
{int dx,dy;

 if (ob->temp3 == -1)
  return;


 if (!(SNAKEHEAD->flags & FL_ATTACKMODE))
    return;

 if (ob->hitpoints <= 0)
  {KillActor(ob);
	ob->temp3 = 0;
	return;
  }

 if (!ob->ticcount)
  {if (ob->state == &s_darkmonkredlink)
	  ob->temp3 = 0;
	else if ((ob!=SNAKEEND) && (ob->state == &s_redlinkhit))
	 NewState((objtype*)(ob->whatever),&s_redlinkhit);
  }

 dx = ob->targettilex-ob->x;
 dy = ob->y-ob->targettiley;
 if ((dx > -0xa000) && (dx < 0xa000) && (dy > -0xa000) && (dy < 0xa000))
	{if (ob->temp1 && ob->temp2)
		 {dx = ob->temp1 - ob->x;
		  dy = ob->y - ob->temp2;
		  ZEROMOM;
		 /*
		 if ((ob->targettilex == ob->temp1) && (ob->targettiley == ob->temp2))
			return; */
		  //ob->x = ob->drawx = ob->targettilex;
		  //ob->y = ob->drawy = ob->targettiley;
		  //ob->tilex = ob->x >> TILESHIFT;
		  //ob->tiley = ob->y >> TILESHIFT;
        //#if ((DEVELOPMENT == 1))
		  //	Debug("\nfollower %d being moved to targetx %4x and targety %4x",
		//	  ob-SNAKEHEAD,ob->x,ob->y);
		 // #endif
		  ob->targettilex = ob->temp1;
		  ob->targettiley = ob->temp2;
		  #if (0)
			Debug("\nfollower %d's new targetx %4x, targety %4x",
			 ob-SNAKEHEAD,ob->temp1,ob->temp2);
		  #endif
		  ob->angle = atan2_appx(dx,dy);
		  ob->dir = angletodir[ob->angle];
		  ParseMomentum(ob,ob->angle);
		 }
	}
 else if (NOMOM)
  {//SNAKEHEAD->dirchoosetime = 0;
	ParseMomentum(ob,ob->angle);
  }
 if (ob->momentumx || ob->momentumy)
  MoveActor(ob);

// ActorMovement(ob);

}


/*
===============
=
= SelectSnakeDir
=
===============
*/


void SelectSnakeDir (objtype *ob)
{
 int spot,centerx,centery,dx,dy;

 spot = MAPSPOT(ob->tilex,ob->tiley,1)-ICONARROWS;

 if ((spot >= 0) && (spot<= 7) && ((ob->dir!=(unsigned)spot)||(!(ob->flags & FL_DONE))))
	{ centerx= (ob->tilex << 16) + HALFGLOBAL1;
	  centery= (ob->tiley << 16) + HALFGLOBAL1;
	  dx = abs(centerx - ob->x);
	  dy = abs(centery - ob->y);

	  if ((dx < SNAKERAD) && (dy < SNAKERAD))
	// new direction
		{ZEROMOM;
		 ob->dir = spot;
		 ob->flags |= FL_DONE;
		 ParseMomentum(ob,dirangle8[ob->dir]);
       SetFinePosition(ob,centerx,centery);
       SetVisiblePosition(ob,ob->x,ob->y);

		 if (ob==SNAKEHEAD) {
                     SoftError("\n path changed at %d, %d",ob->tilex,ob->tiley);
                 }
		}
	}

  MoveActor(ob);

}


void T_SnakePath(objtype*ob)
{objtype*temp,*follower;

 if (SNAKEEND && (SNAKELEVEL == 2))
  {if (CheckLine(SNAKEEND,PLAYER[0],SIGHT))
	 {if (ob->temp3 == -1)               //if snake can see player
													 //and he's presently stopped, restart
		{for(temp=ob;temp;temp=(objtype*)(temp->whatever))
		  {temp->temp3 = 0;
			temp->momentumx = temp->temp1;
			temp->momentumy = temp->temp2;
		  }
		 ob->dirchoosetime = 0;
		}
	 }
	else if (ob->temp3 != -1)     //else if he hasn't been stopped, stop him
	 {for(temp=ob;temp;temp = (objtype*)(temp->whatever))
		{temp->temp1 = temp->momentumx;
		 temp->temp2 = temp->momentumy;
		 temp->temp3 = -1;
		 temp->momentumx = temp->momentumy = 0;
		}
	 }
	else
	 return;
  }


 if (ob->dirchoosetime)
  ob->dirchoosetime--;

 else
     {int count = 0;

      for(temp=ob;temp->whatever;temp=(objtype*)(temp->whatever))
			{follower = (objtype*)(temp->whatever);
			 follower->temp1 = temp->x;
			 follower->temp2 = temp->y;

          SoftError("\n follower %d temp1 set to %4x, temp2 set to %4x",
					  count,temp->x,temp->y);
          count ++;
			}
		ob->dirchoosetime = 2 ;//15
	  }

 if (ob->momentumx || ob->momentumy)
  SelectSnakeDir(ob);
 //else
 // {ParseMomentum(ob,ob->angle);
 //	MoveActor(ob);
 // }


}

void FindClosestPath(objtype*ob)
{int tx,ty,dx,dy,angle;


 tx = (ob->targettilex << 16) + TILEGLOBAL/2;
 ty = (ob->targettiley << 16) + TILEGLOBAL/2;

 dx= tx - ob->x;
 dy= ob->y - ty;
 angle = atan2_appx(dx,dy);

 ZEROMOM;
 ParseMomentum(ob,angle);
 MoveActor(ob);

}


void T_SnakeFindPath(objtype*ob)
{int i,dx,dy,currdist,mindist,map;
 tpoint dstruct,*dummy=&dstruct;
 objtype*temp,*follower;

	if (ob->targettilex || ob->targettiley)
	 {FindClosestPath(ob);
	  dx = ob->targettilex - ob->tilex;
	  dy = ob->targettiley - ob->tiley;
	  if ((!dx) && (!dy))
         {SetTilePosition(ob,ob->tilex,ob->tiley);
          SetVisiblePosition(ob,ob->x,ob->y);
			 ob->y = ob->drawy = (ob->tiley << TILESHIFT) + TILEGLOBAL/2;
			 NewState(ob,&s_snakepath);
			 return;
			}
	 }

	else
    {dummy->which = ACTOR;
	  mindist = 0x7fffffff;
	  for(i=0;i<whichpath;i++)
      {
       SetTilePosition(dummy,SNAKEPATH[i].x,SNAKEPATH[i].y);
       dummy->z = ob->z;
       if (CheckLine(ob,dummy,SIGHT))
        {currdist = FindDistance(ob->tilex-dummy->tilex,ob->tiley-dummy->tiley);
			map = MAPSPOT(ob->tilex,ob->tiley,0)-AREATILE;
			if ((currdist < mindist) && (map >= 0) && (map <= NUMAREAS))
          {ob->targettilex = dummy->tilex;
           ob->targettiley = dummy->tiley;
			  mindist = currdist;
			 }
		  }
		}
	 }

	if (ob->dirchoosetime)
	 ob->dirchoosetime--;
	else
	  {for(temp=ob;temp->whatever;temp=(objtype*)(temp->whatever))
			{follower = (objtype*)(temp->whatever);
			 follower->temp1 = temp->x;
			 follower->temp2 = temp->y;
			}
		ob->dirchoosetime = 2 ;//15
	  }
}



void T_SnakeFinale(objtype*ob)
{

   if ((ob->state == &s_snakefireworks1)||(ob->state == &s_snakefireworks2))
      {
      if (ob->z != (maxheight-200))
         {
         ob->z --;
         return;
         }
      SetGibSpeed(0x4500);
      SpawnParticles(ob,RANDOM,100);

      SpawnParticles(ob,gt_spit,100);
      ResetGibSpeed();
      NewState(ob,&s_dexplosion1);
      }

   else
      {
      if (!ob->ticcount)
         {
         NewState(EXPLOSIONS,&s_megaremove);
      //  SpawnParticles(ob,RANDOM,100);
         // SpawnParticles(ob,SPIT,100);
         return;
         }

      if (ob->dirchoosetime)
         ob->dirchoosetime --;
      else
         {
         ob->dirchoosetime = (GameRandomNumber("snake finale choose",0) % 7) + 15;
         SetGibSpeed(0x3000);
         SpawnParticles(ob,RANDOM,30);
         SpawnParticles(ob,gt_spit,20);
         ResetGibSpeed();
         }
      }
   }



void T_DarkSnakeChase(objtype*ob)
   {
   objtype* temp,*follower;
   int tdir,angle;


   if (!(ob->flags & FL_ATTACKMODE))
      {
      if (!(CheckSight(ob,player) || Near(ob,player,4)))
         return;
      else
         {
         ob->flags |= FL_ATTACKMODE;
         MU_StartSong(song_bosssee);
         }

      }



   if (ob->hitpoints <= 0)
      {
      MU_StartSong(song_bossdie);
      KillActor(ob);
      AddMessage("Oscuro defeated!",MSG_CHEAT);
      return;
      }

   angle = AngleBetween(ob,PLAYER[0]);
   tdir = angletodir[angle];
   if (Near(ob,PLAYER[0],6) && (ob->dir == (unsigned)tdir) && (!(ob->state->condition & SF_DOWN)))
      {
      NewState(ob,&s_snakefire1);
      SD_PlaySoundRTP(SD_SNAKEREADYSND,ob->x,ob->y);
      }

   if (!ob->ticcount)
      {
      if (ob->state == &s_darkmonkredhead)
         ob->temp3 = 0; // no longer hitable
      else if ((ob->state == &s_redheadhit) && (ob != SNAKEEND))
         NewState((objtype*)(ob->whatever),&s_redlinkhit);
      else if (ob->state->condition & SF_UP)
         {
         SpawnMissile(ob,dm_spitobj,0x6000,angle,&s_spit1,0x6000);
         SD_PlaySoundRTP(BAS[ob->obclass].fire,ob->x,ob->y);
         //new->z -= 5;
         }
         //spawn spit;
      }

   if (CheckLine(ob,PLAYER[0],SIGHT))
      {
      ob->targettilex = PLAYER[0]->x;
      ob->targettiley = PLAYER[0]->y;
      }

   if (ob->dirchoosetime)
      {
      ob->dirchoosetime--;
      ActorMovement(ob);
      if (NOMOM)
         ob->dirchoosetime = 0;
      }

   else
      {//if (ob)
      for(temp=ob;temp->whatever;temp=(objtype*)(temp->whatever))
         {
         follower = (objtype*)(temp->whatever);
         follower->temp1 = temp->x;
         follower->temp2 = temp->y;
         }
      SelectChaseDir(ob);
      ob->dirchoosetime = 7 ;//15
      }
   }



void T_DarkmonkReact(objtype*ob)
{
 if (ob->z < nominalheight)
	  {MISCVARS->monkz += MZADJUST;
		ob->z = nominalheight + (MISCVARS->monkz >> 16);
		//ob->z++;
		return;
	  }

 else
  {int ocl;

	ocl = ob->temp3;

	if (ocl == p_kesobj)
	  NewState(ob,&s_darkmonkabsorb1);
	else if (ocl == p_heatseekobj)
	  NewState(ob,&s_darkmonkhball1);
	else if (ocl == p_firebombobj)
	  NewState(ob,&s_darkmonkbreathe1);
	else
	  NewState(ob,&s_darkmonkchase1);
	ob->dirchoosetime = 0;
  }

}



void T_DarkmonkCharge(objtype*ob)
{int dx,dy;

 dx = abs(PLAYER[0]->x - ob->x);
 dy = abs(PLAYER[0]->y - ob->y);
 if ((dx < 0xa000) && (dy < 0xa000))
  {DamageThing(PLAYER[0],10);
   Collision(PLAYER[0],ob,0,0);
	M_CheckPlayerKilled(PLAYER[0]);
  }

 if (!ob->ticcount)
  ob->speed >>= 1;

 if (ob->dirchoosetime)
  ob->dirchoosetime --;

 if (NOMOM || (!ob->dirchoosetime))
	{ob->angle = AngleBetween(ob,PLAYER[0]);
	 ob->dir = angletodir[ob->angle];
	 ParseMomentum(ob,ob->angle);
	 ob->dirchoosetime = 5;
	}

 ActorMovement(ob);


}


void T_DarkmonkLandAndFire(objtype*ob)
{

  if (ob->z < nominalheight)
	  {MISCVARS->monkz += MZADJUST;
		ob->z = nominalheight + (MISCVARS->monkz >> 16);
		//ob->z++;
		return;
	  }
  if (Near(ob,PLAYER[0],3))
	 {if (GameRandomNumber("darkmonkland",0)<128)
		NewState(ob,&s_darkmonkbball1);
	  else
		{ob->angle = AngleBetween(ob,PLAYER[0]);
		 ob->dir = angletodir[ob->angle];
		 ob->speed <<= 1;       // goes twice as fast
		 ZEROMOM;
		 ParseMomentum(ob,ob->angle);
		 ob->dirchoosetime = 5; // change dir every 5 tics
		 ob->hitpoints -= 200; // big penalty for charging
		 if (ob->hitpoints <= 0)
			{objtype*column = (objtype*)(ob->whatever);

          EnableObject((long)column);
			 ob->whatever = NULL;

			 KillActor(ob);
			 NewState(ob,&s_darkmonkfastspawn);
          AddMessage("Oscuro flees!",MSG_CHEAT);
			 return;
			}
		 NewState(ob,&s_darkmonkcharge1);
		}

	 }
  else if (ob->temp1)
	 NewState(ob,&s_darkmonklightning1);
  else
	 NewState(ob,&s_dmgreenthing1);
  ob->temp1 ^= 1;
  ob->dirchoosetime = 0;

}


void T_DarkmonkChase(objtype*ob)
{int chance,dx,dy,dist;


  if (!Near(ob,PLAYER[0],2))
	{if (ob->z > (maxheight - 100))
	  {MISCVARS->monkz -= MZADJUST;
		ob->z = nominalheight + (MISCVARS->monkz >> 16);
		//ob->z--;
		return;
	  }
	}
  else if (ob->z < nominalheight)
	  {MISCVARS->monkz += MZADJUST;
		ob->z = nominalheight + (MISCVARS->monkz >> 16);
		//ob->z++;
		return;
	  }


  if (CheckLine(ob,PLAYER[0],SIGHT))
	 {ob->targettilex = PLAYER[0]->x;
	  ob->targettiley = PLAYER[0]->y;
	 }

  if (!ob->ticcount)
	 {
	  if (CheckLine(ob,PLAYER[0],SHOOT))   // got a shot at player?
		 {dx = abs(ob->tilex - PLAYER[0]->tilex);
		  dy = abs(ob->tiley - PLAYER[0]->tiley);
		  dist = dx>dy ? dx : dy;
		  if (!dist || dist==1)
			  chance = 300;//300;
		  else
			  chance = 400/dist;//300/dist;

		  if (GameRandomNumber("T_DarkMonkChase",0) < chance)
			 {NewState(ob,&s_dmlandandfire);
			  return;
			 }
		 }
	 }

	if (ob->dirchoosetime)
	 ob->dirchoosetime--;

	if ((ob->flags & FL_STUCK) || (!ob->dirchoosetime))
	  {SelectChaseDir(ob);
		ob->dirchoosetime = M_CHOOSETIME(ob);
	  }

	else
	 {if (NOMOM)
		ParseMomentum(ob,dirangle8[ob->dir]);
	  ActorMovement(ob);

	 }

}


//====================== End of Boss Functions ===========================//






void T_GunStand(objtype*ob)
{int dy,dx,infrontof,dz;
 objtype* temp;

// if (ob->target)
//  Error("gun reset with non-null target");
 if (ValidAreanumber(ob->areanumber)) {
     for(temp=firstareaactor[ob->areanumber];temp;temp=temp->nextinarea)


	{if (temp == ob)
		continue;
	 if (temp->obclass == ob->obclass)
	  continue;
	 if ((!(temp->flags & FL_SHOOTABLE)) || (temp->flags & FL_DYING))
	  continue;

	 dy = ob->y - temp->y;
	 dx = ob->x - temp->x;
	 dz = ob->z - temp->z;
	 if ((abs(dy)>0x40000) || (abs(dx)>0x40000) || (abs(dz) > 20))
	  continue;


	 infrontof = 0;

	 switch (ob->dir)
	  {case north:
		if ((dy > 0) && (abs(dx)<0x8000))
		  infrontof = 1;
		break;

		case east:
		if ((dx < 0) && (abs(dy)<0x8000))
		  infrontof = 1;
		break;

		case south:
		if ((dy < 0) && (abs(dx)<0x8000))
		  infrontof = 1;
		break;

		case west:
		if ((dx > 0) && (abs(dy)<0x8000))
		  infrontof = 1;
		break;
		
		default:
		break;
	  }

	 if (infrontof && CheckLine(ob,temp,SHOOT))
		{ob->target = temp;
		 NewState(ob,&s_gunraise1);
		 return;
		}
	}
 }
}


void T_4WayGunStand(objtype*ob)
   {
   int dy,dx,dz;
   objtype* temp;

   if (ob->target)
   Error("gun reset with non-null target");
   if (ValidAreanumber(ob->areanumber)) {
       for(temp = firstareaactor[ob->areanumber];temp;temp= temp->nextinarea)
      {
      if (temp == ob)
         continue;

      if (temp->obclass == ob->obclass)
         continue;

      if ((!(temp->flags & FL_SHOOTABLE)) || (temp->flags & FL_DYING))
         continue;

      dy = abs(ob->x-temp->x);
      dx = abs(ob->y-temp->y);
      dz = abs(ob->z-temp->z);
      if ((dx < 0x40000) && (dy < 0x40000) && (dz< 20) && CheckLine(ob,temp,SHOOT))
         {//if ((dx < 0x8000) || (dy <0x8000))
         ob->target = temp;
         NewState(ob,&s_4waygunfire1);
         return;
         }
      }
   }
}

void A_GunShoot(objtype*ob)
{ int   dx,dy,dz,damage,infrontof,tnear,savedangle;
  objtype * target;

 if (!ob->ticcount)
  {target = (objtype*)(ob->target);
   if (!target)
      Error("an instance of %s called gunshoot without a target\n",debugstr[ob->obclass]);
   if ((!(target->flags & FL_SHOOTABLE)) || (target->flags & FL_DYING))
	 {NewState(ob,&s_gunlower1);
	  ob->target = NULL;
	  return;
	 }

   dx = target->x-ob->x;
	dy = ob->y-target->y;
	dz = ob->z-target->z;


	tnear = ((abs(dy)<0x40000) && (abs(dx)<0x40000) && (abs(dz) < 20));
	infrontof = 0;

	 switch (ob->dir)
	  {case north:
		if ((dy > 0) && (abs(dx)<0x8000))
		  infrontof = 1;
		break;

		case east:
		if ((dx < 0) && (abs(dy)<0x8000))
		  infrontof = 1;
		break;

		case south:
		if ((dy < 0) && (abs(dx)<0x8000))
		  infrontof = 1;
		break;

		case west:
		if ((dx > 0) && (abs(dy)<0x8000))
		  infrontof = 1;
		break;
		
		default:
		break;
	  }

	if ((!infrontof) || (!CheckLine(ob,target,SHOOT)) ||
		 (!tnear))
	 {NewState(ob,&s_gunlower1);
	  ob->target = NULL;
	  return;
	 }

	//SD_PlaySoundRTP(SD_FIRE,PLAYER[0]->x,PLAYER[0]->y,ob->x,ob->y);
	//hitchance = 128;

//	if (!target)
  //		Error("object called shoot without a target\n");


	damage = DMG_AHGUN;


	if (target->obclass == playerobj)
	  {target->target = ob;
		if (target->flags & FL_BPV)
		  damage >>= 1;

	  }
	savedangle = ob->angle;
	ob->angle = atan2_appx(dx,dy);
	RayShoot(ob,damage,GameRandomNumber("A_GunShoot Accuracy",0) % 20);
	ob->angle = savedangle;
	SD_PlaySoundRTP(SD_BIGEMPLACEFIRESND,ob->x,ob->y);
  }

}


void A_4WayGunShoot(objtype*ob)
   {
   int   dx,dy,dz,damage,savedangle;
   objtype * target;

   if (ob->ticcount == (ob->state->tictime >> 1))
      {
      target = (objtype*)(ob->target);
      if (!target)
         Error("an instance of %s called 4waygunshoot without a target\n",debugstr[ob->obclass]);
      dx = abs(target->x-ob->x);
      dy = abs(ob->y-target->y);
      dz = abs(ob->z-target->z);
      if ((dx > 0x40000) || (dy > 0x40000) || (dz > 20) ||
          (!CheckLine(ob,target,SHOOT)) ||
          (!(target->flags & FL_SHOOTABLE)) ||
          (target->flags & FL_DYING)
         )
         {
         ob->target = NULL;
         NewState(ob,&s_4waygun);
         return;
         }


      //SD_PlaySoundRTP(SD_FIRE,PLAYER[0]->x,PLAYER[0]->y,ob->x,ob->y);
      //hitchance = 128;

      // if (!target)
      //     Error("object called shoot without a target\n");


      damage = DMG_AHGUN;
      SD_PlaySoundRTP(BAS[ob->obclass].fire,ob->x,ob->y);

      if (target->obclass == playerobj)
         {
         target->target = ob;
         if (target->flags & FL_BPV)
            damage >>= 1;
         }

      savedangle = ob->angle;
      ob->angle = 0;
      RayShoot(ob,damage,GameRandomNumber("A_4WayGunShoot Accuracy",0) % 20);
      ob->angle = ANG90;
      RayShoot(ob,damage,GameRandomNumber("A_4WayGunShoot Accuracy",0) % 20);
      ob->angle = ANG180;
      RayShoot(ob,damage,GameRandomNumber("A_4WayGunShoot Accuracy",0) % 20);
      ob->angle = ANG270;
      RayShoot(ob,damage,GameRandomNumber("A_4WayGunShoot Accuracy",0) % 20);
      ob->angle = savedangle;
      }
   }


void A_Drain (objtype *ob)
   {
   int dx,dy,dz,damage;

	dx = abs(PLAYER[0]->x - ob->x);
	dy = abs(PLAYER[0]->y - ob->y);
	dz = abs(PLAYER[0]->z - ob->z);

	if ((dx > TOUCHDIST) || (dy > TOUCHDIST) || (dz > (TOUCHDIST>>10)))
      {
      NewState(ob,&s_dmonkshoot5);
      return;
      }

	if (ob->ticcount)
      return;

	else
      {
      damage = (GameRandomNumber("A_Drain",ob->obclass) >> 3);
      DamageThing (PLAYER[0],damage);
      ob->hitpoints += damage;
      if (ob->hitpoints > starthitpoints[gamestate.difficulty][ob->obclass])
         ob->hitpoints = starthitpoints[gamestate.difficulty][ob->obclass];

      Collision(PLAYER[0],ob,0,0);
      if (PLAYER[0]->flags & FL_DYING)
         PLAYER[0]->target = ob;
      M_CheckPlayerKilled(PLAYER[0]);
      SD_PlaySoundRTP(SD_MONKGRABSND,ob->x,ob->y);
      }
   }






void  A_DmonkAttack(objtype*ob)
{int angle,nobclass,nspeed,altangle1=0,altangle2=0,zoff=0,sound;
 statetype *nstate;


  if (!ob->ticcount)
     {
     ob->hitpoints -= 120;//120;
     if (ob->hitpoints <= 0)
        {
        objtype*column = (objtype*)(ob->whatever);

        EnableObject((long)column);
        ob->whatever = NULL;

        KillActor(ob);
        NewState(ob,&s_darkmonkfastspawn);
        return;
        }
     }


 if (ob->dirchoosetime)
  ob->dirchoosetime --;

 else
  {sound = BAS[ob->obclass].fire;
	angle = AngleBetween(ob,PLAYER[0]);
   nspeed = 0x6000;

	if (ob->state == &s_darkmonksphere8)
		{nstate = &s_kessphere1;
		 nobclass = p_kesobj;
		 ob->dirchoosetime = 70;
		}

	else if (ob->state == &s_darkmonkhball7)
		{nstate = &s_handball1;
		 nobclass = dm_heatseekobj;
		 nspeed = 0x3000;
		 ob->dirchoosetime = 5;
		}

	else if (ob->state == &s_darkmonkbball7)
		{nstate = &s_faceball1;
		 nobclass = dm_weaponobj;
		 nspeed = 0x3000;
		 ob->dirchoosetime = 5;
		}

	else if (ob->state == &s_darkmonklightning9)
		{nstate = &s_lightning;
		 nobclass = dm_weaponobj;
		 ob->dirchoosetime = 3;
		 sound++;
		}

	else if (ob->state == &s_dmgreenthing8)
		{nstate = &s_energysphere1;
		 nobclass = dm_weaponobj;
		 sound +=2;
		 ob->dirchoosetime = 70;
		}

	else if (ob->state == &s_darkmonkfspark5)
		{nstate = &s_floorspark1;

		 altangle1 = angle + ANGLES/24;
		 altangle2 = angle - ANGLES/24;
		 Fix(altangle1);
		 Fix(altangle2);
		 nobclass = dm_weaponobj;
		 ob->dirchoosetime = 3;
		 sound += 3;
		}

	else if (ob->state == &s_darkmonkbreathe6)
		{nstate = &s_crossfire1;
		 ob->dirchoosetime = 3;
		 nobclass = dm_weaponobj;
		 zoff = -15;
		 sound += 3;
		}

	SpawnMissile(ob,nobclass,nspeed,angle,nstate,0xb000);
	SD_PlaySoundRTP(sound,ob->x,ob->y);

	new->z = ob->z+zoff;
	if (altangle1)
      {
      SpawnMissile(ob,nobclass,nspeed,altangle1,nstate,0xb000);
      SpawnMissile(ob,nobclass,nspeed,altangle2,nstate,0xb000);
      }
  }



}



#endif // SHAREWARE endif




//=====================================================================//




/*
===============
=
= T_Stand
=
===============
*/

void T_Stand (objtype *ob)
   {
   if (!ob->ticcount)
      SightPlayer (ob);
   else
      SoftError("\n ob type %s ticcount of %d in T_Stand",debugstr[ob->obclass],
                ob->ticcount);
   }





void DamagePlayerActor(objtype *ob, int damage)

   {
   playertype *pstate;

   switch (gamestate.difficulty)
      {
      case 0:  damage >>= 1;
               break;
      case 1:  damage -= (damage >> 2);
               break;
      case 2:  break;
      case 3:  //damage += (damage>>2);
               break;
      //default: Error("Um, Gamestate.Difficulty, uh, has problems.\n");
      }

   if (!damage) damage++;

   M_LINKSTATE(ob,pstate);


   pstate->health -= damage;
   ob->hitpoints = pstate->health;

   SD_PlaySoundRTP(SD_PLAYERTCHURTSND+(pstate->player),ob->x,ob->y);
   if (ob==player)
      {
      damagecount += damage;
      if (cybermanenabled)
         SWIFT_TactileFeedback (10*damage, 15, 15);
      if ( SHOW_BOTTOM_STATUS_BAR() )
         DrawBarHealth (false);
      }

   if (pstate->health<=0)
      {
      pstate->health = 0;
      ob->hitpoints = 0;
      }
   }




void DamageNonPlayerActor(objtype *ob,int damage)
   {
   //if ((ob->obclass == b_darksnakeobj) && (!ob->temp3))
     // return;

   if (!(ob->flags & FL_ATTACKMODE))
      damage <<= 1;

   ob->hitpoints -= damage;
   if (ob->hitpoints <= 0)
      {
      int sound;

      sound = BAS[ob->obclass].die;
      if (ob->obclass == lowguardobj)
         {
         if (ob->shapeoffset)
            sound ++;
         }
      SD_PlaySoundRTP(sound,ob->x,ob->y);
      }
   else
      SD_PlaySoundRTP(BAS[ob->obclass].hit,ob->x,ob->y);

#if (SHAREWARE == 0)
   if ((ob->obclass == b_robobossobj) && (ob->temp2 <= 2))
      {
      if (ob->hitpoints <
          ((3-ob->temp2)*starthitpoints[gamestate.difficulty][ob->obclass]>>2)
         )
         {
         SD_PlaySoundRTP(SD_NMEAPARTSND,ob->x,ob->y);
         ob->temp2++;
         ob->shapeoffset += 16;
         ob->speed += 0x500;
         SpawnNewObj(ob->tilex,ob->tiley,&s_megaexplosions,inertobj);
         new->temp1 = 3;
         new->flags |= FL_ABP;
         MakeActive(new);
         }
      if (ob->temp2 == 1)
         NewState((objtype*)(ob->whatever),&s_NMEhead2);
      }
#endif
   MISCVARS->madenoise = true;
   }



void DamageStaticObject(statobj_t*tempstat,int damage)
   {


   tempstat->hitpoints -= damage;
   if (tempstat->hitpoints <= 0)
      {
      sprites[tempstat->tilex][tempstat->tiley]=NULL;
      tempstat->flags |= FL_NONMARK;
      if (tempstat->flags&FL_LIGHT)
         {

         if (MAPSPOT(tempstat->tilex,tempstat->tiley,2))
            {touchplatetype *tplate;

             for(tplate=touchplate[tempstat->linked_to];tplate;tplate = tplate->nextaction)
                if (tplate->whichobj == (long)(tempstat))
                   RemoveTouchplateAction(tplate,tempstat->linked_to);
            }

         if (tempstat->flags & FL_LIGHTON)
            TurnOffLight(tempstat->tilex,tempstat->tiley);


         if (tempstat->itemnumber<=stat_chandelier)
            //SpawnFallingDebris(tempstat->x,tempstat->y,tempstat->z-32);
            {
            objtype *prevlast = LASTACTOR;

            SpawnSlowParticles(gt_sparks,4,tempstat->x,tempstat->y,tempstat->z-32);
            for(prevlast = prevlast->next;prevlast;prevlast= prevlast->next)
               {
               prevlast->momentumz = 1; // any positive value will do
               prevlast->momentumx >>= 1;
               prevlast->momentumy >>= 1;
               }
            }
         else
            {

            SpawnStatic(tempstat->tilex,tempstat->tiley,stat_metalshards,-1);
            LASTSTAT->flags |= (FL_ABP|FL_NONMARK);
            sprites[tempstat->tilex][tempstat->tiley] = NULL;
            MakeStatActive(LASTSTAT);
            switch (tempstat->itemnumber)
               {
               case stat_lamp:
               case stat_altbrazier1:
               case stat_altbrazier2:
               case stat_torch:
                  SpawnSlowParticles(gt_sparks,5,tempstat->x,tempstat->y,tempstat->z-32);
                  break;
               case stat_floorfire:
                  SpawnSlowParticles(gt_sparks,5,tempstat->x,tempstat->y,tempstat->z);
                  break;
	       default:
		   ;
               }
            }
         SpawnSolidStatic(tempstat);
         SD_PlaySoundRTP(SD_ITEMBLOWSND,tempstat->x,tempstat->y);
         }
      else
         {
         switch (tempstat->itemnumber)
            {
            case stat_dariantouch:
               MISCVARS->ETOUCH[tempstat->linked_to].x = MISCVARS->ETOUCH[tempstat->linked_to].y = 0;
            case stat_tntcrate:
            case stat_bonusbarrel:
               SpawnNewObj(tempstat->tilex,tempstat->tiley,&s_staticexplosion1,inertobj);
               MakeActive(new);
               new->flags |= FL_ABP;
               new->whatever = tempstat;
               new->temp2 = damage;

               if (tempstat->itemnumber == stat_bonusbarrel)
                  {
                  int rand = GameRandomNumber("DamageThing",0);

                  if (rand < 80)
                     {
                     if (rand & 1)
                        SpawnStatic(tempstat->tilex,tempstat->tiley,stat_monkmeal,-1);
                     else
                        SpawnStatic(tempstat->tilex,tempstat->tiley,stat_priestporridge,-1);
                     gamestate.healthtotal ++;
                     }
                  else if (rand < 160)
                     {
                     if (rand & 1)
                        SpawnStatic(tempstat->tilex,tempstat->tiley,stat_lifeitem1,-1);
                     else
                        SpawnStatic(tempstat->tilex,tempstat->tiley,stat_lifeitem3,-1);
                     }
                  else
                     {
                     if (rand & 1)
                        SpawnStatic(tempstat->tilex,tempstat->tiley,stat_mp40,-1);
                     else
                        {
                        SpawnStatic(tempstat->tilex,tempstat->tiley,stat_heatseeker,-1);
                        gamestate.missiletotal ++;
                        }
                     LASTSTAT->flags &= ~FL_RESPAWN;
                     }
                  //LASTSTAT->flags &= ~FL_SHOOTABLE;
                  LASTSTAT->flags |= FL_ABP;
                  MakeStatActive(LASTSTAT);
                  SD_PlaySoundRTP(SD_BONUSBARRELSND,tempstat->x,tempstat->y);
                  }
               else
                  {
                  ExplodeStatic(tempstat);
                  if (tempstat == touchsprite)
                     touchsprite = NULL;
                  }
               SpawnSolidStatic(tempstat);
         //SD_Play(SD_EXPL);
               break;
   #if (SHAREWARE == 0)
            case stat_mine:
               SpawnNewObj(tempstat->tilex,tempstat->tiley,&s_grexplosion1,inertobj);
               MakeActive(new);
               new->flags |= FL_ABP;
               new->whatever = tempstat;
               new->temp2 = damage;
               RemoveStatic(tempstat);
               break;

            case stat_tomlarva:
               SD_PlaySoundRTP(SD_ACTORSQUISHSND,tempstat->x,tempstat->y);
               SpawnGroundExplosion(tempstat->x,tempstat->y,tempstat->z);
               //MISCVARS->gibgravity = GRAVITY/2;
               MISCVARS->fulllightgibs = true;
               SetGibSpeed(0x4000);
               SpawnSlowParticles(GUTS,30,tempstat->x, tempstat->y,tempstat->z);
               ResetGibSpeed();
               MISCVARS->fulllightgibs = false;
               //MISCVARS->gibgravity = -1;
               RemoveStatic(tempstat);
               break;
   #endif

            case stat_lifeitem1:
            case stat_lifeitem2:
            case stat_lifeitem3:
            case stat_lifeitem4:
               SD_PlaySoundRTP(SD_ITEMBLOWSND,tempstat->x,tempstat->y);
               gamestate.treasurecount ++;
               SpawnSlowParticles(gt_sparks,10,tempstat->x,tempstat->y,tempstat->z);
               SpawnSolidStatic(tempstat);
               break;

            default:

               if ((tempstat->itemnumber == stat_plant) ||
                     (tempstat->itemnumber == stat_tree))
                  gamestate.plantcount++;

               //tempstat->shapenum = -1;
               //tempstat->flags &= ~FL_SHOOTABLE;
               ExplodeStatic(tempstat);
               SpawnSolidStatic(tempstat);
               break;
            }
         }
      }
   }


void DamageThing (void *thing, int damage)
   {
   objtype* tempactor;
   statobj_t* tempstat;


   tempactor = (objtype*)thing;
   if (!tempactor)
      return;

   if ((tempactor->which == ACTOR) && (!(tempactor->flags & FL_SHOOTABLE)))
      return;

   if ((tempactor->which == ACTOR) || (tempactor->which == SPRITE))
      {
      if (tempactor->which == ACTOR)
         {
         if (tempactor->obclass == playerobj)
            {
            if ((tempactor->flags & FL_GODMODE) ||
                (tempactor->flags & FL_DOGMODE) ||
                godmode ||
                (gamestate.battlemode == battle_Eluder)
               )
               return;
            DamagePlayerActor(tempactor,damage);
            }

         else
            {
            if ((tempactor->obclass == collectorobj) && (gamestate.SpawnEluder))
               return;
            if (tempactor->hitpoints <= 0)
               return;
            DamageNonPlayerActor(tempactor,damage);
            }
         }
      else
         {
         tempstat = (statobj_t*)thing;

         MISCVARS->madenoise = true;
         if (!(tempstat->flags & FL_SHOOTABLE))
            return;
         DamageStaticObject(tempstat,damage);
         }
      }
   }


void ExplodeStatic(statobj_t*tempstat)
{
 //SpawnSolidStatic(tempstat);

 if (tempstat->flags & FL_WOODEN)
   {
   SpawnStatic(tempstat->tilex,tempstat->tiley,stat_woodfrag,-1);
   if ((gamestate.BattleOptions.RespawnItems) &&
       (tempstat->itemnumber == stat_tntcrate)
      )
      {
      tempstat->linked_to = (long)(LASTSTAT);
      tempstat->flags |= FL_RESPAWN;
      }

   }
 else if (tempstat->flags & FL_METALLIC)
	SpawnStatic(tempstat->tilex,tempstat->tiley,stat_metalfrag,-1);
 else
	SpawnStatic(tempstat->tilex,tempstat->tiley,stat_rubble,-1);
 LASTSTAT->flags |= (FL_ABP|FL_NONMARK);
 sprites[tempstat->tilex][tempstat->tiley] = NULL;
 MakeStatActive(LASTSTAT);
 SD_PlaySoundRTP(SD_ITEMBLOWSND,tempstat->x,tempstat->y);


}





void EnableObject(long object)
   {
   objtype* ob;
   int i,gasicon;
   doorobj_t*tdoor;

   ob = (objtype*)object;

#if (BNACRASHPREVENT == 1)//
		if (ob == 0){return;}
#endif

   ob->flags |= FL_ACTIVE;
   if (ob->obclass == bladeobj)
      {
      ParseMomentum(ob,dirangle8[ob->dir]);
      if (ob->whatever)
         {
         objtype *passenger=(objtype*)(ob->whatever);

         passenger->momentumx += ob->momentumx;
         passenger->momentumy += ob->momentumy;
         }
      }

   if (ob->obclass == gasgrateobj)
      {
      NewState(ob,&s_gas2);
      SD_PlaySoundRTP(SD_GASSTARTSND,ob->x,ob->y);
      ob->dirchoosetime = GASTICS;
      for(i=0;i<doornum;i++)
         {

         tdoor = doorobjlist[i];
         gasicon = MAPSPOT(tdoor->tilex,tdoor->tiley,1);
         if (gasicon == GASVALUE)
            LinkedCloseDoor(i);
         }

      MU_StoreSongPosition();
      MU_StartSong(song_gason);
      MISCVARS->GASON = 1;
      ob->temp3 = 105;
      }
   else if (ob->obclass == pillarobj)
      {
      ob->flags |= FL_FLIPPED;
      gamestate.secretcount++;
      }
   if (!(ob->flags & FL_ABP))
      {
      ob->flags |= FL_ABP;
      MakeActive(ob);
      }
   }

void DisableObject(long object)
{objtype*ob;

 ob = (objtype*)object;
 ob->flags &= ~FL_ACTIVE;
}


void T_MoveColumn(objtype* ob)
{int spot,index;


 if (!(ob->flags & FL_ACTIVE))
  return;
/*
 switch (ob->dir)
  {case north:
	 ob->momentumy = -PILLARMOM;
	 break;
	case south:
	 ob->momentumy =  PILLARMOM;
	 break;
	case east:
	 ob->momentumx =  PILLARMOM;
	 break;
	case west:
	 ob->momentumx = -PILLARMOM;
	 break;
  }

*/
 ActorMovement(ob);
 index = touchindices[ob->tilex][ob->tiley];
 if (index)
  TRIGGER[index-1] = 1;
 ob->temp1 -= PILLARMOM;

 if ((ob->temp1 <= 0) || NOMOM)
  {ZEROMOM;
	ob->temp1 = 0x20000;
	ob->flags &= ~FL_ACTIVE;
	spot = MAPSPOT(ob->tilex,ob->tiley,1)-ICONARROWS;
	if ((spot >= 0) && (spot <= 7))
	 {ob->dir = spot;
     if (!ob->temp2)
        {
        gamestate.secrettotal++;
        }
     else
        {
        ob->flags |= FL_ACTIVE;
        }
	  ParseMomentum(ob,dirangle8[ob->dir]);
	 }
	else
	 ob->flags |= FL_DONE;

  }
}



boolean NextToDoor(objtype*ob)
{
  int tilex,tiley,centerx,centery,dx,dy;

  tilex = ob->tilex;
  tiley = ob->tiley;


  if (M_ISDOOR(tilex+1,tiley) || M_ISDOOR(tilex-1,tiley))
	{centery = (tiley << TILESHIFT) + HALFGLOBAL1;
	 dy = abs(ob->y - centery);
	 if (dy < 0x2000)
		return true;
	}

  if (M_ISDOOR(tilex,tiley+1) || M_ISDOOR(tilex,tiley-1))
	{centerx = (tilex << TILESHIFT) + HALFGLOBAL1;
	 dx = abs(ob->x - centerx);
	 if (dx < 0x2000)
		return true;
	}

  return false;
}


/*
=================
=
= T_Use
=
=================
*/


void T_Use(objtype*ob)
{if (ob->ticcount)
	return;

 switch (ob->obclass)
  {
#if (SHAREWARE == 0)
   case b_darianobj:
    if (!DoPanicMapping())
       touchsprite->flags |= FL_ACTIVE;
	 if ((!sprites[PLAYER[0]->tilex][PLAYER[0]->tiley]) && (ob->areanumber == PLAYER[0]->areanumber))
	  {SpawnNewObj(PLAYER[0]->tilex,PLAYER[0]->tiley,&s_dspear1,spearobj);
		new->flags |= (FL_ABP);//|FL_INVULNERABLE);
		new->z = 0;
		MakeActive(new);
	  }
	 ZEROMOM;
	 ob->flags |= FL_STUCK;
	 SD_PlaySoundRTP(SD_DARIANUSESND,ob->x,ob->y);
	 //NewState(ob,&s_darianspears);
	 break;
#endif
  default:
      ;
  }


}





#define RollStart(ob,state,angle)     \
   {                                  \
   int oldspeed = ob->speed;          \
                                      \
   ob->speed = ROLLMOMENTUM+0x200;    \
   NewState(ob,state);                \
   ParseMomentum(ob,angle);           \
   ob->speed = oldspeed;              \
   }                                  \





void AvoidPlayerMissile(objtype*ob)
   {
   objtype *temp;
   int dx,dy,dz;
   int magangle,angle1,rollangle1,rollangle2,dangle1,dangle2;

   if (PLAYER0MISSILE == NULL)
      return;

   if (GameRandomNumber("scott missile avoid",0) > 160)
      return;

   if (ob->momentumz)
      return;

   if ((ob->state->think == T_Roll) || (ob->state->think == T_Reset))
      return;

   temp = PLAYER0MISSILE;

   dx = abs(temp->x - ob->x);
   dy = abs(ob->y - temp->y);
   dz = abs(ob->z - temp->z);
   angle1 = AngleBetween(temp,ob);
   magangle = abs(temp->angle - angle1);

   if (magangle > VANG180)
      magangle = ANGLES - magangle;


   if ((magangle > ANGLES/48) || (dx > 0x50000) || (dy > 0x50000) ||
       (dz > 32))
      return;

   rollangle1 = angle1 + ANGLES/4;
   Fix(rollangle1);
   dangle1 = abs(temp->angle - rollangle1);
   if (dangle1 > VANG180)
      dangle1 = ANGLES - dangle1;

   rollangle2 = angle1 - ANGLES/4;
   Fix(rollangle2);
   dangle2 = abs(temp->angle - rollangle2);
   if (dangle2 > VANG180)
      dangle2 = ANGLES - dangle2;

   ob->momentumx = ob->momentumy = 0;

   if (dangle1 > dangle2)
      {
      RollStart(ob,&s_strikerollleft1,rollangle1);
      }
   else
      {
      RollStart(ob,&s_strikerollright1,rollangle2);
      }
   ob->flags |= FL_NOFRICTION;

   ob->target = PLAYER[0];
   //SelectRollDir(ob);


   }




/*
=================
=
= T_Chase
=
=================
*/

void T_Chase (objtype *ob)
{
	int   dx,dy,dz,dist,chance;
	classtype ocl;
	statetype *temp;
	boolean doorok;

	ocl = ob->obclass;


   if ((ocl == deathmonkobj) || (ocl == blitzguardobj))
	 {dx = abs(PLAYER[0]->x - ob->x);
	  dy = abs(ob->y - PLAYER[0]->y);
	  dz = abs(ob->z - PLAYER[0]->z);

	  if ((dx < TOUCHDIST) && (dy < TOUCHDIST) && (dz < (TOUCHDIST >> 10)))
       {
#if (SHAREWARE == 0)
        if (ocl == deathmonkobj)
           {NewState(ob,&s_dmonkshoot1);
            STOPACTOR(ob);
            return;

           }
        else
#endif
          if ((!ob->temp3) && (PLAYERSTATE[0].missileweapon != -1) &&
                 (PLAYERSTATE[0].missileweapon < wp_godhand))
           {NewState(ob,&s_blitzsteal1);
            STOPACTOR(ob);
            return;
           }
       }
	 }

	ob->flags &= ~FL_DODGE;

	//if (CheckLine(ob,PLAYER[0],DIRCHECK) && (ocl != roboguardobj))

	if (!ob->ticcount)
	 {if (CheckLine(ob,PLAYER[0],SIGHT))   // got a shot at player?
		 {if (ocl != roboguardobj)
			 {ob->targettilex = PLAYER[0]->x;
			  ob->targettiley = PLAYER[0]->y;
			 }
		 }

     if (CheckLine(ob,PLAYER[0],SHOOT) && (!(player->flags&FL_DYING)))   // got a shot at player?
		 {
		  dx = abs(ob->tilex - PLAYER[0]->tilex);
		  dy = abs(ob->tiley - PLAYER[0]->tiley);
		  dist = (dx>dy)?(dx):(dy);
		  if ((!dist) || (dist==1))
				chance = 300;
		  else if (ocl >= b_darianobj)
				chance = 400/dist;
		  else
				chance = 300/dist;

		  if (GameRandomNumber("T_Chase",ocl) <chance)
			 {if ((ocl == b_heinrichobj) && (Near(ob,PLAYER[0],4)))
				 goto cdoor;

			  ob->dir = angletodir[AngleBetween(ob,PLAYER[0])];
			  STOPACTOR(ob);
#if (SHAREWARE == 0)
			  if ((ocl == overpatrolobj) && (!Near(ob,PLAYER[0],3)) &&
					(!PLAYERSTATE[0].NETCAPTURED) && (!MISCVARS->NET_IN_FLIGHT))
				  {NewState(ob,&s_opbolo1);
					MISCVARS->NET_IN_FLIGHT = true;
					return;
				  }
#endif
			  if ((ocl == triadenforcerobj) && (!Near(ob,PLAYER[0],3)))
				 {NewState(ob,&s_enforcerthrow1);
				  return;
				 }

			  if ((temp=M_S(AIM)) != NULL)
				 {if ((ob->flags & FL_HASAUTO) && (!ob->temp3))
					 ob->temp3 = (GameRandomNumber("T_Chase FL_HASAUTO",ocl) % 5) + 3;
				  ob->target = PLAYER[0];
				  NewState(ob,temp);
				  return;
				 }

			 }
		  //if ((CheckSight(ob,PLAYER[0])) && (!ob->angle))// &&
				//(ocl != b_heinrichobj))
			 //ob->flags |= FL_DODGE;
		 }

	 }
cdoor:
	doorok = NextToDoor(ob);


	if (ob->dirchoosetime)
	  ob->dirchoosetime--;

	if ((ob->flags & FL_STUCK) || (!ob->dirchoosetime) || doorok)
	  {//if (ob->flags & FL_DODGE)
		// SelectDodgeDir(ob);
		//else
		SelectChaseDir(ob);
		ob->dirchoosetime = M_CHOOSETIME(ob);
	  }

	else
	  {if (NOMOM)
		  ParseMomentum(ob,dirangle8[ob->dir]);
		ActorMovement(ob);
	  }
}



void SpawnMissile(objtype* shooter,classtype nobclass,int nspeed,int nangle,statetype*nstate,int offset)
   {
   GetNewActor();
   MakeActive(new);
   new->which = ACTOR;
   new->obclass = nobclass;
   new->angle = nangle;
   new->speed = nspeed;
   if (shooter->obclass == playerobj)
      offset += FindDistance(shooter->momentumx,shooter->momentumy);

   SetFinePosition(new,shooter->x + FixedMul(offset,costable[nangle]),
                      shooter->y - FixedMul(offset,sintable[nangle]));
   SetVisiblePosition(new,new->x,new->y);
  //SoftError("\n missx:%d, missy:%d, speed:%d, offset:%d, angle%d, drawx:%d, drawy:%d",
    //         new->x,new->y,nspeed,offset,nangle,new->drawx,new->drawy);

   new->z = shooter->z;
   new->areanumber = shooter->areanumber;
   new->soundhandle = -1;
   if (nobclass != inertobj)
      {
      MakeLastInArea(new);
      if (MissileSound == true)
         new->soundhandle = SD_PlaySoundRTP(BAS[new->obclass].operate,new->x,new->y);
      }

   if ((shooter->obclass == playerobj) || (shooter->obclass == wallopobj) ||
       (shooter->obclass == b_robobossobj))
      {
      Set_3D_Momenta(new,new->speed,new->angle,shooter->yzangle);

      if (nobclass == p_drunkmissileobj)
         new->temp1 = new->momentumz;

      new->z -= FixedMulShift(offset,sintable[shooter->yzangle],26);
      if ((shooter->obclass == playerobj) && (shooter->flags & FL_GODMODE))
         new->z -= 10;
      }
   else
      ParseMomentum(new,new->angle);

   if (nobclass == p_drunkmissileobj)
      new->temp3 = VBLCOUNTER/3;


   new->flags |= (FL_NEVERMARK|FL_ABP|FL_NOFRICTION|FL_FULLLIGHT);
   new->whatever = shooter; // keep track of missile possession
   NewState(new,nstate);
   }



/*
=================
=
= T_Roll
=
=================
*/


void T_Roll (objtype *ob)
{
  ActorMovement(ob);

}




/*
===============
=
= T_Path
=
===============
*/

void T_Path (objtype *ob)
   {int dx,dy,dz,ocl,damage=1;
   objtype*temp,*ttarg,*twhat;


   ocl = ob->obclass;

   if (((ocl == firejetobj) || (ocl == bladeobj)) && (!ob->ticcount))
      {
      if (ocl == bladeobj)
         {
         if (ob->state->condition & SF_DOWN )
            ob->flags &= ~FL_BLOCK;
         else if (ob->state->condition & SF_UP)
            {
            ob->flags |= FL_BLOCK;
            damage = 0;
            }
         }

      if ((ob->state->condition & SF_SOUND) && areabyplayer[ob->areanumber])
         SD_PlaySoundRTP(BAS[ob->obclass].operate,ob->x,ob->y);

      if (damage)
         {
         if (ValidAreanumber(ob->areanumber)) {
             for(temp=firstareaactor[ob->areanumber];temp;temp=temp->nextinarea)
                {
                if (temp == ob)
                   continue;

                if (temp->obclass >= roboguardobj)
                   continue;

            //WAS
            ttarg = (objtype*)(temp->target);
            twhat = (objtype*)(temp->whatever);

            if ((M_ISACTOR(ttarg) && (ttarg->obclass == b_robobossobj)) ||
                (M_ISACTOR(twhat) && (twhat->obclass == b_robobossobj))
               )
               continue;

            if ((!(temp->flags & FL_SHOOTABLE)) || (temp->flags & FL_DYING))
               continue;

            if (temp->obclass == playerobj)
               {
               if ((temp->flags & FL_GODMODE) || (temp->flags & FL_DOGMODE))
                  continue;
               if ((temp->flags & FL_AV) && (ocl == firejetobj))
                  continue;
               }
            dx = temp->x - ob->x;
            if (abs(dx) > 0xa000)
               continue;
            dy = temp->y - ob->y;
            if (abs(dy) > 0xa000)
               continue;

            //if (temp->obclass == playerobj)
            //Collision(temp,-temp->momentumx+ob->momentumx,-temp->momentumy + ob->momentumy);
            dz = temp->z - ob->z;
            if (abs(dz) > 32)
               continue;

            DamageThing(temp,EnvironmentDamage(ob));
            Collision(temp,ob,0,0);
            if ((ocl == firejetobj) && (temp->obclass < roboguardobj))
               SD_PlaySoundRTP(SD_PLAYERBURNEDSND,temp->x,temp->y);

            if ((gamestate.violence == vl_excessive) && (temp->obclass < roboguardobj))
               {
               if (ocl == bladeobj)
                  {
                  SpawnParticles(temp,GUTS,1);
                  if (temp->hitpoints <= 0)
                     temp->flags |= FL_HBM;
                  }
               else if (ocl == firejetobj)
                  {
                  if ((temp->hitpoints <= 0) && (temp->z == nominalheight))
                     {

                     temp->hitpoints = 0;
                     temp->flags |= FL_SKELETON;
                     if (temp->obclass == playerobj)
                        {
                        playertype *pstate;

                        temp->flags &= ~FL_COLORED;
                        M_LINKSTATE(temp,pstate);
                        pstate->health = 0;
                        pstate->weapon = -1;
                        }

                     Collision(temp,ob,-temp->momentumx,-temp->momentumy);
                     }
                  }
               }
            }
          }
         }
      }


   if (ob->dir == nodir)
      return;

   if ((ocl != firejetobj) && (ocl != bladeobj) && (ocl != diskobj))
      {
       if (!ob->ticcount)
          {
          if (SightPlayer (ob))
             return;
          }
       else
          SoftError("\n ob type %s with ticcount %d in T_Path",
                     debugstr[ob->obclass],ob->ticcount);
      }

   SelectPathDir (ob);
   if (NOMOM)
      ParseMomentum(ob,dirangle8[ob->dir]);
   }



int EnvironmentDamage(objtype *ob)
   {
   if (BATTLEMODE && (gamestate.BattleOptions.DangerDamage != bo_danger_normal))
      {
      return(gamestate.BattleOptions.DangerDamage);
      }
   else
      {
      int damage = 1;

      switch(ob->obclass)
         {
         case firejetobj:
         case bladeobj:
            damage = 6;
            break;

         case boulderobj:
            damage = 50;
            break;

         case spearobj:
            damage = 7;
            break;

         case gasgrateobj:
            damage = 20;
            break;

         case wallfireobj:
            damage = ((GameRandomNumber("wallfire damage",0) >>3) + 10);
            break;

         case crushcolobj:
            damage = 10;
            break;
	 default:
	     ;
         }

      if (gamestate.difficulty < gd_hard)
         damage >>= 1;

      return damage;
      }
   //SoftError("unknown environment danger");

   }




void T_AutoShootAlign(objtype*ob)
{
  if (ob->dir != (dirtype)ob->temp1)
	 ob->dir = dirorder16[ob->dir][NEXT];
  else
	 NewState(ob,M_S(AIM));

}


void T_AutoRealign(objtype*ob)
{
  if (ob->dir != (dirtype)ob->targettilex)
	 ob->dir = dirorder16[ob->dir][NEXT];
  else
	 {objtype *temp;

	  NewState(ob,M_S(PATH));
	 if (ValidAreanumber(ob->areanumber)) {
	     for(temp = firstareaactor[ob->areanumber];temp;temp= temp->nextinarea)
	
		  {if (temp == ob)
			  continue;
			if (temp->obclass != ob->obclass)
			  continue;
	
			if (temp->flags & FL_DYING)
			  continue;
			if (!temp->state->think)
			  NewState(temp,UPDATE_STATES[PATH][temp->obclass-lowguardobj]);
	
		  }
	 }
    }
}


/*
===============
=
= T_AutoPath
=
===============
*/

void T_AutoPath (objtype *ob)
{objtype *temp;

	// ob->temp3 holds random number of shots before resuming path

  if (CheckLine(ob,PLAYER[0],SIGHT) && (Near(ob,PLAYER[0],4) || MISCVARS->madenoise))

	 {int dx,dy,destdir,ocl;
	  statetype *align,*wait;

	  ocl = ob->obclass;
	  dx = player->x - ob->x;
	  dy = ob->y - player->y;
	  destdir = (angletodir[atan2_appx(dx,dy)] << 1);
	  ob->temp1 = destdir;
	  ob->targettilex = ob->dir; //save old dir
#if (SHAREWARE == 0)
     if (ocl == wallopobj)
		  {//if (ob->temp3)
			 // Error("may be writing over temp3");
			ob->temp3 = (GameRandomNumber("T_WallPath",0)%4) + 1;
			align = &s_wallalign;
			wait = &s_wallwait;
		  }
	  else
#endif
        {align = &s_roboalign;
			wait = &s_robowait;
		  }

	  NewState(ob,align);
	    if (ValidAreanumber(ob->areanumber)) {
       for(temp=firstareaactor[ob->areanumber];temp;temp=temp->nextinarea)
		 {if (temp == ob)
			 continue;

		  if (temp->obclass != ob->obclass)
			 continue;

		  if (temp->flags & FL_DYING)
			 continue;

		  if (CheckLine(temp,PLAYER[0],SIGHT) && (Near(temp,PLAYER[0],4) || MISCVARS->madenoise))
			 {dx = PLAYER[0]->x - temp->x;
			  dy = temp->y - PLAYER[0]->y;
			  destdir = (angletodir[atan2_appx(dx,dy)] << 1);

			  temp->temp1 = destdir;
			  temp->targettilex = temp->dir;
			  NewState(temp,align);
			  temp->temp3 = ob->temp3;
			 }
		  else
			 NewState(temp,wait);
		 }
	  }
	  return;
	 }

  SD_PlaySoundRTP(SD_ROBOTMOVESND,ob->x,ob->y);

  SelectPathDir(ob);

}




/*
===============
=
= A_Shoot
=
= Try to damage the player, based on skill level and player's speed
=
===============
*/

void A_Shoot (objtype *ob)
{
	int   dx,dy,dz,dist;
	int   accuracy,damage,sound;
	objtype * target;
	int   num;
	int   savedangle;

 ActorMovement(ob);

 ob->flags |= FL_FULLLIGHT;
 //if (!(ob->flags & FL_SHOOTABLE))
  //Error("\na dead instance of %s is shooting at you",debugstr[ob->obclass]);

 if (!ob->ticcount)
  {if (ob->obclass == strikeguardobj)
	 ob->flags &= ~FL_NOFRICTION;

	target = (objtype*)(ob->target);
   if (!target)
      Error("an instance of %s called shoot without a target\n",debugstr[ob->obclass]);


   ob->flags &= ~FL_FULLLIGHT;


	dx = (target->x - ob->x);
	dy = (ob->y - target->y);
	dz = target->z-ob->z;


   if ((ob->obclass == blitzguardobj) && (ob->temp3) &&
       (ob->temp3 != stat_gasmask) && (ob->temp3 != stat_asbesto) &&
       (ob->temp3 != stat_bulletproof) &&
       (gamestate.difficulty >= gd_medium) &&
       ((abs(dx) > 0xc000) || (abs(dy) > 0xc000))
      )
      {
      int i;
      missile_stats* newmissiledata;

      newmissiledata = &PlayerMissileData[GetWeaponForItem(ob->temp3)];

      // ready to annihilate this poor bastard

      SpawnMissile(ob,newmissiledata->obclass,newmissiledata->speed,
                   AngleBetween(ob,player), newmissiledata->state,
                   newmissiledata->offset);

      if (newmissiledata->obclass == p_drunkmissileobj)
         {
         for(i=0;i<4;i++)
            {
            SpawnMissile(ob,newmissiledata->obclass,newmissiledata->speed,
                        AngleBetween(ob,player), newmissiledata->state,
                        newmissiledata->offset);
            }
         }
      ob->target = NULL;
      ob->temp2 --;
      if (ob->temp2 == 0)
         ob->temp3 = 0;
      return;
      }


   if ((!areabyplayer[ob->areanumber]) && (target->obclass ==  playerobj))
		return;

	//if (!CheckLine(ob,target,SHOOT))       // player is behind a wall
	  //return;


	savedangle=ob->angle;
	ob->angle = atan2_appx (dx,dy);
	dist = FindDistance(dx,dy);
	ob->yzangle = FINEANGLES-atan2_appx(dist, dz<<10);

	if ((ob->yzangle>MAXYZANGLE) && (ob->yzangle<FINEANGLES-MAXYZANGLE))
		ob->yzangle=MAXYZANGLE;

	dist>>=16;

   accuracy=(WHICHACTOR<<4)+((gamestate.difficulty) << 6);

	num = GameRandomNumber("A_Shoot3",ob->obclass);

	if (num<128) num=128; // Don't let accuracy fall below 50% original

	accuracy=FixedMulShift(num,accuracy,8); // scale accuracy based off randomness

	// check for maximum accuracy;

	if (accuracy>255) accuracy=255;

	if (ob->obclass==highguardobj)
		damage=DMG_MP40;
	else if (ob->obclass == triadenforcerobj)
		damage=DMG_MP40;
	else
      damage=DMG_ENEMYBULLETWEAPON;


	RayShoot (ob, damage, 255-accuracy);

	ob->angle=savedangle;
	sound = BAS[ob->obclass].fire;
	SD_PlaySoundRTP(sound,ob->x,ob->y);
	MISCVARS->madenoise = true;
	if ((!(ob->flags& FL_HASAUTO)) || (!ob->temp3))
	 ob->target = NULL;
  }
}




void A_Repeat(objtype*ob)
{
 ActorMovement(ob);

 if (!ob->ticcount)
  {ob->temp3 --;
   if (ob->temp3 <= 0)
	  NewState(ob,M_S(CHASE));
  }

}


void  A_MissileWeapon(objtype *ob)
{
 int    sound,nspeed,noffset,zoffset;

#if (SHAREWARE == 0)
 int oldyzangle;
#endif
 classtype nobclass;
 statetype*nstate;


 if ((ob->obclass == wallopobj) || (ob->obclass == roboguardobj));
	//SelectPathDir(ob);
 else
	ActorMovement(ob);

 if (!ob->ticcount)
  {
#if (SHAREWARE == 0)
   if ((ob->obclass == wallopobj) && (!ob->temp3))
      {
      NewState(ob,&s_wallrestore);
      return;
      }
#endif
	if ((ob->obclass == b_darianobj) && (!CheckLine(ob,PLAYER[0],SHOOT)))
	 {NewState(ob,M_S(CHASE));
	  return;
	 }
 // Move sounds, flags into switch cases

	sound = BAS[ob->obclass].fire;
	nspeed = 0x4000;
	noffset = 0x8000;
	zoffset = 0;
	switch (ob->obclass)
    {

	  case triadenforcerobj:
		nobclass = grenadeobj;
		nstate= &s_grenade1;
		sound++;
		break;


	  case roboguardobj:
		nobclass = shurikenobj;
		nspeed = 0x2000;
		noffset = 0x10000;
		nstate = &s_robogrdshuriken1;
		break;

	 /*
	  case b_darkmonkobj:
		nobclass = dmfballobj;
		nstate = &s_dmfball1;
		break;
	  */
	  /*
	  case b_robobossobj:
		nobclass = bigshurikenobj;
		nstate = &s_oshuriken1;
		break;
		 */
#if (SHAREWARE == 0)
	  case b_darianobj:
		nobclass = missileobj;
		//nspeed = 0x100;
		//noffset = 0x18000;
		nstate = &s_missile1;
		zoffset = -20;
		break;


     case dfiremonkobj:
		nobclass = fireballobj;
		nstate = &s_monkfire1;
		break;


     case overpatrolobj:
		nobclass = netobj;
		nstate = &s_bolocast1;
		sound ++;
		break;


     case wallopobj:
		 {int dx,dy,dz,xydist;

		  ob->temp2 ^= 1; // increment numfired
		  ob->temp3 --; // decrement random fire no.


		  dx = PLAYER[0]->x-ob->x;
		  dy = ob->y-PLAYER[0]->y;
		  if (GameRandomNumber("bcraft shoot up/down",0) < 128)
			 dz = 10;
		  else
			 dz = -10;
		  xydist = FindDistance(dx,dy);
		  oldyzangle = ob->yzangle;
		  ob->yzangle = atan2_appx(xydist,dz<<10);
		  if (ob->temp2)
			 {nobclass = missileobj;
			  nstate = &s_missile1;
			 }
		  else
			 {nobclass = bigshurikenobj;
			  nstate = &s_bstar1;
			 }
		 }
		break;
#endif
    default:
	;
    }

	SpawnMissile(ob,nobclass,nspeed,AngleBetween(ob,PLAYER[0]),nstate,noffset);
	new->z += zoffset;
	SD_PlaySoundRTP(sound,ob->x,ob->y);
	MISCVARS->madenoise = true;
	if (ob->obclass == triadenforcerobj)
	  {//new->flags |= (FL_SHOOTABLE);
		new->temp1 = 0x50000;
	  }
#if (SHAREWARE == 0)
	else if (ob->obclass == wallopobj)
	  ob->yzangle = oldyzangle;
#endif


  }
}


void   A_Wallfire(objtype *ob)
{
 if (!(ob->flags & FL_ACTIVE))
  return;

 if (!ob->ticcount)
  {SpawnMissile(ob,wallfireobj,0x4000,ob->angle,&s_crossfire1,0xa000);
	if (areabyplayer[ob->areanumber])
	  SD_PlaySoundRTP(SD_FIRECHUTESND,ob->x,ob->y);
	new->dir = angletodir[new->angle];
	new->z = nominalheight;
  }

}


void T_Reset(objtype *ob)
{
 ActorMovement(ob);

 if (ob->ticcount)
	return;

 ob->momentumx = ob->momentumy = ob->dirchoosetime = 0;
 ob->flags &= ~FL_NOFRICTION;



}

void SelectRollDir(objtype *ob)
{int angle,tryx,tryy;


 if (ob->state == &s_strikerollright1)
  angle = AngleBetween(ob,PLAYER[0]) + ANGLES/4;
 else
  angle = AngleBetween(ob,PLAYER[0]) - ANGLES/4;

 Fix(angle);
 tryx = ob->x + FixedMul(0x20000l,costable[angle]);
 tryy = ob->y - FixedMul(0x20000l,sintable[angle]);
 ZEROMOM;
 if (QuickSpaceCheck(ob,tryx,tryy))
	{int oldspeed;

	 oldspeed = ob->speed;
	 ob->speed = ROLLMOMENTUM;
	 ParseMomentum(ob,angle);
	 ob->speed = oldspeed;
	 ob->dirchoosetime = 5;
	 ob->flags |= FL_NOFRICTION;
	}
 else
  ob->dirchoosetime = 0;


}


void SelectDodgeDir (objtype *ob)
{
	int      dx,dy,i,tx,ty;
	unsigned absdx,absdy;
	dirtype  dirtry[5];
	dirtype  turnaround,tdir,olddir;


	olddir = ob->dir;
	if (ob->flags & FL_FIRSTATTACK)
	{
	//
	// turning around is only ok the very first time after noticing the
	// player
	//
		turnaround = nodir;
		ob->flags &= ~FL_FIRSTATTACK;
	}
	else
		turnaround=opposite[ob->dir];


	if (ob->targettilex || ob->targettiley)
		{tx = ob->targettilex;
		 ty = ob->targettiley;
		 dx= tx - ob->x;
		 dy= ty - ob->y;
		 if ( ((dx < MINACTORDIST) && (dx > -MINACTORDIST)) &&
				((dy < MINACTORDIST) && (dy > -MINACTORDIST)))
		  {dx= PLAYER[0]->x-ob->x;
			dy= PLAYER[0]->y-ob->y;
		  }
		}
	  else
   	  {dx= PLAYER[0]->x-ob->x;
			dy= PLAYER[0]->y-ob->y;
		  }



//
// arange 5 direction choices in order of preference
// the four cardinal directions plus the diagonal straight towards
// the player
//
	if (dx>ACTORSIZE)
	{
		dirtry[1]= east;
		dirtry[3]= west;
	}
	else if (dx < -ACTORSIZE)
	{
		dirtry[1]= west;
		dirtry[3]= east;
	}

	if (dy>ACTORSIZE)
	{
		dirtry[2]= south; // south
		dirtry[4]= north; // north
	}
	else if (dy <-ACTORSIZE)
	{
		dirtry[2]= north; // north
		dirtry[4]= south; // south
	}

//
// randomize a bit for dodging
//
	absdx = abs(dx);
	absdy = abs(dy);

	if (absdx > absdy)
	{
		tdir = dirtry[1];
		dirtry[1] = dirtry[2];
		dirtry[2] = tdir;
		tdir = dirtry[3];
		dirtry[3] = dirtry[4];
		dirtry[4] = tdir;
	}

	if (GameRandomNumber("SelectDogeDir",ob->obclass) < 128)
	{
		tdir = dirtry[1];
		dirtry[1] = dirtry[2];
		dirtry[2] = tdir;
		tdir = dirtry[3];
		dirtry[3] = dirtry[4];
		dirtry[4] = tdir;
	}

	dirtry[0] = diagonal [dirtry[1]] [dirtry[2]];

	ZEROMOM;

	for (i=0;i<5;i++)
	  {if ((dirtry[i] == nodir) || (dirdiff[dirtry[i]][olddir] > 1))
		 continue;

		M_CHECKDIR(ob,dirtry[i]);
	}

//
// turn around only as a last resort
//
//	for(tdir = east;tdir<=southeast;tdir++)
	//if (tdir != turnaround)
//	 M_CHECKDIR(ob,tdir);

	if (turnaround != nodir)
	 M_CHECKDIR(ob,turnaround);

}




#define TryAbruptProximalDirections(trydir)             \
   {                                                    \
   next = dirorder[trydir][NEXT];                       \
   prev = dirorder[trydir][PREV];                       \
   if (GameRandomNumber("actor choose dir",0) < 128)    \
      {                                                 \
      dirtype temp = next;                              \
                                                        \
      next = prev;                                      \
      prev = temp;                                      \
      }                                                 \
                                                        \
   if (!dirtried[next])                                 \
      {                                                 \
      M_CHECKDIR(ob,next);                              \
      dirtried[next]=1;                                 \
      }                                                 \
                                                        \
   if (!dirtried[prev])                                 \
      {                                                 \
      M_CHECKDIR(ob,prev);                              \
      dirtried[prev]=1;                                 \
      }                                                 \
                                                        \
   }


#define TrySmoothProximalDirections(trydir)                        \
   {                                                               \
                                                                   \
   if (((trydir == olddir) || (dirdiff[trydir][olddir] < 2)) &&    \
      (!dirtried[trydir]))                                         \
      {                                                            \
      M_CHECKDIR(ob,trydir);                                       \
      dirtried[trydir] = 1;                                        \
      }                                                            \
   next = dirorder[olddir][NEXT];                                  \
   prev = dirorder[olddir][PREV];                                  \
                                                                   \
   if (dirdiff[trydir][next] <= dirdiff[trydir][prev])             \
      {                                                            \
      start = next;                                                \
      whichway = NEXT;                                             \
      }                                                            \
   else                                                            \
      {                                                            \
      start = prev;                                                \
      whichway = PREV;                                             \
      }                                                            \
                                                                   \
   for (tdir= start; tdir != dirorder[trydir][whichway];           \
        tdir = dirorder[tdir][whichway]                            \
       )                                                           \
      {                                                            \
      if (dirtried[tdir])                                          \
         continue;                                                 \
      M_CHECKDIR(ob,tdir);                                         \
      dirtried[tdir]=1;                                            \
      }                                                            \
                                                                   \
   }



#define ChasePlayer(ob)                          \
   {                                             \
   dx= player->x-ob->x;                          \
   dy= ob->y-player->y;                          \
   if ((abs(dx) < 0xb000) && (abs(dy) < 0xb000)) \
      return;                                    \
   dummy.x = player->x;                          \
   dummy.y = player->y;                          \
   }


void SelectChaseDir (objtype *ob)
   {
	int dx,dy,whichway,tx,ty,actrad,visible,
		 realdiff;
	dirtype dtry1,dtry2,tdir,olddir,next,prev,start,straight;
	tpoint dummy;
	byte dirtried[9] = {0};

	olddir=ob->dir;
	visible = CheckLine(ob,PLAYER[0],SIGHT);

	/*
	if (ob->flags & FL_FIRSTATTACK)
	{
	//
	// turning around is only ok the very first time after noticing the
	// player
	//
		turnaround = opposite[ob->dir];
		ob->flags &= ~FL_FIRSTATTACK;
	}
	else
		turnaround=nodir;
	*/
	dummy.which = ACTOR;
	dummy.z = ob->z;
	if (ob->targettilex || ob->targettiley)
      {
      tx = ob->targettilex;
      ty = ob->targettiley;
      dx= tx - ob->x;
      dy= ob->y - ty;
      dummy.x = tx;
      dummy.y = ty;
      if ((abs(dx) < 0x2000) && (abs(dy) < 0x2000))
         ChasePlayer(ob);
		}
	else
      ChasePlayer(ob);

	//if ((abs(dx) < 0x10000) && (abs(dy) < 0x10000))
	 //return;
	straight = angletodir[atan2_appx(dx,dy)];
	realdiff = dirdiff[straight][ob->dir];
	ZEROMOM;

   //insertion 20

	actrad = ACTORSIZE;
	dtry1=nodir;
	dtry2=nodir;

	if (dx> actrad)
		dtry1= east;
	else if (dx< -actrad)
		dtry1= west;
	if (dy> actrad)
		dtry2=north;
	else if (dy < -actrad)
		dtry2= south;


	if (abs(dy)>abs(dx))
      {
      tdir=dtry1;
      dtry1=dtry2;
      dtry2=tdir;
      }

	if (GameRandomNumber("chase minor",0) < 80)
      {
      tdir=dtry1;
      dtry1=dtry2;
      dtry2=tdir;
      }

	ZEROMOM;

	if ((!visible) || (realdiff > 2))  // don't worry about abrupt or unrealistic turns if player
						// can't see guards
      {
      M_CHECKDIR(ob,straight);
      dirtried[straight]=1;
      next = dirorder[straight][NEXT];
      prev = dirorder[straight][PREV];

      if ((dtry1 != nodir) && (dtry1 != straight))
         {
         M_CHECKDIR(ob,dtry1);
         dirtried[dtry1]=1;
         }


      if ((dtry2 != nodir) && (!dirtried[dtry2]))
         {
         M_CHECKDIR(ob,dtry2);
         dirtried[dtry2]=1;
         }

      if (dtry1 != nodir)
         TryAbruptProximalDirections(dtry1);

      if (dtry2 != nodir)
         TryAbruptProximalDirections(dtry2);
		}

   else
      {
      if (realdiff < 2)
         {
         M_CHECKDIR(ob,straight);
         dirtried[straight]=1;
         }

      if (dtry1 != nodir)
         TrySmoothProximalDirections(dtry1);

      if (dtry2 != nodir)
         TrySmoothProximalDirections(dtry2);

		}

	if ((dtry1!=nodir) || (dtry2!=nodir))
      {
      if (GameRandomNumber("actor choose dir",0) < 128)
         whichway = NEXT;
      else
         whichway = PREV;

      for(tdir = dirorder[olddir][whichway];tdir != olddir;tdir = dirorder[tdir][whichway])
         {
         if (dirtried[tdir])
            continue;
         M_CHECKDIR(ob,tdir);
         }
      }

   ob->dir = olddir;
   }



int Near(objtype *ob,void* what,int distance)

{
 objtype *aw;

 aw = (objtype*) what;

 if (FindDistance((aw->x - ob->x),(aw->y - ob->y)) <= (distance<<16))
    return 1;
 return 0;

}



/*
===============
=
= SelectPathDir
=
===============
*/

void SelectPathDir (objtype *ob)
   {
	int spot,centerx,centery,dx,dy,set,done,radius,ocl;

	ocl = ob->obclass;


	if ((ocl == bladeobj) && (!(ob->flags & FL_ACTIVE)))
	  return;

	spot = MAPSPOT(ob->tilex,ob->tiley,1)-ICONARROWS;
	set = ((ocl == wallopobj) || (ocl == roboguardobj));
	done = (((!set) && (ob->dir == (dirtype)spot)) ||
			  (set && (ob->dir == (dirtype)(spot<<1))));

	if ((spot >= 0) && (spot<= 7) && (!done))
      {
      centerx= (ob->tilex << 16) + HALFGLOBAL1;
      centery= (ob->tiley << 16) + HALFGLOBAL1;
      dx = abs(centerx - ob->x);
      dy = abs(centery - ob->y);
      //radius = (ob->speed > 0x4800)?(0xb000):(0x4000);
      radius = 0x4000;

      if ((dx < radius) && (dy < radius))
	// new direction
         {
         ZEROMOM;
         if ((ocl == wallopobj) || (ocl == roboguardobj))
           {
           ob->dir = spot<<1;
           ParseMomentum(ob,dirangle16[ob->dir]);
           }
         else
           {
           ob->dir = spot;
           ParseMomentum(ob,dirangle8[ob->dir]);
           }
         dx = centerx - ob->x;
         dy = centery - ob->y;
         SetFinePosition(ob,centerx,centery);
         SetVisiblePosition(ob,centerx,centery);
		 /*
		 if (((ocl == bladeobj) || (ocl == diskobj)) && ob->whatever)
			{objtype*passenger = (objtype*)(ob->whatever);

			 passenger->x += dx;
			 passenger->y += dy;
			 passenger->drawx = passenger->x;
			 passenger->drawy = passenger->y;
			 passenger->tilex = passenger->x >> 16;
			 passenger->tiley = passenger->y >> 16;
			}*/
//		 if (ob==SNAKEHEAD)
//		  Debug("\n path changed at %d, %d",
//			ob->tilex,ob->tiley);
         }
      }
	if (NOMOM)
      {
      if ((ocl == wallopobj) || (ocl == roboguardobj))
         ParseMomentum(ob,dirangle16[ob->dir]);
		else
         ParseMomentum(ob,dirangle8[ob->dir]);
      }

	//if ((ob->obclass == firejetobj) || (ob->obclass == bladeobj))
	 //MoveActor(ob);
	//else
   ActorMovement(ob);

   }



/*
================
=
= CheckSight
=
= Checks a straight line between player and current object
=
= If the sight is ok, check alertness and angle to see if they notice
=
= returns true if the player has been spoted
=
================
*/


boolean CheckSight (objtype *ob,void *atwhat)
{
	long     deltax,deltay;
	objtype * what;
//
// don't bother tracing a line if the area isn't connected to the player's
//
	if (!areabyplayer[ob->areanumber])
		return false;

//
// if the player is real close, sight is automatic
//
	what = (objtype*)atwhat;
	deltax = what->x - ob->x;
	deltay = what->y - ob->y;

	if ((deltax > -MINSIGHT) && (deltax < MINSIGHT)	&&
		 (deltay > -MINSIGHT) && (deltay < MINSIGHT))
		return true;

//
// see if they are looking in the right direction
//
	switch (ob->dir)
	{
	case north:
		if (deltay > 0)
			return false;
		break;

	case east:
		if (deltax < 0)
			return false;
		break;

	case south:
		if (deltay < 0)
			return false;
		break;

	case west:
		if (deltax > 0)
			return false;
		break;
	default:
	    ;
	}

//
// trace a line to check for blocking tiles (corners)
//
	return CheckLine (ob,atwhat,SIGHT);

}



void ActivateEnemy(objtype*ob)
{statetype *temp;


 ob->flags |= (FL_ATTACKMODE|FL_FIRSTATTACK);
 if (ob->obclass == roboguardobj)
	return;

 if (ob->temp3 == SNEAKY)
	{ NewState(ob,&s_sneakyrise1);
	  ob->temp3=0;
	}
 else if ((temp = M_S(CHASE)) != NULL)
	NewState(ob,temp);
 /*
 ob->speed = ENEMYRUNSPEED;
 */
 if (ob->obclass == b_heinrichobj)
    ob->speed = 7*ob->speed/2;
 else if (ob->obclass == b_darianobj)
    ob->speed = 3*SPDPATROL;
 if (ob->door_to_open != -1)
	ob->door_to_open = -1; // ignore the door opening command
 ob->dirchoosetime = 0;


}


/*
===============
=
= FirstSighting
=
= Puts an actor into attack mode and possibly reverses the direction
= if the player is behind it
=
===============
*/

void FirstSighting (objtype *ob)
   {
   statetype *temp;
   int sound;

   if (ob->temp3 == SNEAKY)
      {
      NewState(ob,&s_sneakyrise1);
      ob->temp3=0;
      if (ob->shapeoffset==0)
      SD_PlaySoundRTP(SD_SNEAKYSPRINGMSND,ob->x,ob->y);
      else
      SD_PlaySoundRTP(SD_SNEAKYSPRINGFSND,ob->x,ob->y);
      }
   else if ((temp = M_S(CHASE)) != NULL)
      {
      int rand;

      NewState(ob,temp);
      sound = BAS[ob->obclass].see;
      rand = GameRandomNumber("FirstSighting low",0);
      if ((ob->obclass > lowguardobj) && (ob->obclass <= blitzguardobj) && (rand < 128)) //hack for alternate
         sound++;
         //if ((ob->obclass == lowguardobj) && (rand < 80))
         //sound ++;
      else if (ob->obclass == lowguardobj)
         {if (rand < 128)
         {if ((PLAYERSTATE[0].player == 1) || (PLAYERSTATE[0].player == 3))
            sound++;
         }
         else
         sound += 2;

         if (ob->shapeoffset)
         sound += 4;

         }
      SD_PlaySoundRTP(sound,ob->x,ob->y);
      if ((ob->obclass>=b_darianobj) && (ob->obclass<=b_darksnakeobj))
         {
         MU_StartSong(song_bosssee);
         }
      }

   /*
   ob->speed = ENEMYRUNSPEED;*/
   if (ob->obclass == b_heinrichobj)
      ob->speed = 7*ob->speed/2;
   else if (ob->obclass == b_darianobj)
      ob->speed = 3*SPDPATROL;
   if (ob->door_to_open != -1)
   ob->door_to_open = -1; // ignore the door opening command
   ob->dirchoosetime = 0;
   ob->flags |= (FL_ATTACKMODE|FL_FIRSTATTACK);

   }


/*
===============
=
= SightPlayer
=
= Called by actors that ARE NOT chasing the player.  If the player
= is detected (by sight, noise, or proximity), the actor is put into
= it's combat frame and true is returned.
=
= Incorporates a random reaction delay
=
===============
*/

boolean SightPlayer (objtype *ob)
   {
	//if (ob->flags & FL_ATTACKMODE)
	  //Error ("An instance of %s in ATTACKMODE called SightPlayer!",debugstr[ob->obclass]);

	if (!areabyplayer[ob->areanumber])
      return false;

   if (ob->obclass == b_robobossobj)
      {
      if (!(CheckSight(ob,player) || Near(ob,player,6)))
         return false;

      }


   else if (ob->flags & FL_AMBUSH)
      {
      if (!CheckSight (ob,PLAYER[0]))
         {
         //SoftError("\n failed from ambush in SightPlayer");
         return false;
         }
      ob->flags &= ~FL_AMBUSH;
      }
	else
      {
      if (ob->temp3 == SNEAKY)
         {
         if (!Near(ob,PLAYER[0],2))
            return false;
         }
      else if (!((MISCVARS->madenoise) ||
                 (CheckSight (ob,player)) ||
                 (Near(ob,player,4))
                )
              )
         {
         //SoftError("\n failed from SightPlayer");
         return false;
         }
      }

	FirstSighting (ob);

	return true;
   }


/*
=====================
=
= CheckLine
=
= Returns true if a straight line between two obs is unobstructed
=
=====================
*/



boolean CheckLine (void *from, void *to, int condition)
{
	objtype   *tempactor,*ob,*orig;
	statobj_t *checksprite;
	int destx,desty,destz;
	int desttilex,desttiley;
	int snx,sny;
	int incr[2];
	int thedir[2];
	int cnt;
	int grid[2];
	int index;
	int vx,vy;
	int yzangle;
	int value;
	int dx,dy,dz;
	int xydist;
	int otx,oty,count=0;



   ob = (objtype*)to;
	orig = (objtype*)from;
	if (ob->which == SPRITE)
		{
		destx = ((statobj_t*)to)->x;
		desty = ((statobj_t*)to)->y;
		destz = ((statobj_t*)to)->z;
		}
	else
		{
		destx = ob->x;
		desty = ob->y;
		destz = ob->z;
		}

	desttilex=destx>>16;
	desttiley=desty>>16;

	if ((desttilex == orig->tilex) && (desttiley == orig->tiley))
	  return true;


	dx=destx-orig->x;
	dy=orig->y-desty;
   dz=orig->z-destz;
	xydist = FindDistance(dx,dy);
	yzangle = atan2_appx(xydist,dz<<10);

	if ((yzangle>MAXYZANGLE) && (yzangle<FINEANGLES-MAXYZANGLE))
	  {
		#if (0)
		 Debug("\nfailed from yzangle");
		#endif
		return false;
	  }

	//angle = atan2_appx(dx,dy);
	otx = orig->x >> TILESHIFT;
	oty = orig->y >> TILESHIFT;

	if (xydist==0)
		{
      /*
      SoftError("\nCheckLine:xydist=0");
		if (orig->which == ACTOR)
		  SoftError("shooter: %s",debugstr[orig->obclass]);
		if (ob->which == ACTOR)
        SoftError("target: %s",debugstr[ob->obclass]);*/
		vy=-dy;
		vx=dx;
		}
	else
		{
		vy = -FixedDiv2(dy,xydist);
		vx = FixedDiv2(dx,xydist);
		}
	snx=orig->x&0xffff;
	sny=orig->y&0xffff;

	grid[0]=otx;
	grid[1]=oty;

	if (vx>0)
		{
		thedir[0]=1;
		snx^=0xffff;
		incr[1]=-vx;
		}
	else
		{
		thedir[0]=-1;
		incr[1]=vx;
		}
	if (vy>0)
		{
		thedir[1]=1;
		sny^=0xffff;
		incr[0]=vy;
		}
	else
		{
		thedir[1]=-1;
		incr[0]=-vy;
		}
	cnt=FixedMul(snx,incr[0])+FixedMul(sny,incr[1]);


   do
		{count ++;
       /*
       if (count > 1000)
         Error("possible infinite loop in CheckLine");
		 if ((grid[0] < 0) || (grid[0] > (MAPSIZE-1)) ||
			  (grid[1] < 0) || (grid[1] > (MAPSIZE-1)))
			Error("out of bounds in check line, grid[0] = %d, grid[1] = %d",grid[0],grid[1]);
        */
		if ((grid[0]==desttilex) && (grid[1]==desttiley))
			return true;
		tempactor = (objtype*)actorat[grid[0]][grid[1]];
		value = tilemap[grid[0]][grid[1]];
		checksprite = sprites[grid[0]][grid[1]];
		if (value)
			{
			if (value&0x8000)
				{
				if (!(value&0x4000))
					{
               doorobj_t*dptr = doorobjlist[value&0x3ff];

               if (dptr->position < 0x8000)
						{

                  int x = (grid[0] << 16) + 0x8000;
                  int y = (grid[1] << 16) + 0x8000;

                  if (dptr->vertical)
                     {
                     if (abs(dx) > abs(x-orig->x))
                        return false;
                     }
                  else
                     {
                     if (abs(dy) > abs(orig->y-y))
                        return false;
                     }
						}
					}
				else
					{
					if (condition == SHOOT)
						{
                  if ( maskobjlist[value&0x3ff]->flags & MW_SHOOTABLE )
							{
                      #if (0)
                      SoftError("\nfailed from shootable mask");
                    #endif
							 return false;
							}
                  else if ( maskobjlist[value&0x3ff]->flags & MW_WEAPONBLOCKING )
							{
                      #if (0)
                      SoftError("\nfailed from block mask");
                      #endif
							 return false;
							}
						}
               else if ((condition == MISSILE) &&
                        ( maskobjlist[value&0x3ff]->flags & MW_BLOCKING )
                       )
                  return false;
					}
				}
			else
				{
             #if (0)
            SoftError("\n obx %d, oby %d, origx %d, origy %d"
                      "\n xydist %d, vx %d, vy %d",ob->x,ob->y,orig->x,
                      orig->y,xydist,vx,vy);

            SoftError("\nfailed from normal wall");
             #endif
            return false;
				}
			}
		if (condition == SHOOT)
			{
			if (tempactor && (tempactor->which == ACTOR) &&
				(tempactor->flags & FL_BLOCK) && (tempactor != orig) &&
				(tempactor != ob)) //&&
//   			(InRange(orig,tempactor,
//             FindDistance(orig->x-tempactor->x,orig->y-tempactor->y) )
//             ==true) )
			  {
            #if (0)
             SoftError("\nfailed from actor");
            #endif
				return false;
			  }
			}

		if (checksprite && (checksprite->flags & FL_BLOCK) && (condition == SHOOT) &&
			 ((void *)checksprite != to) &&
          (checksprite->itemnumber!=stat_disk) &&
         (InRange(orig,(objtype *)checksprite,
			 FindDistance(orig->x-checksprite->x,orig->y-checksprite->y) )
			 ==true) )

			{
          #if (0)
          SoftError("\nfailed from sprite");
          #endif
			 return false;
			}

		if (tempactor && (tempactor->which == PWALL))
			{
        #if (0)
          SoftError("\nfailed from pushwall");
          #endif
			 return false;
			}
		index=(cnt>=0);
		cnt+=incr[index];
		grid[index]+=thedir[index];
		}
	while (1);
}







/*
=====================
=
= ShootActor
=
= Shoot an actor.
=
=====================
*/
void ShootActor(objtype * shooter, objtype * target, int damage, int accuracy, int angle)
{
	int dx,dy,dist;
	int newmomx, newmomy;
	int tcl;


	if (target->flags & FL_DYING)
	  return;

	dx = abs(shooter->x - target->x);
	dy = abs(shooter->y - target->y);
	dist = FindDistance(dx,dy)>>16;

	tcl=target->obclass;

	if (tcl==playerobj)
		{
		target->target=shooter;
		if (target->flags&FL_BPV)
         {
         playertype *pstate;

         M_LINKSTATE(target,pstate);
         pstate->protectiontime -= (damage<<1);
         if (pstate->protectiontime < 1)
            pstate->protectiontime = 1;
         if (target==player)
            GM_UpdateBonus (pstate->protectiontime, false);
         return;
         }
		else if ((target->flags&FL_GODMODE) || (target->flags&FL_DOGMODE) || godmode)
			return;
		//damage=FixedMulShift((gamestate.difficulty+1),damage,2); // player object difficulty
		}

	else if (tcl == NMEsaucerobj)
		{
		target->momentumx = target->momentumy = 0;
		NewState(target,&s_explosion1);
		target->flags &= ~FL_SHOOTABLE;
		return;
		}
	else if (tcl == b_darianobj)
		 MISCVARS->ESAU_SHOOTING = false;
	else if ((tcl == strikeguardobj) || (tcl == b_heinrichobj))
		 target->target = shooter;

	if ((  (!(target->flags & FL_SHOOTABLE)) ||
			(tcl == roboguardobj) || (tcl == wallopobj) ||
			(tcl == patrolgunobj) ) &&
		 (tcl!=playerobj)  )
		SpawnMetalSparks(target,angle);

   else if ((tcl < b_darianobj) || (tcl > b_darksnakeobj))
		{

		//target->flags &= ~FL_USE;

		damage=FixedMulShift(511-accuracy,damage,9); // Min half damage
		if (dist<64)
			{
			if (dist>2)
				damage=FixedMulShift(63-dist,damage,6);
			if (damage<1)
				damage=1;
			}
		else
			damage=1;

		if (damage>MAXDAMAGE) damage=MAXDAMAGE; // absolutely clip it

		DamageThing(target,damage);
		if ((tcl == collectorobj) && gamestate.SpawnDeluder)
         {
         Collision(target,shooter,0,0);
         if (target->hitpoints <= 0)
            BATTLE_CheckGameStatus(battle_shot_deluder,shooter->dirchoosetime);
         }

		else
		  {newmomx = FixedMul(damage<<7,costable[angle]);
			newmomy = -FixedMul(damage<<7,sintable[angle]);
         Collision(target,shooter,-(target->momentumx)+newmomx,-(target->momentumy)+newmomy);
			if (tcl == playerobj)
				{
				 playertype * pstate;

				 M_LINKSTATE(target,pstate);
				 if (pstate->health <= 0)
					{

					    if (shooter->obclass == playerobj) {
						if (!target->momentumz)
						  BATTLE_PlayerKilledPlayer(battle_kill_with_bullet,shooter->dirchoosetime,target->dirchoosetime);
						else
						  BATTLE_PlayerKilledPlayer(battle_kill_with_bullet_in_air,shooter->dirchoosetime,target->dirchoosetime);
					    }
					}
				}
//      SoftError("ShootActor: damage=%ld dist=%ld\n",damage,dist);

			if ((GameRandomNumber("disembowel",0)<64) && (gamestate.violence == vl_excessive))
			  {
			  int temp;
			  temp=target->temp1;
			  target->temp1=angle;

           SpawnParticles(target,DISEMBOWEL,damage>>3);
			  target->temp1=temp;
			  }
			else if (gamestate.violence > 0)
			  SpawnBlood(target,angle);
		  }
		}
}


/*
=====================
=
= ShootSprite
=
= Shoot a sprite.
=
=====================
*/
void ShootSprite(objtype * shooter, statobj_t * target, int damage, int accuracy, int angle)
{
   int dx,dy,dist;


   if (!(target->flags & FL_SHOOTABLE))
// Watchout for sprite being passed in as actor WARNING
      SpawnMetalSparks((objtype *)target,angle);

   else
      {
      dx = abs(shooter->x - target->x);
      dy = abs(shooter->y - target->y);
      dist = FindDistance(dx,dy)>>16;

      damage=FixedMulShift(511-accuracy,damage,9); // Min half damage
      if (dist<64)
         {
         if (dist>2)
            damage=FixedMulShift(63-dist,damage,6);
         if (damage<1)
            damage=1;
         }
      else
         damage=1;

      if (damage>MAXDAMAGE) damage=MAXDAMAGE; // absolutely clip it

   //   SoftError("ShootSprite: damage=%ld dist=%ld\n",damage,dist);

      DamageThing((objtype *)target,damage);
      if (FirstExplosionState(new->state))
         new->whatever = shooter;

      SpawnStaticDamage(target, angle);
      }
}


/*
=====================
=
= RayShoot
=
= Cast a ray out at the shooter's angle and yzangle, return
=
=====================
*/



void RayShoot (objtype * shooter, int damage, int accuracy)
{
	objtype   *tempactor;
	statobj_t *checksprite;
	int snx,sny;
	int incr[2];
	int zincr[2];
	int thedir[2];
	int cnt;
	int grid[2];
	int index;
	int vx,vy;
	int angle;
	int yzangle;
	int value;
	int offset;
	int z;
	int lastcnt;
   int bullethole=0;
	enum {gs_door, gs_wall, gs_floor, gs_ceiling, gs_pushwall};
	int smokecondition=0;


   if ((shooter->areanumber==player->areanumber) && (Near(shooter,player,3)))
      SetIllumination(2);

	offset = ((GameRandomNumber("RayShoot",0)-128)>>MAXSHOOTSHIFT);
	offset = FixedMulShift(accuracy,offset,8);

	if (offset>MAXSHOOTOFFSET)
		offset=MAXSHOOTOFFSET;

	else if (offset<-MAXSHOOTOFFSET)
		offset=-MAXSHOOTOFFSET;

	angle=(shooter->angle+offset)&(FINEANGLES-1);

   offset = ((GameRandomNumber("RayShoot",1)-128)>>MAXSHOOTSHIFT);
	offset = FixedMulShift(accuracy,offset,8);

	if (offset>MAXSHOOTOFFSET)
		offset=MAXSHOOTOFFSET;

	else if (offset<-MAXSHOOTOFFSET)
		offset=-MAXSHOOTOFFSET;

	yzangle=(shooter->yzangle+offset)&(FINEANGLES-1);

	vy = -sintable[angle];
	vx = costable[angle];
	snx=shooter->x&0xffff;
	sny=shooter->y&0xffff;
	grid[0]=shooter->tilex;
	grid[1]=shooter->tiley;
	if (shooter->obclass==playerobj)
		{
		playertype * pstate;

		M_LINKSTATE(shooter,pstate);
		z=shooter->z+pstate->playerheight-32;
		}
	else
      z=shooter->z-7;
	if (vx>0)
		{
		thedir[0]=1;
		snx^=0xffff;
		incr[1]=-vx;
		}
	else
		{
		thedir[0]=-1;
		incr[1]=vx;
		}
	if (vy>0)
		{
		thedir[1]=1;
		sny^=0xffff;
		incr[0]=vy;
		}
	else
		{
		thedir[1]=-1;
		incr[0]=-vy;
		}
	zincr[0]=-FixedMulShift(sintable[yzangle],abs(vx),26);
	zincr[1]=-FixedMulShift(sintable[yzangle],abs(vy),26);

	cnt=FixedMul(snx,incr[0])+FixedMul(sny,incr[1]);
	index= (cnt >= 0);
	do
		{
		tempactor = (objtype*)actorat[grid[0]][grid[1]];
		value = tilemap[grid[0]][grid[1]];
		checksprite = sprites[grid[0]][grid[1]];
		if (value)
			{
			if (value&0x8000)
				{
				if (!(value&0x4000))
					{
					if ((doorobjlist[value&0x3ff]->action==dr_closed) || (z<maxheight-64))
						{
						smokecondition=gs_door;
						break;
						}
					else if (doorobjlist[value&0x3ff]->position<=0x8000)
						{
						smokecondition=gs_door;
						break;
						}
					}
				else
					{
					if ( maskobjlist[value&0x3ff]->flags & MW_SHOOTABLE )
						{
						if (z>maxheight-64) // Are we shooting above the glass
							{
							UpdateMaskedWall(value&0x3ff);
							return;
							}
						}
					else if ( maskobjlist[value&0x3ff]->flags & MW_WEAPONBLOCKING )
						{
						smokecondition=gs_door;
						break;
						}
					}
				}
			else
				{
				smokecondition=gs_wall;
				break;
				}
			}

      if (checksprite &&
          ((checksprite->flags & FL_BLOCK)||(checksprite->flags & FL_SHOOTABLE)) &&
          (abs(checksprite->z-z)<32) &&
          (InRange(shooter,(objtype *)checksprite,
			 FindDistance(shooter->x-checksprite->x,shooter->y-checksprite->y) )
          ==true
          )
         )
			{
			ShootSprite(shooter, checksprite, damage, accuracy, angle);
			return;
			}


      if (tempactor)
        {if (tempactor->which == ACTOR)
            {if ((abs(tempactor->z-z)<32 ) && (!(tempactor->flags & FL_DYING)) &&
                (tempactor->flags & FL_BLOCK) && (tempactor != shooter) &&
                (tempactor->obclass!=diskobj) &&
                (InRange(shooter,tempactor,
                 FindDistance(shooter->x-tempactor->x,shooter->y-tempactor->y) )
                 ==true
                )
               )
              {ShootActor(shooter, tempactor, damage, accuracy, angle);
               return;
              }
            }
          else if (tempactor->which == PWALL)
            return;
        }

		if (z<-32)
			{
			smokecondition=gs_ceiling;
			break;
			}
		else if (z>maxheight)
			{
			smokecondition=gs_floor;
			break;
			}
		index= (cnt >= 0);
		cnt+=incr[index];
		z  +=zincr[index];
		grid[index]+=thedir[index];
		}
	while (1);

	if (IsWindow(grid[0],grid[1]))
		return;

	lastcnt=cnt-incr[index];

	if (smokecondition==gs_floor)
		{
		int dist;
		int tangentangle;

		tangentangle=tantable[yzangle];
		if (tangentangle!=0)
			{
         dist=FixedDiv2(((shooter->z-maxheight)<<10),(tangentangle<<1));
			xintercept=shooter->x+FixedMul(dist,costable[angle]);
			yintercept=shooter->y-FixedMul(dist,sintable[angle]);
			}
		z=maxheight;
//      bullethole=5;
		}
	else if (smokecondition==gs_ceiling)
		{
		int dist;
		int tangentangle;

		if (sky!=0)
			return;
		tangentangle=tantable[yzangle];
		if (tangentangle!=0)
			{
         dist=FixedDiv2(((shooter->z+32)<<10),(tangentangle<<1));
			xintercept=shooter->x+FixedMul(dist,costable[angle]);
			yintercept=shooter->y-FixedMul(dist,sintable[angle]);
			}
		z=-32;
//      bullethole=5;
		}
	else
		{
		int dx,dy,xydist;


#define CORNERVALUE  0x500


		if (IsWindow(grid[0],grid[1]))
			return;
		if (lastcnt<0)
			{
			xintercept=grid[0]<<16;
			if (smokecondition==gs_door)
				{
				if (thedir[0]<0)
					xintercept+=0x9fff;
				else
					xintercept+=0x5fff;
				yintercept=FixedScale(xintercept-shooter->x,vy,vx)+shooter->y;
				if ((yintercept>>16)!=grid[1])
					{
					if ((yintercept>>16)>grid[1])
						yintercept=(grid[1]<<16)+0xffff;
					else
						yintercept=(grid[1]<<16);
					}
				}
			else if (smokecondition==gs_wall)
				{
				if (thedir[0]<0)
					{
					objtype * ta;

               xintercept += 0x10000;
               yintercept=FixedScale(xintercept-shooter->x,vy,vx)+shooter->y;

               xintercept += SMOKEWALLOFFSET;
					bullethole=1;
               if (yintercept < ((grid[1] << 16) + CORNERVALUE))
                 bullethole = 0;
               else if (yintercept > ((grid[1] << 16) + 0x10000 - CORNERVALUE))
                 bullethole = 0;

					ta = (objtype*)actorat[grid[0]][grid[1]];
					if ((ta) && (ta->which==PWALL))
						bullethole=0;
					}
				else
					{
					objtype * ta;

               yintercept=FixedScale(xintercept-shooter->x,vy,vx)+shooter->y;
               xintercept-=SMOKEWALLOFFSET;
					bullethole=2;
               if (yintercept < ((grid[1] << 16) + CORNERVALUE))
                 bullethole = 0;
               else if (yintercept > ((grid[1] << 16) + 0x10000 - CORNERVALUE))
                 bullethole = 0;

					ta = (objtype*)actorat[grid[0]][grid[1]];
					if ((ta) && (ta->which==PWALL))
						bullethole=0;
					}
				}
			}
		else
			{
			yintercept=grid[1]<<16;
			if (smokecondition==gs_door)
				{
				if (thedir[1]<0)
					yintercept+=0x9fff;
				else
					yintercept+=0x5fff;
				xintercept=FixedScale(yintercept-shooter->y,vx,vy)+shooter->x;
				if ((xintercept>>16)!=grid[0])
					{
					if ((xintercept>>16)>grid[0])
						xintercept=(grid[0]<<16)+0xffff;
					else
						xintercept=(grid[0]<<16);
					}
				}
			else if (smokecondition==gs_wall)
				{
				if (thedir[1]<0)
					{
					objtype * ta;


               yintercept += 0x10000;
               xintercept=FixedScale(yintercept-shooter->y,vx,vy)+shooter->x;

               yintercept += SMOKEWALLOFFSET;
					bullethole=3;
               if (xintercept < ((grid[0] << 16) + CORNERVALUE))
                 bullethole = 0;
               else if (xintercept > ((grid[0] << 16) + 0x10000 - CORNERVALUE))
                 bullethole = 0;

					ta = (objtype*)actorat[grid[0]][grid[1]];
					if ((ta) && (ta->which==PWALL))
						bullethole=0;
					}
				else
					{
					objtype * ta;

               xintercept=FixedScale(yintercept-shooter->y,vx,vy)+shooter->x;
               yintercept-=SMOKEWALLOFFSET;
					bullethole=4;
               if (xintercept < ((grid[0] << 16) + CORNERVALUE))
                 bullethole = 0;
               else if (xintercept > ((grid[0] << 16) + 0x10000 - CORNERVALUE))
                 bullethole = 0;

					ta = (objtype*)actorat[grid[0]][grid[1]];
					if ((ta) && (ta->which==PWALL))
						bullethole=0;
					}
				}
			}
		dx = xintercept - shooter->x;
		dy = shooter->y - yintercept;
		xydist = FindDistance(dx,dy);
		if (shooter->obclass==playerobj)
			{
			playertype * pstate;

			M_LINKSTATE(shooter,pstate);
         z=shooter->z-FixedMulShift(xydist,tantable[yzangle],25)+pstate->playerheight-32;
			}
		else
         z=shooter->z-FixedMulShift(xydist,tantable[yzangle],25);
		if (smokecondition==gs_wall)
			{
			if (z<-32)
				z=-32;
			}
		}
	SpawnGunSmoke(xintercept,yintercept,z,angle,bullethole);
}


/*
=====================
=
= T_BossDied ()
=
=====================
*/

void T_BossDied (objtype *ob)
{

 if (ob->ticcount)
  return;

	switch (ob->obclass)
	{
		case b_darianobj:
		case b_heinrichobj:
		case b_darkmonkobj:
		case b_robobossobj:
		case b_darksnakeobj:
			playstate = ex_bossdied;
		break;
	default:
	    ;
	}
}


/*
=====================
=
= T_Wind ()
=
=====================
*/

static int WindDistance = 1000;
static int WindCurrentDistance = 1000;
static int WindHandle = -1;
static int WindLastTic = -1;
static int WindPlaying = false;
static int WindPitch = 0;
static int WindDestPitch = 0;
static int WindPitchRate = 0;

void T_Wind
	(
	objtype *ob
	)

	{
	int distance;
	int dx;
	int dy;

	if ( ( GetTicCount() - WindLastTic ) > 0 )
		{
		WindDistance = 1000;

		WindPitch += WindPitchRate;
		if ( WindPitch == WindDestPitch )
			{
			WindDestPitch = ( RandomNumber( "Wind Pitch", 0 ) - 128 ) << 3;
			WindPitchRate = 1;
			if ( WindDestPitch < WindPitch )
				{
				WindPitchRate = -WindPitchRate;
				}
			}
		}
	WindLastTic = GetTicCount();

	dx = ( ob->x - PLAYER[0]->x );
	dy = ( PLAYER[0]->y - ob->y );

	distance = 1000;
   if ( areabyplayer[ ob->areanumber ] )
      {
      distance = ( FindDistance( dx, dy ) ) >> 13;
      }

   if ( distance < WindDistance )
      {
      WindDistance = distance;
      }

   if ( WindDistance < 255 )
      {
      WindPlaying = true;
      WindCurrentDistance = WindDistance;
      }
   else
      {
      if ( WindPlaying )
         {
         WindCurrentDistance += 3;
         }
      }

   if ( WindPlaying )
      {
      if ( WindCurrentDistance < 255 )
         {
         if ( !SD_SoundActive( WindHandle ) )
            {
            WindHandle = SD_PlayPitchedSound( SD_WINDSND,
               255 - WindCurrentDistance, WindPitch );
            }
         else
            {
            SD_SetSoundPitch( WindHandle, WindPitch );
            SD_SetPan( WindHandle, 255 - WindCurrentDistance, 255 - WindCurrentDistance,
               255 - WindCurrentDistance );
            }
         }
      else
         {
         SD_StopSound( WindHandle );
         WindPlaying = false;
         }
      }
   }

/*
=====================
=
= StopWind ()
=
=====================
*/

void StopWind
   (
   void
   )

   {
   objtype *temp;

   FX_SetReverb( 0 );

   SD_StopSound( WindHandle );
   WindDistance        = 1000;
   WindCurrentDistance = 1000;
   WindHandle          = -1;
   WindLastTic         = -1;
   WindPlaying         = false;
   WindPitch           = 0;
   WindDestPitch       = 0;
   WindPitchRate       = 0;


   for(temp=FIRSTACTOR;temp;temp=temp->next)
      {
      if (temp->soundhandle != -1)
         SD_StopSound(temp->soundhandle);
      }
   }

