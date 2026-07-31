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
#ifndef _rt_actor_public
#define _rt_actor_public

//***************************************************************************
//
//                            RT_ACTOR.C (actor functions)
//
//***************************************************************************

#include "states.h"


#define FL_SYNCED          0x400
#define FL_MASTER          0x800
#define FL_SKELETON        0x02
#define GASTICS            1050


#define M_ISACTOR(x)     ((x!=NULL) && (x->which == ACTOR))

#define SF_GUTS          0x20

#define NUMENEMIES       16
#define NUMCLASSES       51
#define MAXTOUCH         10
#define MAXPOP           32
#define MAXDELETE        512

#define InMapBounds(x,y) (((x) >= 0) && ((x) < MAPSIZE) && ((y) >= 0) && ((y) < MAPSIZE))


#define ACTIVE(ob)       ((ob == firstactive) || (ob->prevactive) || (ob->nextactive))

enum {SHOOT,SIGHT,DIRCHECK,EXPLOSION,MISSILE};


typedef enum
   {
   gt_sparks,
   gt_organ,
   gt_rib,
   gt_pinkorgan,
   gt_head,
   gt_arm,
   gt_leg,
   gt_humerus,
   gt_pelvis,
   gt_limb,
   gt_bsoul,
   gt_lsoul,
   gt_spit,
   NumGibTypes
   }gib_t;

#define GUTS         (NumGibTypes)
#define RANDOM       (NumGibTypes + 1)
#define DISEMBOWEL   (NumGibTypes + 2)


#define GASVALUE               192

#define PAINOFFSET             2
#define PMFOFFSET              50
#define REMOTEOFFSET           86

#define ACTORSIZE              0x5800
#define PWALLRAD               0xa000

#ifdef __WATCOMC__
int M_ABS(int value);
#pragma aux M_ABS =  \
		  "test eax,eax",                     \
		  "jge done", \
		  "neg eax",\
		  "done: "         \
		  parm    [eax] \
		  value   [eax]           \
		  modify exact [eax]
#else
#define M_ABS abs
#endif

#define M_CheckPlayerKilled(ob) \
{ if ((ob->obclass == playerobj) && (ob->flags & FL_DYING)) \
	 BATTLE_CheckGameStatus(battle_player_killed,ob->dirchoosetime);\
}



#define SetTilePosition(ob, newtilex, newtiley)    \
  {                                                \
  ob->tilex = newtilex;                            \
  ob->tiley = newtiley;                            \
  ob->x = (ob->tilex << TILESHIFT) + TILEGLOBAL/2; \
  ob->y = (ob->tiley << TILESHIFT) + TILEGLOBAL/2; \
  }


#define SetFinePosition(ob, newx, newy)            \
  {                                                \
  ob->x = newx;                                    \
  ob->y = newy;                                    \
  ob->tilex = (ob->x >> TILESHIFT);                \
  ob->tiley = (ob->y >> TILESHIFT);                \
  }

#define SetVisiblePosition(ob, x, y)               \
  {                                                \
  ob->drawx = x;                                   \
  ob->drawy = y;                                   \
  }


//***************************************************************************
//
//    WeaponDamages
//
//***************************************************************************
#define DMG_PISTOL    13
#define DMG_MP40      10
#define DMG_AHGUN     10
#define DMG_ENEMYBULLETWEAPON     10




typedef struct bas
 {
  short operate,
        see,
        fire,
        hit,
        die;
 } basic_actor_sounds;

extern basic_actor_sounds  *BAS;


typedef struct
 {int x,y;
 }_2Dpoint;

typedef enum {
		  inertobj,
		  playerobj,
		  lowguardobj,
		  highguardobj,
		  overpatrolobj,
		  strikeguardobj,
		  blitzguardobj,
		  triadenforcerobj,
		  deathmonkobj,
		  dfiremonkobj,
		  roboguardobj,

		  b_darianobj,
		  b_heinrichobj,
		  b_robobossobj,
		  b_darkmonkobj,
		  b_darksnakeobj,
		  patrolgunobj,
		  wallopobj,


		  //specials

		  pillarobj,
		  firejetobj,
		  bladeobj,
		  crushcolobj,
		  boulderobj,
		  spearobj,
		  gasgrateobj,
		  springobj,

		  // Missile weapon types

		  shurikenobj,
		  wallfireobj,
		  netobj,
		  h_mineobj,
		  grenadeobj,
		  fireballobj,
		  dmfballobj,
		  bigshurikenobj,
		  missileobj,
		  NMEsaucerobj,
		  dm_weaponobj,
		  dm_heatseekobj,
		  dm_spitobj,
		  p_bazookaobj,
		  p_firebombobj,
		  p_heatseekobj,
		  p_drunkmissileobj,
		  p_firewallobj,
		  p_splitmissileobj,
		  p_kesobj,
		  p_godballobj,
		  collectorobj,
        diskobj,
        rainobj


		  }classtype;


