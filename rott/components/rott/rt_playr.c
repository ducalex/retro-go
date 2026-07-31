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
#include <dos.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "watcom.h"
#include "rt_sound.h"
#include "gmove.h"
#include "states.h"
#include "rt_sqrt.h"
#include "rt_actor.h"
#include "rt_main.h"
#include "rt_playr.h"
#include "isr.h"
#include "rt_draw.h"
#include "rt_ted.h"
#include "rt_door.h"
#include "rt_menu.h"
#include "rt_view.h"
#include "rt_com.h"
#include "rt_in.h"
#include "rt_util.h"
#include "rt_game.h"
#include "rt_rand.h"
#include "z_zone.h"
#include "rt_swift.h"
#include "engine.h"
#include "_rt_play.h"
#include "rt_cfg.h"
#include "rt_spbal.h"
#include "rt_floor.h"
#include "develop.h"
#include "rt_msg.h"
#include "rt_debug.h"
#include "sprites.h"
#include "rt_net.h"
#include "rt_dmand.h"
//MED
#include "memcheck.h"


#define FLYINGZMOM  350000


#if (DEVELOPMENT == 1)
#include "rt_str.h"
#endif

extern boolean usejump;


specials CurrentSpecialsTimes =
      {
      60*VBLCOUNTER, // god
      60*VBLCOUNTER, // dog
      20*VBLCOUNTER, // shrooms
      20*VBLCOUNTER, // elasto
      60*VBLCOUNTER, // asbestos vest
      60*VBLCOUNTER, // bullet proof vest
      GASTICS, // gas mask
      60*VBLCOUNTER, // mercury mode

      300*VBLCOUNTER, // god respawn
      60*VBLCOUNTER, // dog respawn
      60*VBLCOUNTER, // shrooms respawn
      60*VBLCOUNTER, // elasto respawn
      60*VBLCOUNTER, // asbestos vest respawn
      60*VBLCOUNTER, // bullet proof vest respawn
      60*VBLCOUNTER, // gas mask respawn
      60*VBLCOUNTER  // mercury mode respawn
      };

int GRAVITY = NORMAL_GRAVITY;

#ifndef RETRO_GO
ROTTCHARS characters[5]={
   {0x2100,0x4800,100,2,25},  // Taradino Cassatt
   {0x2200,0x5200,85,3,32},   // Thi Barrett
   {0x1f00,0x4000,150,3,20},  // Doug Wendt
   {0x2300,0x5500,70,2,33},   // Lorelei Ni
   {0x2000,0x4400,120,3,25}}; // Ian Paul Freeley
#endif

static const int TD = MINACTORDIST+0x1000;
static const int STRAFEAMOUNT = ((KEYBOARDNORMALTURNAMOUNT >> 10) + (KEYBOARDNORMALTURNAMOUNT >> 12));


static const int GODYZANGLE     = -(9*FINEANGLES/360);
static const int DOGYZANGLE     =  (4*FINEANGLES/360);
static const int SHROOMYZANGLE  =  (15*FINEANGLES/360);
static const int FALLINGYZANGLE = -(15*FINEANGLES/360);
static const int NORMALYZANGLE  = 0;


/*
=============================================================================

			GLOBAL VARIABLES

=============================================================================
*/

int controlbuf[3];
int buttonbits;
extern _2Dpoint LASTSOUND;
//
// player state info
//

#ifndef RETRO_GO
statobj_t      *DEADPLAYER[MAXDEAD];
#endif
int            NUMDEAD;
int            lastpolltime;

#ifndef RETRO_GO
statobj_t      *BulletHoles[MAXBULLETS];
#endif
int            BulletHoleNum;

#ifndef RETRO_GO
objtype        *PLAYER[MAXPLAYERS];
#endif
objtype        *player;
#ifndef RETRO_GO
playertype     PLAYERSTATE[MAXPLAYERS];

playertype     *locplayerstate;
#endif


gametype       gamestate;

boolean        godmode = false;

boolean       missilecam=false;
objtype       * missobj=NULL;
// Player control variables

int KX = 0;
int KY = 0;
int MX = 0;
int MY = 0;
int JX = 0;
int JY = 0;
int CX = 0;
int CY = 0;
boolean vrenabled = false;
int VX = 0;
int VY = 0;

int oldcyberx = 0;
int oldcybery = 0;
int CYBERDEADRANGE = 6000;
boolean CYBERLOOKUP,CYBERLOOKDOWN;

int leftmom = 0;
int rightmom = 0;
int lastmom = 0;
int first    = 1;

int pausedstartedticcount;
boolean RefreshPause = true;

boolean  buttonpoll[NUMBUTTONS];

int      buttonscan[NUMBUTTONS] = {sc_Control, sc_Alt, sc_RShift, sc_Space,
											  sc_PgUp,sc_PgDn,sc_Enter,sc_Delete,
                                   sc_Home,sc_End,sc_1,sc_2,sc_3,sc_4,
                                   sc_CapsLock, sc_F12,
                                   sc_Comma,sc_Period,sc_BackSpace,sc_A,
                                   sc_UpArrow, sc_RightArrow,
                                   sc_DownArrow, sc_LeftArrow,
                                   sc_Tab, sc_T, sc_Z };

int      joyxmax = 0, joyymax = 0, joyxmin = 0, joyymin = 0;

int      buttonmouse[6] = {bt_attack, bt_strafe, di_north,
                           bt_nobutton, bt_use, bt_nobutton };

int      buttonjoy[8] = {bt_attack, bt_strafe, bt_run, bt_use,
                         bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton };

williamdidthis FREE = {84,5,0,0,9,{{done,2,1},{done,2,2},{done,2,3},
  {done,2,4},{done,2,5},{done,2,6},{done,2,7},{done,2,8},
  {reset,2,9}}};

williamdidthis DOGSCRATCH = {128,5,0,0,4,{{done,2,8},{at_pulltrigger,2,9},{done,2,10},
                             {reset,2,11}}};

williamdidthis DOGLICK =    {128,5,0,0,4,{{done,2,4},{done,2,5},{done,2,6},
                             {reset,2,7}}};


williamdidthis WEAPONS[MAXWEAPONS] =

{
 {100,-1,10,0x2000l,3,{{at_pulltrigger,4,1},{done,4,2},{reset,4,0}}}, //single pistol
 {100,-1,10,0x2000l,6,{{at_pulltrigger,2,1},{done,2,2},{done,2,3},
						{at_pulltrigger,2,4},{done,2,5},{reset,2,3}}}, // double pistol
 {70,-1,10,0x2000l,2,{{at_pulltrigger,2,1},{reset,1,2}}}, //mp 40
 {80,5,10,0x50000l,4,{{at_missileweapon,2,1},{done,2,2},{reset,2,3},{reset2,6,0}}}, //bazooka
 {80,5,10,0x10000l,4,{{at_missileweapon,2,1},{done,2,2},{reset,2,3},{reset2,6,0}}}, //firebomb
 {80,5,10,0x50000l,4,{{at_missileweapon,2,1},{done,2,2},{reset,2,3},{reset2,6,0}}}, //heatseeker
 {80,5,10,0x10000l,4,{{at_missileweapon,2,1},{done,2,2},{reset,2,3},{reset2,6,0}}}, //drunk
 {80,5,7,0x10000l,4,{{at_missileweapon,2,1},{done,2,2},{reset,2,2},{reset2,6,0}}}, // firewall
 {125,5,7,0x10000l,7,{{done,3,1},{done,3,2},{done,3,3},{done,3,4},
						  {at_missileweapon,3,5},{done,3,6},{reset,3,7}}},	//GODHAND

#if (SHAREWARE == 0)
 {80,5,7,0x10000l,4,{{at_missileweapon,2,1},{done,2,2},{reset,2,3},{reset2,6,0}}}, //split
 {80,5,7,0x10000l,9,{{done,5,1},{done,5,2},{done,5,3},{done,5,4},
						{at_missileweapon,10,5},{done,5,4},{done,5,3}, // kes
						{done,5,2},{reset,5,1}}},
 {200,5,7,0x10000l,6,{{done,1,1},{done,1,2},{at_pulltrigger,1,3},{at_pulltrigger,1,4},
						  {at_pulltrigger,1,5},{reset,1,6}}},	//BAT
 {128,5,7,0x10000l,3,{{done,2,1},{at_pulltrigger,2,2},{reset,2,3}}}
#endif
};



/*
=============================================================================

					LOCAL FUNCTION PROTOTYPES and VARIABLES

=============================================================================
*/

void     CheckPlayerSpecials(objtype * ob);
void     CheckWeaponStates(objtype * ob);
boolean  CheckSprite (statobj_t* ,int *);
void     T_Tag (objtype *ob);
void     T_Player (objtype *ob);
void     T_BatBlast(objtype*ob);
void     T_Attack (objtype *ob);
void     T_Free (objtype *ob);
void     T_DogUse (objtype *ob);
void     PlayerMove(objtype * ob);
void     Thrust (objtype * ob);
void     CheckWeaponChange (objtype * ob);
void     PlayerMissileAttack(objtype* );
void     Cmd_Use(objtype*);
//void     ComError (char *error, ...);
int      FinddTopYZANGLELIMITvalue(objtype *ob);

statetype s_free = {false,0,0,T_Free,0,&s_free};
statetype s_inelevator = {false,0,420,T_Player,0,&s_player};

#if (SHAREWARE == 0)
statetype s_dogwait = {true,SERIALDOG_W11,50,T_Player,SF_DOGSTATE,&s_serialdog};

statetype s_doguse = {true,SERIALDOG_W11,140,T_DogUse,SF_DOGSTATE,&s_serialdog};
statetype s_doglick = {true,SERIALDOG_W11,0,T_DogLick,SF_DOGSTATE,&s_doglick};
#endif

statetype s_tag = {false,CASSATT_S1,20,T_Tag,0,&s_player};

static SWIFT_3DStatus SWIFTStatus;

//
// curent user input
//

static int turnheldtime;
static int turnaround = 0;
static int turnaroundtime;

//
// Double Click variables
//

static int  DoubleClickTimer[ 3 ]   = { 0 };
static byte DoubleClickCount[ 3 ]   = { 0 };
static byte DoubleClickPressed[ 3 ] = { false };
static int  JoyDblClickTimer[ 4 ]   = { 0 };
static byte JoyDblClickCount[ 4 ]   = { 0 };
static byte JoyDblClickPressed[ 4 ] = { false };


static int PlayerRecording=-1;
static int nettics;

void Move_Player_From_Exit_To_Start(objtype *ob);
void CheckTagGame(objtype *actor1,objtype*actor2);
void CheckFlying(objtype*ob,playertype *pstate);

/*
===============
=
= LoadPlayer
=
===============
*/
void LoadPlayer ( void )
{
	memset (locplayerstate->buttonstate, 0, sizeof(locplayerstate->buttonstate));
   locplayerstate->anglefrac=player->angle<<ANGLEBITS;
   areabyplayer[player->areanumber]=true;
	ConnectAreas();
}


int MaxHitpointsForCharacter(playertype*pstate)
{
   int hp = 100;
   if (BATTLEMODE && (gamestate.BattleOptions.HitPoints != bo_character_hitpoints))
      {
      hp = gamestate.BattleOptions.HitPoints;
      }
   else
      {
      hp = characters[pstate->player].hitpoints;
      }
   if (hp <= 0) {
       hp = 100; // Fallback to prevent crash
   }
   return hp;
}

void InitializeWeapons(playertype*pstate)
{


#if (SHAREWARE == 0)
 if (gamestate.SpawnEluder)
	{pstate->new_weapon = pstate->weapon = pstate->missileweapon = wp_dog;
	 pstate->oldweapon = pstate->oldmissileweapon = wp_dog;
	 pstate->bulletweapon = -1;
	 pstate->HASBULLETWEAPON[wp_pistol] = 0;
	 pstate->HASBULLETWEAPON[wp_twopistol] = 0;
	 pstate->HASBULLETWEAPON[wp_mp40] = 0;
	}
 else
#endif
	{if (gamestate.PlayerHasGun[pstate-&PLAYERSTATE[0]])
		 {pstate->new_weapon = pstate->weapon = pstate->oldweapon =
		  pstate->bulletweapon = wp_pistol;
		  pstate->HASBULLETWEAPON[wp_pistol] = 1;
		  pstate->HASBULLETWEAPON[wp_twopistol] = 0;
		  pstate->HASBULLETWEAPON[wp_mp40] = 0;
		  pstate->missileweapon = pstate->oldmissileweapon = -1;
		 }
	 else
		 {pstate->new_weapon = pstate->weapon = pstate->oldweapon =
		  pstate->bulletweapon = -1;
		  pstate->HASBULLETWEAPON[wp_pistol] = 0;
		  pstate->HASBULLETWEAPON[wp_twopistol] = 0;
		  pstate->HASBULLETWEAPON[wp_mp40] = 0;
		  pstate->missileweapon = pstate->oldmissileweapon = -1;
		 }
	}


 pstate->ammo = -1;
}

void ResetPlayerstate(playertype*pstate)
{

 pstate->batblast = 0;
 pstate->poweruptime = pstate->protectiontime = 0;
 pstate->NETCAPTURED = 0;
 MISCVARS->NET_IN_FLIGHT = 0;
 pstate->weaponuptics = 0;
 pstate->weapondowntics = 0;
 if ((insetupgame==false) || NewGame)
	 pstate->health = MaxHitpointsForCharacter(pstate);
 pstate->keys = 0;

 // Give players all the keys in battle game

 if ( BATTLEMODE )
    {
    pstate->keys = 0x0f;
    }
 pstate->attackframe = pstate->attackcount =
 pstate->weaponframe = 0;
 if (gamestate.battlemode == battle_Tag)
	pstate->weaponheight = TAGHANDHEIGHT;
 else
	pstate->weaponheight = 0;
 pstate->heightoffset = pstate->oldheightoffset = 0;
 if (gamestate.SpawnEluder)
	pstate->playerheight = 40;
 else
	pstate->playerheight = characters[pstate->player].height;
 pstate->falling = false;
 memset (pstate->buttonstate, 0, sizeof(pstate->buttonstate));
 SetPlayerHorizon(pstate,NORMALYZANGLE);
}




/*
===============
=
= SetupPlayerobj
=
===============
*/
void SetupPlayerobj (int tilex, int tiley, int dir, objtype * ob)
{
	playertype *pstate;

	M_LINKSTATE(ob,pstate);

	ob->obclass = playerobj;
	ob->tilex = tilex;
	ob->tiley = tiley;
	actorat[tilex][tiley] = ob;
	ob->areanumber = MAPSPOT(tilex,tiley,0)-AREATILE;
	MakeLastInArea(ob);
	ob->x = ((long)tilex<<TILESHIFT)+TILEGLOBAL/2;
	ob->y = ((long)tiley<<TILESHIFT)+TILEGLOBAL/2;
	ob->z = PlatformHeight(tilex,tiley);
   if ((ob->z == -10) || DiskAt(tilex,tiley))
     ob->z = 0;

	ob->angle = (1-dir)*ANG90;
	ob->which = ACTOR;
	Fix(ob->angle);
   ob->yzangle = 0;

	ob->dir   = angletodir[ob->angle];
	ob->flags = (FL_SHOOTABLE|FL_ABP|FL_BLOCK|FL_COLORED);
	ob->drawx=ob->x;
	ob->drawy=ob->y;
	ob->hitpoints = pstate->health;
	pstate->anglefrac= (ob->angle<<ANGLEBITS);
	pstate->angle=0;
	areabyplayer[ob->areanumber]=true;


	if (ob == player)
		{
		playerdead=false; // local player dead flag
		}
	if (!gamestate.SpawnEluder)
	  ob->shapeoffset = pstate->player*REMOTEOFFSET;

	memset (pstate->buttonstate, 0, sizeof(pstate->buttonstate));
   if (SCREENEYE != NULL)
      {
      NewState(SCREENEYE,&s_megaremove);
      SCREENEYE = NULL;
      }
}



void SetShapeoffset(objtype*ob)
{playertype *pstate;

 M_LINKSTATE(ob,pstate);
 ob->shapeoffset = pstate->player*REMOTEOFFSET;
 ob->flags |= FL_COLORED;
 ob->flags &= ~FL_DYING;

}


/*
===============
=
= RevivePlayerobj
=
===============
*/
void RevivePlayerobj (int tilex, int tiley, int dir, objtype*ob)
{
	playertype *pstate;
	statetype *tstate;

	M_LINKSTATE(ob,pstate);
	tstate = ob->state;
	RemoveFromArea(ob);
	TurnActorIntoSprite(ob);
	if ((LASTSTAT->z < nominalheight) && (!IsPlatform(LASTSTAT->tilex,LASTSTAT->tiley)))
      {
      SpawnParticles(ob,GUTS,10 + gamestate.difficulty);
		RemoveStatic(LASTSTAT);
      }
	else
	  {if (DEADPLAYER[NUMDEAD])
			RemoveStatic(DEADPLAYER[NUMDEAD]);
		DEADPLAYER[NUMDEAD] = LASTSTAT;
		LASTSTAT->linked_to = NUMDEAD;
		NUMDEAD = (NUMDEAD+1)&(MAXDEAD-1);
	  }

	ob->state = tstate;

	SetupPlayerobj (tilex, tiley, dir, ob);
	ConnectAreas();

	ResetPlayerstate(pstate);
	InitializeWeapons(pstate);
   SD_PlaySoundRTP(SD_PLAYERSPAWNSND,ob->x,ob->y);
	if (!gamestate.SpawnEluder)
      {
      ob->shapeoffset = 0;
      ob->flags &= ~FL_COLORED;
      ob->flags |= FL_DYING;
      NewState(ob,&s_respawn1);
      if (gamestate.battlemode == battle_Tag)
         {
         if (BATTLE_Team[ob->dirchoosetime] == BATTLE_It)

            {
            pstate->missileweapon = pstate->oldweapon = pstate->new_weapon =
            pstate->oldmissileweapon = pstate->weapon = wp_godhand;
            pstate->bulletweapon = -1;
            ob->flags |= FL_DESIGNATED;
            }
         else
            {
            pstate->weaponheight = 0;
            }
         }
		}
#if (SHAREWARE == 0)
	else
      NewState(ob,&s_serialdog);
#endif
	if (ob==player)
		DrawPlayScreen(false);
   ob->momentumx = ob->momentumy = ob->momentumz = 0;
}


/*
===============
=
= SpawnPlayerobj
=
===============
*/
void SpawnPlayerobj (int tilex, int tiley, int dir, int playerindex)
{
 playertype *pstate;

	pstate = &PLAYERSTATE[playerindex];

	GetNewActor();
	MakeActive(new);

	// Set player number

	new->dirchoosetime = playerindex;

	// Save off if local player

	if (playerindex==consoleplayer)
		player=new;

	PLAYER[playerindex] = new;

	SetupPlayerobj (tilex, tiley, dir, new);

	if (!gamestate.SpawnEluder)
		NewState(new,&s_player);
#if (SHAREWARE == 0)
   else
      NewState(new,&s_serialdog);
#endif


}

/*
===============
=
= SetupBulletHoleLink
=
===============
*/
void SetupBulletHoleLink (int num, statobj_t * item)
{
   BulletHoles[num] = item;
}

/*
===============
=
= SpawnBulletHole
=
===============
*/
void SpawnBulletHole (int x, int y, int z)
{
   if (M_ISDOOR(x>>16,y>>16))
      return;
   if (BulletHoles[MISCVARS->BulletHoleNum])
      RemoveStatic(BulletHoles[MISCVARS->BulletHoleNum]);
   SpawnInertStatic(x,y,z,stat_bullethole);
	BulletHoles[MISCVARS->BulletHoleNum]=LASTSTAT;
   LASTSTAT->linked_to=MISCVARS->BulletHoleNum;
	MISCVARS->BulletHoleNum = (MISCVARS->BulletHoleNum+1)&(MAXBULLETS-1);
}




void SpawnGunSmoke(int x, int y, int z, int angle, int bullethole)

{
	int chance;

	if ((x<=0) || (y<=0))
      {
      SoftError("SpawnGunSmoke: xy below angle=%d\n",angle);
      return;
      }

   if ((bullethole!=0) && (z>=-32) && (z<=maxheight))
      switch (bullethole)
         {
         case 1:
            SpawnBulletHole(x-BULLETHOLEOFFSET,y,z);
            break;
         case 2:
            SpawnBulletHole(x+BULLETHOLEOFFSET,y,z);
            break;
         case 3:
            SpawnBulletHole(x,y-BULLETHOLEOFFSET,z);
            break;
         case 4:
            SpawnBulletHole(x,y+BULLETHOLEOFFSET,z);
            break;
         case 5:
            SpawnBulletHole(x,y,z);
            break;
         default:
            Error("Invalid bullethole value\n");
            break;
         }

   SpawnInertActor(x,y,z);

   NewState(new,&s_gunsmoke1);

   if (angle < ANGLES/4)
	 {if ((angle < (3*ANGLES/16)) && (angle > (ANGLES/16)))
		chance = 128;
	  else
		chance = 20;
	 }
	else if (angle < ANGLES/2)
	 {if ((angle < (7*ANGLES/16)) && (angle > (5*ANGLES/16)))
		chance = 128;
	  else
		chance = 20;
	 }
	else if (angle < 3*ANGLES/4)
	 {if ((angle < (11*ANGLES/16)) && (angle > (9*ANGLES/16)))
		chance = 128;
	  else
		chance = 20;
	 }
	else
	 {if ((angle < (15*ANGLES/16)) && (angle > (13*ANGLES/16)))
		chance = 128;
	  else
		chance = 20;
	 }

	if (RandomNumber("Wall ricochet check",0)<chance)
	 {int rand;

	  rand = RandomNumber("Spawn Ricochet Sound in SpawnGunSmoke",0);
	  if (rand < 80)
		 SD_PlaySoundRTP(SD_RICOCHET1SND,new->x,new->y);
	  else if (rand < 160)
		 SD_PlaySoundRTP(SD_RICOCHET2SND,new->x,new->y);
	  else
		 SD_PlaySoundRTP(SD_RICOCHET3SND,new->x,new->y);
	 }
}

void  SpawnBlood(objtype * ob, int angle)
   {

   SpawnInertActor(ob->x-(costable[angle]>>5),
                   ob->y+(sintable[angle]>>5),ob->z);

   NewState(new,&s_bloodspurt1);

   if ((new->x<=0) || (new->y<=0))
	   Error("SpawnBlood: bad x,y obj->obclass=%d\n",ob->obclass);
   }

