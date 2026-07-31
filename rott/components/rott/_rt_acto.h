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
#ifndef _rt_actor_private
#define _rt_actor_private

#define MAXGIBS            600
#define HAAPT              24
#define VAAPT              24
#define GODHAPT            1024
#define GODVAPT            1024
#define MAXDELTAYZSHOOT    (((5*FINEANGLES)/360))
#define MAXDELTAYZSEE      (((15*FINEANGLES)/360))
#define MAXSHOOTOFFSET     (((15*FINEANGLES)/360))
#define MAXSHOOTSHIFT      (1)
#define MAXDAMAGE          (64)
#define MAXYZANGLE         (((30*FINEANGLES)/360))
#define SMOKEWALLOFFSET    (0x800)
#define MZADJUST           0x30000;
#define MAXSTEPHEIGHT      24
#define SIGN(x)            ((x)>=0)?(1):(-1)
#define MAXRAIN            128
#define SG_PSTAT      0x4000
#define SG_PSTATE     0x2000
#define EXPLOSION_DAMAGE   50
#define LOWFALLCLIPZ       (maxheight - 96)
#define HIGHFALLCLIPZ      -5
#define LOWRISECLIPZ       (nominalheight)
#define HIGHRISECLIPZ      64
#define NORMALGIBSPEED     0x2f00

#define FL_PLEADING        0x400
#define FL_EYEBALL         0x400
#define FL_UNDEAD          0x8000

#define NME_DRUNKTYPE       0x01
#define NME_HEATSEEKINGTYPE 0x02

#define NUMSTATES 11

enum {
  STAND,
  PATH,
  COLLIDE1,
  COLLIDE2,
  CHASE,
  USE,
  AIM,
  DIE,
  FIRE,
  WAIT,
  CRUSH
  };

#define SHOTMOM                0x200l
#define NEXT                   1
#define PREV                   0
#define ANGLEMOVE              0x2b000l
#define PAINTIME               5l
#define LOOKAHEAD              (20 << 16)
#define DRAINTIME              70l
#define EXPLOSION_IMPULSE      0x2600l
#define ZEROMOM                ob->momentumx = ob->momentumy = 0
#define NOMOM                  ((!ob->momentumx) && (!ob->momentumy))
#define WHICHACTOR             (ob->obclass-lowguardobj)
#define SPDPATROL              0x600l
//#define ENEMYRUNSPEED          (3*SPDPATROL)
#define ENEMYRUNSPEED          (0xc00)
#define ENEMYFASTRUNSPEED      (5*SPDPATROL)
#define ENEMYINSANESPEED       (7*SPDPATROL)
#define MAXMOVE                0x2000l
#define PROJECTILESIZE         0x6000l
#define DEADFRICTION           0x6000l
#define ROLLMOMENTUM           0x920l
#define PROJSIZE               0x4000l
#define PILLARMOM              0x800l
#define HALFGLOBAL1            (TILEGLOBAL/2)
#define TOUCHDIST              0xb000l
#define STANDDIST              0x5000l
#define SNAKERAD               0x4000l
#define MINSIGHT               0x18000l
#define HBM                    -2
#define SNEAKY                 -3
#define GIBVALUE               -3
#define DISKMOMZ               4

//=========================== macros =============================

#define M_ISWALL(x)           ((x->which == WALL) || (x->which == PWALL) || (x->which == MWALL))
#define M_DISTOK(p1,p2,d)     (abs((p1)-(p2)) <= d)
#define M_NONS(x)             ((x->obclass == wallfireobj) || (x->obclass == pillarobj))
#define M_CHOOSETIME(x)       ((int)(TILEGLOBAL/((x->speed))))
#define M_DIST(x1,x2,y1,y2)   (((x1-x2)*(x1-x2))+((y1-y2)*(y1-y2)))
#define M_S(x)                (UPDATE_STATES[x][ob->obclass-lowguardobj])
#define Fix(a)                (a &= (FINEANGLES - 1))


#define STOPACTOR(ob)                   \
   {                                    \
   ob->momentumx = ob->momentumy = 0;   \
   ob->dirchoosetime = 0;               \
   }


#define M_CHECKDIR(ob,tdir)             \
   {                                    \
   ob->dir = tdir;                      \
   ParseMomentum(ob,dirangle8[tdir]);   \
   ActorMovement(ob);                   \
   if (ob->momentumx || ob->momentumy)  \
      return;                           \
   }                                    \


#define M_CHECKTURN(x,ndir)                             \
   {                                                    \
   if (ndir == olddir)                                  \
      ZEROMOM;                                          \
   ParseMomentum(x,dirangle8[ndir]);                    \
   ActorMovement(x);                                    \
   if (!NOMOM)                                          \
      {                                                 \
      if (ndir != olddir)                               \
         {                                              \
         next = dirorder[olddir][NEXT];                 \
         prev = dirorder[olddir][PREV];                 \
         x->temp1 = ndir;                               \
         if (dirdiff[ndir][next] < dirdiff[ndir][prev]) \
            NewState(x,&s_kristleft);                   \
         else                                           \
            NewState(x,&s_kristright);                  \
         }                                              \
      return;                                           \
      }                                                 \
   }                                                    \