typedef struct objstruct
{
		  thingtype                     which;
		  byte                     tilex,tiley;
		  fixed                    x,y,z;
        int                      shapenum;
		  unsigned                 flags;
		  short                    ticcount;
		  signed short             hitpoints;
		  word                     whichactor;

		  signed short             dirchoosetime;
		  fixed                    drawx,drawy;
		  classtype                obclass;
		  statetype *              state;
		  signed char              door_to_open;
		  byte                     areanumber;
		  signed short             shapeoffset;
		  int                      targettilex,targettiley;


		  dirtype                  dir;
		  short int                angle;
		  short int                yzangle;

		  int                      soundhandle;
		  int                      speed;
		  int                      momentumx, momentumy, momentumz;
		  int                      temp1,temp2,temp3;
		  void                     *whatever,*target;
		  struct objstruct         *next, *prev;
		  struct objstruct         *nextactive, *prevactive;
		  struct objstruct         *nextinarea, *previnarea;

} objtype;




typedef struct b_struct
 {int   NMErotate;
  int   NMEdirstried;
  int   NMEqueuesize;
  int   ESAU_HIDING,ESAU_SHOOTING;
  int   HRAMMING, HMINING;
  int   SOUNDTIME;
  int   redindex,REDTIME;
  int   monkz;
  int   monkup;
  int   firespawned;
  int   notouch,noholes;
  int   nexttouch,nextpop;
  int   popsleft;
  int   DSTATE;
  int   doorcount;
  int   KEYACTORSLEFT;
  int      GASON;
  int      gasindex;
  boolean  NET_IN_FLIGHT;
  boolean  madenoise;
  _2Dpoint ETOUCH[MAXTOUCH],EPOP[MAXPOP],TOMLOC;
  int   NUMWEAPONS;
  int   BulletHoleNum;
  int   NUMBEGGINGKEVINS;
  boolean fulllightgibs;
  boolean directgibs;
  int     gibgravity;
  int     gibspeed;
  boolean supergibflag;
  boolean randgibspeed;
  int     numgibs;
  boolean elevatormusicon;
 }misc_stuff;


extern  boolean          ludicrousgibs;
extern  objtype*         PLAYER0MISSILE;
extern  byte             *deathshapeoffset;
extern  byte             RANDOMACTORTYPE[100];
extern  objtype*         FIRSTACTOR,*LASTACTOR;
extern  objtype          *FIRSTRAIN,*LASTRAIN;
extern  objtype*         SCREENEYE;
extern  objtype          **firstareaactor,**lastareaactor;
extern  misc_stuff       mstruct,*MISCVARS;
extern  int              actorstart,actorstop;
extern  exit_t           playstate;
extern  objtype          *lastactive,*firstactive;
extern  objtype          *new,**objlist,
								 *killerobj;
extern  void             *(*actorat)[MAPSIZE];
extern  int              *angletodir;
extern _2Dpoint          *SNAKEPATH;
/* extern  int              STOPSPEED; */
extern  int              FRICTION;

extern  int              objcount;

