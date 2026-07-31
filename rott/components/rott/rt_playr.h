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
//   RT_PLAYR.C - Player movement and stuff
//
//***************************************************************************

#ifndef _rt_playr_public
#define _rt_playr_public

#include "rt_actor.h"
#include "rt_stat.h"
#include "states.h"
#include "rottnet.h"
#include "rt_battl.h"
#include "develop.h"



extern int mouse_ry_input_scale;

#define LOW_GRAVITY    10000
#define NORMAL_GRAVITY 30000
#define HIGH_GRAVITY   250000
#define TAGHANDHEIGHT  40

#define MAXDEAD        32
#define MAXBULLETS     64

#define MAXCODENAMELENGTH 9

#define ARMED(x)   ( \
                     (gamestate.PlayerHasGun[x]) || \
                     (!BATTLEMODE) \
                   )


typedef struct
    {
    statetype *state;
    int       speed;
    classtype obclass;
    int       offset;
    int       flags;

    }missile_stats;

extern missile_stats PlayerMissileData[13];

extern int GetWeaponForItem(int itemnumber);
extern int GetItemForWeapon(int weapon);


typedef enum
  {reset,
	reset2,
	done,
	at_knife,
	at_pulltrigger,
	at_automatic,
	at_dryheaving,
	at_missileweapon
  } attack_action;


typedef struct atkinf
{
	attack_action attack;
	char  mtics,frame;      // attack is 1 for gun, 2 for knife, 3 for machine guns,
											 // 4 for chaingun (orig. game)
} attack_t;

typedef struct weaptype
{const int screenheight;
 int startammo;
 int base_damage;
 int impulse;
 int numattacks;
 attack_t attackinfo[14];
}williamdidthis;

//
// Interactive input status of device, returned by SWIFT_Get3DStatus
//
typedef struct {
   short	x;
   short	y;
   short	z;
   short	pitch;
   short	roll;
   short	yaw;
   short	buttons;
} SWIFT_3DStatus;


//
// Static data about device, returned by SWIFT_GetStaticDeviceInfo
//
typedef struct
{
   unsigned char	deviceType;
   unsigned char	majorVersion;
	unsigned char	minorVersion;
   unsigned char	coordDescriptor[6];
   unsigned char	reserved[1];
} SWIFT_StaticData;

typedef struct
{
        int                lives;
        int                health;
        int                triads;
		  signed char        ammo;
		  short              poweruptime;
		  short              protectiontime;
        int                new_weapon;
		  int                keys;
        int                weapon;
        int                missileweapon;
        int                bulletweapon;
		  signed char        HASBULLETWEAPON[3];
        int                attackframe,attackcount,weaponframe;
        byte               player;
        int                topspeed;
        int                dmomx;
        int                dmomy;
		  int                angle;
		  int                anglefrac;
        boolean            buttonheld[NUMBUTTONS];
        boolean            buttonstate[NUMBUTTONS];
        int                horizon;
        int                lastmomz;
		  signed short       playerheight;
		  signed short       heightoffset;
		  signed short       weaponheight;
		  byte               weaponx,weapony;
		  word               batblast;
		  signed char        NETCAPTURED;
		  signed char        HASKNIFE;
        int                oldweapon, oldmissileweapon;
		  signed short       weapondowntics,weaponuptics;
		  int                soundtime;
        int                healthtime;
        objtype*           guntarget;
		  int                targettime;
//        int                steptime;
//        int                stepwhich;
        boolean            falling;
		  int                oldshapeoffset;
		  int                uniformcolor;
        char               codename[MAXCODENAMELENGTH];
		  int                oldheightoffset;
        int                team;
} playertype;


typedef struct
{ int topspeed;
  int toprunspeed;
  int hitpoints;
  int accuracy;
  int height;
} ROTTCHARS;


#define M_LINKSTATE(x,y) \
{ y = &PLAYERSTATE[x->dirchoosetime];\
}


extern williamdidthis  DOGSCRATCH;
extern int        GRAVITY;
extern int        controlupdatetime;
extern int        controlupdatestarted;
extern statobj_t  **DEADPLAYER;
extern int        NUMDEAD;
extern objtype    **PLAYER;
extern objtype    *player;
extern playertype *PLAYERSTATE,*locplayerstate;

extern ROTTCHARS  *characters;

extern williamdidthis FREE;
extern statetype s_player;
extern williamdidthis WEAPONS[MAXWEAPONS];
extern boolean cybermanenabled;
extern boolean spaceballenabled;
extern boolean SpaceBallPresent;
extern boolean CybermanPresent;
extern boolean mouseenabled;
extern boolean joystickenabled;
extern boolean joypadenabled;
extern boolean joystickprogressive;
extern int joystickport;
extern int joyxmax, joyymax, joyxmin, joyymin;
extern int buttonscan[NUMBUTTONS];
extern int buttonmouse[6];
extern int buttonjoy[8];
extern boolean  buttonpoll[NUMBUTTONS];
extern boolean godmode;
extern boolean missilecam;
extern objtype       * missobj;
extern int lastpolltime;

extern int controlbuf[3];
extern int buttonbits;

extern int pausedstartedticcount;
extern boolean RefreshPause;


void     PlayNoWaySound(void);
int      GetBonusTimeForItem(int);
int      GetRespawnTimeForItem(int);
int      GetItemForWeapon(int);
void     SetWhoHaveWeapons(void);
int      MaxHitpointsForCharacter(playertype*);
void     RespawnPlayerobj(objtype*);
void     RevivePlayerobj(int,int,int,objtype*);
void     OperateElevatorSwitch(objtype*,int,int,int);
void     ResetPlayerstate(playertype*);
void     InitializeWeapons(playertype*);
void     DropWeapon(objtype*ob);
void     PollKeyboardButtons (void);
void     PollMouseButtons (void);
void     PollJoystickButtons (void);
void     PollKeyboardMove (void);
void     PollCyberman (void);
void     PollMouseMove (void);
void     PollJoystickMove (void);
void     PollControls (void);
void     AddDemoCmd (void);
void     AddRemoteCmd (void);
void     PlayerSlideMove(objtype * ob);
void     SpawnSmoke(objtype*);
void     GetBonus(objtype*,statobj_t*);
void     SpawnPlayerobj(int,int,int,int);
void     ClipPlayer(void);
void     PlayerMove ( objtype * ob );
void     Cmd_Use(objtype*);
void     Cmd_Fire (objtype*ob);
void     SpawnGunSmoke(int x, int y, int z, int angle, int bullethole);
void     SpawnBlood(objtype * ob, int angle);
void     SpawnMetalSparks(objtype * ob, int angle);
void     CheckPlayerSpecials(objtype * ob);
void     LoadPlayer ( void );
void     PlayerTiltHead (objtype * ob);
boolean  InRange (objtype *p, objtype *victim, int distance);
void     CheckUnPause ( void );
void     UpdatePlayers ( void );
void UnTargetActor ( objtype * target );


enum
 { RENORMALIZE = 1,
	PITFALL,
	PITRISE,
	COLUMNCRUSH,
	GODMODERISE,
	GODMODEFALL,
	DOGMODEFALL,
	DOGMODERISE,
	STEPUP,
	STEPDOWN
 };

extern void ResetWeapons(objtype *);
extern void SaveWeapons(objtype*);

extern int whichpath;
extern statobj_t **BulletHoles;

extern boolean vrenabled;

void SetupBulletHoleLink (int num, statobj_t * item);

#endif