void  SpawnMetalSparks(objtype * ob, int angle)
   {
   int rand,dispx=0,dispy=0;


   if (ob->which == ACTOR)
      {
      dispx = ob->momentumx;
      dispy = ob->momentumy;
      }

   SpawnInertActor(ob->x-(costable[angle]>>3)+dispx,
                   ob->y+(sintable[angle]>>3)+dispy,ob->z);

   if (GameRandomNumber("Spawn Metal Sparks",0)<128)
	  NewState(new,&s_hitmetalactor1);
	else
	  NewState(new,&s_hitmetalwall1);

	rand = RandomNumber("Spawn Ricochet Sound",0);
	if (rand < 80)
	 SD_PlaySoundRTP(SD_RICOCHET1SND,new->x,new->y);
	else if (rand < 160)
	 SD_PlaySoundRTP(SD_RICOCHET2SND,new->x,new->y);
	else
	 SD_PlaySoundRTP(SD_RICOCHET3SND,new->x,new->y);
	if ((new->x<=0) || (new->y<=0))
	   Error("SpawnMetalSparks: bad x,y obj->obclass=%d\n",ob->obclass);
}

/*
===============
=
= UnTargetActor ( objtype * target )
=
===============
*/
void UnTargetActor ( objtype * target )
{
   int i;

   for (i=0;i<numplayers;i++)
      {
      if (PLAYERSTATE[i].guntarget==target)
         {
         PLAYERSTATE[i].guntarget=NULL;
         SetNormalHorizon(PLAYER[i]);
         }
      }
}



//=============================================================

int GetWeaponForItem(int itemnumber)
   {
   switch (itemnumber)
      {


      case  stat_twopistol:
         return wp_twopistol;

      case  stat_mp40:
         return wp_mp40;

      case  stat_bazooka:
         return wp_bazooka;

      case  stat_heatseeker:
         return wp_heatseeker;

      case  stat_drunkmissile:
         return wp_drunk;

      case  stat_firebomb:
         return wp_firebomb;

      case  stat_firewall:
         return wp_firewall;

      case  stat_godmode:
         return wp_godhand;

#if (SHAREWARE == 0)


      case  stat_splitmissile:
         return wp_split;

      case  stat_kes:
         return wp_kes;

      case  stat_bat:
         return wp_bat;


      case  stat_dogmode:
         return wp_dog;
#endif
      }
    return 0;
   }


int GetItemForWeapon(int weapon)
   {
   switch (weapon)
      {

      case wp_twopistol:
         return stat_twopistol;

      case  wp_mp40:
         return stat_mp40;

      case  wp_bazooka:
         return stat_bazooka;

      case  wp_heatseeker:
         return stat_heatseeker;

      case  wp_drunk:
         return stat_drunkmissile;

      case  wp_firebomb:
         return stat_firebomb;

      case  wp_firewall:
         return stat_firewall;

      case  wp_godhand:
         return stat_godmode;

#if (SHAREWARE == 0)

      case  wp_split:
         return stat_splitmissile;

      case  wp_kes:
         return stat_kes;

      case  wp_bat:
         return stat_bat;


      case  wp_dog:
         return stat_dogmode;
#endif
      }
    return -1;
   }



#define MF_SINGULAR 0x01

missile_stats PlayerMissileData[13] =

    {
     {0,0,0,0,0},
     {0,0,0,0,0},
     {0,0,0,0,0},

     {&s_p_bazooka1,0x6000,p_bazookaobj,0x7000,MF_SINGULAR},
     {&s_p_bazooka1,0x8000,p_heatseekobj,0x7000,MF_SINGULAR},
     {&s_p_bazooka1,0x6000,p_drunkmissileobj,0x7000,0},
     {&s_p_bazooka1,0x8000,p_firebombobj,0x7000,MF_SINGULAR},
     {&s_p_grenade,0x4000,p_firewallobj,0x8000,0},
     {&s_godfire1,0x3800,p_godballobj,0x8000,0},
     {&s_p_bazooka1,0x8000,p_splitmissileobj,0x7000,MF_SINGULAR},
#if (SHAREWARE == 0)
     {&s_kessphere1,0xc000,p_kesobj,0x5000,MF_SINGULAR},
#else
     {0,0,0,0,0},
#endif
     {0,0,0,0,0},
     {0,0,0,0,0}

    };


void MissileAutoTarget(objtype *ob,missile_stats *mdata)
   {
   int dx,dy,angle,mindist,currdist,magangle,saveangle,
       xydist,dz,yzangle,oldyzangle,saveyzangle;
   objtype *target,*temp;

   mindist = 0x7fffffff;
   target = NULL;
   for(temp = firstactive;temp;temp=temp->nextactive)
      {
      if (temp == ob)
         continue;
      if ((!(temp->flags & FL_SHOOTABLE)) || (temp->flags & FL_DYING))
         continue;
      if (!CheckLine(ob,temp,SHOOT))
         continue;

      dx = temp->x-ob->x;
      dy = ob->y-temp->y;
      dz = ob->z-temp->z;
      xydist = FindDistance(dx,dy);
      yzangle = atan2_appx(xydist,dz<<10);

      angle = atan2_appx(dx,dy);

      magangle = abs(ob->angle - angle);
      if (magangle > VANG180)
         magangle = ANGLES - magangle;

      if (magangle > ANGLESDIV8)
         continue;

      currdist = FindDistance(ob->x-temp->x,ob->y-temp->y);
      if (currdist < mindist)
         {
         mindist = currdist;
         target = temp;
         saveangle = angle;
         saveyzangle = yzangle;
         }
      }

   if (target)
      {
      oldyzangle = ob->yzangle;
      ob->yzangle = saveyzangle;
      SpawnMissile(ob,mdata->obclass,mdata->speed,saveangle,
                   mdata->state,mdata->offset);
      ob->yzangle = oldyzangle;
      }

   else if (ob->flags&FL_GODMODE)
      {
      int saveangle;

      saveangle=ob->yzangle;
      ob->yzangle -= GODYZANGLE;
      Fix(ob->yzangle);
      SpawnMissile(ob,mdata->obclass,mdata->speed,
                   ob->angle,mdata->state,mdata->offset);
      ob->yzangle=saveangle;
      }
   else
      SpawnMissile(ob,mdata->obclass,mdata->speed,ob->angle,
                   mdata->state,mdata->offset);

  }


void PlayerMissileAttack(objtype*ob)

   {
   playertype * pstate;
   missile_stats *newmissiledata;
   M_LINKSTATE(ob,pstate);


   MISCVARS->madenoise = true;
   newmissiledata = &PlayerMissileData[pstate->missileweapon];

   // ready to annihilate this poor bastard

   if ((newmissiledata->obclass == p_godballobj) ||
       (newmissiledata->obclass == p_kesobj))
      MissileAutoTarget(ob,newmissiledata);

   else
      {
      SpawnMissile(ob,newmissiledata->obclass,newmissiledata->speed,ob->angle,
                  newmissiledata->state,newmissiledata->offset);

      if (newmissiledata->obclass == p_drunkmissileobj)
         {
         int i;

         for(i=0;i<4;i++)
            {
            if (!MissileTryMove(new,new->x+new->momentumx,new->y+new->momentumy,new->z))
               {
               new->x = new->drawx = ob->x + (costable[new->angle]>>3);
               new->y = new->drawy = ob->y - (sintable[new->angle]>>3);

               ob->momentumx = -FixedMul(0x5000l,costable[ob->angle]);
               ob->momentumy = FixedMul(0x5000l,sintable[ob->angle]);
               }

            SpawnMissile(ob,newmissiledata->obclass,newmissiledata->speed,ob->angle,
                        newmissiledata->state,newmissiledata->offset);


            }
         }
      }


   if (newmissiledata->flags & MF_SINGULAR)
      PLAYER0MISSILE = new;

   SD_PlaySoundRTP(BAS[new->obclass].fire,ob->x,ob->y);

   // if (new->obclass == p_godballobj)
   //   new->z += 10;

   new->dirchoosetime = 5;
   if (missilecam==true)
      missobj=new;
   if (!MissileTryMove(new,new->x+new->momentumx,new->y+new->momentumy,new->z))
      {
      new->x = new->drawx = ob->x + (costable[new->angle]>>3);
      new->y = new->drawy = ob->y - (sintable[new->angle]>>3);

      ob->momentumx = -FixedMul(0x5000l,costable[ob->angle]);
      ob->momentumy = FixedMul(0x5000l,sintable[ob->angle]);
      }

   }

//====================================================================


boolean InRange (objtype *p, objtype *victim, int distance)
{
	int dx,dy;
	int angle;
	int magangle;

   if (victim->which==SPRITE)
		{
	   dx = ((statobj_t *)victim)->x - p->x;
	   dy = p->y - ((statobj_t *)victim)->y;
      }
   else
      {
	   dx = victim->x - p->x;
	   dy = p->y - victim->y;
      }
	angle = atan2_appx (dx,dy);

	magangle = abs(p->angle - angle);
	if (magangle > VANG180)
		 magangle = ANGLES - magangle;
	if (magangle<(75-(distance>>16)))
		return true;
	else
		return false;
}


void DogAttack(objtype*ob)
   {
   objtype *temp;
   int dx,dy,dz;

   SD_PlaySoundRTP(SD_DOGMODEBITE1SND+(RandomNumber("DogAttack",0)>>7),ob->x,ob->y);
   for(temp=firstareaactor[ob->areanumber];temp;temp=temp->nextinarea)
      {

      if (temp->obclass > b_heinrichobj)
         continue;


      if ((temp == ob) || (temp->obclass == roboguardobj))
         continue;

      if ((!(temp->flags & FL_SHOOTABLE)) || (temp->flags & FL_DYING))
         continue;
      if (temp->obclass == collectorobj)
         continue;


      dx = abs(temp->x - ob->x);
      if (dx > 0xc000)
         continue;

      dy = abs(temp->y - ob->y);
      if (dy > 0xc000)
         continue;

      dz = abs(temp->z - ob->z);
      if (dz > (0xc000>>10))
         continue;

      DamageThing(temp,30);
      if (gamestate.violence == vl_excessive)
         SpawnParticles(temp,GUTS,15);
      Collision(temp,ob,-temp->momentumx,-temp->momentumy);
      if ((temp->obclass == playerobj) && (temp->flags & FL_DYING))
         BATTLE_PlayerKilledPlayer(battle_kill_with_missile,ob->dirchoosetime,temp->dirchoosetime);

      return;
      }
   }


void DogBlast(objtype*ob)
   {
   int txl,txh,tyl,tyh,radius = 0x70000,x,y,tile;
   objtype*temp;
   statobj_t*tstat;

   txl = ((ob->x - radius)>>TILESHIFT);
   tyl = ((ob->y - radius)>>TILESHIFT);

   txh = ((ob->x + radius)>>TILESHIFT);
   tyh = ((ob->y + radius)>>TILESHIFT);

   if (txl < 1)
      txl = 1;
   if (txh > MAPSIZE-1)
      txh = MAPSIZE-1;
   if (tyl < 1)
      tyl = 1;
   if (tyh > MAPSIZE-1)
      tyh = MAPSIZE-1;

   for(x=txl;x<=txh;x++)
      for(y=tyl;y<=tyh;y++)
         {
         temp = (objtype*)actorat[x][y];

         if (temp && (temp->which == ACTOR) && (temp->flags & FL_SHOOTABLE) &&
             (temp != ob) && (temp->obclass < roboguardobj) &&
             (temp->flags & FL_ABP)
            )
            {
            DamageThing(temp,100);
            if ((temp->hitpoints<=0) && (temp->obclass < roboguardobj))
               {
               MISCVARS->supergibflag = true;
               temp->flags |= FL_HBM;
               }
            Collision(temp,ob,0,0);
            MISCVARS->supergibflag = false;

            if ((temp->obclass == playerobj) && (temp->flags & FL_DYING))
               BATTLE_PlayerKilledPlayer(battle_kill_with_missile,ob->dirchoosetime,temp->dirchoosetime);

            }

         tile = tilemap[x][y];
         if ((tile & 0x4000) && (tile & 0x8000))
            {
            maskedwallobj_t * mw;

            mw=maskobjlist[tile&0x3ff];
            if ((mw->flags & MW_SHOOTABLE) && (mw->flags & MW_ABP))
               UpdateMaskedWall(tile&0x3ff);
            }

         tstat = sprites[x][y];
         if (tstat && (tstat->flags & FL_SHOOTABLE) && (tstat->flags & FL_ABP))
               DamageThing(tstat,50);
         }
   }
/*
void DogBlast(objtype*ob)
   {
   int txl,txh,tyl,tyh,radius = 0x70000,x,y,tile;
   objtype*temp;
   statobj_t*tstat;

   txl = ((ob->x - radius)>>TILESHIFT);
   tyl = ((ob->y - radius)>>TILESHIFT);

   txh = ((ob->x + radius)>>TILESHIFT);
   tyh = ((ob->y + radius)>>TILESHIFT);

   if (txl < 1)
      txl = 1;
   if (txh > MAPSIZE-1)
      txh = MAPSIZE-1;
   if (tyl < 1)
      tyl = 1;
   if (tyh > MAPSIZE-1)
      tyh = MAPSIZE-1;

   for(x=txl;x<=txh;x++)
      for(y=tyl;y<=tyh;y++)
         {
         temp = (objtype*)actorat[x][y];

         if (temp && (temp->which == ACTOR) && (temp->flags & FL_SHOOTABLE) &&
            (temp != ob) && (temp->obclass < roboguardobj))
            {
            DamageThing(temp,100);
            if ((temp->hitpoints<=0) && (temp->obclass < roboguardobj))
               {
               MISCVARS->supergibflag = true;
               temp->flags |= FL_HBM;
               }
            Collision(temp,ob,0,0);
            MISCVARS->supergibflag = false;

            if ((temp->obclass == playerobj) && (temp->flags & FL_DYING))
               BATTLE_PlayerKilledPlayer(battle_kill_with_missile,ob->dirchoosetime,temp->dirchoosetime);

            }

         tile = tilemap[x][y];
         if ((tile & 0x4000) && (tile & 0x8000))
            {
            maskedwallobj_t * mw;

            mw=maskobjlist[tile&0x3ff];
            if (mw->flags & MW_SHOOTABLE)
               UpdateMaskedWall(tile&0x3ff);
            }

         tstat = sprites[x][y];
         if (tstat && (tstat->flags & FL_SHOOTABLE))
            DamageThing(tstat,50);
         }
   }
*/

void BatBlast(objtype*ob)
{int angle;
 playertype *pstate;

 M_LINKSTATE(ob,pstate);

 angle = ob->angle - ANGLES/8 + ((++pstate->batblast)*ANGLES/48);
 Fix(angle);
#if (SHAREWARE == 0)
 SpawnMissile(ob,p_bazookaobj,0x6000,angle,&s_batblast1,0xa000);
#endif

}



void BatAttack(objtype*ob)
{objtype *temp,*temp2;
 objtype *grenadetarget;
 statobj_t*tstat;
 int dx,dy,dz,angle,momx,momy,op,magangle;
 int tilexlow,tilexhigh;
 int tileylow,tileyhigh;
 int radius =0x10000;
 int x,y;

 SD_PlaySoundRTP(SD_EXCALISWINGSND,ob->x,ob->y);
 for(temp=firstareaactor[ob->areanumber];temp;temp=temp->nextinarea)
  {if (temp == ob)
	 continue;

   if (temp->flags & FL_DYING)
     continue;

   if ((temp->obclass != grenadeobj) &&
       (!((temp->obclass >= grenadeobj) && (temp->obclass <= p_godballobj))) &&
		 (!(temp->flags & FL_SHOOTABLE) ||
		  (temp->obclass >= roboguardobj))
		)
	 continue;

	dx = abs(temp->x - ob->x);
	dy = abs(temp->y - ob->y);
	dz = abs(temp->z - ob->z);
   if ((dx > 0x10000) || (dy > 0x10000) || (dz > 20))
	 continue;

	magangle = abs(ob->angle - AngleBetween(ob,temp));
	if (magangle > VANG180)
		 magangle = ANGLES - magangle;

	if (magangle > ANGLES/8)
	  continue;


	angle= ob->angle+ANGLES/16;
	Fix(angle);

   if ((temp->obclass >= grenadeobj) && (temp->obclass <= p_godballobj))
     {
      temp->angle += ANGLES/2;
      Fix(temp->angle);
      temp->momentumx = temp->momentumy = temp->momentumz = 0;
      ParseMomentum(temp,temp->angle);
      temp->whatever = ob;
      temp->target = NULL;
      continue;
     }


   else if (temp->obclass != grenadeobj)
	  {momx = FixedMul(0x3000l,costable[angle]);
		momy = -FixedMul(0x3000l,sintable[angle]);
		if (levelheight > 2)
		 {op = FixedMul(GRAVITY,(maxheight-100)<<16) << 1;
		  temp->momentumz = -FixedSqrtHP(op);
		 }
		temp->flags |= FL_NOFRICTION;
		SD_PlaySoundRTP(SD_EXCALIHITSND,ob->x,ob->y);
		if ((gamestate.violence == vl_excessive) && (GameRandomNumber("Bat Gibs",0) < 150))
		  {temp->flags |= FL_HBM;
			DamageThing(temp,50);
		  }
		else
			DamageThing(temp,10);
		if ((temp->flags & FL_HBM) && (temp->hitpoints > 0))
		  temp->flags &= ~FL_HBM;
      Collision(temp,ob,momx,momy);
		if ((temp->obclass == blitzguardobj) && (temp->state == &s_blitzplead7))
		  {temp->shapeoffset += deathshapeoffset[temp->obclass];
			temp->flags |= FL_ALTERNATE;
			NewState(temp,&s_blitzdie3);
			temp->momentumx = temp->momentumy = 0;
		  }
	  }
	else // find target to hit grenade back at
	  {int rand;

		rand = GameRandomNumber("bat/grenade target",0);
		if (rand < 80)
		  {grenadetarget = (objtype*)(temp->whatever); // hit back at george
         GetMomenta(grenadetarget,ob,&(temp->momentumx),&(temp->momentumy),&(temp->momentumz),0x3000);
		  }
		else if (rand < 160) // hit back at first eligible
		  {

			for(temp2 = firstareaactor[ob->areanumber];temp2;temp2 = temp2->nextinarea)
				{magangle = abs(ob->angle-AngleBetween(ob,temp2));
				 if (magangle > VANG180)
					 magangle = ANGLES - magangle;

				 if (magangle > ANGLES/8)
					 continue;
             GetMomenta(temp2,ob,&(temp->momentumx),&(temp->momentumy),&(temp->momentumz),0x3000);
				 break;
				}
		  }
		else // hit wherever
		  {ob->angle += (rand >> 1);
			Fix(ob->angle);
			ob->momentumx = ob->momentumy = 0;
			ParseMomentum(ob,ob->angle);
		  }


		temp->temp1 = 0x70000;
		NewState(temp,&s_grenade1);
	  }
	break;
  }

 for(tstat=firstactivestat;tstat;tstat=tstat->statnext)
  {
	if (!(tstat->flags & FL_SHOOTABLE))
	  continue;

	dx = abs(tstat->x - ob->x);
	dy = abs(tstat->y - ob->y);
	dz = abs(tstat->z - ob->z);

	if ((dx > 0xc000) || (dy > 0xc000) || (dz > 20))
	 continue;

	magangle = abs(ob->angle - AngleBetween(ob,(objtype*)tstat));
	if (magangle > VANG180)
		 magangle = ANGLES - magangle;

	if (magangle > ANGLES/8)
	  continue;

	DamageThing(tstat,50);


  }

 tilexlow = (int)((ob->x-radius) >>TILESHIFT);
 tileylow = (int)((ob->y-radius) >>TILESHIFT);

 tilexhigh = (int)((ob->x+radius) >>TILESHIFT);
 tileyhigh = (int)((ob->y+radius) >>TILESHIFT);

 for (y=tileylow;y<=tileyhigh;y++)
	for (x=tilexlow;x<=tilexhigh;x++)
		{if ((tilemap[x][y]&0x8000) && (tilemap[x][y]&0x4000))
			{maskedwallobj_t * mw;

			 mw=maskobjlist[tilemap[x][y]&0x3ff];
			 if (mw->flags&MW_SHOOTABLE)
					UpdateMaskedWall(tilemap[x][y]&0x3ff);
			}
		}

}

void AutoTargetHorizon(objtype *ob)
   {
   int dx,dy,angle,mindist,magangle,
       xydist,dz;
   objtype *temp;
   playertype * pstate;

	M_LINKSTATE(ob,pstate);

   mindist = 0x7fffffff;
   for(temp = firstactive;temp;temp=temp->nextactive)
      {
      if (temp == ob)
         continue;
      if ((!(temp->flags & FL_SHOOTABLE)) || (temp->flags & FL_DYING))
         continue;
      if (!CheckLine(ob,temp,SHOOT))
         continue;

      dx = temp->x-ob->x;
      dy = ob->y-temp->y;
      dz = ob->z-temp->z;//-pstate->playerheight+32;

      xydist = FindDistance(dx,dy);
      if (abs(dz<<10)>xydist)
         continue;


      angle = atan2_appx(dx,dy);

      magangle = ob->angle - angle;
      Fix(magangle);

      if (
          (magangle>=(ANGLESDIV8/4)) &&
          (magangle<=(FINEANGLES-(ANGLESDIV8/4)))
         )
         {
         continue;
         }

      if (xydist < mindist)
         {
         mindist = xydist;
         pstate->guntarget=temp;
         temp->flags |= FL_TARGET;
         pstate->targettime=oldpolltime+(VBLCOUNTER);
         }
      }
  }

void  GunAttack (objtype *ob)
{
	playertype * pstate;
	int      damage;

	M_LINKSTATE(ob,pstate);

	MISCVARS->madenoise = true;

 switch (pstate->weapon)
	{
	case wp_pistol:
		SD_PlaySoundRTP(SD_ATKPISTOLSND,ob->x,ob->y);
		damage=DMG_PISTOL;
		break;

	case wp_mp40:
		SD_PlaySoundRTP(SD_ATKMP40SND,ob->x,ob->y);
		damage=DMG_MP40;
		break;

	case wp_twopistol:
		SD_PlaySoundRTP(SD_ATKTWOPISTOLSND,ob->x,ob->y);
		damage=DMG_PISTOL;
		break;
	}

   AutoTargetHorizon(ob);
   RayShoot (ob, damage, (characters[pstate->player].accuracy+gamestate.difficulty)<<3);

}