void     SpawnInertActor(int,int,int);
objtype* DiskAt(int tilex,int tiley);
void     GetRainActors(void);
void     DoRain(void);
void     SpawnDisk(int,int,int,boolean);
void     T_ElevDisk(objtype*);
void     Add_To_Delete_Array(void*);
void     Remove_Delete_Array_Entries(void);
void     MakeInactive(objtype*ob);
void     RespawnEluder(void);
void     SpawnCollector(int,int);
void     MakeLastInArea(objtype*);
void     RemoveFromArea(objtype*);
void     GetMomenta(objtype*,objtype*,int*,int*,int*,int);
int      AngleBetween(objtype*,objtype*);
void     TurnActorIntoSprite(objtype*);
void     ResolveDoorSpace(int,int);
void     RemoveObj(objtype*);
void     SpawnParticles(objtype*,int,int);
void     MakeActive(objtype*);
void     SpawnSpear(int,int,int);
void     SpawnBoulder(int,int,int);
void     SpawnCrushingColumn(int,int,int);
void     SpawnFirejet(int,int,int,int);
void     T_Firethink(objtype*);
void     SpawnBlade(int,int,int,int,int);
void     T_OrobotChase(objtype*);
void     SpawnMultiSpriteActor(classtype,int,int,int);
boolean  ActorTryMove(objtype*,int,int,int);
void     A_Repeat(objtype*);
void     T_Heinrich_Defend(objtype*);
void     T_Heinrich_Out_of_Control(objtype*);
void     A_HeinrichShoot(objtype*);
void     T_EsauWait(objtype*);
void     T_EsauRise(objtype*);
void     A_Drain(objtype*ob);
void     T_Explosion(objtype*ob);
void     T_MoveColumn(objtype*);
void     EnableObject(long object);
void     DisableObject(long object);

void     T_Collide(objtype*);
void  Collision(objtype*ob,objtype *attacker,int hitmomentumx,int hitmomentumy);
void     ActorMovement (objtype *);
void     MoveActor(objtype*);

void     InitActorList(void);
void     NewState(objtype*,statetype*);
void     DoActor(objtype*);

void     SpawnPushColumn(int tilex,int tiley,int which,int dir, int linked);
void     SpawnGunThingy(classtype which, int tilex, int tiley, int dir);
void     SpawnStand (classtype which, int tilex, int tiley, int dir, int ambush);
void     SpawnPatrol (classtype which, int tilex, int tiley, int dir);
void     SpawnDeadGuard (int tilex, int tiley);
void     SpawnWallfire(int tilex, int tiley, int dir);
void     SpawnMissile(objtype*,classtype,int,int,statetype*,int);

void     InitHitRect (objtype *ob, unsigned radius);
void     NewState (objtype *ob, statetype *state);
void     SelectPathDir(objtype*);
void     SelectChaseDir (objtype *ob);
boolean  SightPlayer (objtype *ob);
boolean  CheckLine (void*,void *,int);
boolean  CheckSight (objtype *ob,void*);
void     KillActor (objtype *ob);
void     DamageThing (void *, int);
void     MissileHit (objtype *,void*);
void     GetNewActor(void);

void     T_WallPath(objtype*);
void     T_Path (objtype *ob);   //done
void     T_RoboChase(objtype*ob);
void     T_RoboPath(objtype*ob);
void     T_Chase (objtype *ob);      //done
void     T_EsauChase(objtype*ob); //done
void     T_Spears(objtype*);
void     T_Projectile (objtype *ob); //done
void     T_Stand (objtype *ob);      //done
void     T_GunStand(objtype *ob); //done
void     T_Use(objtype *ob);         // done
void     A_Shoot (objtype *ob);        // done
void     A_MissileWeapon(objtype*);    // done
void     A_Wallfire(objtype*);

void     A_Steal(objtype*);

void     T_Roll(objtype*);
void     T_BossDied (objtype *ob);
boolean  QuickSpaceCheck(objtype*,int,int);
void     PushWallMove(int num);
void     SpawnNewObj(unsigned,unsigned,statetype*,classtype);
void     SpawnSpring(int,int);
void     SpawnFourWayGun(int,int);
void     SpawnSnake(int tilex,int tiley);
void     SpawnSneaky(int,int);
boolean  MissileTryMove(objtype*ob,int,int,int);

void     SaveActors(byte ** buf, int * size);
void     LoadActors(byte * buf, int size);

boolean  TurnOffLight0 (int tilex, int tiley);
boolean  TurnOffLight1 (int tilex, int tiley, int i, int j);
boolean  TurnOffLight2 (int tilex, int tiley, int j);
boolean  TurnOffLight3 (int tilex, int tiley, int i);

void     ParseMomentum(objtype *ob,int angle);
void     SpawnGroundExplosion(int x, int y, int z);
void     RayShoot (objtype * shooter, int damage, int accuracy);
void FindEmptyTile(int *stilex, int *stiley);
void T_Wind( objtype *ob );
void StopWind( void );

#endif