#define M_CheckDoor(ob)                          \
   {                                             \
   door = ob->door_to_open;                      \
   if (door != -1)                               \
      {                                          \
      if ((ob->obclass > shurikenobj) &&         \
          (ob->obclass != collectorobj)          \
         )                                       \
         Error("you got it !!!");                \
      LinkedOpenDoor(door);                      \
      if (doorobjlist[door]->action != dr_open)  \
         return;                                 \
      ob->door_to_open = -1;                     \
      }                                          \
   }                                             \

#define M_CheckBossSounds(ob)                                 \
   {                                                          \
   if ((ob->obclass >= b_darianobj) &&                        \
       (ob->obclass <= b_darksnakeobj) &&                     \
       (ob->flags & FL_ATTACKMODE)  &&                        \
       (ob->obclass != b_robobossobj) &&                      \
       (!(ob->flags & FL_DYING))                              \
      )                                                       \
      {                                                       \
      if (MISCVARS->SOUNDTIME)                                \
         MISCVARS->SOUNDTIME --;                              \
      else                                                    \
         {                                                    \
         MISCVARS->SOUNDTIME = 5*VBLCOUNTER;                  \
         if (GameRandomNumber("boss sound check",0)<160)      \
            {                                                 \
            int rand,sound;                                   \
                                                              \
            rand = GameRandomNumber("boss sounds",0);         \
            sound = BAS[ob->obclass].operate;                 \
            if (rand < 160)                                   \
               sound ++;                                      \
            if (rand < 80)                                    \
               sound ++;                                      \
                                                              \
            SD_PlaySoundRTP(sound,ob->x,ob->y);               \
            }                                                 \
         }                                                    \
      if (MISCVARS->REDTIME)                                  \
         {                                                    \
         MISCVARS->REDTIME --;                                \
         MISCVARS->redindex = ((MISCVARS->REDTIME >> 1) & 15);\
         }                                                    \
      }                                                       \
   }




#define SET_DEATH_SHAPEOFFSET(ob)                     \
   {                                                  \
   ob->flags |= FL_ALTERNATE;                         \
   ob->shapeoffset += deathshapeoffset[ob->obclass];  \
   }


#define RESET_DEATH_SHAPEOFFSET(ob)                   \
   {                                                  \
   ob->flags &= ~FL_ALTERNATE;                        \
   ob->shapeoffset -= deathshapeoffset[ob->obclass];  \
   }

#define LOW_VIOLENCE_DEATH_SHOULD_BE_SET(ob)          \
     ((gamestate.violence < vl_high) &&               \
      (ob->obclass >= lowguardobj) &&                 \
      (ob->obclass <= triadenforcerobj) &&            \
      (!(ob->flags & FL_ALTERNATE))                   \
     )                                                \

#define LOW_VIOLENCE_DEATH_IS_SET(ob)   (ob->flags & FL_ALTERNATE)

#define LOW_VIOLENCE_PAIN_SHOULD_BE_SET  LOW_VIOLENCE_DEATH_SHOULD_BE_SET

#define LOW_VIOLENCE_PAIN_IS_SET  LOW_VIOLENCE_DEATH_IS_SET

#define SET_PAIN_SHAPEOFFSET  SET_DEATH_SHAPEOFFSET

#define RESET_PAIN_SHAPEOFFSET  RESET_DEATH_SHAPEOFFSET


// default = actor

typedef struct  sat
 { int          x,y,z;
	unsigned     flags;
	int          hitpoints;
	int          targetx,targety;
	int          angle;
	int          yzangle;
	int          speed;
	int          momentumx,momentumy,momentumz;
	int          temp1,temp2,temp3;
	int          whateverindex,targetindex;

	short        ticcount;
	short        shapeoffset;
	short        stateindex;
	short        dirchoosetime;

	byte         areanumber;
	byte         obclass;
	signed char  door_to_open;
	signed char  dir;

 }saved_actor_type;


typedef struct
 {thingtype which;
  byte tilex,tiley;
  fixed x,y,z;
 }tpoint;


//========================== Function Prototypes ==============================

void     MissileMovement(objtype*);
boolean  MissileTryMove(objtype*,int,int,int);
void     T_DarkSnakeChase(objtype*);
void     HeatSeek(objtype*);
boolean  CheckDoor(objtype *ob,doorobj_t*,int,int);
boolean  NextToDoor(objtype*ob);
void     MissileHit (objtype *ob,void*);
int      Near(objtype*,void*,int);
void     FirstSighting(objtype*);
void     SelectOrobotChaseDir(objtype*);
void     SelectPathDir(objtype*);
void     SelectChaseDir(objtype*);
void     SelectRoboChaseDir(objtype*);
void     SelectDodgeDir(objtype*);
void     SelectRollDir (objtype*);
void     SelectTouchDir(objtype*);
void     SelectMineDir(objtype*);
boolean  WallCheck(int,int);
boolean  NMEspincheck(objtype*);
void     TurnActorIntoSprite(objtype*ob);
void     ActivateEnemy(objtype*);
#endif