/*
===============
=
= Cmd_Fire
=
===============
*/
void Cmd_Fire (objtype*ob)
   {
   playertype *pstate;

   M_LINKSTATE(ob,pstate);

//   pstate->buttonheld[bt_attack] = true;

   if (pstate->NETCAPTURED && (!pstate->HASKNIFE))
      return;

   if (W_CHANGE(pstate))
      return;

   pstate->attackframe = 0;

   if ((ob==player) && (pstate->weapon < wp_mp40) && (!pstate->NETCAPTURED))
      gamestate.DODEMOCRATICBONUS1 = false;

   if (!pstate->NETCAPTURED)
      {
      if (pstate->weapon <= wp_mp40)
         NewState(ob,&s_pgunattack1);
 #if (SHAREWARE == 0)
      else if ((pstate->weapon == wp_bat) && (pstate->batblast >= BBTIME))
         {
         pstate->batblast = 0;
         NewState(ob,&s_pbatblast);
         }
      else if (pstate->weapon == wp_dog)
         NewState(ob,&s_serialdogattack);
 #endif
      else
         NewState(ob,&s_pmissattack1);

 #if (SHAREWARE == 0)

      if ((pstate->weapon == wp_dog) && (!ob->momentumz))
         ob->momentumz = -0x50000;
 #endif
      pstate->attackcount = WEAPONS[pstate->weapon].attackinfo[0].mtics;
      pstate->weaponframe = WEAPONS[pstate->weapon].attackinfo[0].frame;
      }

   else if (pstate->NETCAPTURED == 1)
      {
      NewState(player,&s_free);
      pstate->attackcount = FREE.attackinfo[0].mtics;
      pstate->weaponframe = FREE.attackinfo[0].frame;
      }

   }

void PlayNoWaySound ( void )
   {
   if (player->flags & FL_DOGMODE)
      SD_Play(SD_DOGMODEBITE2SND);
   else if ((locplayerstate->player == 1) || (locplayerstate->player == 3))
      SD_Play(SD_PLAYERTBHURTSND);
   else
      SD_Play(SD_NOWAYSND);
   }


/*
===============
=
= Cmd_Use
=
===============
*/

boolean AreJumping = false;//bna added
int		oldzval;
int donttilt=0;



void Cmd_Use (objtype*ob)
{
   int             checkx,checky,doorn,
                  /*newtilex,newtiley,oldtilex,oldtiley,*/elevnum,
                  wallx,wally;
//      statobj_t*      tempsprite=NULL;
   objtype*        tempactor= NULL;
   doorobj_t*      tempdoor=NULL;
   pwallobj_t*      temppwall=NULL;
   wall_t*         tempwall=NULL;
   int             index;
   playertype * pstate;



   M_LINKSTATE(ob,pstate);

#if (SHAREWARE == 0)

   if ((pstate->weapon == wp_dog) && (ob->state != &s_doguse) &&
       (ob->state != &s_dogwait) && (!W_CHANGE(pstate))
      )
      {
      pstate->attackframe = 0;
      NewState(ob,&s_doguse);
      pstate->attackcount = DOGSCRATCH.attackinfo[0].mtics;
      pstate->weaponframe = DOGSCRATCH.attackinfo[0].frame;
      ob->momentumz = -0x40000;
      return;

      }

   else
#endif
     if ((ob->flags & FL_DESIGNATED) && (BATTLEMODE) && (gamestate.battlemode == battle_Tag))
        {
        NewState(ob,&s_tag);
      //return;
        }
//
// find which cardinal direction the player is facing
//
   if (ob->angle < FINEANGLES/8 || ob->angle > 7*FINEANGLES/8)
      {
            checkx = ob->tilex + 1;
            checky = ob->tiley;
            ob->dir = east;
            wallx = (checkx << TILESHIFT);
            wally = (checky << TILESHIFT) + TILEGLOBAL/2;
      }
   else if (ob->angle < 3*FINEANGLES/8)
      {
            checkx = ob->tilex;
            checky = ob->tiley-1;
            ob->dir = north;
            wally = (checky << TILESHIFT) + TILEGLOBAL;
            wallx = (checkx << TILESHIFT) + TILEGLOBAL/2;
      }
   else if (ob->angle < 5*FINEANGLES/8)
      {
            checkx = ob->tilex - 1;
            checky = ob->tiley;
            ob->dir = west;
            wallx = (checkx << TILESHIFT) + TILEGLOBAL;
            wally = (checky << TILESHIFT) + TILEGLOBAL/2;
      }
   else
      {
            checkx = ob->tilex;
            checky = ob->tiley + 1;
            ob->dir = south;
            wally = (checky << TILESHIFT);
            wallx = (checkx << TILESHIFT) + TILEGLOBAL/2;
      }


   if (actorat[checkx][checky])
      {
      tempdoor=(doorobj_t*)actorat[checkx][checky];
      tempactor = (objtype*)actorat[checkx][checky];
      tempwall = (wall_t*)actorat[checkx][checky];
      }
   doorn = tilemap[checkx][checky] & ~0x2000;
//      if (sprites[checkx][checky])
//       tempsprite = sprites[checkx][checky];
   if (doorn == (elevatorstart + 6))
      return;

   //bna ++ jumpmode
   //SetTextMode (  );
   if (!BATTLEMODE){//dont use jump in battle, spoils sync
   if (usejump == true){
   if (pstate->buttonheld[bt_use]){
   if ((AreJumping == false)&&(ob->z > 0)&&(doorn==0)){
	    oldzval = ob->z;
		ob->z -= 15;
		ob->momentumz += GRAVITY;
		AreJumping = true;
		donttilt=10;
		return;
   } 
		AreJumping = false;
		return;
   }
   }
   }
   //bna 


   if (pstate->buttonheld[bt_use])
      return;

   if (doorn == (elevatorstart + 1))
      {
      tilemap[checkx][checky]++;              // flip switch
      if (MAPSPOT(ob->tilex,ob->tiley,1) == ALTELEVATORTILE);
         // playstate = ex_secretlevel;
      else if (ob==player)
         playstate = ex_completed;
      }


   else if (doorn == (elevatorstart + 5))

      {
      elevnum = MAPSPOT(ob->tilex,ob->tiley,1) - 90;
      tempwall->flags |= FL_S_FLIPPED;
      OperateElevatorSwitch(ob,elevnum,checkx,checky);
      }
   else if (tempdoor && tempdoor->which==PWALL)
      {
      temppwall=(pwallobj_t *)tempdoor;
      OperatePushWall (temppwall->num,ob->dir, ob == player );
      }
   else if ((doorn&0x8000) && (!(doorn&0x4000)))
      {
      doorobj_t* dptr = doorobjlist[doorn&0x3ff];
      int dnum = doorn&0x3ff;
      int lock;

      OperateDoor (pstate->keys, dnum, (ob == player));
      if (dptr->eindex != -1)
         {
         elevator_t*eptr;


         lock = dptr->lock;
         if ( lock && !( pstate->keys & ( 1 << ( lock - 1 ) ) ) )
            {
            if (ob==player)
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

         eptr = &ELEVATOR[dptr->eindex];
         if (((dnum == eptr->door1) && (eptr->state == ev_rad)) ||
             ((dnum == eptr->door2) && (eptr->state == ev_ras))
            )
            if (ob == player)
               AddMessage("Elevator is on the way.",MSG_GAME);

         OperateElevatorDoor(dnum);
         }

      }

   else if ((tempactor) && (tempactor->which == ACTOR) &&
            (tempactor->obclass == pillarobj) &&
            DISTOK(ob->x,tempactor->x,TD) &&
            DISTOK(ob->y,tempactor->y,TD) &&
            (!(tempactor->flags & FL_DONE)) &&
            (!MAPSPOT(tempactor->tilex,tempactor->tiley,2))
           )

      {if ((tempactor->dir == nodir) ||
           (tempactor->dir == ob->dir))
            {if (tempactor->dir == nodir)
                {
                tempactor->dir = ob->dir;
                ParseMomentum(tempactor,dirangle8[tempactor->dir]);
                }
             SD_PlaySoundRTP ( SD_PUSHWALLSND, tempactor->x, tempactor->y );
             tempactor->flags |= FL_ACTIVE;
             tempactor->flags |= FL_FLIPPED;
//               MakeActive(tempactor);
             tempactor->whatever = ob;
             gamestate.secretcount++;
            }

      }
   else if ((tempwall) && (tempwall->which == WALL) &&
            (tempwall->flags & FL_SWITCH) )
      {
      tempwall->flags |= FL_S_FLIPPED;
      if ((tempwall->flags & FL_W_INVERTED) &&
            DISTOK(ob->x,wallx,TD) &&
            DISTOK(ob->y,wally,TD) &&
            DISTOK(ob->z,0,32)
         )
         {
         index = touchindices[checkx][checky]-1;
         if (!(tempwall->flags & FL_ON))
            {
            maskobjlist[tilemap[checkx][checky]&0x3ff]->toptexture++;
            tempwall->flags |= FL_ON;
            TRIGGER[index] = 1;
            SD_PlaySoundRTP(SD_TOUCHPLATESND,ob->x,ob->y);
            if (ob==player)
               AddMessage("Switch turned on.",MSG_GAME);
            }
         else if (tempwall->flags & FL_REVERSIBLE)
            {
            maskobjlist[tilemap[checkx][checky]&0x3ff]->toptexture--;
            tempwall->flags &= ~FL_ON;
            TRIGGER[index] = 1;
            SD_PlaySoundRTP(SD_TOUCHPLATESND,ob->x,ob->y);
            if (ob==player)
               AddMessage("Switch turned off.",MSG_GAME);
            }

         }
      else if (DISTOK(ob->x,wallx,TD) &&
               DISTOK(ob->y,wally,TD) &&
               !(tempwall->flags & FL_W_INVERTED)
              )
         {
         index = touchindices[checkx][checky]-1;
         if (!(tempwall->flags & FL_ON))
            {
            tilemap[checkx][checky]++;
            tempwall->flags |= FL_ON;
            TRIGGER[index] = 1;
            SD_PlaySoundRTP(SD_TOUCHPLATESND,ob->x,ob->y);
            if (ob==player)
               AddMessage("Switch turned on.",MSG_GAME);
            }
         else if (tempwall->flags & FL_REVERSIBLE)
            {
            tilemap[checkx][checky]--;
            tempwall->flags &= ~FL_ON;
            TRIGGER[index] = 1;
            SD_PlaySoundRTP(SD_TOUCHPLATESND,ob->x,ob->y);
            if (ob==player)
               AddMessage("Switch turned off.",MSG_GAME);
            }

         }
      }
   else if ((tempwall) && (tempwall->which == WALL) && (ob==player)){
      PlayNoWaySound();
   	  //bna ++ jumpmode
   //SetTextMode (  );
   if (!BATTLEMODE){//dint use jump in battle, spoils sync
   if (usejump == true){
   if (pstate->buttonheld[bt_use]){
   if ((AreJumping == false)&&(ob->z > 0)&&(doorn==0)){
	    oldzval = ob->z;
		ob->z -= 15;
		ob->momentumz += GRAVITY;
		AreJumping = true;
		donttilt=10;
		return;
   } 
		AreJumping = false;
		return;
   }
   }
   }
	  //bna 
	  }
//      else
//         SD_PlaySoundRTP (SD_NOWAYSND,ob->x,ob->y);
//   pstate->buttonheld[bt_use] = true;
}


/*
=============================================================================

						  USER CONTROL

=============================================================================
*/


//******************************************************************************
//
// PollKeyboardButtons
//
//******************************************************************************

void PollKeyboardButtons (void)
{
   int i;

   QueueLetterInput ();
   IN_UpdateKeyboard();

   for (i = 0; i < NUMBUTTONS; i++)
   {
      if (Keystate[buttonscan[i]])
      {
         buttonpoll[i] = true;
      }
   }
}

//******************************************************************************
//
// PollMouseButtons
//
//******************************************************************************
extern boolean usemouselook;
void PollMouseButtons (void)
   {
   int i;
   int buttons;
   int mask;
   int press;

   buttons = IN_GetMouseButtons();

   mask = 1;
   for( i = 0; i < 3; i++, mask <<= 1 )
      {
      press = buttons & mask;

      if ( press )
         {//SetTextMode (  );
//         if ( ( buttonmouse[ i ] != bt_nobutton ) &&
//            ( DoubleClickCount[ i ] != 2 ) )
         if ( buttonmouse[ i ] != bt_nobutton )
            {
            buttonpoll[ buttonmouse[ i ] ] = true;
			//bna added
			if ((i == 1)&&(usemouselook == true)){
			//if rightclick set horizon to 512 (normall)
				playertype * pstate;
				pstate=&PLAYERSTATE[consoleplayer];
				pstate->horizon = 512;
				// SetNormalHorizon(PLAYER[0]);
			}
			//bna added

            }
         }

      // Check double-click
      if ( buttonmouse[ i + 3 ] != bt_nobutton )
         {
         if ( press )
            {
            // Was the button pressed last tic?
            if ( !DoubleClickPressed[ i ] )
               {
               // Yes, take note of it
               DoubleClickPressed[ i ] = true;

               // Is this the first click, or a really late click?
               if ( ( DoubleClickCount[ i ] == 0 ) ||
                  ( GetTicCount() >= DoubleClickTimer[ i ] ) )
                  {
                  // Yes, now wait for a second click
                  DoubleClickTimer[ i ] = GetTicCount() + DoubleClickSpeed;

                     //( tics << 5 );
                  DoubleClickCount[ i ] = 1;
                  }
               else
                  {
                  // Second click
                  buttonpoll[ buttonmouse[ i + 3 ] ] = true;
                  DoubleClickTimer[ i ] = 0;
                  DoubleClickCount[ i ] = 2;
                  }
               }
            else
               {
               // After second click, button remains pressed
               // until user releases it
               if ( DoubleClickCount[ i ] == 2 )
                  {
                  buttonpoll[ buttonmouse[ i + 3 ] ] = true;
                  }
               }
            }
         else
            {
            if ( DoubleClickCount[ i ] == 2 )
               {
               DoubleClickCount[ i ] = 0;
               }
            DoubleClickPressed[ i ] = false;
            }
         }
      }
   }


//******************************************************************************
//
// PollJoystickButtons
//
//******************************************************************************
void PollJoystickButtons
   (
   void
   )

   {
   int i;
   int buttons;
   int mask;
   int num;
   int press;

   buttons = IN_JoyButtons ();

   if ( joypadenabled )
      {
      num = 4;
      mask = 1;
      }
   else
      {
      num = 2;
      if ( joystickport )
         {
         mask = 4;
         }
      else
         {
         mask = 1;
         }
      }

   for( i = 0; i < num; i++, mask <<= 1 )
      {
      press = buttons & mask;

      if ( press )
         {
//         if ( ( buttonjoy[ i ] != bt_nobutton ) &&
//            ( JoyDblClickCount[ i ] != 2 ) )
         if ( buttonjoy[ i ] != bt_nobutton )
            {
            buttonpoll[ buttonjoy[ i ] ] = true;
            }
         }

      // Check double-click
      if ( buttonjoy[ i + 4 ] != bt_nobutton )
         {
         if ( press )
            {
            // Was the button pressed last tic?
            if ( !JoyDblClickPressed[ i ] )
               {
               // Yes, take note of it
               JoyDblClickPressed[ i ] = true;

               // Is this the first click, or a really late click?
               if ( ( JoyDblClickCount[ i ] == 0 ) ||
                  ( GetTicCount() >= JoyDblClickTimer[ i ] ) )
                  {
                  // Yes, now wait for a second click
                  JoyDblClickTimer[ i ] = GetTicCount() + DoubleClickSpeed;

                     //( tics << 5 );
                  JoyDblClickCount[ i ] = 1;
                  }
               else
                  {
                  // Second click
                  buttonpoll[ buttonjoy[ i + 4 ] ] = true;
                  JoyDblClickTimer[ i ] = 0;
                  JoyDblClickCount[ i ] = 2;
                  }
               }
            else
               {
               // After second click, button remains pressed
               // until user releases it
               if ( JoyDblClickCount[ i ] == 2 )
                  {
                  buttonpoll[ buttonjoy[ i + 4 ] ] = true;
                  }
               }
            }
         else
            {
            if ( JoyDblClickCount[ i ] == 2 )
               {
               JoyDblClickCount[ i ] = 0;
               }
            JoyDblClickPressed[ i ] = false;
            }
         }
      }
   }


//===========================================================================

//******************************************************************************
//
// PollKeyboardMove
//
//******************************************************************************

void PollKeyboardMove
   (
   void
   )

   {
   if ( ( buttonpoll[ bt_turnaround ] ) && ( turnaround == 0 ) )
      {
      turnaround = 1;
      turnaroundtime = 15 + tics;
      turnheldtime = 0;
      }
   if ( turnaround == 0 )
      {
      if ( buttonpoll[ di_east ] )
         {
         turnheldtime+=tics;
         if (turnheldtime>=TURBOTURNTIME)
            {
            KX = -KEYBOARDNORMALTURNAMOUNT;
            }
         else
            {
            KX = -KEYBOARDPREAMBLETURNAMOUNT;
            }
         }
      else if ( buttonpoll[ di_west ] )
         {
         turnheldtime+=tics;
         if (turnheldtime>=TURBOTURNTIME)
            {
            KX = KEYBOARDNORMALTURNAMOUNT;
            }
         else
            {
            KX = KEYBOARDPREAMBLETURNAMOUNT;
            }
         }
      else
         {
		   KX = 0;
         turnheldtime=0;
         }
      if ( (buttonpoll[bt_run]) &&
           ( (turnheldtime>=TURBOTURNTIME) || (turnheldtime==0) )
         )
         KX = FixedMul(KX,TURBOTURNAMOUNT);
      }
   else
      {
      KX=TURNAROUNDSPEED;
      turnaroundtime-=tics;
      if (turnaroundtime<=0)
         {
         turnaround=0;
         KX=((turnaroundtime*TURNAROUNDSPEED)>>1);
         }
      }

   if ( buttonpoll[ di_north ] )
      {
      KY = -BASEMOVE;
      }
   else if ( buttonpoll[ di_south ] )
      {
      KY = BASEMOVE;
      }
   else
      KY = 0;

   if (buttonpoll[bt_run])
      {
      KY <<= 1;
      }
}

//******************************************************************************
//
// PollMouseMove
//
//******************************************************************************

//#define MOUSE_RY_SHIFT 12
//#define MOUSE_TZ_SHIFT 3
#define MOUSE_TZ_SENSITIVITY_SCALE 65535
#define MOUSE_RY_SENSITIVITY_SCALE 18725*2
//#define MOUSE_RY_INPUT_SCALE 6000
#define MOUSE_TZ_INPUT_SCALE 20
int mouse_ry_input_scale = 5000;

int sensitivity_scalar[15] =
    {
    0,1,2,3,4,5,6,8,11,13,15,18,12,13,14
    };
//#define MOUSE_RY_SCALE 65535
//#define MOUSE_TZ_SCALE 65535
#define MAXMOUSETURN 7000000

/* use SDL mouse */
#define USESDLMOUSE 1


extern int inverse_mouse;
 double Y_MouseSpeed=70;

void PollMouseMove (void)
{
   int  mousexmove, mouseymove;
   double Ys;
//SetTextMode();

   Ys=(Y_MouseSpeed/100);
//

// const long inverse_mouse  = 1; //set  to -1 to invert mouse
// inverse_mouse def moved to RT_CFG.C

#ifdef USESDLMOUSE
   INL_GetMouseDelta(&mousexmove, &mouseymove);
#else
   PollMouse();//Uses DirectInput mouse in DInput.cpp
   mousexmove=MX;
   mouseymove=MY;
#endif

   if (abs(mousexmove)>abs(mouseymove))
      mouseymove/=2;
   else
      mousexmove/=2;
   MX = 0;
   MY = 0;


   if ((abs (mouseymove)) >= threshold)
      {//
      MY =  MOUSE_TZ_INPUT_SCALE*mouseymove;
	  MY *= inverse_mouse;
	  if (usemouselook == true){
		  if (MY > 0){
				playertype * pstate;
				pstate=&PLAYERSTATE[consoleplayer];
				//if (pstate->horizon > 512){
					pstate->horizon -= Ys * (2*sensitivity_scalar[mouseadjustment]);
				//}
		  }
		  else if (MY < 0){
		  		playertype * pstate;
				pstate=&PLAYERSTATE[consoleplayer];
				//SetTextMode (  );
				pstate->horizon += Ys * (2*sensitivity_scalar[mouseadjustment]);
				//buttonpoll[ bt_horizonup ] = true;
		  }
		  MY = 0;
	  }else{
		 // MY += FixedMul(MY,mouseadjustment*MOUSE_TZ_SENSITIVITY_SCALE);
		  if (abs(mouseymove)>200)
			 {
			 buttonpoll[bt_run]=true;
			 // buttonpoll[ bt_lookup ] = true;
			 }
		  }
	  }



		 

   if ((abs (mousexmove)) >= threshold)
   {
      //MX = -MOUSE_RY_INPUT_SCALE*mousexmove;
      MX = -mouse_ry_input_scale*mousexmove;
      MX += FixedMul(MX,sensitivity_scalar[mouseadjustment]*MOUSE_RY_SENSITIVITY_SCALE);
   //   if (abs(MX) > MAXMOUSETURN)
     //   MX = MAXMOUSETURN*SGN(MX);
	  if (usemouselook == true){
		  if (abs(mouseymove)>10)
		  {
			 buttonpoll[bt_run]=true;
			  //buttonpoll[ bt_lookdown ] = true;
		  }
	  }
   }
//   if (MY > 0)
//      MX -= (MX/2);

//   MX=0;
//   MY=0;

}


//******************************************************************************
//
// PollJoystickMove
//
//******************************************************************************

void PollJoystickMove (void)
{
   int   joyx,joyy;

   INL_GetJoyDelta (joystickport, &joyx, &joyy);
   if ( joypadenabled )
      {
      if (joyx >= threshold)
         {
         buttonpoll[ di_east ] = true;
         }
      if (-joyx >= threshold)
         {
         buttonpoll[ di_west ] = true;
         }
      if ( joyy >= threshold )
         {
         buttonpoll[ di_south ] = true;
         }
      if ( -joyy >= threshold )
         {
         buttonpoll[ di_north ] = true;
         }
      }
   else
      {
      if ((abs (joyx)) >= threshold)
         {
         JX = ((-joyx)<<13)+((-joyx)<<11);
         turnheldtime += tics;
         }
      else
         JX = 0;

      if ((abs (joyy)) >= threshold)
         {
         JY = joyy<<4;
         }
      else
         JY = 0;
      if (buttonpoll[bt_run])
         {
         JX <<= 1;
         JY <<= 1;
         }
      }
   }

//******************************************************************************
//
// StartVRFeedback
//
//******************************************************************************

void StartVRFeedback (int guntype)
{
#ifdef DOS
   union REGS inregs;
   union REGS outregs;

   inregs.x.eax = VR_FEEDBACK_SERVICE;
   inregs.x.ebx = 1;
   inregs.x.ecx = guntype;
   int386 (0x33, &inregs, &outregs);
#else
	STUB_FUNCTION;
#endif
}

//******************************************************************************
//
// StopVRFeedback
//
//******************************************************************************

void StopVRFeedback (void)
{
#ifdef DOS
   union REGS inregs;
   union REGS outregs;

   inregs.x.eax = VR_FEEDBACK_SERVICE;
   inregs.x.ebx = 0;
   int386 (0x33, &inregs, &outregs);
#else
	STUB_FUNCTION;
#endif
}

//******************************************************************************
//
// PollVirtualReality
//
//******************************************************************************

#define VR_BUTTON(x) ((vr_buttons>>x) & 1)

void PollVirtualReality (void)
{
#ifdef DOS
   union REGS inregs;
   union REGS outregs;
   short int  mousexmove,
              mouseymove;
   word vr_buttons;

   inregs.x.eax = VR_INPUT_SERVICE;

   inregs.x.ebx = player->angle;
   inregs.x.ecx = player->yzangle;

   int386 (0x33, &inregs, &outregs);

   vr_buttons = outregs.w.bx;

   buttonpoll[bt_run          ] |= VR_BUTTON(VR_RUNBUTTON          );
   buttonpoll[bt_strafeleft   ] |= VR_BUTTON(VR_STRAFELEFTBUTTON   );
   buttonpoll[bt_straferight  ] |= VR_BUTTON(VR_STRAFERIGHTBUTTON  );
   buttonpoll[bt_attack       ] |= VR_BUTTON(VR_ATTACKBUTTON       );
   buttonpoll[bt_lookup       ] |= VR_BUTTON(VR_LOOKUPBUTTON       );
   buttonpoll[bt_lookdown     ] |= VR_BUTTON(VR_LOOKDOWNBUTTON     );
   buttonpoll[bt_swapweapon   ] |= VR_BUTTON(VR_SWAPWEAPONBUTTON   );
   buttonpoll[bt_use          ] |= VR_BUTTON(VR_USEBUTTON          );
   buttonpoll[bt_horizonup    ] |= VR_BUTTON(VR_HORIZONUPBUTTON    );
   buttonpoll[bt_horizondown  ] |= VR_BUTTON(VR_HORIZONDOWNBUTTON  );
   buttonpoll[bt_map          ] |= VR_BUTTON(VR_MAPBUTTON          );
   buttonpoll[bt_pistol       ] |= VR_BUTTON(VR_PISTOLBUTTON       );
   buttonpoll[bt_dualpistol   ] |= VR_BUTTON(VR_DUALPISTOLBUTTON   );
   buttonpoll[bt_mp40         ] |= VR_BUTTON(VR_MP40BUTTON         );
   buttonpoll[bt_missileweapon] |= VR_BUTTON(VR_MISSILEWEAPONBUTTON);
   buttonpoll[bt_recordsound  ] |= VR_BUTTON(VR_RECORDBUTTON       );

   mousexmove = outregs.w.cx;
	mouseymove = outregs.w.dx;

   VX = 0;
   VY = 0;


   if ((abs (mouseymove)) >= threshold)
      {
      VY =  MOUSE_TZ_INPUT_SCALE*mouseymove;
      if (abs(mouseymove)>200)
         {
         buttonpoll[bt_run]=true;
         }
      }

   if ((abs (mousexmove)) >= threshold)
      {
      VX = -mouse_ry_input_scale*mousexmove;
      VX += FixedMul(MX,sensitivity_scalar[mouseadjustment]*MOUSE_RY_SENSITIVITY_SCALE);
      if (abs(mousexmove)>10)
         {
         buttonpoll[bt_run]=true;
         }
      }
#else
	STUB_FUNCTION;
#endif      
}


//******************************************************************************
//
// PollMove ()
//
//******************************************************************************

boolean aimbuttonpressed=false;
void PollMove (void)
{
	int angle;
   int x, y;


   x = KX + MX + JX + CX + VX;
   y = KY + MY + JY + CY + VY;

   if (buttonpoll[bt_aimbutton])
      {
      if (y>0)
         {
         buttonpoll[bt_horizonup]=1;
         y=0;
         aimbuttonpressed=true;
         }
      else if (y<0)
         {
         buttonpoll[bt_horizondown]=1;
         y=0;
         aimbuttonpressed=true;
         }
      else if (aimbuttonpressed==false)
         {
         buttonpoll[bt_lookup]=1;
         buttonpoll[bt_lookdown]=1;
         }
      }
   else
      {
      aimbuttonpressed=false;
      }

   if (player->flags & FL_FLEET)
		y += y>>1;

   if ((locplayerstate->NETCAPTURED == 1) && (!locplayerstate->HASKNIFE))
   {
      if (first)
      {
         nettics = GetTicCount() + (VBLCOUNTER * 4);
         first = 0;
      }

      if (x > 0)
         {
         rightmom += NETMOM;
         if (lastmom!=0)
            controlbuf[2]=x<<1;
         lastmom=0;
         }
      else
         if (x < 0)
            {
            leftmom += NETMOM;
            if (lastmom!=1)
               controlbuf[2]=x<<1;
            lastmom=1;
            }
         else
         {
            rightmom -= (NETMOM >> 2);
            if (rightmom < 0)
               rightmom = 0;
            leftmom  -= (NETMOM >> 2);
            if (leftmom < 0)
               leftmom = 0;
         }

      if ((GetTicCount() > nettics) && (rightmom > (NETMOM * 2)) &&
                                  (leftmom > (NETMOM * 2)))
      {
         rightmom = 0;
         leftmom  = 0;
         first    = 1;
         lastmom^=1;
			locplayerstate->NETCAPTURED = 0;
			MISCVARS->NET_IN_FLIGHT = false;
         NewState(player, &s_player);
			locplayerstate->weaponuptics = WEAPONS[locplayerstate->weapon].screenheight/GMOVE;
			locplayerstate->weaponheight = locplayerstate->weaponuptics*GMOVE ;
      }
   }
   else if ((buttonpoll[bt_strafe]) && (turnaround==0))
   {
      // strafing
      if (x < 0)
         {
			angle = (player->angle - FINEANGLES/4)&(FINEANGLES-1);

         x = (x>>10) + (x >> 11);

         controlbuf[0] = -(FixedMul (x, costable[angle]));
         controlbuf[1] = FixedMul (x, sintable[angle]);
			}
      else if (x > 0)
			{
         angle = (player->angle + FINEANGLES/4)&(FINEANGLES-1);

         x = (x>>10) + (x >> 11);

			controlbuf[0] = FixedMul (x, costable[angle]);
         controlbuf[1] = -(FixedMul (x, sintable[angle]));
         }
      if (y != 0)
         {
         controlbuf[0] += -(FixedMul (y, viewcos));
         controlbuf[1] += (FixedMul (y, viewsin));
         }
   }
   else
   {
      if (y != 0)
         {
         controlbuf[0] = -FixedMul (y, viewcos);
         controlbuf[1] = FixedMul (y, viewsin);
         }

      if (x != 0)
         controlbuf[2] = x;
   }

   if (buttonpoll[bt_strafeleft])
      {
      angle = (player->angle - FINEANGLES/4)&(FINEANGLES-1);
      controlbuf[0] += -(FixedMul (STRAFEAMOUNT, costable[angle]));
      controlbuf[1] +=   FixedMul (STRAFEAMOUNT, sintable[angle]);
		}
   else if (buttonpoll[bt_straferight])
      {
      angle = (player->angle + FINEANGLES/4)&(FINEANGLES-1);
      controlbuf[0] += -(FixedMul (STRAFEAMOUNT, costable[angle]));
      controlbuf[1] +=   FixedMul (STRAFEAMOUNT, sintable[angle]);
      }
}


//******************************************************************************
//
// PollCyberman ()
//
//******************************************************************************

void PollCyberman (void)
{
   int i;
   int mask;
   int press;

   SWIFT_Get3DStatus (&SWIFTStatus);

   mask = 4;
   for( i = 0; i < 3; i++, mask >>= 1 )
      {
      press = SWIFTStatus.buttons & mask;

      if ( press )
         {
//         if ( ( buttonmouse[ i ] != bt_nobutton ) &&
//            ( DoubleClickCount[ i ] != 2 ) )
         if ( buttonmouse[ i ] != bt_nobutton )
            {
            buttonpoll[ buttonmouse[ i ] ] = true;
            }
         }

      // Check double-click
      if ( buttonmouse[ i + 3 ] != bt_nobutton )
         {
         if ( press )
            {
            // Was the button pressed last tic?
            if ( !DoubleClickPressed[ i ] )
               {
               // Yes, take note of it
               DoubleClickPressed[ i ] = true;

               // Is this the first click, or a really late click?
               if ( ( DoubleClickCount[ i ] == 0 ) ||
                  ( GetTicCount() >= DoubleClickTimer[ i ] ) )
                  {
                  // Yes, now wait for a second click
                  DoubleClickTimer[ i ] = GetTicCount() + DoubleClickSpeed;

                     //( tics << 5 );
                  DoubleClickCount[ i ] = 1;
                  }
               else
                  {
                  // Second click
                  buttonpoll[ buttonmouse[ i + 3 ] ] = true;
                  DoubleClickTimer[ i ] = 0;
                  DoubleClickCount[ i ] = 2;
                  }
               }
            else
               {
               // After second click, button remains pressed
               // until user releases it
               if ( DoubleClickCount[ i ] == 2 )
                  {
                  buttonpoll[ buttonmouse[ i + 3 ] ] = true;
                  }
               }
            }
         else
            {
            if ( DoubleClickCount[ i ] == 2 )
               {
               DoubleClickCount[ i ] = 0;
               }
            DoubleClickPressed[ i ] = false;
            }
         }
      }

   if (SWIFTStatus.pitch > 0)
     CYBERLOOKUP = true;
   else if (SWIFTStatus.pitch < 0)
     CYBERLOOKDOWN = true;

   if ((abs (SWIFTStatus.x)) > CYBERDEADRANGE)
   {
      CX = -(SGN (SWIFTStatus.x) * (( (abs(SWIFTStatus.x)-CYBERDEADRANGE) ) << 10));
      turnheldtime += tics;
   }
   else
      if (SWIFTStatus.x != oldcyberx)
		{
         turnheldtime += tics;
         if (SWIFTStatus.x > oldcyberx)
				CX = -(0xB8000);
         else
				CX = 0xB8000;

         oldcyberx = SWIFTStatus.x;
      }
      else
         CX = 0;

   if ((abs (SWIFTStatus.y)) > CYBERDEADRANGE)
   {
      CY = SWIFTStatus.y >> 2;
   }
	else
      CY = 0;
}

//******************************************************************************
//
// PollAssassin ()
//
//******************************************************************************

#define MAXRAMPS 5
typedef struct
   {
   int  min;
   int  factor;
   } RampType;
void PollAssassin (void)
{
   int i;
   int mask;
   int press;
   int yaw;
   int strafeAngle;
   int acc;
   int numramps=4;
   RampType ramp[MAXRAMPS]={
                              {0,280000},
                              {4,380000},
                              {10,480000},
                              {25,680000},
//                              {25,( (1<<26)/80  )}
                            };

   SWIFT_Get3DStatus (&SWIFTStatus);

   mask = 4;
   for( i = 0; i < 3; i++, mask >>= 1 )
      {
      press = SWIFTStatus.buttons & mask;

      if ( press )
         {
//         if ( ( buttonmouse[ i ] != bt_nobutton ) &&
//            ( DoubleClickCount[ i ] != 2 ) )
         if ( buttonmouse[ i ] != bt_nobutton )
            {
            buttonpoll[ buttonmouse[ i ] ] = true;
            }
         }

      // Check double-click
      if ( buttonmouse[ i + 3 ] != bt_nobutton )
         {
         if ( press )
            {
            // Was the button pressed last tic?
            if ( !DoubleClickPressed[ i ] )
               {
               // Yes, take note of it
               DoubleClickPressed[ i ] = true;

               // Is this the first click, or a really late click?
               if ( ( DoubleClickCount[ i ] == 0 ) ||
                  ( GetTicCount() >= DoubleClickTimer[ i ] ) )
                  {
                  // Yes, now wait for a second click
                  DoubleClickTimer[ i ] = GetTicCount() + DoubleClickSpeed;

                     //( tics << 5 );
                  DoubleClickCount[ i ] = 1;
                  }
               else
                  {
                  // Second click
                  buttonpoll[ buttonmouse[ i + 3 ] ] = true;
                  DoubleClickTimer[ i ] = 0;
                  DoubleClickCount[ i ] = 2;
                  }
               }
            else
               {
               // After second click, button remains pressed
               // until user releases it
               if ( DoubleClickCount[ i ] == 2 )
                  {
                  buttonpoll[ buttonmouse[ i + 3 ] ] = true;
                  }
               }
            }
         else
            {
            if ( DoubleClickCount[ i ] == 2 )
               {
               DoubleClickCount[ i ] = 0;
               }
            DoubleClickPressed[ i ] = false;
            }
         }
      }

   buttonpoll[bt_horizonup] |=   ((SWIFTStatus.buttons>>7) & 1);
   buttonpoll[bt_horizondown] |= ((SWIFTStatus.buttons>>8) & 1);

   if ( abs(SWIFTStatus.pitch) < (20<<6) )
      {
      SWIFTStatus.pitch = 0;
      }
   else
      {
      SWIFTStatus.pitch -= SGN(SWIFTStatus.pitch)*(20<<6);
      }

   if ( abs(SWIFTStatus.pitch) > (60<<6) )
      {
      buttonpoll[bt_run] = 1;
      }

   if ( abs(SWIFTStatus.roll) > (80<<6) )
      {
      buttonpoll[bt_run] = 1;
      }


   if ( abs(SWIFTStatus.roll) < (20<<6) )
      {
      SWIFTStatus.roll = 0;
      }
   else
      {
      SWIFTStatus.roll -= SGN(SWIFTStatus.roll)*(20<<6);
      }

   strafeAngle = (player->angle - FINEANGLES/4)&(FINEANGLES-1);

   controlbuf[0] += -(FixedMulShift (SWIFTStatus.pitch, viewcos,16))+
                   FixedMulShift (-SWIFTStatus.roll, costable[strafeAngle], 16);

   controlbuf[1] +=  FixedMulShift (SWIFTStatus.pitch, viewsin,16) -
                   FixedMulShift (-SWIFTStatus.roll, sintable[strafeAngle], 16);

   yaw = abs(SWIFTStatus.yaw);
   acc = 0;
   for (i=0;i<numramps;i++)
      {
      if (yaw > ramp[i].min)
         {
         if (i>0)
            {
            acc += ramp[i].min*(ramp[i].factor-ramp[i-1].factor);
            }
         }
      else
         {
         i++;
         break;
         }
      }
   controlbuf[2]= SWIFTStatus.yaw * ramp[i-1].factor - acc;
}


//******************************************************************************
//
// PollControls
//
// Gets user or demo input, call once each frame
//
// controlx     set between -100 and 100 per tic
// controly
//
//******************************************************************************


void PollControls (void)
{
   int   i;

   if (standalone==true)
      return;

   lastpolltime=controlupdatetime;

   memset (buttonpoll, 0, sizeof(buttonpoll));

   controlbuf[0] = controlbuf[1] = controlbuf[2] = 0;
   CYBERLOOKUP = CYBERLOOKDOWN = false;

   if (gamestate.autorun==1)
	   buttonpoll[bt_run] = true;


//
// get button states
//
   PollKeyboardButtons ();

   if (mouseenabled && !cybermanenabled)
      PollMouseButtons ();

	if (joystickenabled)
      PollJoystickButtons ();


//
// get movements
//
   if (joystickenabled)
      PollJoystickMove ();

	if (cybermanenabled)
      PollCyberman ();

   else if (mouseenabled && MousePresent)
      PollMouseMove ();

   PollKeyboardMove ();

   if (vrenabled)
      PollVirtualReality ();

   PollMove ();

   if (spaceballenabled)
      PollSpaceBall ();

   else if (assassinenabled)
       PollAssassin ();


	buttonbits = 0;
	if (player->flags & FL_DYING) // Player has died
      {
      if ((playerdead==true) &&
          ( buttonpoll[ bt_strafe ] ||
            buttonpoll[ bt_attack ] ||
            buttonpoll[ bt_use ] ||
            ((gamestate.battlemode == battle_Hunter) &&
             (BATTLE_Team[player->dirchoosetime] == BATTLE_It)
            )
          )
         )
         {
         AddRespawnCommand();
         }
      memset (buttonpoll, 0, sizeof(buttonpoll));
      controlbuf[0] = controlbuf[1] = controlbuf[2] = 0;
      }

   if ((PausePressed==true) && (modemgame==false))
      {
      PausePressed=false;
      if (GamePaused==true)
         AddPauseStateCommand(COM_UNPAUSE);
      else
         {
         AddPauseStateCommand(COM_PAUSE);
         }
      }
   if (Keyboard[sc_Insert] && Keyboard[sc_X]){
	      AddExitCommand();
   }
//bna section
   if (Keyboard[sc_5]){
	//	 SetTextMode (  );
      weaponscale +=  1000;
	  //testval++;
   }
   if (Keyboard[sc_6]){
	//	 SetTextMode (  );
      weaponscale -=  1000;
	  	//  testval--;
   }
//bna section end 





   for (i = (NUMTXBUTTONS-1); i >= 0; i--)
      {
      buttonbits <<= 1;
		if (buttonpoll[i])
         buttonbits |= 1;
		}

   UpdateClientControls();
}


void ResetWeapons(objtype *ob)
{playertype *pstate;

 M_LINKSTATE(ob,pstate);

 pstate->weapondowntics = WEAPONS[pstate->weapon].screenheight/GMOVE;
 pstate->new_weapon = pstate->oldweapon;
 pstate->missileweapon = pstate->oldmissileweapon;
 ob->shapeoffset = pstate->oldshapeoffset;
 pstate->attackframe = pstate->weaponframe = 0;

 if ((ob==player) && SHOW_BOTTOM_STATUS_BAR() )
	DrawBarAmmo (false);
}




void SaveWeapons(objtype*ob)
{playertype *pstate;

 if ((ob->flags&FL_DOGMODE) || (ob->flags&FL_GODMODE))
    return;

 //pstate = (ob==player)?(&playerstate):(&remoteplayerstate);
 M_LINKSTATE(ob,pstate);

 pstate->weapondowntics = WEAPONS[pstate->weapon].screenheight/GMOVE;
 pstate->oldweapon = pstate->new_weapon;
 pstate->oldmissileweapon = pstate->missileweapon;
 pstate->oldshapeoffset = ob->shapeoffset;

}

/*
void SaveWeapons(objtype*ob)
{playertype *pstate;

 if ((ob->flags&FL_DOGMODE) || (ob->flags&FL_GODMODE))
    return;

 //pstate = (ob==player)?(&playerstate):(&remoteplayerstate);
 M_LINKSTATE(ob,pstate);

 pstate->weapondowntics = WEAPONS[pstate->weapon].screenheight/GMOVE;
 pstate->oldweapon = pstate->weapon;
 pstate->oldmissileweapon = pstate->missileweapon;
 pstate->oldshapeoffset = ob->shapeoffset;

}
*/


#define GiveProtection(flag,time,sound)                  \
   {                                                     \
   if ((ob->flags & flag) || (ob->flags & FL_GODMODE) || \
       (ob->flags & FL_DOGMODE))                         \
      return;                                            \
   ob->flags &= ~(FL_BPV|FL_GASMASK|FL_AV);              \
   ob->flags |= flag;                                    \
   SD_PlaySoundRTP(sound,ob->x, ob->y);                  \
   pstate->protectiontime = time;                        \
   gamestate.supercount ++;                              \
   }




boolean GivePowerup(objtype *ob,int flag,int time,int sound)
   {
   playertype *pstate;


   if ((ob->flags & flag) ||
       (ob->flags & FL_GODMODE) ||
       (ob->flags & FL_DOGMODE)
      )
      return false;


   M_LINKSTATE(ob,pstate);

   /*
   if (ob->flags & FL_DOGMODE)
      {
      ob->temp2 = DOGMODERISE;
      ResetWeapons(ob);
      if (ob->state->condition & SF_DOGSTATE)
         NewState(ob,&s_player);
      }
   else if (ob->flags & FL_GODMODE)
      {
      ob->temp2 = GODMODEFALL;
      ResetWeapons(ob);
      }
   */
   ob->flags &= ~(FL_SHROOMS|FL_FLEET|FL_ELASTO|FL_GODMODE|FL_DOGMODE);
   ob->flags |= flag;
   pstate->poweruptime = time;
   pstate->soundtime = 0;
   SD_PlaySoundRTP(sound,ob->x, ob->y);
   gamestate.supercount ++;
   return true;

   }



void GiveLifePoints(objtype *ob,int points)
   {
   SD_PlaySoundRTP(SD_GETBONUSSND,ob->x, ob->y);

   UpdateTriads (ob,points);
   if (ob==player)
      DrawTriads (false);

   }


boolean GiveBulletWeapon(objtype *ob,int bulletweapon,statobj_t*check)
   {
   playertype *pstate;

   M_LINKSTATE(ob,pstate);

   if ((ob->flags & FL_DOGMODE) || (ob->flags & FL_GODMODE))
      return false;

   if (!ARMED(ob->dirchoosetime))
      return false;

   if (pstate->HASBULLETWEAPON[bulletweapon])
      return false;

   GiveWeapon(ob,bulletweapon);
   if ( gamestate.BattleOptions.WeaponPersistence )
      {
      LASTSTAT->z = check->z;
      }
   SD_PlaySoundRTP(SD_GETWEAPONSND,ob->x, ob->y);
   return true;

   }


boolean GivePlayerMissileWeapon(objtype *ob, playertype *pstate,
                                statobj_t *check)
   {
   if  ((ob->flags & FL_DOGMODE) || (ob->flags & FL_GODMODE))
      return false;


   if (!ARMED(ob->dirchoosetime))
      return false;

   if ((GetWeaponForItem(check->itemnumber) == pstate->missileweapon) &&
       (check->ammo == stats[check->itemnumber].ammo) &&
       (pstate->ammo == stats[check->itemnumber].ammo)
      )
      return false;

   SD_PlaySoundRTP(SD_GETWEAPONSND,ob->x, ob->y);
   GiveMissileWeapon(ob,GetWeaponForItem(check->itemnumber));
   if (gamestate.BattleOptions.WeaponPersistence)
         LASTSTAT->z = check->z;

   gamestate.missilecount ++;

   if (BATTLEMODE && (gamestate.BattleOptions.Ammo != bo_normal_shots))
      {if (gamestate.BattleOptions.Ammo == bo_one_shot)
         pstate->ammo = 1;
      else
         pstate->ammo = -1;
      }
   else
      pstate->ammo = check->ammo;
   if ((ob==player) && SHOW_BOTTOM_STATUS_BAR() )
      DrawBarAmmo (false);

   return true;

   }


#define LocalBonusMessage(string)  \
   {                               \
   if (ob == player)               \
     AddMessage(string,MSG_BONUS); \
   }
#define LocalBonus1Message(string)  \
   {                               \
   if (ob == player)               \
     AddMessage(string,MSG_BONUS1); \
   }


/*
=================================
=
=   GetBonusTimeForItem
=
=================================
*/

int GetBonusTimeForItem(int itemnumber)
   {
   specials *which;

   if ( BATTLEMODE )
      {
      which = &gamestate.SpecialsTimes;
      }
   else
      {
      which = &CurrentSpecialsTimes;
      }

   switch(itemnumber)
      {
      case stat_godmode:
         return which->GodModeTime;

      case stat_dogmode:
         return which->DogModeTime;

      case stat_mushroom:
         return which->ShroomsModeTime;

      case stat_elastic:
         return which->ElastoModeTime;

      case stat_asbesto:
         return which->AsbestosVestTime;

      case stat_bulletproof:
         return which->BulletProofVestTime;

      case stat_gasmask:
         return which->GasMaskTime;

      case stat_fleetfeet:
         return which->MercuryModeTime;
      }

   return -1;

   }




/*
=================================
=
=   GetRespawnTimeForItem
=
=================================
*/



int GetRespawnTimeForItem(int itemnumber)
   {
   switch(itemnumber)
      {
      case stat_godmode:
         return gamestate.SpecialsTimes.GodModeRespawnTime;

      case stat_dogmode:
         return gamestate.SpecialsTimes.DogModeRespawnTime;

      case stat_mushroom:
         return gamestate.SpecialsTimes.ShroomsModeRespawnTime;

      case stat_elastic:
         return gamestate.SpecialsTimes.ElastoModeRespawnTime;

      case stat_asbesto:
         return gamestate.SpecialsTimes.AsbestosVestRespawnTime;

      case stat_bulletproof:
         return gamestate.SpecialsTimes.BulletProofVestRespawnTime;

      case stat_gasmask:
         return gamestate.SpecialsTimes.GasMaskRespawnTime;

      case stat_fleetfeet:
         return gamestate.SpecialsTimes.MercuryModeRespawnTime;

      }

   return gamestate.BattleOptions.RespawnTime * VBLCOUNTER;


   }


/*
===================
=
= GetBonus
=
===================
*/
void GetBonus (objtype*ob,statobj_t *check)
{
	int heal;
	playertype * pstate;
   boolean randompowerup;

	M_LINKSTATE(ob,pstate);

   randompowerup=false;

randomlabel:
	switch (check->itemnumber)
	{

	case    stat_knifestatue:
      SD_PlaySoundRTP(SD_GETKNIFESND,ob->x, ob->y);
      if (ob==player)
         locplayerstate->HASKNIFE = 1;
      SD_PlaySoundRTP(PlayerSnds[locplayerstate->player], ob->x, ob->y);
      LocalBonusMessage("You found a knife.");
      break;
   case    stat_pedgoldkey:
      LocalBonusMessage("You found the \\EGold key.");
      goto keys;
   case    stat_pedsilverkey:
      LocalBonusMessage("You got the \\FSilver key.");
      goto keys;
   case    stat_pedironkey:
      LocalBonusMessage("You got the \\8Iron key.");
      goto keys;
   case    stat_pedcrystalkey:
      LocalBonusMessage("You got the \\4Oscuro key.");
keys:
		GiveKey (check->itemnumber - stat_pedgoldkey);
		SD_PlaySoundRTP (SD_GETKEYSND,ob->x, ob->y);
		break;
	case    stat_monkmeal:
		if (pstate->health == MaxHitpointsForCharacter(pstate))
			return;
		SD_PlaySoundRTP (SD_GETHEALTH1SND,ob->x, ob->y);
      LocalBonusMessage("You ate some Monk Meal.");
		HealPlayer (10,ob);
		gamestate.healthcount ++;
		break;
	case    stat_monkcrystal1:
		if (pstate->health == MaxHitpointsForCharacter(pstate))
			return;
		SD_PlaySoundRTP (SD_GETHEALTH2SND,ob->x, ob->y);
      LocalBonusMessage("You picked up a small Monk Crystal.");

		HealPlayer (10,ob);
		gamestate.healthcount ++;
		break;
	case   stat_monkcrystal2:
		if (pstate->health == MaxHitpointsForCharacter(pstate))
			return;
      SD_PlaySoundRTP (SD_GETHEALTH2SND,ob->x, ob->y);
      LocalBonusMessage("You picked up a large Monk Crystal.");

		HealPlayer (50,ob);
		gamestate.healthcount ++;
		break;
	case    stat_priestporridge:
		if (pstate->health == MaxHitpointsForCharacter(pstate))
			return;
      SD_PlaySoundRTP (SD_GETHEALTH1SND,ob->x, ob->y);
		if (check->flags & FL_ACTIVE)
         {
         HealPlayer (50,ob);
         LocalBonusMessage("You ate some Priest Porridge Hot.");
         }
		else
         {
         HealPlayer (20,ob);

         LocalBonusMessage("You ate some Priest Porridge.");
         }
		gamestate.healthcount ++;
		break;

	case   stat_healingbasin:
		if (pstate->health == MaxHitpointsForCharacter(pstate))
			return;
      SD_PlaySoundRTP (SD_GETHEALTH2SND,ob->x, ob->y);
		heal = 25 + (GameRandomNumber("GetBonus",0) >> 2);
		HealPlayer (heal,ob);
      LocalBonusMessage("You drank from the healing basin.");

		gamestate.healthcount ++;
		gamestate.democraticcount ++;
		break;

	case stat_oneup:
		if (abs(pstate->health - MaxHitpointsForCharacter(pstate))
			 < (MaxHitpointsForCharacter(pstate)>>2) )
         {
         GiveLives(1);
         LocalBonusMessage("Extra Life!");
         }
		else
         {
         HealPlayer(MaxHitpointsForCharacter(pstate),ob);
         LocalBonusMessage("Full Health!");
         }
		SD_PlaySoundRTP(SD_GET1UPSND,ob->x, ob->y);
		break;
	case stat_threeup:
		if (abs(pstate->health - MaxHitpointsForCharacter(pstate))
			 < (MaxHitpointsForCharacter(pstate)>>2) )
			{
         GiveLives(3);
         LocalBonusMessage("Three Extra Lives!");

			}
		else
         {
         HealPlayer(MaxHitpointsForCharacter(pstate),ob);
         GiveLives(2);
         LocalBonusMessage("Full Health AND Two Extra Lives!");

         }
		SD_PlaySoundRTP(SD_GET3UPSND,ob->x, ob->y);
		break;

   case stat_scotthead:
      // Give Apogee's phone number as points
      GivePoints( 2764331 );
      LocalBonusMessage( "Whoa...Scott's Mystical Head!");
      LocalBonus1Message( "You get 2,764,331 points!");
      SD_PlaySoundRTP(SD_GETHEADSND,ob->x, ob->y);
		break;

	case stat_twopistol:
      if (GiveBulletWeapon(ob,wp_twopistol,check)==false)
         return;
      LocalBonusMessage("You got an extra pistol.");

		break;
	case stat_mp40:
      if (GiveBulletWeapon(ob,wp_mp40,check)==false)
         return;
      LocalBonusMessage("You picked up an MP40.");

      break;

	case stat_bazooka:
      if (GivePlayerMissileWeapon(ob,pstate,check)==false)
         return;

      LocalBonusMessage("You bagged a bazooka!");
      break;

   case stat_firebomb:
      if (GivePlayerMissileWeapon(ob,pstate,check)==false)
         return;

      LocalBonusMessage("You found a Firebomb!");
      break;


	case stat_heatseeker:
      if (GivePlayerMissileWeapon(ob,pstate,check)==false)
         return;

      LocalBonusMessage("You have a Heat-seeker!");
      break;


	case stat_drunkmissile:
      if (GivePlayerMissileWeapon(ob,pstate,check)==false)
         return;

      LocalBonusMessage("You recovered a Drunk Missile!");
      break;

	case stat_firewall:
      if (GivePlayerMissileWeapon(ob,pstate,check)==false)
         return;

      LocalBonusMessage("You filched a FlameWall!");
      break;

	case stat_splitmissile:
      if (GivePlayerMissileWeapon(ob,pstate,check)==false)
         return;

      LocalBonusMessage("You snagged a Split Missile!");
      break;

	case stat_kes:
      if (GivePlayerMissileWeapon(ob,pstate,check)==false)
         return;

      LocalBonusMessage("You wield the Dark Staff!");
      break;

	case stat_bat:
      if (GivePlayerMissileWeapon(ob,pstate,check)==false)
         return;

      LocalBonusMessage("You picked up the Excalibat.");
      break;


	case stat_lifeitem1:
      GiveLifePoints(ob,1);
      if (timelimitenabled)
         timelimit+=(VBLCOUNTER);

		break;

	case stat_lifeitem2:
      GiveLifePoints(ob,5);
      if (timelimitenabled)
         timelimit+=(2*VBLCOUNTER);
       break;

   case stat_lifeitem3:
      GiveLifePoints(ob,10);
      if (timelimitenabled)
         timelimit+=(5*VBLCOUNTER);
       break;

   case stat_lifeitem4:
      GiveLifePoints(ob,25);
      if (timelimitenabled)
         timelimit+=(10*VBLCOUNTER);
       break;

	case stat_random:
		switch (GameRandomNumber("GetBonus",0)>>6)
			{
			case 0:
				check->itemnumber=stat_godmode;
				break;
			case 1:
				check->itemnumber=stat_elastic;
				break;
			case 2:
				check->itemnumber=stat_dogmode;
				break;
			case 3:
				check->itemnumber=stat_mushroom;
				break;
			}
      randompowerup=true;
      LocalBonus1Message("Random powerup gives you . . .");
		goto randomlabel;
		break;

   case stat_bulletproof:
      GiveProtection(FL_BPV, GetBonusTimeForItem(stat_bulletproof),
         SD_GETBVESTSND);
      LocalBonusMessage("Bulletproof Armor!");

      goto drw;

	case stat_gasmask:
      GiveProtection(FL_GASMASK, GetBonusTimeForItem(stat_gasmask),
                     SD_GETMASKSND);
      LocalBonusMessage("You put on a Gas Mask.");

      goto drw;

	case stat_asbesto:
      GiveProtection(FL_AV,GetBonusTimeForItem(stat_asbesto),
                     SD_GETAVESTSND);
      LocalBonusMessage("Asbestos Armor! Oh so itchy!");

      goto drw;

   case stat_elastic:
      if (GivePowerup(ob,FL_ELASTO,GetBonusTimeForItem(stat_elastic),
                  SD_GETELASTSND) == false)
         return;
      LocalBonusMessage("Elasto Mode!");

      ob->flags |= FL_NOFRICTION;
      goto drw;

   case stat_fleetfeet:
      if (GivePowerup(ob,FL_FLEET,GetBonusTimeForItem(stat_fleetfeet),
                  SD_GETFLEETSND) == false)
         return;

      LocalBonus1Message("Mercury Mode!");
      LocalBonusMessage("Press Look Up and Down to fly.");

      ob->flags &= ~FL_NOFRICTION;
      goto drw;


   case stat_mushroom:
      if (GivePowerup(ob,FL_SHROOMS,GetBonusTimeForItem(stat_mushroom),
                  SD_GETSHROOMSSND) == false)
         return;
      LocalBonusMessage("Shrooms Mode!");

      ob->flags &= ~FL_NOFRICTION;
      gamestate.democraticcount ++;
      goto drw;


   case stat_godmode:

      if (ob->flags & FL_GODMODE)
         return;

      if (!ARMED(ob->dirchoosetime))
         return;

      ob->flags &= ~FL_NOFRICTION;

      if (ob->flags & FL_DOGMODE)
         {
         ob->temp2 = DOGMODERISE;
         ResetWeapons(ob);
         if (ob->state->condition & SF_DOGSTATE)
            NewState(ob,&s_player);
         }

      SaveWeapons(ob);
      SpawnNewObj(ob->tilex,ob->tiley,&s_flash1,inertobj);
      new->flags |= FL_ABP;
      new->x = new->drawx = ob->x;
      new->y = new->drawy = ob->y;
      MakeActive(new);
      new->z = ob->z;


      SetPlayerHorizon(pstate,GODYZANGLE);

      ob->flags &= ~(FL_GODMODE|FL_SHROOMS|FL_ELASTO|FL_FLEET|FL_DOGMODE);
      ob->flags |= FL_GODMODE;
      ob->temp2 = GODMODERISE;
      pstate->poweruptime = GetBonusTimeForItem(stat_godmode);
      pstate->soundtime = 0;
      GiveMissileWeapon(ob,wp_godhand);
      SD_PlaySoundRTP(SD_GETGODSND,ob->x, ob->y);
      gamestate.supercount ++;
      LocalBonusMessage("God Mode!");

      goto drw;


#if (SHAREWARE == 0)
	case stat_dogmode:
      if (ob->flags & FL_DOGMODE)
         return;

      if (!ARMED(ob->dirchoosetime))
         return;

      ob->flags &= ~FL_NOFRICTION;

      if (ob->flags & FL_GODMODE)
         {
         ob->temp2 = GODMODEFALL;
         ResetWeapons(ob);
         }


      SaveWeapons(ob);
      ob->shapeoffset = 0;
      NewState(ob,&s_serialdog);

      SpawnNewObj(ob->tilex,ob->tiley,&s_flash1,inertobj);
      new->flags |= FL_ABP;
      new->x = new->drawx = ob->x;
      new->y = new->drawy = ob->y;
      MakeActive(new);
      new->z = ob->z;

      SetPlayerHorizon(pstate,DOGYZANGLE);

      ob->flags &= ~(FL_DOGMODE|FL_SHROOMS|FL_ELASTO|FL_FLEET|FL_GODMODE);
      ob->flags |= FL_DOGMODE;
      ob->temp2 = DOGMODEFALL;
      pstate->poweruptime = GetBonusTimeForItem(stat_dogmode);
      pstate->soundtime = 0;
      GiveMissileWeapon(ob,wp_dog);
      SD_PlaySoundRTP(SD_GETDOGSND,ob->x, ob->y);
      gamestate.supercount ++;
      LocalBonusMessage("Dog Mode!");

      goto drw;
   #endif

 drw:

		if (ob==player)
         GM_DrawBonus (check->itemnumber);

		break;

	case stat_dipball1:
	case stat_dipball2:
	case stat_dipball3:
      SD_PlaySoundRTP(SD_GETBONUSSND,ob->x, ob->y);
		gamestate.dipballs++;
      LocalBonusMessage("You discovered a Developer Ball!");

	break;

   case stat_collector:
      if (gamestate.battlemode==battle_CaptureTheTriad)
        {if (pstate->team == check->hitpoints)
           return;
         ob->flags |= FL_DESIGNATED;
         UpdateKills = true;
         LocalBonusMessage( "You picked up a triad!  RUN!!!");
        }
      else
        BATTLE_CheckGameStatus(battle_get_collector_item,ob->dirchoosetime);

      SD_PlaySoundRTP(SD_GETBONUSSND,ob->x, ob->y);
		break;
#if (SHAREWARE == 0)
	case stat_mine:
		SpawnNewObj(check->tilex,check->tiley,&s_grexplosion1,inertobj);
		new->flags |= FL_ABP;
		new->whatever = check;
		new->temp2 = 100;
		MakeActive(new);
		SD_PlaySoundRTP(SD_EXPLODESND,check->x,check->y);
		break;
#endif
	default:
      SD_PlaySoundRTP(SD_GETHEADSND,ob->x, ob->y);
		break;
	}
	//StartBonusFlash ();
	if (check->flags & FL_CHANGES)
		{
		switch (check->itemnumber)
			{
			case stat_pedgoldkey:
			case stat_pedsilverkey:
			case stat_pedironkey:
			case stat_pedcrystalkey:
				check->itemnumber = stat_emptypedestal;
				check->shapenum = stats[stat_emptypedestal].picnum;
				check->flags = stats[stat_emptypedestal].flags|FL_ABP;
				check->count = 0;
				check->numanims = 0;
				break;
			case stat_healingbasin:
				check->itemnumber = stat_emptybasin;
				check->shapenum = stats[stat_emptybasin].picnum;
				check->flags = stats[stat_emptybasin].flags|FL_ABP;
				check->count = 0;
				check->numanims = 0;
				break;
			/*
				case stat_tablebullets:
				check->itemnumber = stat_emptytable;
				check->shapenum = stats[stat_emptytable].picnum;
				check->flags = stats[stat_emptytable].flags|FL_ABP;
				break;
			case stat_statuewithpole:
				check->itemnumber = stat_armornopole;
				check->shapenum = stats[stat_armornopole].picnum;
				check->flags = stats[stat_armornopole].flags|FL_ABP;
				break; */
			case stat_pit:
				check->shapenum ++;
				check->flags &= ~FL_CHANGES;
				check->flags &= ~FL_BONUS;
				break;
			case stat_knifestatue:
				check->itemnumber = stat_emptystatue;
				check->shapenum = stats[stat_emptystatue].picnum;
				check->flags = stats[stat_emptystatue].flags|FL_ABP;
				break;

			default:
			    ;
			}
		}
	else
	  {if (check == sprites[check->tilex][check->tiley])
         {
         statobj_t *checkstat;

         checkstat = (statobj_t*)DiskAt(check->tilex,check->tiley);
         if (checkstat && (checkstat->which == SPRITE))
            sprites[check->tilex][check->tiley] = checkstat;
         else
            sprites[check->tilex][check->tiley] = NULL;
         }

      if (randompowerup==true)
         check->itemnumber = stat_random;
      RemoveStatic(check);
	  }
}


/*
===================
=
= DropWeapon
=
===================
*/

void DropWeapon(objtype *ob)
{playertype *pstate;
 int dtilex,dtiley;



 M_LINKSTATE(ob,pstate);

 if ((pstate->missileweapon == -1) || (pstate->missileweapon == wp_godhand)
#if (SHAREWARE == 0)
     || (pstate->missileweapon == wp_dog)
#endif
     )
	return;

 pstate->weapondowntics = WEAPONS[pstate->weapon].screenheight/3;
 pstate->new_weapon = pstate->bulletweapon;

 dtilex = ob->tilex;
 dtiley = ob->tiley;

 FindEmptyTile(&dtilex, &dtiley);


 SpawnStatic(dtilex,dtiley,GetItemForWeapon(pstate->missileweapon),9);
 gamestate.missiletotal ++;

 LASTSTAT->ammo = pstate->ammo;
 LASTSTAT->flags |= FL_ABP;
 LASTSTAT->flags &= ~FL_RESPAWN;
 MakeStatActive(LASTSTAT);
 pstate->weaponx = ob->tilex;
 pstate->weapony = ob->tiley;
 pstate->ammo = -1;
 pstate->missileweapon = -1;
 if ((ob==player) && SHOW_BOTTOM_STATUS_BAR() )
	 DrawBarAmmo (false);
}


/*
===================
=
= Thrust
=
===================
*/

void Thrust ( objtype * ob )
{statobj_t *tstat;
 int dx,dy,dz,rad,index,otherteam;
 objtype*temp;
 playertype * pstate;

	PlayerMove(ob);
	M_LINKSTATE(ob,pstate);

	if ((gamestate.battlemode == battle_CaptureTheTriad) &&
		 (ob->flags & FL_DESIGNATED) &&
       (ob->tilex == TEAM[pstate->team].tilex) &&
       (ob->tiley == TEAM[pstate->team].tiley))
      {if (ob == player)
         SD_Play(PlayerSnds[locplayerstate->player]);

       if (BATTLE_CheckGameStatus(battle_captured_triad,ob->dirchoosetime)==
           battle_no_event)
          {
          ob->flags &= ~FL_DESIGNATED;
          UpdateKills = true;
          otherteam = (pstate->team ^ 1);
          SpawnStatic(TEAM[otherteam].tilex,TEAM[otherteam].tiley,stat_collector,9);
          LASTSTAT->flags |= FL_COLORED;
          LASTSTAT->hitpoints = otherteam;
          LASTSTAT->flags |= FL_ABP;
          MakeStatActive(LASTSTAT);
          }
       return;
		}

	if ((ob->tilex != pstate->weaponx) || (ob->tiley != pstate->weapony))
	  pstate->weaponx = pstate->weapony = 0;

   index = touchindices[ob->tilex][ob->tiley];
	if (index && (abs(ob->z - PlatformHeight(ob->tilex,ob->tiley)) < 5))
	 {if (!TRIGGER[index-1]){
#if (BNACRASHPREVENT == 1)
		if (touchplate[index-1] != 0) { //	CRASH IN SHAREWARE 'ride em cowboy' BNA FIX
			 //SetTextMode (  ); qwert  // DONT ALLOW BAD touchplate ( == 0 ) see rt_door.c
#endif
			 if (touchplate[index-1]->complete)
				SD_PlaySoundRTP(SD_BADTOUCHSND,ob->x,ob->y);
			 else {
				SD_PlaySoundRTP(SD_TOUCHPLATESND,ob->x,ob->y);
				if (ob == player)
					AddMessage("Touchplate triggered.",MSG_GAME);  
			 }
#if (BNACRASHPREVENT == 1)
		}else{SD_PlaySoundRTP(SD_BADTOUCHSND,ob->x,ob->y);} 
		//	CRASH IN SHAREWARE 'ride em cowboy' BNA FIX
#endif
	}
	  TRIGGER[index-1] = 1;
	 }

	tstat = sprites[ob->tilex][ob->tiley];
	if (tstat)

	 {dx = abs(ob->x - tstat->x);
	  dy = abs(ob->y - tstat->y);
	  dz = abs(ob->z - tstat->z);
     if ((dx < 0x8000) && (dy < 0x8000) && (dz<=35))
		{
#define OBJECT_IS_BONUS( tstat )                  ( ( tstat )->flags & FL_BONUS )
#define PLAYER_IS_SWITCHING_WEAPONS( pstate )     ( W_CHANGE( pstate ) )
//#define IS_ATTACKING( ob )                        ( ( ( ob )->state->think == T_Attack ) || ( ( ob )->state->think == T_BatBlast ) )
#define IS_WEAPON( tstat )                        ( ( tstat )->flags & FL_WEAPON )
#define PLAYER_MAY_NOT_GET_WEAPON( pstate, ob )   ( PLAYER_IS_SWITCHING_WEAPONS( pstate ))// || IS_ATTACKING( ob ) )
#define OK_TO_PICK_UP( tstat, pstate, ob )        ( !( IS_WEAPON( tstat ) && PLAYER_MAY_NOT_GET_WEAPON( pstate, ob ) ) )
#define WHERE_PLAYER_DROPPED_WEAPON( ob, pstate ) ( ( ( ob )->tilex == ( pstate )->weaponx ) && ( ( ob )->tiley == ( pstate )->weapony ) )

       if ( OBJECT_IS_BONUS( tstat ) &&
          OK_TO_PICK_UP( tstat, pstate, ob ) &&
          (!WHERE_PLAYER_DROPPED_WEAPON( ob, pstate ))
          )
          {
           GetBonus(ob,tstat);
           if (PLAYER_IS_SWITCHING_WEAPONS( pstate ))
             {statetype *nstate;
#if (SHAREWARE == 0)
              if (ob->state->condition & SF_DOGSTATE)
                nstate = &s_serialdog;
              else
#endif
                nstate = &s_player;
              pstate->attackframe = pstate->weaponframe = pstate->batblast = 0;
              NewState(ob,nstate);
             }


          }
       else if ((tstat->itemnumber == stat_pit) &&
                (ob->temp2 != PITFALL) &&
                (!(ob->flags & FL_FLEET))
               )
			 {ob->temp2 = PITFALL;
			  ob->momentumx = ob->momentumy = 0;
			  //ob->momentumz = 4;
			  ob->momentumz = GRAVITY;

			  if (ob->whatever == NULL)
				  {ob->whatever = tstat;
					SD_PlaySoundRTP(SD_PITTRAPSND,ob->x,ob->y);
					tstat->shapenum = stats[tstat->itemnumber].picnum + 1;
					MakeStatInactive(tstat);
			  //tstat->flags &= ~FL_ABP;
				  }
			 }
       else if ((tstat->itemnumber == stat_heatgrate) &&
                (!(ob->flags & FL_DYING)) && (!(ob->flags & FL_AV))
               )
			 {DamageThing(ob,1);
           Collision(ob,(objtype*)tstat,0,0);
           M_CheckPlayerKilled(ob);
			 }


		}
	 }

//================ Check special player/actor links ======================

   if (ob->whatever)
	 {temp = (objtype*)(ob->whatever);

		 dx = abs(ob->x - temp->x);
		 dy = abs(ob->y - temp->y);
       dz = abs(ob->z - temp->z);
		 if (ob->flags & FL_RIDING)
			rad = MINACTORDIST;
		 else
         rad = 0x8000;

       if ((dx >rad) || (dy > rad) || (dz > 64))
			{
         if ((temp->obclass == bladeobj) || (temp->obclass == diskobj))
			 {temp->whatever = NULL;
			  ob->flags &= ~FL_RIDING;
			 }

			else if (ob->temp2 == COLUMNCRUSH)
			  ob->temp2 = RENORMALIZE;
			else if (ob->z < nominalheight)
			  ob->momentumz = 40000;
			else if (temp->which == SPRITE)
			 {tstat = (statobj_t*)(ob->whatever);
			  MakeStatActive(tstat);
			  SetNormalHorizon(ob);
			 }
			ob->whatever = NULL;
			}
	 }

//=========================================================================


   if (BATTLEMODE && (MAPSPOT(ob->tilex,ob->tiley,1) == EXITTILE))
      {
      Move_Player_From_Exit_To_Start(ob);
      return;
      }

   if (ob==player)
		{
      if (MAPSPOT(ob->tilex,ob->tiley,1) == EXITTILE)
         playstate=ex_completed;
      else if ( ( MAPSPOT( ob->tilex, ob->tiley, 2 ) & 0xff00 ) == 0xe400 )
         playstate = ex_secretdone;
		else if (MAPSPOT(ob->tilex,ob->tiley,1) == SECRETEXITTILE)
			playstate=ex_secretlevel;
		}
}



void Move_Player_From_Exit_To_Start(objtype *ob)
   {
   int i = 0,oldarea,newarea;
   objtype *tplayer;
   int travelangle,dangle;
   playertype *pstate;

   M_LINKSTATE(ob,pstate);


   oldarea = ob->areanumber;
   newarea = AREANUMBER(FIRST.x,FIRST.y);
   if (oldarea != newarea)
      {
      RemoveFromArea(ob);
      ob->areanumber = newarea;
      MakeLastInArea(ob);
      }

   SetTilePosition(ob,FIRST.x,FIRST.y);
   ob->z = PlatformHeight(ob->tilex,ob->tiley);
   if ((ob->z == -10) || DiskAt(ob->tilex,ob->tiley))
      ob->z = 0;

   ConnectAreas();
   travelangle = atan2_appx(ob->momentumx,ob->momentumy);
   dangle = ob->angle - travelangle;

   ob->angle = (1-FIRST.dir)*ANG90 + dangle;
	Fix(ob->angle);

	ob->dir   = angletodir[ob->angle];
	pstate->anglefrac= (ob->angle<<ANGLEBITS);
	pstate->angle=0;


   for(i=0;i<numplayers;i++)
      {
      tplayer = PLAYER[i];

      if (ob == tplayer)
         continue;

      if (
         (tplayer->tilex == FIRST.x) &&
         (tplayer->tiley == FIRST.y) &&
         (!(tplayer->flags & FL_DYING))
         )
         {
         playertype *pstate;

         M_LINKSTATE(tplayer,pstate);

         pstate->health = tplayer->hitpoints = 0;
         tplayer->flags |= FL_HBM;
         Collision(tplayer,ob,0,0);
         BATTLE_PlayerKilledPlayer(battle_kill_by_crushing,ob->dirchoosetime,tplayer->dirchoosetime);\

         }
      }


   }



void PlayerSlideMove(objtype * ob)
{int tryx, tryy, tryz;

 tryx = ob->x + ob->momentumx;
 tryy = ob->y + ob->momentumy;
 tryz = ob->z + (ob->momentumz >> 16);

 if (ActorTryMove(ob,ob->x,tryy,tryz))
    {
	 if (ob->momentumx>HITWALLSPEED)
	    SD_PlaySoundRTP(SD_HITWALLSND,ob->x,ob->y);
    ob->momentumx=0;
    }
 else if (ActorTryMove(ob,tryx,ob->y,tryz))
    {
    if (ob->momentumy>HITWALLSPEED)
	    SD_PlaySoundRTP(SD_HITWALLSND,ob->x,ob->y);
	 ob->momentumy=0;
	 }
 else
    {
	 if (FindDistance(ob->momentumx,ob->momentumy)>(HITWALLSPEED*3/2))
	    SD_PlaySoundRTP(SD_HITWALLSND,ob->x,ob->y);
    ob->momentumx=0;
    ob->momentumy=0;
    }
}

/*
===================
=
= SetNormalHorizon
=
===================
*/

void SetNormalHorizon (objtype * ob)
{
	playertype * pstate;

	M_LINKSTATE(ob,pstate);

   if (ob->flags&FL_DOGMODE)
      {
      SetPlayerHorizon(pstate,DOGYZANGLE);
      }
   else if (ob->flags&FL_GODMODE)
      {
      SetPlayerHorizon(pstate,GODYZANGLE);
      }
   else
      {
      SetPlayerHorizon(pstate,NORMALYZANGLE);
      }
}

/*
===================
=
= PlayerTiltHead
=
===================
*/
extern int iG_playerTilt;
extern double dTopYZANGLELIMIT;
void PlayerTiltHead (objtype * ob)
{
	playertype * pstate;
   int dyz=0;
   int yzangle;

//bna++   jumpmode
  if ((donttilt > 0)){
	  donttilt--;
	  return;
  }

	M_LINKSTATE(ob,pstate);

   yzangle=ob->yzangle+HORIZONYZOFFSET;
   Fix(yzangle);

   if (
        (pstate->lastmomz!=ob->momentumz) &&
        (ob->momentumz==0) &&
        (
          (!(ob->flags&FL_FLEET)) ||
          (
            (ob->flags&FL_FLEET) &&
            (ob->z==nominalheight)
          )
        )
      )
      SetNormalHorizon(ob);

   pstate->lastmomz=ob->momentumz;

   if (ob->flags&FL_SHROOMS)
      {
      ob->yzangle = FixedMulShift(SHROOMYZANGLE,sintable[(oldpolltime<<6)&(FINEANGLES-1)],16);
      Fix(ob->yzangle);
      return;
      }
   else if (pstate->guntarget)
      {
		int dx,dy,dz;
      int xydist;
      int yzangle;

      dx = ob->x - pstate->guntarget->x;
		dy = ob->y - pstate->guntarget->y;
      xydist = FindDistance(dx,dy)-0x8000;
      if (ob->z==pstate->guntarget->z)
         dz = 0;
      else
         {
         dz = (ob->z-pstate->guntarget->z)<<10;
         }
      yzangle = atan2_appx(xydist,dz);

      yzangle += HORIZONYZOFFSET;
      Fix(yzangle);
      SetPlayerHorizon(pstate,yzangle-HORIZONYZOFFSET);

		if (oldpolltime>pstate->targettime)
         {
         if (pstate->guntarget)
            pstate->guntarget->flags &= ~FL_TARGET;
			pstate->guntarget=NULL;
         SetNormalHorizon(ob);
         }
      }
   else
      {
      if (pstate->buttonstate[bt_horizonup])
         {
         if (yzangle!=pstate->horizon)
            {
            SetPlayerHorizon(pstate,yzangle-HORIZONYZOFFSET);
            }
         else
            {
            SetPlayerHorizon(pstate,(pstate->horizon-HORIZONYZOFFSET+YZHORIZONSPEED));
            }
         }
      else if (pstate->buttonstate[bt_horizondown])
         {
         if (yzangle!=pstate->horizon)
            {
            SetPlayerHorizon(pstate,yzangle-HORIZONYZOFFSET);
            }
         else
            {
            SetPlayerHorizon(pstate,(pstate->horizon-HORIZONYZOFFSET-YZHORIZONSPEED));
            }
         }
      if (pstate->buttonstate[bt_lookup] || CYBERLOOKUP)
         {
         if (!(ob->flags & FL_FLEET))
            {
            dyz=YZTILTSPEED;
            if (pstate->buttonstate[bt_lookdown] || CYBERLOOKDOWN)
               {
               dyz=0;
               }

            SetNormalHorizon(ob);
            }
         }
      if (pstate->buttonstate[bt_lookdown] || CYBERLOOKDOWN)
         {
         if (!(ob->flags & FL_FLEET))
            {
            dyz=-YZTILTSPEED;
            if (pstate->buttonstate[bt_lookup] || CYBERLOOKUP)
               {
               dyz=0;
               }
            SetNormalHorizon(ob);
            }
         }
      if (!(ob->flags&FL_DOGMODE) &&
               !(ob->flags&FL_GODMODE) &&
               !(ob->flags&FL_FLEET)   &&
               !(ob->flags&FL_RIDING)  &&
               (ob->momentumz > (GRAVITY<<1))//(ob->momentumz>0x1000)
            )
         {
         SetPlayerHorizon(pstate,FALLINGYZANGLE);
         }
      }



   if ((yzangle!=pstate->horizon) && (dyz==0))
      {
      int speed;

      speed=SNAPBACKSPEED;
		if (yzangle<pstate->horizon)
         yzangle+=speed;
		else
         yzangle-=speed;
      if ((abs(yzangle-pstate->horizon))<SNAPBACKSPEED)
         yzangle=pstate->horizon;
      }
//SetTextMode();

	if (yzangle != 512){
		FinddTopYZANGLELIMITvalue(ob);
	}


   yzangle+=dyz;
   if (yzangle-HORIZONYZOFFSET>YZANGLELIMIT)
       yzangle=HORIZONYZOFFSET+YZANGLELIMIT;
/*   else if (yzangle-HORIZONYZOFFSET<-TopYZANGLELIMIT)//bnafix
       yzangle=HORIZONYZOFFSET-TopYZANGLELIMIT;//bnafix
dTopYZANGLELIMIT*/
   else if (yzangle-HORIZONYZOFFSET<-dTopYZANGLELIMIT)//bnafix
       yzangle=HORIZONYZOFFSET-dTopYZANGLELIMIT;//bnafix
   ob->yzangle=yzangle-HORIZONYZOFFSET;
   Fix(ob->yzangle);

   iG_playerTilt = ob->yzangle;

}

//----------------------------------------------------------------------
// bna added function
// if a player is to close to wall, looking down max
//,this func limit the dTopYZANGLELIMIT value when
// facing a wall 
#define SMALLANGLE 90
//there is small angles where didnt work
//so we check for them to = 90/2048 = aprox, 15 degrees
int FinddTopYZANGLELIMITvalue(objtype *ob)
{/*


checkx = ob->tilex + 1;
checky = ob->tiley + 1;
if (actorat[checkx][checky]){
	return 0;
}
return 1;

checkx = ob->tilex ;
checky = ob->tiley;

// find which direction the player is facing
//and check if it is a wall

*/

   //use lowest down angle		   	   	
	dTopYZANGLELIMIT = (26*FINEANGLES/360);
   
	if (ob->angle < 256 || ob->angle > 1792) {
	   if ((tilemap[ob->tilex + 1][ob->tiley])!=0){

			return 0;
	   }
       //ob->dir = east;
   }else if (ob->angle < 768) {
	   if ((tilemap[ob->tilex][ob->tiley-1])!=0){
			return 0;
	   }
   }else if (ob->angle < 1280) {
	   if ((tilemap[ob->tilex-1][ob->tiley])!=0){
			return 0;
	   }
   }else{
	   if ((tilemap[ob->tilex][ob->tiley+1])!=0){
			return 0;
	   }
   }


   //use middle down angle
   dTopYZANGLELIMIT = (42*FINEANGLES/360);

   if ((ob->angle > 768-SMALLANGLE)&&(ob->angle <= 768)) { 
   	   if ((tilemap[ob->tilex -1][ob->tiley])!=0){//ob->tiley-1
			return 0;
	   }
   }
   if ((ob->angle < 1280+SMALLANGLE)&&(ob->angle >= 1280)) { 
   	   if ((tilemap[ob->tilex - 1][ob->tiley])!=0){//ob->tiley+1
			return 0;
	   }
   } 
   if ((ob->angle > 256)&&(ob->angle <= 256+SMALLANGLE)) { 
   	   if ((tilemap[ob->tilex + 1][ob->tiley])!=0){//ob->tiley-1
			return 0;
	   }
   }   
   if ((ob->angle < 1792)&&(ob->angle >= 1792-SMALLANGLE)) { 
   	   if ((tilemap[ob->tilex + 1][ob->tiley])!=0){//ob->tiley+1
			return 0;
	   }
   } 
   if ((ob->angle < 1280)&&(ob->angle >= 1280-SMALLANGLE)) { 
   	   if ((tilemap[ob->tilex ][ob->tiley+1])!=0){//ob->tilex-1
			return 0;
	   }
   } 
   if ((ob->angle > 1792)&&(ob->angle <= 1792+SMALLANGLE)) { 
   	   if ((tilemap[ob->tilex ][ob->tiley+1])!=0){//ob->tiley+1
			return 0;
	   }
   } 
   if ((ob->angle > 768)&&(ob->angle <= 768+SMALLANGLE)) { 
   	   if ((tilemap[ob->tilex][ob->tiley-1])!=0){//ob->tiley-1
			return 0;
	   }
   }
   if ((ob->angle < 256)&&(ob->angle >= 256-SMALLANGLE)) { 
   	   if ((tilemap[ob->tilex][ob->tiley-1])!=0){//ob->tiley-1
			return 0;
	   }
   }

   //use max down angle
   dTopYZANGLELIMIT = (90*FINEANGLES/360);
   return 1;
}
// bna added function end
//----------------------------------------------------------------------




/*
===================
=
= UpdatePlayers
=
=================== QWERTYU
*/
void UpdatePlayers ( void )
{
   objtype * obj;
//   playertype * pstate;

	for (obj = FIRSTACTOR; obj->obclass==playerobj; obj = obj->next)
		 {

//ErrorDontQuit("obj->next = ",obj->next);
#if (BNACRASHPREVENT == 1)//crashed here when oscuro and larves were all killed
		if (obj->next == 0){return;}
#endif
       obj->speed=FindDistance(obj->momentumx, obj->momentumy);
//       M_LINKSTATE(obj,pstate);
//       pstate->steptime-=obj->speed;
//       if (pstate->steptime<0)
//          {
//          while (pstate->steptime<0)
//             pstate->steptime+=PLAYERSTEPTIME;
//          pstate->stepwhich^=1;
//          SD_PlaySoundRTP(SD_WALK1SND+pstate->stepwhich,obj->x,obj->y);
//          }
		 }
}


/*
===================
=
= PlayerMove
=
===================
*/

void PlayerMove ( objtype * ob )
{playertype *pstate;

   M_LINKSTATE(ob,pstate);
   if (ob->flags & FL_FLEET)
      CheckFlying(ob,pstate);
   ActorMovement(ob);
   pstate->anglefrac = (pstate->anglefrac+pstate->angle)&((FINEANGLES<<ANGLEBITS)-1);
	ob->angle = (pstate->anglefrac >> ANGLEBITS);
	ob->dir = angletodir[ob->angle];
	if (ob==player)
		UpdateLightLevel(player->areanumber);

   PlayerTiltHead(ob);

   if (IsWindow(ob->tilex,ob->tiley))
		{
		if (!(ob->flags & FL_DYING))
			{
         pstate->health = 0;
         pstate->falling=true;
         if ((ob->flags & FL_GODMODE) || (ob->flags & FL_DOGMODE) ||
             (gamestate.battlemode == battle_Eluder) || (godmode==true))
            {
            KillActor(ob);
            NewState(ob,&s_remotedie1);
            }
         else
            Collision(ob,(objtype*)NULL,0,0);
         ob->flags |= FL_DYING;
         M_CheckPlayerKilled(ob);
			}
		}
}

/*
===============
=
= T_Tag
=
===============
*/

void  T_Tag (objtype *ob)
{
	playertype *pstate;

	CheckPlayerSpecials(ob);
	//CheckWeaponStates(ob);
	M_LINKSTATE(ob,pstate);
	Thrust(ob);

	if (ob->ticcount > 4)
		pstate->weaponheight -= GMOVE;
	else
		pstate->weaponheight += GMOVE;

	if ((ob->ticcount > 1) && (ob->ticcount < 8) && (!(ob->flags & FL_DIDTAG)))
	  {int i,dx,dy,magangle;

		for(i=0;i<numplayers;i++)
		  {if (PLAYER[i] == ob)
			  continue;

			dx = abs(PLAYER[i]->x - ob->x);
			dy = abs(PLAYER[i]->y - ob->y);
			if ((dx > 0xc000) || (dy > 0xc000))
			 continue;
			magangle = abs(ob->angle - AngleBetween(ob,PLAYER[i]));
			if (magangle > VANG180)
				magangle = ANGLES - magangle;

			if (magangle > ANGLES/8)
				continue;
         CheckTagGame(ob,PLAYER[i]);
			break;
		  }

	  }


//Commented out until we find if it's valid
/*
	if (!ob->ticcount)
	  {if ( pstate->buttonstate[bt_use] && !pstate->buttonheld[bt_use] )
			pstate->buttonstate[bt_use] = false;
		if ( pstate->buttonstate[bt_attack] && !pstate->buttonheld[bt_attack])
			pstate->buttonstate[bt_attack] = false;
	  }
*/
}

#if (SHAREWARE == 0)
#define GET_RESETSTATE(ob,resetstate)      \
   {                                       \
   if (ob->state->condition & SF_DOGSTATE) \
      resetstate = &s_serialdog;           \
   else                                    \
      resetstate = &s_player;              \
   }
#else
#define GET_RESETSTATE(ob,resetstate)      \
   {                                       \
   resetstate = &s_player;                 \
   }
#endif

/*
===============
=
= T_Attack
=
===============
*/

void  T_Attack (objtype *ob)
{
	attack_t   *cur;

	playertype *pstate;
   statetype *resetstate;


   Thrust(ob);
   if (ob->flags & FL_DYING)
     return;

   resetstate = ob->state;
   CheckPlayerSpecials(ob);
   if (resetstate != ob->state)
      {
      return;
      }


   if ((ob->flags & FL_PAIN) && (!ob->ticcount))
	  ob->flags &= ~FL_PAIN;

	CheckWeaponStates(ob);
	M_LINKSTATE(ob,pstate);
   GET_RESETSTATE(ob,resetstate);


   if (ARMED(ob->dirchoosetime)
   //(gamestate.battlemode != battle_Tag)
      )

      {
      if (pstate->weapondowntics == 1)  // change to up; during change, up and down
                                // are never zero at the same time
         {
   #if (SHAREWARE == 0)
         if (pstate->weapon == wp_kes)
            {
            pstate->weapondowntics = 0;
            pstate->weaponuptics = KESTICS/(2*GMOVE);
            }
         else
   #endif
            {
            pstate->weapondowntics = 0;
            pstate->weaponframe = pstate->attackframe = 0;

            if (pstate->NETCAPTURED == -1)
               {
               pstate->weaponuptics = FREE.screenheight/GMOVE;
               pstate->weaponheight = pstate->weaponuptics*GMOVE ;
               pstate->NETCAPTURED = 1;
     //          return;
               }
            else if (pstate->NETCAPTURED == -2)
               {
               pstate->weaponuptics = WEAPONS[pstate->weapon].screenheight/GMOVE;
               pstate->weaponheight = pstate->weaponuptics*GMOVE ;
               pstate->NETCAPTURED = 0;

              //return;
               }
            else
               {
               pstate->weaponuptics = WEAPONS[pstate->new_weapon].screenheight/GMOVE;
               pstate->weapon = pstate->new_weapon;
               pstate->weaponheight = pstate->weaponuptics*GMOVE ;
               }
            }
         }
      }
  else if (gamestate.battlemode == battle_Hunter)
     {
     if (pstate->weapondowntics == 1)

        {
        pstate->weapondowntics = 0;
        pstate->weaponframe = pstate->attackframe = pstate->batblast = 0;
        pstate->weapon = pstate->new_weapon;
        NewState(ob,resetstate);
        return;
        }
     }




#if (SHAREWARE==0)
   if ((pstate->buttonstate[bt_use]) &&
       (pstate->weapon != wp_dog)
      )
#else
   if (pstate->buttonstate[bt_use])
#endif
		Cmd_Use (ob);

   if (pstate->attackframe >= WEAPONS[pstate->weapon].numattacks)
      Error("\n attackframe %d for weapon %d gt numattacks %d",
            pstate->attackframe,pstate->weapon,
            WEAPONS[pstate->weapon].numattacks
           );

   cur = &(WEAPONS[pstate->weapon].attackinfo[pstate->attackframe]);
	if (! pstate->attackcount)
      {
      switch (cur->attack)
         {
         case reset:
            if (vrenabled && (ob==player))
               {
               StopVRFeedback();
               }
            ob->flags &= ~FL_FULLLIGHT;
            if (pstate->ammo)
               {
               if (BATTLEMODE && (pstate->weapon == wp_firewall))
                  break;
               if (BATTLEMODE && (pstate->weapon == wp_firebomb))
                  break;
               if ((pstate->buttonstate[bt_attack]) && (pstate->weapon <= wp_firewall))
                  {
                  if (pstate->weapon <= wp_mp40)
                     {
                     if (pstate->weapon == wp_twopistol)
                        pstate->attackframe -= 6;
                     else if (pstate->weapon == wp_pistol)
                        pstate->attackframe -= 3;
                     else if (pstate->weapon == wp_mp40)
                        pstate->attackframe -= 2;

                     if (ob->state == &s_pmissattack2)
                        NewState(ob,&s_pmissattack1);
                     else if (ob->state == &s_pgunattack2)
                        NewState(ob,&s_pgunattack1);

                     }

                  }
               else
                  {
                  NewState(ob,resetstate);
                  pstate->attackframe = pstate->weaponframe = 0;
                  pstate->batblast = 0;
                  return;
                  }
               }
            else
               {
               NewState(ob,resetstate);
               pstate->attackframe = pstate->weaponframe = 0;
               pstate->new_weapon = pstate->bulletweapon;
               pstate->batblast = 0;
               pstate->ammo = -1;
               pstate->missileweapon = -1;
               pstate->weapondowntics = WEAPONS[pstate->weapon].screenheight/GMOVE;

               if ((ob==player) && SHOW_BOTTOM_STATUS_BAR() )
                     DrawBarAmmo (false);

               return;
               }
            break;

         case reset2:
            if (vrenabled && (ob==player))
               {
               StopVRFeedback();
               }
            ob->flags &= ~FL_FULLLIGHT;
            if (pstate->buttonstate[bt_attack] && pstate->ammo)
               {
               pstate->attackframe -= 4;
               if (ob->state == &s_pmissattack2)
                  NewState(ob,&s_pmissattack1);
               else if (ob->state == &s_pgunattack2)
                  NewState(ob,&s_pgunattack1);
               }
            else
               {
               NewState(ob,resetstate);
               pstate->attackframe = pstate->weaponframe = 0;
               pstate->batblast = 0;
               return;
               }

            break;


         case at_pulltrigger:
            if (vrenabled && (ob==player))
               {
               StartVRFeedback(1);
               }
            ob->flags |= FL_FULLLIGHT;

      #if (SHAREWARE == 0)

            if (pstate->batblast >= BBTIME)
               DogBlast(ob);

            else if (pstate->weapon == wp_dog)
               DogAttack(ob);
            else if (pstate->weapon == wp_bat)
               BatAttack (ob);
            else
      #endif
               GunAttack(ob);

            break;

         case at_missileweapon:
            if (!pstate->ammo)
               {
               pstate->attackframe = pstate->weaponframe = 0;
               pstate->batblast = 0;
               return;
               }

            if (vrenabled && (ob==player))
               {
               StartVRFeedback(1);
               }
            ob->flags |= FL_FULLLIGHT;
            if (ob==player)
               SetIllumination(2);
            PlayerMissileAttack(ob);


            if (!(ob->flags & FL_GODMODE) && !godmode)
               {
               if (!BATTLEMODE || (gamestate.BattleOptions.Ammo != bo_infinite_shots))
                  pstate->ammo--;

               if ((ob==player) && SHOW_BOTTOM_STATUS_BAR() )
                  DrawBarAmmo (false);
               }
   #if (SHAREWARE == 0)
            if (pstate->weapon == wp_kes)
               pstate->weapondowntics = KESTICS/(2*GMOVE);
   #endif
            break;

	 default:
	     ;
         }

      pstate->attackframe++;
      cur = &(WEAPONS[pstate->weapon].attackinfo[pstate->attackframe]);
      pstate->weaponframe = cur->frame;
      pstate->attackcount = cur->mtics;

      }
   else
      pstate->attackcount --;
   }

/*
===============
=
= T_BatBlast
=
===============
*/

void  T_BatBlast (objtype *ob)
{
	attack_t   *cur;
	playertype *pstate;


   Thrust(ob);
   CheckPlayerSpecials(ob);
   M_LINKSTATE(ob,pstate);


//Commented out until we find if it's valid
/*
	if (!ob->ticcount)
	 {if ( pstate->buttonstate[bt_use] && !pstate->buttonheld[bt_use] )
		pstate->buttonstate[bt_use] = false;

	  if ( pstate->buttonstate[bt_attack] && !pstate->buttonheld[bt_attack])
		pstate->buttonstate[bt_attack] = false;
	 }
*/
//
// change frame and fire
//

	BatBlast(ob);



   if (pstate->attackframe >= WEAPONS[pstate->weapon].numattacks)
      Error("\n attackframe %d for weapon %d gt numattacks %d",
            pstate->attackframe,pstate->weapon,
            WEAPONS[pstate->weapon].numattacks
           );



   cur = &(WEAPONS[pstate->weapon].attackinfo[pstate->attackframe]);
	if (! pstate->attackcount)
		{if (cur->attack == reset)
			{
			 if (!(ob->flags & FL_GODMODE) && (!godmode))
				{pstate->ammo --;
             if ((ob==player) && SHOW_BOTTOM_STATUS_BAR() )
						DrawBarAmmo (false);
				}

			 if (!pstate->ammo)
				{pstate->new_weapon = pstate->bulletweapon;
				 pstate->ammo = -1;
				 pstate->missileweapon = -1;
				 pstate->weapondowntics = WEAPONS[pstate->weapon].screenheight/GMOVE;

				}

			 NewState(ob,&s_player);
			 pstate->attackframe = pstate->weaponframe = 0;
			 pstate->batblast = 0;
			 return;
			}
		 pstate->attackframe++;
		 pstate->weaponframe = WEAPONS[pstate->weapon].attackinfo[pstate->attackframe].frame;
		 cur = &(WEAPONS[pstate->weapon].attackinfo[pstate->attackframe]);
		 pstate->attackcount = cur->mtics;
		}
	else
		pstate->attackcount --;

}




void  T_Free (objtype *ob)
{
	attack_t   *cur;


  if (!ob->ticcount)
  {


//Commented out until we find if it's valid
/*
	if ( locplayerstate->buttonstate[bt_use] && !locplayerstate->buttonheld[bt_use] )
		locplayerstate->buttonstate[bt_use] = false;

	if ( locplayerstate->buttonstate[bt_attack] && !locplayerstate->buttonheld[bt_attack])
		locplayerstate->buttonstate[bt_attack] = false;
*/
   Thrust(ob);

  }
  CheckPlayerSpecials(ob);






  cur = &(FREE.attackinfo[locplayerstate->attackframe]);

	if (!locplayerstate->attackcount)
	 {
		if ((locplayerstate->weaponframe > 3) && (locplayerstate->weaponframe < 8))
		  locplayerstate->NETCAPTURED ++;
		else if (locplayerstate->weaponframe == 8)
		 {
			  locplayerstate->NETCAPTURED = -2;
			  MISCVARS->NET_IN_FLIGHT = false;
			  NewState(ob,&s_player);
			  locplayerstate->weapondowntics = FREE.screenheight/GMOVE;
		  return;
		 }

		locplayerstate->attackframe++;
		cur = &(FREE.attackinfo[locplayerstate->attackframe]);
		locplayerstate->weaponframe = cur->frame;

		locplayerstate->attackcount = cur->mtics;

	 }
	else
	 locplayerstate->attackcount --;
 }



void Switch_Who_Is_It_For_Tag(objtype *actor1,objtype *actor2)
{
 playertype *pstate;

 M_LINKSTATE(actor1,pstate);
 if (pstate->buttonstate[bt_use])
      {playertype *pstate2;

      M_LINKSTATE(actor2,pstate2);
      pstate2->oldmissileweapon = pstate2->oldweapon = pstate2->new_weapon =
      pstate2->missileweapon = pstate2->weapon = wp_godhand;
      pstate2->weaponheight = 144;
      pstate2->weaponuptics = (144 - TAGHANDHEIGHT)/GMOVE;
      pstate2->weapondowntics = 0;
      actor1->flags |= FL_DIDTAG;
      actor2->flags |= FL_DESIGNATED;
      UpdateKills = true;

      actor1->flags &= ~FL_DESIGNATED;
      BATTLE_PlayerKilledPlayer(battle_player_tagged,
         actor1->dirchoosetime, actor2->dirchoosetime);
      }

}



void CheckTagGame(objtype *actor1,objtype*actor2)
{
 //if ((actor1->obclass != playerobj) || (actor2->obclass != playerobj))
	//return;

 if (!BATTLEMODE)
	return;

 if (gamestate.battlemode != battle_Tag)
	return;

 SD_PlaySoundRTP(SD_GETBONUSSND,actor1->x,actor1->y);

 if (actor1->flags & FL_DESIGNATED)
   Switch_Who_Is_It_For_Tag(actor1,actor2);
 else if (actor2->flags & FL_DESIGNATED)
   Switch_Who_Is_It_For_Tag(actor2,actor1);

}






/*
======================
=
= CheckWeaponChange
=
= Keys 1-4 change weapons
=
======================
*/

void CheckWeaponChange (objtype * ob)
{
//   int     i;

	playertype * pstate;


	M_LINKSTATE(ob,pstate);
	//pstate = (ob == player)?(&playerstate):(&remoteplayerstate);
	if (W_CHANGE(pstate))
	 return;

	if (!ARMED(ob->dirchoosetime))
	  return;

	if ((ob->flags & FL_DOGMODE) || (ob->flags & FL_GODMODE))
		return;

#if (WEAPONCHEAT==1)
		if (godmode && Keyboard[sc_Insert])
			{
			SD_Play(SD_SELECTWPNSND);

			// FOR DEBUG only
			/*
			if (pstate->buttonstate[bt_run])
			 {
			 if (pstate->weapon == 0)
			  pstate->new_weapon = wp_bat;
			 else
			  pstate->new_weapon --;
			 }
			else
			 {*/
			 if (pstate->weapon == MAXWEAPONS-1)
			  pstate->new_weapon = 0;
			 else
			  pstate->new_weapon ++;
			 //}
			if (pstate->new_weapon <= wp_mp40)
			 {pstate->bulletweapon = pstate->new_weapon;
			  pstate->HASBULLETWEAPON[pstate->new_weapon] = 1;
			 }
			else
			 pstate->missileweapon = pstate->new_weapon;

         pstate->ammo = stats[GetItemForWeapon(pstate->missileweapon)].ammo;
         if (pstate->ammo<1)
            pstate->ammo=5;
         StartWeaponChange;
         //pstate->weapondowntics = WEAPONS[pstate->weapon].screenheight/GMOVE;
         //if ((ob==player) && SHOW_BOTTOM_STATUS_BAR() )
			  //		DrawBarAmmo (false);
			}
		else if (pstate->buttonstate[bt_swapweapon])
#else
		if (pstate->buttonstate[bt_swapweapon])
#endif
		 {
		  if ((pstate->weapon == pstate->bulletweapon) &&
				(pstate->missileweapon != -1))
			  {pstate->new_weapon = pstate->missileweapon;
				StartWeaponChange;
			  }

		  else if (pstate->weapon != pstate->bulletweapon)
			{pstate->new_weapon = pstate->bulletweapon;
			 StartWeaponChange;
			}
		 }
      else if ((pstate->buttonstate[bt_dropweapon]) &&
         (!gamestate.BattleOptions.WeaponPersistence))
			{
			if (pstate->weapon==pstate->bulletweapon)
				{
				if ((ob==player) && (!(pstate->buttonheld[bt_dropweapon])))
               PlayNoWaySound();
				}
			else
				{
				if (sprites[ob->tilex][ob->tiley])
					{
					if ((ob==player) && (!(pstate->buttonheld[bt_dropweapon])))
                  PlayNoWaySound();
					}
				else
					{
					DropWeapon(ob);
					}
				}
			}



	 if (pstate->buttonstate[bt_pistol])
		{if (pstate->weapon != wp_pistol)
			{pstate->new_weapon = pstate->bulletweapon = wp_pistol;
			 StartWeaponChange;
			}
		}

	 else if (pstate->buttonstate[bt_dualpistol])
		{if ((pstate->weapon != wp_twopistol) && pstate->HASBULLETWEAPON[wp_twopistol])
			{pstate->new_weapon = pstate->bulletweapon = wp_twopistol;
			 StartWeaponChange;
			}
		}
	 else if (pstate->buttonstate[bt_mp40])
		{if ((pstate->weapon != wp_mp40) && pstate->HASBULLETWEAPON[wp_mp40])
			{pstate->new_weapon = pstate->bulletweapon = wp_mp40;
			 StartWeaponChange;
			}
		}
	 else if (pstate->buttonstate[bt_missileweapon])
		{if ((pstate->weapon != pstate->missileweapon) && (pstate->missileweapon != -1))
			{pstate->new_weapon = pstate->missileweapon;
			 StartWeaponChange;
			}
		}
}


void SetWhoHaveWeapons(void)
   {
   playertype *pstate;
   objtype    *ob;
   int        i;

   for ( i = 0; i < numplayers; i++ )
      {
      ob = PLAYER[ i ];
		M_LINKSTATE( ob, pstate );

		if ( ARMED( ob->dirchoosetime ))
         {
         if (pstate->weapon == -1)
            {
            if (( pstate->missileweapon != -1 ) && (pstate->ammo>0))
               pstate->new_weapon = pstate->missileweapon;

            else
               {
               if ( pstate->bulletweapon == -1 )
                  {
                  pstate->HASBULLETWEAPON[ wp_pistol ] = 1;
                  pstate->bulletweapon = wp_pistol;
                  }
               pstate->new_weapon = pstate->bulletweapon;
               pstate->ammo = -1;
               }

            pstate->weapon = pstate->new_weapon;
            pstate->weapondowntics = 0;
            pstate->weaponuptics = WEAPONS[pstate->new_weapon].screenheight/GMOVE;
            pstate->weaponheight = pstate->weaponuptics*GMOVE ;
            pstate->attackframe = pstate->weaponframe =
            pstate->batblast = 0;
            if (i == consoleplayer)
               DrawBarAmmo(false);
            }
			}
		else
			{
			if (ob->flags & FL_DOGMODE)
            {
            ob->temp2 = DOGMODERISE;
            pstate->oldweapon = -1;
            ResetWeapons(ob);
            if (ob->state->condition & SF_DOGSTATE)
               NewState(ob,&s_player);
				}
			else if (ob->flags & FL_GODMODE)
            {
            ob->temp2 = GODMODEFALL;
            pstate->oldweapon = -1;
            ResetWeapons(ob);
				}
			else
            {
            pstate->new_weapon = -1;
            StartWeaponChange;
				}
         pstate->attackframe = pstate->weaponframe =
         pstate->batblast = 0;
			}
		}
	}








void CheckWeaponStates(objtype*ob)
{playertype *pstate;

 M_LINKSTATE(ob,pstate);

 if (gamestate.battlemode == battle_Tag)
	{if (pstate->weapondowntics)
		{pstate->weaponheight += GMOVE;
		 pstate->weapondowntics --;
		 if (!pstate->weapondowntics)
			pstate->weapon = pstate->missileweapon = pstate->bulletweapon =
			pstate->new_weapon = pstate->oldweapon = pstate->oldmissileweapon = -1;

		}


	 if (pstate->weaponuptics)
		{pstate->weaponheight -= GMOVE;
		 pstate->weaponuptics --;
		}
	 return;
	}

 if (pstate->weapondowntics)
		{pstate->weaponheight += GMOVE;
#if (SHAREWARE == 0)
       if ((pstate->weapon == wp_kes) && pstate->attackframe)
			pstate->weaponheight += GMOVE;
#endif
		 pstate->weapondowntics --;
		}
 else if (pstate->weaponuptics)
		{pstate->weaponheight -= GMOVE;
#if (SHAREWARE == 0)
       if ((pstate->weapon == wp_kes) && pstate->attackframe)
			pstate->weaponheight -= GMOVE;
#endif

       pstate->weaponuptics --;
		}
 else if ((pstate->weapon == pstate->new_weapon) && (!pstate->NETCAPTURED))
		pstate->weaponheight = 0;

}


/*
=============================
=
= CheckSpecialSounds
=
=============================
*/

#define REGDOGSTATE(ob) \
   ((ob->state == &s_serialdog) || (ob->state == &s_serialdog2) || \
    (ob->state == &s_serialdog3) || (ob->state == &s_serialdog4)   \
   )

static int dyingvolume=255;
void CheckSpecialSounds(objtype *ob, playertype *pstate)
   {
   int shift;

   if ((!BATTLEMODE) && (ob == player))
      {
      if (pstate->health < MaxHitpointsForCharacter(locplayerstate)/5)
         {
         pstate->healthtime ++;
         if (pstate->healthtime > 2*VBLCOUNTER)
            {
            pstate->healthtime = 0;
            SD_PlayPitchedSound ( SD_PLAYERDYINGSND, dyingvolume, 0 );
            if (dyingvolume>80)
               dyingvolume-=40;
            }
         }
      else
         {
         dyingvolume=255;
         }
      }

   if (((ob->flags & FL_GODMODE) || (ob->flags & FL_DOGMODE)) &&
       (!W_CHANGE(pstate))
      )
      {
      pstate->soundtime++;
      if (pstate->soundtime > (2*VBLCOUNTER))
         {
         int rand;

         rand = GameRandomNumber("player special sound",0);
         shift = (pstate->soundtime>>5);
         if ((rand << shift) > 3500)
            {
            int sound;

            pstate->soundtime = 0;

            rand = GameRandomNumber("player god scare",0);

            if (ob->flags & FL_GODMODE)
               {
               sound = SD_GODMODE1SND;
               if (rand < 160)
                  sound++;
               if (rand < 80)
                  sound ++;
               SD_PlaySoundRTP(sound,ob->x,ob->y);
               }
#if (SHAREWARE == 0)
            else if ((!pstate->batblast) && (!ob->momentumz) &&
                     (REGDOGSTATE(ob))
                    )
               {
               sound = SD_DOGMODEPANTSND;
               if (rand < 128)
                  sound += 2;
               NewState(ob,&s_doglick);
               pstate->attackframe = pstate->weaponframe =
               pstate->batblast = 0;
               SD_PlaySoundRTP(sound,ob->x,ob->y);
               }
#endif
            }
         }
      }




   else if (ob->flags & FL_GASMASK)
      {
      pstate->soundtime ++;

      if (pstate->soundtime > (3*VBLCOUNTER))
         {
         pstate->soundtime = 0;
			SD_PlaySoundRTP(SD_GASMASKSND,ob->x,ob->y);
         }
      }


   else if (MISCVARS->GASON)
      {
      pstate->soundtime ++;
      if (pstate->soundtime > (2*VBLCOUNTER))
         {
         int rand;

         rand = GameRandomNumber("player cough sound",0);
         shift = (pstate->soundtime>>5);
         if ((rand << shift) > 2000)
            {
            pstate->soundtime = 0;
            if ((pstate->player == 1) || (pstate->player == 3))
               SD_PlaySoundRTP(SD_PLAYERCOUGHFSND,ob->x,ob->y);
            else
               SD_PlaySoundRTP(SD_PLAYERCOUGHMSND,ob->x,ob->y);
            }
         }
      }
   }

/*
static int dyingvolume=255;
void CheckSpecialSounds(objtype *ob, playertype *pstate)
   {
   int shift;

   if ((!BATTLEMODE) && (ob == player))
      {
      if (pstate->health < MaxHitpointsForCharacter(locplayerstate)/5)
         {
         pstate->healthtime ++;
         if (pstate->healthtime > 2*VBLCOUNTER)
            {
            pstate->healthtime = 0;
            SD_PlayPitchedSound ( SD_PLAYERDYINGSND, dyingvolume, 0 );
            if (dyingvolume>80)
               dyingvolume-=40;
            }
         }
      else
         {
         dyingvolume=255;
         }
      }

   if (((ob->flags & FL_GODMODE) || (ob->flags & FL_DOGMODE)) &&
       (!W_CHANGE(pstate))
      )
      {
      pstate->soundtime++;
      if (pstate->soundtime > (2*VBLCOUNTER))
         {
         int rand;

         rand = GameRandomNumber("player special sound",0);
         shift = (pstate->soundtime>>5);
         if ((rand << shift) > 3500)
            {
            int sound;

            pstate->soundtime = 0;

            rand = GameRandomNumber("player god scare",0);

            if (ob->flags & FL_GODMODE)
               {
               sound = SD_GODMODE1SND;
               if (rand < 160)
                  sound++;
               if (rand < 80)
                  sound ++;
               SD_PlaySoundRTP(sound,ob->x,ob->y);
               }
#if (SHAREWARE == 0)
            else if (!pstate->batblast)
               {
               sound = SD_DOGMODEPANTSND;
               if (rand < 128)
                  sound += 2;
               NewState(ob,&s_doglick);
               pstate->attackframe = pstate->weaponframe =
               pstate->batblast = 0;
               SD_PlaySoundRTP(sound,ob->x,ob->y);
               }
#endif
            }
         }
      }




   else if (ob->flags & FL_GASMASK)
      {
      pstate->soundtime ++;

      if (pstate->soundtime > (3*VBLCOUNTER))
         {
         pstate->soundtime = 0;
			SD_PlaySoundRTP(SD_GASMASKSND,ob->x,ob->y);
         }
      }


   else if (MISCVARS->GASON)
      {
      pstate->soundtime ++;
      if (pstate->soundtime > (2*VBLCOUNTER))
         {
         int rand;

         rand = GameRandomNumber("player cough sound",0);
         shift = (pstate->soundtime>>5);
         if ((rand << shift) > 2000)
            {
            pstate->soundtime = 0;
            if ((pstate->player == 1) || (pstate->player == 3))
               SD_PlaySoundRTP(SD_PLAYERCOUGHFSND,ob->x,ob->y);
            else
               SD_PlaySoundRTP(SD_PLAYERCOUGHMSND,ob->x,ob->y);
            }
         }
      }
   }

*/


/*
=============================
=
= CheckProtectionsAndPowerups
=
=============================
*/



void CheckProtectionsAndPowerups(objtype *ob, playertype *pstate)
   {
	if (pstate->poweruptime)
		{
		pstate->poweruptime --;
		if (pstate->poweruptime < 0)
			pstate->poweruptime = 0;
		if (ob==player)
			GM_UpdateBonus (pstate->poweruptime, true);
		}
	else
		{
		if (ob->flags & FL_ELASTO)
			{
			ob->flags &= ~FL_NOFRICTION;
			if (ob==player)
				GM_UpdateBonus (pstate->poweruptime, true);
			SD_PlaySoundRTP(SD_LOSEMODESND,ob->x, ob->y);
			}
		else if (ob->flags & FL_GODMODE)
			{ob->temp2 = GODMODEFALL;
          if ((pstate->player == 1) || (pstate->player == 3))
				SD_PlaySoundRTP(SD_GODWOMANSND,ob->x, ob->y);
			 else
				SD_PlaySoundRTP(SD_GODMANSND,ob->x, ob->y);
			 ResetWeapons(ob);
			}
		else if (ob->flags & FL_DOGMODE)
			{int wall;

			 wall = tilemap[ob->tilex][ob->tiley];
			 if ((wall & 0x4000) && (wall & 0x8000))
				{maskedwallobj_t * mw;

				 mw=maskobjlist[wall&0x3ff];
				 if (mw->flags&MW_NONDOGBLOCKING)
					return;
				}


			 ob->temp2 = DOGMODERISE;
          if ((pstate->player == 1) || (pstate->player == 3))
				SD_PlaySoundRTP(SD_DOGWOMANSND,ob->x, ob->y);
			 else
				SD_PlaySoundRTP(SD_DOGMANSND,ob->x, ob->y);
			 ResetWeapons(ob);
			 if (ob->state->condition & SF_DOGSTATE)
            {
            NewState(ob,&s_player);
            pstate->attackframe = pstate->weaponframe = pstate->batblast = 0;
            }
			}
		else if (ob->flags & FL_SHROOMS)
			{
			ResetFocalWidth ();
			SD_PlaySoundRTP(SD_LOSEMODESND,ob->x, ob->y);
			}
		else if (ob->flags & FL_FLEET)
			SD_PlaySoundRTP(SD_LOSEMODESND,ob->x, ob->y);

      ob->flags &= ~(FL_SHROOMS|FL_ELASTO|FL_FLEET|FL_GODMODE|FL_DOGMODE);
		}

   if (pstate->protectiontime)
      {
      pstate->protectiontime --;
      if (pstate->protectiontime <= 0)
         {
         SD_PlaySoundRTP(SD_LOSEMODESND,ob->x, ob->y);
         pstate->protectiontime = 0;
         }
      if (ob==player)
         GM_UpdateBonus (pstate->protectiontime, false);
      }
   else
      ob->flags &= ~(FL_BPV|FL_AV|FL_GASMASK);

   }



/*
=============================
=
= CheckFlying
=
=============================
*/

void CheckFlying(objtype*ob,playertype *pstate)
   {


   if (pstate->buttonstate[bt_lookup] || CYBERLOOKUP)
      {
      ob->momentumz = -FLYINGZMOM;
      }
   if (pstate->buttonstate[bt_lookdown] || CYBERLOOKDOWN)
      {
      ob->momentumz = FLYINGZMOM;
      }

   /*
   if (!M_ISDOOR(((ob->x + costable[ob->angle])>>16),
         ((ob->y - sintable[ob->angle])>>16)) &&
         (pstate->buttonstate[bt_use]) &&
         (!pstate->buttonheld[bt_use])
      )
      {
      int op;
      int dist;

      dist=JETPACKTHRUST;
      if (ob->z-dist<-15)
         dist=ob->z+15;
      if (dist>0)
         {
         op = FixedMul(GRAVITY,(dist<<16)) << 1;
         ob->momentumz = -FixedSqrtHP(op);
         SD_PlaySoundRTP(SD_FLYINGSND,ob->x,ob->y);
         }
      }
   */
   }



/*
=============================
=
= CheckTemp2Codes
=
=============================
*/


#define IN_AIR(ob) \
             (!((ob->z == nominalheight) ||              \
                (IsPlatform(ob->tilex,ob->tiley)) ||    \
                (DiskAt(ob->tilex,ob->tiley))           \
               )                                        \
             )                                          \


void CheckTemp2Codes(objtype *ob,playertype *pstate)
   {
	int pitheight;
	int godheight;
	int dogheight;
	int height;
   int oldz;
   int destheightoffset;


	pitheight    = maxheight - 8;
	height = ob->z + pstate->playerheight;
	dogheight    = ob->z + DOGOFFSET;
	godheight    = ob->z + GODOFFSET;

   //SoftError("\nheightoffset: %d, temp2: %d",pstate->heightoffset,ob->temp2);


   if (!((ob->temp2 == PITFALL) || (ob->temp2 == PITRISE)))
      {

      oldz = ob->z;
      //SoftError("\n zmom %d, oldz %d, newz %d",ob->momentumz,oldz,ob->z);

      if (ob->flags & FL_FLEET)
         {
         if (IN_AIR(ob))
            pstate->heightoffset = FixedMulShift(0x4000,sintable[(oldpolltime<<6)&2047],28);

         ob->z += ((ob->momentumz+0x8000)>>16);
         ob->momentumz = 0;

         }
      else
         {
         ob->z += (ob->momentumz>>16);
         ob->momentumz += GRAVITY;
         }


      if (ob->z >= nominalheight)
         {
         ob->z = nominalheight;
         ob->momentumz = 0;
         if ((oldz < nominalheight) && (!(ob->flags & FL_RIDING)))
            {
            SD_PlaySoundRTP(SD_PLAYERLANDSND,ob->x,ob->y);
            }

         if (!(ob->flags & FL_ELASTO))
            ob->flags &= ~FL_NOFRICTION;
         }
      else if (height < 1)
         {
         ob->momentumz = 0;
			ob->z = 1-pstate->playerheight;
         }



		switch (ob->temp2)
			{case RENORMALIZE:
				 pstate->heightoffset --;

             if (ob->flags & FL_DOGMODE)
                destheightoffset = DOGOFFSET-pstate->playerheight;
             else
                destheightoffset = 0;

             if (pstate->heightoffset <= destheightoffset)
               {
               ob->temp2 = 0;
               pstate->heightoffset = 0;
               }
				 pstate->oldheightoffset = pstate->heightoffset;

				 break;

			 case COLUMNCRUSH:
				 pstate->heightoffset += 4;
				 if (pstate->heightoffset >= 30)
					pstate->heightoffset = 30;

				 pstate->oldheightoffset = pstate->heightoffset;
				 break;

			 case GODMODERISE:
				 pstate->heightoffset --;
				 pstate->oldheightoffset = pstate->heightoffset;
				 if ((height+pstate->heightoffset) <= godheight)
					{
					 ob->temp2 = 0;
					 pstate->poweruptime = POWERUPTICS;
					}
				 break;


			 case GODMODEFALL:
				 pstate->heightoffset ++;
				 pstate->oldheightoffset = pstate->heightoffset;
				 SetPlayerHorizon(pstate,NORMALYZANGLE);
				 if (pstate->heightoffset >= 0)
				  {
					ob->temp2 = 0;
					ob->flags &= ~FL_GODMODE;
					SetNormalHorizon(ob);
				  }
				 break;

			 case DOGMODERISE:
				 pstate->heightoffset --;
				 pstate->oldheightoffset = pstate->heightoffset;
             SetPlayerHorizon(pstate,NORMALYZANGLE);
				 if (pstate->heightoffset <= 0)
					{
					 ob->temp2 = 0;
					 ob->flags &= ~FL_DOGMODE;
					 SetNormalHorizon(ob);
					}
				 break;


			 case DOGMODEFALL:
				 pstate->heightoffset ++;
				 pstate->oldheightoffset = pstate->heightoffset;
             if (pstate->heightoffset >= (DOGOFFSET-pstate->playerheight))
					 {
					 ob->temp2 = 0;
					 pstate->poweruptime = POWERUPTICS;
					 }
				 break;

          case STEPUP:
				 //Debug("\n\n heightoffset adjusted from %d to %d",
					//	 pstate->heightoffset,pstate->heightoffset - STEPADJUST);
				 pstate->heightoffset -= STEPADJUST;
				 if (pstate->heightoffset <= pstate->oldheightoffset)
					{ob->temp2 = 0;
					 //Debug("\n done adjusting heightoffset");
					 pstate->heightoffset = pstate->oldheightoffset;
					}
				 break;

			 case STEPDOWN:
				 pstate->heightoffset += STEPADJUST;
				 if (pstate->heightoffset >= pstate->oldheightoffset)
					{ob->temp2 = 0;
					 pstate->heightoffset = pstate->oldheightoffset;
					}
				 break;

          case 0:
             if (!((ob->flags & FL_FLEET) ||
                   (ob->flags & FL_DOGMODE) ||
                   (ob->flags & FL_GODMODE)
                  )
                )
                pstate->heightoffset = 0;
				 break;

			}


	  }


	else if (ob->temp2 == PITFALL)
		{
		if (ob->z != pitheight)
			{
			ob->z += (ob->momentumz>>16);
			ob->momentumz += GRAVITY;
			if (ob->z >= pitheight)
				{
				ob->z = pitheight;
				ob->momentumz = 0;
            if (!(ob->flags & FL_DYING))
               {DamageThing(ob,10);
                Collision(ob,(objtype*)NULL,0,0);
                M_CheckPlayerKilled(ob);
               }
				}
			}
		else if (ob->momentumx || ob->momentumy)
			{
			ob->temp2 = PITRISE;
			ob->momentumz = -2;
			}
		}

	else if (ob->temp2 == PITRISE)
		{ob->z += ob->momentumz;
		 if (ob->z <= nominalheight)
			 {ob->z = nominalheight;
			  ob->temp2 = 0;
			  ob->momentumz = 0;
           if (pstate->heightoffset)
              ob->temp2 = RENORMALIZE;
			 }
		}






   }



/*
=============================
=
= CheckRemoteRecording
=
=============================
*/

void CheckRemoteRecording(objtype *ob,playertype *pstate)
   {
   if (networkgame==true)
		{
		if ( (pstate->buttonstate[bt_recordsound]) &&
			  (!pstate->buttonheld[bt_recordsound])
			)
			{
			if (SD_RecordingActive()==false)
				{
				SD_SetRecordingActive ();
				PlayerRecording=ob->dirchoosetime;
				if (ob==player)
					{
                   SD_StartRecordingSound();
                   UpdateClientControls();
					}
				}
			}
		else if ( (pstate->buttonheld[bt_recordsound]) &&
					 (!pstate->buttonstate[bt_recordsound])
				  )
			{
			if (SD_RecordingActive()==true)
				{
				if (ob->dirchoosetime==PlayerRecording)
					{
					if (ob==player)
						SD_StopRecordingSound();
					SD_ClearRecordingActive();
					PlayerRecording=-1;
                   UpdateClientControls();
                   }
				}
			}
		}
   }


/*
=============================
=
= CheckPlayerSpecials
=
=============================
*/


void CheckPlayerSpecials(objtype * ob)
   {
	playertype * pstate;

	M_LINKSTATE(ob,pstate);

	// Check for recording of sound

   CheckRemoteRecording(ob,pstate);
   CheckTemp2Codes(ob,pstate);

	if (ob->flags & FL_DYING)
	  return;

   CheckSpecialSounds(ob,pstate);
   CheckProtectionsAndPowerups(ob,pstate);
   }

#if (SHAREWARE == 0)
/*
===============
=
= T_DogUse
=
===============
*/

void  T_DogUse (objtype *ob)
{
  attack_t   *cur;
  playertype *pstate;
  statetype *oldstate;

  M_LINKSTATE(ob,pstate);

  Thrust(ob);
  oldstate = ob->state;
  CheckPlayerSpecials(ob);
  if (ob->state != oldstate)
    {
    return;
    }




//Commented out until we find if it's valid
/*
  if (!ob->ticcount)
	{if ( pstate->buttonstate[bt_use] && !pstate->buttonheld[bt_use] )
		pstate->buttonstate[bt_use] = false;
	}
*/

   if (pstate->attackframe >= DOGSCRATCH.numattacks)
      Error("\n attackframe %d for DOGSCRATCH gt numattacks %d",
            pstate->attackframe,DOGSCRATCH.numattacks
           );



  cur = &(DOGSCRATCH.attackinfo[pstate->attackframe]);
  if (! pstate->attackcount)
	 {switch (cur->attack)
		 {case reset:
			  {
           NewState(ob,&s_dogwait);
           pstate->attackframe = pstate->weaponframe = pstate->batblast = 0;
			  return;
			  }
			  break;
		  case at_pulltrigger:
			 pstate->buttonheld[bt_use]=false;
          Cmd_Use(ob);
			 break;
                  default:
                         break;
		 }
	  pstate->attackframe++;
     cur = &(DOGSCRATCH.attackinfo[pstate->attackframe]);
     pstate->weaponframe = cur->frame;
	  pstate->attackcount = cur->mtics;

	 }
  else
	  pstate->attackcount --;



}

/*
===============
=
= T_DogLick
=
===============
*/



void  T_DogLick (objtype *ob)
{
  attack_t   *cur;
  playertype *pstate;
  statetype *oldstate;

  M_LINKSTATE(ob,pstate);

  Thrust(ob);
  oldstate = ob->state;
  CheckPlayerSpecials(ob);
  if (ob->state != oldstate)
    {
    return;
    }



//Commented out until we find if it's valid
/*
  if (!ob->ticcount)
	{if ( pstate->buttonstate[bt_use] && !pstate->buttonheld[bt_use] )
		pstate->buttonstate[bt_use] = false;
	}
*/


   if (pstate->attackframe >= DOGLICK.numattacks)
      Error("\n attackframe %d for DOGLICK gt numattacks %d",
            pstate->attackframe,DOGLICK.numattacks
           );


  cur = &(DOGLICK.attackinfo[pstate->attackframe]);
  if (! pstate->attackcount)
    {if (cur->attack == reset)
        {
        NewState(ob,&s_serialdog);
        pstate->attackframe = pstate->weaponframe = 0;
        return;
        }
	  pstate->attackframe++;
	  cur = &(DOGLICK.attackinfo[pstate->attackframe]);
	  pstate->weaponframe = cur->frame;
	  pstate->attackcount = cur->mtics;

	 }
  else
	  pstate->attackcount --;

 //	if ( playerstate.buttonstate[bt_attack] && (!playerstate.buttonheld[bt_attack]) && (!W_CHANGE))
  //		Cmd_Fire (ob);


}

#endif

void T_DeadWait(objtype*ob)
{
	playertype *pstate;

   M_LINKSTATE(ob,pstate);
   if ((ob->flags & FL_DESIGNATED) && (gamestate.battlemode == battle_CaptureTheTriad))
     {int otherteam = (pstate->team ^ 1);

      ob->flags &= ~FL_DESIGNATED;
      UpdateKills = true;
      SpawnStatic(TEAM[otherteam].tilex,TEAM[otherteam].tiley,stat_collector,9);
		LASTSTAT->flags |= FL_COLORED;
      LASTSTAT->hitpoints = otherteam;
      LASTSTAT->flags |= FL_ABP;
      MakeStatActive(LASTSTAT);

	  }
   //ob->momentumx=0;
   //ob->momentumy=0;

	if (pstate->heightoffset<36)
		{pstate->heightoffset++;
		 pstate->oldheightoffset = pstate->heightoffset;
		}

   if (ob==player)
      {
      UpdateLightLevel(player->areanumber);
      if ((pstate->falling==true) || (ob->momentumz==0))
         {
         if (BATTLEMODE)
				playerdead=true;
         else
			   playstate = ex_died;
         }
		}
	/*
   if (BATTLEMODE)
		{
		objtype * killer=(objtype *)ob->target;

		dx = killer->x - ob->x;
		dy = ob->y - killer->y;

		if (dx && dy)
			ob->angle = atan2_appx (dx,dy);
		}
	*/
	//CheckPlayerSpecials(ob);
}

/*
===============
=
= T_Player
=
===============
*/

void  T_Player (objtype *ob)
{
  playertype *pstate;
  statetype *oldstate;

#if (SHAREWARE == 0)
  int eluder;
#endif

  M_LINKSTATE(ob,pstate);



#if (SHAREWARE == 0)
  eluder = ((pstate->weapon == wp_dog) && gamestate.SpawnEluder);
#endif

  oldstate = ob->state;

  if (ob->flags&FL_DYING)
	  {

      CheckPlayerSpecials(ob);
      PlayerMove(ob);
      if (
          (pstate->falling==true) ||
          (
           (!ob->momentumx) &&
           (!ob->momentumy) &&
           (!ob->momentumz) &&
           (!ob->state->tictime)
          )
         )
		  {
			KillActor(ob);
			if (ob->state == &s_remoteguts12)
           NewState(ob,&s_gutwait);
			else
			  NewState(ob,&s_deadwait);
		  }
		return;
	  }

  Thrust(ob);
  if (ob->flags & FL_DYING)
    return;

  oldstate = ob->state;
  CheckPlayerSpecials(ob);
  if (ob->state != oldstate)
     {
     if (ob->flags & FL_DYING)
        return;
     }

  if (!(ob->flags & FL_PAIN))
	 {if (!(ob->state->condition & SF_DOGSTATE))
		 {if ((ob->momentumx || ob->momentumy) && (ob->state->condition != SF_DOWN))
			  NewState(ob,&s_remotemove1);
		  else if (NOMOM && (ob->state != &s_player))
			  NewState(ob,&s_player);
		 }
     else if (NOMOM
#if (SHAREWARE == 0)

     && (ob->state != &s_dogwait)
#endif

     )

         NewState(ob,ob->state);

	 }
  else if (!ob->ticcount)
    {if (!(ob->state->condition & SF_DOGSTATE))
		  NewState(ob,&s_player);

	  ob->flags &= ~FL_PAIN;

	 }

  if (ob->flags & FL_DIDTAG)
	{ob->flags &= ~FL_DIDTAG;
	 pstate->weapondowntics = (144 - TAGHANDHEIGHT)/GMOVE;
	}


  CheckWeaponStates(ob);


  if (ARMED(ob->dirchoosetime)
     //(gamestate.battlemode != battle_Tag)
     )
     {
     if (pstate->weapondowntics == 1)  // change to up; during change, up and down
									  // are never zero at the same time
        {
        pstate->weapondowntics = 0;
		  pstate->weaponframe = pstate->attackframe = 0;

		  if (pstate->NETCAPTURED == -1)
           {
			  pstate->weaponuptics = FREE.screenheight/GMOVE;
			  pstate->weaponheight = pstate->weaponuptics*GMOVE ;
			  pstate->NETCAPTURED = 1;
  //          return;
           }
		  else if (pstate->NETCAPTURED == -2)
           {
			  pstate->weaponuptics = WEAPONS[pstate->weapon].screenheight/GMOVE;
			  pstate->weaponheight = pstate->weaponuptics*GMOVE ;
			  pstate->NETCAPTURED = 0;

			  //return;
           }
		  else
           {
           pstate->weaponuptics = WEAPONS[pstate->new_weapon].screenheight/GMOVE;
           pstate->weapon = pstate->new_weapon;

           pstate->weaponheight = pstate->weaponuptics*GMOVE ;
			  }
        }

     else
        CheckWeaponChange (ob);
     }

  else if (gamestate.battlemode == battle_Hunter)
     {
     if (pstate->weapondowntics == 1)

        {
        pstate->weapondowntics = 0;
        pstate->weaponframe = pstate->attackframe = pstate->batblast = 0;
        pstate->weapon = pstate->new_weapon;
        }
     }

//   if ( pstate->buttonstate[bt_use] && (!W_CHANGE(pstate)) )

   if ( pstate->buttonstate[bt_use] )
      Cmd_Use (ob);


	if (W_CHANGE(pstate))
	  return;




   if ((!ARMED(ob->dirchoosetime))
#if (SHAREWARE == 0)
        && (pstate->weapon != wp_dog)
#endif
        )
		return;

	if (pstate->buttonstate[bt_attack])
      {
#if (SHAREWARE == 0)

      if (eluder)
			Cmd_Fire(ob);

      else if ((pstate->weapon == wp_bat) ||
               (pstate->weapon == wp_dog)
              )
         {
         int oldblast=pstate->batblast;

         pstate->batblast ++;

         if (pstate->weapon==wp_bat)
            {
            if (pstate->batblast == 20)
               SD_PlaySoundRTP(SD_EXCALIBUILDSND,ob->x,ob->y);
            }
         else
            {
            if ((pstate->batblast>>4)!=(oldblast>>4))
               {
               int handle;

               handle=SD_PlaySoundRTP(SD_DOGMODEPREPBLASTSND,ob->x,ob->y);
               SD_SetSoundPitch(handle,-(BBTIME<<3)+(pstate->batblast<<3));
               }
            }
         if (pstate->batblast < BBTIME)
			  return;

         if (pstate->weapon == wp_bat)
				SD_PlaySoundRTP(SD_EXCALIBLASTSND,ob->x,ob->y);
         else
            SD_PlaySoundRTP(SD_DOGMODEBLASTSND,ob->x,ob->y);
         }

      if ((pstate->weapon != wp_split) || (!pstate->buttonheld[bt_attack]))
#endif
  //    if (!pstate->buttonheld[bt_attack])
//#endif
         {

#if (SHAREWARE == 0)

         if (pstate->weapon == wp_kes)
			  SD_PlaySoundRTP(SD_GRAVBUILDSND,ob->x,ob->y);
#endif

			Cmd_Fire (ob);
         }
      }

#if (SHAREWARE == 0)

   else if (
             ((pstate->weapon == wp_bat) ||
              ((pstate->weapon == wp_dog) && (!eluder))
             ) &&
             (pstate->buttonheld[bt_attack])
           )
      {
      if (pstate->weapon == wp_bat)
         SD_StopSound(SD_EXCALIBUILDSND);
      pstate->batblast = 0;
      Cmd_Fire(ob);
      }
#endif

}



